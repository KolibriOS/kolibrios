 
format ELF
include '../proc32.inc'
section '.text' executable

public fabs_ as "fabs"

fabs_:

	fld	qword[esp+4]
	fabs

	ret

