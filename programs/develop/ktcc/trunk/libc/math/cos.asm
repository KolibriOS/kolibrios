 
format ELF
include '../proc32.inc'
section '.text' executable

public cos_ as "cos"

cos_:

	fld	qword[esp+4]
	fcos

	ret

