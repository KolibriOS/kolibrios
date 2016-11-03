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

#include <stdlib.h>

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
    int    __subsystem__;
};

extern void init_global_reent();
extern void init_stdio();
extern void __init_conio();
extern void __fini_conio();

extern void tls_init(void);
extern int main (int, char **, char **);

/* NOTE: The code for initializing the _argv, _argc, and environ variables
 *       has been moved to a separate .c file which is included in both
 *       crt1.c and dllcrt1.c. This means changes in the code don't have to
 *       be manually synchronized, but it does lead to this not-generally-
 *       a-good-idea use of include. */

char* __appenv;
int   __appenv_size;

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
__libc_init (void)
{
    struct   app_hdr *header = NULL;
    int retval = 0;

    char **argv;
    int    argc;

    tls_init();
    init_global_reent();
    init_stdio();

    if(header->__subsystem__ == 3)
        __init_conio();

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

    retval = main(argc, argv, NULL);
done:
    if(header->__subsystem__ == 3)
        __fini_conio();

    exit (retval);
}

