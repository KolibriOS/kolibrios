#ifndef _LINUX_WORKQUEUE_H
#define _LINUX_WORKQUEUE_H

#include <linux/list.h>
#include <syscall.h>

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);

/*
 * Workqueue flags and constants.  For details, please refer to
 * Documentation/workqueue.txt.
 */
enum {
    WQ_NON_REENTRANT    = 1 << 0, /* guarantee non-reentrance */
    WQ_UNBOUND          = 1 << 1, /* not bound to any cpu */
    WQ_FREEZABLE        = 1 << 2, /* freeze during suspend */
    WQ_MEM_RECLAIM      = 1 << 3, /* may be used for memory reclaim */
    WQ_HIGHPRI          = 1 << 4, /* high priority */
    WQ_CPU_INTENSIVE    = 1 << 5, /* cpu instensive workqueue */

    WQ_DRAINING         = 1 << 6, /* internal: workqueue is draining */
    WQ_RESCUER          = 1 << 7, /* internal: workqueue has rescuer */

    WQ_MAX_ACTIVE       = 512,    /* I like 512, better ideas? */
    WQ_MAX_UNBOUND_PER_CPU  = 4,      /* 4 * #cpus for unbound wq */
    WQ_DFL_ACTIVE       = WQ_MAX_ACTIVE / 2,
};


struct workqueue_struct {
    spinlock_t lock;
    struct list_head worklist;
    struct list_head delayed_worklist;
};

struct work_struct {
    struct list_head entry;
    struct workqueue_struct *data;
    work_func_t func;
};

struct delayed_work {
    struct work_struct work;
    unsigned int delay;
};

static inline struct delayed_work *to_delayed_work(struct work_struct *work)
{
    return container_of(work, struct delayed_work, work);
}

extern struct workqueue_struct *system_wq;

void run_workqueue(struct workqueue_struct *cwq);

struct workqueue_struct *alloc_workqueue_key(const char *fmt,
                           unsigned int flags, int max_active);


#define alloc_ordered_workqueue(fmt, flags, args...)            \
        alloc_workqueue(fmt, WQ_UNBOUND | (flags), 1, ##args)

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



#endif  /*  _LINUX_WORKQUEUE_H  */
