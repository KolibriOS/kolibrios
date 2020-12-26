/* INCLUDE */
#include "tp.c"

const char header[] = "TinyPy for KolibriOS";

int main(int argc, const char *argv[]) {
    /* INIT */
    tp_vm *tp = tp_init(argc, argv);
    kolibri_init(tp);
    CONSOLE_INIT(header);
    tp_call(tp,"py2bc","tinypy",tp_None);
//  con_printf("Done");
    tp_deinit(tp);
    
    // Exit console
    con_exit(0);
    
    // Exit program
    return 0;
}

/**/
