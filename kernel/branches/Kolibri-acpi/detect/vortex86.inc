;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; 20/11/2013 yogev_ezra: Initial version (Vortex86 SoC type detection)
; 26/11/2013 yogev_ezra: Added CPU speed modifier and MMX support flag detection
; Thanks for help to: dunkaist, eAndrew, hidnplayr, Mario

$Revision: 4310 $

VORTEX86DEBUG = 0                       ; For testing in emulators and in non-Vortex86 CPU computers, set this to 1
VORTEX86DEBUGVALUE = 0x35504d44         ; FAKE port output = used for testing
NORTH_BRIDGE = 0x80000000               ; Base address of Vortex86 PCI North Bridge
SOUTH_BRIDGE = 0x80003800               ; Base address of Vortex86 PCI South Bridge

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
        db      0x33, 'MX '     ; id=3  MMX is available starting from CPU code 'MX' (id=3)
        db      0x34, 'DX2'     ; id=4
        db      0x35, 'MX+'     ; id=5
        db      0x37, 'EX '     ; id=7
Vortex86SoCnum = ($ - Vortex86SoClist) / 4      ; Calculate the total number of known Vortex86 CPUs
endg

; When in debug mode, perform SoC detection regardless of the actual CPU vendor (even for vendors other than DMP)
; When in normal (not debug) mode, check the CPU vendor first, and perform SoC detection only if vendor is 'Vortex86 SoC'
if ~ VORTEX86DEBUG        
        cmp     [cpu_vendor], 'Vort'
        jnz     .Vortex86end            ; If the CPU vendor is not 'Vortex86 SoC', skip the SoC detection
end if

        mov     eax, NORTH_BRIDGE+0x90  ; 0x80000090 = PCI Configuration Address Register to read from (32-bit register - accessed as DWORD)
        call    .Vortex86PCIreg         ; Get the CPU code from Vortex86 SoC North Bridge PCI register (Register Offset: 93H~90H)

if VORTEX86DEBUG                        ; When in debug mode, pretend that we received port output equal to "VORTEX86DEBUGVALUE"
        mov     eax, VORTEX86DEBUGVALUE
end if
        
        DEBUGF  1, "K : Vortex86 SoC type register (93H~90H) returned 0x"
        test    eax, eax                ; Check whether the port output was '\0'
        jz      .nullPCIoutput          ; In case the result is '\0' (NULL), skip further testing and exit
        mov     [Vortex86CPUcode], eax                      ; Save HEX CPU code to Vortex86CPUcode (so it can be used later)
        DEBUGF  1, "%x (%s): ", eax, Vortex86CPUcode        ; Print the CPU code (as HEX and as string) to debug log

        mov     ebx, 0x444d5000         ; Apply Vortex86 CPU code mask (all Vortex86 SoC have ID in form of "0xNN504d44")
        bswap   eax                     ; Assumed it is Vortex86 SoC, the highest byte identifies the exact CPU, so move it to the lowest byte
        mov     bl, al                  ; Copy SoC type to BL since EAX (that includes AL) is used implicitly in "LODSD" command below
        cmp     eax, ebx                ; Now see whether the 3 higher bytes were "0x504d44" (which means it's Vortex86)
        jnz     .notVortex86            ; If it's not Vortex86 - go say so and exit

        sub     al, 0x30                ; Current Vortex86 CPU codes are in the range of 31h-37h, so convert them to integer (1,2,...)
        mov     [Vortex86CPUid], al     ; Save the CPUid (1=Vortex86SX, 2=Vortex86DX, ..., 7=Vortex86EX, ...)
        
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

        DEBUGF  1, "%s (id=%d)\n", Vortex86SoCname, [Vortex86CPUid]:1           ; Say what we have found (CPU name and id)
        jmp     .Vortex86

.notVortex86:                           ; In case this register is used by other CPUs for other purpose, it's interesting what it contains
        DEBUGF  1, "not a Vortex86 CPU\n"
        jmp     .Vortex86end

.unknownVortex86:                       ; It is Vortex86 CPU, but it's not in the list above
        DEBUGF  1, "unknown Vortex86 CPU (id=%d)\n", [Vortex86CPUid]:1          ; Inform the user that the CPU is Vortex86 but name is unknown

.Vortex86:
        mov     eax, NORTH_BRIDGE+0x60  ; 0x80000060 = PCI Configuration Address Register to read from (32-bit register - accessed as DWORD)
        call    .Vortex86PCIreg         ; Get current flags of Vortex86SoC North Bridge STRAP Register (Register Offset: 63h~60h)
        DEBUGF  1, "K : Vortex86 STRAP Register (63h~60h) returned 0x%x\n",eax

        mov     eax, SOUTH_BRIDGE+0xC0  ; 0x800038C0 = PCI Configuration Address Register to read from (32-bit register - accessed as DWORD)
        call    .Vortex86PCIreg         ; Flags of Vortex86 South Bridge Internal Peripheral Feature Control Register (Register Offset: C3h~C0h)
        DEBUGF  1, "K : Vortex86 Internal Peripheral Feature Control Register (C3h~C0h) returned 0x%x\n",eax

        mov     eax, SOUTH_BRIDGE+0xCC  ; 0x800038CC = PCI Configuration Address Register to read from (8-bit register - accessed as BYTE)
        call    .Vortex86PCIreg         ; Flags of Vortex86 South Bridge Internal Peripheral Feature Control Register III (Register Offset: CCh)
        DEBUGF  1, "K : Vortex86 Internal Peripheral Feature Control Register III (CCh) returned 0x%x\n",al
        
        mov     eax, NORTH_BRIDGE+0xA0  ; 0x800000A0 = PCI Configuration Address Register to read from (32-bit register - accessed as DWORD)
        call    .Vortex86PCIreg         ; Get current flags of Vortex86SoC North Bridge Host Control Register (Register Offset: A3h~A0h)
        DEBUGF  1, "K : Vortex86 Host Control Register (A3h~A0h) returned 0x%x: CPU speed is ",eax
        mov     bl, al                  ; The lower byte of Vortex86 Host Control Register contains CPU speed modifier and MMX support status
        mov     bh, al                  ; Backup the current AL value, so later we can test whether the value has changed
        and     bl, 00000111b           ; CPU speed modifier is stored in bits 0-2. Value=0 means MAX speed, other values - speed reduction
        jz      .Vortex86CPUspeedMAX    ; 0s in bits 0-2: CPU is at MAX speed (no need to modify)
        inc     ebx                     ; The actual value is 1 less than 'Divide by' setting (value '001' means 'Divide by 2', etc.)
        DEBUGF  1, "reduced (divide by %d).\nK : Vortex86 changing CPU speed to ", bl    ; Print the current CPU speed modifier to the log
        and     al, 11111000b           ; At least one of the bits 0-2 contains 1: CPU is at reduced speed. Set bits 0-2 to 0s to change to MAX
.Vortex86CPUspeedMAX:
        DEBUGF  1, "MAX\n"              ; Now the CPU should be running at MAX speed (don't write the value to PCI port yet)

        cmp     [Vortex86CPUid], 3      ; MMX is available starting from CPU code 'MX' (id=3)
        jb      .skipVortex86MMX        ; No MMX support - skip MMX support status detection (for id=1,2)
        DEBUGF  1, "K : Vortex86 MMX support status: MMX is "                   ; Bits 5-6 in Host Control Register contain MMX status
        test    al, 100000b             ; On MMX-capable Vortex86 SoC, Bit5 = is MMX enabled? (1=Yes/0=No)
        jnz     .Vortex86MMXenabled     ; MMX is already enabled (Bit5=1)
        DEBUGF  1, "DISABLED - enabling it for this session\n"                  ; Print to the log that MMX is disabled
        or      al, 100000b             ; Enable MMX support (don't write the value to PCI port yet)
        jmp     .AfterMMXenabled
.Vortex86MMXenabled:
        DEBUGF  1, "ENABLED\n"          ; Print to the log that MMX is enabled
.AfterMMXenabled:
        DEBUGF  1, "K : Vortex86 MMX report to CPUID: "                         ; Print to the log what CPUID command knowns about MMX support
        test    al, 1000000b            ; On MMX-capable Vortex86 SoC, Bit6 = report MMX support to CPUID? (1=Yes/0=No)        
        jnz     .Vortex86MMXreported    ; MMX is already reported to CPUID (Bit6=1)
        DEBUGF  1, "OFF - turning it ON for this session\n"                     ; Print to the log that MMX will now be reported to CPUID
        or      al, 1000000b            ; Turn on MMX reporting to CPUID (don't write the value to PCI port yet)
        jmp     .skipVortex86MMX
.Vortex86MMXreported:
        DEBUGF  1, "ON\n"               ; Print to the log that MMX reporting to CPUID is enabled

.skipVortex86MMX:
        cmp     bh, al                  ; Check whether AL has changed before (if it did, we need to write it back to PCI port)
        jz      .Vortex86end            ; No change - no need to write to the port
        out     dx, al                  ; Write the changed data to PCI port
        DEBUGF  1, "K : Vortex86 Host Control Register (A3h~A0h) new value is 0x%x\n",eax
        jmp     .Vortex86end

.Vortex86PCIreg:                        ; Procedure receives input register value in EAX, and returns the output value also in EAX
        mov     dx, 0xcf8               ; CF8h = Vortex86 PCI Configuration Address port
        out     dx, eax                 ; Send request to PCI address port to retrieve data from this address
        mov     dl, 0xfc                ; CFCh = Vortex86 PCI Configuration Data port
        in      eax, dx                 ; Read data from PCI data port
        ret
        
.nullPCIoutput:                         ; Emulators and non-Vortex86 CPU computers will usually return \0 in this register
        DEBUGF  1, "0 (NULL)\n"

.Vortex86end: