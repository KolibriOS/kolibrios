#include <ddk.h>
#include <linux/mm.h>
#include <syscall.h>

#define KMAP_MAX    256

static struct mutex kmap_mutex;
static struct page* kmap_table[KMAP_MAX];
static int kmap_av;
static int kmap_first;
static void* kmap_base;

int kmap_init()
{
    kmap_base = AllocKernelSpace(KMAP_MAX*4096);
    if(kmap_base == NULL)
        return -1;

    kmap_av = KMAP_MAX;
    MutexInit(&kmap_mutex);
    return 0;
};

void *kmap(struct page *page)
{
    void *vaddr = NULL;
    int i;

    do
    {
        MutexLock(&kmap_mutex);
        if(kmap_av != 0)
        {
            for(i = kmap_first; i < KMAP_MAX; i++)
            {
                if(kmap_table[i] == NULL)
                {
                    kmap_av--;
                    kmap_first = i;
                    kmap_table[i] = page;
                    vaddr = kmap_base + (i<<12);
                    MapPage(vaddr,(addr_t)page,3);
                    break;
                };
            };
        };
        MutexUnlock(&kmap_mutex);
    }while(vaddr == NULL);

    return vaddr;
};

void *kmap_atomic(struct page *page) __attribute__ ((alias ("kmap")));

void kunmap(struct page *page)
{
    void *vaddr;
    int   i;

    MutexLock(&kmap_mutex);

    for(i = 0; i < KMAP_MAX; i++)
    {
        if(kmap_table[i] == page)
        {
            kmap_av++;
            if(i < kmap_first)
                kmap_first = i;
            kmap_table[i] = NULL;
            vaddr = kmap_base + (i<<12);
            MapPage(vaddr,0,0);
            break;
        };
    };

    MutexUnlock(&kmap_mutex);
};

void kunmap_atomic(void *vaddr)
{
    int i;

    MapPage(vaddr,0,0);

    i = (vaddr - kmap_base) >> 12;

    MutexLock(&kmap_mutex);

    kmap_av++;
    if(i < kmap_first)
        kmap_first = i;
    kmap_table[i] = NULL;

    MutexUnlock(&kmap_mutex);
}

