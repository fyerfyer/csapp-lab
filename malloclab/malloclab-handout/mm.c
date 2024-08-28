#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "fyerfyer's team",
    /* First member's full name */
    "fyerfyer",
    /* First member's email address */
    "fuy60703@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4           /* Word and header/footer size (bytes) */
#define DSIZE 8           /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */
#define OVERHEAD 8        /* Overhead of header and footer (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp, compute address of its predecessor and successor */
#define PRED(bp)  (*(char **)(bp))
#define SUCC(bp)  (*(char **)((char *)(bp) + WSIZE))

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
static char *free_listp = 0;  /* Pointer to first free block */
static char *next_fitp = 0;   /* Next fit pointer */

/* Function prototypes */
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void *coalesce(void *bp);
static void insert_block(void *bp);
static void remove_block(void *bp);

/*
 * mm_init - Initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(OVERHEAD, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(OVERHEAD, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));  /* Epilogue header */
    free_listp = NULL;
    next_fitp = heap_listp + (2 * WSIZE); /* Set next_fitp to start after prologue */

    /* Extend the heap by CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    /* Adjust block size to include overhead and alignment reqs. */
    asize = MAX(ALIGN(size) + DSIZE, 2 * DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block involves coalescing the block with adjacent free blocks.
 */
void mm_free(void *ptr)
{
    if (ptr == NULL)
        return;

    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    if (ptr == NULL)
        return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/*
 * extend_heap - Extend heap with a new free block and return its pointer.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));       /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));       /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * find_fit - Find a fit for a block with asize bytes.
 */
static void *find_fit(size_t asize)
{
    char *bp = next_fitp;

    /* First search from the next_fitp */
    for ( ; bp != NULL; bp = SUCC(bp)) {
        if (GET_SIZE(HDRP(bp)) >= asize) {
            next_fitp = bp;
            return bp;
        }
    }

    /* If no fit found, search from the beginning */
    for (bp = free_listp; bp != next_fitp; bp = SUCC(bp)) {
        if (GET_SIZE(HDRP(bp)) >= asize) {
            next_fitp = bp;
            return bp;
        }
    }

    return NULL; /* No fit found */
}

/*
 * place - Place a block of asize bytes at the beginning of free block bp.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2 * DSIZE)) {
        /* Split the block */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        remove_block(bp);
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insert_block(bp);
    } else {
        /* No split */
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        remove_block(bp);
    }
}

/*
 * coalesce - Coalesce the free block if adjacent blocks are free.
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        insert_block(bp);
    } else if (prev_alloc && !next_alloc) {    /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_block(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insert_block(bp);
    } else if (!prev_alloc && next_alloc) {    /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        remove_block(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insert_block(bp);
    } else {                                   /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        remove_block(PREV_BLKP(bp));
        remove_block(NEXT_BLKP(bp));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insert_block(bp);
    }
    return bp;
}

/*
 * insert_block - Insert a block into the free list. The block is inserted in the front
 */
static void insert_block(void *bp) 
{
    SUCC(bp) = free_listp;         /* The back of bp points to free_listp */
    PRED(bp) = NULL;               /* The prev of bp is NULL */
    
    if (free_listp != NULL) {      
        PRED(free_listp) = bp;     /* The prev of free_listp now points to bp */
    }
    
    free_listp = bp;               /* Update the new free_listp pointer */
}

/*
 * remove_block - Remove a block from the free list.
 */
static void remove_block(void *bp) 
{
    /* Same as linked list removal */
    if (PRED(bp) != NULL) {        
        SUCC(PRED(bp)) = SUCC(bp); 
    } else {                       
        free_listp = SUCC(bp);     
    }
    
    if (SUCC(bp) != NULL) {       
        PRED(SUCC(bp)) = PRED(bp);
    }
}
