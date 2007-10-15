format ELF

include "proc32.inc"

section '.text' executable
public _ksys_get_system_clock

align 4
proc _ksys_get_system_clock stdcall

	mov   eax,3
	int   0x40
	ret

endp