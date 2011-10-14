;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2008. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Detect all BIOS hard drives.
; diamond, 2008

        xor     cx, cx
        mov     es, cx
        mov     di, 0x9080
        mov     byte [es:di-1], cl
        cmp     [preboot_biosdisk], 1
        jnz     bdde
        mov     dl, 80h
bdds:
        mov     ah, 15h
        push    cx dx di
        int     13h
        pop     di dx cx
        jc      bddc
        test    ah, ah
        jz      bddc
        inc     cx
        mov     ah, 48h
        push    ds
        push    es
        pop     ds
        mov     si, 0xA000
        mov     word [si], 1Eh
        mov     ah, 48h
        int     13h
        pop     ds
        jc      bddc2
        inc     byte [es:0x907F]
        cmp     word [es:si], 1Eh
        jb      bddl
        cmp     word [es:si+1Ah], 0xFFFF
        jz      bddl
        mov     al, dl
        stosb
        push    ds
        lds     si, [es:si+1Ah]
        mov     al, [si+6]
        and     al, 0xF
        stosb
        mov     al, byte [si+4]
        shr     al, 4
        and     ax, 1
        cmp     word [si], 1F0h
        jz      @f
        inc     ax
        inc     ax
        cmp     word [si], 170h
        jz      @f
        or      ax, -1
;        mov     ax, -1
@@:
        stosw
        pop     ds
        jmp     bddc2
bddl:
        mov     al, dl
        stosb
        xor     ax, ax
        stosb
        dec     ax
        stosw
;        mov     al, 0
;        stosb
;        mov     ax, -1
;        stosw
bddc2:
        cmp     cl, [es:0x475]
        jae     bdde
bddc:
        inc     dl
        jnz     bdds
bdde:
