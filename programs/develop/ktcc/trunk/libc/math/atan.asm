 
format ELF
include '../proc32.inc'
section '.text' executable

public atan_ as "atan"

atan_:

	fld	qword[esp+4]
	fld1
	fpatan

	ret

