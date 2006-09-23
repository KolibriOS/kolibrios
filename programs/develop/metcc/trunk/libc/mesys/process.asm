format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_get_process_table,8
;arg1 - pointer to information
;arg2 - pid
  mov   edx,ebx
  mov   eax,9
  mov   ebx,[esp+4]
  mov   ecx,[esp+8]
  int   0x40
  mov   ebx,edx
  ret   8