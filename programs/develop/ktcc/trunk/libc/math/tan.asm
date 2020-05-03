 
format ELF
include '../proc32.inc'
section '.text' executable

public tan_ as "tan"

tan_:

	fld	qword[esp+4]
	fptan
	fxch

	ret

