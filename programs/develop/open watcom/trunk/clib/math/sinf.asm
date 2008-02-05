
include mdef.inc

           modstart sinf

           xdefp _sinf

_sinf proc
           fsin
           fnstsw  ax
           test   eax, 0x400
           jnz     L1
           ret

L1:        fldpi
           fadd    st,st
           fxch    st(1)
L2:
           fprem1
           fnstsw  ax
           test   eax,0x400
           jnz     L2
           fstp    st(1)
           fsin
           ret
_sinf endp

           endmod
           end
