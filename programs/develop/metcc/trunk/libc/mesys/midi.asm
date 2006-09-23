format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_midi_reset,0
  mov  edx,ebx
  mov  eax,20
  xor  ebx,ebx
  inc  ebx
  int  0x40
  mov  ebx,edx
  ret
  
public_stdcall _msys_midi_send,4
;arg1 - data
  mov  edx,ebx
  mov  eax,20
  mov  ebx,2
  xor  ecx,ecx
  mov  cl,[esp+4]
  mov  ebx,edx
  ret  4