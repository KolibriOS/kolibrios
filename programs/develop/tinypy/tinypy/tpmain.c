#include "tp.c"
/* INCLUDE */
const char header[]="TinyPy for kolibriOS";

//extern void kolibri_dbg_init(tp_vm *tp); //__attribute__((__stdcall__));

void main(int argc, const char *argv[]) {
    tp_vm *tp = tp_init(argc, argv);
    kolibri_init(tp);
    CONSOLE_INIT(header);
    /* INIT */
    tp_call(tp,"py2bc","tinypy",tp_None);
    con_printf("Done");
    tp_deinit(tp);
    kol_exit();
    return;
}

/**/
