#include <menuet/os.h>
#include "http_msg.h"
//#include "http.h"

#define NULL 0

#define __stdcall __attribute__((stdcall))

extern int dll_load();
extern int mem_Free();
extern int mem_Alloc();
extern int mem_ReAlloc();

int (* __stdcall http_init)(void);
// On the next line, we should tell the C compiler that this procedure actually returns a pointer. (to the http_msg struct)
unsigned int (* __stdcall http_get) (char * url, unsigned int identifier, unsigned int flags, char * add_head); //yay, it's NOT uint, but hey, C is stubborn, and I'm dumb
int (* __stdcall http_receive) (unsigned int identifier);
void (* __stdcall http_free) (unsigned int identifier);
char * (* __stdcall http_find_header_field) (struct http_msg *http_ahoy, char *field_name); 
char * (* __stdcall http_unescape_url) (char * url_asciiz);
char * (* __stdcall http_post) (char *url, unsigned int identifier, unsigned int flags, char *headers, char *content_type, int content_length);
int (* __stdcall http_send) (struct http_msg *handle, char *data, unsigned int length);
void (* __stdcall http_disconnect) (struct http_msg *handle);

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

int kol_exit(){
  __menuet__debug_out("kol_exit()..Exiting..\n");
  __menuet__sys_exit();
}

void HTTP_INIT()
{
const IMP_ENTRY *imp;

imp = __kolibri__cofflib_load("/sys/lib/http.obj");
if (imp == NULL)
	kol_exit();

http_init = ( __stdcall  int(*)()) 
		__kolibri__cofflib_getproc (imp, "lib_init");
if (http_init == NULL)
  {
  __menuet__debug_out("http_init() is NULL. Exiting.\n");
  kol_exit();
  }
 else
   __menuet__debug_out("\nhttp_init() initialised properly.\n");


http_get = ( __stdcall  unsigned int (*)(char*, char*)) 
		__kolibri__cofflib_getproc  (imp, "get");
if (http_get == NULL)
  {
    __menuet__debug_out("http_get() is NULL. Exiting.\n");
    kol_exit();
  }
http_free = ( __stdcall  void (*)(unsigned int)) 
		__kolibri__cofflib_getproc  (imp, "free");
if (http_free == NULL)
  {
    __menuet__debug_out("http_free() is NULL. Exiting.\n");
    kol_exit();
  }	
http_receive = ( __stdcall  int (*)(unsigned int))
		__kolibri__cofflib_getproc  (imp, "receive");

if (http_receive == NULL)
  {
    __menuet__debug_out("http_receive() is NULL. Exiting.\n");
    kol_exit();
  }

http_find_header_field = ( __stdcall  char *(*)(struct http_msg*, char *)) 
		__kolibri__cofflib_getproc  (imp, "find_header_field");
if (http_find_header_field == NULL)
  {
    __menuet__debug_out("http_find_header_field() is NULL. Exiting.\n");
    kol_exit();
  }

http_unescape_url = ( __stdcall  char *(*)(char *))
  __kolibri__cofflib_getproc  (imp, "unescape");

if(http_unescape_url == NULL)
  {
    __menuet__debug_out("http_unescape_url() is NULL. Exiting.\n");
    kol_exit();
  }

 http_post = ( __stdcall  char *(*)(char *, char *, char *, int))
  __kolibri__cofflib_getproc  (imp, "post");

 if(http_post == NULL)
   {
     __menuet__debug_out("http_post() is NULL. Exiting.\n");
     kol_exit();
  }


 http_send = ( __stdcall  int (*)(struct http_msg *, char *, unsigned int))
  __kolibri__cofflib_getproc  (imp, "send");

 if(http_send == NULL)
   {
     __menuet__debug_out("http_send() is NULL. Exiting.\n");
     kol_exit();
  }

 
 http_disconnect = ( __stdcall  void (*)(struct http_msg *))
  __kolibri__cofflib_getproc  (imp, "disconnect");

 if(http_disconnect == NULL)
   {
     __menuet__debug_out("http_disconnect() is NULL. Exiting.\n");
     kol_exit();
  }

 

__menuet__debug_out("HTTP init...\n");
HTTP_YAY();

__menuet__debug_out("ok...\n");
}
