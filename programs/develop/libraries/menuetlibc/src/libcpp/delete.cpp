extern "C" {
#include<assert.h>
}

extern "C" void free(void *);
extern "C" void __menuet__sys_exit(void);

void operator delete(void * ptr)
{
 free(ptr);
}

void operator delete[](void * ptr)
{
 free(ptr);
}

static bool pure_virtual_call=false;

extern "C" {
extern "C" void __menuet__sys_exit();
void __cxa_pure_virtual(void)
{
 assert(!pure_virtual_call);
 __menuet__sys_exit();
}
void _pure_virtual(void)
{
 assert(!pure_virtual_call);
 __menuet__sys_exit();
}
void __pure_virtual(void)
{
 assert(!pure_virtual_call);
 __menuet__sys_exit();
}
}
