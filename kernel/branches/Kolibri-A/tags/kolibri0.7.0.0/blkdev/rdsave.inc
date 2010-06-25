$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
        mov     eax, saverd_fileinfo
        mov     [saverd_fileinfo.name], ebx
        pushad
        push    eax
        call    file_system_lfn
        pop     eax
        popad
        mov     [esp+36], eax
        ret
