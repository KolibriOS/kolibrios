#include <ksys.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    #ifdef _DYNAMIC
        ksys_coff_etable_t *libc=_ksys_cofflib_load("/sys/lib/libc.obj");
        debug_printf = _ksys_cofflib_getproc(libc, "debug_printf");
    #endif
    char test_stack[1000000];
    debug_printf("argc = %d\n", argc);
    debug_printf("Argument array:\n");

    for(int i = 0; i < argc; i++) {
        debug_printf("argv[%d] = %s\n", i, argv[i]);    
    }
    debug_printf("Done!\n");
    return 0;
}
