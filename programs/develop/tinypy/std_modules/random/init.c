#include "random.c"

/*
 * random_mod_init()
 *
 * random module initialization function
 */
void random_init(TP)
{
    /*
     * module dict for random
     */
    tp_obj random_mod = tp_dict(tp);

    /*
     * bind functions to random module
     */
    tp_set(tp, random_mod, tp_string("seed"),       tp_fnc(tp, random_seed));
    tp_set(tp, random_mod, tp_string("getstate"),   tp_fnc(tp, random_getstate));
    tp_set(tp, random_mod, tp_string("setstate"),   tp_fnc(tp, random_setstate));
    tp_set(tp, random_mod, tp_string("jumpahead"),  tp_fnc(tp, random_jumpahead));
    tp_set(tp, random_mod, tp_string("random"),     tp_fnc(tp, random_random));

    /*
     * bind usual distribution random variable generator
     */
    tp_set(tp, random_mod, tp_string("uniform"),        tp_fnc(tp, random_uniform));
    tp_set(tp, random_mod, tp_string("normalvariate"),  tp_fnc(tp, random_normalvariate));
    tp_set(tp, random_mod, tp_string("lognormvariate"), tp_fnc(tp, random_lognormvariate));
    tp_set(tp, random_mod, tp_string("expovariate"),    tp_fnc(tp, random_expovariate));
    tp_set(tp, random_mod, tp_string("vonmisesvariate"), tp_fnc(tp, random_vonmisesvariate));
    tp_set(tp, random_mod, tp_string("gammavariate"),   tp_fnc(tp, random_gammavariate));
    tp_set(tp, random_mod, tp_string("betavariate"),    tp_fnc(tp, random_betavariate));
    tp_set(tp, random_mod, tp_string("paretovariate"),  tp_fnc(tp, random_paretovariate));
    tp_set(tp, random_mod, tp_string("weibullvariate"), tp_fnc(tp, random_weibullvariate));
    tp_set(tp, random_mod, tp_string("randrange"),      tp_fnc(tp, random_randrange));
    tp_set(tp, random_mod, tp_string("randint"),        tp_fnc(tp, random_randint));
    tp_set(tp, random_mod, tp_string("choice"),         tp_fnc(tp, random_choice));
    tp_set(tp, random_mod, tp_string("shuffle"),        tp_fnc(tp, random_shuffle));

    /*
     * bind special attributes to random module
     */
    tp_set(tp, random_mod, tp_string("__doc__"),  tp_string("Random variable generators."));
    tp_set(tp, random_mod, tp_string("__name__"), tp_string("random"));
    tp_set(tp, random_mod, tp_string("__file__"), tp_string(__FILE__));

    /*
     * bind random module to tinypy modules[]
     */
    tp_set(tp, tp->modules, tp_string("random"), random_mod);
}
