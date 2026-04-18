
include mdef.inc

           modstart cosf

           xdefp _cosf

_cosf proc
           fcos
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
           fcos
           ret
_cosf endp

           endmod
           end
