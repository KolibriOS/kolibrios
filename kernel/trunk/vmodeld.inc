$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; Load of videomode driver in memory
;
; (driver is located at VMODE_BASE  - 32kb) // if this area not occuped anything
;
; Author: Trans
; Date:  19.07.2003
;
; Include in MeOS kernel and compile with FASM
;


; LOAD VIDEOMODE DRIVER
                                       ; If vmode.mdr file not found
        or    eax,-1                   ; Driver ID = -1 (not present in system)
        mov   [VMODE_BASE],eax   ;
        mov   [VMODE_BASE+0x100],byte 0xC3     ; Instruction RETN - driver loop

        mov     esi, vmode
        xor     ebx, ebx
        mov     ecx, 0x8000            ; size of memory area for driver
        mov     edx, VMODE_BASE        ; Memory position of driver
        call    fs_RamdiskRead
