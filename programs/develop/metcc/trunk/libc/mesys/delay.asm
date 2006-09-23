format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_delay,4
;arg1 - time
  mov   edx,ebx
  mov   eax,5
  mov   ebx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4