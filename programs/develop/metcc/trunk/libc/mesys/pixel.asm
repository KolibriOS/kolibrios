format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_putpixel,12
;arg1 - x
;arg2 - y
;arg3 - color
  push  ebx
  xor   eax,eax
  mov   ebx,[esp+8]
  inc   eax
  mov   ecx,[esp+12]
  mov   edx,[esp+16]
  int   0x40
  pop   ebx
  ret   12
