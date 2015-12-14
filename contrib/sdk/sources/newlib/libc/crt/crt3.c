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

void *load_libc();
void* get_entry_point(void *raw);

void _pei386_runtime_relocator (void){};

void  __attribute__((noreturn))
__crt_startup (void)
{
    struct   app_hdr *header;
    void    *img;
    void __attribute__((noreturn)) (*entry)(void *img);

    img = load_libc();

    if(img == NULL)
    {
        asm ("int $0x40" ::"a"(-1));
    };

    entry = get_entry_point(img);
    entry(img);
}



