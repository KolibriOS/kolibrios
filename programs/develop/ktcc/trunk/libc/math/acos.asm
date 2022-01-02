 
format ELF
include '../proc32.inc'
section '.text' executable

public acos_ as "acos"

acos_:

;       acos(x) = atan(sqrt((1-x*x)/(x*x)))

	fld	qword[esp+4]
	fld     st0           ;Duplicate X on tos.
    fmul    st0, st1	  ;Compute X**2.
    fld     st0           ;Duplicate X**2 on tos.
    fld1                    ;Compute 1-X**2.
    fsub   st0, st1
    fdiv   st0, st1		;Compute (1-x**2)/X**2.
    fsqrt                   ;Compute sqrt((1-X**2)/X**2).
    fld1                    ;To compute full arctangent.
    fpatan                  ;Compute atan of the above.
    ret

