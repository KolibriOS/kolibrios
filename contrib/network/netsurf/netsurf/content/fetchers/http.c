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



struct http_msg {
unsigned int socket;
unsigned int flags;
unsigned int write_ptr;
unsigned int buffer_length;
unsigned int chunk_ptr;
unsigned int timestamp;
unsigned int status;
unsigned int header_length;
unsigned int content_length;
unsigned int content_received;
char data; //unknown size
};


int (* __stdcall http_init)();
unsigned int (* __stdcall http_get) (char * url); //yay, it's NOT uint, but hey, C is stubborn, and I'm dumb
int (* __stdcall http_process) (unsigned int identifier);
void (* __stdcall http_free) (unsigned int identifier);


int HTTP_YAY(){
	asm volatile ("pusha\n\
			   movl $mem_Alloc, %eax\n\
			   movl $mem_Free, %ebx\n\
			   movl $mem_ReAlloc, %ecx\n\
			   movl $dll_load, %edx\n\
			   movl http_init, %esi\n\
			   call *%esi\n\
			   popa");
}

///===========================

void HTTP_INIT()
{
IMP_ENTRY *imp;

imp = __kolibri__cofflib_load("/sys/lib/http.obj");
if (imp == NULL)
	kol_exit();

http_init = ( __stdcall  int(*)()) 
		__kolibri__cofflib_getproc (imp, "lib_init");
if (http_init == NULL)
	kol_exit();

http_get = ( __stdcall  unsigned int (*)(char*)) 
		__kolibri__cofflib_getproc  (imp, "get");
if (http_get == NULL)
	kol_exit();

http_free = ( __stdcall  void (*)(unsigned int)) 
		__kolibri__cofflib_getproc  (imp, "free");
if (http_free == NULL)
	kol_exit();

	
http_process = ( __stdcall  int (*)(unsigned int)) 
		__kolibri__cofflib_getproc  (imp, "process");
if (http_process == NULL)
	kol_exit();

__menuet__debug_out("HTTP init...\n");
HTTP_YAY();

__menuet__debug_out("ok...\n");

}
