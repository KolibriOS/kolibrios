/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <menuet/sem.h>

typedef struct BLOCK {
  size_t size;
  struct BLOCK *next;
  int bucket;
} BLOCK;

#define BEFORE(bp)	((BLOCK *)((char *)bp - *(int *)((char *)bp - 4) - 8))
#define BEFSZ(bp)	(*(size_t *)((char *)bp - 4))
#define ENDSZ(bp)	(*(size_t *)((char *)bp + bp->size + 4))
#define AFTER(bp)	((BLOCK *)((char *)bp + bp->size + 8))
#define DATA(bp)	((char *)&(bp->next))

#define NUMSMALL	0
#define ALIGN		8
#define SMALL		(NUMSMALL*ALIGN)

DECLARE_STATIC_SEM(malloc_mutex)

static BLOCK *slop = 0;
static BLOCK *freelist[30];
#if NUMSMALL
static BLOCK *smallblocks[NUMSMALL];
#endif

static inline void malloc_lock(void)
{
 sem_lock(&malloc_mutex);
}

static inline void malloc_unlock(void)
{
 sem_unlock(&malloc_mutex);
}

#define MIN_SAVE_EXTRA	64
#define BIG_BLOCK	4096

#define DEBUG 0

#if DEBUG
static void
check(BLOCK *b)
{
  printf("check %08x %d %08x %d\n", b, b->size, &(ENDSZ(b)), ENDSZ(b));
}
#define CHECK(p) do { check(p); assert(p->size == ENDSZ(p)); consistency(); } while (0)
#define CHECK1(p) do { check(p); assert(p->size == ENDSZ(p)); } while (0)
static void
consistency()
{
#if 0
  int b;
  BLOCK *bl;
  if (slop)
    CHECK1(slop);
  for (b=0; b<32; b++)
    for (bl=freelist[b]; bl; bl=bl->next)
      CHECK1(bl);
#endif
}
#else
#define CHECK(p)
#endif

static inline int
size2bucket(size_t size)
{
  int rv=0;
  size>>=2;
  while (size)
  {
    rv++;
    size>>=1;
  }
  return rv;
}

static inline int
b2bucket(BLOCK *b)
{
  if (b->bucket == -1)
    b->bucket = size2bucket(b->size);
  return b->bucket;
}

static inline BLOCK *
split_block(BLOCK *b, size_t size)
{
  BLOCK *rv = (BLOCK *)((char *)b + size+8);
#if DEBUG
  printf("  split %u/%08x to %u/%08x, %u/%08x\n",
	 b->size, b, size, b, b->size - size - 8, rv);
#endif
  rv->size = b->size - size - 8;
  rv->bucket = -1;
  b->size = size;
  ENDSZ(b) = b->size;
  ENDSZ(rv) = rv->size;
  CHECK(b);
  CHECK(rv);
  return rv;
}

#define RET(rv) CHECK(rv); ENDSZ(rv) |= 1; rv->size |= 1; return DATA(rv)

void * malloc(size_t size)
{
  int b, chunk_size;
  BLOCK *rv, **prev;
  static BLOCK *expected_sbrk = 0;

  if (size<ALIGN) size = ALIGN;
  size = (size+(ALIGN-1))&~(ALIGN-1);
#if DEBUG
  printf("malloc(%u)\n", size);
#endif
 
#if NUMSMALL
  if (size < SMALL)
  {
    rv = smallblocks[size/ALIGN];
    if (rv)
    {
      smallblocks[size/ALIGN] = rv->next;
      return DATA(rv);
    }
  }
#endif

  if (slop && slop->size >= size)
  {
    rv = slop;
#if DEBUG
    printf("  using slop %u/%08x\n", slop->size, slop);
#endif
    if (slop->size >= size+MIN_SAVE_EXTRA)
    {
      slop = split_block(slop, size);
#if DEBUG
      printf("  remaining slop %u/%08x\n", slop->size, slop);
#endif
    }
    else
      slop = 0;
    RET(rv);
  }

  b = size2bucket(size);
  prev = &(freelist[b]);
  for (rv=freelist[b]; rv; prev=&(rv->next), rv=rv->next)
  {
    if (rv->size >= size && rv->size < size+size/4)
    {
      *prev = rv->next;
      RET(rv);
    }
  }

  while (b < 30)
  {
    prev = &(freelist[b]);
#if DEBUG
    printf("  checking bucket %d\n", b);
#endif
    for (rv=freelist[b]; rv; prev=&(rv->next), rv=rv->next)
      if (rv->size >= size)
      {
#if DEBUG
	printf("    found size %d/%08x\n", rv->size, rv);
#endif
	*prev = rv->next;
	if (rv->size >= size+MIN_SAVE_EXTRA)
	{
#if DEBUG
	  printf("    enough to save\n");
#endif
	  if (slop)
	  {
	    b = b2bucket(slop);
#if DEBUG
	    printf("    putting old slop %u/%08x on free list %d\n",
		   slop->size, slop, b);
#endif
	    slop->next = freelist[b];
	    freelist[b] = slop;
	  }
	  slop = split_block(rv, size);
#if DEBUG
	  printf("    slop size %u/%08x\n", slop->size, slop);
#endif
	}
	RET(rv);
      }
    b++;
  }

  chunk_size = size+16; /* two ends plus two placeholders */
  rv = (BLOCK *)sbrk(chunk_size);
  if (rv == (BLOCK *)(-1))
    return 0;
#if DEBUG
  printf("sbrk(%d) -> %08x, expected %08x\n", chunk_size, rv, expected_sbrk);
#endif
  if (rv == expected_sbrk)
  {
    expected_sbrk = (BLOCK *)((char *)rv + chunk_size);
    /* absorb old end-block-marker */
#if DEBUG
    printf("  got expected sbrk\n");
#endif
    rv = (BLOCK *)((char *)rv - 4);
  }
  else
  {
    expected_sbrk = (BLOCK *)((char *)rv + chunk_size);
#if DEBUG
    printf("    disconnected sbrk\n");
#endif
    /* build start-block-marker */
    rv->size = 1;
    rv = (BLOCK *)((char *)rv + 4);
    chunk_size -= 8;
  }
  rv->size = chunk_size - 8;
  ENDSZ(rv) = rv->size;
  AFTER(rv)->size = 1;
  CHECK(rv);

  RET(rv);
}

static inline BLOCK *
merge(BLOCK *a, BLOCK *b, BLOCK *c)
{
  int bu;
  BLOCK *bp, **bpp;

#if DEBUG
  printf("  merge %u/%08x + %u/%08x = %u\n",
	 a->size, a, b->size, b, a->size+b->size+8);
#endif

  CHECK(a);
  CHECK(b);
  CHECK(c);
  if (c == slop)
  {
#if DEBUG
    printf("  snipping slop %u/%08x\n", slop->size, slop);
#endif
    slop = 0;
  }
  bu = b2bucket(c);
#if DEBUG
  printf("bucket for %u/%08x is %d\n", c->size, c, bu);
#endif
  bpp = freelist+bu;
  for (bp=freelist[bu]; bp; bpp=&(bp->next), bp=bp->next)
  {
#if DEBUG
    printf("  %08x", bp);
#endif
    if (bp == c)
    {
#if DEBUG
      printf("\n  snipping %u/%08x from freelist[%d]\n", bp->size, bp, bu);
#endif
      *bpp = bp->next;
      break;
    }
  }
  CHECK(c);

  a->size += b->size + 8;
  a->bucket = -1;
  ENDSZ(a) = a->size;

  CHECK(a);
  return a;
}

void
free(void *ptr)
{
  int b;
  BLOCK *block;
  if (ptr == 0)
    return;
  block = (BLOCK *)((char *)ptr-4);

#if NUMSMALL
  if (block->size < SMALL)
  {
    block->next = smallblocks[block->size/ALIGN];
    smallblocks[block->size/ALIGN] = block;
    return;
  }
#endif

  block->size &= ~1;
  ENDSZ(block) &= ~1;
  block->bucket = -1;
#if DEBUG
  printf("free(%u/%08x)\n", block->size, block);
#endif

  CHECK(block);
  if (! (AFTER(block)->size & 1))
  {
    CHECK(AFTER(block));
  }
  if (! (BEFSZ(block) & 1))
  {
    CHECK(BEFORE(block));
    block = merge(BEFORE(block), block, BEFORE(block));
  }
  CHECK(block);
  if (! (AFTER(block)->size & 1))
  {
    CHECK(AFTER(block));
    block = merge(block, AFTER(block), AFTER(block));
  }
  CHECK(block);
  
  b = b2bucket(block);
  block->next = freelist[b];
  freelist[b] = block;
  CHECK(block);
}

void * realloc(void *ptr, size_t size)
{
 BLOCK *b;
 char *newptr;
 size_t copysize;
 if (ptr == 0) return malloc(size);
 b = (BLOCK *)((char *)ptr-4);
 copysize = b->size & ~1;
 if (size <= copysize)
 {
  return ptr;
  copysize = size;
 }
 newptr = (char *)malloc(size);
 if (!newptr) return NULL;
 memcpy(newptr, ptr, copysize);
 free(ptr);
 return newptr;
}
