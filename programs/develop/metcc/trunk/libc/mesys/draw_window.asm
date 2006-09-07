format ELF
section '.text' executable

public _msys_draw_window
_msys_draw_window:
;arg1 - xcoord
;arg2 - ycoord
;arg3 - xsize
;arg4 - ysize
;arg5 - workcolor
;arg6 - type
;arg7 - captioncolor
;arg8 - windowtype
;arg9 - bordercolor
  push ebp
  mov  ebp,esp
  push ebx esi edi
  mov  ebx,[ebp+8]
  shl  ebx,16
  mov  bx,[ebp+16]
  mov  ecx,[ebp+12]
  shl  ecx,16
  mov  cx,[ebp+20]
  mov  edx,[ebp+28]
  shl  edx,24
  add  edx,[ebp+24]
  mov  esi,[ebp+36]
  shl  esi,24
  add  esi,[ebp+32]
  mov  edi,[ebp+40]
  xor  eax,eax
  int  0x40
  pop  edi esi ebx
  pop  ebp
  ret  36
