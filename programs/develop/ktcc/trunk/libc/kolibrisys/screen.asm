format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _ksys_get_screen_size,8
;arg1 - x
;arg2 - y
  mov   eax,14
  int   0x40
  mov   ecx,[esp+8]
  mov   [ecx],ax
  mov   word [ecx+2],0
  shr   eax,16
  mov   ecx,[esp+4]
  mov   [ecx],eax
  ret   8