#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

#include <linux/list.h>
#include <syscall.h>

typedef struct __wait_queue wait_queue_t;
typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int flags, void *key);

typedef struct __wait_queue_head wait_queue_head_t;

struct __wait_queue
{
    wait_queue_func_t func;
    struct list_head task_list;
    evhandle_t evnt;
};

struct __wait_queue_head
{
    spinlock_t lock;
    struct list_head task_list;
};

static inline void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
    list_add(&new->task_list, &head->task_list);
}


#define __wait_event(wq, condition)                                     \
do {                                                                    \
        DEFINE_WAIT(__wait);                                            \
                                                                        \
        for (;;) {                                                      \
                prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);    \
                if (condition)                                          \
                        break;                                          \
                schedule();                                             \
        }                                                               \
        finish_wait(&wq, &__wait);                                      \
} while (0)



#define wait_event_timeout(wq, condition, timeout)          \
({                                                          \
    long __ret = timeout;                                   \
do{                                                         \
    wait_queue_t __wait = {                                 \
        .task_list = LIST_HEAD_INIT(__wait.task_list),      \
        .evnt      = CreateEvent(NULL, MANUAL_DESTROY),     \
    };                                                      \
    unsigned long flags;                                    \
                                                            \
    spin_lock_irqsave(&wq.lock, flags);                     \
    if (list_empty(&__wait.task_list))                      \
        __add_wait_queue(&wq, &__wait);                     \
    spin_unlock_irqrestore(&wq.lock, flags);                \
                                                            \
    for(;;){                                                \
        if (condition)                                      \
            break;                                          \
        WaitEventTimeout(__wait.evnt, timeout);             \
    };                                                      \
    if (!list_empty(&__wait.task_list)) {                   \
        spin_lock_irqsave(&wq.lock, flags);                 \
        list_del_init(&__wait.task_list);                   \
        spin_unlock_irqrestore(&wq.lock, flags);            \
    };                                                      \
    DestroyEvent(__wait.evnt);                              \
} while (0);                                                \
    __ret;                                                  \
})

#define wait_event_interruptible_timeout(wq, condition, timeout)    \
        wait_event_timeout(wq, condition, timeout)


#define wait_event(wq, condition)                           \
do{                                                         \
    wait_queue_t __wait = {                                 \
        .task_list = LIST_HEAD_INIT(__wait.task_list),      \
        .evnt      = CreateEvent(NULL, MANUAL_DESTROY),     \
    };                                                      \
    unsigned long flags;                                    \
                                                            \
    spin_lock_irqsave(&wq.lock, flags);                     \
    if (list_empty(&__wait.task_list))                      \
        __add_wait_queue(&wq, &__wait);                     \
    spin_unlock_irqrestore(&wq.lock, flags);                \
                                                            \
    for(;;){                                                \
        if (condition)                                      \
            break;                                          \
        WaitEvent(__wait.evnt);                             \
    };                                                      \
    if (!list_empty_careful(&__wait.task_list)) {           \
        spin_lock_irqsave(&wq.lock, flags);                 \
        list_del_init(&__wait.task_list);                   \
        spin_unlock_irqrestore(&wq.lock, flags);            \
    };                                                      \
    DestroyEvent(__wait.evnt);                              \
} while (0)




static inline
void wake_up_all(wait_queue_head_t *q)
{
    wait_queue_t *curr;
    unsigned long flags;

    spin_lock_irqsave(&q->lock, flags);
    list_for_each_entry(curr, &q->task_list, task_list)
    {
//        printf("raise event \n");

        kevent_t event;
        event.code = -1;
        RaiseEvent(curr->evnt, 0, &event);
    }
    spin_unlock_irqrestore(&q->lock, flags);
}


static inline void
init_waitqueue_head(wait_queue_head_t *q)
{
    spin_lock_init(&q->lock);
    INIT_LIST_HEAD(&q->task_list);
};


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

struct work_struct;

struct workqueue_struct {
    spinlock_t lock;
    struct list_head worklist;
};

typedef void (*work_func_t)(struct work_struct *work);

struct work_struct {
    struct list_head entry;
    struct workqueue_struct *data;
    work_func_t func;
};

struct delayed_work {
    struct work_struct work;
};

struct workqueue_struct *alloc_workqueue_key(const char *fmt,
                           unsigned int flags, int max_active);


#define alloc_ordered_workqueue(fmt, flags, args...)            \
        alloc_workqueue(fmt, WQ_UNBOUND | (flags), 1, ##args)

int queue_delayed_work(struct workqueue_struct *wq,
                        struct delayed_work *dwork, unsigned long delay);

#define INIT_DELAYED_WORK(_work, _func)         \
    do {                                        \
        INIT_LIST_HEAD(&(_work)->work.entry);   \
        (_work)->work.func = _func;             \
    } while (0)


struct completion {
    unsigned int done;
    wait_queue_head_t wait;
};

int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key);


#define DEFINE_WAIT_FUNC(name, function)                                \
        wait_queue_t name = {                                           \
                .func           = function,                             \
                .task_list      = LIST_HEAD_INIT((name).task_list),     \
                .evnt           = CreateEvent(NULL, MANUAL_DESTROY),    \
        }

#define DEFINE_WAIT(name) DEFINE_WAIT_FUNC(name, autoremove_wake_function)


#endif

