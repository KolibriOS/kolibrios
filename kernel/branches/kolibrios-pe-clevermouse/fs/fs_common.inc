;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

fsReadCMOS:
        out     70h, al
        in      al, 71h
        xor     ah, ah
        shl     ax, 4
        shr     al, 4
        aad
        ret

fsGetTime:
        mov     al, 7
        call    fsReadCMOS
        ror     eax, 8
        mov     al, 8
        call    fsReadCMOS
        ror     eax, 8
        mov     al, 9
        call    fsReadCMOS
        add     eax, 2000
        ror     eax, 16
        push    eax
        xor     eax, eax
        call    fsReadCMOS
        ror     eax, 8
        mov     al, 2
        call    fsReadCMOS
        ror     eax, 8
        mov     al, 4
        call    fsReadCMOS
        ror     eax, 16
        push    eax
        mov     esi, esp
        add     esp, 8
fsCalculateTime:
; in: esi -> data block
; out: eax = seconds since 01.01.2001
        movzx   eax, word [esi+6]
        sub     eax, 2001
        jnc     @f
        xor     eax, eax
@@:
        mov     edx, months
        mov     ebx, eax
        inc     eax
        test    eax, 3
        jnz     @f
        add     edx, 12
@@:
        movzx   eax, byte [esi+5]
        dec     eax
        xor     ecx, ecx
@@:
        dec     eax
        js      @f
        add     cl, [edx+eax]
        adc     ch, 0
        jmp     @b
@@:
        mov     eax, ebx    ; years
        mov     edx, 365
        mul     edx
        shr     ebx, 2
        add     eax, ebx
        add     eax, ecx
        mov     bl, [esi+4]
        dec     eax
        add     eax, ebx    ; days
        mov     dl, 24
        mul     edx
        mov     bl, [esi+2]
        add     eax, ebx    ; hours
        mov     ecx, 60
        mul     ecx
        mov     bl, [esi+1]
        add     eax, ebx    ; minutes
        mul     ecx
        mov     bl, [esi]
        add     eax, ebx
        ret

iglobal
months  db  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
months2 db  31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
endg

fsTime2bdfe:
; in: eax = seconds since 01.01.2001
; edi -> data block
; out: edi = edi+8
        xor     edx, edx
        mov     ecx, 60
        div     ecx
        mov     [edi], dl
        xor     edx, edx
        div     ecx
        mov     [edi+1], dl
        xor     edx, edx
        mov     cl, 24
        div     ecx
        mov     [edi+2], dx
        xor     edx, edx
        mov     cx, 365
        div     ecx
        mov     ebx, eax
        add     ebx, 2001
        shr     eax, 2
        sub     edx, eax
        jns     @f
        dec     ebx
        add     edx, 365
        test    ebx, 3
        jnz     @f
        inc     edx
@@:
        xor     eax, eax
        mov     ecx, months-1
        test    ebx, 3
        jnz     @f
        add     ecx, 12
@@:
        inc     ecx
        inc     eax
        sub     dl, [ecx]
        jnc     @b
        dec     dh
        jns     @b
        add     dl, [ecx]
        inc     edx
        mov     [edi+4], dl
        mov     [edi+5], al
        mov     [edi+6], bx
        add     edi, 8
        ret
