format ELF

include "proc32.inc"

section '.text' executable

public _msys_debug_out

align 4
proc _msys_debug_out stdcall, c:byte

  xor ecx,ecx
  mov	cl,byte[c]
  mov	ebx,1
  mov	eax,63
  int	0x40
  ret

endp
