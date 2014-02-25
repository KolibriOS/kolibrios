;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  IRC client for KolibriOS                                       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org,                           ;;
;;     text encoder/decoder by Clevermouse.                        ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

version equ '0.16'

; connection status
STATUS_DISCONNECTED     = 0
STATUS_RESOLVING        = 1
STATUS_CONNECTING       = 2
STATUS_CONNECTED        = 3

; window flags
FLAG_UPDATED            = 1 shl 0
FLAG_RECEIVING_NAMES    = 1 shl 1

; window types
WINDOWTYPE_NONE         = 0
WINDOWTYPE_SERVER       = 1
WINDOWTYPE_CHANNEL      = 2
WINDOWTYPE_CHAT         = 3
WINDOWTYPE_LIST         = 4
WINDOWTYPE_DCC          = 5

; supported encodings
CP866                   = 0
CP1251                  = 1
UTF8                    = 2

; settings
USERCMD_MAX_SIZE        = 400

WIN_MIN_X               = 600
WIN_MIN_Y               = 170

TEXT_X                  = 5
TEXT_Y                  = TOP_Y + 2

TOP_Y                   = 24
BOTTOM_Y                = 15

MAX_WINDOWS             = 20
MAX_USERS               = 4096
TEXT_BUFFERSIZE         = 1024*1024

MAX_NICK_LEN            = 32
MAX_REAL_LEN            = 32    ; realname
MAX_SERVER_NAME         = 256

MAX_CHANNEL_LEN         = 40
MAX_CHANNELS            = 37

MAX_COMMAND_LEN         = 512

TIMESTAMP               = 3     ; 3 = hh:mm:ss, 2 = hh:mm, 0 = no timestamp

MAX_WINDOWNAME_LEN      = 256

WINDOW_BTN_START        = 100
WINDOW_BTN_CLOSE        = 2
WINDOW_BTN_LIST         = 3

SCROLLBAR_WIDTH         = 14
USERLIST_WIDTH          = 100

FONT_HEIGHT             = 9
FONT_WIDTH              = 6

format binary as ""

use32

        org     0x0

        db      'MENUET01'              ; 8 byte id
        dd      1                       ; header version
        dd      START                   ; program start
        dd      I_END                   ; program image size
        dd      IM_END+2048             ; required amount of memory
        dd      IM_END+2048
        dd      param
        dd      path

include "../../macros.inc"
include "../../proc32.inc"
include "../../dll.inc"
include "../../network.inc"
include "../../struct.inc"
include "../../develop/libraries/box_lib/trunk/box_lib.mac"

struct  window
        data_ptr        dd ?
        flags           db ?
        type            db ?
        name            rb MAX_WINDOWNAME_LEN
        users           dd ?
        users_scroll    dd ?
        selected        dd ?            ; selected user, 0 if none selected

        text_start      dd ?            ; pointer to current textbox data
        text_end        dd ?
        text_print      dd ?            ; pointer to first character to print on screen
        text_line_print dd ?            ; line number of that character
        text_write      dd ?            ; write pointer
        text_lines      dd ?            ; total number of lines
        text_scanned    dd ?            ; pointer to beginning of unscanned data (we still need to count number of lines, insert newline characters,..)

ends

struct  window_data
        text            rb TEXT_BUFFERSIZE
        names           rb MAX_NICK_LEN * MAX_USERS
ends

include "encodings.inc"
include "window.inc"
include "serverparser.inc"
include "userparser.inc"
include "socket.inc"
include "gui.inc"
include "users.inc"
include "textbox.inc"


START:

        mcall   68, 11                  ; init heap so we can allocate memory dynamically

; wanted events
        mcall   40, EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_STACK+EVM_MOUSE+EVM_MOUSE_FILTER

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

; find path to main settings file (ircc.ini)
        mov     edi, path               ; Calculate the length of zero-terminated string
        xor     al, al
        mov     ecx, 1024
        repne   scasb
        dec     edi
        mov     eax, '.ini'
        stosd
        xor     al, al
        stosb

; Fill the window buffer with zeros
        mov     edi, windows
        mov     ecx, (sizeof.window*MAX_WINDOWS+3)/4
        xor     eax, eax
        rep     stosd

; clear command area too
        mov     edi, servercommand
        mov     ecx, 600/4
        rep     stosd

; allocate window data block
        mov     ebx, windows
        call    window_create_textbox
        test    eax, eax
        jz      error
        mov     [ebx + window.type], WINDOWTYPE_SERVER

; get system colors
        mcall   48, 3, colors, 40

; set edit box and scrollbar colors
        mov     eax, [colors.work]
        mov     [scroll1.bg_color], eax
        mov     [scroll2.bg_color], eax

        mov     eax, [colors.work_button]
        mov     [scroll1.front_color], eax
        mov     [scroll2.front_color], eax

        mov     eax, [colors.work_text]
        mov     [scroll1.line_color], eax
        mov     [scroll2.line_color], eax

        mov     [scroll1.type], 1               ; 0 = simple, 1 = skinned
        mov     [scroll2.type], 1

; get settings from ini
        invoke  ini.get_str, path, str_user, str_nick, user_nick, MAX_NICK_LEN, default_nick
        invoke  ini.get_str, path, str_user, str_real, user_real_name, MAX_REAL_LEN, default_real
        invoke  ini.get_str, path, str_user, str_quitmsg, quit_msg, 250, default_quit

; Welcome user
        mov     esi, str_welcome
        call    print_text2

; Check if parameter contains an URL
        cmp     byte[param], 0
        je      @f
        mov     esi, param
        mov     ecx, 1024
        call    cmd_usr_server.now
  @@:

; Draw window a first time, so we can figure out skin size
        call    draw_window

redraw:
        call    draw_window

mainloop:
        mcall   10              ; wait for event

        dec     eax
        jz      redraw

        dec     eax
        jz      main_window_key

        dec     eax
        jz      button

        cmp     al, 3
        je      mouse

        call    process_network_event

        mov     edi, [window_active]
        test    [edi + window.flags], FLAG_UPDATED
        jz      .no_update
        call    draw_channel_text
        mov     edi, [window_active]
        cmp     [edi + window.type], WINDOWTYPE_CHANNEL
        jne     .no_update
        call    draw_channel_list
  .no_update:

        jmp     mainloop

button:

        mcall   17              ; get id
        ror     eax, 8

        cmp     ax, 1           ; close program
        je      exit

        cmp     ax, WINDOW_BTN_CLOSE
        jne     @f
        call    cmd_usr_close_window
        jmp     mainloop

  @@:
        cmp     ax, WINDOW_BTN_LIST
        jne     @f

        push    eax

        mcall   37, 1           ; Get mouse position
        sub     ax, TEXT_Y
        mov     bl, FONT_HEIGHT
        div     bl
        and     eax, 0x000000ff
        inc     eax
        add     eax, [scroll1.position]
        mov     ebx, [window_active]
        mov     [ebx + window.selected], eax

        call    draw_channel_list

        pop     eax
        test    eax, 1 shl 25   ; Right mouse button pressed?
        jz      mainloop

; TODO: check if selected nick is my nick!

; Right mouse BTN was pressed, open chat window
        mov     ebx, [window_active]
        mov     eax, [ebx + window.selected]
        dec     eax
        imul    eax, MAX_NICK_LEN
        mov     ebx, [ebx + window.data_ptr]
        lea     esi, [ebx + window_data.names + eax]
        call    window_open
        push    [window_print]
        pop     [window_active]
        call    redraw

        jmp     mainloop

  @@:
        sub     ax, WINDOW_BTN_START
        jb      exit

        cmp     ax, MAX_WINDOWS
        ja      exit

; OK, time to switch to another window.
        mov     dx, sizeof.window
        mul     dx
        shl     edx, 16
        mov     dx, ax
        add     edx, windows
        cmp     [edx + window.type], WINDOWTYPE_NONE
        je      exit
        mov     [window_active], edx

        mov     [scroll2.position], 1           ;;; FIXME
        call    draw_window

        jmp     mainloop

exit:

        cmp     [socketnum], 0
        je      @f
        mov     esi, quit_msg
        call    cmd_usr_quit_server
  @@:

error:

        mcall   -1



main_window_key:

        mcall   2

        push    dword edit1
        call    [edit_box_key]

;        cmp     ah, 178
;        jne     .no_up
;
;        jmp     mainloop
;
;
;  .no_up:
;        cmp     ah, 177
;        jne     .no_down
;
;        jmp     mainloop
;
;  .no_down:
        cmp     ah, 13          ; enter
        jne     no_send2

        call    user_parser

        mov     eax, [edit1.size]

        mov     [edit1.size], 0
        mov     [edit1.pos], 0

        push    dword edit1
        call    [edit_box_draw]

        call    draw_channel_text

        jmp     mainloop
  no_send2:

        jmp     mainloop

mouse:
        push    dword edit1
        call    [edit_box_mouse]

;        mcall   37, 7
;        movsx   eax, ax
;        add     [scroll2.position], eax

; TODO: check if scrollbar is active?
        mov     edi, [window_active]
        cmp     [edi + window.type], WINDOWTYPE_CHANNEL
        jne     @f
        push    [scroll1.position]
        push    dword scroll1
        call    [scrollbar_mouse]
        pop     eax
        cmp     eax, [scroll1.position] ; did the scrollbar move?
        je      @f
        call    draw_channel_list
  @@:

; TODO: check if scrollbar is active?
        mov     edi, [window_active]
        mov     eax, [edi + window.text_lines]
        cmp     eax, [textbox_height]
        jbe     @f
        push    dword scroll2
        call    [scrollbar_mouse]
        mov     edi, [window_active]
        mov     edx, [scroll2.position]
        sub     edx, [edi + window.text_line_print]
        je      @f
        call    draw_channel_text.scroll_to_pos
  @@:

        jmp     mainloop


; DATA AREA

encoding_text:
db      'CP866 '
db      'CP1251'
db      'UTF-8 '
encoding_text_len = 6

join_header             db 3, '3* ', 0
quit_header             db 3, '5* ', 0
nick_header             db 3, '2* ', 0
kick_header             db 3, '5* ', 0
mode_header             db 3, '2* ', 0
part_header             db 3, '5* ', 0
topic_header            db 3, '3* ', 0
action_header           db 3, '6* ', 0
ctcp_header             db 3, '13-> [', 0
msg_header              db 3, '7-> *', 0
ctcp_version            db '] VERSION', 10, 0
ctcp_ping               db '] PING', 10, 0
ctcp_time               db '] TIME', 10, 0

has_left_channel        db ' has left ', 0
joins_channel           db ' has joined ', 0
is_now_known_as         db ' is now known as ', 0
has_quit_irc            db ' has quit IRC', 10, 0

sets_mode               db ' sets mode ', 0
str_kicked              db ' is kicked from ', 0
str_by                  db ' by ', 0
str_nickchange          db 'Nickname is now ', 0
str_realchange          db 'Real name is now ', 0
str_talking             db 'Now talking in ', 0
str_topic               db 'Topic is "', 0
str_topic_end           db '".', 10, 0
str_setby               db 'Set by ', 0

str_connecting          db 3, '3* Connecting to ', 0
str_sockerr             db 3, '5* Socket error', 10, 0
str_dnserr              db 3, '5* Unable to resolve hostname', 10, 0
str_refused             db 3, '5* Connection refused', 10, 0
str_disconnected        db 3, '5* Server disconnected', 10, 0
str_reconnect           db 3, '5* Connection reset by user', 10, 0
str_notconnected        db 3, '5* Not connected to server', 10, 0

str_dotnewline          db '.',10, 0
str_newline             db 10, 0
str_1                   db 3, '13 -', 0
str_2                   db '- ', 0

str_help                db 10, 'following commands are available:', 10
                        db 10
                        db '/nick <nick>        : change nickname to <nick>', 10
                        db '/real <real name>   : change real name to <real name>', 10
                        db '/server <address>   : connect to server <address>', 10
                        db '/code <code>        : change codepage to cp866, cp1251, or utf8', 10, 0

str_welcome             db 3, '3 ___', 3, '7__________', 3, '6_________  ', 3, '4         __   __               __', 10
                        db 3, '3|   \', 3, '7______   \', 3, '6_   ___ \ ', 3, '4   ____ |  | |__| ____   _____/  |_', 10
                        db 3, '3|   |', 3, '7|       _/', 3, '6    \  \/ ', 3, '4 _/ ___\|  | |  |/ __ \ /    \   __\', 10
                        db 3, '3|   |', 3, '7|    |   \', 3, '6     \____', 3, '4 \  \___|  |_|  \  ___/|   |  \  |', 10
                        db 3, '3|___|', 3, '7|____|_  /', 3, '6\______  /', 3, '4  \___  >____/__|\___  >___|  /__|', 10
                        db 3, '3     ', 3, '7       \/ ', 3, '6       \/ ', 3, '4      \/             \/     \/', 10
                        db 10
                        db 'Welcome to the KolibriOS IRC client v', version, 10
                        db 10
                        db 'Type /help for help', 10, 10, 0

str_version             db 'VERSION '
str_programname         db 'KolibriOS IRC client v', version, 0

str_user                db 'user', 0
str_nick                db 'nick', 0
str_real                db 'realname', 0
str_email               db 'email', 0
str_quitmsg             db 'quitmsg', 0

default_nick            db 'kolibri_user', 0
default_real            db 'Kolibri User', 0
default_quit            db 'KolibriOS forever', 0

irc_colors              dd 0xffffff     ;  0 white
                        dd 0x000000     ;  1 black
                        dd 0x00007f     ;  2 blue (navy)
                        dd 0x009300     ;  3 green
                        dd 0xff0000     ;  4 red
                        dd 0x7f0000     ;  5 brown (maroon)
                        dd 0x9c009c     ;  6 purple
                        dd 0xfc7f00     ;  7 olive
                        dd 0xffff00     ;  8 yellow
                        dd 0x00fc00     ;  9 light green
                        dd 0x009393     ; 10 teal
                        dd 0x00ffff     ; 11 cyan
                        dd 0x0000fc     ; 12 royal blue
                        dd 0xff00ff     ; 13 pink
                        dd 0x7f7f7f     ; 14 grey
                        dd 0xd4d0c4     ; 15 light grey (silver)

sockaddr1:
        dw AF_INET4
.port   dw 0x0b1a       ; 6667          FIXMEEEEEE
.ip     dd 0
        rb 10


status                  dd STATUS_DISCONNECTED


textbox_height          dd 12                   ; in characters
textbox_width           dd 78                   ; in characters, not pixels ;)

window_active           dd windows
window_print            dd windows

align 4
@IMPORT:

library network,        'network.obj',\
        libini,         'libini.obj',\
        boxlib,         'box_lib.obj'

import  network,\
        getaddrinfo,    'getaddrinfo',\
        freeaddrinfo,   'freeaddrinfo',\
        inet_ntoa,      'inet_ntoa'

import  libini,\
        ini.get_str,    'ini_get_str',\
        ini.get_int,    'ini_get_int'

import  boxlib,\
        edit_box_draw,  'edit_box',\
        edit_box_key,   'edit_box_key',\
        edit_box_mouse, 'edit_box_mouse',\
        scrollbar_draw, 'scrollbar_v_draw',\
        scrollbar_mouse,'scrollbar_v_mouse'

        ;         width, left, top
edit1   edit_box  0, 0, 0, 0xffffff, 0x6f9480, 0, 0, 0, USERCMD_MAX_SIZE, usercommand, mouse_dd, ed_focus, 25, 25
        ;         xsize, xpos, ysize, ypos, btn_height, max, cur, pos, bgcol, frcol, linecol
scroll1 scrollbar SCROLLBAR_WIDTH, 0, 0, TOP_Y, SCROLLBAR_WIDTH, 0, 0, 0, 0, 0, 0, 1
scroll2 scrollbar SCROLLBAR_WIDTH, 0, 0, TOP_Y, SCROLLBAR_WIDTH, 0, 0, 0, 0, 0, 0, 1

usercommand     db '/server chat.freenode.net', 0
                rb MAX_COMMAND_LEN

I_END:

utf8_bytes_rest dd ?            ; bytes rest in current UTF8 sequence
utf8_char       dd ?            ; first bits of current UTF8 character

packetbuf       rb 1024         ; buffer for packets to server
path            rb 1024
param           rb 1024

servercommand   rb 600

thread_info     rb 1024
xsize           dd ?
ysize           dd ?
mouse_dd        dd ?

colors          system_colors

irc_server_name rb MAX_SERVER_NAME      ; TODO: move this server URL into window struct
socketnum       dd ?                    ; TODO: same for socket

user_nick       rb MAX_NICK_LEN
user_real_name  rb MAX_REAL_LEN
quit_msg        rb 250

diff16 "windows", 0, $ + 1*sizeof.window ;+ 6
windows         rb MAX_WINDOWS*sizeof.window

IM_END:







