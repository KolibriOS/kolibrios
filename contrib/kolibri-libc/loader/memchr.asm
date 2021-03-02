format ELF
include "__lib__.inc"
fun      equ __func@memchr
fun_str  equ 'memchr'
section '.text'
fun_name db fun_str, 0
section '.data'
extrn lib_name
public fun as fun_str
fun dd fun_name
lib dd lib_name
