/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the kos32-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Source code for the startup proceedures used by all programs. This code
 * is compiled to make crt1.o, which should be located in the library path.
 *
 */

void *load_libc(void);
void* get_entry_point(void *raw);

void  __attribute__((noreturn))
__crt_startup (void)
{
    void __attribute__((noreturn)) (*entry)(void *img);
    void    *img;

    img = load_libc();

    if(!img)
    {
        asm ("int $0x40" ::"a"(-1));
    };

    entry = get_entry_point(img);
    entry(img);
}


