;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; // Alver 22.06.2008 // {
align 4
dtext_asciiz_esi:                          ; for skins title out
    push eax
    xor eax, eax
    inc eax
    jmp    dtext.1
; } \\ Alver \\

align 4
dtext:  ; Text String Output (rw by Johnny_B[john@kolibrios.org])
        ; ebx x & y
        ; ecx style ( 0xX0000000 ) & color ( 0x00RRGGBB )
        ; X = ABnnb:
        ; nn = font
        ; A = 0 <=> output esi characters; otherwise output ASCIIZ string
        ; B = 1 <=> fill background with color eax
        ; edx start of text
        ; edi 1 force

; // Alver 22.06.2008 // {
       push eax
        xor     eax, eax
.1:
; } \\ Alver \\
        pushad
        call    [disable_mouse]

        movsx   eax, bx         ; eax=y
        sar     ebx, 16         ; ebx=x
        xchg    eax, ebx        ; eax=x, ebx=y
        cmp     esi, 255
        jb      .loop
        mov     esi, 255
.loop:
        test    ecx, ecx
        js      .test_asciiz
        dec     esi
        js      .end
        jmp     @f
.test_asciiz:
        cmp     byte [edx], 0
        jz      .end
; // Alver 22.06.2008 // {
        cmp byte [esp+28], 1
        jne   @f
        dec     esi
        js        .end
; } \\ Alver \\
@@:
        inc     edx
        pushad
        movzx   edx, byte [edx-1]
        test    ecx, 0x10000000
        jnz     .font2
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
        mov     ecx, [esp+4+20h+1Ch]
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
        jmp     .loop
.font2:
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
        mov     ecx, [esp+12+20h+1Ch]
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
        jmp     .loop
.end:
        popad
        pop  eax                ; << // Alver 22.06.2008 // <<
        ret
