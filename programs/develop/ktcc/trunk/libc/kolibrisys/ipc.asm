format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _ksys_send_message,12
;arg1 - pid
;arg2 - msg
;arg3 - size
  push  ebx esi
  mov   eax,60
  mov   ebx,2
  mov   ecx,[esp+12]
  mov   edx,[esp+16]
  mov   esi,[esp+20]
  int   0x40
  pop   esi ebx
  ret   12
  
public_stdcall _ksys_define_receive_area,8
;arg1 - area
;arg2 - size
  push  ebx
  mov   eax,60
  mov   ebx,1
  mov   ecx,[esp+8]
  mov   edx,[esp+12]
  int   0x40
  pop   ebx
  ret   8