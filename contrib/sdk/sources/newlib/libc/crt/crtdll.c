/*
 * crtdll.c
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Source code for the shared libc startup proceedures. This code is compiled
 * to make libc.dll, which should be located in the library path.
 *
 */

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

void _pei386_runtime_relocator (void);
void init_loader(void *libc_image);
void init_reent();

int link_app();
void* get_entry_point(void *raw);
int (*entry)(int, char **, char **);

char* __appenv;
int   __appenv_size;

extern char _tls_map[128];

char * __libc_getenv(const char *name)
{
    return NULL;
}

static int split_cmdline(char *cmdline, char **argv)
{
    enum quote_state
    {
        QUOTE_NONE,         /* no " active in current parm       */
        QUOTE_DELIMITER,    /* " was first char and must be last */
        QUOTE_STARTED       /* " was seen, look for a match      */
    };

    enum quote_state state;
    unsigned int argc;
    char *p = cmdline;
    char *new_arg, *start;

    argc = 0;

    for(;;)
    {
        /* skip over spaces and tabs */
        if ( *p )
        {
            while (*p == ' ' || *p == '\t')
                ++p;
        }

        if (*p == '\0')
            break;

        state = QUOTE_NONE;
        if( *p == '\"' )
        {
            p++;
            state = QUOTE_DELIMITER;
        }
        new_arg = start = p;
        for (;;)
        {
            if( *p == '\"' )
            {
                p++;
                if( state == QUOTE_NONE )
                {
                    state = QUOTE_STARTED;
                }
                else
                {
                    state = QUOTE_NONE;
                }
                continue;
            }

            if( *p == ' ' || *p == '\t' )
            {
                if( state == QUOTE_NONE )
                {
                    break;
                }
            }

            if( *p == '\0' )
                break;

            if( *p == '\\' )
            {
                if( p[1] == '\"' )
                {
                    ++p;
                    if( p[-2] == '\\' )
                    {
                        continue;
                    }
                }
            }
            if( argv )
            {
                *(new_arg++) = *p;
            }
            ++p;
        };

        if( argv )
        {
            argv[ argc ] = start;
            ++argc;

            /*
              The *new = '\0' is req'd in case there was a \" to "
              translation. It must be after the *p check against
              '\0' because new and p could point to the same char
              in which case the scan would be terminated too soon.
            */

            if( *p == '\0' )
            {
                *new_arg = '\0';
                break;
            }
            *new_arg = '\0';
            ++p;
        }
        else
        {
            ++argc;
            if( *p == '\0' )
            {
                break;
            }
            ++p;
        }
    }

    return argc;
};

void  __attribute__((noreturn))
libc_crt_startup (void *libc_base)
{
    struct   app_hdr *header = NULL;
    int retval = 0;

    char **argv;
    int    argc;

    _pei386_runtime_relocator();

    memset(_tls_map, 0xFF, 32*4);
    _tls_map[0] = 0xE0;
    init_reent();
    init_stdio();
    __do_global_ctors();

 //   __appenv = load_file("/sys/system.env", &__appenv_size);

    init_loader(libc_base);

    if( link_app() == 0)
        goto done;

    if( header->cmdline[0] != 0)
    {
        argc = split_cmdline(header->cmdline, NULL) + 1;
        argv = alloca((argc+1)*sizeof(char*));
        argv[0] = header->path;

        split_cmdline(header->cmdline, argv + 1);
    }
    else
    {
        argc = 1;
        argv = alloca((argc+1)*sizeof(char*));
        argv[0] = header->path;
    }
    argv[argc] = NULL;

    retval = header->main(argc, argv, NULL);
done:
    exit (retval);
}

