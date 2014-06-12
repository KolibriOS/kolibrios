#include<menuet/os.h>
#include<stdio.h>
#include<stdlib.h>

void * __menuet__exec_thread(void (* func_ptr)(void),__u32 stack_size,int * retp)
{
 void * __stk, * __ret;
 __ret=__stk=malloc(stack_size);
 __stk+=stack_size-1;
 __asm__ __volatile__("int $0x40":"=a"(*retp):"0"(51L),"b"(1L),"c"((__u32)func_ptr),"d"((__u32)__stk));
 return __ret;
}
