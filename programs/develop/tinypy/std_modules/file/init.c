#include "tinypy.h"
#include "fs.c"

#define EXPORT(MOD_NAME, F_NAME, F_POINT) tp_set(tp, MOD_NAME , tp_string(F_NAME), tp_fnc(tp, F_POINT))

extern tp_obj tp_dict(TP);
extern tp_obj tp_fnc(TP,tp_obj v(TP));


void file_init(TP)
{
    tp_obj file_mod = tp_dict(tp);
    EXPORT(file_mod, "open"  , kolibri_open);
    
    tp_set(tp, file_mod, tp_string("__doc__"), tp_string("File module (read / write)"));
    tp_set(tp, file_mod, tp_string("__name__"), tp_string("File"));
    tp_set(tp, file_mod, tp_string("__file__"), tp_string(__FILE__));
    
    tp_set(tp, tp->modules, tp_string("file"), file_mod);
}
