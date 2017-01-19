;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2017. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  tftpc.asm - TFTP client for KolibriOS                          ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

__DEBUG__       = 0
__DEBUG_LEVEL__ = 1

use32
        org     0x0

        db      'MENUET01'
        dd      0x1
        dd      START
        dd      I_END
        dd      IM_END+0x1000
        dd      IM_END+0x1000
        dd      0, 0

include '../../proc32.inc'
include '../../macros.inc'
include '../../develop/libraries/libs-dev/libio/libio.inc'
include '../../dll.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'

include '../../network.inc'
include '../../debug-fdo.inc'

TIMEOUT         = 100
buffer_len      = 4096

opcode_rrq      = 1 shl 8
opcode_wrq      = 2 shl 8
opcode_data     = 3 shl 8
opcode_ack      = 4 shl 8
opcode_error    = 5 shl 8

; read/write request packet
;
;  2 bytes     string    1 byte     string   1 byte
;  ------------------------------------------------
; | Opcode |  Filename  |   0  |    Mode    |   0  |
;  ------------------------------------------------

; data packet
;
;  2 bytes     2 bytes      n bytes
;  ----------------------------------
; | Opcode |   Block #  |   Data     |
;  ----------------------------------

; acknowledgement packet
;
;  2 bytes     2 bytes
;  ---------------------
; | Opcode |   Block #  |
;  ---------------------

; error packet
;
;  2 bytes  2 bytes        string    1 byte
;  ----------------------------------------
; | Opcode |  ErrorCode |   ErrMsg   |   0  |
;  ----------------------------------------


START:

        mcall   68, 11

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     exit

home:
        mcall   40, EVM_MOUSE + EVM_MOUSE_FILTER + EVM_REDRAW + EVM_BUTTON + EVM_KEY + EVM_STACK

redraw:
        call    draw_window

mainloop:
        mcall   10
        dec     eax
        jz      redraw
        dec     eax
        jz      key
        dec     eax
        jz      button

        invoke  edit_box_mouse, edit1
        invoke  edit_box_mouse, edit2
        invoke  edit_box_mouse, edit3
        invoke  edit_box_mouse, edit4

        invoke  option_box_mouse, Option_boxs1
        invoke  option_box_mouse, Option_boxs2

        jmp     mainloop

button:
        mcall   17

        cmp     ah, 0x10        ; connect button
        je      start_transfer

        test    ah , ah
        jz      mainloop

exit:
        mcall   -1
key:
        mcall   2

        invoke  edit_box_key, edit1
        invoke  edit_box_key, edit2
        invoke  edit_box_key, edit3
        invoke  edit_box_key, edit4

        jmp     mainloop


draw_window:
; get system colors
        mcall   48, 3, sc, 40

        mcall   12, 1

        mov     edx, [sc.work]
        or      edx, 0x34000000
        xor     esi, esi
        mov     edi, str_title
        mcall   0, 50 shl 16 + 400, 30 shl 16 + 180

        mov     ebx, 35 shl 16 + 10
        mov     ecx, 0x80000000
        or      ecx, [sc.work_text]
        mov     edx, str_server
        mcall   4
        mov     ebx, 5 shl 16 + 30
        mov     edx, str_source
        mcall
        mov     ebx, 11 shl 16 + 50
        mov     edx, str_destination
        mcall
        mov     ebx, 47 shl 16 + 72
        mov     edx, str_mode
        mcall
        mov     ebx, 160 shl 16 + 72
        mov     edx, str_method
        mcall
        mov     ebx, 270 shl 16 + 72
        mov     edx, str_blocksize
        mcall

        invoke  edit_box_draw, edit1
        invoke  edit_box_draw, edit2
        invoke  edit_box_draw, edit3
        invoke  edit_box_draw, edit4

        invoke  option_box_draw, Option_boxs1
        invoke  option_box_draw, Option_boxs2

        mov     esi, [sc.work_button]
        mcall   8, 210 shl 16 + 170, 105 shl 16 + 16, 0x10

        mcall   38, 10 shl 16 + 380, 130 shl 16 + 130, [sc.work_graph]

        cmp     [errormsg], 0
        je      .no_error

        mov     ecx, 0x80000000
        or      ecx, [sc.work_text]
        mcall   4, 20 shl 16 + 137, , [errormsg]
        mcall   12, 2
        jmp     .draw_btn

  .no_error:
        mov     ecx, 0x80000000
        or      ecx, [sc.work_text]
        mcall   4, 350 shl 16 + 137, , str_kb_s
        mcall   4, 50 shl 16 + 137, , str_complete
        mcall   47, 1 shl 31 + 7 shl 16 + 1, kbps, 305 shl 16 + 137, [sc.work_text]
        mcall   47, 1 shl 31 + 3 shl 16 + 1, done, 25 shl 16 + 137

  .draw_btn:
        cmp     [socketnum], 0
        je      .no_transfer

        mov     ecx, 0x80000000
        or      ecx, [sc.work_button_text]
        mcall   4, 270 shl 16 + 110, , str_stop

        mcall   12, 2
        ret

  .no_transfer:
        mov     ecx, 0x80000000
        or      ecx, [sc.work_button_text]
        mcall   4, 260 shl 16 + 110, , str_transfer

        mcall   12, 2
        ret



start_transfer:

; resolve the hostname
        mov     [errormsg], str_err_resolve

        push    esp     ; reserve stack place

        push    esp     ; fourth parameter
        push    0       ; third parameter
        push    0       ; second parameter
        push    dword SRV     ; first parameter
        call    [getaddrinfo]

        pop     esi

        ; test for error
        test    eax, eax
        jnz     home

        mov     eax, [esi + addrinfo.ai_addr]
        mov     eax, [eax + sockaddr_in.sin_addr]
        mov     [sockaddr.ip], eax

        ; free allocated memory
        push    esi
        call    [freeaddrinfo]

; Open a socket & connect to server
        mov     [errormsg], str_err_socket

        mcall   socket, AF_INET4, SOCK_DGRAM, 0
        cmp     eax, -1
        je      home
        mov     [socketnum], eax

        mcall   connect, [socketnum], sockaddr, sockaddr_len
        cmp     eax, -1
        je      home

; Create the read/write request packet
        mov     word[buffer], opcode_rrq
        cmp     [option_group2], op3
        je      @f
        mov     word[buffer], opcode_wrq
      @@:

; Copy in the remote filename (asciiz)
        xor     al, al
        mov     edi, remote_addr
        mov     ecx, 255
        repnz   scasb
        lea     ecx, [edi - remote_addr - 1]
        mov     esi, remote_addr
        mov     edi, buffer+2
        rep     movsb
        stosb

; Append the data type (asciiz)
        cmp     [option_group1], op1
        je      .ascii
        mov     esi, octet
        movsd
        movsb
        jmp     .send_request

      .ascii:
        mov     esi, netascii
        movsd
        movsd

; Send the request to the server
      .send_request:
        xor     al, al
        stosb
        lea     esi, [edi - buffer]
        xor     edi, edi
        mcall   send, [socketnum], buffer
        cmp     eax, -1
        je      home

; Jump to send/receive code
        cmp     word[buffer], opcode_wrq
        je      tftp_send


tftp_receive:

        mcall   40, EVM_REDRAW + EVM_BUTTON + EVM_STACK
        mov     [last_ack], 0
        mov     [errormsg], 0

        call    draw_window

; Open/create local file
        mov     [file_struct.subfn], 2
        mov     [file_struct.offset], 0
        mov     [file_struct.size], 0
        mcall   70, file_struct

; Truncate it to 0 bytes
        mov     [file_struct.subfn], 4
        mcall   70, file_struct

; Set parameters for writing to file
        mov     [file_struct.subfn], 3
        mov     [file_struct.data], buffer + 4

  .loop:
        mcall   23, TIMEOUT
        dec     eax
        jz      .red
        dec     eax
        dec     eax
        jz      .button

        mcall   recv, [socketnum], buffer, buffer_len, MSG_DONTWAIT     ; receive data
        cmp     eax, -1
        je      .loop

        DEBUGF  1, "Got %u bytes\n", eax
        cmp     word[buffer], opcode_error
        je      tftp_error
        cmp     word[buffer], opcode_data
        jne     .error

; Verify ACK number
        mov     bx, word[buffer + 2]
        rol     bx, 8
        cmp     [last_ack], 0
        je      @f
        cmp     [last_ack], bx
        jne     .packet_got_lost
        inc     bx
      @@:
        mov     [last_ack], bx

; Write to the file
        lea     ecx, [eax - 4]
        mov     [file_struct.size], ecx
        mcall   70, file_struct
        add     [file_struct.offset], ecx

; Send ACK
        mov     word[buffer], opcode_ack
        mcall   send, [socketnum], buffer, 4, 0
        jmp     .loop

  .packet_got_lost:
        ;TODO
        jmp     .loop

  .red:
        call    draw_window
        jmp     .loop

  .button:
        mcall   17
        cmp     ah, 1
        jne     .abort

        mcall   close, [socketnum]
        mcall   -1

  .abort:
        mcall   close, [socketnum]
        xor     eax, eax
        mov     [socketnum], eax
        mov     [errormsg], str_abort
        jmp     home

  .error:
        mcall   close, [socketnum]
        xor     eax, eax
        mov     [socketnum], eax
        mov     [errormsg], str_err_unexp
        jmp     home

  .done:
        mcall   close, [socketnum]
        xor     eax, eax
        mov     [socketnum], eax
        mov     [errormsg], str_success
        jmp     home



tftp_send:

        mcall   40, EVM_REDRAW + EVM_BUTTON + EVM_STACK
        mov     [last_ack], 0
        mov     [errormsg], 0

        call    draw_window

        mov     [file_struct.subfn], 0
        mov     [file_struct.offset], 0
        mov     [file_struct.size], buffer_len
        mov     [file_struct.data], buffer + 4

  .next:
        mov     edi, buffer
        mov     ax, opcode_data
        stosw
        mov     ax, [last_ack]
        stosw

        mcall   70, file_struct
        test    eax, eax
;        jnz     .done
        mov     [size], ebx

  .resend:
        mov     ebx, [size]
        lea     esi, [ebx + 4]
        xor     edi, edi
        mcall   send, [socketnum], buffer

  .loop:
        mcall   23, TIMEOUT
        dec     eax
        jz      .red
        dec     eax
        dec     eax
        jz      .button

        mcall   recv, [socketnum], buffer, buffer_len, MSG_DONTWAIT
        cmp     eax, -1
        je      .loop

        cmp     word[buffer], opcode_error
        je      tftp_error
        cmp     word[buffer], opcode_ack
        jne     .error

        mov     ax, [last_ack]
        cmp     word[buffer+2], ax
        jne     .resend

        mov     eax, [size]
        cmp     eax, buffer_len
        jb      .done
        add     [file_struct.offset], eax

        inc     [last_ack]
        jmp     .next

  .red:
        call    draw_window
        jmp     .loop

  .button:
        mcall   17
        cmp     ah, 1
        jne     .abort

        mcall   close, [socketnum]
        mcall   -1

  .abort:
        mcall   close, [socketnum]
        xor     eax, eax
        mov     [socketnum], eax
        mov     [errormsg], str_abort
        jmp     home

  .error:
        mcall   close, [socketnum]
        xor     eax, eax
        mov     [socketnum], eax
        mov     [errormsg], str_err_unexp
        jmp     home

  .done:
        mcall   close, [socketnum]
        xor     eax, eax
        mov     [socketnum], eax
        mov     [errormsg], str_success
        jmp     home



tftp_error:
        mcall   close, [socketnum]
        xor     eax, eax
        mov     [socketnum], eax

        mov     ax, word[buffer+2]
        xchg    al, ah

        test    ax, ax
        jz      .0
        dec     ax
        jz      .1
        dec     ax
        jz      .2
        dec     ax
        jz      .3
        dec     ax
        jz      .4
        dec     ax
        jz      .5
        dec     ax
        jz      .6
        dec     ax
        jz      .7

  .0:
        mov     [errormsg], str_error.0
        jmp     home
  .1:
        mov     [errormsg], str_error.1
        jmp     redraw
  .2:
        mov     [errormsg], str_error.2
        jmp     home
  .3:
        mov     [errormsg], str_error.3
        jmp     home
  .4:
        mov     [errormsg], str_error.4
        jmp     home
  .5:
        mov     [errormsg], str_error.5
        jmp     home
  .6:
        mov     [errormsg], str_error.6
        jmp     home
  .7:
        mov     [errormsg], str_error.7
        jmp     home

;-------------------------
; DATA

socketnum       dd 0
kbps            dd 0
done            dd 0
errormsg        dd str_welcome

sockaddr:
                dw AF_INET4
                dw 0x4500       ; 69
  .ip           dd ?
sockaddr_len = $ - sockaddr

file_struct:
  .subfn        dd ?
  .offset       dd ?
                dd 0
  .size         dd ?
  .data         dd ?
                db 0
  .filename     dd local_addr

align 16
@IMPORT:

library box_lib         , 'box_lib.obj'         ,\
        network         , 'network.obj'

import  box_lib                                 ,\
        edit_box_draw   , 'edit_box'            ,\
        edit_box_key    , 'edit_box_key'        ,\
        edit_box_mouse  , 'edit_box_mouse'      ,\
        version_ed      , 'version_ed'          ,\
        init_checkbox   , 'init_checkbox2'      ,\
        check_box_draw  , 'check_box_draw2'     ,\
        check_box_mouse , 'check_box_mouse2'    ,\
        version_ch      , 'version_ch2'         ,\
        option_box_draw , 'option_box_draw'     ,\
        option_box_mouse, 'option_box_mouse'    ,\
        version_op      , 'version_op'

import  network                                 ,\
        inet_ntoa       , 'inet_ntoa'           ,\
        getaddrinfo     , 'getaddrinfo'         ,\
        freeaddrinfo    , 'freeaddrinfo'


edit1 edit_box 300, 80, 5, 0xffffff, 0x6f9480, 0, 0, 0, 255, SRV, mouse_dd, ed_focus, 13, 13
edit2 edit_box 300, 80, 25, 0xffffff, 0x6f9480, 0, 0, 0, 255, remote_addr, mouse_dd, 0, 4, 4
edit3 edit_box 300, 80, 45, 0xffffff, 0x6f9480, 0, 0, 0, 255, local_addr, mouse_dd, 0, 12, 12
edit4 edit_box 40, 340, 68, 0xffffff, 0x6f9480, 0, 0, 0, 5, BLK, mouse_dd, ed_figure_only, 3, 3

op1 option_box option_group1, 80, 68, 6, 12, 0xffffff, 0, 0, netascii, octet-netascii
op2 option_box option_group1, 80, 85, 6, 12, 0xFFFFFF, 0, 0, octet, get-octet

op3 option_box option_group2, 210, 68, 6, 12, 0xffffff, 0, 0, get, put-get
op4 option_box option_group2, 210, 85, 6, 12, 0xFFFFFF, 0, 0, put, BLK-put

option_group1   dd op1
option_group2   dd op3
Option_boxs1    dd op1, op2, 0
Option_boxs2    dd op3, op4, 0

str_title       db 'TFTP client', 0
str_server      db 'Server:', 0
str_source      db 'Remote file:', 0
str_destination db 'Local file:', 0
str_mode        db 'Mode:', 0
str_method      db 'Method:', 0
str_blocksize   db 'Blocksize:', 0
str_kb_s        db 'kB/s', 0
str_complete    db '% complete', 0
str_transfer    db 'Transfer', 0
str_stop        db 'Stop', 0

str_error:
.0              db 'Error.', 0                      ; not further defined error
.1              db 'File not found.', 0
.2              db 'Access violation.', 0
.3              db 'Disk full or allocation exceeded.', 0
.4              db 'Illegal TFTP operation.', 0
.5              db 'Unknown transfer ID.', 0
.6              db 'File already exists.', 0
.7              db 'No such user.', 0

str_welcome     db 'Welcome.', 0
str_err_resolve db 'Unable to resolve server address.', 0
str_err_socket  db 'Socket error.', 0
str_err_unexp   db 'Unexpected command from server.', 0
str_success     db 'Operation completed successfully.', 0
str_abort       db 'Operation aborted by user.', 0

netascii        db 'NetASCII'
octet           db 'Octet'
get             db 'GET'
put             db 'PUT'

BLK             db "512", 0, 0, 0

SRV             db "192.168.1.115", 0
                rb (SRV + 256 - $)

remote_addr     db "file", 0
                rb (remote_addr + 256 - $)

local_addr      db "/tmp0/1/file", 0
                rb (local_addr + 256 - $)

include_debug_strings

I_END:

last_ack        dw ?
size            dd ?
mouse_dd        dd ?

sc              system_colors

buffer          rb buffer_len

IM_END: