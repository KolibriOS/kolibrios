/* default reentrant pointer when multithread enabled */

#include <_ansi.h>
#include <string.h>
#include <reent.h>

#ifdef __getreent
#undef __getreent
#endif

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
    "movl %0, %%fs:0"
    ::"r"(ent));
    __sinit(ent);
}

struct _reent *
_DEFUN_VOID(__getreent)
{
    struct _reent *ent;

    __asm__ __volatile__(
    "movl %%fs:0, %0"
    :"=r"(ent));
    return ent;
}

void __mutex_lock(volatile int *val)
{
    int tmp;

    __asm__ __volatile__ (
"0:\n\t"
    "mov %0, %1\n\t"
    "testl %1, %1\n\t"
    "jz 1f\n\t"

    "movl $68, %%eax\n\t"
    "movl $1,  %%ebx\n\t"
    "int  $0x40\n\t"
    "jmp 0b\n\t"
"1:\n\t"
    "incl %1\n\t"
    "xchgl %0, %1\n\t"
    "testl %1, %1\n\t"
	"jnz 0b\n"
    : "+m" (*val), "=&r"(tmp)
    ::"eax","ebx" );
}
