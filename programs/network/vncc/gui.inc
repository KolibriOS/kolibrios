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

draw_gui:
        mcall   40, EVM_MOUSE + EVM_MOUSE_FILTER + EVM_REDRAW + EVM_BUTTON + EVM_KEY

  .redraw:
        mcall   12, 1                           ; start window draw
                                                ; DRAW WINDOW
        xor     eax, eax                        ; function 0 _logon: define and draw window
        mov     ebx, 160 shl 16 + 330           ; [x start]:[x size]
        mov     ecx, 160 shl 16 + 100           ; [y start]:[y size]
        mov     edx, 0x34DDDDDD                 ; color of work area RRGGBB
        mov     edi, name                       ; WINDOW LABEL
        mcall

        cmp     [status], STATUS_CONNECTING
        ja      @f

        mov     ebx, 25 shl 16 + 24
        xor     ecx, ecx
        mov     edx, serverstr
        mov     esi, userstr-serverstr
        mcall   4                               ; "server" text

        invoke  edit_box_draw, URLbox           ; Server textbox

        cmp     [status], STATUS_INITIAL
        jne     .redraw_done

        mov     ebx, 220 shl 16 + 85
        mov     ecx, 47 shl 16 + 16
        mov     edx, 2
        mov     esi, 0xCCCCCC
        mcall   8                               ; Connect button

        mov     ebx, 240 shl 16 + 52
        mov     edx, connectstr
        mov     esi, loginstr-connectstr
        mcall   4                               ; Connect button text

        jmp     .redraw_done

  @@:
        cmp     [status], STATUS_LOGIN
        ja      @f

        mov     ebx, 25 shl 16 + 14
        xor     ecx, ecx
        mov     edx, userstr
        mov     esi, passstr-userstr
        mcall   4                               ; "user" text

        add     bl, 20
        mov     edx, passstr
        mov     esi, connectstr-passstr         ; "password" text
        mcall

        invoke  edit_box_draw, USERbox          ; username textbox
        invoke  edit_box_draw, PASSbox          ; password textbox

        cmp     [status], STATUS_REQ_LOGIN
        jne     .redraw_done

        mov     ebx, 220 shl 16 + 85
        mov     ecx, 47 shl 16 + 16
        mov     edx, 3
        mov     esi, 0xCCCCCC
        mcall   8                               ; Login button

        mov     ebx, 240 shl 16 + 52
        mov     edx, loginstr
        mov     esi, loginstr_e-loginstr
        mcall   4                               ; Login button text

        jmp     .redraw_done

  @@:
        cmp     [status], STATUS_DISCONNECTED
        jb      .redraw_done

        mov     ebx, 25 shl 16 + 15
        mov     ecx, 0x80ca0000                 ; red ASCIIZ string
        mov     edx, [status]
        sub     edx, 10
        mov     edx, [err_msg+4*edx]
        mcall   4                               ; print error message to window

  .redraw_done:
        mov     [update_gui], 0
        mcall   12, 2

  .loop:
        cmp     [update_gui], 0
        jne     .redraw
        cmp     [status], STATUS_CONNECTED
        je      .connected

        mcall   23, 10                          ; wait for event
        dec     eax                             ; redraw request ?
        jz      .redraw
        dec     eax                             ; key in buffer ?
        jz      .key
        dec     eax                             ; button in buffer ?
        jz      .btn
        sub     eax, 3
        jz      .mouse
        jmp     .loop

  .connected:
        ret

  .key:                                         ; key event handler
        mcall   2                               ; read key

        cmp     [status], STATUS_INITIAL
        jne     @f
        test    [URLbox.flags], ed_focus
        jz      mainloop
        cmp     ah, 13                          ; enter (return) key
        je      .connect
        invoke  edit_box_key, URLbox
  @@:
        cmp     [status], STATUS_REQ_LOGIN
        jne     .loop
        cmp     ah, 13                          ; enter (return) key
        je      .login
        invoke  edit_box_key, USERbox
        invoke  edit_box_key, PASSbox
  @@:
        jmp     .loop

  .btn:
        mcall   17                              ; get id

        cmp     ah, 1                           ; close ?
        jz      .close
        cmp     ah, 2                           ; connect ?
        je      .connect
        cmp     ah, 3                           ; login ?
        je      .login

        jmp     .loop

  .connect:
        mov     eax, [URLbox.pos]
        mov     byte[serveraddr+eax], 0

        mov     [status], STATUS_CONNECTING
        inc     [update_gui]

; Create network thread
        mcall   51, 1, thread_start, thread_stack
        cmp     eax, -1
        jne     @f
        mov     [status], STATUS_THREAD_ERR
;        inc     [update_gui]
  @@:
        jmp     .loop

  .login:
        mov     [status], STATUS_LOGIN
        inc     [update_gui]
        jmp     .loop

  .mouse:
        cmp     [status], STATUS_INITIAL
        jne     @f
        invoke  edit_box_mouse, URLbox
  @@:
        cmp     [status], STATUS_REQ_LOGIN
        jne     @f
        invoke  edit_box_mouse, USERbox
        invoke  edit_box_mouse, PASSbox
  @@:
        jmp     .loop

  .close:
        mov     [status], STATUS_CLOSED
        mcall   -1