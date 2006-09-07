format ELF
section '.text' executable
public _msys_get_key
_msys_get_key:
  mov   eax,2
  int   0x40
  ret
  
public _msys_set_keyboard_mode
_msys_set_keyboard_mode:
;arg1 - mode
  mov   edx,ebx
  mov   eax,66 
  xor   ebx,ebx
  inc   ebx
  mov   ecx,[esp+4]
  mov   ebx,edx
  ret   4