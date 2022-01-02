format ELF
section '.text' executable
public _ksys_write_text

_ksys_write_text:
;arg1 - x
;arg2 - y
;arg3 - color
;arg4 - text
;arg5 - len

  mov   eax,4
  mov   ebx,[esp+4]
  shl   ebx,16
  mov   bx,[esp+8]
  mov   ecx,[esp+12]
  mov   edx,[esp+16]
  mov   esi,[esp+20]
  int   0x40

  ret   20