#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H
/*
 * Linux wait queue related types and methods
 */
#include <linux/list.h>
#include <linux/stddef.h>
#include <linux/spinlock.h>
#include <asm/current.h>
#include <syscall.h>

typedef struct __wait_queue wait_queue_t;
typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int flags, void *key);
int default_wake_function(wait_queue_t *wait, unsigned mode, int flags, void *key);

/* __wait_queue::flags */
#define WQ_FLAG_EXCLUSIVE	0x01
#define WQ_FLAG_WOKEN		0x02

struct __wait_queue {
	unsigned int		flags;
	void			*private;
	wait_queue_func_t	func;
	struct list_head	task_list;
	evhandle_t evnt;
};

struct wait_bit_key {
	void			*flags;
	int			bit_nr;
#define WAIT_ATOMIC_T_BIT_NR	-1
	unsigned long		timeout;
};

struct wait_bit_queue {
	struct wait_bit_key	key;
	wait_queue_t		wait;
};

struct __wait_queue_head {
	spinlock_t		lock;
	struct list_head	task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

struct task_struct;

/*
 * Macros for declaration and initialisaton of the datatypes
 */

#define __WAITQUEUE_INITIALIZER(name, tsk) {				\
	.private	= tsk,						\
	.func		= default_wake_function,			\
	.task_list	= { NULL, NULL } }

#define DECLARE_WAITQUEUE(name, tsk)					\
	wait_queue_t name = __WAITQUEUE_INITIALIZER(name, tsk)

#define __WAIT_QUEUE_HEAD_INITIALIZER(name) {				\
	.lock		= __SPIN_LOCK_UNLOCKED(name.lock),		\
	.task_list	= { &(name).task_list, &(name).task_list } }

#define DECLARE_WAIT_QUEUE_HEAD(name) \
	wait_queue_head_t name = __WAIT_QUEUE_HEAD_INITIALIZER(name)

#define __WAIT_BIT_KEY_INITIALIZER(word, bit)				\
	{ .flags = word, .bit_nr = bit, }

#define __WAIT_ATOMIC_T_KEY_INITIALIZER(p)				\
	{ .flags = p, .bit_nr = WAIT_ATOMIC_T_BIT_NR, }

extern void __init_waitqueue_head(wait_queue_head_t *q, const char *name, struct lock_class_key *);

#ifdef CONFIG_LOCKDEP
# define __WAIT_QUEUE_HEAD_INIT_ONSTACK(name) \
	({ init_waitqueue_head(&name); name; })
# define DECLARE_WAIT_QUEUE_HEAD_ONSTACK(name) \
	wait_queue_head_t name = __WAIT_QUEUE_HEAD_INIT_ONSTACK(name)
#else
# define DECLARE_WAIT_QUEUE_HEAD_ONSTACK(name) DECLARE_WAIT_QUEUE_HEAD(name)
#endif

static inline void init_waitqueue_entry(wait_queue_t *q, struct task_struct *p)
{
	q->flags	= 0;
	q->private	= p;
	q->func		= default_wake_function;
}

static inline void
init_waitqueue_func_entry(wait_queue_t *q, wait_queue_func_t func)
{
	q->flags	= 0;
	q->private	= NULL;
	q->func		= func;
}

static inline int waitqueue_active(wait_queue_head_t *q)
{
	return !list_empty(&q->task_list);
}

extern void add_wait_queue(wait_queue_head_t *q, wait_queue_t *wait);
extern void add_wait_queue_exclusive(wait_queue_head_t *q, wait_queue_t *wait);
extern void remove_wait_queue(wait_queue_head_t *q, wait_queue_t *wait);

static inline void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
	list_add(&new->task_list, &head->task_list);
}

/*
 * Used for wake-one threads:
 */
static inline void
__add_wait_queue_exclusive(wait_queue_head_t *q, wait_queue_t *wait)
{
	wait->flags |= WQ_FLAG_EXCLUSIVE;
	__add_wait_queue(q, wait);
}

static inline void __add_wait_queue_tail(wait_queue_head_t *head,
					 wait_queue_t *new)
{
	list_add_tail(&new->task_list, &head->task_list);
}

static inline void
__add_wait_queue_tail_exclusive(wait_queue_head_t *q, wait_queue_t *wait)
{
	wait->flags |= WQ_FLAG_EXCLUSIVE;
	__add_wait_queue_tail(q, wait);
}

static inline void
__remove_wait_queue(wait_queue_head_t *head, wait_queue_t *old)
{
	list_del(&old->task_list);
}

typedef int wait_bit_action_f(struct wait_bit_key *, int mode);
void __wake_up(wait_queue_head_t *q, unsigned int mode, int nr, void *key);
void __wake_up_locked_key(wait_queue_head_t *q, unsigned int mode, void *key);
void __wake_up_sync_key(wait_queue_head_t *q, unsigned int mode, int nr, void *key);
void __wake_up_locked(wait_queue_head_t *q, unsigned int mode, int nr);
void __wake_up_sync(wait_queue_head_t *q, unsigned int mode, int nr);
void __wake_up_bit(wait_queue_head_t *, void *, int);
int __wait_on_bit(wait_queue_head_t *, struct wait_bit_queue *, wait_bit_action_f *, unsigned);
int __wait_on_bit_lock(wait_queue_head_t *, struct wait_bit_queue *, wait_bit_action_f *, unsigned);
void wake_up_bit(void *, int);
void wake_up_atomic_t(atomic_t *);
int out_of_line_wait_on_bit(void *, int, wait_bit_action_f *, unsigned);
int out_of_line_wait_on_bit_timeout(void *, int, wait_bit_action_f *, unsigned, unsigned long);
int out_of_line_wait_on_bit_lock(void *, int, wait_bit_action_f *, unsigned);
int out_of_line_wait_on_atomic_t(atomic_t *, int (*)(atomic_t *), unsigned);
wait_queue_head_t *bit_waitqueue(void *, int);

/*

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

*/

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
    do{                                                     \
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

#define wait_event_interruptible(wq, condition)             \
({                                                          \
    int __ret = 0;                                          \
    if (!(condition))                                       \
        wait_event(wq, condition);                          \
    __ret;                                                  \
})

static inline
void wake_up(wait_queue_head_t *q)
{
    wait_queue_t *curr;
    unsigned long flags;

    spin_lock_irqsave(&q->lock, flags);
    curr = list_first_entry_or_null(&q->task_list, typeof(*curr), task_list);
    if(curr != NULL)
    {
        if(!WARN_ON(curr->evnt.handle == 0))
        {
            kevent_t event = {0};
            event.code = -1;
            RaiseEvent(curr->evnt, 0, &event);
        }
    }
    spin_unlock_irqrestore(&q->lock, flags);
}

static inline
void wake_up_interruptible(wait_queue_head_t *q)
{
    wait_queue_t *curr;
    unsigned long flags;

    spin_lock_irqsave(&q->lock, flags);
    curr = list_first_entry_or_null(&q->task_list, typeof(*curr), task_list);
    if(curr != NULL)
    {
        if(!WARN_ON(curr->evnt.handle == 0))
        {
            kevent_t event = {0};
            event.code = -1;
            RaiseEvent(curr->evnt, 0, &event);
        }
    }
    spin_unlock_irqrestore(&q->lock, flags);
}

static inline
void wake_up_all(wait_queue_head_t *q)
{
    wait_queue_t *curr;
    unsigned long flags;
    spin_lock_irqsave(&q->lock, flags);
    list_for_each_entry(curr, &q->task_list, task_list)
    {
        if(WARN_ON(curr->evnt.handle == 0))
            continue;
        kevent_t event = {0};
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


//struct completion {
//    unsigned int done;
//    wait_queue_head_t wait;
//};

void prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state);
void finish_wait(wait_queue_head_t *q, wait_queue_t *wait);
int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key);


#define DEFINE_WAIT_FUNC(name, function)                                \
        wait_queue_t name = {                                           \
                .func           = function,                             \
                .task_list      = LIST_HEAD_INIT((name).task_list),     \
                .evnt           = CreateEvent(NULL, MANUAL_DESTROY),    \
        }

#define DEFINE_WAIT(name) DEFINE_WAIT_FUNC(name, autoremove_wake_function)


#endif

