#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CONTENT_DISPLAY_LEN 50
#define	MAXLINE	 8192  /* Max text line length */

/* Cache line definition, which is simply a node of doubly linkedlist */
typedef struct cache {
  char content[MAX_OBJECT_SIZE];
  char uri[MAXLINE];
  struct cache *next;
  struct cache *prev;
  int content_len;
} cache_line;

/* Proto for cache */
void cache_init();
void free_cache();
int get_cached_obj(char *uri, char *content, int *content_len);
int put_cached_content(char *uri, char *content, int content_len);
void insert(cache_line *cache_ins);
void delete(cache_line *cache_ins);
void evict(int content_len);
void display_cache();
