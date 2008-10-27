
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>

typedef struct
{
   link_t link;
   link_t adj;
   addr_t base;
   size_t size;
   void*  parent;
   u32_t  state;
}md_t;

#define   MD_FREE    1
#define   MD_USED    2

typedef struct {
    SPINLOCK_DECLARE(lock);   /**< this lock protects everything below */

    u32_t  availmask;
    link_t free[32];

    link_t used;
}heap_t;


slab_cache_t *md_slab;
slab_cache_t *phm_slab;


heap_t        lheap;
heap_t        sheap;



static inline void _set_lmask(count_t idx)
{ asm volatile ("bts %0, _lheap"::"r"(idx):"cc"); }

static inline void _reset_lmask(count_t idx)
{ asm volatile ("btr %0, _lheap"::"r"(idx):"cc"); }

static inline void _set_smask(count_t idx)
{ asm volatile ("bts %0, _sheap"::"r"(idx):"cc"); }

static inline void _reset_smask(count_t idx)
{ asm volatile ("btr %0, _sheap"::"r"(idx):"cc"); }


int __fastcall init_heap(addr_t base, size_t size)
{
   md_t *md;
   u32_t i;

   ASSERT(base != 0);
   ASSERT(size != 0)
   ASSERT((base & 0x3FFFFF) == 0);
   ASSERT((size & 0x3FFFFF) == 0);

   for (i = 0; i < 32; i++)
   {
     list_initialize(&lheap.free[i]);
     list_initialize(&sheap.free[i]);
   };

   list_initialize(&lheap.used);
   list_initialize(&sheap.used);


   md_slab = slab_cache_create(sizeof(md_t), 32,NULL,NULL,SLAB_CACHE_MAGDEFERRED);

   md = (md_t*)slab_alloc(md_slab,0);

   list_initialize(&md->adj);
   md->base = base;
   md->size = size;
   md->parent = NULL;
   md->state = MD_FREE;

   list_prepend(&md->link, &lheap.free[31]);
   lheap.availmask = 0x80000000;
   sheap.availmask = 0x00000000;

  // phm_slab = slab_cache_create(sizeof(phismem_t), 32,NULL,NULL,SLAB_CACHE_MAGDEFERRED);

   return 1;
};

md_t* __fastcall find_large_md(size_t size)
{
   md_t *md = NULL;

   count_t idx0;
   u32_t mask;

   ASSERT((size & 0x3FFFFF) == 0);

   idx0 = (size>>22) - 1 < 32 ? (size>>22) - 1 : 31;
   mask = lheap.availmask & ( -1<<idx0 );

   if(mask)
   {
     if(idx0 == 31)
     {
        md_t *tmp = (md_t*)lheap.free[31].next;
        while((link_t*)tmp != &lheap.free[31])
        {
          if(tmp->size >= size)
          {
            DBG("remove large tmp %x\n", tmp);

            md = tmp;
            break;
          };
        };
        tmp = (md_t*)tmp->link.next;
     }
     else
     {
       idx0 = _bsf(mask);

       ASSERT( !list_empty(&lheap.free[idx0]))

       md = (md_t*)lheap.free[idx0].next;
     };
   }
   else
     return NULL;

   ASSERT(md->state == MD_FREE);

   list_remove((link_t*)md);
   if(list_empty(&lheap.free[idx0]))
     _reset_lmask(idx0);

   if(md->size > size)
   {
     count_t idx1;
     md_t *new_md = (md_t*)slab_alloc(md_slab,0);         /* FIXME check */

     link_initialize(&new_md->link);
     list_insert(&new_md->adj, &md->adj);

     new_md->base = md->base;
     new_md->size = size;
     new_md->state = MD_USED;

     md->base+= size;
     md->size-= size;

     idx1 = (md->size>>22) - 1 < 32 ? (md->size>>22) - 1 : 31;

     list_prepend(&md->link, &lheap.free[idx1]);
     _set_lmask(idx1);

     return new_md;
   };
   md->state = MD_USED;

   return md;
}

md_t* __fastcall find_small_md(size_t size)
{
    eflags_t efl;

    md_t *md = NULL;

    count_t idx0;
    u32_t mask;

    ASSERT((size & 0xFFF) == 0);

    efl = safe_cli();

    idx0 = (size>>12) - 1 < 32 ? (size>>12) - 1 : 31;
    mask = sheap.availmask & ( -1<<idx0 );

    DBG("smask %x size %x idx0 %x mask %x\n",sheap.availmask, size, idx0, mask);

    if(mask)
    {
        if(idx0 == 31)
        {
            ASSERT( !list_empty(&sheap.free[31]));

            md_t *tmp = (md_t*)sheap.free[31].next;
            while((link_t*)tmp != &sheap.free[31])
            {
                if(tmp->size >= size)
                {
                    md = tmp;
                    break;
                };
                tmp = (md_t*)tmp->link.next;
            };
        }
        else
        {
            idx0 = _bsf(mask);

            ASSERT( !list_empty(&sheap.free[idx0]));

            md = (md_t*)sheap.free[idx0].next;
        }
    };

    if(md)
    {
        DBG("remove md %x\n", md);

        ASSERT(md->state==MD_FREE);

        list_remove((link_t*)md);
        if(list_empty(&sheap.free[idx0]))
            _reset_smask(idx0);
    }
    else
    {
        md_t *lmd;
        lmd = find_large_md((size+0x3FFFFF)&~0x3FFFFF);

        DBG("get large md %x\n", lmd);

        if( !lmd)
        {
            safe_sti(efl);
            return NULL;
        };

        md = (md_t*)slab_alloc(md_slab,0);    /* FIXME check */

        link_initialize(&md->link);
        list_initialize(&md->adj);
        md->base = lmd->base;
        md->size = lmd->size;
        md->parent  = lmd;
        md->state = MD_USED;
    };

    if(md->size > size)
    {
        count_t idx1;
        md_t *new_md = (md_t*)slab_alloc(md_slab,0);    /* FIXME check */

        link_initialize(&new_md->link);
        list_insert(&new_md->adj, &md->adj);

        new_md->base = md->base;
        new_md->size = size;
        new_md->parent = md->parent;
        new_md->state = MD_USED;

        md->base+= size;
        md->size-= size;
        md->state = MD_FREE;

        idx1 = (md->size>>12) - 1 < 32 ? (md->size>>12) - 1 : 31;

        DBG("insert md %x, base %x size %x idx %x\n", md,md->base, md->size,idx1);

        if( idx1 < 31)
          list_prepend(&md->link, &sheap.free[idx1]);
        else
        {
            if( list_empty(&sheap.free[31]))
                list_prepend(&md->link, &sheap.free[31]);
            else
            {
                md_t *tmp = (md_t*)sheap.free[31].next;

                while((link_t*)tmp != &sheap.free[31])
                {
                    if(md->base < tmp->base)
                        break;
                    tmp = (md_t*)tmp->link.next;
                }
                list_insert(&md->link, &tmp->link);
            };
        };

        _set_smask(idx1);

        safe_sti(efl);

        return new_md;
    };

    md->state = MD_USED;

    safe_sti(efl);

    return md;
}

void __fastcall free_small_md(md_t *md)
{
    eflags_t  efl ;
    md_t     *fd;
    md_t     *bk;
    count_t   idx;

    efl = safe_cli();
    spinlock_lock(&sheap.lock);

    if( !list_empty(&md->adj))
    {
        bk = (md_t*)md->adj.prev;
        fd = (md_t*)md->adj.next;

        if(fd->state == MD_FREE)
        {
            idx = (fd->size>>12) - 1 < 32 ? (fd->size>>12) - 1 : 31;

            list_remove((link_t*)fd);
            if(list_empty(&sheap.free[idx]))
                _reset_smask(idx);

            md->size+= fd->size;
            md->adj.next = fd->adj.next;
            md->adj.next->prev = (link_t*)md;
            slab_free(md_slab, fd);
        };
        if(bk->state == MD_FREE)
        {
            idx = (bk->size>>12) - 1 < 32 ? (bk->size>>12) - 1 : 31;

            list_remove((link_t*)bk);
            if(list_empty(&sheap.free[idx]))
                _reset_smask(idx);

            bk->size+= md->size;
            bk->adj.next = md->adj.next;
            bk->adj.next->prev = (link_t*)bk;
            slab_free(md_slab, md);
            md = fd;
        };
    };

    md->state = MD_FREE;

    idx = (md->size>>12) - 1 < 32 ? (md->size>>12) - 1 : 31;

    _set_smask(idx);

    if( idx < 31)
        list_prepend(&md->link, &sheap.free[idx]);
    else
    {
        if( list_empty(&sheap.free[31]))
            list_prepend(&md->link, &sheap.free[31]);
        else
        {
            md_t *tmp = (md_t*)sheap.free[31].next;

            while((link_t*)tmp != &sheap.free[31])
            {
                if(md->base < tmp->base)
                    break;
                tmp = (md_t*)tmp->link.next;
            }
            list_insert(&md->link, &tmp->link);
        };
    };
    spinlock_unlock(&sheap.lock);
    safe_sti(efl);

};


#define page_tabs 0xDF800000

/*
phismem_t* __fastcall phis_alloc(count_t count)
{
   phismem_t *phm;
   count_t tmp;
   phm = (phismem_t*)slab_alloc(phm_slab, 0);

   phm->count = count;
   tmp = count;
   while(tmp)
   {
      u32_t order;

      asm volatile ("bsr %0, %1":"=&r"(order):"r"(tmp):"cc");
      asm volatile ("btr %0, %1" :"=r"(tmp):"r"(order):"cc");

      phm->frames[order] = core_alloc(order);

   };

   return phm;
}

void map_phm(addr_t base, phismem_t *phm, u32_t mapflags)
{
   count_t count;
   addr_t  *pte;

   count = phm->count;
   pte = &((addr_t*)page_tabs)[base>>12];

   while(count)
   {
     u32_t order;
     addr_t frame;
     count_t size;

     asm volatile ("bsr %0, %1":"=&r"(order):"r"(count):"cc");
     asm volatile ("btr %0, %1" :"=r"(count):"r"(order):"cc");

     frame = phm->frames[order] | mapflags;
     size = (1 << order);
     while(size--)
     {
       *pte++ = frame;
       frame+= 4096;
     }
   }
};
*/

void * __fastcall mem_alloc(size_t size, u32_t flags)
{
    eflags_t efl;

    md_t *md;

    DBG("\nmem_alloc: %x bytes\n", size);

    ASSERT(size != 0);

    size = (size+4095)&~4095;

    md = find_small_md(size);

    if( md )
    {
        ASSERT(md->state == MD_USED);

        if( flags & PG_MAP )
        {
            count_t tmp = size >> 12;
            addr_t  *pte = &((addr_t*)page_tabs)[md->base>>12];

            while(tmp)
            {
                u32_t  order;
                addr_t frame;
                size_t size;

                asm volatile ("bsr %1, %0":"=&r"(order):"r"(tmp):"cc");
                asm volatile ("btr %1, %0" :"=r"(tmp):"r"(order):"cc");

                frame = core_alloc(order) | flags;         /* FIXME check */

                size = (1 << order);
                while(size--)
                {
                    *pte++ = frame;
                    frame+= 4096;
                };
            };
        };

        efl = safe_cli();
        spinlock_lock(&sheap.lock);

        if( list_empty(&sheap.used) )
            list_prepend(&md->link, &sheap.used);
        else
        {
            md_t *tmp = (md_t*)sheap.used.next;

            while((link_t*)tmp != &sheap.used)
            {
                if(md->base < tmp->base)
                    break;
                tmp = (md_t*)tmp->link.next;
            }
            list_insert(&md->link, &tmp->link);
        };

        spinlock_unlock(&sheap.lock);
        safe_sti(efl);

        DBG("allocate: %x size %x\n\n",md->base, size);
        return (void*)md->base;
    };
    return NULL;
};

void __fastcall mem_free(void *mem)
{
    eflags_t efl;

    md_t *tmp;
    md_t *md = NULL;

    DBG("mem_free: %x\n",mem);

    ASSERT( mem != 0 );
    ASSERT( ((addr_t)mem & 0xFFF) == 0 );
    ASSERT( ! list_empty(&sheap.used));

    efl = safe_cli();

    tmp = (md_t*)sheap.used.next;

    while((link_t*)tmp != &sheap.used)
    {
        if( tmp->base == (addr_t)mem )
        {
            md = tmp;
            break;
        };
        tmp = (md_t*)tmp->link.next;
    }

    if( md )
    {
        DBG("\tmd: %x base: %x size: %x\n",md, md->base, md->size);

        ASSERT(md->state == MD_USED);

        count_t tmp  = md->size >> 12;
        addr_t  *pte = &((addr_t*)page_tabs)[md->base>>12];

        while(tmp--)
        {
            *pte++ = 0;
            asm volatile (
                "invlpg (%0)"
                :
                :"r" (mem) );
            mem+= 4096;
        };
        list_remove((link_t*)md);
        free_small_md( md );
    }
    else
    {
        DBG("\tERROR: invalid base address: %x\n", mem);
    };

    safe_sti(efl);
};
