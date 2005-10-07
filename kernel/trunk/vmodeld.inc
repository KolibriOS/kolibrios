;
; Load of videomode driver in memory
; 
; (driver is located at 0x760000-0x768000  - 32kb) // if this area not occuped anything
;
; Author: Trans
; Date:  19.07.2003
;
; Include in MeOS kernel and compile with FASM
;


;vmode db 'VMODE   MDR'  ; MDR - Menuet Driver 
; must be located after fonts filenames in kernel.asm

; LOAD VIDEOMODE DRIVER
                                       ; If vmode.mdr file not found
        or    eax,-1                   ; Driver ID = -1 (not present in system)
        mov   [0x760000],eax           ;
        mov   [0x760100],byte 0xC3     ; Instruction RETN - driver loop

        mov   eax,vmode                ; File name of driver
        mov   esi,12
        mov   ebx,0
        mov   ecx,26000
        mov   edx,0x760000             ; Memory position of driver
        call  fileread