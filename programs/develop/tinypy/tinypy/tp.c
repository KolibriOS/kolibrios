#ifndef TP_COMPILER
#define TP_COMPILER 1
#endif

#include "tp.h"
#include "list.c"
#include "koconsole.c"
#include "dict.c"
#include "misc.c"
#include "string.c"
#include "builtins.c"
#include "gc.c"
#include "ops.c"
void tp_compiler(TP);
#include "vm.c"

tp_obj tp_None = {TP_NONE};

#if TP_COMPILER
#include "bc.c"
void tp_compiler(TP) {
    tp_import(tp,0,"tokenize",tp_tokenize);
    tp_import(tp,0,"parse",tp_parse);
    tp_import(tp,0,"encode",tp_encode);
    tp_import(tp,0,"py2bc",tp_py2bc);
    tp_call(tp,"py2bc","_init",tp_None);
}
#else
void tp_compiler(TP) { }
#endif

/**/
