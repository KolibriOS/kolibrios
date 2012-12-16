
#include  <linux/types.h>
#include <syscall.h>
#include "hmm.h"

int init_hmm(struct hmm *mm, u32 count)
{
    u32* data;

    if( mm == NULL)
        return -EINVAL;

    data = malloc(count*sizeof(u32*));
    if( data )
    {
        int i;

        for(i = 0; i < count-1; )
            data[i] = ++i;
        data[i] = 0;

        mm->table = data;
        mm->next  = 0;
        mm->avail = count;
        mm->count = count;

        return 0;
    };
    return -ENOMEM;
};

u32  alloc_handle(struct hmm *mm)
{
    u32 handle = 0;
    u32 ifl;

    ifl = safe_cli();
    if(mm->avail)
    {
        handle = mm->next;
        mm->next = mm->table[handle];
        mm->avail--;
        handle++;
    }
    safe_sti(ifl);

    return handle;
};

int free_handle(struct hmm *mm, u32 handle)
{
    int ret = -1;
    u32 ifl;

    handle--;

    ifl = safe_cli();
    if(handle < mm->count)
    {
        mm->table[handle] = mm->next;
        mm->next = handle;
        mm->avail++;
        ret = 0;
    };
    safe_sti(ifl);

    return ret;
};



