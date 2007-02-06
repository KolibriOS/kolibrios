

//#include "kolibc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef unsigned int dword;
typedef unsigned int size_t;


#define PINUSE_BIT    1
#define CINUSE_BIT    2
#define INUSE_BITS    3


struct m_seg
{
  char*         base;          /* base address */
  dword         size;          /* allocated size */
  struct m_seg* next;          /* ptr to next segment */
  dword         flags;         /* mmap and extern flag */
};

struct m_chunk
{
  dword           prev_foot;  /* Size of previous chunk (if free).  */
  dword           head;       /* Size and inuse bits. */
  struct m_chunk* fd;         /* double links -- used only if free. */
  struct m_chunk* bk;
};

typedef struct m_chunk* mchunkptr;

struct t_chunk
{
  /* The first four fields must be compatible with malloc_chunk */
  dword           prev_foot;
  dword           head;
  
  struct t_chunk* fd;
  struct t_chunk* bk;

  struct t_chunk* child[2];

  struct t_chunk* parent;
  dword           index;
};

typedef struct t_chunk* tchunkptr;
typedef struct t_chunk* tbinptr;

typedef struct m_state 
{
  dword      smallmap;
  dword      treemap;
//  DWORD      dvsize;
  dword      topsize;
  char*      least_addr;
//  mchunkptr  dv;
  mchunkptr  top;
  dword      magic;
  struct m_chunk    smallbins[32];
  tbinptr    treebins[32];
};


void _cdecl  mf_init();
void* _cdecl dlmalloc(size_t);
void* _cdecl dlrealloc(void *,size_t);
void  _cdecl dlfree(void*);


dword compute_tree_index(size_t s);

static void insert_chunk(mchunkptr P, size_t S);
static void insert_large_chunk(tchunkptr X, size_t S);

static void unlink_large_chunk(tchunkptr X);

//void replace_dv(mchunkptr P, size_t S);
static void* malloc_small(size_t nb);
static void* malloc_large(size_t nb);

#define leftshift_for_tree_index(i) \
   ((i == 31)? 0 : (31 - (i >> 1) + 8 - 2))

#define leftmost_child(t) ((t)->child[0] != 0? (t)->child[0] : (t)->child[1])
#define chunk2mem(p)    (void*)((char*)p + 8)
#define mem2chunk(mem)  (mchunkptr)((char*)mem - 8)
#define chunk_plus_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))


#ifdef __cplusplus
}
#endif /* __cplusplus */



