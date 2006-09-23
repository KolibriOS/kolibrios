format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_debug_out,4
;arg1 - char to out
  push	ebx
  mov	ecx,[esp+8]
  mov	ebx,1
  mov	eax,63
  int	0x40
  pop	ebx
  ret   4
