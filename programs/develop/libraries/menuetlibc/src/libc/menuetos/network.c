#include <menuet/os.h>


#define NULL 0
#define __stdcall __attribute__((stdcall))

extern int dll_load();
extern int mem_Free();
extern int mem_Alloc();
extern int mem_ReAlloc();

int kol_exit(){
  __menuet__sys_exit();
}


int (* __stdcall network_init)();
void (* __stdcall freeaddrinfo)(struct addrinfo* ai);
int (* __stdcall getaddrinfo)( const char* hostname, const char* servname, const struct addrinfo* hints, struct addrinfo **res);
char * (* __stdcall inet_ntoa)(struct in_addr in);
unsigned long (* __stdcall inet_addr)( const char* hostname);


int NETWORK_YAY(){
asm volatile ("pusha\n\
movl $mem_Alloc, %eax\n\
movl $mem_Free, %ebx\n\
movl $mem_ReAlloc, %ecx\n\
movl $dll_load, %edx\n\
movl network_init, %esi\n\
call *%esi\n\
popa\n");
}


void NETWORK_INIT()
{
IMP_ENTRY *imp;

imp = __kolibri__cofflib_load("/sys/lib/network.obj");
if (imp == NULL)
  kol_exit();

network_init = ( __stdcall int(*)())
__kolibri__cofflib_getproc (imp, "lib_init");
if (network_init == NULL)
  kol_exit();

freeaddrinfo = ( __stdcall void (*)(struct addrinfo*))
__kolibri__cofflib_getproc (imp, "freeaddrinfo");
if (freeaddrinfo == NULL)
  kol_exit();

getaddrinfo = ( __stdcall int (*)(const char*, const char*, const struct addrinfo*, struct addrinfo **))
__kolibri__cofflib_getproc (imp, "getaddrinfo");
if (getaddrinfo == NULL)
  kol_exit();

inet_ntoa = ( __stdcall char * (*)(struct in_addr))
__kolibri__cofflib_getproc (imp, "inet_ntoa");
if (inet_ntoa == NULL)
  kol_exit();

inet_addr = ( __stdcall unsigned long (*)(const char *))
__kolibri__cofflib_getproc (imp, "inet_addr");
if (inet_addr == NULL)
  kol_exit();

__menuet__debug_out("NETWORK init...\n");

NETWORK_YAY();

__menuet__debug_out("ok...\n");

} 