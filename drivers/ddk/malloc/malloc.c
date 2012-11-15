/*
  This is a version (aka dlmalloc) of malloc/free/realloc written by
  Doug Lea and released to the public domain, as explained at
  http://creativecommons.org/licenses/publicdomain.  Send questions,
  comments, complaints, performance data, etc to dl@cs.oswego.edu

* Version 2.8.4 Wed May 27 09:56:23 2009  Doug Lea  (dl at gee)

   Note: There may be an updated version of this malloc obtainable at
           ftp://gee.cs.oswego.edu/pub/misc/malloc.c
         Check before installing!

* Quickstart

  This library is all in one file to simplify the most common usage:
  ftp it, compile it (-O3), and link it into another program. All of
  the compile-time options default to reasonable values for use on
  most platforms.  You might later want to step through various
  compile-time and dynamic tuning options.

  For convenience, an include file for code using this malloc is at:
     ftp://gee.cs.oswego.edu/pub/misc/malloc-2.8.4.h
  You don't really need this .h file unless you call functions not
  defined in your system include files.  The .h file contains only the
  excerpts from this file needed for using this malloc on ANSI C/C++
  systems, so long as you haven't changed compile-time options about
  naming and tuning parameters.  If you do, then you can create your
  own malloc.h that does include all settings by cutting at the point
  indicated below. Note that you may already by default be using a C
  library containing a malloc that is based on some version of this
  malloc (for example in linux). You might still want to use the one
  in this file to customize settings or to avoid overheads associated
  with library versions.

*/

#include <ddk.h>
#include <mutex.h>
#include <syscall.h>

struct malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               head;       /* Size and inuse bits. */
  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;
};

typedef struct malloc_chunk  mchunk;
typedef struct malloc_chunk* mchunkptr;
typedef struct malloc_chunk* sbinptr;  /* The type of bins of chunks */
typedef unsigned int bindex_t;         /* Described below */
typedef unsigned int binmap_t;         /* Described below */
typedef unsigned int flag_t;           /* The type of various bit flag sets */



/* ------------------- size_t and alignment properties -------------------- */

/* The maximum possible size_t value has all bits set */
#define MAX_SIZE_T           (~(size_t)0)

/* The byte and bit size of a size_t */
#define SIZE_T_SIZE             (sizeof(size_t))
#define SIZE_T_BITSIZE          (sizeof(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#define SIZE_T_ZERO             ((size_t)0)
#define SIZE_T_ONE              ((size_t)1)
#define SIZE_T_TWO              ((size_t)2)
#define SIZE_T_FOUR             ((size_t)4)
#define TWO_SIZE_T_SIZES        (SIZE_T_SIZE<<1)
#define FOUR_SIZE_T_SIZES       (SIZE_T_SIZE<<2)
#define SIX_SIZE_T_SIZES        (FOUR_SIZE_T_SIZES+TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T         (MAX_SIZE_T / 2U)

#define USE_LOCK_BIT            (2U)
#define USE_MMAP_BIT            (SIZE_T_ONE)
#define USE_NONCONTIGUOUS_BIT   (4U)

/* segment bit set in create_mspace_with_base */
#define EXTERN_BIT              (8U)

#define HAVE_MMAP               1
#define CALL_MMAP(s)            MMAP_DEFAULT(s)
#define CALL_MUNMAP(a, s)       MUNMAP_DEFAULT((a), (s))
#define CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#define MAX_RELEASE_CHECK_RATE  4095
#define NO_SEGMENT_TRAVERSAL    1
#define MALLOC_ALIGNMENT        ((size_t)8U)
#define CHUNK_OVERHEAD          (SIZE_T_SIZE)
#define DEFAULT_GRANULARITY     ((size_t)128U * (size_t)1024U)
#define DEFAULT_MMAP_THRESHOLD  ((size_t)512U * (size_t)1024U)
#define DEFAULT_TRIM_THRESHOLD  ((size_t)1024U * (size_t)1024U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)

/* True if address a has acceptable alignment */
#define is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

/* the number of bytes to offset an address to align it */
#define align_offset(A)\
 ((((size_t)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))


#define MFAIL                ((void*)(MAX_SIZE_T))
#define CMFAIL               ((char*)(MFAIL)) /* defined for convenience */

/* For sys_alloc, enough padding to ensure can malloc request on success */
#define SYS_ALLOC_PADDING (TOP_FOOT_SIZE + MALLOC_ALIGNMENT)

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#define TOP_FOOT_SIZE\
  (align_offset(chunk2mem(0))+pad_request(sizeof(struct malloc_segment))+MIN_CHUNK_SIZE)

/* ------------------- Chunks sizes and alignments ----------------------- */

#define MCHUNK_SIZE         (sizeof(mchunk))

/* MMapped chunks need a second word of overhead ... */
#define MMAP_CHUNK_OVERHEAD (TWO_SIZE_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#define MMAP_FOOT_PAD       (FOUR_SIZE_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#define MIN_CHUNK_SIZE\
  ((MCHUNK_SIZE + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* conversion from malloc headers to user pointers, and back */
#define chunk2mem(p)        ((void*)((char*)(p)       + TWO_SIZE_T_SIZES))
#define mem2chunk(mem)      ((mchunkptr)((char*)(mem) - TWO_SIZE_T_SIZES))
/* chunk associated with aligned address A */
#define align_as_chunk(A)   (mchunkptr)((A) + align_offset(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#define MAX_REQUEST         ((-MIN_CHUNK_SIZE) << 2)
#define MIN_REQUEST         (MIN_CHUNK_SIZE - CHUNK_OVERHEAD - SIZE_T_ONE)

/* pad request bytes into a usable size */
#define pad_request(req) \
   (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* pad request, checking for minimum (but not maximum) */
#define request2size(req) \
  (((req) < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(req))

/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#define PINUSE_BIT          (SIZE_T_ONE)
#define CINUSE_BIT          (SIZE_T_TWO)
#define FLAG4_BIT           (SIZE_T_FOUR)
#define INUSE_BITS          (PINUSE_BIT|CINUSE_BIT)
#define FLAG_BITS           (PINUSE_BIT|CINUSE_BIT|FLAG4_BIT)

/* Head value for fenceposts */
#define FENCEPOST_HEAD      (INUSE_BITS|SIZE_T_SIZE)

/* extraction of fields from head words */
#define cinuse(p)           ((p)->head & CINUSE_BIT)
#define pinuse(p)           ((p)->head & PINUSE_BIT)
#define is_inuse(p)         (((p)->head & INUSE_BITS) != PINUSE_BIT)
#define is_mmapped(p)       (((p)->head & INUSE_BITS) == 0)

#define chunksize(p)        ((p)->head & ~(FLAG_BITS))

#define clear_pinuse(p)     ((p)->head &= ~PINUSE_BIT)

/* Treat space at ptr +/- offset as a chunk */
#define chunk_plus_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))
#define chunk_minus_offset(p, s) ((mchunkptr)(((char*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->head & ~FLAG_BITS)))
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_foot) ))

/* extract next chunk's pinuse bit */
#define next_pinuse(p)  ((next_chunk(p)->head) & PINUSE_BIT)

/* Set size, pinuse bit, and foot */
#define set_size_and_pinuse_of_free_chunk(p, s)\
  ((p)->head = (s|PINUSE_BIT), set_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#define set_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#define overhead_for(p)\
 (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)


struct malloc_tree_chunk {
  /* The first four fields must be compatible with malloc_chunk */
  size_t                    prev_foot;
  size_t                    head;
  struct malloc_tree_chunk* fd;
  struct malloc_tree_chunk* bk;

  struct malloc_tree_chunk* child[2];
  struct malloc_tree_chunk* parent;
  bindex_t                  index;
};

typedef struct malloc_tree_chunk  tchunk;
typedef struct malloc_tree_chunk* tchunkptr;
typedef struct malloc_tree_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#define leftmost_child(t) ((t)->child[0] != 0? (t)->child[0] : (t)->child[1])


struct malloc_segment {
  char*        base;             /* base address */
  size_t       size;             /* allocated size */
  struct malloc_segment* next;   /* ptr to next segment */
  flag_t       sflags;           /* mmap and extern flag */
};

#define is_mmapped_segment(S)  ((S)->sflags & USE_MMAP_BIT)
#define is_extern_segment(S)   ((S)->sflags & EXTERN_BIT)

typedef struct malloc_segment  msegment;
typedef struct malloc_segment* msegmentptr;

/* ---------------------------- malloc_state ----------------------------- */

/*
   A malloc_state holds all of the bookkeeping for a space.
   The main fields are:

  Top
    The topmost chunk of the currently active segment. Its size is
    cached in topsize.  The actual size of topmost space is
    topsize+TOP_FOOT_SIZE, which includes space reserved for adding
    fenceposts and segment records if necessary when getting more
    space from the system.  The size at which to autotrim top is
    cached from mparams in trim_check, except that it is disabled if
    an autotrim fails.

  Designated victim (dv)
    This is the preferred chunk for servicing small requests that
    don't have exact fits.  It is normally the chunk split off most
    recently to service another small request.  Its size is cached in
    dvsize. The link fields of this chunk are not maintained since it
    is not kept in a bin.

  SmallBins
    An array of bin headers for free chunks.  These bins hold chunks
    with sizes less than MIN_LARGE_SIZE bytes. Each bin contains
    chunks of all the same size, spaced 8 bytes apart.  To simplify
    use in double-linked lists, each bin header acts as a malloc_chunk
    pointing to the real first node, if it exists (else pointing to
    itself).  This avoids special-casing for headers.  But to avoid
    waste, we allocate only the fd/bk pointers of bins, and then use
    repositioning tricks to treat these as the fields of a chunk.

  TreeBins
    Treebins are pointers to the roots of trees holding a range of
    sizes. There are 2 equally spaced treebins for each power of two
    from TREE_SHIFT to TREE_SHIFT+16. The last bin holds anything
    larger.

  Bin maps
    There is one bit map for small bins ("smallmap") and one for
    treebins ("treemap).  Each bin sets its bit when non-empty, and
    clears the bit when empty.  Bit operations are then used to avoid
    bin-by-bin searching -- nearly all "search" is done without ever
    looking at bins that won't be selected.  The bit maps
    conservatively use 32 bits per map word, even if on 64bit system.
    For a good description of some of the bit-based techniques used
    here, see Henry S. Warren Jr's book "Hacker's Delight" (and
    supplement at http://hackersdelight.org/). Many of these are
    intended to reduce the branchiness of paths through malloc etc, as
    well as to reduce the number of memory locations read or written.

  Segments
    A list of segments headed by an embedded malloc_segment record
    representing the initial space.

  Address check support
    The least_addr field is the least address ever obtained from
    MORECORE or MMAP. Attempted frees and reallocs of any address less
    than this are trapped (unless INSECURE is defined).

  Magic tag
    A cross-check field that should always hold same value as mparams.magic.

  Flags
    Bits recording whether to use MMAP, locks, or contiguous MORECORE

  Statistics
    Each space keeps track of current and maximum system memory
    obtained via MORECORE or MMAP.

  Trim support
    Fields holding the amount of unused topmost memory that should trigger
    timming, and a counter to force periodic scanning to release unused
    non-topmost segments.

  Locking
    If USE_LOCKS is defined, the "mutex" lock is acquired and released
    around every public call using this mspace.

  Extension support
    A void* pointer and a size_t field that can be used to help implement
    extensions to this malloc.
*/

/* Bin types, widths and sizes */
#define NSMALLBINS        (32U)
#define NTREEBINS         (32U)
#define SMALLBIN_SHIFT    (3U)
#define SMALLBIN_WIDTH    (SIZE_T_ONE << SMALLBIN_SHIFT)
#define TREEBIN_SHIFT     (8U)
#define MIN_LARGE_SIZE    (SIZE_T_ONE << TREEBIN_SHIFT)
#define MAX_SMALL_SIZE    (MIN_LARGE_SIZE - SIZE_T_ONE)
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD)

struct malloc_state {
  binmap_t   smallmap;
  binmap_t   treemap;
  size_t     dvsize;
  size_t     topsize;
  char*      least_addr;
  mchunkptr  dv;
  mchunkptr  top;
  size_t     trim_check;
  size_t     release_checks;
  size_t     magic;
  mchunkptr  smallbins[(NSMALLBINS+1)*2];
  tbinptr    treebins[NTREEBINS];
  size_t     footprint;
  size_t     max_footprint;
  flag_t     mflags;
  struct mutex  lock;     /* locate lock among fields that rarely change */
  msegment   seg;
  void*      extp;      /* Unused but available for extensions */
  size_t     exts;
};

typedef struct malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

struct malloc_params
{
    volatile size_t  magic;
    size_t  page_size;
    size_t  granularity;
    size_t  mmap_threshold;
    size_t  trim_threshold;
    flag_t  default_mflags;
};

static struct malloc_params mparams;

#define ensure_initialization() (void)(mparams.magic != 0 || init_mparams())

static struct malloc_state _gm_;
#define gm                 (&_gm_)
#define is_global(M)       ((M) == &_gm_)

#define is_initialized(M)  ((M)->top != 0)


//struct mutex malloc_global_mutex;

static DEFINE_MUTEX(malloc_global_mutex);

#define ACQUIRE_MALLOC_GLOBAL_LOCK()  MutexLock(&malloc_global_mutex);
#define RELEASE_MALLOC_GLOBAL_LOCK()  MutexUnlock(&malloc_global_mutex);

#define PREACTION(M)  ( MutexLock(&(M)->lock))
#define POSTACTION(M) { MutexUnlock(&(M)->lock); }


/* ---------------------------- Indexing Bins ---------------------------- */

#define is_small(s)         (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)      ((s)  >> SMALLBIN_SHIFT)
#define small_index2size(i) ((i)  << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX     (small_index(MIN_CHUNK_SIZE))

/* addressing by index. See above about smallbin repositioning */
#define smallbin_at(M, i)   ((sbinptr)((char*)&((M)->smallbins[(i)<<1])))
#define treebin_at(M,i)     (&((M)->treebins[i]))


#define compute_tree_index(S, I)\
{\
  unsigned int X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int K;\
    __asm__("bsrl\t%1, %0\n\t" : "=r" (K) : "g"  (X));\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

/* Bit representing maximum resolved size in a treebin at i */
#define bit_for_tree_index(i) \
   (i == NTREEBINS-1)? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#define leftshift_for_tree_index(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_tree_index(i) \
   ((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
   (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#define idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#define clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#define smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#define mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#define clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#define treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#define least_bit(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#define left_bits(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#define same_or_left_bits(x) ((x) | -(x))


/* index corresponding to given bit. Use x86 asm if possible */

#define compute_bit2idx(X, I)\
{\
  unsigned int J;\
  __asm__("bsfl\t%1, %0\n\t" : "=r" (J) : "g" (X));\
  I = (bindex_t)J;\
}


#define mark_inuse_foot(M,p,s)

/* Get/set size at footer */
#define get_foot(p, s)  (((mchunkptr)((char*)(p) + (s)))->prev_foot)
#define set_foot(p, s)  (((mchunkptr)((char*)(p) + (s)))->prev_foot = (s))

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#define set_inuse(M,p,s)\
  ((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
  ((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#define set_inuse_and_pinuse(M,p,s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  ((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set size, cinuse and pinuse bit of this chunk */
#define set_size_and_pinuse_of_inuse_chunk(M, p, s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT))


#define assert(x)
#define RTCHECK(e)  __builtin_expect(e, 1)

#define check_free_chunk(M,P)
#define check_inuse_chunk(M,P)
#define check_malloced_chunk(M,P,N)
#define check_mmapped_chunk(M,P)
#define check_malloc_state(M)
#define check_top_chunk(M,P)

/* Check if address a is at least as high as any from MORECORE or MMAP */
#define ok_address(M, a) ((char*)(a) >= (M)->least_addr)
/* Check if address of next chunk n is higher than base chunk p */
#define ok_next(p, n)    ((char*)(p) < (char*)(n))
/* Check if p has inuse status */
#define ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#define ok_pinuse(p)     pinuse(p)

#define CORRUPTION_ERROR_ACTION(m)                             \
        do {                                                   \
            printf("%s malloc heap corrupted\n",__FUNCTION__); \
            while(1)                                           \
            {                                                  \
                delay(100);                                    \
            }                                                  \
        }while(0)                                              \


#define USAGE_ERROR_ACTION(m, p)                               \
        do {                                                   \
            printf("%s malloc heap corrupted\n",__FUNCTION__); \
            while(1)                                           \
            {                                                  \
                delay(100);                                    \
            }                                                  \
        }while(0)                                              \

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#define insert_small_chunk(M, P, S) {\
  bindex_t I  = small_index(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  assert(S >= MIN_CHUNK_SIZE);\
  if (!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  else if (RTCHECK(ok_address(M, B->fd)))\
    F = B->fd;\
  else {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#define unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  bindex_t I = small_index(S);\
  assert(P != B);\
  assert(P != F);\
  assert(chunksize(P) == small_index2size(I));\
  if (F == B)\
    clear_smallmap(M, I);\
  else if (RTCHECK((F == smallbin_at(M,I) || ok_address(M, F)) &&\
                   (B == smallbin_at(M,I) || ok_address(M, B)))) {\
    F->bk = B;\
    B->fd = F;\
  }\
  else {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Unlink the first chunk from a smallbin */
#define unlink_first_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  assert(P != B);\
  assert(P != F);\
  assert(chunksize(P) == small_index2size(I));\
  if (B == F)\
    clear_smallmap(M, I);\
  else if (RTCHECK(ok_address(M, F))) {\
    B->fd = F;\
    F->bk = B;\
  }\
  else {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#define replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  if (DVS != 0) {\
    mchunkptr DV = M->dv;\
    assert(is_small(DVS));\
    insert_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}


/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#define insert_large_chunk(M, X, S) {\
  tbinptr* H;\
  bindex_t I;\
  compute_tree_index(S, I);\
  H = treebin_at(M, I);\
  X->index = I;\
  X->child[0] = X->child[1] = 0;\
  if (!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->parent = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  else {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_tree_index(I);\
    for (;;) {\
      if (chunksize(T) != S) {\
        tchunkptr* C = &(T->child[(K >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1]);\
        K <<= 1;\
        if (*C != 0)\
          T = *C;\
        else if (RTCHECK(ok_address(M, C))) {\
          *C = X;\
          X->parent = T;\
          X->fd = X->bk = X;\
          break;\
        }\
        else {\
          CORRUPTION_ERROR_ACTION(M);\
          break;\
        }\
      }\
      else {\
        tchunkptr F = T->fd;\
        if (RTCHECK(ok_address(M, T) && ok_address(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->parent = 0;\
          break;\
        }\
        else {\
          CORRUPTION_ERROR_ACTION(M);\
          break;\
        }\
      }\
    }\
  }\
}

/*
  Unlink steps:

  1. If x is a chained node, unlink it from its same-sized fd/bk links
     and choose its bk node as its replacement.
  2. If x was the last node of its size, but not a leaf node, it must
     be replaced with a leaf node (not merely one with an open left or
     right), to make sure that lefts and rights of descendents
     correspond properly to bit masks.  We use the rightmost descendent
     of x.  We could use any other leaf, but this is easy to locate and
     tends to counteract removal of leftmosts elsewhere, and so keeps
     paths shorter than minimally guaranteed.  This doesn't loop much
     because on average a node in a tree is near the bottom.
  3. If x is the base of a chain (i.e., has parent links) relink
     x's parent and children to x's replacement (or null if none).
*/

#define unlink_large_chunk(M, X) {\
  tchunkptr XP = X->parent;\
  tchunkptr R;\
  if (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    if (RTCHECK(ok_address(M, F))) {\
      F->bk = R;\
      R->fd = F;\
    }\
    else {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  else {\
    tchunkptr* RP;\
    if (((R = *(RP = &(X->child[1]))) != 0) ||\
        ((R = *(RP = &(X->child[0]))) != 0)) {\
      tchunkptr* CP;\
      while ((*(CP = &(R->child[1])) != 0) ||\
             (*(CP = &(R->child[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      if (RTCHECK(ok_address(M, RP)))\
        *RP = 0;\
      else {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  if (XP != 0) {\
    tbinptr* H = treebin_at(M, X->index);\
    if (X == *H) {\
      if ((*H = R) == 0) \
        clear_treemap(M, X->index);\
    }\
    else if (RTCHECK(ok_address(M, XP))) {\
      if (XP->child[0] == X) \
        XP->child[0] = R;\
      else \
        XP->child[1] = R;\
    }\
    else\
      CORRUPTION_ERROR_ACTION(M);\
    if (R != 0) {\
      if (RTCHECK(ok_address(M, R))) {\
        tchunkptr C0, C1;\
        R->parent = XP;\
        if ((C0 = X->child[0]) != 0) {\
          if (RTCHECK(ok_address(M, C0))) {\
            R->child[0] = C0;\
            C0->parent = R;\
          }\
          else\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        if ((C1 = X->child[1]) != 0) {\
          if (RTCHECK(ok_address(M, C1))) {\
            R->child[1] = C1;\
            C1->parent = R;\
          }\
          else\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      else\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}

/* Relays to large vs small bin operations */

#define insert_chunk(M, P, S)\
  if (is_small(S)) insert_small_chunk(M, P, S)\
  else { tchunkptr TP = (tchunkptr)(P); insert_large_chunk(M, TP, S); }

#define unlink_chunk(M, P, S)\
  if (is_small(S)) unlink_small_chunk(M, P, S)\
  else { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#define use_lock(M)           ((M)->mflags &   USE_LOCK_BIT)
#define enable_lock(M)        ((M)->mflags |=  USE_LOCK_BIT)
#define disable_lock(M)       ((M)->mflags &= ~USE_LOCK_BIT)

#define use_mmap(M)           ((M)->mflags &   USE_MMAP_BIT)
#define enable_mmap(M)        ((M)->mflags |=  USE_MMAP_BIT)
#define disable_mmap(M)       ((M)->mflags &= ~USE_MMAP_BIT)

#define use_noncontiguous(M)  ((M)->mflags &   USE_NONCONTIGUOUS_BIT)
#define disable_contiguous(M) ((M)->mflags |=  USE_NONCONTIGUOUS_BIT)

#define set_lock(M,L)\
 ((M)->mflags = (L)?\
  ((M)->mflags | USE_LOCK_BIT) :\
  ((M)->mflags & ~USE_LOCK_BIT))

/* page-align a size */
#define page_align(S)\
 (((S) + (mparams.page_size - SIZE_T_ONE)) & ~(mparams.page_size - SIZE_T_ONE))

/* granularity-align a size */
#define granularity_align(S)\
  (((S) + (mparams.granularity - SIZE_T_ONE))\
   & ~(mparams.granularity - SIZE_T_ONE))


/* For mmap, use granularity alignment  */
#define mmap_align(S) granularity_align(S)

/* For sys_alloc, enough padding to ensure can malloc request on success */
#define SYS_ALLOC_PADDING (TOP_FOOT_SIZE + MALLOC_ALIGNMENT)

#define is_page_aligned(S)\
   (((size_t)(S) & (mparams.page_size - SIZE_T_ONE)) == 0)
#define is_granularity_aligned(S)\
   (((size_t)(S) & (mparams.granularity - SIZE_T_ONE)) == 0)

/*  True if segment S holds address A */
#define segment_holds(S, A)\
  ((char*)(A) >= S->base && (char*)(A) < S->base + S->size)

/* Return segment holding given address */
static msegmentptr segment_holding(mstate m, char* addr)
{
    msegmentptr sp = &m->seg;
    for (;;) {
        if (addr >= sp->base && addr < sp->base + sp->size)
            return sp;
        if ((sp = sp->next) == 0)
            return 0;
    }
}

/* Return true if segment contains a segment link */
static int has_segment_link(mstate m, msegmentptr ss)
{
    msegmentptr sp = &m->seg;
    for (;;) {
        if ((char*)sp >= ss->base && (char*)sp < ss->base + ss->size)
            return 1;
        if ((sp = sp->next) == 0)
            return 0;
    }
}

static inline void* os_mmap(size_t size)
{
  void* ptr = KernelAlloc(size);
  printf("%s %x %d bytes\n",__FUNCTION__, ptr, size);
  return (ptr != 0)? ptr: MFAIL;
}

static inline int os_munmap(void* ptr, size_t size)
{
    return (KernelFree(ptr) != 0) ? 0 : -1;
}

#define should_trim(M,s)  ((s) > (M)->trim_check)


#define MMAP_DEFAULT(s)        os_mmap(s)
#define MUNMAP_DEFAULT(a, s)   os_munmap((a), (s))
#define DIRECT_MMAP_DEFAULT(s) os_mmap(s)

#define internal_malloc(m, b) malloc(b)
#define internal_free(m, mem) free(mem)

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
static void* mmap_alloc(mstate m, size_t nb)
{
    size_t mmsize = mmap_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
    if (mmsize > nb)       /* Check for wrap around 0 */
    {
        char* mm = (char*)(os_mmap(mmsize));
        if (mm != CMFAIL)
        {
            size_t offset = align_offset(chunk2mem(mm));
            size_t psize = mmsize - offset - MMAP_FOOT_PAD;
            mchunkptr p = (mchunkptr)(mm + offset);
            p->prev_foot = offset;
            p->head = psize;
            mark_inuse_foot(m, p, psize);
            chunk_plus_offset(p, psize)->head = FENCEPOST_HEAD;
            chunk_plus_offset(p, psize+SIZE_T_SIZE)->head = 0;

            if (m->least_addr == 0 || mm < m->least_addr)
                m->least_addr = mm;
            if ((m->footprint += mmsize) > m->max_footprint)
                m->max_footprint = m->footprint;
            assert(is_aligned(chunk2mem(p)));
            check_mmapped_chunk(m, p);
            return chunk2mem(p);
        }
    }
    return 0;
}

/* Realloc using mmap */
static mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb)
{
    size_t oldsize = chunksize(oldp);
    if (is_small(nb)) /* Can't shrink mmap regions below small size */
        return 0;
  /* Keep old chunk if big enough but not too big */
    if (oldsize >= nb + SIZE_T_SIZE &&
      (oldsize - nb) <= (mparams.granularity << 1))
        return oldp;
    else
    {
        size_t offset = oldp->prev_foot;
        size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
        size_t newmmsize = mmap_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
        char* cp = (char*)CALL_MREMAP((char*)oldp - offset,
                                  oldmmsize, newmmsize, 1);
        if (cp != CMFAIL)
        {
            mchunkptr newp = (mchunkptr)(cp + offset);
            size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
            newp->head = psize;
            mark_inuse_foot(m, newp, psize);
            chunk_plus_offset(newp, psize)->head = FENCEPOST_HEAD;
            chunk_plus_offset(newp, psize+SIZE_T_SIZE)->head = 0;

            if (cp < m->least_addr)
                m->least_addr = cp;
            if ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
                m->max_footprint = m->footprint;
            check_mmapped_chunk(m, newp);
            return newp;
        }
    }
    return 0;
}

/* ---------------------------- setting mparams -------------------------- */

/* Initialize mparams */
static int init_mparams(void) {

    ACQUIRE_MALLOC_GLOBAL_LOCK();

    if (mparams.magic == 0)
    {
        size_t magic;
        size_t psize;
        size_t gsize;

        psize = 4096;
        gsize = DEFAULT_GRANULARITY;

    /* Sanity-check configuration:
       size_t must be unsigned and as wide as pointer type.
       ints must be at least 4 bytes.
       alignment must be at least 8.
       Alignment, min chunk size, and page size must all be powers of 2.
    */

        mparams.granularity = gsize;
        mparams.page_size = psize;
        mparams.mmap_threshold = DEFAULT_MMAP_THRESHOLD;
        mparams.trim_threshold = DEFAULT_TRIM_THRESHOLD;
        mparams.default_mflags = USE_LOCK_BIT|USE_MMAP_BIT|USE_NONCONTIGUOUS_BIT;

    /* Set up lock for main malloc area */
        gm->mflags = mparams.default_mflags;
        MutexInit(&gm->lock);

        magic = (size_t)(GetTimerTicks() ^ (size_t)0x55555555U);
        magic |= (size_t)8U;    /* ensure nonzero */
        magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
        mparams.magic = magic;
    }

    RELEASE_MALLOC_GLOBAL_LOCK();
    return 1;
}

/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
static void init_top(mstate m, mchunkptr p, size_t psize)
{
  /* Ensure alignment */
    size_t offset = align_offset(chunk2mem(p));
    p = (mchunkptr)((char*)p + offset);
    psize -= offset;

    m->top = p;
    m->topsize = psize;
    p->head = psize | PINUSE_BIT;
  /* set size of fake trailing chunk holding overhead space only once */
    chunk_plus_offset(p, psize)->head = TOP_FOOT_SIZE;
    m->trim_check = mparams.trim_threshold; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
static void init_bins(mstate m)
{
  /* Establish circular links for smallbins */
    bindex_t i;
    for (i = 0; i < NSMALLBINS; ++i) {
        sbinptr bin = smallbin_at(m,i);
        bin->fd = bin->bk = bin;
    }
}

/* Allocate chunk and prepend remainder with chunk in successor base. */
static void* prepend_alloc(mstate m, char* newbase, char* oldbase,
                           size_t nb)
{
    mchunkptr p = align_as_chunk(newbase);
    mchunkptr oldfirst = align_as_chunk(oldbase);
    size_t psize = (char*)oldfirst - (char*)p;
    mchunkptr q = chunk_plus_offset(p, nb);
    size_t qsize = psize - nb;
    set_size_and_pinuse_of_inuse_chunk(m, p, nb);

    assert((char*)oldfirst > (char*)q);
    assert(pinuse(oldfirst));
    assert(qsize >= MIN_CHUNK_SIZE);

  /* consolidate remainder with first chunk of old base */
    if (oldfirst == m->top) {
        size_t tsize = m->topsize += qsize;
        m->top = q;
        q->head = tsize | PINUSE_BIT;
        check_top_chunk(m, q);
    }
    else if (oldfirst == m->dv) {
        size_t dsize = m->dvsize += qsize;
        m->dv = q;
        set_size_and_pinuse_of_free_chunk(q, dsize);
    }
    else {
        if (!is_inuse(oldfirst)) {
            size_t nsize = chunksize(oldfirst);
            unlink_chunk(m, oldfirst, nsize);
            oldfirst = chunk_plus_offset(oldfirst, nsize);
            qsize += nsize;
        }
        set_free_with_pinuse(q, qsize, oldfirst);
        insert_chunk(m, q, qsize);
        check_free_chunk(m, q);
    }

    check_malloced_chunk(m, chunk2mem(p), nb);
    return chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
static void add_segment(mstate m, char* tbase, size_t tsize, flag_t mmapped)
{
  /* Determine locations and sizes of segment, fenceposts, old top */
    char* old_top = (char*)m->top;
    msegmentptr oldsp = segment_holding(m, old_top);
    char* old_end = oldsp->base + oldsp->size;
    size_t ssize = pad_request(sizeof(struct malloc_segment));
    char* rawsp = old_end - (ssize + FOUR_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
    size_t offset = align_offset(chunk2mem(rawsp));
    char* asp = rawsp + offset;
    char* csp = (asp < (old_top + MIN_CHUNK_SIZE))? old_top : asp;
    mchunkptr sp = (mchunkptr)csp;
    msegmentptr ss = (msegmentptr)(chunk2mem(sp));
    mchunkptr tnext = chunk_plus_offset(sp, ssize);
    mchunkptr p = tnext;
    int nfences = 0;

  /* reset top to new space */
    init_top(m, (mchunkptr)tbase, tsize - TOP_FOOT_SIZE);

  /* Set up segment record */
    assert(is_aligned(ss));
    set_size_and_pinuse_of_inuse_chunk(m, sp, ssize);
    *ss = m->seg; /* Push current record */
    m->seg.base = tbase;
    m->seg.size = tsize;
    m->seg.sflags = mmapped;
    m->seg.next = ss;

  /* Insert trailing fenceposts */
    for (;;) {
        mchunkptr nextp = chunk_plus_offset(p, SIZE_T_SIZE);
        p->head = FENCEPOST_HEAD;
        ++nfences;
        if ((char*)(&(nextp->head)) < old_end)
            p = nextp;
        else
            break;
    }
    assert(nfences >= 2);

  /* Insert the rest of old top into a bin as an ordinary free chunk */
    if (csp != old_top) {
        mchunkptr q = (mchunkptr)old_top;
        size_t psize = csp - old_top;
        mchunkptr tn = chunk_plus_offset(q, psize);
        set_free_with_pinuse(q, psize, tn);
        insert_chunk(m, q, psize);
    }

  check_top_chunk(m, m->top);
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
static void* sys_alloc(mstate m, size_t nb)
{
    char* tbase = CMFAIL;
    size_t tsize = 0;
    flag_t mmap_flag = 0;

    ensure_initialization();

    printf("%s %d bytes\n", __FUNCTION__, nb);

  /* Directly map large chunks, but only if already initialized */
    if (use_mmap(m) && nb >= mparams.mmap_threshold && m->topsize != 0)
    {
        void* mem = mmap_alloc(m, nb);
        if (mem != 0)
            return mem;
    }

  /*
    Try getting memory in any of three ways (in most-preferred to
    least-preferred order):
    1. A call to MORECORE that can normally contiguously extend memory.
       (disabled if not MORECORE_CONTIGUOUS or not HAVE_MORECORE or
       or main space is mmapped or a previous contiguous call failed)
    2. A call to MMAP new space (disabled if not HAVE_MMAP).
       Note that under the default settings, if MORECORE is unable to
       fulfill a request, and HAVE_MMAP is true, then mmap is
       used as a noncontiguous system allocator. This is a useful backup
       strategy for systems with holes in address spaces -- in this case
       sbrk cannot contiguously expand the heap, but mmap may be able to
       find space.
    3. A call to MORECORE that cannot usually contiguously extend memory.
       (disabled if not HAVE_MORECORE)

   In all cases, we need to request enough bytes from system to ensure
   we can malloc nb bytes upon success, so pad with enough space for
   top_foot, plus alignment-pad to make sure we don't lose bytes if
   not on boundary, and round this up to a granularity unit.
  */

    if (HAVE_MMAP && tbase == CMFAIL)   /* Try MMAP */
    {
        size_t rsize = granularity_align(nb + SYS_ALLOC_PADDING);
        if (rsize > nb)  /* Fail if wraps around zero */
        {
            char* mp = (char*)(CALL_MMAP(rsize));
            if (mp != CMFAIL)
            {
                tbase = mp;
                tsize = rsize;
                mmap_flag = USE_MMAP_BIT;
            }
        }
    }


    if (tbase != CMFAIL)
    {

        if ((m->footprint += tsize) > m->max_footprint)
            m->max_footprint = m->footprint;

        if (!is_initialized(m))  /* first-time initialization */
        {
            if (m->least_addr == 0 || tbase < m->least_addr)
                m->least_addr = tbase;
            m->seg.base = tbase;
             m->seg.size = tsize;
            m->seg.sflags = mmap_flag;
            m->magic = mparams.magic;
            m->release_checks = MAX_RELEASE_CHECK_RATE;
            init_bins(m);

            if (is_global(m))
                init_top(m, (mchunkptr)tbase, tsize - TOP_FOOT_SIZE);
            else
            {
        /* Offset top by embedded malloc_state */
                mchunkptr mn = next_chunk(mem2chunk(m));
                init_top(m, mn, (size_t)((tbase + tsize) - (char*)mn) -TOP_FOOT_SIZE);
            }
        }
        else
        {
      /* Try to merge with an existing segment */
            msegmentptr sp = &m->seg;
      /* Only consider most recent segment if traversal suppressed */
            while (sp != 0 && tbase != sp->base + sp->size)
                sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->next;
            if (sp != 0 && !is_extern_segment(sp) &&
               (sp->sflags & USE_MMAP_BIT) == mmap_flag &&
                segment_holds(sp, m->top))  /* append */
            {
                sp->size += tsize;
                init_top(m, m->top, m->topsize + tsize);
            }
            else
            {
                if (tbase < m->least_addr)
                    m->least_addr = tbase;
                sp = &m->seg;
                while (sp != 0 && sp->base != tbase + tsize)
                    sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->next;
                if (sp != 0 &&
                    !is_extern_segment(sp) &&
                    (sp->sflags & USE_MMAP_BIT) == mmap_flag)
                {
                    char* oldbase = sp->base;
                    sp->base = tbase;
                    sp->size += tsize;
                    return prepend_alloc(m, tbase, oldbase, nb);
                }
                else
                    add_segment(m, tbase, tsize, mmap_flag);
            }
        }

        if (nb < m->topsize)  /* Allocate from new or extended top space */
        {
            size_t rsize = m->topsize -= nb;
            mchunkptr p = m->top;
            mchunkptr r = m->top = chunk_plus_offset(p, nb);
            r->head = rsize | PINUSE_BIT;
            set_size_and_pinuse_of_inuse_chunk(m, p, nb);
            check_top_chunk(m, m->top);
            check_malloced_chunk(m, chunk2mem(p), nb);
            return chunk2mem(p);
        }
    }

//    MALLOC_FAILURE_ACTION;
    return 0;
}


/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
static size_t release_unused_segments(mstate m)
{
    size_t released = 0;
    int nsegs = 0;
    msegmentptr pred = &m->seg;
    msegmentptr sp = pred->next;
    while (sp != 0)
    {
        char* base = sp->base;
        size_t size = sp->size;
        msegmentptr next = sp->next;
        ++nsegs;
        if (is_mmapped_segment(sp) && !is_extern_segment(sp))
        {
            mchunkptr p = align_as_chunk(base);
            size_t psize = chunksize(p);
      /* Can unmap if first chunk holds entire segment and not pinned */
            if (!is_inuse(p) && (char*)p + psize >= base + size - TOP_FOOT_SIZE)
            {
                tchunkptr tp = (tchunkptr)p;
                assert(segment_holds(sp, (char*)sp));
                if (p == m->dv) {
                    m->dv = 0;
                    m->dvsize = 0;
                }
                else {
                    unlink_large_chunk(m, tp);
                }
                if (CALL_MUNMAP(base, size) == 0)
                {
                    released += size;
                    m->footprint -= size;
          /* unlink obsoleted record */
                    sp = pred;
                    sp->next = next;
                }
                else { /* back out if cannot unmap */
                    insert_large_chunk(m, tp, psize);
                }
            }
        }
        if (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
            break;
        pred = sp;
        sp = next;
    }
  /* Reset check counter */
    m->release_checks = ((nsegs > MAX_RELEASE_CHECK_RATE)?
                       nsegs : MAX_RELEASE_CHECK_RATE);
    return released;
}

static int sys_trim(mstate m, size_t pad)
{
    size_t released = 0;
    ensure_initialization();
    if (pad < MAX_REQUEST && is_initialized(m))
    {
        pad += TOP_FOOT_SIZE; /* ensure enough room for segment overhead */

        if (m->topsize > pad)
        {
      /* Shrink top space in granularity-size units, keeping at least one */
            size_t unit = mparams.granularity;
            size_t extra = ((m->topsize - pad + (unit - SIZE_T_ONE)) / unit -
                             SIZE_T_ONE) * unit;
            msegmentptr sp = segment_holding(m, (char*)m->top);

            if (!is_extern_segment(sp))
            {
                if (is_mmapped_segment(sp))
                {
                    if (HAVE_MMAP &&
                        sp->size >= extra &&
                        !has_segment_link(m, sp))  /* can't shrink if pinned */
                    {
                        size_t newsize = sp->size - extra;
            /* Prefer mremap, fall back to munmap */
                        if ((CALL_MREMAP(sp->base, sp->size, newsize, 0) != MFAIL) ||
                            (CALL_MUNMAP(sp->base + newsize, extra) == 0))
                        {
                            released = extra;
                        }
                    }
                }
            }

            if (released != 0)
            {
                sp->size -= released;
                m->footprint -= released;
                init_top(m, m->top, m->topsize - released);
                check_top_chunk(m, m->top);
            }
        }

    /* Unmap any unused mmapped segments */
        if (HAVE_MMAP)
            released += release_unused_segments(m);

    /* On failure, disable autotrim to avoid repeated failed future calls */
        if (released == 0 && m->topsize > m->trim_check)
        m->trim_check = MAX_SIZE_T;
    }

    return (released != 0)? 1 : 0;
}



/* ---------------------------- malloc support --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
static void* tmalloc_large(mstate m, size_t nb) {
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  bindex_t idx;
  compute_tree_index(nb, idx);
  if ((t = *treebin_at(m, idx)) != 0) {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_tree_index(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    for (;;) {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      if (trem < rsize) {
        v = t;
        if ((rsize = trem) == 0)
          break;
      }
      rt = t->child[1];
      t = t->child[(sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1];
      if (rt != 0 && rt != t)
        rst = rt;
      if (t == 0) {
        t = rst; /* set t to least subtree holding sizes > nb */
        break;
      }
      sizebits <<= 1;
    }
  }
  if (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
    binmap_t leftbits = left_bits(idx2bit(idx)) & m->treemap;
    if (leftbits != 0) {
      bindex_t i;
      binmap_t leastbit = least_bit(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  while (t != 0) { /* find smallest of tree or subtree */
    size_t trem = chunksize(t) - nb;
    if (trem < rsize) {
      rsize = trem;
      v = t;
    }
    t = leftmost_child(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  if (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
    if (RTCHECK(ok_address(m, v))) { /* split */
      mchunkptr r = chunk_plus_offset(v, nb);
      assert(chunksize(v) == rsize + nb);
      if (RTCHECK(ok_next(v, r))) {
        unlink_large_chunk(m, v);
        if (rsize < MIN_CHUNK_SIZE)
          set_inuse_and_pinuse(m, v, (rsize + nb));
        else {
          set_size_and_pinuse_of_inuse_chunk(m, v, nb);
          set_size_and_pinuse_of_free_chunk(r, rsize);
          insert_chunk(m, r, rsize);
        }
        return chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  return 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
static void* tmalloc_small(mstate m, size_t nb)
{
    tchunkptr t, v;
    size_t rsize;
    bindex_t i;
    binmap_t leastbit = least_bit(m->treemap);
    compute_bit2idx(leastbit, i);
    v = t = *treebin_at(m, i);
    rsize = chunksize(t) - nb;

    while ((t = leftmost_child(t)) != 0) {
        size_t trem = chunksize(t) - nb;
        if (trem < rsize) {
            rsize = trem;
            v = t;
        }
    }

    if (RTCHECK(ok_address(m, v))) {
        mchunkptr r = chunk_plus_offset(v, nb);
        assert(chunksize(v) == rsize + nb);
        if (RTCHECK(ok_next(v, r))) {
            unlink_large_chunk(m, v);
            if (rsize < MIN_CHUNK_SIZE)
                set_inuse_and_pinuse(m, v, (rsize + nb));
            else {
                set_size_and_pinuse_of_inuse_chunk(m, v, nb);
                set_size_and_pinuse_of_free_chunk(r, rsize);
                replace_dv(m, r, rsize);
            }
            return chunk2mem(v);
        }
    }

    CORRUPTION_ERROR_ACTION(m);
    return 0;
}

/* --------------------------- memalign support -------------------------- */

static void* internal_memalign(mstate m, size_t alignment, size_t bytes)
{
    if (alignment <= MALLOC_ALIGNMENT)    /* Can just use malloc */
        return internal_malloc(m, bytes);
    if (alignment <  MIN_CHUNK_SIZE) /* must be at least a minimum chunk size */
        alignment = MIN_CHUNK_SIZE;
    if ((alignment & (alignment-SIZE_T_ONE)) != 0) {/* Ensure a power of 2 */
        size_t a = MALLOC_ALIGNMENT << 1;
        while (a < alignment) a <<= 1;
        alignment = a;
    }

    if (bytes >= MAX_REQUEST - alignment) {
        if (m != 0)  { /* Test isn't needed but avoids compiler warning */
//            MALLOC_FAILURE_ACTION;
        }
    }
    else
    {
        size_t nb = request2size(bytes);
        size_t req = nb + alignment + MIN_CHUNK_SIZE - CHUNK_OVERHEAD;
        char* mem = (char*)internal_malloc(m, req);
        if (mem != 0)
        {
            void* leader = 0;
            void* trailer = 0;
            mchunkptr p = mem2chunk(mem);

            PREACTION(m);

            if ((((size_t)(mem)) % alignment) != 0)  /* misaligned */
            {
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
                char* br = (char*)mem2chunk((size_t)(((size_t)(mem +
                                                       alignment -
                                                       SIZE_T_ONE)) &
                                             -alignment));
                char* pos = ((size_t)(br - (char*)(p)) >= MIN_CHUNK_SIZE)?
                             br : br+alignment;
                mchunkptr newp = (mchunkptr)pos;
                size_t leadsize = pos - (char*)(p);
                size_t newsize = chunksize(p) - leadsize;

                if (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
                    newp->prev_foot = p->prev_foot + leadsize;
                    newp->head = newsize;
                }
                else { /* Otherwise, give back leader, use the rest */
                    set_inuse(m, newp, newsize);
                    set_inuse(m, p, leadsize);
                    leader = chunk2mem(p);
                }
                p = newp;
            }

      /* Give back spare room at the end */
            if (!is_mmapped(p))
            {
                size_t size = chunksize(p);
                if (size > nb + MIN_CHUNK_SIZE)
                {
                    size_t remainder_size = size - nb;
                    mchunkptr remainder = chunk_plus_offset(p, nb);
                    set_inuse(m, p, nb);
                    set_inuse(m, remainder, remainder_size);
                    trailer = chunk2mem(remainder);
                }
            }

            assert (chunksize(p) >= nb);
            assert((((size_t)(chunk2mem(p))) % alignment) == 0);
            check_inuse_chunk(m, p);
            POSTACTION(m);
            if (leader != 0) {
                internal_free(m, leader);
            }
            if (trailer != 0) {
                internal_free(m, trailer);
            }
            return chunk2mem(p);
        }
    }
    return 0;
}

void* memalign(size_t alignment, size_t bytes)
{
    return internal_memalign(gm, alignment, bytes);
}


void* malloc(size_t bytes)
{
  /*
     Basic algorithm:
     If a small request (< 256 bytes minus per-chunk overhead):
       1. If one exists, use a remainderless chunk in associated smallbin.
          (Remainderless means that there are too few excess bytes to
          represent as a chunk.)
       2. If it is big enough, use the dv chunk, which is normally the
          chunk adjacent to the one used for the most recent small request.
       3. If one exists, split the smallest available chunk in a bin,
          saving remainder in dv.
       4. If it is big enough, use the top chunk.
       5. If available, get memory from system and use it
     Otherwise, for a large request:
       1. Find the smallest available binned chunk that fits, and use it
          if it is better fitting than dv chunk, splitting if necessary.
       2. If better fitting than any binned chunk, use the dv chunk.
       3. If it is big enough, use the top chunk.
       4. If request size >= mmap threshold, try to directly mmap this chunk.
       5. If available, get memory from system and use it

     The ugly goto's here ensure that postaction occurs along all paths.
  */

    ensure_initialization(); /* initialize in sys_alloc if not using locks */

    PREACTION(gm);
    {
        void* mem;
        size_t nb;

        if (bytes <= MAX_SMALL_REQUEST)
        {
            bindex_t idx;
            binmap_t smallbits;
            nb = (bytes < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(bytes);
            idx = small_index(nb);
            smallbits = gm->smallmap >> idx;

            if ((smallbits & 0x3U) != 0) /* Remainderless fit to a smallbin. */
            {
                mchunkptr b, p;
                idx += ~smallbits & 1;       /* Uses next bin if idx empty */
                b = smallbin_at(gm, idx);
                p = b->fd;
                assert(chunksize(p) == small_index2size(idx));
                unlink_first_small_chunk(gm, b, p, idx);
                set_inuse_and_pinuse(gm, p, small_index2size(idx));
                mem = chunk2mem(p);
                check_malloced_chunk(gm, mem, nb);
                goto postaction;
            }
            else if (nb > gm->dvsize)
            {
                if (smallbits != 0) /* Use chunk in next nonempty smallbin */
                {
                    mchunkptr b, p, r;
                    size_t rsize;
                    bindex_t i;
                    binmap_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
                    binmap_t leastbit = least_bit(leftbits);
                    compute_bit2idx(leastbit, i);
                    b = smallbin_at(gm, i);
                    p = b->fd;
                    assert(chunksize(p) == small_index2size(i));
                    unlink_first_small_chunk(gm, b, p, i);
                    rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
                    if (SIZE_T_SIZE != 4 && rsize < MIN_CHUNK_SIZE)
                        set_inuse_and_pinuse(gm, p, small_index2size(i));
                    else
                    {
                        set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
                        r = chunk_plus_offset(p, nb);
                        set_size_and_pinuse_of_free_chunk(r, rsize);
                        replace_dv(gm, r, rsize);
                    }
                    mem = chunk2mem(p);
                    check_malloced_chunk(gm, mem, nb);
                    goto postaction;
                }
                else if (gm->treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0)
                {
                    check_malloced_chunk(gm, mem, nb);
                    goto postaction;
                }
            }
        }
        else if (bytes >= MAX_REQUEST)
            nb = MAX_SIZE_T; /* Too big to allocate. Force failure (in sys alloc) */
        else
        {
            nb = pad_request(bytes);
            if (gm->treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0)
            {
                check_malloced_chunk(gm, mem, nb);
                goto postaction;
            }
        }

        if (nb <= gm->dvsize) {
            size_t rsize = gm->dvsize - nb;
            mchunkptr p = gm->dv;
            if (rsize >= MIN_CHUNK_SIZE) { /* split dv */
                mchunkptr r = gm->dv = chunk_plus_offset(p, nb);
                gm->dvsize = rsize;
                set_size_and_pinuse_of_free_chunk(r, rsize);
                set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
            }
            else { /* exhaust dv */
                size_t dvs = gm->dvsize;
                gm->dvsize = 0;
                gm->dv = 0;
                set_inuse_and_pinuse(gm, p, dvs);
            }
            mem = chunk2mem(p);
            check_malloced_chunk(gm, mem, nb);
            goto postaction;
        }
        else if (nb < gm->topsize) { /* Split top */
            size_t rsize = gm->topsize -= nb;
            mchunkptr p = gm->top;
            mchunkptr r = gm->top = chunk_plus_offset(p, nb);
            r->head = rsize | PINUSE_BIT;
            set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
            mem = chunk2mem(p);
            check_top_chunk(gm, gm->top);
            check_malloced_chunk(gm, mem, nb);
            goto postaction;
        }

        mem = sys_alloc(gm, nb);

  postaction:
        POSTACTION(gm);
        return mem;
    }

    return 0;
}


void free(void* mem)
{
  /*
     Consolidate freed chunks with preceeding or succeeding bordering
     free chunks, if they exist, and then place in a bin.  Intermixed
     with special cases for top, dv, mmapped chunks, and usage errors.
  */

    if (mem != 0)
    {
        mchunkptr p  = mem2chunk(mem);

#define fm gm

        PREACTION(fm);
        {
            check_inuse_chunk(fm, p);
            if (RTCHECK(ok_address(fm, p) && ok_inuse(p)))
            {
                size_t psize = chunksize(p);
                mchunkptr next = chunk_plus_offset(p, psize);
                if (!pinuse(p))
                {
                    size_t prevsize = p->prev_foot;
                    if (is_mmapped(p))
                    {
                        psize += prevsize + MMAP_FOOT_PAD;
                        if (CALL_MUNMAP((char*)p - prevsize, psize) == 0)
                        fm->footprint -= psize;
                        goto postaction;
                    }
                    else
                    {
                        mchunkptr prev = chunk_minus_offset(p, prevsize);
                        psize += prevsize;
                        p = prev;
                        if (RTCHECK(ok_address(fm, prev)))  /* consolidate backward */
                        {
                            if (p != fm->dv)
                            {
                                unlink_chunk(fm, p, prevsize);
                            }
                            else if ((next->head & INUSE_BITS) == INUSE_BITS)
                            {
                                fm->dvsize = psize;
                                set_free_with_pinuse(p, psize, next);
                                goto postaction;
                            }
                        }
                        else
                            goto erroraction;
                    }
                }

                if (RTCHECK(ok_next(p, next) && ok_pinuse(next)))
                {
                    if (!cinuse(next))    /* consolidate forward */
                    {
                        if (next == fm->top)
                        {
                            size_t tsize = fm->topsize += psize;
                            fm->top = p;
                            p->head = tsize | PINUSE_BIT;
                            if (p == fm->dv)
                            {
                                fm->dv = 0;
                                fm->dvsize = 0;
                            }
                            if (should_trim(fm, tsize))
                                sys_trim(fm, 0);
                            goto postaction;
                        }
                        else if (next == fm->dv)
                        {
                            size_t dsize = fm->dvsize += psize;
                            fm->dv = p;
                            set_size_and_pinuse_of_free_chunk(p, dsize);
                            goto postaction;
                        }
                        else
                        {
                            size_t nsize = chunksize(next);
                            psize += nsize;
                            unlink_chunk(fm, next, nsize);
                            set_size_and_pinuse_of_free_chunk(p, psize);
                            if (p == fm->dv)
                            {
                                fm->dvsize = psize;
                                goto postaction;
                            }
                        }
                    }
                    else
                        set_free_with_pinuse(p, psize, next);

                    if (is_small(psize))
                    {
                        insert_small_chunk(fm, p, psize);
                        check_free_chunk(fm, p);
                    }
                    else
                    {
                        tchunkptr tp = (tchunkptr)p;
                        insert_large_chunk(fm, tp, psize);
                        check_free_chunk(fm, p);
                        if (--fm->release_checks == 0)
                            release_unused_segments(fm);
                    }
                    goto postaction;
                }
            }
    erroraction:
        USAGE_ERROR_ACTION(fm, p);
    postaction:
        POSTACTION(fm);
        }
    }
#undef fm
}


