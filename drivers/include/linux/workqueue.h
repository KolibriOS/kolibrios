/*
 * workqueue.h --- work queue handling for Linux.
 */

#ifndef _LINUX_WORKQUEUE_H
#define _LINUX_WORKQUEUE_H

#include <linux/list.h>
#include <linux/linkage.h>
#include <linux/bitops.h>
#include <linux/lockdep.h>
#include <linux/threads.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>

struct workqueue_struct;

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);
void __stdcall delayed_work_timer_fn(unsigned long __data);

/*
 * The first word is the work queue pointer and the flags rolled into
 * one
 */
#define work_data_bits(work) ((unsigned long *)(&(work)->data))

enum {
	WORK_STRUCT_PENDING_BIT	= 0,	/* work item is pending execution */
	WORK_STRUCT_DELAYED_BIT	= 1,	/* work item is delayed */
	WORK_STRUCT_PWQ_BIT	= 2,	/* data points to pwq */
	WORK_STRUCT_LINKED_BIT	= 3,	/* next work is linked to this one */
#ifdef CONFIG_DEBUG_OBJECTS_WORK
	WORK_STRUCT_STATIC_BIT	= 4,	/* static initializer (debugobjects) */
	WORK_STRUCT_COLOR_SHIFT	= 5,	/* color for workqueue flushing */
#else
	WORK_STRUCT_COLOR_SHIFT	= 4,	/* color for workqueue flushing */
#endif

	WORK_STRUCT_COLOR_BITS	= 4,

	WORK_STRUCT_PENDING	= 1 << WORK_STRUCT_PENDING_BIT,
	WORK_STRUCT_DELAYED	= 1 << WORK_STRUCT_DELAYED_BIT,
	WORK_STRUCT_PWQ		= 1 << WORK_STRUCT_PWQ_BIT,
	WORK_STRUCT_LINKED	= 1 << WORK_STRUCT_LINKED_BIT,
#ifdef CONFIG_DEBUG_OBJECTS_WORK
	WORK_STRUCT_STATIC	= 1 << WORK_STRUCT_STATIC_BIT,
#else
	WORK_STRUCT_STATIC	= 0,
#endif

	/*
	 * The last color is no color used for works which don't
	 * participate in workqueue flushing.
	 */
	WORK_NR_COLORS		= (1 << WORK_STRUCT_COLOR_BITS) - 1,
	WORK_NO_COLOR		= WORK_NR_COLORS,

	/* not bound to any CPU, prefer the local CPU */
	WORK_CPU_UNBOUND	= NR_CPUS,

	/*
	 * Reserve 7 bits off of pwq pointer w/ debugobjects turned off.
	 * This makes pwqs aligned to 256 bytes and allows 15 workqueue
	 * flush colors.
	 */
	WORK_STRUCT_FLAG_BITS	= WORK_STRUCT_COLOR_SHIFT +
				  WORK_STRUCT_COLOR_BITS,

	/* data contains off-queue information when !WORK_STRUCT_PWQ */
	WORK_OFFQ_FLAG_BASE	= WORK_STRUCT_COLOR_SHIFT,

	__WORK_OFFQ_CANCELING	= WORK_OFFQ_FLAG_BASE,
	WORK_OFFQ_CANCELING	= (1 << __WORK_OFFQ_CANCELING),

	/*
	 * When a work item is off queue, its high bits point to the last
	 * pool it was on.  Cap at 31 bits and use the highest number to
	 * indicate that no pool is associated.
	 */
	WORK_OFFQ_FLAG_BITS	= 1,
	WORK_OFFQ_POOL_SHIFT	= WORK_OFFQ_FLAG_BASE + WORK_OFFQ_FLAG_BITS,
	WORK_OFFQ_LEFT		= BITS_PER_LONG - WORK_OFFQ_POOL_SHIFT,
	WORK_OFFQ_POOL_BITS	= WORK_OFFQ_LEFT <= 31 ? WORK_OFFQ_LEFT : 31,
	WORK_OFFQ_POOL_NONE	= (1LU << WORK_OFFQ_POOL_BITS) - 1,

	/* convenience constants */
	WORK_STRUCT_FLAG_MASK	= (1UL << WORK_STRUCT_FLAG_BITS) - 1,
	WORK_STRUCT_WQ_DATA_MASK = ~WORK_STRUCT_FLAG_MASK,
	WORK_STRUCT_NO_POOL	= (unsigned long)WORK_OFFQ_POOL_NONE << WORK_OFFQ_POOL_SHIFT,

	/* bit mask for work_busy() return values */
	WORK_BUSY_PENDING	= 1 << 0,
	WORK_BUSY_RUNNING	= 1 << 1,

	/* maximum string length for set_worker_desc() */
	WORKER_DESC_LEN		= 24,
};

struct work_struct {
	struct list_head entry;
	struct workqueue_struct *data;
	work_func_t func;
#ifdef CONFIG_LOCKDEP
	struct lockdep_map lockdep_map;
#endif
};

#define WORK_DATA_INIT()	ATOMIC_LONG_INIT(WORK_STRUCT_NO_POOL)
#define WORK_DATA_STATIC_INIT()	\
	ATOMIC_LONG_INIT(WORK_STRUCT_NO_POOL | WORK_STRUCT_STATIC)

struct delayed_work {
	struct work_struct work;
	unsigned int delay;
	/* target workqueue and CPU ->timer uses to queue ->work */
	struct workqueue_struct *wq;
	int cpu;
};

static inline struct delayed_work *to_delayed_work(struct work_struct *work)
{
	return container_of(work, struct delayed_work, work);
}

struct execute_work {
	struct work_struct work;
};

struct workqueue_struct {
    spinlock_t lock;
    struct list_head worklist;
    struct list_head delayed_worklist;
};
extern struct workqueue_struct *system_wq;

void run_workqueue(struct workqueue_struct *cwq);

struct workqueue_struct *alloc_workqueue_key(const char *fmt,
                           unsigned int flags, int max_active);


#define alloc_ordered_workqueue(fmt, flags, args...)            \
        alloc_workqueue(fmt, WQ_UNBOUND | (flags), 1, ##args)

bool queue_work(struct workqueue_struct *wq, struct work_struct *work);
int queue_delayed_work(struct workqueue_struct *wq,
                        struct delayed_work *dwork, unsigned long delay);

bool schedule_delayed_work(struct delayed_work *dwork, unsigned long delay);


#define INIT_WORK(_work, _func)                 \
    do {                                        \
        INIT_LIST_HEAD(&(_work)->entry);        \
        (_work)->func = _func;                  \
    } while (0)


#define INIT_DELAYED_WORK(_work, _func)         \
    do {                                        \
        INIT_LIST_HEAD(&(_work)->work.entry);   \
        (_work)->work.func = _func;             \
    } while (0)

static inline bool schedule_work(struct work_struct *work)
{
    return queue_work(system_wq, work);
}


#endif  /*  _LINUX_WORKQUEUE_H  */
