/* default reentrant pointer when multithread enabled */

#include <_ansi.h>
#include <string.h>
#include <reent.h>
#include <kos32sys.h>


void init_reent()
{
    struct _reent *ent;

    ent = user_alloc(sizeof(struct _reent));

    _REENT_INIT_PTR(ent);

    __asm__ __volatile__(
    "movl %0, %%fs:16"
    ::"r"(ent));
    __sinit(ent);
}
