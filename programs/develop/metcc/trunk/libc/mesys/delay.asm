format ELF
section '.text' executable
public _msys_delay
_msys_delay:
;arg1 - time
  mov   edx,ebx
  mov   eax,5
  mov   ebx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4