
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
};

int    _argc;
char **_argv;


void    __fastcall init_loader(void *libc_image);
void*   __fastcall create_image(void *raw);
int     __fastcall link_image(void *img_base);
void* get_entry_point(void *raw);
int (*entry)(int, char **, char **);


void init_reent();

jmp_buf loader_env;

void  __attribute__((noreturn))
__thread_startup (int (*entry)(void*), void *param,
                  void *stacklow, void *stackhigh)
{
    int retval;

    asm volatile ( "xchgw %bx, %bx");

    __asm__ __volatile__(               // save stack limits
    "movl %0, %%fs:4    \n\t"           // use TLS
    "movl %1, %%fs:8    \n\t"
    ::"r"(stacklow), "r"(stackhigh));

    init_reent();                       // initialize thread reentry structure

    retval = entry(param);              // call user thread function

    _exit(retval);
};

char * __libc_getenv(const char *name)
{
    return NULL;
}

char  __appcwd[1024];
int   __appcwdlen;
char* __appenv;
int   __appenv_size;

void  __attribute__((noreturn))
crt_startup (void *libc_base, void *obj_base, uint32_t *params)
{
    struct   app_hdr *header;
    char *arg[2];

    int len;
    char *p;

    void *my_app;
    int retval = 0;

//    user_free(obj_base);

    init_reent();
    __initPOSIXHandles();
 //   __appenv = load_file("/sys/system.env", &__appenv_size);

    init_loader(libc_base);

    my_app = create_image((void*)(params[0]));

    if( link_image(my_app)==0)
        goto done;

    header = (struct app_hdr*)NULL;

    __appcwdlen = strrchr(header->path, '/') - header->path;
    __appcwdlen = __appcwdlen > 1022 ? 1022 : __appcwdlen;
    memcpy(__appcwd, header->path, __appcwdlen);
    set_cwd(__appcwd);

#ifdef BRAVE_NEW_WORLD
    len = strlen(header->path);
    p = alloca(len+1);
    memcpy(p, header->path, len);
    p[len]=0;

    arg[0] = p;
#else
    arg[0] = header->path;
#endif

    _argc = 1;

    if( header->cmdline != 0)
    {
#ifdef BRAVE_NEW_WORLD
        len = strlen(header->cmdline);
        if(len)
        {
            p = alloca(len+1);
            memcpy(p, header->cmdline, len);
            p[len]=0;
            _argc = 2;
            arg[1] = p;
        };
#else
        _argc = 2;
        arg[1] = header->cmdline;
#endif
    };

    _argv = arg;

    entry = get_entry_point(my_app);

//    __asm__ __volatile__("int3");

    retval = entry(_argc, _argv, NULL);

done:
    exit (retval);
}


