format ELF
section '.text' executable
public _msys_window_redraw
_msys_window_redraw:
;arg1 - status
  mov   edx,ebx
  mov   eax,12
  mov   ebx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4