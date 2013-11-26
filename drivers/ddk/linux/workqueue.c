#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <ddk.h>

extern int driver_wq_state;

struct workqueue_struct *alloc_workqueue(const char *fmt,
                           unsigned int flags,
                           int max_active)
{
    struct workqueue_struct *wq;

    wq = kzalloc(sizeof(*wq),0);
    if (!wq)
        goto err;

    INIT_LIST_HEAD(&wq->worklist);
    INIT_LIST_HEAD(&wq->delayed_worklist);

    return wq;
err:
    return NULL;
}



void run_workqueue(struct workqueue_struct *cwq)
{
    unsigned long irqflags;

//    dbgprintf("wq: %x head %x, next %x\n",
//               cwq, &cwq->worklist, cwq->worklist.next);

    while(driver_wq_state != 0)
    {
        spin_lock_irqsave(&cwq->lock, irqflags);

        while (!list_empty(&cwq->worklist))
        {
            struct work_struct *work = list_entry(cwq->worklist.next,
                                        struct work_struct, entry);
            work_func_t f = work->func;
            list_del_init(cwq->worklist.next);
//            printf("work %p, func %p\n",
//                      work, f);

            spin_unlock_irqrestore(&cwq->lock, irqflags);
            f(work);
            spin_lock_irqsave(&cwq->lock, irqflags);
        }

        spin_unlock_irqrestore(&cwq->lock, irqflags);

        delay(1);
    };
}


bool queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
    unsigned long flags;

    if(!list_empty(&work->entry))
        return 0;

//    dbgprintf("%s %p queue: %p\n", __FUNCTION__, work, wq);

    spin_lock_irqsave(&wq->lock, flags);

    list_add_tail(&work->entry, &wq->worklist);

    spin_unlock_irqrestore(&wq->lock, flags);

    return 1;
};


void __stdcall delayed_work_timer_fn(unsigned long __data)
{
    struct delayed_work *dwork = (struct delayed_work *)__data;
    struct workqueue_struct *wq = dwork->work.data;

    queue_work(wq, &dwork->work);
}

int queue_delayed_work(struct workqueue_struct *wq,
                        struct delayed_work *dwork, unsigned long delay)
{
    struct work_struct *work = &dwork->work;

    if (delay == 0)
        return queue_work(wq, &dwork->work);

//    dbgprintf("%s %p queue: %p\n", __FUNCTION__, &dwork->work, wq);

    work->data = wq;
    TimerHS(delay,0, delayed_work_timer_fn, dwork);
    return 1;
}


bool schedule_delayed_work(struct delayed_work *dwork, unsigned long delay)
{
    return queue_delayed_work(system_wq, dwork, delay);
}

bool mod_delayed_work(struct workqueue_struct *wq,
                                    struct delayed_work *dwork,
                                    unsigned long delay)
{
    return queue_delayed_work(wq, dwork, delay);
}

int del_timer(struct timer_list *timer)
{
    int ret = 0;

    if(timer->handle)
    {
        CancelTimerHS(timer->handle);
        timer->handle = 0;
        ret = 1;
    };
    return ret;
};

bool cancel_work_sync(struct work_struct *work)
{
    unsigned long flags;
    int ret = 0;

    spin_lock_irqsave(&system_wq->lock, flags);
    if(!list_empty(&work->entry))
    {
        list_del(&work->entry);
        ret = 1;
    };
    spin_unlock_irqrestore(&system_wq->lock, flags);
    return ret;
}

bool cancel_delayed_work(struct delayed_work *dwork)
{
    return cancel_work_sync(&dwork->work);
}

bool cancel_delayed_work_sync(struct delayed_work *dwork)
{
    return cancel_work_sync(&dwork->work);
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
    int ret = 0;
    expires - GetTimerTicks();

    if(timer->handle)
    {
        CancelTimerHS(timer->handle);
        timer->handle = 0;
        ret = 1;
    };

    timer->handle = TimerHS(expires, 0, timer->function, timer->data);

    return ret;
}

