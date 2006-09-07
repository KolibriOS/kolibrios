format ELF
section '.text' executable
public _msys_putpixel
_msys_putpixel:
;arg1 - x
;arg2 - y
;arg3 - color
  push  ebx
  xor   eax,eax
  mov   ebx,[esp+8]
  inc   eax
  mov   ecx,[esp+12]
  mov   edx,[esp+16]
  int   0x40
  pop   ebx
  ret   12
