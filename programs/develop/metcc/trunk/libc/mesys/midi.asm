format ELF
section '.text' executable
public _msys_midi_reset
_msys_midi_reset:
  mov  edx,ebx
  mov  eax,20
  xor  ebx,ebx
  inc  ebx
  int  0x40
  mov  ebx,edx
  ret
  
public _msys_midi_send
_msys_midi_send:
;arg1 - data
  mov  edx,ebx
  mov  eax,20
  mov  ebx,2
  xor  ecx,ecx
  mov  cl,[esp+4]
  mov  ebx,edx
  ret  4