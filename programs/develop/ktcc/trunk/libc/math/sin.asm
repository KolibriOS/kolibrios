 
format ELF
include '../proc32.inc'
section '.text' executable

public sin_ as "sin"

sin_:

	fld	qword[esp+4]
	fsin

	ret

