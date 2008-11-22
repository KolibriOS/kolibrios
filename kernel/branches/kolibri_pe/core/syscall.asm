
format MS COFF

public _i40


section '.text' code readable align 16

align 16

_i40:
        ;   xchg bx, bx

           int 0x41
           iretd

