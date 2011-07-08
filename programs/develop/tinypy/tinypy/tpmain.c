#include "tp.c"
/* INCLUDE */
const char header[]="TinyPy for kolibriOS";
const int argc = 2;
extern _stdcall void testmod_init(tp_vm *tp);
void main(void) {
    char *argv[2]={"tpmain", "test.py"};

    CONSOLE_INIT(header);
    con_printf("TinyPy console, version 1.1.\n");
    con_printf("Enter program file:");
    if (!(argv[1] = malloc(256)))
      con_printf("Memory error\n");
    con_gets(argv[1], 256);
    argv[1][strlen(argv[1]) - 1] = '\0';
    con_printf("Running file %s\n", argv[1]);
    tp_vm *tp = tp_init(argc, argv);
    kolibri_init(tp);
    /* INIT */
    tp_call(tp,"py2bc","tinypy",tp_None);
    tp_deinit(tp);
    return;
}

/**/
