use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem
        dd      mem
        dd      0, 0

start:
        push    68
        pop     eax
        push    16
        pop     ebx
        push    drvname
        pop     ecx
        int     0x40
        push    -1
        pop     eax
        int     0x40

drvname db      'COM_MOUSE',0
i_end:
align 16
rb 16
mem:
