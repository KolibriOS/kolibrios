format ELF
;include "public_stdcall.inc"

public _ksys_get_process_table

section '.text' executable

_ksys_get_process_table:
;arg1 - pointer to information
;arg2 - pid
  mov   eax,9
  mov   ebx,[esp+4]
  mov   ecx,[esp+8]
  int   0x40

  ret   8