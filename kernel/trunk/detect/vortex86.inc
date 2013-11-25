;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; 20/11/2013 yogev_ezra: Initial version                       ;;
;; Thanks for help to: dunkaist, eAndrew, hidnplayr, Mario      ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

VORTEX86DEBUG = 0                       ; For testing in emulators and in non-Vortex86 CPU computers, set this to 1
VORTEX86DEBUGVALUE = 0x35504d44         ; FAKE port output = used for testing

; Detect Vortex86 CPU and generate CPU name in string format (PCI address at 93H~90H in Vortex86 North Bridge contains SoC type)
; Available Vortex86 CPU codes taken from Coreboot project. New codes should be added to "Vortex86SoClist" below
; #define DMP_CPUID_SX      0x31504d44  ("DMP1")
; #define DMP_CPUID_DX      0x32504d44  ("DMP2")
; #define DMP_CPUID_MX      0x33504d44  ("DMP3")
; #define DMP_CPUID_DX2     0x34504d44  ("DMP4")
; #define DMP_CPUID_MX_PLUS 0x35504d44  ("DMP5")
; #define DMP_CPUID_EX      0x37504d44  ("DMP7")

iglobal
Vortex86CPUcode dd ?                    ; Vortex86 CPU code in HEX format (4 bytes), can be shown as string if converted to ASCII characters
Vortex86CPUid   db 0                    ; Vortex86 CPU id in integer format (1=Vortex86SX, 2=Vortex86DX, ...)
Vortex86SoCname db 'Vortex86   ',0      ; This variable will hold the full name of Vortex86 SoC
Vortex86SoClist:                        ; List of Vortex86 CPUs known today. Add new record to this list when new CPU becomes available
        db      0x31, 'SX '     ; id=1
        db      0x32, 'DX '     ; id=2
        db      0x33, 'MX '     ; id=3
        db      0x34, 'DX2'     ; id=4
        db      0x35, 'MX+'     ; id=5
        db      0x37, 'EX '     ; id=6
Vortex86SoCnum = ($ - Vortex86SoClist) / 4      ; Calculate the total number of known Vortex86 CPUs (if id=Vortex86SoCnum+1 --> unknown SoC)
endg

; When in debug mode, perform SoC detection regardless of the actual CPU vendor (even for vendors other than DMP)
; When in normal (not debug) mode, check the CPU vendor first, and perform SoC detection only if vendor is 'Vortex86 SoC'
if ~ VORTEX86DEBUG        
        cmp     [cpu_vendor], 'Vort'
        jnz     .Vortex86end            ; If the CPU vendor is not 'Vortex86 SoC', skip the SoC detection
end if        

        mov     dx, 0xcf8               ; CF8h = Vortex86 PCI Configuration Address port
        mov     eax, OS_BASE+0x90       ; 0x80000090 = Starting PCI address to read from (32-bit register - accessed as DWORD)
        out     dx, eax                 ; Send request to PCI address port to retrieve data from this address
        mov     dx, 0xcfc               ; CFCh = Vortex86 PCI Configuration Data port
        in      eax, dx                 ; Read data (SoC type) from PCI data port

if VORTEX86DEBUG                        ; When in debug mode, pretend that we received port output equal to "VORTEX86DEBUGVALUE"
        mov     eax, VORTEX86DEBUGVALUE
end if        
        
        DEBUGF  1, "K : Vortex86 SoC register returned 0x"
        test    eax, eax                ; Check whether the port output was '\0'
        jz      .nullPCIoutput          ; In case the result is '\0' (NULL), skip further testing and exit
        mov     [Vortex86CPUcode], eax                      ; Save HEX CPU code to Vortex86CPUcode (so it can be used later)
        DEBUGF  1, "%x (%s): ", eax, Vortex86CPUcode        ; Print the CPU code (as HEX and as string) to debug log

        mov     ebx, 0x444d5000         ; Apply Vortex86 CPU code mask (all Vortex86 SoC have ID in form of "0xNN504d44")
        bswap   eax                     ; Assumed it is Vortex86 SoC, the highest byte identifies the exact CPU, so move it to the lowest byte
        mov     bl, al                  ; Copy SoC type to BL since EAX (that includes AL) is used implicitly in "LODSD" command below
        cmp     eax, ebx                ; Now see whether the 3 higher bytes were "0x504d44" (which means it's Vortex86)
        jnz     .notVortex86            ; Sorry, it's not Vortex86 - go say so and exit

        mov     esi, Vortex86SoClist    ; ESI points to the start of Vortex86SoClist (used implicitly in "LODSD" command below)
        xor     ecx, ecx                ; Zero ECX (it is used as counter)
        cld                             ; Clears the DF flag in the EFLAGS register (DF=0 --> String operations increment ESI)
@@:
        inc     ecx                     ; Increment our counter
        cmp     ecx, Vortex86SoCnum     ; Check if we iterated Vortex86SoCnum times already (i.e. went over the entire Vortex86SoClist)
        ja      .unknownVortex86        ; If the entire list was tested and our CPU is not in that list, it is unknown Vortex86 SoC
        lodsd                           ; Load DWORD at address DS:ESI into EAX (puts 1 line from Vortex86SoClist into EAX, then increments ESI)
        cmp     bl, al                  ; Check if our CPU matches the current record in the list
        jne     @b                      ; No match --> repeat with next record

        shr     eax, 8                              ; Match found --> drop the SoC type code from Vortex86SoClist name and replace it with \0
        mov     dword [Vortex86SoCname+8], eax      ; Concatenate it with prefix to receive complete SoC name (\0 is string termination)
        mov     [Vortex86CPUid], cl                 ; Save the CPUid (1=Vortex86SX, 2=Vortex86DX, ..., Vortex86SoCnum+1=Unknown Vortex86)
        DEBUGF  1, "%s (id=%d)\n", Vortex86SoCname, [Vortex86CPUid]:1               ; Say what we have found (CPU name and id)
        jmp     .Vortex86end

.notVortex86:                           ; In case this register is used by other CPUs for other purpose, it's interesting what it contains
        DEBUGF  1, "not a Vortex86 CPU\n"
        jmp     .Vortex86end

.unknownVortex86:
        mov     [Vortex86CPUid], cl                 ; Save the CPUid (Vortex86SoCnum+1=Unknown Vortex86)
        DEBUGF  1, "unknown Vortex86 CPU (id=%d, last known is %d)\n", [Vortex86CPUid]:1, Vortex86SoCnum
        jmp     .Vortex86end

.nullPCIoutput:                         ; Emulators and non-Vortex86 CPU computers will usually return \0 in this register
        DEBUGF  1, "0 (NULL)\n"

.Vortex86end: