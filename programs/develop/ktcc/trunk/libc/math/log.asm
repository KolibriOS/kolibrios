 
format ELF
include '../proc32.inc'
section '.text' executable

public log_ as "log"

log_:

;       ln(x) = lg(x)/lg(e).

	fld	qword[esp+4]
	fld1
	fxch
    fyl2x           ;Compute 1*lg(x).
    fldl2e          ;Load lg(e).
    fdivp st1, st0	;Compute lg(x)/lg(e).
    ret
