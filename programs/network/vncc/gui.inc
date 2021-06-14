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

BTN_H equ 22
TEXT_ECX equ 00010000b shl 24 + 0

draw_gui:
        mcall   67, 160, 160, 330, 100          ; resize and move the window
        mcall   66, 1, 0                        ; switch keyboard to ascii mode
  .first_time:
        mcall   40, EVM_MOUSE + EVM_MOUSE_FILTER + EVM_REDRAW + EVM_BUTTON + EVM_KEY

  .redraw:
        mcall   12, 1                           ; start window draw
                                                ; DRAW WINDOW
        xor     eax, eax                        ; function 0 _logon: define and draw window
        mov     ebx, 160 shl 16 + 330           ; [x start]:[x size]
        mov     ecx, 160 shl 16 + 130           ; [y start]:[y size]
        mov     edx, 0x34DDDDDD                 ; color of work area RRGGBB
        mov     edi, name                       ; WINDOW LABEL
        mcall

        cmp     [status], STATUS_CONNECTING
        ja      .login?

        mov     ebx, 10 shl 16 + 22
        mov     ecx, TEXT_ECX
        mov     edx, serverstr
        mov     esi, userstr-serverstr
        mcall   4                               ; "server" text
		
		mcall   4, <70, 45>, 0 shl 24 + 0x666666, port_notice, port_notice_e - port_notice

        invoke  edit_box_draw, URLbox           ; Server textbox

        cmp     [status], STATUS_CONNECTING
        jne     @f

        mov     ebx, 220 shl 16 + 85
        mov     ecx, 57 shl 16 + BTN_H
        mov     edx, 4
        mov     esi, 0xCCCCCC
        mcall   8                               ; Cancel button

        mov     ebx, 260 shl 16 + 62
		mov     ecx, TEXT_ECX
        mov     edx, cancelstr
        mov     esi, okstr-cancelstr
        mcall   4                               ; Cancel button text

        jmp     .redraw_done

  @@:
        cmp     [status], STATUS_CONNECT
        jne     .redraw_done

        mov     ebx, 220 shl 16 + 85
        mov     ecx, 57 shl 16 + BTN_H
        mov     edx, 2
        mov     esi, 0xCCCCCC
        mcall   8                               ; Connect button

        mov     ebx, 236 shl 16 + 62
		mov     ecx, TEXT_ECX
        mov     edx, connectstr
        mov     esi, loginstr-connectstr
        mcall   4                               ; Connect button text

        jmp     .redraw_done

  .login?:
        cmp     [status], STATUS_LOGIN
        ja      @f

        mov     ebx, 10 shl 16 + 12
        mov     ecx, TEXT_ECX
        mov     edx, userstr
        mov     esi, passstr-userstr
        mcall   4                               ; "user" text

        add     bl, 25
        mov     edx, passstr
        mov     esi, connectstr-passstr         ; "password" text
        mcall

        invoke  edit_box_draw, USERbox          ; username textbox
        invoke  edit_box_draw, PASSbox          ; password textbox

        cmp     [status], STATUS_REQ_LOGIN
        jne     .redraw_done

        mov     ebx, 220 shl 16 + 85
        mov     ecx, 67 shl 16 + BTN_H
        mov     edx, 3
        mov     esi, 0xCCCCCC
        mcall   8                               ; Login button

        mov     ebx, 120 shl 16 + 85
        mov     ecx, 67 shl 16 + BTN_H
        mov     edx, 4
        mov     esi, 0xCCCCCC
        mcall   8                               ; Cancel button

        mov     ebx, 240 shl 16 + 71
		mov     ecx, TEXT_ECX
        mov     edx, loginstr
        mov     esi, cancelstr-loginstr
        mcall   4                               ; Login button text
		
        mov     ebx, 140 shl 16 + 71
        mov     edx, cancelstr
        mov     esi, okstr-cancelstr
        mcall   4                               ; Cancel button text

        jmp     .redraw_done

  @@:
        cmp     [status], STATUS_DISCONNECTED
        jb      .redraw_done

        mov     ebx, 15 shl 16 + 10
        mov     ecx, 0x00ca0000                 ; red ASCIIZ string
        mov     edx, [status]
        sub     edx, 10
        mov     edx, [err_msg+4*edx]
  .restart:
        xor     esi, esi
  .pr_loop:
        cmp     byte[edx+esi], 0
        je      .last
        cmp     byte[edx+esi], 0x0a
        je      .print
        inc     esi
        jmp     .pr_loop
  .print:
        mcall   4
        add     edx, esi
        inc     edx
        add     ebx, 10
        jmp     .restart
  .last:
        mcall   4                               ; print error message to window

        mov     ebx, 220 shl 16 + 85
        mov     ecx, 47 shl 16 + BTN_H
        mov     edx, 5
        mov     esi, 0xCCCCCC
        mcall   8                               ; OK button

        mov     ebx, 256 shl 16 + 52
		mov     ecx, TEXT_ECX
        mov     edx, okstr
        mov     esi, okstr_e-okstr
        mcall   4                               ; OK button text

  .redraw_done:
        and     [work], not WORK_GUI
        mcall   12, 2

  .loop:
        test    [work], WORK_GUI
        jnz     .redraw
        cmp     [status], STATUS_CONNECTED
        je      connected

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


  .key:                                         ; key event handler
        mcall   2                               ; read key

        cmp     [status], STATUS_CONNECT
        jne     @f
        test    [URLbox.flags], ed_focus
        jz      mainloop
        cmp     ah, 13                          ; enter (return) key
        je      .connect
        invoke  edit_box_key, URLbox
        jmp     .loop
  @@:
        cmp     [status], STATUS_REQ_LOGIN
        jne     @f
        cmp     ah, 13                          ; enter (return) key
        je      .login
        invoke  edit_box_key, USERbox
        invoke  edit_box_key, PASSbox
        jmp     .loop
  @@:
        cmp     [status], STATUS_DISCONNECTED
        jb      @f
        cmp     ah, 13
        je      .ok
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
        cmp     ah, 4
        je      .cancel
        cmp     ah, 5
        je      .ok

        jmp     .loop

  .connect:
        mov     eax, [URLbox.pos]
        mov     byte[serveraddr+eax], 0
        call    open_connection
        jmp     .loop

  .login:
        mov     [status], STATUS_LOGIN
        or      [work], WORK_GUI
        jmp     .loop

  .cancel:
        mcall   18, 18, [thread_id]             ; kill thread
  .ok:
        cmp     [status], STATUS_LIB_ERR
        je      .close

        and     [URLbox.flags], not ed_disabled
        mov     [USERbox.size], 0
        mov     [PASSbox.size], 0
        mov     [status], STATUS_CONNECT
        mov     [name.dash], 0
        mcall   71, 1, name                     ; reset window caption
        or      [work], WORK_GUI
        jmp     .loop

  .mouse:
        cmp     [status], STATUS_CONNECT
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


open_connection:

        or      [URLbox.flags], ed_disabled
        mov     [status], STATUS_CONNECTING

; Create network thread
        mcall   51, 1, thread_start, thread_stack
        cmp     eax, -1
        jne     @f
        mov     [status], STATUS_THREAD_ERR
  @@:
        mov     [thread_id], eax
        or      [work], WORK_GUI

        ret