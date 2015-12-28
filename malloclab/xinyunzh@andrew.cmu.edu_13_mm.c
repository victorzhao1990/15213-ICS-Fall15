/*
 * Xinyun (Victor) Zhao Andrew ID: xinyunzh
 * The implementation is based on simple explicit free list (double linked list)
 * with first fit find block strategy.
 *
 * I choosed mm-textbook.c as the starting point for writing this solution.
 * Most function header and macros are quite same, and even for
 * the allocated/free block pattern.
 * The major differences and optimizations are listed below:
 *
 * 1. The free list header is declared as a global pointer variable.
 *    It's initialized with NULL value. When there are any newly free blocks
 *    have being produced, the global pointer will be set to the pointer that
 *    point to the first block of the free blocks. If the free list is empty,
 *    The header will be reset to NULL again.
 *
 * 2. The pointer values for the first free block in the free list and last free
 *    block in the list are the value of initial heap_listp value and the value
 *    of Prologue block pointer.
 *
 * 3. The size reduction for the minimum free block.
 *    In the previous versions of my code, I defined the minimum free block to
 *    be 24 bytes, which contains 8 bytes overhead (footer and header)
 *    and 16 bytes for the pointers that point to the previous free block and
 *    next free block, named by PRED and SUCC respectively. In this version,
 *    I use two single word to store the pointers information. The idea is that
 *    the size of the heap will never be greater than or equal to 2^32 bytes, as
 *    it is mentioned in the handouts. I use the initial value of heap_listp as
 *    the origin to calculate the offset for the pointer information.
 *    The key value and the methods of calculation have been well defined by the
 *    macros and the global static variable that I wrote. Thus, the size for the
 *    minimum free blocks has been reduced to 16 bytes.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mm.h"
#include "memlib.h"

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */

/* I changed to this value in order to acheive high util */
#define CHUNKSIZE  (1<<11)  /* Extend heap by this amount (bytes) */

/* Single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define SIZE_PTR(p) ((size_t *)(((char *)(p)) - SIZE_T_SIZE))

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);

/* Additionals by myself */
static char *init_heap_listp; /* To store the initialized heap_listp */
#define MIN_FREE_BLK_SIZE 16 /* 8 overhead 8 ptr offset size */

/* Given block ptr bp, compute offset between init_heap_listp and bp */
#define OFFSET(bp) ((unsigned int)((char *)bp - (char *)init_heap_listp))

/* Given block ptr bp, read address of the next and previous free block */
#define GET_SUCC_FREE_BLKP(bp) ((GET((char *)bp+WSIZE)+(char *)init_heap_listp))
#define GET_PRED_FREE_BLKP(bp) ((GET(bp) + (char *)init_heap_listp))

/* Given block ptr bp, write address(offset) of the next and previous free blk*/
#define PUT_SUCC_FREE_BLKP(bp, p) (PUT((char *)bp + WSIZE, OFFSET(p)))
#define PUT_PRED_FREE_BLKP(bp, p) (PUT((char *)bp, OFFSET(p)))

static void insert(void *bp); /* Insert the curr free block into the list */
static void delete(void *bp); /* Delete the curr free block from the list */
static void print_heap();     /* Print the current heap status */
static void print_free_list();/* Print the current free list status */

/* Doesn't point to anything at the very beginning */
static char *free_list_header = NULL; /* Store the header address for free blk*/
static char *prologue_bp = NULL; /* Store the block pointer for prologue block*/

/* End my story */



/*
 * mm_init - Initialize the memory manager
 */
int mm_init(void)
{

    size_t init_size = 4 * WSIZE;
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(init_size)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */

    init_heap_listp = heap_listp;

    free_list_header = NULL;

    heap_listp += (2*WSIZE); // Move to Prologue bp
    prologue_bp = heap_listp;


    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    // mm_checkheap(__LINE__);
    return 0;
}

/*
 * malloc - Allocate a block with at least size bytes of payload
 */
void *malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;
    if (heap_listp == 0){
        mm_init();
    }

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= MIN_FREE_BLK_SIZE - DSIZE)
        asize = MIN_FREE_BLK_SIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);


    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;

    place(bp, asize);
    // mm_checkheap(__LINE__);
    return bp;
}

/*
 * free - Free a block
 */
void free(void *bp)
{
    if (bp == 0)
        return;

    size_t size = GET_SIZE(HDRP(bp));
    if (heap_listp == 0){
        mm_init();
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    coalesce(bp);
    // mm_checkheap(__LINE__);
}

/*
 * realloc - Naive implementation of realloc
 */
void *realloc(void *ptr, size_t size) /* TODO */
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    // mm_checkheap(__LINE__);
    return newptr;
}

/*
 * calloc - Allocate the block and set it to zero.
 */
void *calloc (size_t nmemb, size_t size)
{
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    // mm_checkheap(__LINE__);
    return newptr;
}


/*
 * The remaining routines are internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) /* TODO */
{

    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
      return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /*
     * Each case maps to the allocation pattern in the slides
     * Dynamic Memory Allocation: Advanced Concepts
     */
    if (prev_alloc && next_alloc) {            /* Case 1 */
      insert(bp);
    }

    else if (!prev_alloc && next_alloc) {      /* Case 2 */
      /* Optimization: Just merge block, keep original bp */
      size += GET_SIZE(HDRP(PREV_BLKP(bp)));
      bp = PREV_BLKP(bp); /* Use the prev blkp as the new ptr */
      /* The header has been modified pre-header to new size */
      PUT(HDRP(bp), PACK(size, 0));
      /* The header has been modified current footer to new size */
      PUT(FTRP(bp), PACK(size, 0));
    }

    else if (prev_alloc && !next_alloc) {      /* Case 3 next is freed */
      size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
      /* remove next blkp from the list */
      delete(NEXT_BLKP(bp));
      /* curr header 2 new size */
      PUT(HDRP(bp), PACK(size, 0));
      /* The header has been modified The new footer 2 new size */
      PUT(FTRP(bp), PACK(size, 0));
      /* insert curr blkp into the list */
      insert(bp);
    }

    else {                                     /* Case 4 */
      /*
       * Optimization: Just delete the right hand side block from the list
       *               Use the left hand side bp as the new bp.
       */
      size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
      delete(NEXT_BLKP(bp));
      bp = PREV_BLKP(bp);
      /* Prev header to new size */
      PUT(HDRP(bp), PACK(size, 0));
      /* The header has been modified. The new footer 2 new size */
      PUT(FTRP(bp), PACK(size, 0));
    }

    return bp;
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp)); /* Get curr block size */

    if ((csize - asize) >= MIN_FREE_BLK_SIZE) {
        /* Modified the header value */
        PUT(HDRP(bp), PACK(asize, 1));
        /* Use the modified header value to change the footer */
        PUT(FTRP(bp), PACK(asize, 1));
        /* remove the block from the free list */
        delete(bp);
        /* Move to the next blk, the size has been revised*/
        bp = NEXT_BLKP(bp);
        /* modified the new blk header value*/
        PUT(HDRP(bp), PACK(csize-asize, 0));
        /* use the modified new blk header value to modified the footer value */
        PUT(FTRP(bp), PACK(csize-asize, 0));
        /* Don't forget to insert the remainder back */
        insert(bp);
    }
    else { /* Cannot split */
        /* Modified the header value */
        PUT(HDRP(bp), PACK(csize, 1));
        /* Use the modified header value to change the footer */
        PUT(FTRP(bp), PACK(csize, 1));
        /* Just delete it from the free list */
        delete(bp);
    }
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize)
{
    /* First-fit search */
    void *bp;

    /* Free list is empty */
    if (free_list_header == NULL) {
       return NULL;
    }

    /* Stop at prologue, while prologue is allocted */
    for (bp = free_list_header;
        GET_ALLOC(HDRP(bp)) == 0; bp = GET_SUCC_FREE_BLKP(bp)) {
        if (asize <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL; /* No fit */
}

/* insert - insert the curr free block into the beginning of the list */
void insert(void *bp) {
  if (free_list_header == NULL) { /* Free list is empty, just insert one blk */
    /* Set the pointer for the next free block to the address of prologue */
    PUT_SUCC_FREE_BLKP(bp, prologue_bp);
    /* Set the pointer for previous free block to init_heap_listp */
    PUT_PRED_FREE_BLKP(bp, init_heap_listp);
    free_list_header = (char *)bp; /* add to list */
  } else { /* Free list is not empty */
    /* Set the pointer for the next free block to the current free list header*/
    PUT_SUCC_FREE_BLKP(bp, free_list_header);
    /* Set the pointer for the previous free blk to init_heap_listp */
    PUT_PRED_FREE_BLKP(bp, init_heap_listp);
    /* Set the ptr for previous free blk of curr first node for to curr node */
    PUT_PRED_FREE_BLKP(free_list_header, bp);
    /* Insert the new block into the list */
    free_list_header = bp;
  }
}

/* delete - Delete the current block from the list */
void delete(void *bp) {

  /* Only node in list */
  if ((GET_PRED_FREE_BLKP(bp) == init_heap_listp)
    && (GET_SUCC_FREE_BLKP(bp) == prologue_bp)) {
    free_list_header = NULL; /* Simply reset the free_list_header */
  }
  /* bp points to the last node in list, not the first node */
  else if ((GET_PRED_FREE_BLKP(bp) != init_heap_listp)
    && (GET_SUCC_FREE_BLKP(bp) == prologue_bp)) {
    /* Set the previous free block's pointer for next free block to prologue */
    PUT_SUCC_FREE_BLKP(GET_PRED_FREE_BLKP(bp), prologue_bp);
  }
  /* bp points to first node in list, not the last node */
  else if ((GET_PRED_FREE_BLKP(bp) == init_heap_listp)
    && (GET_SUCC_FREE_BLKP(bp) != prologue_bp)) {
    /* Set the next free block's pointer for previous free block to init_listp*/
    PUT_PRED_FREE_BLKP(GET_SUCC_FREE_BLKP(bp), init_heap_listp);
    /* Reset the free_list_header to the address of next free block */
    free_list_header = GET_SUCC_FREE_BLKP(bp); /* list ptr point to next node */
  }
  /*
   * Normal case, the internal node in the list,
   * not head, node end, there are nodes in two sides
   */
  else if ((GET_PRED_FREE_BLKP(bp) != init_heap_listp)
    && (GET_SUCC_FREE_BLKP(bp) != prologue_bp)) {
    /* Let the next free block connect to the previous block */
    PUT_PRED_FREE_BLKP(GET_SUCC_FREE_BLKP(bp), GET_PRED_FREE_BLKP(bp));
    /* Let the previous free block connect to the next free block */
    PUT_SUCC_FREE_BLKP(GET_PRED_FREE_BLKP(bp), GET_SUCC_FREE_BLKP(bp));
  }
}

/*
 * mm_checkheap - Check the heap for correctness.
 */
void mm_checkheap(int lineno)
{

    char *check_bp;
    char *free_bp;
    int blk_idx;
    int free_blk_idx;
    int free_blk_counter = 0;


    /* Prologue block check */
    /* Since heap_listp is already point to prologue blk */
    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE)) {
      printf("*******************************************\n");
      printf("Checking Prologue block. \n");
      printf("Prologue size doesn't match in header. \n");
      printf("Error happens at Line %d. \n", lineno);
      print_heap();
      printf("Prologue block check ends. \n");
      printf("*******************************************\n");
      exit(1);
    }

    if ((GET_SIZE(FTRP(heap_listp)) != DSIZE)) {
      printf("*******************************************\n");
      printf("Checking Prologue block. \n");
      printf("Prologue size doesn't match in footer.\n");
      printf("Error happens at Line %d. \n", lineno);
      print_heap();
      printf("Prologue block check ends. \n");
      printf("*******************************************\n");
      exit(1);
    }

    if (!(GET_ALLOC(HDRP(heap_listp)))) {
      printf("*******************************************\n");
      printf("Checking Prologue block. \n");
      printf("Prologue header alloc bit has been modified. \n");
      printf("Error happens at Line %d. \n", lineno);
      print_heap();
      printf("Prologue block check ends. \n");
      printf("*******************************************\n");
      exit(1);
    }

    if (!(GET_ALLOC(FTRP(heap_listp)))) {
      printf("*******************************************\n");
      printf("Checking Prologue block. \n");
      printf("Prologue footer alloc bit has been modified. \n");
      printf("Error happens at Line %d. \n", lineno);
      print_heap();
      printf("Prologue block check ends. \n");
      printf("*******************************************\n");
      exit(1);
    }

    /* Block check */
    blk_idx = 0;

    for (check_bp = NEXT_BLKP(heap_listp);
      GET_SIZE(HDRP(check_bp)) > 0; check_bp = NEXT_BLKP(check_bp)) {

      if ((GET_SIZE(HDRP(check_bp)) != (GET_SIZE(FTRP(check_bp))))) {
        printf("*******************************************\n");
        printf("Now checking block %d. \n", blk_idx);
        printf("Block size doesn't match. Footer and header. \n");
        printf("Error happens at Line %d. \n", lineno);
        print_free_list();
        print_heap();
        printf("Block check ends. \n");
        printf("*******************************************\n");
        exit(1);
      }

      if ((GET_SIZE(HDRP(check_bp))) < MIN_FREE_BLK_SIZE) {
        printf("*******************************************\n");
        printf("Now checking block %d. \n", blk_idx);
        printf("Block size less than MIN_FREE_BLK_SIZE(24). Size is %u. \n"
          , GET_SIZE(HDRP(check_bp)));
        printf("Error happens at Line %d. \n", lineno);
        print_heap();
        printf("Block check ends. \n");
        printf("*******************************************\n");
        exit(1);
      }
      if ((GET_SIZE(HDRP(check_bp)) % DSIZE)) {
        printf("*******************************************\n");
        printf("Now checking block %d. \n", blk_idx);
        printf("Block address doesn't align. \n");
        printf("Error happens at Line %d. \n", lineno);
        print_heap();
        printf("Block check ends. \n");
        printf("*******************************************\n");
        exit(1);
      }
      if ((GET_ALLOC(HDRP(check_bp))) != (GET_ALLOC(FTRP(check_bp)))) {
        printf("*******************************************\n");
        printf("Now checking block %d. \n", blk_idx);
        printf("Alloc bits don't match. \n");
        printf("Error happens at Line %d. \n", lineno);
        print_heap();
        printf("Block check ends. \n");
        printf("*******************************************\n");
        exit(1);
      }

      /* Coalescing check */
      if ((GET_SIZE(NEXT_BLKP(check_bp))) > 0) {
        if ((!GET_ALLOC(HDRP(check_bp)))
          && (!GET_ALLOC(HDRP(NEXT_BLKP(check_bp))))) {
          printf("*******************************************\n");
          printf("Now checking block %d. \n", blk_idx);
          printf("Block %d and block %d didn't coalese. \n"
            , blk_idx, blk_idx + 1);
          printf("Error happens at Line %d. \n", lineno);
          print_heap();
          printf("Block check ends. \n");
          printf("*******************************************\n");
          exit(1);
        }
      }

      if (!(GET_ALLOC(HDRP(check_bp)))) {
        free_blk_counter++;
      }
      blk_idx++;
    }

    /* Heap boundary check */
    if ((mem_heap_hi() + 1) != check_bp) {
      printf("*******************************************\n");
      printf("Now checking heap boundaries. \n");
      printf("It seems that bp doesn't reach heap boundary. \n");
      printf("Error happens at Line %d. \n", lineno);
      print_heap();
      printf("*******************************************\n");
      exit(1);
    }

    /* Reach to epilogue block */

    /* Epilogue block check */
    if ((GET_SIZE(HDRP(check_bp)))) {
      printf("*******************************************\n");
      printf("Epilogue block check begins. \n");
      printf("Epilogue size error. \n");
      printf("Error happens at Line %d \n", lineno);
      print_heap();
      printf("Epilogue block check ends. \n");
      printf("*******************************************\n");
      exit(1);
    }
    if (!(GET_ALLOC(HDRP(check_bp)))) {
      printf("*******************************************\n");
      printf("Epilogue block check begins. \n");
      printf("Epilogue alloc bit error. \n");
      printf("Error happens at Line %d. \n", lineno);
      print_heap();
      printf("Epilogue block check ends. \n");
      printf("*******************************************\n");
      exit(1);
    }


    /* Free list check */
    /* consistent check */
    free_blk_idx = 0;
    if (free_list_header != NULL) {
      free_bp = free_list_header;
      if ((GET_SUCC_FREE_BLKP(free_bp) == prologue_bp)
        && (GET_PRED_FREE_BLKP(free_bp) != init_heap_listp)) {
          printf("*******************************************\n");
          printf("Free block check begins. \n");
          printf("First blk pred ptr doesn't point to NULL.\n");
          printf("Error happens at Line %d \n", lineno);
          print_free_list();
          printf("*******************************************\n");
          exit(1);
      }

      free_bp = GET_SUCC_FREE_BLKP(free_bp);
      free_blk_idx = 1;

      while (free_bp != prologue_bp) {
        if (((GET_SUCC_FREE_BLKP(free_bp))) > ((char *)mem_heap_hi())) {
          printf("*******************************************\n");
          printf("Free block check begins. \n");
          printf("Succ ptr %p out of bound. Heap hi. \n"
            , GET_SUCC_FREE_BLKP(free_bp));
          printf("Error happens at Line %d. \n", lineno);
          print_free_list();
          printf("*******************************************\n");
          exit(1);
        }

        if (((GET_SUCC_FREE_BLKP(free_bp)) < ((char *)mem_heap_lo()))) {
          printf("*******************************************\n");
          printf("Free block check begins. \n");
          printf("Succ ptr out of bound. Heap lo. \n");
	        printf("Succ ptr is %p mem_heap_lo is %p. \n"
            , GET_SUCC_FREE_BLKP(free_bp), mem_heap_lo());
          printf("Error happens at Line %d. \n", lineno);
          print_free_list();
          printf("*******************************************\n");
          exit(1);
        }

        if ((GET_PRED_FREE_BLKP(free_bp) != init_heap_listp)
          && (((GET_PRED_FREE_BLKP(free_bp))) < ((char *)mem_heap_lo()))) {
          printf("*******************************************\n");
          printf("Prev ptr out of bound. Heap lo. \n");
          printf("Prev ptr is %p mem_heap_lo is %p. \n"
            , GET_PRED_FREE_BLKP(free_bp), mem_heap_lo());
          printf("Error happens at Line %d. \n", lineno);
          print_free_list();
          printf("*******************************************\n");
          exit(1);
        }

        if (((GET_SUCC_FREE_BLKP(free_bp)) != prologue_bp)
 && ((GET_PRED_FREE_BLKP(GET_SUCC_FREE_BLKP(free_bp))) != ((char *)free_bp))) {
          printf("*******************************************\n");
          printf("Free block check begins. \n");
          printf("Succ's pred ptr doesn't point to curr free blk.\n");
          printf("Error happens at Line %d. \n", lineno);
          print_free_list();
          printf("*******************************************\n");
          exit(1);
        }
        free_bp = GET_SUCC_FREE_BLKP(free_bp);
        free_blk_idx++;
      }
    }

    /* Free blocks number check */
    if (free_blk_counter != free_blk_idx) {
      printf("*******************************************\n");
      printf("Free blk counter check begins. \n");
      printf("Blocks traversal counting is %d, free list counting is %d \n"
        , free_blk_counter, blk_idx);
      printf("Error happens at Line %d. \n", lineno);
      print_free_list();
      printf("*******************************************\n");
      exit(1);
    }
}

/* print_heap - Print the current heap distribution status */
static void print_heap() {
  char *check_bp = heap_listp;
  int blk_idx = 0;
  /* Print prologue status */
  printf("Prlg Blk: Bp %p. Hd sz %u. Hd al bt %d. Ft sz %u. Ft al bt %d. \n",
    check_bp, GET_SIZE(HDRP(check_bp)), GET_ALLOC(HDRP(check_bp))
    , GET_SIZE(FTRP(check_bp)), GET_ALLOC(FTRP(check_bp)));

  for (check_bp = NEXT_BLKP(check_bp);
    GET_SIZE(HDRP(check_bp)) > 0; check_bp = NEXT_BLKP(check_bp)) {
    printf("Blk %d: Addr %p. Hd sz %u. Hd al bt %d. Ft sz %u. Ft al bt %d. \n"
      , blk_idx, check_bp, GET_SIZE(HDRP(check_bp)), GET_ALLOC(HDRP(check_bp))
      , GET_SIZE(FTRP(check_bp)), GET_ALLOC(FTRP(check_bp)));
    blk_idx++;
  }

  printf("Eplg Blk: Bp %p. Hd sz %u. Hd al bt %u. \n"
    , check_bp, GET_SIZE(HDRP(check_bp)), GET_ALLOC(HDRP(check_bp)));
}

/* print_free_list - Print all blocks' information in the current free list*/
static void print_free_list() {
  char *free_bp = free_list_header;
  int free_blk_idx = 0;

  if (free_bp == NULL) {
    printf("Free list is empty. \n");
  } else {
    for (; free_bp != prologue_bp;
      free_bp = GET_SUCC_FREE_BLKP(free_bp)) {
        printf("Free Blk %d: Addr %p. Hd sz %u. Hd al bt %d. Ft sz %u."
          , free_blk_idx, free_bp, GET_SIZE(HDRP(free_bp))
          , GET_ALLOC(HDRP(free_bp)), GET_SIZE(FTRP(free_bp)));
        printf("Ft al bt %d. Succ ptr %p. Prev ptr %p. \n"
          , GET_ALLOC(FTRP(free_bp)), GET_SUCC_FREE_BLKP(free_bp)
          , GET_PRED_FREE_BLKP(free_bp));
      free_blk_idx++;
    }
  }
}
