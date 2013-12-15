
#include <_ansi.h>
#include <reent.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include <sys/kos_io.h>

struct app_hdr
{
    char  banner[8];
    int   version;
    int   start;
    int   iend;
    int   memsize;
    int   stacktop;
    char  *cmdline;
    char  *path;
    int    reserved;
    void  *__idata_start;
    void  *__idata_end;
    int  (*main)(int argc, char **argv, char **envp);
};

int    _argc;
char **_argv;


void* get_entry_point(void *raw);
int (*entry)(int, char **, char **);

void init_loader(void *libc_image);


void init_reent();

jmp_buf loader_env;

void  __attribute__((noreturn))
__thread_startup (int (*entry)(void*), void *param,
                  void *stacklow, void *stackhigh)
{
    int retval;

    asm volatile ( "xchgw %bx, %bx");

    __asm__ __volatile__(               // save stack limits
    "movl %0, %%fs:8    \n\t"           // use TLS
    "movl %1, %%fs:12    \n\t"
    ::"r"(stacklow), "r"(stackhigh));

    init_reent();                       // initialize thread reentry structure

    retval = entry(param);              // call user thread function

    _exit(retval);
};

char * __libc_getenv(const char *name)
{
    return NULL;
}

void _pei386_runtime_relocator (void);
int link_app();

char  __appcwd[1024];
int   __appcwdlen;
char* __appenv;
int   __appenv_size;

static char *arg[2];

extern char _tls_map[128];

void  __attribute__((noreturn))
libc_crt_startup (void *libc_base)
{
    struct   app_hdr *header = NULL;

    int len;
    char *p;

    void *my_app;
    int retval = 0;

    _pei386_runtime_relocator();

    memset(_tls_map, 0xFF, 32*4);
    _tls_map[0] = 0xE0;
    init_reent();
    __initPOSIXHandles();

 //   __appenv = load_file("/sys/system.env", &__appenv_size);

    init_loader(libc_base);

    if( link_app() == 0)
        goto done;

    __appcwdlen = strrchr(header->path, '/') - header->path;
    __appcwdlen = __appcwdlen > 1022 ? 1022 : __appcwdlen;
    memcpy(__appcwd, header->path, __appcwdlen);
    set_cwd(__appcwd);

    arg[0] = header->path;

    if( header->cmdline[0] != 0)
    {
        _argc = 2;
        arg[1] = header->cmdline;
    }
    else _argc = 1;

    _argv = arg;

    retval = header->main(_argc, _argv, NULL);
done:
    exit (retval);
}


