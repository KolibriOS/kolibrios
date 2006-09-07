format ELF
section '.text' executable
public _msys_get_system_clock
_msys_get_system_clock:
  mov   eax,3
  int   0x40
  ret