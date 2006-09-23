format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_get_system_clock,0
  mov   eax,3
  int   0x40
  ret