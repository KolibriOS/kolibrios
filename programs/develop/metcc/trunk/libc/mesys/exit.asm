format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_exit,0
  xor   eax,eax
  dec   eax
  int   0x40
; ret  