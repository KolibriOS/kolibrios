#include <ddk.h>
#include <linux/mm.h>
#include <drm/drmP.h>
#include <linux/hdmi.h>
#include "radeon.h"

int x86_clflush_size;
unsigned int tsc_khz;

struct file *shmem_file_setup(const char *name, loff_t size, unsigned long flags)
{
    struct file *filep;
    int count;

    filep = __builtin_malloc(sizeof(*filep));

    if(unlikely(filep == NULL))
        return ERR_PTR(-ENOMEM);

    count = size / PAGE_SIZE;

    filep->pages = kzalloc(sizeof(struct page *) * count, 0);
    if(unlikely(filep->pages == NULL))
    {
        kfree(filep);
        return ERR_PTR(-ENOMEM);
    };

    filep->count     = count;
    filep->allocated = 0;
    filep->vma       = NULL;

//    printf("%s file %p pages %p count %d\n",
//              __FUNCTION__,filep, filep->pages, count);

    return filep;
}

static void *check_bytes8(const u8 *start, u8 value, unsigned int bytes)
{
        while (bytes) {
                if (*start != value)
                        return (void *)start;
                start++;
                bytes--;
        }
        return NULL;
}

/**
 * memchr_inv - Find an unmatching character in an area of memory.
 * @start: The memory area
 * @c: Find a character other than c
 * @bytes: The size of the area.
 *
 * returns the address of the first character other than @c, or %NULL
 * if the whole buffer contains just @c.
 */
void *memchr_inv(const void *start, int c, size_t bytes)
{
        u8 value = c;
        u64 value64;
        unsigned int words, prefix;

        if (bytes <= 16)
                return check_bytes8(start, value, bytes);

        value64 = value;
#if defined(ARCH_HAS_FAST_MULTIPLIER) && BITS_PER_LONG == 64
        value64 *= 0x0101010101010101;
#elif defined(ARCH_HAS_FAST_MULTIPLIER)
        value64 *= 0x01010101;
        value64 |= value64 << 32;
#else
        value64 |= value64 << 8;
        value64 |= value64 << 16;
        value64 |= value64 << 32;
#endif

        prefix = (unsigned long)start % 8;
        if (prefix) {
                u8 *r;

                prefix = 8 - prefix;
                r = check_bytes8(start, value, prefix);
                if (r)
                        return r;
                start += prefix;
                bytes -= prefix;
        }

        words = bytes / 8;

        while (words) {
                if (*(u64 *)start != value64)
                        return check_bytes8(start, value, 8);
                start += 8;
                words--;
        }

        return check_bytes8(start, value, bytes % 8);
}



#define _U  0x01    /* upper */
#define _L  0x02    /* lower */
#define _D  0x04    /* digit */
#define _C  0x08    /* cntrl */
#define _P  0x10    /* punct */
#define _S  0x20    /* white space (space/lf/tab) */
#define _X  0x40    /* hex digit */
#define _SP 0x80    /* hard space (0x20) */

extern const unsigned char _ctype[];

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)  ((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)  ((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)  ((__ismask(c)&(_C)) != 0)
#define isdigit(c)  ((__ismask(c)&(_D)) != 0)
#define isgraph(c)  ((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)  ((__ismask(c)&(_L)) != 0)
#define isprint(c)  ((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunct(c)  ((__ismask(c)&(_P)) != 0)
/* Note: isspace() must return false for %NUL-terminator */
#define isspace(c)  ((__ismask(c)&(_S)) != 0)
#define isupper(c)  ((__ismask(c)&(_U)) != 0)
#define isxdigit(c) ((__ismask(c)&(_D|_X)) != 0)

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

static inline unsigned char __tolower(unsigned char c)
{
    if (isupper(c))
        c -= 'A'-'a';
    return c;
}

static inline unsigned char __toupper(unsigned char c)
{
    if (islower(c))
        c -= 'a'-'A';
    return c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

/*
 * Fast implementation of tolower() for internal usage. Do not use in your
 * code.
 */
static inline char _tolower(const char c)
{
    return c | 0x20;
}



void msleep(unsigned int msecs)
{
    msecs /= 10;
    if(!msecs) msecs = 1;

     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (msecs));
     __asm__ __volatile__ (
     "":::"ebx");

};


/* simple loop based delay: */
static void delay_loop(unsigned long loops)
{
        asm volatile(
                "       test %0,%0      \n"
                "       jz 3f           \n"
                "       jmp 1f          \n"

                ".align 16              \n"
                "1:     jmp 2f          \n"

                ".align 16              \n"
                "2:     dec %0          \n"
                "       jnz 2b          \n"
                "3:     dec %0          \n"

                : /* we don't need output */
                :"a" (loops)
        );
}


static void (*delay_fn)(unsigned long) = delay_loop;

void __delay(unsigned long loops)
{
        delay_fn(loops);
}


inline void __const_udelay(unsigned long xloops)
{
        int d0;

        xloops *= 4;
        asm("mull %%edx"
                : "=d" (xloops), "=&a" (d0)
                : "1" (xloops), ""
                (loops_per_jiffy * (HZ/4)));

        __delay(++xloops);
}

void __udelay(unsigned long usecs)
{
        __const_udelay(usecs * 0x000010c7); /* 2**32 / 1000000 (rounded up) */
}

unsigned int _sw_hweight32(unsigned int w)
{
#ifdef CONFIG_ARCH_HAS_FAST_MULTIPLIER
        w -= (w >> 1) & 0x55555555;
        w =  (w & 0x33333333) + ((w >> 2) & 0x33333333);
        w =  (w + (w >> 4)) & 0x0f0f0f0f;
        return (w * 0x01010101) >> 24;
#else
        unsigned int res = w - ((w >> 1) & 0x55555555);
        res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
        res = (res + (res >> 4)) & 0x0F0F0F0F;
        res = res + (res >> 8);
        return (res + (res >> 16)) & 0x000000FF;
#endif
}
EXPORT_SYMBOL(_sw_hweight32);


void usleep_range(unsigned long min, unsigned long max)
{
    udelay(max);
}
EXPORT_SYMBOL(usleep_range);


void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
    void *p;

    p = kmalloc(len, gfp);
    if (p)
        memcpy(p, src, len);
    return p;
}

void cpu_detect1()
{

    u32 junk, tfms, cap0, misc;
    int i;

    cpuid(0x00000001, &tfms, &misc, &junk, &cap0);

    if (cap0 & (1<<19))
    {
        x86_clflush_size = ((misc >> 8) & 0xff) * 8;
    }

#if 0
    cpuid(0x80000002, (unsigned int*)&cpuinfo.model_name[0], (unsigned int*)&cpuinfo.model_name[4],
          (unsigned int*)&cpuinfo.model_name[8], (unsigned int*)&cpuinfo.model_name[12]);
    cpuid(0x80000003, (unsigned int*)&cpuinfo.model_name[16], (unsigned int*)&cpuinfo.model_name[20],
          (unsigned int*)&cpuinfo.model_name[24], (unsigned int*)&cpuinfo.model_name[28]);
    cpuid(0x80000004, (unsigned int*)&cpuinfo.model_name[32], (unsigned int*)&cpuinfo.model_name[36],
          (unsigned int*)&cpuinfo.model_name[40], (unsigned int*)&cpuinfo.model_name[44]);

    printf("\n%s\n\n",cpuinfo.model_name);

    cpuinfo.def_mtrr = read_msr(MSR_MTRRdefType);
    cpuinfo.mtrr_cap = read_msr(IA32_MTRRCAP);

    printf("MSR_MTRRdefType %016llx\n\n", cpuinfo.def_mtrr);

    cpuinfo.var_mtrr_count = (u8_t)cpuinfo.mtrr_cap;

    for(i = 0; i < cpuinfo.var_mtrr_count; i++)
    {
        u64_t mtrr_base;
        u64_t mtrr_mask;

        cpuinfo.var_mtrr[i].base = read_msr(MTRRphysBase_MSR(i));
        cpuinfo.var_mtrr[i].mask = read_msr(MTRRphysMask_MSR(i));

        printf("MTRR_%d base: %016llx mask: %016llx\n", i,
               cpuinfo.var_mtrr[i].base,
               cpuinfo.var_mtrr[i].mask);
    };

    unsigned int cr0, cr3, cr4, eflags;

    eflags = safe_cli();

    /* Enter the no-fill (CD=1, NW=0) cache mode and flush caches. */
    cr0 = read_cr0() | (1<<30);
    write_cr0(cr0);
    wbinvd();

    cr4 = read_cr4();
    write_cr4(cr4 & ~(1<<7));

    cr3 = read_cr3();
    write_cr3(cr3);

    /* Save MTRR state */
    rdmsr(MSR_MTRRdefType, deftype_lo, deftype_hi);

    /* Disable MTRRs, and set the default type to uncached */
    native_write_msr(MSR_MTRRdefType, deftype_lo & ~0xcff, deftype_hi);
    wbinvd();

    i = 0;
    set_mtrr(i++,0,0x80000000>>12,MTRR_WB);
    set_mtrr(i++,0x80000000>>12,0x40000000>>12,MTRR_WB);
    set_mtrr(i++,0xC0000000>>12,0x20000000>>12,MTRR_WB);
    set_mtrr(i++,0xdb800000>>12,0x00800000>>12,MTRR_UC);
    set_mtrr(i++,0xdc000000>>12,0x04000000>>12,MTRR_UC);
    set_mtrr(i++,0xE0000000>>12,0x10000000>>12,MTRR_WC);

    for(; i < cpuinfo.var_mtrr_count; i++)
        set_mtrr(i,0,0,0);

    write_cr3(cr3);

    /* Intel (P6) standard MTRRs */
    native_write_msr(MSR_MTRRdefType, deftype_lo, deftype_hi);

    /* Enable caches */
    write_cr0(read_cr0() & ~(1<<30));

    /* Restore value of CR4 */
    write_cr4(cr4);

    safe_sti(eflags);

    printf("\nnew MTRR map\n\n");

    for(i = 0; i < cpuinfo.var_mtrr_count; i++)
    {
        u64_t mtrr_base;
        u64_t mtrr_mask;

        cpuinfo.var_mtrr[i].base = read_msr(MTRRphysBase_MSR(i));
        cpuinfo.var_mtrr[i].mask = read_msr(MTRRphysMask_MSR(i));

        printf("MTRR_%d base: %016llx mask: %016llx\n", i,
               cpuinfo.var_mtrr[i].base,
               cpuinfo.var_mtrr[i].mask);
    };
#endif

    tsc_khz = (unsigned int)(GetCpuFreq()/1000);
}


static atomic_t fence_context_counter = ATOMIC_INIT(0);

/**
 * fence_context_alloc - allocate an array of fence contexts
 * @num:        [in]    amount of contexts to allocate
 *
 * This function will return the first index of the number of fences allocated.
 * The fence context is used for setting fence->context to a unique number.
 */
unsigned fence_context_alloc(unsigned num)
{
        BUG_ON(!num);
        return atomic_add_return(num, &fence_context_counter) - num;
}
EXPORT_SYMBOL(fence_context_alloc);


int fence_signal(struct fence *fence)
{
        unsigned long flags;

        if (!fence)
                return -EINVAL;

//        if (!ktime_to_ns(fence->timestamp)) {
//                fence->timestamp = ktime_get();
//                smp_mb__before_atomic();
//        }

        if (test_and_set_bit(FENCE_FLAG_SIGNALED_BIT, &fence->flags))
                return -EINVAL;

//        trace_fence_signaled(fence);

        if (test_bit(FENCE_FLAG_ENABLE_SIGNAL_BIT, &fence->flags)) {
                struct fence_cb *cur, *tmp;

                spin_lock_irqsave(fence->lock, flags);
                list_for_each_entry_safe(cur, tmp, &fence->cb_list, node) {
                        list_del_init(&cur->node);
                        cur->func(fence, cur);
                }
                spin_unlock_irqrestore(fence->lock, flags);
        }
        return 0;
}
EXPORT_SYMBOL(fence_signal);

int fence_signal_locked(struct fence *fence)
{
        struct fence_cb *cur, *tmp;
        int ret = 0;

        if (WARN_ON(!fence))
                return -EINVAL;

//        if (!ktime_to_ns(fence->timestamp)) {
//                fence->timestamp = ktime_get();
//                smp_mb__before_atomic();
//        }

        if (test_and_set_bit(FENCE_FLAG_SIGNALED_BIT, &fence->flags)) {
                ret = -EINVAL;

                /*
                 * we might have raced with the unlocked fence_signal,
                 * still run through all callbacks
                 */
        }// else
//                trace_fence_signaled(fence);

        list_for_each_entry_safe(cur, tmp, &fence->cb_list, node) {
                list_del_init(&cur->node);
                cur->func(fence, cur);
        }
        return ret;
}
EXPORT_SYMBOL(fence_signal_locked);


void fence_enable_sw_signaling(struct fence *fence)
{
        unsigned long flags;

        if (!test_and_set_bit(FENCE_FLAG_ENABLE_SIGNAL_BIT, &fence->flags) &&
            !test_bit(FENCE_FLAG_SIGNALED_BIT, &fence->flags)) {
//                trace_fence_enable_signal(fence);

                spin_lock_irqsave(fence->lock, flags);

                if (!fence->ops->enable_signaling(fence))
                        fence_signal_locked(fence);

                spin_unlock_irqrestore(fence->lock, flags);
        }
}
EXPORT_SYMBOL(fence_enable_sw_signaling);



signed long
fence_wait_timeout(struct fence *fence, bool intr, signed long timeout)
{
        signed long ret;

        if (WARN_ON(timeout < 0))
                return -EINVAL;

//        trace_fence_wait_start(fence);
        ret = fence->ops->wait(fence, intr, timeout);
//        trace_fence_wait_end(fence);
        return ret;
}
EXPORT_SYMBOL(fence_wait_timeout);

void fence_release(struct kref *kref)
{
        struct fence *fence =
                        container_of(kref, struct fence, refcount);

//        trace_fence_destroy(fence);

        BUG_ON(!list_empty(&fence->cb_list));

        if (fence->ops->release)
                fence->ops->release(fence);
        else
                fence_free(fence);
}
EXPORT_SYMBOL(fence_release);

void fence_free(struct fence *fence)
{
        kfree_rcu(fence, rcu);
}
EXPORT_SYMBOL(fence_free);


reservation_object_add_shared_inplace(struct reservation_object *obj,
                                      struct reservation_object_list *fobj,
                                      struct fence *fence)
{
        u32 i;

        fence_get(fence);

//        preempt_disable();
        write_seqcount_begin(&obj->seq);

        for (i = 0; i < fobj->shared_count; ++i) {
                struct fence *old_fence;

                old_fence = rcu_dereference_protected(fobj->shared[i],
                                                reservation_object_held(obj));

                if (old_fence->context == fence->context) {
                        /* memory barrier is added by write_seqcount_begin */
                        RCU_INIT_POINTER(fobj->shared[i], fence);
                        write_seqcount_end(&obj->seq);
                        preempt_enable();

                        fence_put(old_fence);
                        return;
                }
        }

        /*
         * memory barrier is added by write_seqcount_begin,
         * fobj->shared_count is protected by this lock too
         */
        RCU_INIT_POINTER(fobj->shared[fobj->shared_count], fence);
        fobj->shared_count++;

        write_seqcount_end(&obj->seq);
//        preempt_enable();
}



static void
reservation_object_add_shared_replace(struct reservation_object *obj,
                                      struct reservation_object_list *old,
                                      struct reservation_object_list *fobj,
                                      struct fence *fence)
{
        unsigned i;
        struct fence *old_fence = NULL;

        fence_get(fence);

        if (!old) {
                RCU_INIT_POINTER(fobj->shared[0], fence);
                fobj->shared_count = 1;
                goto done;
        }

        /*
         * no need to bump fence refcounts, rcu_read access
         * requires the use of kref_get_unless_zero, and the
         * references from the old struct are carried over to
         * the new.
         */
        fobj->shared_count = old->shared_count;

        for (i = 0; i < old->shared_count; ++i) {
                struct fence *check;

                check = rcu_dereference_protected(old->shared[i],
                                                reservation_object_held(obj));

                if (!old_fence && check->context == fence->context) {
                        old_fence = check;
                        RCU_INIT_POINTER(fobj->shared[i], fence);
                } else
                        RCU_INIT_POINTER(fobj->shared[i], check);
        }
        if (!old_fence) {
                RCU_INIT_POINTER(fobj->shared[fobj->shared_count], fence);
                fobj->shared_count++;
        }

done:
//        preempt_disable();
        write_seqcount_begin(&obj->seq);
        /*
         * RCU_INIT_POINTER can be used here,
         * seqcount provides the necessary barriers
         */
        RCU_INIT_POINTER(obj->fence, fobj);
        write_seqcount_end(&obj->seq);
//        preempt_enable();

        if (old)
                kfree_rcu(old, rcu);

        if (old_fence)
                fence_put(old_fence);
}


int reservation_object_reserve_shared(struct reservation_object *obj)
{
        struct reservation_object_list *fobj, *old;
        u32 max;

        old = reservation_object_get_list(obj);

        if (old && old->shared_max) {
                if (old->shared_count < old->shared_max) {
                        /* perform an in-place update */
                        kfree(obj->staged);
                        obj->staged = NULL;
                        return 0;
                } else
                        max = old->shared_max * 2;
        } else
                max = 4;

        /*
         * resize obj->staged or allocate if it doesn't exist,
         * noop if already correct size
         */
        fobj = krealloc(obj->staged, offsetof(typeof(*fobj), shared[max]),
                        GFP_KERNEL);
        if (!fobj)
                return -ENOMEM;

        obj->staged = fobj;
        fobj->shared_max = max;
        return 0;
}
EXPORT_SYMBOL(reservation_object_reserve_shared);

void reservation_object_add_shared_fence(struct reservation_object *obj,
                                         struct fence *fence)
{
        struct reservation_object_list *old, *fobj = obj->staged;

        old = reservation_object_get_list(obj);
        obj->staged = NULL;

        if (!fobj) {
                BUG_ON(old->shared_count >= old->shared_max);
                reservation_object_add_shared_inplace(obj, old, fence);
        } else
                reservation_object_add_shared_replace(obj, old, fobj, fence);
}
EXPORT_SYMBOL(reservation_object_add_shared_fence);


void reservation_object_add_excl_fence(struct reservation_object *obj,
                                       struct fence *fence)
{
        struct fence *old_fence = reservation_object_get_excl(obj);
        struct reservation_object_list *old;
        u32 i = 0;

        old = reservation_object_get_list(obj);
        if (old)
                i = old->shared_count;

        if (fence)
                fence_get(fence);

//        preempt_disable();
        write_seqcount_begin(&obj->seq);
        /* write_seqcount_begin provides the necessary memory barrier */
        RCU_INIT_POINTER(obj->fence_excl, fence);
        if (old)
                old->shared_count = 0;
        write_seqcount_end(&obj->seq);
//        preempt_enable();

        /* inplace update, no shared fences */
        while (i--)
                fence_put(rcu_dereference_protected(old->shared[i],
                                                reservation_object_held(obj)));

        if (old_fence)
                fence_put(old_fence);
}
EXPORT_SYMBOL(reservation_object_add_excl_fence);

void
fence_init(struct fence *fence, const struct fence_ops *ops,
             spinlock_t *lock, unsigned context, unsigned seqno)
{
        BUG_ON(!lock);
        BUG_ON(!ops || !ops->wait || !ops->enable_signaling ||
               !ops->get_driver_name || !ops->get_timeline_name);

        kref_init(&fence->refcount);
        fence->ops = ops;
        INIT_LIST_HEAD(&fence->cb_list);
        fence->lock = lock;
        fence->context = context;
        fence->seqno = seqno;
        fence->flags = 0UL;

//        trace_fence_init(fence);
}
EXPORT_SYMBOL(fence_init);


#include <linux/rcupdate.h>

struct rcu_ctrlblk {
        struct rcu_head *rcucblist;     /* List of pending callbacks (CBs). */
        struct rcu_head **donetail;     /* ->next pointer of last "done" CB. */
        struct rcu_head **curtail;      /* ->next pointer of last CB. */
//        RCU_TRACE(long qlen);           /* Number of pending CBs. */
//        RCU_TRACE(unsigned long gp_start); /* Start time for stalls. */
//        RCU_TRACE(unsigned long ticks_this_gp); /* Statistic for stalls. */
//        RCU_TRACE(unsigned long jiffies_stall); /* Jiffies at next stall. */
//        RCU_TRACE(const char *name);    /* Name of RCU type. */
};

/* Definition for rcupdate control block. */
static struct rcu_ctrlblk rcu_sched_ctrlblk = {
        .donetail       = &rcu_sched_ctrlblk.rcucblist,
        .curtail        = &rcu_sched_ctrlblk.rcucblist,
//        RCU_TRACE(.name = "rcu_sched")
};

static void __call_rcu(struct rcu_head *head,
                       void (*func)(struct rcu_head *rcu),
                       struct rcu_ctrlblk *rcp)
{
        unsigned long flags;

//        debug_rcu_head_queue(head);
        head->func = func;
        head->next = NULL;

        local_irq_save(flags);
        *rcp->curtail = head;
        rcp->curtail = &head->next;
//        RCU_TRACE(rcp->qlen++);
        local_irq_restore(flags);
}

/*
 * Post an RCU callback to be invoked after the end of an RCU-sched grace
 * period.  But since we have but one CPU, that would be after any
 * quiescent state.
 */
void call_rcu_sched(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
        __call_rcu(head, func, &rcu_sched_ctrlblk);
}

fb_get_options(const char *name, char **option)
{
    return 1;

}

ktime_t ktime_get(void)
{
    ktime_t t;

    t.tv64 = GetClockNs();

    return t;
}

void radeon_cursor_reset(struct drm_crtc *crtc)
{

}

/* Greatest common divisor */
unsigned long gcd(unsigned long a, unsigned long b)
{
        unsigned long r;

        if (a < b)
                swap(a, b);

        if (!b)
                return a;
        while ((r = a % b) != 0) {
                a = b;
                b = r;
        }
        return b;
}

void vfree(const void *addr)
{
    KernelFree(addr);
}


int set_memory_uc(unsigned long addr, int numpages)
{
    return 0;
};

int set_memory_wb(unsigned long addr, int numpages)
{
    return 0;
};

char *strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *copy = __builtin_malloc(len);
    if (copy)
    {
        memcpy (copy, str, len);
    }
    return copy;
}

