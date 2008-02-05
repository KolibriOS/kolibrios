
include mdef.inc

           modstart tanf

           xdefp _tanf

_tanf proc
           fptan
           fnstsw  ax
           test    eax, 0x400
           jnz     L1
           fstp    st(0)
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
           fptan
           fstp    st(0)
           ret
_tanf endp

           endmod
           end
