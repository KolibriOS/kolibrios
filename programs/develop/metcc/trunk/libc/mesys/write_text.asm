format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_write_text,20
;arg1 - x
;arg2 - y
;arg3 - color
;arg4 - text
;arg5 - len
  push  ebx esi
  mov   eax,4
  mov   ebx,[esp+12]
  shl   ebx,16
  mov   bx,[esp+16]
  mov   ecx,[esp+20]
  mov   edx,[esp+24]
  mov   esi,[esp+28]
  int   0x40
  pop   esi ebx
  ret   20