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


; LOAD VIDEOMODE DRIVER
                                       ; If vmode.mdr file not found
        or    eax,-1                   ; Driver ID = -1 (not present in system)
        mov   [0x760000],eax           ;
        mov   [0x760100],byte 0xC3     ; Instruction RETN - driver loop

        mov     esi, vmode
        xor     ebx, ebx
        mov     ecx, 0x8000            ; size of memory area for driver
        mov     edx, 0x760000          ; Memory position of driver
        call    fs_RamdiskRead