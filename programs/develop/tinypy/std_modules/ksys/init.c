#include "tinypy.h"
#include "syscalls.c"

#define EXPORT(MOD_NAME, F_NAME, F_POINT) tp_set(tp, MOD_NAME , tp_string(F_NAME), tp_fnc(tp, F_POINT))

extern tp_obj tp_dict(TP);
extern tp_obj tp_fnc(TP,tp_obj v(TP));


void ksys_init(TP)
{
    tp_obj ksys_mod = tp_dict(tp);
    // syscalls 
    EXPORT(ksys_mod, "debug_print"  , _debug_print);
    EXPORT(ksys_mod, "start_draw"   , _start_draw);
    EXPORT(ksys_mod, "end_draw"     , _end_draw);
    EXPORT(ksys_mod, "create_window", _create_window);
    EXPORT(ksys_mod, "create_button", _create_button);
    EXPORT(ksys_mod, "draw_text"    , _draw_text);
    EXPORT(ksys_mod, "get_event"    , _get_event);
    EXPORT(ksys_mod, "get_button"    ,_get_button);
    EXPORT(ksys_mod, "get_sys_colors",_get_sys_colors);
    EXPORT(ksys_mod, "get_key"      , _get_key);
    
    tp_set(tp, ksys_mod, tp_string("__doc__"), tp_string("KolibriOS system specific functions."));
    tp_set(tp, ksys_mod, tp_string("__name__"), tp_string("kolibri"));
    tp_set(tp, ksys_mod, tp_string("__file__"), tp_string(__FILE__));

    tp_set(tp, tp->modules, tp_string("ksys"), ksys_mod);
}
