/*
 * crt1.c
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Source code for the startup proceedures used by all programs. This code
 * is compiled to make crt1.o, which should be located in the library path.
 *
 */

/* Hide the declaration of _fmode with dllimport attribute in stdlib.h to
   avoid problems with older GCC. */

#include <newlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/kos_io.h>

#include "cpu_features.h"


/* NOTE: The code for initializing the _argv, _argc, and environ variables
 *       has been moved to a separate .c file which is included in both
 *       crt1.c and dllcrt1.c. This means changes in the code don't have to
 *       be manually synchronized, but it does lead to this not-generally-
 *       a-good-idea use of include. */


extern char __cmdline[];
extern char __pgmname[];

extern int main (int, char **, char **);

int   _errno;
int   _fmode;

char  __appcwd[1024];
int   __appcwdlen;

int    _argc;
char **_argv;

static char *arg[2];

void   _exit(int __status) __attribute__((noreturn));


char * __libc_getenv(const char *name)
{
    return NULL;
}

void __main (){};

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

typedef void (*ctp)();
static void __do_global_ctors ()
{
  extern int __CTOR_LIST__;
  int *c = &__CTOR_LIST__;
  c++;
  while (*c)
    {
      ctp d = (ctp)*c;
      (d)();
      c++;
    }
}

void  __attribute__((noreturn))
__crt_startup (void)
{
    int nRet;
    struct   app_hdr *header;


    init_global_reent();

  /*
   * Initialize floating point unit.
   */
    __cpu_features_init ();   /* Do we have SSE, etc.*/
//  _fpreset ();              /* Supplied by the runtime library. */

    __do_global_ctors();

    __appcwdlen = strrchr(&__pgmname, '/') - &__pgmname + 1;
    __appcwdlen = __appcwdlen > 1023 ? 1023 : __appcwdlen;
    memcpy(__appcwd, &__pgmname, __appcwdlen);
    __appcwd[__appcwdlen] = 0;

    set_cwd(__appcwd);

    arg[0] = &__pgmname;

    if( __cmdline[0] != 0)
    {
        _argc = 2;
        arg[1] = &__cmdline;
    } else _argc = 1;

    _argv = arg;

  /*
   * Sets the default file mode.
   * If _CRT_fmode is set, also set mode for stdin, stdout
   * and stderr, as well
   * NOTE: DLLs don't do this because that would be rude!
   */
//  _mingw32_init_fmode ();


    nRet = main (_argc, _argv, NULL);

  /*
   * Perform exit processing for the C library. This means
   * flushing output and calling 'atexit' registered functions.
   */
    exit (nRet);
}



