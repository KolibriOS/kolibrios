;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2009. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Query physical memory map from BIOS.
; diamond, 2009

        push    ds
; first call to fn E820
        mov     eax, 0xE820
        xor     ebx, ebx
        mov     es, bx
        mov     ds, bx
        mov     di, 0x9104
        mov     [di-4], ebx ; no blocks yet
        mov     ecx, 20
        mov     edx, 0x534D4150
        int     15h
        jc      no_E820
        cmp     eax, 0x534D4150
        jnz     no_E820
e820_mem_loop:
        cmp     byte [di+16], 1 ; ignore non-free areas
        jnz     e820_mem_next
        inc     byte [0x9100]
        add     di, 20
e820_mem_next:
; consequent calls to fn E820
        test    ebx, ebx
        jz      e820_test_done
        cmp     byte [0x9100], 32
        jae     e820_test_done
        mov     eax, 0xE820
        int     15h
        jc      e820_test_done
        jmp     e820_mem_loop
no_E820:
; let's hope for mem_test from init.inc
e820_test_done:
        pop     ds
