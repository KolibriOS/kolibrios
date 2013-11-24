;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;***************************************************
;      предварительная очистка области таблицы
;      поиск и занесение в таблицу приводов FDD
;      автор Mario79
;***************************************************
        xor     eax, eax
        mov     edi, DRIVE_DATA
        mov     ecx, DRIVE_DATA_SIZE/4
        cld
        rep stosd

        mov     al, 0x10
        out     0x70, al
        mov     cx, 0xff
wait_cmos:
        dec     cx
        test    cx, cx
        jnz     wait_cmos
        in      al, 0x71
        mov     [DRIVE_DATA], al
        test    al, al
        jz      @f

        stdcall attach_int_handler, 6, FDCInterrupt, 0
        DEBUGF  1, "K : Set IDE IRQ6 return code %x\n", eax
        call    floppy_init
@@:

