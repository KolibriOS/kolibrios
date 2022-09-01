/* default reentrant pointer when multithread enabled */

#include <_ansi.h>
#include <string.h>
#include <sys/reent.h>
#include <sys/ksys.h>

extern _VOID   _EXFUN(__sinit,(struct _reent *));

void init_reent()
{
    struct _reent *ent;

    ent = _ksys_alloc(sizeof(struct _reent));

    _REENT_INIT_PTR_ZEROED(ent);

    __asm__ __volatile__(
    "movl %0, %%fs:16"
    ::"r"(ent));
    __sinit(ent);
}
