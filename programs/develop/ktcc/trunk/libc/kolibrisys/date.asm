format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _ksys_get_date,0
  mov   eax,29
  int   0x40
  ret