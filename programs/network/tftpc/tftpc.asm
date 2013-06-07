;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
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
include '../../libio.inc'
include '../../dll.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'

include '../../network.inc'


filebuffer_size = 4*4096        ; 16kb   (dont try to change it yet..)
TIMEOUT         = 100
buffer_len      = 1500

opcode_rrq      = 1
opcode_wrq      = 2
opcode_data     = 3
opcode_ack      = 4
opcode_error    = 5

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

stop_transfer:
        mcall   40, 00100111b

red_win:
        call draw_window

align 4
still:
        mcall   10

        dec     eax
        jz      red_win

        dec     eax
        jz      key

        dec     eax
        jz      button

        push    dword edit1
        call    [edit_box_mouse]

        push    dword edit2
        call    [edit_box_mouse]

        push    dword edit3
        call    [edit_box_mouse]

        push    dword edit4
        call    [edit_box_mouse]

        push    dword Option_boxs1
        call    [option_box_mouse]

        push    dword Option_boxs2
        call    [option_box_mouse]

        jmp     still

button:
        mcall   17

        cmp     ah,0x10
        je      start_transfer


        test    ah , ah
        jz      still

exit:   mcall   -1
key:
        mcall   2

        push    dword edit1
        call    [edit_box_key]

        push    dword edit2
        call    [edit_box_key]

        push    dword edit3
        call    [edit_box_key]

        push    dword edit4
        call    [edit_box_key]

        jmp still


align 4
draw_window:
        mcall   12,1
        mcall   0, (50*65536+400), (30*65536+180), 0x34AABBCC, 0x085080DD, str_title

        mcall   4, 35*65536+10, 0x80000000, str_server

        mov     ebx, 5*65536+30
        mov     edx, str_source
        mcall

        mov     ebx, 11*65536+50
        mov     edx, str_destination
        mcall

        mov     ebx, 47*65536+72
        mov     edx, str_mode
        mcall

        mov     ebx, 160*65536+72
        mov     edx, str_method
        mcall

        mov     ebx, 270*65536+72
        mov     edx, str_blocksize
        mcall

        push    dword edit1
        call    [edit_box_draw]

        push    dword edit2
        call    [edit_box_draw]

        push    dword edit3
        call    [edit_box_draw]

        push    dword edit4
        call    [edit_box_draw]

        push    dword Option_boxs1
        call    [option_box_draw]

        push    dword Option_boxs2
        call    [option_box_draw]

        mcall   8,210*65536+170, 105*65536+16,0x00000010,0x085080DD

        mcall   4,260*65536+110, 0x80000000, str_transfer

        mcall   38,10*65536+380, 130*65536+130,0x00000000

        mcall   4,350*65536+137, 0x80000000, str_kb_s

        mcall   47,1 shl 31 + 7 shl 16 + 1,kbps,305*65536+137,0x00000000

        mcall   4,50*65536+137, 0x80000000, str_complete

        mcall   47,1 shl 31 + 3 shl 16 + 1,done,25*65536+137,0x00000000

        mcall   12,2

        ret





start_transfer:

        ; first, resolve the hostname

        push    esp     ; reserve stack place

        push    esp     ; fourth parameter
        push    0       ; third parameter
        push    0       ; second parameter
        push    dword SRV     ; first parameter
        call    [getaddrinfo]

        pop     esi

; test for error
        test    eax, eax
        jnz     still

        mov     esi, [esi]
        mov     esi, [esi + sockaddr_in.sin_addr]
        mov     dword [IP], esi

        mcall   socket, AF_INET4, SOCK_DGRAM, 0                ; socket_open
        cmp     eax, -1
        je      still

        mov     [socketnum], eax

        mcall   connect, [socketnum], sockaddr, sockaddr_len         ; socket_connect
        cmp     eax, -1
        je      still

        mov     word [I_END], opcode_rrq
        cmp     [option_group2],op3
        je      @f
        mov     word [I_END], opcode_wrq
      @@:

        xor     al , al
        mov     edi, remote_addr
        mov     ecx, 250
        repnz   scasb
        sub     edi, remote_addr-1
        mov     ecx, edi
        mov     edi, I_END+2
        mov     esi, remote_addr
        rep     movsb

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

      .send_request:

        xor     al, al
        stosb

        sub     edi, I_END
        mov     esi, edi
        mcall   send, [socketnum], I_END

        mcall   40, 10000101b

        mov     [last_ack], 0






receive_data_loop:

        mcall   23, TIMEOUT

        dec     eax
        jz      .red

        dec     eax
        jz      .key


        mcall   recv, [socketnum], buffer, buffer_len, 0  ; receive data

        cmp     word[buffer], opcode_data
        jne     .error

        mov     bx, [last_ack]
        cmp     word [buffer + 2], bx
        jne     .packet_got_lost
        inc     [last_ack]

        cmp     eax, 4+512
        je      .continue

; last packet, or something else
.error:

.packet_got_lost:



.continue:

        mov     word[buffer], opcode_ack                ; send ack
        mcall   send, [socketnum], buffer, 4, 0

        jmp     receive_data_loop

.red:

        call    draw_window

        jmp     receive_data_loop


.key:
        mcall   2
        cmp     ah, 2
        jz      exit

        ; close socket ?

        jmp     receive_data_loop






;--------------------------------


send_:

        invoke  file_open, local_addr, O_READ
        or      eax, eax
        jz      .exit
        mov     [fh], eax

        stdcall mem.Alloc, filebuffer_size
        or      eax, eax
        jz      .exit
        mov     [fb], eax

        mov     [last_ack], 0
        mov     [fo], 0

.read_chunk:

        invoke  file_seek, [fh], [fo], SEEK_END
        cmp     eax, -1
        je      .exit
        invoke  file_read, [fh], [fb], filebuffer_size
        cmp     eax, -1
        je      .exit
        add     [fo], filebuffer_size
        cmp     eax, filebuffer_size
        je      .packet

        ; ijhidfhfdsndsfqk

.packet:

        movzx   esi, [last_ack]
        and     esi, 0x000000001f   ; last five bits    BUFFER SIZE MUST BE 16 kb for this to work !!!
        shl     esi, 9              ; = * 512
        add     esi, [fb]
        mov     edi, buffer
        mov     ax, opcode_data
        stosw
        mov     ax, [last_ack]
        stosw
        mov     ecx, 512/4
        rep     movsd

        mcall   send, [socketnum], buffer, 4+512, 0       ; send data


.loop:

        mcall   23, TIMEOUT

        dec     eax
        jz      .red

        dec     eax
        jz      .key

        mcall   recv, [socketnum], buffer, buffer_len, 0  ; receive ack

        cmp     word[buffer], opcode_ack
        jne     .exit

        mov     ax, [last_ack]
        cmp     word[buffer+2], ax
        jne     .packet
        inc     [last_ack]
        test    [last_ack],0x001f
        jz      .read_chunk
        jmp     .packet


.red:

        call    draw_window

        jmp     .loop


.key:
        mcall   2
        cmp     ah, 2
        jz      exit

        ; close socket ?

        jmp     .loop

.exit:
        invoke  file_close, [fh]
        jmp     still





;-------------------------
; DATA

socketnum      dd 0
kbps           dd 0
done           dd 0

sockaddr:
        dw AF_INET4
        dw 0x4500       ; 69
IP      db 192,168,1,115
sockaddr_len = $ - sockaddr

align 16
@IMPORT:

library box_lib , 'box_lib.obj', \
        io_lib  , 'libio.obj', \
        network , 'network.obj'

import  box_lib                                 ,\
        edit_box_draw    ,'edit_box'            ,\
        edit_box_key     ,'edit_box_key'        ,\
        edit_box_mouse   ,'edit_box_mouse'      ,\
        version_ed       ,'version_ed'          ,\
        init_checkbox    ,'init_checkbox2'      ,\
        check_box_draw   ,'check_box_draw2'     ,\
        check_box_mouse  ,'check_box_mouse2'    ,\
        version_ch       ,'version_ch2'         ,\
        option_box_draw  ,'option_box_draw'     ,\
        option_box_mouse ,'option_box_mouse'    ,\
        version_op       ,'version_op'

import  io_lib                                  ,\
        file_find_first , 'file_find_first'     ,\
        file_find_next  , 'file_find_next'      ,\
        file_find_close , 'file_find_close'     ,\
        file_size       , 'file_size'           ,\
        file_open       , 'file_open'           ,\
        file_read       , 'file_read'           ,\
        file_write      , 'file_write'          ,\
        file_seek       , 'file_seek'           ,\
        file_tell       , 'file_tell'           ,\
        file_eof?       , 'file_iseof'          ,\
        file_seteof     , 'file_seteof'         ,\
        file_truncate   , 'file_truncate'       ,\
        file_close      , 'file_close'

import  network                                         ,\
        inet_ntoa               , 'inet_ntoa'           ,\
        getaddrinfo             , 'getaddrinfo'         ,\
        freeaddrinfo            , 'freeaddrinfo'


edit1 edit_box 300,80,5 ,0xffffff,0x6f9480,0,0,0,99 ,SRV,mouse_dd,ed_focus,  11,11
edit2 edit_box 300,80,25,0xffffff,0x6a9480,0,0,0,99 ,remote_addr,mouse_dd,ed_figure_only, 5,5
edit3 edit_box 300,80,45,0xffffff,0x6a9480,0,0,0,99 ,local_addr,mouse_dd,ed_figure_only, 27,27
edit4 edit_box 40,340,68,0xffffff,0x6a9480,0,0,0,5 ,BLK,mouse_dd,ed_figure_only, 3,3

op1 option_box option_group1,80,68,6,12,0xffffff,0,0,netascii,octet-netascii
op2 option_box option_group1,80,85,6,12,0xFFFFFF,0,0,octet,get-octet

op3 option_box option_group2,210,68,6,12,0xffffff,0,0,get,put-get
op4 option_box option_group2,210,85,6,12,0xFFFFFF,0,0,put,BLK-put

option_group1   dd op1
option_group2   dd op3
Option_boxs1    dd op1,op2,0
Option_boxs2    dd op3,op4,0

str_title       db 'TFTP client for KolibriOS',0
str_server      db 'Server:',0
str_source      db 'Remote file:',0
str_destination db 'Local file:',0
str_mode        db 'Mode:',0
str_method      db 'Method:',0
str_blocksize   db 'Blocksize:',0
str_kb_s        db 'kb/s',0
str_complete    db '% complete',0
str_transfer    db 'Transfer',0

str_error:
._0 db 'Not defined, see error message (if any).',0
._1 db 'File not found.',0
._2 db 'Access violation.',0
._3 db 'Disk full or allocation exceeded.',0
._4 db 'Illegal TFTP operation.',0
._5 db 'Unknown transfer ID.',0
._6 db 'File already exists.',0
._7 db 'No such user.',0


netascii db 'NetASCII'
octet    db 'Octet'
get      db 'GET'
put      db 'PUT'

BLK      db "512",0,0,0

last_ack dw ?

fh       dd ?   ; file handle
fo       dd ?   ; file offset
fb       dd ?   ; file buffer

SRV db "192.168.1.115",0
rb (SRV + 256 - $)

remote_addr db "IMG00",0
rb (remote_addr + 256 - $)

local_addr  db "/hd0/1/KolibriOS/kernel.mnt",0
rb (local_addr + 256 - $)

I_END:
mouse_dd        dd ?
buffer:
rb buffer_len

IM_END: