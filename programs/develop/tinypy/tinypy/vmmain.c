#define TP_COMPILER 0
#include "tp.c"

int main(int argc, char *argv[]) {
    tp_vm *tp = tp_init(argc,argv);
    tp_import(tp,argv[1],"__main__",0);
    tp_deinit(tp);
    return(0);
}
