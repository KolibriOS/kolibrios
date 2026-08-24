;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


iglobal
saverd_fileinfo:
        dd      2               ; subfunction: write
        dd      0               ; (reserved)
        dd      0               ; (reserved)
.size:
        dd      0
        dd      RAMDISK
        db      0
.name:
        dd      ?
endg
sysfn_saveramdisk:      ; 18.6 = SAVE RAMDISK IMAGE
; ecx -> path to the image file; ecx = 0 - write the image back to the
; medium it was booted from (a floppy or the file found by rdload.inc)
        test    ecx, ecx
        jnz     .have_path
if ~ defined extended_primary_loader
        mov     ecx, read_image_fsinfo.name
        cmp     [BOOT.rd_load_from], RD_LOAD_FROM_HD
        je      .have_path
end if
        movi    eax, ERROR_UNSUPPORTED_FS
        cmp     [BOOT.rd_load_from], RD_LOAD_FROM_FLOPPY
        jne     .done
        mov     ebx, 1
        call    save_image
        imul    eax, ERROR_DEVICE       ; 0 -> 0, 1 -> ERROR_DEVICE
        jmp     .done
.have_path:
        mov     ebx, saverd_fileinfo
        mov     [ebx+21], ecx
        mov     eax, [ramdisk_actual_size]
        shl     eax, 9
        mov     [ebx+12], eax
        pushad
        call    file_system_lfn_protected ;in ebx
        popad
.done:
        mov     [esp+32], eax
        ret
