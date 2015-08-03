;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  vncc.asm - VNC client for KolibriOS                            ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

__DEBUG__       = 1
__DEBUG_LEVEL__ = 1

xpos            = 4             ; coordinates of image
ypos            = 22            ;

TIMEOUT         = 5             ; timeout in seconds

use32

        org     0x0

        db      "MENUET01"      ; 8 byte id
        dd      0x01            ; header version
        dd      START           ; start of code
        dd      I_END           ; size of image
        dd      IM_END          ; memory for app
        dd      IM_END          ; esp
        dd      0x0 , 0x0       ; I_Param , I_Path


include "../../macros.inc"
include "../../debug-fdo.inc"
include "../../proc32.inc"
include "../../dll.inc"
include "../../struct.inc"
include "../../develop/libraries/box_lib/trunk/box_lib.mac"
include "../../network.inc"

struct  pixel_format
        bpp             db ?
        depth           db ?
        big_endian      db ?
        true_color      db ?
        red_max         dw ?
        green_max       dw ?
        blue_max        dw ?
        red_shift       db ?
        green_shift     db ?
        blue_shift      db ?
        padding         rb 3
ends

struct  framebuffer
        width           dw ?
        height          dw ?
        pixelformat     pixel_format
        name_length     dd ?
        name            rb 256
ends

include "logon.inc"
include "raw.inc"
include "copyrect.inc"
include "thread.inc"

START:

        mcall   68, 11                  ; init heap

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        call    logon

        mcall   40, 0                   ; disable all events
        mcall   67, 0, 0, 0, 0          ; resize the window (hide it)
        mcall   51, 1, thread_start, thread_stack

        DEBUGF  1,"Thread created: %u\n", eax

      @@:
        mcall   5, 10
        cmp     byte[thread_ready], 0
        je      @r

        mcall   40, EVM_MOUSE + EVM_MOUSE_FILTER + EVM_KEY + EVM_REDRAW + EVM_BUTTON

        mov     edx, dword[screen]
        movzx   esi, dx
        shr     edx, 16
        add     edx, 2*xpos
        add     esi, ypos+xpos
        mcall   67, 10, 10              ; resize the window

mainloop:
        mcall   10                      ; wait for event

        dec     eax
        jz      redraw
        dec     eax
        jz      key
        dec     eax
        jz      button
        sub     eax, 3
        jz      mouse
        jmp     mainloop

key:

;        DEBUGF  1,"Sending key event\n"

        mcall   2
        mov     byte[keyevent.key+3], ah

        mcall   send, [socketnum], keyevent, 8, 0
        jmp     mainloop

mouse:

;        DEBUGF  1,"Sending mouse event\n"

        mcall   37, 1           ; get mouse pos
        sub     eax, xpos shl 16 + ypos
        bswap   eax
        mov     [pointerevent.x], ax
        shr     eax, 16
        mov     [pointerevent.y], ax

        mcall   37, 2           ; get mouse buttons
        test    al, 00000010b   ; test if right button was pressed  (bit 1 in kolibri)
        jz      @f
        add     al, 00000010b   ; in RFB protocol it is bit 2, so if we add bit 2 again, we"ll get bit 3 and bit 1 will remain the same
      @@:
        mov     [pointerevent.mask],al

        mcall   send, [socketnum], pointerevent, 6, 0
        jmp     mainloop

redraw:

;        DEBUGF  1,"Drawing window\n"

        mcall   12, 1

        mov     ebx, dword[screen]
        movzx   ecx, bx
        shr     ebx, 16
        mov     edx, 0x74ffffff
        mov     edi, name
        mcall   0               ; draw window

        call    drawbuffer

        mcall   12, 2

        jmp     mainloop

drawbuffer:
        mcall   7, framebuffer_data, dword[screen], 0
        ret

button:
        mcall   17              ; get id

exit:
        DEBUGF  1, "Closing time!\n"
        mcall   close, [socketnum]
        mcall   -1

no_rfb:
        DEBUGF  1, "This is no vnc server!\n"
        jmp     exit

invalid_security:
        DEBUGF  1, "Security error: %s\n", receive_buffer+5
        jmp     exit


; DATA AREA

include_debug_strings    ; ALWAYS present in data section

handshake          db "RFB 003.003", 10
ClientInit         db 0         ; not shared
beep               db 0x85, 0x25, 0x85, 0x40, 0

pixel_format32     db 0         ; setPixelformat
                   db 0, 0, 0   ; padding
.bpp               db 32        ; bits per pixel
.depth             db 32        ; depth
.big_endian        db 0         ; big-endian flag
.true_color        db 1         ; true-colour flag
.red_max           db 0, 255    ; red-max
.green_max         db 0, 255    ; green-max
.blue_max          db 0, 255    ; blue-max
.red_shif          db 0         ; red-shift
.green_shift       db 8         ; green-shift
.blue_shift        db 16        ; blue-shift
                   db 0, 0, 0   ; padding

pixel_format16     db 0         ; setPixelformat
                   db 0, 0, 0   ; padding
.bpp               db 16        ; bits per pixel
.depth             db 15        ; depth
.big_endian        db 0         ; big-endian flag
.true_color        db 1         ; true-colour flag
.red_max           db 0, 31     ; red-max
.green_max         db 0, 31     ; green-max
.blue_max          db 0, 31     ; blue-max
.red_shif          db 0         ; red-shift
.green_shift       db 5         ; green-shift
.blue_shift        db 10        ; blue-shift
                   db 0, 0, 0   ; padding

pixel_format8      db 0         ; setPixelformat
                   db 0, 0, 0   ; padding
.bpp               db 8         ; bits per pixel
.depth             db 6         ; depth
.big_endian        db 0         ; big-endian flag
.true_color        db 1         ; true-colour flag
.red_max           db 0, 3      ; red-max
.green_max         db 0, 3      ; green-max
.blue_max          db 0, 3      ; blue-max
.red_shif          db 0         ; red-shift
.green_shift       db 2         ; green-shift
.blue_shift        db 4         ; blue-shift
                   db 0, 0, 0   ; padding

encodings          db 2         ; setEncodings
                   db 0         ; padding
                   db 0, 2      ; number of encodings
                   db 0, 0, 0, 0        ; raw encoding        (DWORD, Big endian order)
                   db 0, 0, 0, 1        ; Copyrect encoding
;                   db 0, 0, 0, 2        ; RRE
;                   db 0, 0, 0, 5        ; HexTile
;                   db 0, 0, 0, 15       ; TRLE
;                   db 0, 0, 0, 16       ; ZRLE

fbur               db 3         ; frame buffer update request
.inc               db 0         ; incremental
.x                 dw 0
.y                 dw 0
.width             dw 0
.height            dw 0

keyevent           db 4         ; keyevent
.down              db 0         ; down-flag
                   dw 0         ; padding
.key               dd 0         ; key

pointerevent       db 5         ; pointerevent
.mask              db 0         ; button-mask
.x                 dw 0         ; x-position
.y                 dw 0         ; y-position


sockaddr1:
        dw AF_INET4
.port   dw 0x0c17               ; 5900
.ip     dd 0
        rb 10

thread_ready    db 0
mouse_dd        dd ?

URLbox          edit_box 200, 25, 16, 0xffffff, 0x6f9480, 0, 0, 0, 65535, serveraddr, mouse_dd, ed_focus, 0, 0

; import
align 4
@IMPORT:

library network,                "network.obj",\
        box_lib,                "box_lib.obj",\
        archiver,               "archiver.obj"

import  network,\
        getaddrinfo,            "getaddrinfo",  \
        freeaddrinfo,           "freeaddrinfo", \
        inet_ntoa,              "inet_ntoa"

import  box_lib,\
        edit_box_draw,          "edit_box",\
        edit_box_key,           "edit_box_key",\
        edit_box_mouse,         "edit_box_mouse",\
        scrollbar_v_draw,       "scrollbar_v_draw",\
        scrollbar_v_mouse,      "scrollbar_v_mouse",\
        scrollbar_h_draw,       "scrollbar_h_draw",\
        scrollbar_h_mouse,      "scrollbar_h_mouse"

import  archiver,\
        deflate_unpack,         "deflate_unpack"

name                    db "VNC client "
name.dash               db 0, " "

I_END:

servername              rb 64+1

socketnum               dd ?
datapointer             dd ?
rectangles              dw ?

rectangle:
.width                  dw ?
.height                 dw ?
.x                      dw ?
.y                      dw ?

screen:                 ; Remote screen resolution
.height                 dw ?
.width                  dw ?

serveraddr              rb 65536
receive_buffer          rb 5*1024*1024  ; 5 mb buffer for received data (incoming frbupdate etc)
framebuffer_data        rb 1024*1024*3  ; framebuffer

                        rb 0x1000
thread_stack:

                        rb 0x1000

IM_END:


