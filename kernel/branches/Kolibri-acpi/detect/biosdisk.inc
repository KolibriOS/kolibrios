;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2008-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Detect all BIOS hard drives.
; diamond, 2008
; Do not include USB mass storages. CleverMouse, 2013

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
; We are going to call int 13h/func 48h, Extended get drive parameters.
; The latest version of the EDD specification is 3.0.
; There are two slightly incompatible variants for version 3.0;
; original one from Phoenix in 1998, see e.g.
; http://www.t10.org/t13/technical/d98120r0.pdf, and T13 draft,
; http://www.t13.org/documents/UploadedDocuments/docs2004/d1572r3-EDD3.pdf
; T13 draft addresses more possible buses, so it gives additional 8 bytes
; for device path.
; Most BIOSes follow Phoenix, but T13 version is also known to be used
; (e.g. systems based on AMD Geode).
; Fortunately, there is an in/out length field, so
; it is easy to tell what variant was selected by the BIOS:
; Phoenix-3.0 has 42h bytes, T13-3.0 has 4Ah bytes.
; Note that 2.0 has 1Eh bytes, 1.1 has 1Ah bytes; both variants of 3.0 have
; the same structure for first 1Eh bytes, compatible with previous versions.
; Note also that difference between Phoenix-3.0 and T13-3.0 starts near the
; end of the structure, so the current code doesn't even need to distinguish.
; It needs, however, give at least 4Ah bytes as input and expect that BIOS
; could return 42h bytes as output while still giving all the information.
        mov     ah, 48h
        push    ds
        push    es
        pop     ds
        mov     si, 0xA000
        mov     word [si], 4Ah
        mov     ah, 48h
        int     13h
        pop     ds
        jc      bddc2
        cmp     word [es:si], 1Eh
        jb      .noide
        cmp     word [es:si+1Ah], 0xFFFF
        jz      .noide
        inc     byte [es:0x907F]
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
.noide:
        cmp     word [es:si], 42h
        jb      .nousb
        cmp     word [es:si+28h], 'US'
        jnz     .nousb
        cmp     byte [es:si+2Ah], 'B'
        jz      bddc2
.nousb:
        inc     byte [es:0x907F]
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
