align 4
dtext:  ; Text String Output (rw by Johnny_B[john@kolibrios.org])
        ; eax x & y
        ; ebx style ( 0xX0000000 ) & color ( 0x00RRGGBB )
        ; X = ABnnb:
        ; nn = font
        ; A = 0 <=> output edx characters; otherwise output ASCIIZ string
        ; B = 1 <=> fill background with color esi
        ; ecx start of text
        ; edi 1 force

        pushad

        call    [disable_mouse]
        mov     ebp, ecx        ; ebp=pointer to text
        mov     ecx, ebx        ; ecx=color
        movsx   ebx, ax         ; ebx=y
        sar     eax, 16         ; eax=x
        cmp     edx, 255
        jb      .loop
        mov     edx, 255
.loop:
        test    ecx, ecx
        js      .test_asciiz
        dec     edx
        js      .end
        jmp     @f
.test_asciiz:
        cmp     byte [ebp], 0
        jz      .end
@@:
        push    edx
        movzx   edx, byte [ebp]
        inc     ebp
        test    ecx, 0x10000000
        jnz     .font2
        pushad
        mov     esi, 9
        lea     ebp, [FONT_I+8*edx+edx]
.symloop1:
        mov     dl, byte [ebp]
        or      dl, 1 shl 6
.pixloop1:
        shr     dl, 1
        jz      .pixloop1end
        jnc     .nopix
        call    [putpixel]
        jmp     .pixloop1cont
.nopix:
        test    ecx, 0x40000000
        jz      .pixloop1cont
        push    ecx
        mov     ecx, [esp+4+4]
        call    [putpixel]
        pop     ecx
.pixloop1cont:
        inc     eax
        jmp     .pixloop1
.pixloop1end:
        sub     eax, 6
        inc     ebx
        inc     ebp
        dec     esi
        jnz     .symloop1
        popad
        add     eax, 6
        pop     edx
        jmp     .loop
.font2:
        pushad
        add     edx, edx
        lea     ebp, [FONT_II+4*edx+edx+1]
        push    9
        movzx   esi, byte [ebp-1]
.symloop2:
        mov     dl, byte [ebp]
        push    esi
.pixloop2:
        shr     dl, 1
        jnc     .nopix2
        call    [putpixel]
        jmp     .pixloop2cont
.nopix2:
        test    ecx, 0x40000000
        jz      .pixloop2cont
        push    ecx
        mov     ecx, [esp+12+4]
        call    [putpixel]
        pop     ecx
.pixloop2cont:
        inc     eax
        dec     esi
        jnz     .pixloop2
        pop     esi
        sub     eax, esi
        inc     ebx
        inc     ebp
        dec     dword [esp]
        jnz     .symloop2
        pop     eax
        add     dword [esp+28], esi
        popad
        pop     edx
        jmp     .loop
.end:
        popad
        ret
