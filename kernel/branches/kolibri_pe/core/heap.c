
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
   u32_t  reserved;
}md_t;

typedef struct {
   SPINLOCK_DECLARE(lock);   /**< this lock protects everything below */

   u32_t  availmask;
   link_t list[32];
}heap_t;

slab_cache_t *md_slab;
slab_cache_t *phm_slab;

heap_t lheap;
heap_t sheap;

static inline void _set_lmask(count_t idx)
{ asm volatile ("bts DWORD PTR [_lheap], %0"::"r"(idx):"cc"); }

static inline void _reset_lmask(count_t idx)
{ asm volatile ("btr DWORD PTR [_lheap], %0"::"r"(idx):"cc"); }

static inline void _set_smask(count_t idx)
{ asm volatile ("bts DWORD PTR [_sheap], %0"::"r"(idx):"cc"); }

static inline void _reset_smask(count_t idx)
{ asm volatile ("btr DWORD PTR [_sheap], %0"::"r"(idx):"cc"); }


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
     list_initialize(&lheap.list[i]);
     list_initialize(&sheap.list[i]);
   };

   md_slab = slab_cache_create(sizeof(md_t), 32,NULL,NULL,SLAB_CACHE_MAGDEFERRED);

   md = (md_t*)slab_alloc(md_slab,0);

   list_initialize(&md->adj);
   md->base = base;
   md->size = size;
   md->parent = NULL;
   md->reserved = 0;

   list_prepend(&md->link, &lheap.list[31]);
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
        md_t *tmp = (md_t*)lheap.list[31].next;
        while((link_t*)tmp != &lheap.list[31])
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

       ASSERT( !list_empty(&lheap.list[idx0]))

       md = (md_t*)lheap.list[idx0].next;
     };
   }
   else
     return NULL;

   list_remove((link_t*)md);
   if(list_empty(&lheap.list[idx0]))
     _reset_lmask(idx0);

   if(md->size > size)
   {
     count_t idx1;
     md_t *new_md = (md_t*)slab_alloc(md_slab,0);

     link_initialize(&new_md->link);
     list_insert(&new_md->adj, &md->adj);

     new_md->base = md->base;
     new_md->size = size;

     md->base+= size;
     md->size-= size;

     idx1 = (md->size>>22) - 1 < 32 ? (md->size>>22) - 1 : 31;

     list_prepend(&md->link, &lheap.list[idx1]);
     _set_lmask(idx1);

     return new_md;
   }
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
        md_t *tmp = (md_t*)sheap.list[31].next;
        while((link_t*)tmp != &sheap.list[31])
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
       ASSERT( !list_empty(&sheap.list[idx0]))
       md = (md_t*)sheap.list[idx0].next;
     }
   };

   if(md)
   {
     DBG("remove md %x\n", md);

     list_remove((link_t*)md);
     if(list_empty(&sheap.list[idx0]))
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

     md = (md_t*)slab_alloc(md_slab,0);
     link_initialize(&md->link);
     list_initialize(&md->adj);
     md->base = lmd->base;
     md->size = lmd->size;
     md->parent  = lmd;
     md->reserved = 0;
   };

   if(md->size > size)
   {
     count_t idx1;
     md_t *new_md = (md_t*)slab_alloc(md_slab,0);

     link_initialize(&new_md->link);
     list_insert(&new_md->adj, &md->adj);

     new_md->base = md->base;
     new_md->size = size;
     new_md->parent = md->parent;
     new_md->reserved = 0;

     md->base+= size;
     md->size-= size;

     idx1 = (md->size>>12) - 1 < 32 ? (md->size>>12) - 1 : 31;

     DBG("insert md %x, base %x size %x idx %x\n", md,md->base, md->size,idx1);

     if( idx1 < 31)
       list_prepend(&md->link, &sheap.list[idx1]);
     else
     {
       if( list_empty(&sheap.list[31]))
         list_prepend(&md->link, &sheap.list[31]);
       else
       {
         md_t *tmp = (md_t*)sheap.list[31].next;

         while((link_t*)tmp != &sheap.list[31])
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
   }
   safe_sti(efl);
   return md;
}

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

#define page_tabs 0xDF800000

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

void* __fastcall mem_alloc(size_t size, u32_t flags)
{
   md_t *md;
   phismem_t *phm;

   size = (size+4095)&~4095;

   md = find_small_md(size);
   if( md )
   {
     phm = phis_alloc(size>>12);
     map_phm(md->base , phm, flags);
     return (void*)md->base;
   }
   return NULL;
};

void * __fastcall heap_alloc(size_t size, u32_t flags)
{
   md_t *md;

   size = (size+4095)&~4095;

   md = find_small_md(size);

   if( md )
   {
     if( flags & PG_MAP )
     {
       count_t tmp = size >> 12;
       addr_t  *pte = &((addr_t*)page_tabs)[md->base>>12];

       while(tmp)
       {
         u32_t  order;
         addr_t frame;
         size_t size;

         asm volatile ("bsr %0, %1":"=&r"(order):"r"(tmp):"cc");
         asm volatile ("btr %0, %1" :"=r"(tmp):"r"(order):"cc");

         frame = core_alloc(order) | flags;

         size = (1 << order);
         while(size--)
         {
           *pte++ = frame;
           frame+= 4096;
         };
       };
     };
     DBG("alloc_heap: %x size %x\n\n",md->base, size);
     return (void*)md->base;
   };
   return NULL;
};


