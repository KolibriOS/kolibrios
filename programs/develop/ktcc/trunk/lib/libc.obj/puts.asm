format ELF
include "__lib__.inc"
fun      equ puts
fun_str  equ 'puts'
section '.text'
fun_name db fun_str, 0
section '.data'
extrn lib_name
public fun
fun dd fun_name
lib dd lib_name
