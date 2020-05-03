 
format ELF
include '../proc32.inc'
section '.text' executable

public fabsf

fabsf:

	fld	dword[esp+4]
	fabs

	ret

