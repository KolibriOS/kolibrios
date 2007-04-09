; CLEAR 0x280000-0xF00000

        xor   eax,eax
        mov   edi,0x280000
        mov   ecx,(0x100000*0xF-0x280000) / 4
        cld
        rep   stosd
; CLEAR 0x80000-0x90000
;       xor   eax,eax

        mov   edi,0x80000
        mov   ecx,(0x90000-0x80000)/4
;       cld
        rep   stosd

; CLEAR KERNEL UNDEFINED GLOBALS
        mov   edi, endofcode
        mov   ecx, (uglobals_size/4)+4
        rep   stosd

; SAVE & CLEAR 0-0xffff

        mov   esi,0x0000
        mov   edi,0x2F0000
        mov   ecx,0x10000 / 4
        cld
        rep   movsd
        xor   eax,eax
        mov   edi,0
        mov   ecx,0x10000 / 4
        cld
        rep   stosd