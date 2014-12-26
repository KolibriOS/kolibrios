format COFF
include "public_stdcall.inc"
section '.text' code
public_stdcall __ksys_line,20
;arg1 - x1
;arg2 - y1
;arg3 - x2
;arg4 - y2
;arg5 - color
  push  ebx
  mov   ebx,[esp+8]
  shl   ebx,16
  mov   bx,[esp+16]
  mov   ecx,[esp+12]
  shl   ecx,16
  mov   cx,[esp+20]
  mov   edx,[esp+24]
  mov   eax,38
  int   0x40
  pop   ebx
  ret   20