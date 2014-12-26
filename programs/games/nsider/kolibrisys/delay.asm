format COFF
include "public_stdcall.inc"
section '.text' code
public_stdcall __ksys_delay,4
;arg1 - time
  mov   edx,ebx
  mov   eax,5
  mov   ebx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4