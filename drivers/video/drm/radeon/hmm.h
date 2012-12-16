
#ifndef __HMM_H__
#define __HMM_H__

struct hmm
{
    u32  *table;
    u32   next;
    u32   avail;
    u32   count;
};

int  init_hmm(struct hmm *mm, u32 count);
u32  alloc_handle(struct hmm *mm);
int  free_handle(struct hmm *mm, u32 handle);

#define hmm_get_data(mm, handle)                  \
        ((mm)->table[(handle)-1])

#define hmm_set_data(mm, handle, val)             \
        ((mm)->table[(handle)-1]) = (u32)(val)


#endif /* __HMM_H__ */
