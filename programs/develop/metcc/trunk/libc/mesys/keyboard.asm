format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_get_key,0
  mov   eax,2
  int   0x40
  ret
  
public_stdcall _msys_set_keyboard_mode,4
;arg1 - mode
  mov   edx,ebx
  mov   eax,66 
  xor   ebx,ebx
  inc   ebx
  mov   ecx,[esp+4]
  mov   ebx,edx
  ret   4