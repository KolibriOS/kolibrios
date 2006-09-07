format ELF
section '.text' executable

public _msys_debug_out
_msys_debug_out:
;arg1 - char to out
  push	ebx
  mov	ecx,[esp+8]
  mov	ebx,1
  mov	eax,63
  int	0x40
  pop	ebx
  ret   4
