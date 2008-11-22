
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>

addr_t thr_ptr;

slab_cache_t *thr_slab;

extern addr_t sys_pdbr;

void init_threads()
{
    thr_slab = slab_cache_create(sizeof(thr_t), 16,
               NULL,NULL,SLAB_CACHE_MAGDEFERRED);
};


thr_t* __fastcall create_systhread(addr_t entry_ptr)
{
    static count_t  thr_cnt = 0;
    static count_t  slot = 1;

    thr_t   *thr;
    addr_t   thr_stack;

    thr = (thr_t*)slab_alloc(thr_slab,0);
    thr_stack = PA2KA(core_alloc(1));

    thr_cnt++;

    thr->eax = (thr_cnt<<8)|slot;
    thr->tid = (thr_cnt<<8)|slot;

    thr->slot = slot;

    slot++;

    thr->pdir = KA2PA(&sys_pdbr);

    thr->ebx = 0;

    thr->edi = 0;
    thr->esi = 0;
    thr->ebp = 0;
    thr->edx = 0;
    thr->ecx = 0;

    thr->cs  = sel_srv_code;
    thr->eflags = EFL_IOPL1;
    thr->esp = thr_stack + 8192;
    thr->ss = sel_srv_stack;

    thr->thr_flags    = 0;

    thr->ticks_left   = 8;
    thr->quantum_size = 8;

    thr->eip = entry_ptr;

    //lock_enqueue(thr_ptr);       /* add to scheduling queues */

    return thr;
};

