/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3 */

#include "tinypy.h"

#define GET_NUM_ARG() TP_TYPE(TP_NUMBER).number.val
#define GET_STR_ARG() TP_TYPE(TP_STRING).string.val

static tp_obj _add(TP){
    unsigned num1 = (unsigned)GET_NUM_ARG();
    unsigned num2 = (unsigned)GET_NUM_ARG();
    return tp_number(num1 | num2);
} 

static tp_obj _mul(TP){
    unsigned num1 = (unsigned)GET_NUM_ARG();
    unsigned num2 = (unsigned)GET_NUM_ARG();
    return tp_number(num1 & num2);
    
}

