#include <newlib.h>

void init_reent();

void  __attribute__((noreturn))
__thread_startup (int (*entry)(void*), void *param,
                  void *stacklow, void *stackhigh)
{
    int retval;

    __asm__ __volatile__(               // save stack limits
    "movl %0, %%fs:4    \n\t"           // use TLS
    "movl %1, %%fs:8    \n\t"
    ::"r"(stacklow), "r"(stackhigh));

    init_reent();                       // initialize thread reentry structure

    retval = entry(param);              // call user thread function

    _exit(retval);
};

