#include "tp.c"

const char header[]="TinyPy for kolibriOS";
const int argc = 2;

void main(void) {
    char *argv[2]={"tpmain", ""};

    CONSOLE_INIT(header);
    con_printf("TinyPy console, version 1.1.\n");
    con_printf("Enter program file:");
    if (!(argv[1] = malloc(256)))
      con_printf("Memory error\n");
    con_gets(argv[1], 256);
    argv[1][strlen(argv[1]) - 1] = '\0';
    con_printf("Running file %s\n", argv[1]);
    tp_vm *tp = tp_init(argc, argv);
    
    tp_call(tp,"py2bc","tinypy",tp_None);
    tp_deinit(tp);
    return;
}

/**/
