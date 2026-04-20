init_crc_table:
        xor     edx, edx
        mov     edi, crc_table
.1:
        mov     ecx, 8
        mov     eax, edx
.2:
        shr     eax, 1
        jnc     @f
        xor     eax, 0xEDB88320
@@:
        loop    .2
        stosd
        inc     dl
        jnz     .1
        ret

crc_continue:
; in: eax = pervios crc, ecx = size, esi->buffer
; out: eax = crc
        xor     eax, -1
        jecxz   crc.end
        jmp     crc.loop

crc:
; in: ecx=size, esi->buffer
; out: eax=crc
        or      eax, -1
        jecxz   .end
.loop:
        movzx   edx, al
        xor     dl, byte [esi]
        inc     esi
        shr     eax, 8
        xor     eax, [crc_table+edx*4]
        loop    .loop
.end:
        xor     eax, -1
        ret
