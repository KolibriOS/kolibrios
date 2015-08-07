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

        mov     edi, keymap
        call    convert_keymap

        mov     edi, keymap_shift
        call    convert_keymap

        mov     edi, keymap_alt
        call    convert_keymap

        ret



convert_keymap:

        push    edi
        lea     esi, [edi+128]
        mov     ecx, 128
        xor     eax, eax
  .loop:
        lodsb
        shl     ax, 8
        stosw
        dec     ecx
        jnz     .loop
        pop     edi

; Fill in some keysyms for non-ascii keys
        mov     word[edi+01*2], 0x1bff  ; Escape
        mov     word[edi+14*2], 0x08ff  ; Backspace
        mov     word[edi+15*2], 0x09ff  ; Tab
        mov     word[edi+28*2], 0x0dff  ; Enter
        mov     word[edi+29*2], 0xe3ff  ; Left control key
        mov     word[edi+42*2], 0xe1ff  ; Left shift
        mov     word[edi+54*2], 0xe2ff  ; Right shift
        mov     word[edi+56*2], 0xe9ff  ; Left alt key
        mov     word[edi+59*2], 0xbeff  ; f1
        mov     word[edi+60*2], 0xbfff  ; f2
        mov     word[edi+61*2], 0xc0ff  ; f3
        mov     word[edi+62*2], 0xc1ff  ; f4
        mov     word[edi+63*2], 0xc2ff  ; f5
        mov     word[edi+64*2], 0xc3ff  ; f6
        mov     word[edi+65*2], 0xc4ff  ; f7
        mov     word[edi+66*2], 0xc5ff  ; f8
        mov     word[edi+67*2], 0xc6ff  ; f9
        mov     word[edi+68*2], 0xc7ff  ; f10
        mov     word[edi+71*2], 0x50ff  ; home
        mov     word[edi+72*2], 0x52ff  ; up
        mov     word[edi+73*2], 0x55ff  ; pg up
        mov     word[edi+75*2], 0x51ff  ; left
        mov     word[edi+77*2], 0x53ff  ; right
        mov     word[edi+79*2], 0x57ff  ; end
        mov     word[edi+80*2], 0x54ff  ; down
        mov     word[edi+81*2], 0x56ff  ; pg down
        mov     word[edi+82*2], 0x63ff  ; insert
        mov     word[edi+83*2], 0xffff  ; delete
        mov     word[edi+87*2], 0xc8ff  ; f11
        mov     word[edi+88*2], 0xc9ff  ; f12
        mov     word[edi+91*2], 0xebff  ; left super

        ret