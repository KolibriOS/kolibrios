;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;;                  16 BIT ENTRY FROM BOOTSECTOR                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

os_code = code_l - tmp_gdt
if defined pretest_build
  PREBOOT_TIMEOUT = 0 ; seconds
else
  PREBOOT_TIMEOUT = 5
end if

use16
org 0
        ; struct kernel_header
        jmp     start_of_code   ; jump
        db      'KolibriOS '    ; signature
        db      'v0.7.7.0'      ; FIXME: get distribution version from git tag?
.offset db      '        ',13,10,0
        dd      B32-KERNEL_BASE ; offset of the kernel's 32-bit entry point

cur_line_pos = 0
offset_len = 0
__x = BUILD_OFFSET
if __x > 0
  store byte '+' at .offset+cur_line_pos
  if __x < 10
    offset_len = 2  ; '+' and a digit
  else if __x < 100
    offset_len = 3
  else if __x < 1000
    offset_len = 4
  else if __x < 10000
    offset_len = 5
  else
    offset_len = 6
  end if
  cur_line_pos = cur_line_pos + offset_len - 1
  while __x > 0
    store byte __x mod 10 + '0' at .offset+cur_line_pos
    cur_line_pos = cur_line_pos - 1
    __x = __x / 10
  end while
end if
if BUILD_DBGTAG > 0
  store byte '-' at .offset+offset_len
  store byte BUILD_DBGTAG at .offset+offset_len+1
end if

include "boot/bootstr.inc"      ; language-independent boot messages
include "boot/preboot.inc"

; Language support for locales: de_DE, es_ES, ru_RU (CP866), et_EE, en_US.
if lang eq de_DE
        include "boot/bootstr-de_DE.inc"        ; German system boot messages
else if lang eq es_ES
        include "boot/bootstr-es_ES.inc"        ; Spanish system boot messages
else if lang eq ru_RU
        include "boot/bootstr-ru_RU.inc"        ; Russian system boot messages
        include "boot/bootfont-ru_RU.inc"       ; Russian font
else if lang eq et_EE
        include "boot/bootstr-et_EE.inc"        ; Estonian system boot messages
        include "boot/bootfont-et_EE.inc"       ; Estonian font
else
        include "boot/bootstr-en_US.inc"        ; English system boot messages (default)
end if

include "boot/bootcode.inc"     ; 16 bit system boot code
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
        dd     tmp_gdt+KERNEL_BASE
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

if ~ lang eq es_ES
diff16 "end of bootcode",0,$+KERNEL_BASE
end if
