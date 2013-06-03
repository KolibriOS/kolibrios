/* default reentrant pointer when multithread enabled */

#include <_ansi.h>
#include <string.h>
#include <reent.h>

static inline
void *user_alloc(int size)
{
    void  *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=eax"(val)
    :"a"(68),"b"(12),"c"(size));
    return val;
}

void init_reent()
{
    struct _reent *ent;

    ent = user_alloc(sizeof(struct _reent));

    _REENT_INIT_PTR(ent);

    __asm__ __volatile__(
    "movl %0, %%fs:12"
    ::"r"(ent));
    __sinit(ent);
}




