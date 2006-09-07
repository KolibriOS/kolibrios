format ELF
section '.text' executable
public _msys_putimage
_msys_putimage:
;arg1 - x
;arg2 - y
;arg3 - xsize
;arg4 - ysize
;arg5 - image
  push ebx
  mov  ebx,[esp+24]
  mov  ecx,[esp+16]
  shl  ecx,16
  mov  ecx,[esp+20]
  mov  ebx,[esp+8]
  shl  ebx,16
  mov  ebx,[esp+12]
  mov  eax,7
  int  0x40
  pop  ebx
  ret  20