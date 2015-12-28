/*
 * This cache suite using LRU algorithm to store the temporary web reponses from
 * remote server.
 */
#include "cache.h"

static cache_line *dummy;
static int free_space;

static int readcnt; /* Initially = 0 */
static sem_t mutex, w; /* Both initially = 1 */

/* unit_test - It will test the necessity of the cache suite. */
int unit_test(int argc, char **argv) {
  char content[MAX_OBJECT_SIZE];
  int content_len;
  cache_init();
  display_cache();
  put_cached_content("http://www.cmu.edu", "Hello CMU", 40);
  display_cache();
  put_cached_content("http://victorzhao1.com", "Hello Victor1", 10);
  display_cache();
  put_cached_content("http://victorzhao2.com", "Hello Victor2", 10);
  display_cache();
  put_cached_content("http://victorzhao3.com", "Hello Victor3", 10);
  display_cache();
  put_cached_content("http://victorzhao4.com", "Hello Victor4", 10);
  display_cache();
  put_cached_content("http://victorzhao5.com", "Hello Victor5", 10);
  display_cache();
  put_cached_content("http://victorzhao6.com", "Hello Victor6", 10);
  display_cache();
  get_cached_obj("http://victorzhao5.com", content, &content_len);
  printf("content %s.\n", content);
  printf("len %d.\n", content_len);
  free_cache();
  return 0;
}

/*
 * cache_init - Initialize a cache structure that
 * initalize a pointer point to dummy cache_line.
 *
 */
void cache_init() {
  dummy = (cache_line *)malloc(sizeof(cache_line));
  strcpy(dummy -> content, "This is dummy's content");
  strcpy(dummy -> uri, "This is dummy's uri");
  dummy -> content_len = 0;
  dummy -> prev = NULL;
  dummy -> next = NULL;
  free_space = MAX_CACHE_SIZE;
  readcnt = 0;
  sem_init(&mutex, 0, 1);
  sem_init(&w, 0, 1);
}

/*
 * get_cached_obj - Fetch cached content from cache structure.
 *   On error, return 1.
 */
int get_cached_obj(char *uri, char *content, int *content_len) {
  cache_line *c_line;
  P(&mutex);
  readcnt++;
  if (readcnt == 1) {
    P(&w);
  }
  V(&mutex);


  if ((dummy -> next) != NULL) {
    for (c_line = dummy -> next; c_line != NULL; c_line = c_line -> next) {
      if (!strcasecmp(c_line -> uri, uri)) {
        memcpy(content, c_line -> content, c_line -> content_len); // copy info
        *content_len = c_line -> content_len; // same to above
        P(&mutex);
        readcnt--;
        if (readcnt == 0) {
          V(&w);
        }
        V(&mutex);
        // TODO: update the cache structure
        P(&w);
        delete(c_line);
        insert(c_line);
        V(&w);
        return 0;
      }
    }
  }
  P(&mutex);
  readcnt--;
  if (readcnt == 0) {
    V(&w);
  }
  V(&mutex);
  printf("Cache obj not found.\n");
  return 1;
}

/*
 * put_cached_content - Store the new content into the cache structure.
 *  On error, return 1 instead;
 */
int put_cached_content(char *uri, char *content, int content_len) {
  if (content_len > MAX_OBJECT_SIZE) {
    printf("Error: Content length exceed the maximum object length.\n");
    return 1;
  }
  P(&w);
  if (free_space < content_len) {
    // TODO: eviction
    evict(content_len);
  }

  cache_line *c_ins = (cache_line *)malloc(sizeof(cache_line));
  strcpy(c_ins -> uri, uri);
  memcpy(c_ins -> content, content, content_len);
  c_ins -> content_len = content_len;
  insert(c_ins);
  free_space = free_space - content_len;
  V(&w);
  return 0;
}

/*
 * insert - Insert the new cache line into cache structure,
 *  which is the the linkedlist head.
 */
void insert(cache_line *cache_ins) {
  if ((dummy -> next) == NULL) { // Cache is empty, just insert.
    dummy -> next = cache_ins;
    cache_ins -> prev = dummy;
  } else {
    cache_ins -> next = dummy -> next; // let cache next point to real head
    (dummy -> next) -> prev = cache_ins; // let cache next's prev point to ins
    dummy -> next = cache_ins; // let dummy next point to ins;
    cache_ins -> prev = dummy; // ins prev point back to dummy;
  }
}

/* delete - Remove the corresponding node from the cache list */
void delete(cache_line *cache_ins) {
  /* Only cache line in the list */
  if (((dummy -> next) == cache_ins) && ((cache_ins -> next) == NULL)) {
    dummy -> next = NULL;
  }
  // point to the last line in the cache not the first line
  else if (((cache_ins -> next) == NULL) && ((cache_ins -> prev) != dummy)) {
    (cache_ins -> prev) -> next = NULL;
  }
  // point to the first line in cache not the last line
  else if (((cache_ins -> prev) == dummy) &&
           ((cache_ins -> next) -> next != NULL)) {
    dummy -> next = cache_ins -> next; // skip
    cache_ins -> prev = dummy; // point back
  }
  else if (((cache_ins -> prev) != dummy) && ((cache_ins -> next) != NULL)) {
    (cache_ins -> prev) -> next = cache_ins -> next; // skip
    (cache_ins -> next) -> prev = cache_ins -> prev; // point back
  }
}

/*
 * evict - Evict cache line to meet the requirement
 */
void evict(int content_len) {
  cache_line *tail = dummy;
  cache_line *tmp;
  while ((tail -> next) != NULL) {
    tail = tail -> next;
  }

  while ((tail != dummy) && (free_space < content_len)) {
    free_space = free_space + (tail -> content_len);
    tmp = tail;
    tail = tail -> prev;
    delete(tmp);
    free(tmp);
  }
  if (free_space < content_len) {
    printf("Freeing all content doesn't not meet the requirement.\n");
  }
}

/* free_cache - Free the whole cache structure after the malloc */
void free_cache() {
  P(&w);
  cache_line *ptr = dummy;
  while (ptr != NULL) {
    cache_line *tmp = ptr;
    ptr = ptr -> next;
    free(tmp);
  }
  V(&w);
}

/* display_cache - Display the cache structure */
void display_cache() {
  char c[CONTENT_DISPLAY_LEN];
  cache_line *ptr;
  strncpy(c, dummy -> content, CONTENT_DISPLAY_LEN);
  printf("****************************************\n");
  printf("Display the cache structure.\n");
  printf("Cache uri: %s, content length: %d, content: %s\n",
    dummy -> uri, dummy -> content_len, c);
  for (ptr = dummy -> next; ptr != NULL; ptr = ptr -> next) {
      strncpy(c, ptr -> content, CONTENT_DISPLAY_LEN);
      printf("Cache uri: %s, content length: %d, content: %s\n",
        ptr -> uri, ptr -> content_len, c);
  }
  printf("Current remaining space is %d.\n", free_space);
  printf("****************************************\n");
}
