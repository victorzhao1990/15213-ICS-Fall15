/*
 * proxy-cached.c - a multi-threaded web proxy using LRU cache
 *  Started from tiny.c and echoserver and echoclient
 *  by Xinyun (Victor) Zhao Andrew ID: xinyunzh
 */

/*
 * Dev Note:
 * Updated on Dec. 2 2015
 *  1. Able to handle both Simple-Request and Full-Request defined in RFC 1945
 * Updated on Dec. 3 2015
 *  2. Able to using pthread to achieve current requests.
 * Updated on Dec. 6 2015
 *  3. Cached version finished.
 * Updated on Dec. 7 2015
 *  4. Fix bug in get_cached_obj.
 *  5. Still crash when visiting the dns unresolved
 *  sites.
 *  6. Add signal handler to free cache when quit the program
 * Updated on Dec. 8 2015
 *  7. Able to survive when malformed uri coming into through telnet
 */
#include <stdio.h>
#include "csapp.h"
#include <pthread.h>
#include "cache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
										char *longmsg);
void process_requesthdrs(rio_t *rp, char *hdr2server, char *server_hostname);
int parse_uri(char *uri, char *abs_path, char *server_hostname,
								char *server_port);
void *thread(void *vargp);
void do_server(int fd, int connfd2server, char *request2server, char *uri);
void sigint_handler(int sig);
int host_verify(const char *host, char *port);

/* main - The main routine of web proxy */
int main(int argc, char **argv) {
  int listenfd;
  int *connfdp;
  socklen_t clientlen;
  pthread_t tid;

  /* Enough space for any address */ //line:netp:echoserveri:sockaddrstorage
  struct sockaddr_storage clientaddr;
  char client_hostname[MAXLINE], client_port[MAXLINE];
  signal(SIGPIPE, SIG_IGN); // don't want to terminate the process due to sig
	signal(SIGINT, sigint_handler);
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(0);
  }
  cache_init();
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    if ((connfdp = malloc(sizeof(int))) == NULL) {
      printf("Malloc error!\n");
      return 0;
    }
    *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
										client_port, MAXLINE, 0);
    printf("Connected to (%s, %s)\n", client_hostname, client_port);
    Pthread_create(&tid, NULL, thread, connfdp);
  }
  exit(0);
}

/* Thread routine */
void *thread(void *vargp) {
  int connfd = *((int *)vargp);
  Pthread_detach(pthread_self());
  free(vargp);
  doit(connfd);
  if (close(connfd) < 0) {
    printf("Close error!");
  }
  return NULL;
}

/* doit - Similar to the function in Tiny. Parse URIs and connect to server*/
void doit(int fd) {
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  rio_t rio;

  char request2server[MAXLINE];
  char abs_path[MAXLINE];
  char server_hostname[MAXLINE];
  char server_port[MAXLINE];
  char hdr2server[MAXLINE];
  int connfd2server;

  char cached_content[MAX_OBJECT_SIZE];
  int cached_content_len;
	int hostveri_rc;
	char hostveri_err_msg[MAXLINE];

  /* Read request line*/
  Rio_readinitb(&rio, fd);
  if (!Rio_readlineb(&rio, buf, MAXLINE)) { //line:netp:doit:readrequest
    return;
  }

  printf("%s", buf); // display line and headers

  // TODO: deal with malformed GET request

  sscanf(buf, "%s %s %s", method, uri, version); //line:netp:doit:parserequest
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not Implemented",
      "Proxy does not implement this method");
    return;
  }

  // convert http version to 1.0
  if ((!strcasecmp(version, "HTTP/1.1")) || (strlen(version) == 0)) {
    strcpy(version, "HTTP/1.0");
  } else if (strcasecmp(version, "HTTP/1.0")) {
    // neither consistent with HTTP/1.0 nor the Simple-Request (no version)
    clienterror(fd, method, "400", "Bad Request",
      "The HTTP version is neither HTTP/1.1 nor HTTP/1.0");
    return;
  }

  // TODO: Parse URI and resend the request
  if (parse_uri(uri, abs_path, server_hostname, server_port)) {
    // parse failed
    clienterror(fd, method, "400", "Bad Request", "URI format error");
    return;
  }
	if ((hostveri_rc = host_verify(server_hostname, server_port)) != 0) {
		strcpy(hostveri_err_msg, gai_strerror(hostveri_rc));
		clienterror(fd, method, "400", "Bad Request", hostveri_err_msg);
    return;
	}

  if (!get_cached_obj(uri, cached_content, &cached_content_len)) { // in cache
    printf("Content in cache!\n");
    rio_writen(fd, cached_content, cached_content_len);
    return; // end
  }

  // Init request line
  sprintf(request2server, "%s %s %s\r\n", method, abs_path, version);
  // TODO: process hdr
  process_requesthdrs(&rio, hdr2server, server_hostname);
  // Concat the line and header
  strcat(request2server, hdr2server);

  // TODO: connect to server
  if ((connfd2server = open_clientfd(server_hostname, server_port)) < 0) {
    printf("Establish to server error!\n");
		clienterror(fd, method, "500", "Internal error"
			, "Establish to server error!\n");
    return;
  }
  do_server(fd, connfd2server, request2server, uri);
}

/* do_server - doit's replica targeting the real server */
void do_server(int fd, int connfd2server, char *request2server, char *uri) {
  rio_t rio_server;
  int request2serverlen;
  char response_from_server[MAXLINE];
  int response_len;
  char cached_content[MAX_OBJECT_SIZE];
  int cached_content_len = 0; /* empty at beginning */
  int flag_so_big = 0;

  rio_readinitb(&rio_server, connfd2server);
  request2serverlen = strlen(request2server);

  if (rio_writen(connfd2server, request2server, request2serverlen)
        != request2serverlen) {
    printf("rio_writen error!");
    return;
  }

  while ((response_len = rio_readnb(&rio_server, response_from_server, MAXLINE))
            > 0) { // read until EOF
    rio_writen(fd, response_from_server, response_len); // just write with stuff
    if ((cached_content_len + response_len) <= MAX_OBJECT_SIZE) { // not so big
      memcpy(cached_content + cached_content_len, response_from_server,
        response_len);
      cached_content_len = cached_content_len + response_len;
    } else {
      printf("Cannot add to cache. Target is so big!\n");
      flag_so_big = 1;
    }
  }
  if (!flag_so_big) {
    put_cached_content(uri, cached_content, cached_content_len);
  }

  if (close(connfd2server) < 0) {
    printf("Close error\n");
    return;
  }
}

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
     char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Proxy server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */

/*
 * process_requesthdrs - read HTTP request headers and make a new one
 */
/* $begin process_requesthdrs */
void process_requesthdrs(rio_t *rp, char *hdr2server, char *server_hostname)
{
    char buf[MAXLINE];
    // TODO: init header
    sprintf(hdr2server, "Host: %s\r\n", server_hostname);
    strcat(hdr2server, user_agent_hdr);
    strcat(hdr2server, conn_hdr);
    strcat(hdr2server, proxy_conn_hdr);

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
      Rio_readlineb(rp, buf, MAXLINE);
      if ((!strncasecmp(buf, "Host:", strlen("Host:")))
	&& (!strncasecmp(buf, "User-Agent:", strlen("User-Agent:")))
	&& (!strncasecmp(buf, "Connection:", strlen("Connection:")))
	&& (!strncasecmp(buf, "Proxy-Connection:", strlen("Proxy-Connection")))) {
	// Not the default header info
        strcat(hdr2server, buf);
      }
    }
    strcat(hdr2server, "\r\n"); // don't forget the add the termination
    return;
}
/* $end process_requesthdrs */

/*
 * parse_uri - parse URI into abs_path, server_hostname and server_port
 *             return 0 if successfully parsed, 1 if failed(malformed req)
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *abs_path, char *server_hostname,
	char *server_port) {
  // http_URL = "http:" "//" host [ ":" port ] [ abs_path ]
  char uri_cpy[MAXLINE];
  char *port_ptr;
  char *abs_ptr;
  int port = 80;
  // init
  strcpy(uri_cpy, uri);
  strcpy(abs_path, "/");
  if (!strncasecmp(uri_cpy, "http://", strlen("http://"))) {
    sscanf(uri_cpy, "http://%s", uri_cpy); // remove the prefix
  } else {
    return 1; // not contain the http:// prefix malformed uri
  }

  if ((port_ptr = strchr(uri_cpy, ':')) != NULL) {
    *port_ptr = '\0'; // termination uri_cpy
    strcpy(server_hostname, uri_cpy);

    port_ptr++; // point to real number
    port = atoi(port_ptr);

    if ((abs_ptr = strchr(port_ptr, '/')) != NULL) {
      abs_ptr++;
      strcat(abs_path, abs_ptr);
    }
  } else { // no port num, only abs path
    if ((abs_ptr = strchr(uri_cpy, '/')) != NULL) {
      *abs_ptr = '\0';
      strcpy(server_hostname, uri_cpy);
      abs_ptr++;
      strcat(abs_path, abs_ptr);
    } else { // just like http://www.cmu.edu
      strcpy(server_hostname, uri_cpy);
    }
  }
  sprintf(server_port, "%d", port); // convert it back to string
  return 0;
}
/* $end parse_uri */

/*
 * sigint_handler - The kernel sends a SIGINT to proxy whenver the user type
 *   ctrl-c at keyboard. Free the cache if necessary.
 */
void sigint_handler(int sig) {
  sigset_t mask, prev_mask;

  Sio_puts("Caught SIGINT!\n");
  // G3
  Sigemptyset(&mask);
  Sigfillset(&mask);

  Sigprocmask(SIG_BLOCK, &mask, &prev_mask);
  free_cache();

  Sigprocmask(SIG_SETMASK, &prev_mask, NULL);

  _exit(0);
}

/*
 * host_verify - Incase of name or service not known error, deal with it.
 *       Simply use the getaddrinfo to skip the invalid hostname or port
 */
int host_verify(const char *host, char *port) {
	struct addrinfo hints, *listp;
	int rc;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_flags |= AI_ADDRCONFIG;
	if ((rc = getaddrinfo(host, port, &hints, &listp)) != 0) {
		printf("getaddrinfo error: %s\n", gai_strerror(rc));
		return rc;
	}
	freeaddrinfo(listp);
	return 0;
}
