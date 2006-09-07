format ELF
section '.text' executable
public _msys_exit
_msys_exit:
  xor   eax,eax
  dec   eax
  int   0x40
; ret  