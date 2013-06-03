#include <_ansi.h>
#include <string.h>
#include <reent.h>

void init_global_reent()
{
    struct _reent *ent;

    ent =_GLOBAL_REENT;

    _REENT_INIT_PTR(ent);

    __asm__ __volatile__(
    "movl %0, %%fs:12"
    ::"r"(ent));
//    __sinit(ent);
}

