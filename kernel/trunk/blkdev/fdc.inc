;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


iglobal
  ;function pointers.
  fdc_irq_func    dd fdc_null
endg

uglobal
  dmasize         db 0x0
  dmamode         db 0x0
endg

fdc_init:                               ;start with clean tracks.
        mov edi,OS_BASE+0xD201
        mov al,0
        mov ecx,160
        rep stosb
ret

fdc_filesave:                   ;ebx: cluster to be saved.
        pusha                   ;returns immediately. does not trigger a write.
        mov eax,ebx
        add eax,31
        mov bl,18
        div bl
        mov ah,0
        add eax,OS_BASE+0xD201
        mov [eax],byte 1                ;This track is now dirty.
        popa
ret

fdc_irq:
        call [fdc_irq_func]
fdc_null:
ret

save_image:
    call   reserve_flp
    call   restorefatchain
    pusha
    call   check_label
    cmp    [FDC_Status],0
    jne    unnecessary_save_image
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],0      ; Сторона
    mov    [FDD_Sector],1      ; Сектор
    mov    esi,RAMDISK
    call   SeekTrack
save_image_1:
    push   esi
    call   take_data_from_application_1
    pop    esi
    add    esi,512
    call   WriteSectWithRetr
;    call   WriteSector
    cmp    [FDC_Status],0
    jne    unnecessary_save_image
    inc    [FDD_Sector]
    cmp    [FDD_Sector],19
    jne    save_image_1
    mov    [FDD_Sector],1
    inc    [FDD_Head]
    cmp    [FDD_Head],2
    jne    save_image_1
    mov    [FDD_Head],0
    inc    [FDD_Track]
    call   SeekTrack
    cmp    [FDD_Track],80
    jne    save_image_1
unnecessary_save_image:
    mov    [fdc_irq_func],fdc_null
    popa
    mov    [flp_status],0
    ret

