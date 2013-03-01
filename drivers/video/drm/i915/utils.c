#include <ddk.h>
#include <linux/mm.h>
#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
#include "intel_drv.h"


struct file *shmem_file_setup(const char *name, loff_t size, unsigned long flags)
{
    struct file *filep;
    int count;

    filep = malloc(sizeof(*filep));

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

struct page *shmem_read_mapping_page_gfp(struct file *filep,
                                         pgoff_t index, gfp_t gfp)
{
    struct page *page;

//    dbgprintf("%s, file %p index %d\n", __FUNCTION__, filep, index);

    if(unlikely(index >= filep->count))
        return ERR_PTR(-EINVAL);

    page = filep->pages[index];

    if(unlikely(page == NULL))
    {
        page = (struct page *)AllocPage();

        if(unlikely(page == NULL))
            return ERR_PTR(-ENOMEM);

        filep->pages[index] = page;
    };

    return page;
};

unsigned long vm_mmap(struct file *file, unsigned long addr,
         unsigned long len, unsigned long prot,
         unsigned long flag, unsigned long offset)
{
    char *mem, *ptr;
    int i;

    if (unlikely(offset + PAGE_ALIGN(len) < offset))
        return -EINVAL;
    if (unlikely(offset & ~PAGE_MASK))
        return -EINVAL;

    mem = UserAlloc(len);
    if(unlikely(mem == NULL))
        return -ENOMEM;

    for(i = offset, ptr = mem; i < offset+len; i+= 4096, ptr+= 4096)
    {
        struct page *page;

        page = shmem_read_mapping_page_gfp(file, i/PAGE_SIZE,0);

        if (unlikely(IS_ERR(page)))
            goto err;

        MapPage(ptr, (addr_t)page, PG_SHARED|PG_UW);
    }

    return (unsigned long)mem;
err:
    UserFree(mem);
    return -ENOMEM;
};

void shmem_file_delete(struct file *filep)
{
//    printf("%s file %p pages %p count %d\n",
//            __FUNCTION__, filep, filep->pages, filep->count);

    if(filep->pages)
        kfree(filep->pages);
}
