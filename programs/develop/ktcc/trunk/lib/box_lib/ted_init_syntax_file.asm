format ELF

include "__lib__.inc"

fun      equ ted_init_syntax_file
fun_str  equ 'ted_init_syntax_file'

section '.text'

fun_name db fun_str, 0

section '.data'

extrn lib_name
public fun

fun dd fun_name
lib dd lib_name
