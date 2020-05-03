 
format ELF
include '../proc32.inc'
section '.text' executable

public sqrt

sqrt:

	fld	qword[esp+4]
	fsqrt

	ret

