;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  VNC client for KolibriOS                                       ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

generate_keymap:

; Read keymaps from kernel
        mcall   26, 2, 1, keymap+128
        mcall   26, 2, 2, keymap_shift+128
        mcall   26, 2, 3, keymap_alt+128

        mov     esi, keymap+128
        mov     edi, keymap
        mov     ecx, 128
        call    convert_keymap

        mov     esi, keymap_shift+128
        mov     edi, keymap_shift
        mov     ecx, 128
        call    convert_keymap

        mov     esi, keymap_alt+128
        mov     edi, keymap_alt
        mov     ecx, 128
        call    convert_keymap

        ret



convert_keymap:
  .loop:
        lodsb

        cmp     al, 0x08        ; Backspace
        jne     @f
        mov     ax, 0x08ff
        jmp     .next
  @@:
        cmp     al, 0x09        ; Tab
        jne     @f
        mov     ax, 0x09ff
        jmp     .next
  @@:
        cmp     al, 0x0d        ; Enter
        jne     @f
        mov     ax, 0x0dff
        jmp     .next
  @@:
        cmp     al, 0x1b        ; Escape
        jne     @f
        mov     ax, 0x1bff
        jmp     .next
  @@:
        cmp     al, 0x34        ; Insert
        jne     @f
        mov     ax, 0x63ff
        jmp     .next
  @@:
        cmp     al, 0xb6        ; Delete
        jne     @f
        mov     ax, 0xffff
        jmp     .next
  @@:
        cmp     al, 0xb4        ; Home
        jne     @f
        mov     ax, 0x50ff
        jmp     .next
  @@:
        cmp     al, 0xb5        ; End
        jne     @f
        mov     ax, 0x57ff
        jmp     .next
  @@:
        cmp     al, 0xb8        ; PgUp
        jne     @f
        mov     ax, 0x55ff
        jmp     .next
  @@:
        cmp     al, 0xb7        ; PgDown
        jne     @f
        mov     ax, 0x56ff
        jmp     .next
  @@:
        cmp     al, 0xb0        ; Left
        jne     @f
        mov     ax, 0x51ff
        jmp     .next
  @@:
        cmp     al, 0xb2        ; Up
        jne     @f
        mov     ax, 0x52ff
        jmp     .next
  @@:
        cmp     al, 0xb3        ; Right
        jne     @f
        mov     ax, 0x53ff
        jmp     .next
  @@:
        cmp     al, 0xb1        ; Down
        jne     @f
        mov     ax, 0x54ff
        jmp     .next
  @@:

        shl     ax, 8
  .next:
        stosw
        dec     ecx
        jnz     .loop
        ret