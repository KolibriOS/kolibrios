 
format ELF
include '../proc32.inc'
section '.text' executable

public sqrtf

sqrtf:

	fld	dword[esp+4]
	fsqrt

	ret

