format ELF
section '.text' executable
public _msys_make_button
_msys_make_button:
;arg1 - x
;arg2 - y
;arg3 - xsize
;arg4 - ysize
;arg5 - id
;arg6 - color
  push  ebx esi
  mov   ebx,[esp+12]
  shl   ebx,16
  mov   bx,[esp+20]
  mov   ecx,[esp+16]
  shl   ecx,16
  mov   cx,[esp+24]
  mov   edx,[esp+28]
  mov   esi,[esp+32]
  mov   eax,8
  int   0x40
  pop   esi ebx
  ret   24
  
public _msys_get_button_id  
_msys_get_button_id:
  mov   eax,17
  int   0x40
  test  al,al
  jnz   .no_button
  shr   eax,8
  ret
.no_button:
  xor   eax,eax
  dec   eax
  ret