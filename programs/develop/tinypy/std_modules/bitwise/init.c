#include "tinypy.h"
#include "bitwise.c"

#define EXPORT(MOD_NAME, F_NAME, F_POINT) tp_set(tp, MOD_NAME , tp_string(F_NAME), tp_fnc(tp, F_POINT))

void bitwise_init(TP)
{
    tp_obj bit_mod = tp_dict(tp);

    EXPORT(bit_mod, "add" , _add);
    EXPORT(bit_mod, "mul" , _mul);

    tp_set(tp, bit_mod, tp_string("__doc__"), tp_string("Bitwise operations for large numbers"));
    tp_set(tp, bit_mod, tp_string("__name__"), tp_string("bitwise"));
    tp_set(tp, bit_mod, tp_string("__file__"), tp_string(__FILE__));

    tp_set(tp, tp->modules, tp_string("bitwise"), bit_mod);
}
