;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


flm               db 0
preboot_lfb       db 0
preboot_bootlog   db 0
boot_drive        db 0
_bx_from_load:     dw 'r1'  ; структура для хранения параметров- откуда гашрузились, берется ниже из bx ; {SPraid}[13.03.2007]
                           ; a,b,c,d - винчестеры, r - рам диск
                           ; # диска... символ, а не байт. '1', а не 1

align 4
old_ints_h:
    dw      0x400
    dd      0
    dw      0

kernel_restart_bootblock:
    db    1    ; version
    dw    1    ; floppy image is in memory
    dd    0    ; cannot save parameters

; table for move to extended memory (int 15h, ah=87h)
align 8

fwmovedesc:
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0
        db      0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0

        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

