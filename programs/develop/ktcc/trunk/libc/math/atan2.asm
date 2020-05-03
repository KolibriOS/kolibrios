 
format ELF
include '../proc32.inc'
section '.text' executable
            
public atan2_ as "atan2"

atan2_:

	fld	qword[esp+8]
	fld	qword[esp+4]
	fpatan

	ret

