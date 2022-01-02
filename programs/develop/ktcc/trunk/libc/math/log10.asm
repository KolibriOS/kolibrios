 
format ELF
include '../proc32.inc'
section '.text' executable

public log10_ as "log10"

log10_:

;       ln(x) = lg(x)/lg(e).

	fld	qword[esp+4]
	fld1
	fxch
    fyl2x           ;Compute 1*lg(x).
    fldl2t          ;Load lg(10).
    fdivp  st1, st0	;Compute lg(x)/lg(10).
    ret
