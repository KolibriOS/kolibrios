format ELF
section '.text' executable
public _msys_send_message
_msys_send_message:
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
  
public _msys_define_receive_area
_msys_define_receive_area:
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