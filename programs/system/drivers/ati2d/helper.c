
#include "common.h"

void usleep(u32 delay)
{
  if(!delay) delay++;
  delay*=2000;

  asm __volatile__
  (
     "1:\n\t"
      "xorl %%eax, %%eax \n\t"
      "cpuid \n\t"
      "decl %%edi \n\t"
      "jnz 1b"
      :
      :"D"(delay)
      :"eax","ebx","ecx","edx"
  );
}

u32 __PciApi(int cmd)
{
  u32 retval;

  asm __volatile__
  (
    "call *__imp__PciApi"
    :"=eax" (retval)
    :"a" (cmd)
    :"memory"
  );
  return retval;
};

/*
u32 __RegService(char *name, srv_proc_t proc)
{
  u32 retval;

  asm __volatile__
  (
    "pushl %%eax \n\t"
    "pushl %%ebx \n\t"
    "call *__imp__RegService \n\t"
    :"=eax" (retval)
    :"a" (proc), "b" (name)
    :"memory"
  );
  return retval;
};
*/

void *__CreateObject(u32 pid, size_t size)
{
  void *retval;

  asm __volatile__
  (
    "call *__imp__CreateObject \n\t"
    :"=eax" (retval)
    :"a" (size),"b"(pid)
    :"esi","edi", "memory"
  );

  return retval;
}

void *__DestroyObject(void *obj)
{
  asm __volatile__
  (
    "call *__imp__DestroyObject"
    :
    :"a" (obj)
    :"ebx","edx","esi","edi", "memory"
  );
}
