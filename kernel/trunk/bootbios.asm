;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                  16 BIT ENTRY FROM BOOTSECTOR                        ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include 'macros.inc'
include 'struct.inc'
include 'lang.inc'
include 'encoding.inc'
include 'const.inc'

os_code = code_l - tmp_gdt
PREBOOT_TIMEOUT = 5   ; seconds

use16
                  org   0x0
        jmp     start_of_code

if lang eq sp
include "kernelsp.inc"  ; spanish kernel messages
else if lang eq et
version db    'Kolibri OS  versioon 0.7.7.0+    ',13,10,13,10,0
else
version db    'Kolibri OS  version 0.7.7.0+     ',13,10,13,10,0
end if

include "boot/bootstr.inc"     ; language-independent boot messages
include "boot/preboot.inc"

if lang eq ge
include "boot/bootge.inc"     ; german system boot messages
else if lang eq sp
include "boot/bootsp.inc"     ; spanish system boot messages
else if lang eq ru
include "boot/bootru.inc"      ; russian system boot messages
include "boot/ru.inc"          ; Russian font
else if lang eq et
include "boot/bootet.inc"      ; estonian system boot messages
include "boot/et.inc"          ; Estonian font
else
include "boot/booten.inc"      ; english system boot messages
end if

include "boot/bootcode.inc"    ; 16 bit system boot code
include "bus/pci/pci16.inc"
include "detect/biosdisk.inc"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                  SWITCH TO 32 BIT PROTECTED MODE                     ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CR0 Flags - Protected mode and Paging

        mov     ecx, CR0_PE+CR0_AM

; Enabling 32 bit protected mode

        sidt    [cs:old_ints_h]

        cli                             ; disable all irqs
        cld
        mov     al, 255                 ; mask all irqs
        out     0xa1, al
        out     0x21, al
   l.5:
        in      al, 0x64                ; Enable A20
        test    al, 2
        jnz     l.5
        mov     al, 0xD1
        out     0x64, al
   l.6:
        in      al, 0x64
        test    al, 2
        jnz     l.6
        mov     al, 0xDF
        out     0x60, al
   l.7:
        in      al, 0x64
        test    al, 2
        jnz     l.7
        mov     al, 0xFF
        out     0x64, al

        lgdt    [cs:tmp_gdt]            ; Load GDT
        mov     eax, cr0                ; protected mode
        or      eax, ecx
        and     eax, 10011111b *65536*256 + 0xffffff ; caching enabled
        mov     cr0, eax
        jmp     pword os_code:B32       ; jmp to enable 32 bit mode

align 8
tmp_gdt:

        dw     23
        dd     tmp_gdt+0x10000
        dw     0
code_l:
        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10011010b
        db     0x00

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10010010b
        db     0x00

include "data16.inc"

if ~ lang eq sp
diff16 "end of bootcode",0,$+0x10000
end if

use32
org $+0x10000

align 4
B32:
