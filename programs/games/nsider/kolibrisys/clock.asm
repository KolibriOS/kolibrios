format COFF

include "../../../proc32.inc"

section '.text' code
public __ksys_get_system_clock

align 4
proc __ksys_get_system_clock stdcall

	mov   eax,3
	int   0x40
	ret

endp