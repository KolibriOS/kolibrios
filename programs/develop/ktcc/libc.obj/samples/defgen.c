#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

#define ARGC_VALID 3

enum ARGV_FILE {
    IN = 1,
    OUT = 2
};

void show_help(void)
{
    puts("Usage: defgen [lib.obj] [lib.def]");
}

int main(int argc, char** argv)
{

    if (argc != ARGC_VALID) {
        show_help();
        return 0;
    }

    ksys_dll_t* obj_dll = _ksys_dlopen(argv[IN]);
    FILE* outfile = fopen(argv[OUT], "w");

    if (!obj_dll) {
        printf("File '%s' not found!\n", argv[IN]);
        return 1;
    }

    if (!outfile) {
        printf("Unable to create file:'%s'!\n", argv[OUT]);
        return 2;
    }

    fprintf(outfile, "LIBRARY %s\n\n", basename(argv[IN]));
    fputs("EXPORTS\n", outfile);

    int i = 0;
    while (obj_dll[i].func_name) {
        fprintf(outfile, "%s\n", obj_dll[i].func_name);
        i++;
    }
    fclose(outfile);
    return 0;
}
