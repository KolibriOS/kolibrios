/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* alloca.c -- allocate automatically reclaimed memory
   (Mostly) portable public-domain implementation -- D A Gwyn

   This implementation of the PWB library alloca function,
   which is used to allocate space off the run-time stack so
   that it is automatically reclaimed upon procedure exit,
   was inspired by discussions with J. Q. Johnson of Cornell.
   J.Otto Tennant <jot@cray.com> contributed the Cray support.

   There are some preprocessor constants that can
   be defined when compiling for your specific system, for
   improved efficiency; however, the defaults should be okay.

   The general concept of this implementation is to keep
   track of all alloca-allocated blocks, and reclaim any
   that are found to be deeper in the stack than the current
   invocation.  This heuristic does not reclaim storage as
   soon as it becomes invalid, but it will do so eventually.

   As a special case, alloca(0) reclaims storage without
   allocating any.  It is a good idea to use alloca(0) in
   your main control loop, etc. to force garbage collection.  */

#include <stdlib.h>

#ifdef alloca
#undef alloca
#endif

/* If your stack is a linked list of frames, you have to
   provide an "address metric" ADDRESS_FUNCTION macro.  */

#define ADDRESS_FUNCTION(arg) &(arg)

#define	NULL	0

/* Define STACK_DIRECTION if you know the direction of stack
   growth for your system; otherwise it will be automatically
   deduced at run-time.

   STACK_DIRECTION > 0 => grows toward higher addresses
   STACK_DIRECTION < 0 => grows toward lower addresses
   STACK_DIRECTION = 0 => direction of growth unknown  */

#define	STACK_DIRECTION	-1

#define	STACK_DIR	STACK_DIRECTION	/* Known at compile-time.  */

/* An "alloca header" is used to:
   (a) chain together all alloca'ed blocks;
   (b) keep track of stack depth.

   It is very important that sizeof(header) agree with malloc
   alignment chunk size.  The following default should work okay.  */

#ifndef	ALIGN_SIZE
#define	ALIGN_SIZE	sizeof(double)
#endif

typedef union hdr
{
  char align[ALIGN_SIZE];	/* To force sizeof(header).  */
  struct
  {
    union hdr *next;		/* For chaining headers.  */
    char *deep;			/* For stack depth measure.  */
  } h;
} header;

static header *last_alloca_header = NULL;	/* -> last alloca header.  */

/* Return a pointer to at least SIZE bytes of storage,
   which will be automatically reclaimed upon exit from
   the procedure that called alloca.  Originally, this space
   was supposed to be taken from the current stack frame of the
   caller, but that method cannot be made to work for some
   implementations of C, for example under Gould's UTX/32.  */

void *
alloca(size_t size)
{
  char probe;			/* Probes stack depth: */
  char *depth = &probe;
  
  /* Reclaim garbage, defined as all alloca storage that
     was allocated from deeper in the stack than currently. */
  {
    header *hp;			/* Traverses linked list.  */

    for (hp = last_alloca_header; hp != NULL;)
      if ((STACK_DIR > 0 && hp->h.deep > depth)
	  || (STACK_DIR < 0 && hp->h.deep < depth))
      {
	header *np = hp->h.next;

	free ((void *) hp);	/* Collect garbage.  */

	hp = np;		/* -> next header.  */
      }
      else
	break;			/* Rest are not deeper.  */

    last_alloca_header = hp;	/* -> last valid storage.  */
  }

  if (size == 0)
    return NULL;		/* No allocation required.  */

  /* Allocate combined header + user data storage.  */

  {
    void * newp = malloc (sizeof (header) + size);
    if (newp == 0)
      abort();
    /* Address of header.  */

    ((header *) newp)->h.next = last_alloca_header;
    ((header *) newp)->h.deep = depth;

    last_alloca_header = (header *) newp;

    /* User storage begins just after header.  */

    return (void *) ((char *) newp + sizeof (header));
  }
}
