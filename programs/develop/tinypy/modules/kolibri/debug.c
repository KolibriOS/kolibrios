#include "tp.h"

void debug_write_byte(const char ch){
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(63), "b"(1), "c"(ch)
    );
}

tp_obj kolibri_debug_print(TP)
{
    tp_obj str = TP_TYPE(TP_STRING);
    for(int i=0; i < str.string.len; i++)
    {
       debug_write_byte(str.string.val[i]); 
    }
    return tp_None;
} 

