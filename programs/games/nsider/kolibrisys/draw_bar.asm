format COFF
include "public_stdcall.inc"
section '.text' code
public_stdcall __ksys_draw_bar,20
;arg1 - x
;arg2 - y
;arg3 - xsize
;arg4 - ysize
;arg5 - color
  push  ebx
  mov   eax,13
  mov   ebx,[esp+8]
  shl   ebx,16
  mov   bx,[esp+16]
  mov   ecx,[esp+12]
  shl   ecx,16
  mov   cx,[esp+20]
  mov   edx,[esp+24]
  int   0x40
  pop   ebx
  ret   20