 
format ELF
include '../proc32.inc'
section '.text' executable
            
public exp_ 	as "exp"
public exp2_ 	as "exp2"

SaveCW		dw ?
MaskedCW	dw ?

;               2**x = 2**int(x) * 2**frac(x).
;               We can easily compute 2**int(x) with fscale and
;               2**frac(x) using f2xm1.
exp2_int:
                fstcw   [SaveCW]

; Modify the control word to truncate when rounding.

                fstcw   [MaskedCW]
                or      byte ptr MaskedCW + 1, 1100b
                fldcw   [MaskedCW]

                fld     st0           ;Duplicate tos.
                fld     st0
                frndint                 ;Compute integer portion.

                fxch                    ;Swap whole and int values.
                fsub    st0, st1    ;Compute fractional part.

                f2xm1                   ;Compute 2**frac(x)-1.
                fld1
                faddp    st1, st0	;Compute 2**frac(x).

                fxch                    ;Get integer portion.
                fld1                    ;Compute 1*2**int(x).
                fscale
                fstp    st1           ;Remove st(1) (which is 1).

                fmulp  st1, st0		;Compute 2**int(x) * 2**frac(x).
                fstp    st1           ;Remove st1

                fldcw   [SaveCW]     ;Restore rounding mode.
                ret

exp_:
;       exp(x) = 2**(x * lg(e))

		fld	qword[esp+4]
        fldl2e          ;Put lg(e) onto the stack.
        fmulp st1, st0	;Compute x*lg(e).
        call    exp2_int;Compute 2**(x * lg(e))
        ret

exp2_:          
		fld	qword[esp+4]
        call    exp2_int;Compute 2 ** x
        ret


