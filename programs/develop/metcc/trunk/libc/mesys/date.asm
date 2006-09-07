format ELF
section '.text' executable
public _msys_get_date
_msys_get_date:
  mov   eax,29
  int   0x40
  ret