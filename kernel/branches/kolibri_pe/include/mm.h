
typedef struct
{
  link_t buddy_link;        /**< link to the next free block inside one  order */
  count_t refcount;         /**< tracking of shared frames  */
  u32_t buddy_order;        /**< buddy system block order */
  void *parent;             /**< If allocated by slab, this points there */
} frame_t;

typedef struct {
  SPINLOCK_DECLARE(lock);   /**< this lock protects everything below */
  pfn_t base;               /**< frame_no of the first frame in the frames array */
  count_t count;            /**< Size of zone */

  frame_t *frames;          /**< array of frame_t structures in this zone */
  count_t free_count;       /**< number of free frame_t structures */
  count_t busy_count;       /**< number of busy frame_t structures */

  u32_t max_order;
  link_t order[21];

	int flags;
} zone_t;


# define PA2KA(x) (((u32_t) (x)) + OS_BASE)
# define KA2PA(x) (((u32_t) (x)) - OS_BASE)

#define PAGE_SIZE    4096
#define FRAME_WIDTH  12

#define BUDDY_SYSTEM_INNER_BLOCK  0xff

static inline count_t SIZE2FRAMES(size_t size)
{
	if (!size)
		return 0;
  return (count_t) ((size - 1) >> FRAME_WIDTH) + 1;
}

static inline addr_t PFN2ADDR(pfn_t frame)
{
  return (addr_t) (frame << FRAME_WIDTH);
}

static inline pfn_t ADDR2PFN(addr_t addr)
{
	return (pfn_t) (addr >> FRAME_WIDTH);
};

void init_mm();

pfn_t  core_alloc(u32_t order);

pfn_t alloc_page() __attribute__ ((deprecated));
pfn_t __stdcall alloc_pages(count_t count) __asm__ ("_alloc_pages") __attribute__ ((deprecated));

void core_free(pfn_t frame);
void frame_free(pfn_t frame);
