
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>


#define  MD_FREE    1
#define  MD_USED    2

typedef struct {
    u32_t  av_mapped;
    u32_t  av_unmapped;

    link_t mapped[32];
    link_t unmapped[32];

    link_t used;

    SPINLOCK_DECLARE(lock);   /**< this lock protects everything below */
}heap_t;


slab_cache_t *md_slab;
slab_cache_t *phm_slab;


heap_t        lheap;
heap_t        sheap;


static inline void _set_lavu(count_t idx)
{ asm volatile ("bts %0, _lheap+4"::"r"(idx):"cc"); }

static inline void _reset_lavu(count_t idx)
{ asm volatile ("btr %0, _lheap+4"::"r"(idx):"cc"); }

static inline void _set_savm(count_t idx)
{ asm volatile ("bts %0, _sheap"::"r"(idx):"cc"); }

static inline void _reset_savm(count_t idx)
{ asm volatile ("btr %0, _sheap"::"r"(idx):"cc"); }

static inline void _set_savu(count_t idx)
{ asm volatile ("bts %0, _sheap+4"::"r"(idx):"cc"); }

static inline void _reset_savu(count_t idx)
{ asm volatile ("btr %0, _sheap+4"::"r"(idx):"cc"); }


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
        list_initialize(&lheap.mapped[i]);
        list_initialize(&lheap.unmapped[i]);

        list_initialize(&sheap.mapped[i]);
        list_initialize(&sheap.unmapped[i]);
    };

    list_initialize(&lheap.used);
    list_initialize(&sheap.used);

    md_slab = slab_cache_create(sizeof(md_t), 16,NULL,NULL,SLAB_CACHE_MAGDEFERRED);

    md = (md_t*)slab_alloc(md_slab,0);

    list_initialize(&md->adj);
    md->base = base;
    md->size = size;
    md->parent = NULL;
    md->state = MD_FREE;

    list_prepend(&md->link, &lheap.unmapped[31]);
    lheap.av_mapped    = 0x00000000;
    lheap.av_unmapped  = 0x80000000;
    sheap.av_mapped    = 0x00000000;
    sheap.av_unmapped  = 0x00000000;

    return 1;
};

md_t* __fastcall find_large_md(size_t size)
{
    md_t *md = NULL;

    count_t idx0;
    u32_t mask;

    ASSERT((size & 0x3FFFFF) == 0);

    idx0 = (size>>22) - 1 < 32 ? (size>>22) - 1 : 31;
    mask = lheap.av_unmapped & ( -1<<idx0 );

    if(mask)
    {
        if(idx0 == 31)
        {
            md_t *tmp = (md_t*)lheap.unmapped[31].next;
            while(&tmp->link != &lheap.unmapped[31])
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

            ASSERT( !list_empty(&lheap.unmapped[idx0]))

            md = (md_t*)lheap.unmapped[idx0].next;
        };
    }
    else
        return NULL;

    ASSERT(md->state == MD_FREE);

    list_remove((link_t*)md);
    if(list_empty(&lheap.unmapped[idx0]))
        _reset_lavu(idx0);

    if(md->size > size)
    {
        count_t idx1;
        md_t *new_md = (md_t*)slab_alloc(md_slab,0);         /* FIXME check */

        link_initialize(&new_md->link);
        list_insert(&new_md->adj, &md->adj);

        new_md->base   = md->base;
        new_md->size   = size;
        new_md->parent = NULL;
        new_md->state  = MD_USED;

        md->base+= size;
        md->size-= size;

        idx1 = (md->size>>22) - 1 < 32 ? (md->size>>22) - 1 : 31;

        list_prepend(&md->link, &lheap.unmapped[idx1]);
        _set_lavu(idx1);

        return new_md;
    };
    md->state = MD_USED;

    return md;
}

md_t* __fastcall find_unmapped_md(size_t size)
{
    eflags_t efl;

    md_t *md = NULL;

    count_t idx0;
    u32_t mask;

    ASSERT((size & 0xFFF) == 0);

    efl = safe_cli();

    idx0 = (size>>12) - 1 < 32 ? (size>>12) - 1 : 31;
    mask = sheap.av_unmapped & ( -1<<idx0 );

    DBG("smask %x size %x idx0 %x mask %x\n",sheap.av_unmapped, size, idx0, mask);

    if(mask)
    {
        if(idx0 == 31)
        {
            ASSERT( !list_empty(&sheap.unmapped[31]));

            md_t *tmp = (md_t*)sheap.unmapped[31].next;
            while( &tmp->link != &sheap.unmapped[31])
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

            ASSERT( !list_empty(&sheap.unmapped[idx0]));

            md = (md_t*)sheap.unmapped[idx0].next;
        }
    };

    if(md)
    {
        DBG("remove md %x\n", md);

        ASSERT(md->state==MD_FREE);
        ASSERT(md->parent != NULL);

        list_remove((link_t*)md);
        if(list_empty(&sheap.unmapped[idx0]))
            _reset_savu(idx0);
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

        ASSERT(lmd->size != 0);
        ASSERT(lmd->base != 0);
        ASSERT((lmd->base & 0x3FFFFF) == 0);
        ASSERT(lmd->parent == NULL);

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
          list_prepend(&md->link, &sheap.unmapped[idx1]);
        else
        {
            if( list_empty(&sheap.unmapped[31]))
                list_prepend(&md->link, &sheap.unmapped[31]);
            else
            {
                md_t *tmp = (md_t*)sheap.unmapped[31].next;

                while( &tmp->link != &sheap.unmapped[31])
                {
                    if(md->base < tmp->base)
                        break;
                    tmp = (md_t*)tmp->link.next;
                }
                list_insert(&md->link, &tmp->link);
            };
        };

        _set_savu(idx1);

        safe_sti(efl);

        return new_md;
    };

    md->state = MD_USED;

    safe_sti(efl);

    return md;
}

md_t* __fastcall find_mapped_md(size_t size)
{
    eflags_t efl;

    md_t *md = NULL;

    count_t idx0;
    u32_t mask;

    ASSERT((size & 0xFFF) == 0);

    efl = safe_cli();

    idx0 = (size>>12) - 1 < 32 ? (size>>12) - 1 : 31;
    mask = sheap.av_mapped & ( -1<<idx0 );

    DBG("small av_mapped %x size %x idx0 %x mask %x\n",sheap.av_mapped, size,
         idx0, mask);

    if(mask)
    {
        if(idx0 == 31)
        {
            ASSERT( !list_empty(&sheap.mapped[31]));

            md_t *tmp = (md_t*)sheap.mapped[31].next;
            while( &tmp->link != &sheap.mapped[31])
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

            ASSERT( !list_empty(&sheap.mapped[idx0]));

            md = (md_t*)sheap.mapped[idx0].next;
        }
    };

    if(md)
    {
        DBG("remove md %x\n", md);

        ASSERT(md->state==MD_FREE);

        list_remove((link_t*)md);
        if(list_empty(&sheap.mapped[idx0]))
            _reset_savm(idx0);
    }
    else
    {
        md_t    *lmd;
        addr_t  frame;
        addr_t  *pte;
        int i;

        lmd = find_large_md((size+0x3FFFFF)&~0x3FFFFF);

        DBG("get large md %x\n", lmd);

        if( !lmd)
        {
            safe_sti(efl);
            return NULL;
        };

        ASSERT(lmd->size != 0);
        ASSERT(lmd->base != 0);
        ASSERT((lmd->base & 0x3FFFFF) == 0);
        ASSERT(lmd->parent == NULL);

        frame = core_alloc(10);                        /* FIXME check */

        lmd->parent = (void*)frame;

        pte = &((addr_t*)page_tabs)[lmd->base>>12];    /* FIXME remove */

        for(i = 0; i<1024; i++)
        {
           *pte++ = frame;
           frame+= 4096;
        }

        md = (md_t*)slab_alloc(md_slab,0);             /* FIXME check */

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

        md->base+= size;
        md->size-= size;
        md->state = MD_FREE;

        idx1 = (md->size>>12) - 1 < 32 ? (md->size>>12) - 1 : 31;

        DBG("insert md %x, base %x size %x idx %x\n", md,md->base, md->size,idx1);

        if( idx1 < 31)
          list_prepend(&md->link, &sheap.mapped[idx1]);
        else
        {
            if( list_empty(&sheap.mapped[31]))
                list_prepend(&md->link, &sheap.mapped[31]);
            else
            {
                md_t *tmp = (md_t*)sheap.mapped[31].next;

                while( &tmp->link != &sheap.mapped[31])
                {
                    if(md->base < tmp->base)
                        break;
                    tmp = (md_t*)tmp->link.next;
                }
                list_insert(&md->link, &tmp->link);
            };
        };

        _set_savm(idx1);

        md = new_md;
    };

    md->state = MD_USED;

    safe_sti(efl);

    return md;
}

void __fastcall free_unmapped_md(md_t *md)
{
    eflags_t  efl ;
    md_t     *fd;
    md_t     *bk;
    count_t   idx;

    ASSERT(md->parent != NULL);

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
            if(list_empty(&sheap.unmapped[idx]))
                _reset_savu(idx);

            md->size+= fd->size;
            md->adj.next = fd->adj.next;
            md->adj.next->prev = (link_t*)md;
            slab_free(md_slab, fd);
        };
        if(bk->state == MD_FREE)
        {
            idx = (bk->size>>12) - 1 < 32 ? (bk->size>>12) - 1 : 31;

            list_remove((link_t*)bk);
            if(list_empty(&sheap.unmapped[idx]))
                _reset_savu(idx);

            bk->size+= md->size;
            bk->adj.next = md->adj.next;
            bk->adj.next->prev = (link_t*)bk;
            slab_free(md_slab, md);
            md = fd;
        };
    };

    md->state = MD_FREE;

    idx = (md->size>>12) - 1 < 32 ? (md->size>>12) - 1 : 31;

    _set_savu(idx);

    if( idx < 31)
        list_prepend(&md->link, &sheap.unmapped[idx]);
    else
    {
        if( list_empty(&sheap.unmapped[31]))
            list_prepend(&md->link, &sheap.unmapped[31]);
        else
        {
            md_t *tmp = (md_t*)sheap.unmapped[31].next;

            while( &tmp->link != &sheap.unmapped[31])
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

void __fastcall free_mapped_md(md_t *md)
{
    eflags_t  efl ;
    md_t     *fd;
    md_t     *bk;
    count_t   idx;

    ASSERT(md->parent != NULL);
    ASSERT( ((md_t*)(md->parent))->parent != NULL);

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
            if(list_empty(&sheap.mapped[idx]))
                _reset_savm(idx);

            md->size+= fd->size;
            md->adj.next = fd->adj.next;
            md->adj.next->prev = (link_t*)md;
            slab_free(md_slab, fd);
        };
        if(bk->state == MD_FREE)
        {
            idx = (bk->size>>12) - 1 < 32 ? (bk->size>>12) - 1 : 31;

            list_remove((link_t*)bk);
            if(list_empty(&sheap.mapped[idx]))
                _reset_savm(idx);

            bk->size+= md->size;
            bk->adj.next = md->adj.next;
            bk->adj.next->prev = (link_t*)bk;
            slab_free(md_slab, md);
            md = fd;
        };
    };

    md->state = MD_FREE;

    idx = (md->size>>12) - 1 < 32 ? (md->size>>12) - 1 : 31;

    _set_savm(idx);

    if( idx < 31)
        list_prepend(&md->link, &sheap.mapped[idx]);
    else
    {
        if( list_empty(&sheap.mapped[31]))
            list_prepend(&md->link, &sheap.mapped[31]);
        else
        {
            md_t *tmp = (md_t*)sheap.mapped[31].next;

            while( &tmp->link != &sheap.mapped[31])
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


md_t* __fastcall  md_alloc(size_t size, u32_t flags)
{
    eflags_t efl;

    md_t *md;

    size = (size+4095)&~4095;

    if( flags & PG_MAP )
    {
        md = find_mapped_md(size);

        if( !md )
            return NULL;

        ASSERT(md->state == MD_USED);
        ASSERT(md->parent != NULL);

        md_t *lmd = (md_t*)md->parent;

        ASSERT( lmd != NULL);
        ASSERT( lmd->parent != NULL);

        addr_t  frame  = (md->base - lmd->base + (addr_t)lmd->parent)|
                         (flags & 0xFFF);
        DBG("frame %x\n", frame);
        ASSERT(frame != 0);

        count_t  tmp = size >> 12;
        addr_t  *pte = &((addr_t*)page_tabs)[md->base>>12];

        while(tmp--)
        {
            *pte++ = frame;
            frame+= 4096;
        };
    }
    else
    {
        md = find_unmapped_md(size);
        if( !md )
            return NULL;

        ASSERT(md->parent != NULL);
        ASSERT(md->state == MD_USED);
    }

    return md;
};


void __fastcall md_free(md_t *md)
{

    if( md )
    {
        md_t *lmd;

        DBG("free md: %x base: %x size: %x\n",md, md->base, md->size);

        ASSERT(md->state == MD_USED);

        list_remove((link_t*)md);

        lmd = (md_t*)md->parent;

        ASSERT(lmd != 0);

        if(lmd->parent != 0)
        {
            addr_t   mem = md->base;
            addr_t  *pte = &((addr_t*)page_tabs)[md->base>>12];
            count_t  tmp  = md->size >> 12;

            while(tmp--)
            {
                *pte++ = 0;
                asm volatile ( "invlpg (%0)" ::"r" (mem) );
                mem+= 4096;
            };
            free_mapped_md( md );
        }
        else
            free_unmapped_md( md );
    }

    return;
};

void * __fastcall mem_alloc(size_t size, u32_t flags)
{
    eflags_t efl;

    md_t *md;

    DBG("\nmem_alloc: %x bytes\n", size);

    ASSERT(size != 0);

    md = md_alloc(size, flags);

    if( !md )
        return NULL;

    efl = safe_cli();
    spinlock_lock(&sheap.lock);

    if( list_empty(&sheap.used) )
        list_prepend(&md->link, &sheap.used);
    else
    {
        md_t *tmp = (md_t*)sheap.used.next;

        while( &tmp->link != &sheap.used)
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

    while( &tmp->link != &sheap.used)
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
        md_free( md );

    }
    else
        DBG("\tERROR: invalid base address: %x\n", mem);

    safe_sti(efl);
};
