;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


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
sysfn_saveramdisk:      ; 18.6 = SAVE FLOPPY IMAGE (HD version only)
        mov     ebx, saverd_fileinfo
        mov     [ebx+21], ecx
        mov     eax, [ramdisk_actual_size]
        shl     eax, 9
        mov     [ebx+12], eax
        pushad
        call    file_system_lfn_protected ;in ebx
        popad
        mov     [esp+32], eax
        ret
