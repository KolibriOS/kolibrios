#include <_ansi.h>
#include <string.h>
#include <reent.h>

void init_global_reent()
{
    struct _reent *ent;

    ent =_GLOBAL_REENT;

    __asm__ __volatile__(
    "movl %0, %%fs:16"
    ::"r"(ent));
}

