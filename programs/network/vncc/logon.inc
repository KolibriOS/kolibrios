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

logon:
        mcall   40, EVM_MOUSE + EVM_MOUSE_FILTER + EVM_REDRAW + EVM_BUTTON + EVM_KEY

  .redraw:
        mcall   12, 1           ; start window draw
        pusha
                                                ; DRAW WINDOW
        xor     eax, eax                        ; function 0 _logon: define and draw window
        mov     ebx, 160 shl 16 + 330           ; [x start]:[x size]
        mov     ecx, 160 shl 16 + 100           ; [y start]:[y size]
        mov     edx, 0x34DDDDDD                 ; color of work area RRGGBB
        mov     edi, name                       ; WINDOW LABEL
        mcall

        mov     eax, 8                          ; LOGON BUTTON
        mov     ebx, 220 shl 16 + 85
        mov     ecx, 47 shl 16 + 16
        mov     edx, 2
        mov     esi, 0xCCCCCC
        mcall

        cmp     byte[mode], 0
        je      .servermode

        mov     eax, 4                          ; function 4 write text to window
        mov     ebx, 25 shl 16 + 15             ; [x start]:[y start]
        xor     ecx, ecx
        mov     edx, userstr                    ; pointer to text beginning
        mov     esi, passstr-userstr            ; text length
        mcall

        add     bl, 19
        mov     edx, passstr                    ; pointer to text beginning
        mov     esi, connectstr-passstr         ; text length
        mcall

        jmp     .drawtherest

   .servermode:
        mov     eax, 4                          ; function 4 write text to window
        mov     ebx, 25 shl 16 + 15             ; [x start] *65536 + [y start]
        xor     ecx, ecx
        mov     edx, serverstr                  ; pointer to text beginning
        mov     esi, userstr-serverstr          ; text length
        mcall

        invoke  edit_box_draw, URLbox

   .drawtherest:
        mov     ebx, 240 shl 16 + 49            ; [x start] *65536 + [y start]
        mov     edx, connectstr                 ; pointer to text beginning
        mov     esi, connect_e-connectstr       ; text length
        mcall

        popa
        inc     ebx
        mcall

  .loop:
        mcall   10                      ; wait for event
        dec     eax             ; redraw request ?
        jz      .redraw
        dec     eax             ; key in buffer ?
        jz      .key
        dec     eax             ; button in buffer ?
        jz      .btn
        sub     eax, 3
        jz      .mouse
        jmp     .loop

  .key:                         ; key event handler
        mcall   2               ; read key

        test    [URLbox.flags], ed_focus
        jz      mainloop
        cmp     ah, 13          ; enter (return) key
        je      .go
        invoke  edit_box_key, URLbox
        jmp     .loop

  .go:
        mov     eax, [URLbox.pos]
        mov     byte[serveraddr+eax], 0
        ret

  .btn:
        mcall   17              ; get id

        cmp     ah, 1           ; close ?
        jz      .close
        cmp     ah, 2           ; logon ?
        je      .go

        jmp     .loop

  .mouse:
        mcall   23
        invoke  edit_box_mouse, URLbox

        jmp     .loop

  .close:
        mcall   -1


; DATA AREA

passchar      db "*"
passlen       dd 0

addr          dd 0
temp          dd 0
mode          db 0              ; 0 = connection details, 1 = authentication

serverstr     db "server:"
userstr       db "username:"
passstr       db "password:"
connectstr    db "connect !"
connect_e:

I_END_logon:
