;
;
; VNC Client for kolibrios by hidnplayr
;
; hidnplayr@gmail.com
;

format binary as ""

use32

        org     0x0

        db      'MENUET01'              ; 8 byte id
        dd      0x01                    ; header version
        dd      START                   ; start of code
        dd      I_END                   ; size of image
        dd      IM_END                  ; memory for app
        dd      IM_END                  ; esp
        dd      0x0 , 0x0               ; I_Param , I_Path

__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1

STRLEN = 64      ; password and server max length
xpos = 4         ; coordinates of image
ypos = 22        ;

TIMEOUT = 5     ; timeout in seconds

include '../macros.inc'
include '../debug-fdo.inc'
include '../proc32.inc'
include '../dll.inc'
include '../struct.inc'
include '../network.inc'

include 'structs.inc'
include 'logon.inc'
include 'raw.inc'
include 'thread.inc'

START:

        mcall   68, 11                  ; init heap

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        call    red_logon

        mcall   40, 0                   ; no events
        mcall   67, 0, 0, 0, 0          ; resize the window (hide it)
        mcall   51, 1, thread_start, thread_stack

        DEBUGF 1,'Thread created: %u\n', eax

      @@:
        mcall   5, 10
        cmp     byte [thread_ready], 0
        je      @r

        mcall   40, 100111b             ; mouse, button, key, redraw


        mov     edx, dword [screen]
        movzx   esi, dx
        shr     edx, 16
        add     edx, 2*xpos
        add     esi, ypos+xpos
        mcall   67, 10, 10              ; resize the window

mainloop:
        mcall   23, 50                  ; wait for event, 0,5s timeout

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

        DEBUGF  1,'Sending key event\n'

        mcall   2
        mov     byte [keyevent.key+3], ah

        mcall   send, [socketnum], keyevent, 8, 0
        jmp     mainloop

mouse:

        DEBUGF  1,'Sending mouse event\n'

        mcall   37, 1           ; get mouse pos
        sub     eax, xpos shl 16 + ypos
        bswap   eax
        mov     [pointerevent.x], ax
        shr     eax, 16
        mov     [pointerevent.y], ax

        mcall   37, 2           ; get mouse buttons
        test    al, 00000010b   ; test if right button was pressed  (bit 1 in kolibri)
        jz      @f
        add     al, 00000010b   ; in RFB protocol it is bit 2, so if we add bit 2 again, we'll get bit 3 and bit 1 will remain the same
      @@:
        mov     [pointerevent.mask],al

        mcall   send, [socketnum], pointerevent, 6, 0
        jmp     mainloop

redraw:

        DEBUGF  1,'Drawing window\n'

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
        DEBUGF  1, 'Closing time!\n'
        mcall   close, [socketnum]
        mcall   -1

no_rfb:
        DEBUGF  1, 'This is no vnc server!\n'
        jmp     exit

invalid_security:
        DEBUGF  1, 'Security error: %s\n', receive_buffer+5
        jmp     exit


; DATA AREA

include_debug_strings    ; ALWAYS present in data section

handshake          db 'RFB 003.003', 10
shared             db 0
beep               db 0x85,0x25,0x85,0x40,0

pixel_format32     db 0       ; setPixelformat
                   rb 3       ; padding
.bpp               db 32      ; bits per pixel
.depth             db 32      ; depth
.big_endian        db 0       ; big-endian flag
.true_color        db 1       ; true-colour flag
.red_max           db 0,255   ; red-max
.green_max         db 0,255   ; green-max
.blue_max          db 0,255   ; blue-max
.red_shif          db 0       ; red-shift
.green_shift       db 8       ; green-shift
.blue_shift        db 16      ; blue-shift
                   rb 3       ; padding

pixel_format16     db 0       ; setPixelformat
                   rb 3       ; padding
.bpp               db 16      ; bits per pixel
.depth             db 15      ; depth
.big_endian        db 0       ; big-endian flag
.true_color        db 1       ; true-colour flag
.red_max           db 0,31    ; red-max
.green_max         db 0,31    ; green-max
.blue_max          db 0,31    ; blue-max
.red_shif          db 0       ; red-shift
.green_shift       db 5       ; green-shift
.blue_shift        db 10      ; blue-shift
                   rb 3       ; padding

pixel_format8      db 0       ; setPixelformat
                   rb 3       ; padding
.bpp               db 8       ; bits per pixel
.depth             db 6       ; depth
.big_endian        db 0       ; big-endian flag
.true_color        db 1       ; true-colour flag
.red_max           db 0,3     ; red-max
.green_max         db 0,3     ; green-max
.blue_max          db 0,3     ; blue-max
.red_shif          db 0       ; red-shift
.green_shift       db 2       ; green-shift
.blue_shift        db 4       ; blue-shift
                   rb 3       ; padding

encodings          db 2       ; setEncodings
                   rb 1       ; padding
                   db 1,0     ; number of encodings
                   db 0,0,0,0 ; raw encoding        (DWORD, Big endian order)
                   db 1,0,0,0 ; Copyrect encoding

fbur               db 3       ; frame buffer update request
.inc               db 0       ; incremental
.x                 dw 0
.y                 dw 0
.width             dw 0
.height            dw 0

keyevent           db 4       ; keyevent
.down              db 0       ; down-flag
                   dw 0       ; padding
.key               dd 0       ; key

pointerevent       db 5       ; pointerevent
.mask              db 0       ; button-mask
.x                 dw 0       ; x-position
.y                 dw 0       ; y-position


sockaddr1:
        dw AF_INET4
.port   dw 0x0c17       ; 5900
.ip     dd 0
        rb 10

thread_ready    db 0
; import
align 4
@IMPORT:

library         network, 'network.obj'
import          network,        \
                getaddrinfo,    'getaddrinfo',  \
                freeaddrinfo,   'freeaddrinfo', \
                inet_ntoa,      'inet_ntoa'

name    db 'VNC client', 0

I_END:

socketnum          dd ?
datapointer        dd ?

frame:
.width             dw ?
.height            dw ?
.x                 dw ?
.y                 dw ?

screen:
.height            dw ?
.width             dw ?

receive_buffer     rb 5*1024*1024 ; 5 mb buffer for received data (incoming frbupdate etc)
framebuffer_data   rb 1024*768*3  ; framebuffer


                rb 0x1000
thread_stack:

                rb 0x1000
IM_END:


