#ifndef __LBSYNC_H__
#define __LBSYNC_H__

typedef struct
{
    volatile int lock;
    unsigned int handle;
}mutex_t;

int __fastcall mutex_init(mutex_t *mutex);
int __fastcall mutex_destroy(mutex_t *mutex);
void __fastcall mutex_lock(mutex_t *mutex);
int __fastcall mutex_trylock (mutex_t *mutex);
void __fastcall mutex_unlock(mutex_t *mutex);

#endif
