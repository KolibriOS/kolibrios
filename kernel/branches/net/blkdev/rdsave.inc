;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision $


iglobal
saverd_fileinfo:
        dd      2               ; subfunction: write
        dd      0               ; (reserved)
        dd      0               ; (reserved)
        dd      1440*1024       ; size 1440 Kb
        dd      RAMDISK
        db      0
.name:
        dd      ?
endg
sysfn_saveramdisk:      ; 18.6 = SAVE FLOPPY IMAGE (HD version only)
        call    restorefatchain
        mov     ebx, saverd_fileinfo
        mov     [saverd_fileinfo.name], ecx
        pushad
        call    file_system_lfn ;in ebx
        popad
        mov     [esp+32], eax
        ret
