// Xinyun (Victor) Zhao Andrew ID: xinyunzh
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cachelab.h"

typedef struct cache_line {
  unsigned valid;
  unsigned long tag;
  unsigned long blockOff;
  int age;
} c_line; 

int vflag = 0;
int hit = 0;
int miss = 0;
int evict = 0;

void readTrace(char *path, int svalue, int S, int E, int bvalue, c_line cacheSim[S][E]);

void processTrace(char identifier, unsigned long address, int size, int svalue, int S, int E, int bvalue, c_line cacheSim[S][E]);

void operation(long tag, long idx, long offset, int S, int E, c_line cacheSim[S][E]);


int main (int argc, char **argv) {
  int svalue = 0;
  int evalue = 0;
  int bvalue = 0;
  char *tvalue = NULL;
  int index;
  int c;
  int S = 0;
  int E = 0;

  opterr = 0;

  while ((c = getopt(argc, argv, "vs:E:b:t:")) != -1) {
    switch (c) {
    case 'v':
      vflag = 1;
      break;
    case 's':
      svalue = atoi(optarg);
      break;
    case 'E':
      evalue = atoi(optarg);
      break;
    case 'b':
      bvalue = atoi(optarg);
      break;
    case 't':
      tvalue = optarg;
      break;
    default:
      abort();
    }
  }
  // Only for testing output
  // printf("vflag = %d, svalue = %d, evalue = %d, bvalue = %d, tvalue = %s\n", vflag, svalue, evalue, bvalue, tvalue);

  for (index = optind; index < argc; index++) {
    printf("Non-option argument %s\n", argv[index]);
  }

  // Battle begins
  
  S = 1 << svalue;
  E = evalue;
  c_line cacheSim[S][E];
  memset(cacheSim, 0, S * E * sizeof(c_line));
  readTrace(tvalue, svalue, S,  E, bvalue, cacheSim);
  
  printf("hits:%d misses:%d evictions:%d\n", hit, miss, evict);
  printSummary(hit, miss, evict);
  return 0;  
}

void readTrace(char *path, int svalue, int S, int E, int bvalue, c_line cacheSim[S][E]) {
  FILE *pFile; 
  pFile = fopen(path, "r");
  char identifier;
  unsigned long address;
  int size;

  while (fscanf(pFile, " %c %lx,%d", &identifier, &address, &size) == 3) {
    if ('I' == identifier) {
      continue;
    }

    if (1 == vflag) {
      printf("%c %lx,%d", identifier, address, size);
    }
    processTrace(identifier, address, size, svalue, S, E, bvalue, cacheSim);
  }

  fclose(pFile);
}

void processTrace(char identifier, unsigned long address, int size, int svalue, int S, int E, int bvalue, c_line cacheSim[S][E]) {
  // Masking
  long one = 1;
  long tagMask = (one << 63) >> (64 - (svalue + bvalue + 1));
  long lowTagMask = ~((one << 63) >> (svalue + bvalue - 1));
  long offsetMask = ~((one << 63) >> (63 - bvalue));
  long idxMask = ~(tagMask | offsetMask);

  long tag = ((address & tagMask) >> (svalue + bvalue)) & lowTagMask;
  long idx = (address & idxMask) >> bvalue;
  long offset = address & offsetMask;
  
  switch (identifier) {
  case 'L':
    operation(tag, idx, offset, S, E, cacheSim);
    if (1 == vflag) {
      printf("\n");
    }
    break;

  case 'S': // should be same
    operation(tag, idx, offset, S, E, cacheSim);
    if (1 == vflag) {
      printf("\n");
    }
    break;

  case 'M':
    operation(tag, idx, offset, S, E, cacheSim);
    operation(tag, idx, offset, S, E, cacheSim);
    if (1 == vflag) {
      printf("\n");
    }
    break;

  default:
    return;
  }
}

void operation(long tag, long idx, long offset, int S, int E, c_line cacheSim[S][E]) {
  
  // TODO: load opr
  c_line *set = cacheSim[idx];
  int i = 0;
  int j = 0;
  int fullFlag = 1;

  for (i = 0; i < E; i++) {
    if (set[i].valid == 1) {
      if (set[i].tag == tag) {
	hit++;
	if (1 == vflag) {
	  printf(" hit ");
	}
	// update rest age field
        if (E > 1) {
	  set[i].age = 1;
	  for (j = 0; j < E; j++) {
	    if (i != j) {
	      set[j].age++; // older
	    }
	  }
	}
	return;
      }
    }
  }
  // miss
  miss++;
  if (1 == vflag) {
    printf(" miss ");
  }

  for (i = 0; i < E; i++) {
    if (0 == set[i].valid) {
      fullFlag = 0;
      // load into the cold one
      set[i].valid = 1;
      set[i].tag = tag;
      set[i].age = 0;
      break;
    }
  }
  if (1 == fullFlag) { //evict
    if (1 == vflag) {
      printf(" eviction ");
    }
    evict++;
    int setIdx = 0;
    int preAge = 1;
    for (i = 0; i < E; i++) {
      if (set[i].age > preAge) {
	setIdx = i;
      }
    }
    //replace
    set[setIdx].valid = 1;
    set[setIdx].tag = tag;
    set[setIdx].age = 0;
  } 
  for (i = 0; i < E; i++) {
    set[i].age++;
  }
}
