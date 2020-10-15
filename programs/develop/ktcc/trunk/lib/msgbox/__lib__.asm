format ELF

include '__lib__.inc'

section '.text'

public lib_name

lib_name db 0x55, 0xAA, lib_name_str, 0
