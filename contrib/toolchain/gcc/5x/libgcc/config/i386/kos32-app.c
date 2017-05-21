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
        // Inform the user via BOARD that libc could not be loaded.
        char *errormsg = "[ERROR] libc.dll failed to load. is /kolibrios folder configured?\n";
        while (*errormsg) {
          __asm__ __volatile__("int $0x40"::"a"(63), "b"(1), "c"(*errormsg));
          ++errormsg;
        }

        // Exit
        asm ("int $0x40" ::"a"(-1));
    };

    entry = get_entry_point(img);
    entry(img);
}


