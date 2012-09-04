
typedef struct
{
  link_t   link;
  addr_t   base;
  size_t   size;
  addr_t   pte[0];

}mmap_t;


typedef struct
{
  link_t  buddy_link;         /**< link to the next free block inside one  order */
  u16_t   refcount;           /**< tracking of shared frames  */
  u16_t   buddy_order;        /**< buddy system block order */
  void   *parent;             /**< If allocated by slab, this points there */
} frame_t;


typedef struct
{
  SPINLOCK_DECLARE(lock);     /**< this lock protects everything below */
  pfn_t    base;              /**< frame_no of the first frame in the frames array */
  count_t  count;             /**< Size of zone */

  frame_t *frames;            /**< array of frame_t structures in this zone */
  count_t  free_count;        /**< number of free frame_t structures */
  count_t  busy_count;        /**< number of busy frame_t structures */

  u32_t    max_order;
  link_t   order[21];

  int      flags;
} zone_t;


typedef struct
{
    link_t  link;
    link_t  adj;
    addr_t  base;
    size_t  size;
    void   *parent;
    u32_t   state;
}md_t;


#define PG_MAP          1
#define PG_WRITE        2
#define PG_USER         4

#define PG_SW           3
#define PG_UW           7


#define PAGE_SIZE    4096
#define PAGE_WIDTH     12


# define PA2KA(x) (((addr_t) (x)) + OS_BASE)
# define KA2PA(x) (((addr_t) (x)) - OS_BASE)

static inline count_t SIZE2FRAMES(size_t size)
{
    if (!size)
        return 0;
  return (count_t) ((size - 1) >> PAGE_WIDTH) + 1;
}

static inline addr_t PFN2ADDR(pfn_t frame)
{
  return (addr_t) (frame << PAGE_WIDTH);
}

static inline pfn_t ADDR2PFN(addr_t addr)
{
    return (pfn_t) (addr >> PAGE_WIDTH);
};

void init_mm();
void init_pg_slab();

void* __fastcall frame_get_parent(pfn_t pfn);
void  __fastcall frame_set_parent(pfn_t pfn, void *data);


addr_t __fastcall core_alloc(u32_t order);
void   __fastcall core_free(addr_t frame);


addr_t alloc_page(void);


md_t* __fastcall md_alloc(size_t size, u32_t flags) ;
void  __fastcall md_free(md_t *md);

addr_t __fastcall __export mem_alloc(size_t size, u32_t flags) asm ("MemAlloc");
void   __fastcall __export mem_free(addr_t mem) asm ("MemFree");

addr_t __fastcall frame_alloc(size_t size);
size_t __fastcall frame_free(addr_t addr);

