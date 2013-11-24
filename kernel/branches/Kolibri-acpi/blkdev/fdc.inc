;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


uglobal
  dmasize         db 0x0
  dmamode         db 0x0
endg

fdc_init:                               ;start with clean tracks.
        mov     edi, OS_BASE+0xD201
        mov     al, 0
        mov     ecx, 160
        rep stosb
        ret

save_image:
        cmp     [ramdisk_actual_size], FLOPPY_CAPACITY
        jnz     .fail
        pusha
        mov     ecx, floppy_mutex
        call    mutex_lock
        mov     [flp_number], bl
        call    floppy_read_bootsector
        cmp     [FDC_Status], 0
        jne     .unnecessary_save_image
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 0; Сторона
        mov     [FDD_Sector], 1; Сектор
        mov     esi, RAMDISK
        call    SeekTrack
.save_image_1:
        call    take_data_from_application_1
        call    WriteSectWithRetr
;    call   WriteSector
        cmp     [FDC_Status], 0
        jne     .unnecessary_save_image
        inc     [FDD_Sector]
        cmp     [FDD_Sector], 19
        jne     .save_image_1
        mov     [FDD_Sector], 1
        inc     [FDD_Head]
        cmp     [FDD_Head], 2
        jne     .save_image_1
        mov     [FDD_Head], 0
        inc     [FDD_Track]
        call    SeekTrack
        cmp     [FDD_Track], 80
        jne     .save_image_1
.unnecessary_save_image:
        cmp     [FDC_Status], 0
        pushf
        mov     ecx, floppy_mutex
        call    mutex_unlock
        popf
        popa
        jnz     .fail
        xor     eax, eax
        ret
.fail:
        movi    eax, 1
        ret
