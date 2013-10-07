;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
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

version equ '0.1'

; connection status
STATUS_DISCONNECTED     = 0
STATUS_RESOLVING        = 1
STATUS_CONNECTING       = 2
STATUS_CONNECTED        = 3

; window flags
FLAG_UPDATED            = 1 shl 0
FLAG_CLOSE              = 1 shl 1
FLAG_RECEIVING_NAMES    = 1 shl 2

; window types
WINDOWTYPE_SERVER       = 0
WINDOWTYPE_CHANNEL      = 1
WINDOWTYPE_CHAT         = 2
WINDOWTYPE_LIST         = 3
WINDOWTYPE_DCC          = 4

; supported encodings
CP866                   = 0
CP1251                  = 1
UTF8                    = 2

; settings
USERCMD_MAX_SIZE        = 400

WIN_MIN_X               = 600
WIN_MIN_Y               = 165

TEXT_X                  = 5
TEXT_Y                  = 30

TOP_Y                   = 25

MAX_WINDOWS             = 20
MAX_USERS               = 4096

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

SCROLLBAR_WIDTH         = 12

USERLIST_X              = 98


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
include '../../develop/libraries/box_lib/trunk/box_lib.mac'

struct  window
        data_ptr        dd ?            ; zero if not used
        flags           db ?
        type            db ?
        name            rb MAX_WINDOWNAME_LEN
        users           dd ?
        users_scroll    dd ?
        selected        dd ?            ; selected user, 0 if none selected
ends

struct  window_data
        text            rb 120*60
        title           rb 256
        names           rb MAX_NICK_LEN * MAX_USERS
        usertext        rb 256
        usertextlen     dd ?
ends

include "encodings.inc"
include "window.inc"                    ; also contains text print routines
include "serverparser.inc"
include "userparser.inc"
include "socket.inc"
include "gui.inc"
include "users.inc"


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
        call    window_create
        mov     ebx, windows
        mov     [ebx + window.data_ptr], eax
        mov     [ebx + window.flags], 0
        mov     [ebx + window.type], WINDOWTYPE_SERVER
        add     eax, window_data.text
        mov     [text_start], eax

        call    window_refresh

; get system colors
        mcall   48, 3, colors, 40

; set edit box and scrollbar colors
        mov     eax, [colors.work]
        mov     [scroll1.bg_color], eax

        mov     eax, [colors.work_button]
        mov     [scroll1.front_color], eax

        mov     eax, [colors.work_text]
        mov     [scroll1.line_color], eax

; get settings from ini
        invoke  ini.get_str, path, str_user, str_nick, user_nick, MAX_NICK_LEN, default_nick
        invoke  ini.get_str, path, str_user, str_real, user_real_name, MAX_REAL_LEN, default_real
        invoke  ini.get_str, path, str_user, str_quitmsg, quit_msg, 250, default_quit

; Welcome user
        mov     esi, str_welcome
        call    print_text2

        call    draw_window ;;; FIXME (gui is not correctly drawn first time because of window sizes)

redraw:
        call    draw_window

still:

; wait here for event
        mcall   10

        dec     eax
        jz      redraw

        dec     eax
        jz      main_window_key

        dec     eax
        jz      button

        cmp     al, 3
        je      mouse

        call    process_network_event

        mov     edx, [window_active]
        test    [edx + window.flags], FLAG_UPDATED
        jz      .no_update
        and     [edx + window.flags], not FLAG_UPDATED
        mov     edx, [edx + window.data_ptr]
        add     edx, window_data.text
        call    draw_channel_text
  .no_update:
        call    print_channel_list

        jmp     still

button:

        mcall   17              ; get id
        ror     eax, 8

        cmp     ax, 1           ; close program
        je      exit

        cmp     ax, WINDOW_BTN_CLOSE
        jne     @f

        call    window_close
        jmp     still

  @@:
        cmp     ax, WINDOW_BTN_LIST
        jne     @f

        push    eax

        mcall   37, 1           ; Get mouse position
        sub     ax, TEXT_Y
        mov     bl, 10
        div     bl
        and     eax, 0x000000ff
        inc     eax
        add     eax, [scroll1.position]
        mov     ebx, [window_active]
        mov     [ebx + window.selected], eax

        call    print_channel_list

        pop     eax
        test    eax, 1 shl 25   ; Right mouse button pressed?
        jz      still

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

        jmp     still

  @@:
        sub     ax, WINDOW_BTN_START
        jb      exit

        cmp     ax, MAX_WINDOWS
        ja      exit

        mov     dx, sizeof.window
        mul     dx
        shl     edx, 16
        mov     dx, ax
        add     edx, windows
        cmp     [edx + window.data_ptr], 0
        je      exit
        mov     [window_active], edx
        call    window_refresh
        call    draw_window

        jmp     still

exit:

        cmp     [socketnum], 0
        je      @f
        mov     esi, quit_msg
        call    cmd_usr_quit_server
  @@:

        mcall   -1



main_window_key:

        mcall   2

        push    dword edit1
        call    [edit_box_key]

        cmp     ah, 13          ; enter
        jne     no_send2

        call    user_parser

        mov     [edit1.size], 0
        mov     [edit1.pos], 0

        push    dword edit1
        call    [edit_box_draw]

        mov     edx, [window_active]
        mov     edx, [edx + window.data_ptr]
        add     edx, window_data.text
        call    draw_channel_text

        jmp     still
  no_send2:

        jmp     still

mouse:
        push    dword edit1
        call    [edit_box_mouse]

; TODO: check if scrollbar is active
        push    [scroll1.position]
        push    dword scroll1
        call    [scrollbar_v_mouse]
        pop     eax
        cmp     eax, [scroll1.position] ; did the scrollbar move?
        je      @f
        call    print_channel_list
  @@:

        jmp     still


; DATA AREA

encoding_text:
db      'CP866 '
db      'CP1251'
db      'UTF-8 '
encoding_text_len = 6

action_header           db '*** ', 0
action_header_short     db '* ', 0
ctcp_header             db '-> [',0
ctcp_version            db '] VERSION',10,0
ctcp_ping               db '] PING',10,0
ctcp_time               db '] TIME',10,0
has_left_channel        db ' has left ', 0
joins_channel           db ' has joined ', 0
is_now_known_as         db ' is now known as ', 0
has_quit_irc            db ' has quit IRC', 10, 0
sets_mode               db ' sets mode ', 0
kicked                  db ' is kicked from ', 0
str_talking             db 'Now talking in ',0
str_topic               db 'Topic is ',0
str_setby               db 'Set by ',0

str_version             db 'VERSION '
str_programname         db 'KolibriOS IRC client ', version, 0

str_user                db 'user', 0
str_nick                db 'nick', 0
str_real                db 'realname', 0
str_email               db 'email', 0
str_quitmsg             db 'quitmsg', 0

default_nick            db 'kolibri_user', 0
default_real            db 'Kolibri User', 0
default_quit            db 'KolibriOS forever', 0

str_welcome             db 10
                        db ' ______________________           __   __               __',10
                        db '|   \______   \_   ___ \    ____ |  | |__| ____   _____/  |_',10
                        db '|   ||       _/    \  \/  _/ ___\|  | |  |/ __ \ /    \   __\',10
                        db '|   ||    |   \     \____ \  \___|  |_|  \  ___/|   |  \  |',10
                        db '|___||____|_  /\______  /  \___  >____/__|\___  >___|  /__|',10
                        db '            \/        \/       \/             \/     \/',10
                        db 10
                        db 'Welcome to IRC client ',version,' for KolibriOS',10
                        db 10
                        db 'Type /help for help',10,0

str_nickchange          db 'Nickname is now ',0
str_realchange          db 'Real name is now ',0
str_dotnewline          db '.',10, 0
str_newline             db 10, 0
str_connecting          db 10,'* Connecting to ',0
str_help                db 10,'following commands are available:',10
                        db 10
                        db '/nick <nick>        : change nickname to <nick>',10
                        db '/real <real name>   : change real name to <real name>',10
                        db '/server <address>   : connect to server <address>',10
                        db '/code <code>        : change codepage to cp866, cp1251, or utf8',10,0

str_1                   db ' -',0
str_2                   db '- ',0

str_sockerr             db 'Socket Error',10,0
str_dnserr              db 'Unable to resolve hostname.',10,0
str_refused             db 'Connection refused',10,0

sockaddr1:
        dw AF_INET4
.port   dw 0x0b1a       ; 6667
.ip     dd 0
        rb 10


status                  dd STATUS_DISCONNECTED

text_start              dd ?                    ; pointer to current textbox data
irc_data                dd 0x0                  ; encoder
textbox_width           dd 80                   ; in characters, not pixels ;)
pos                     dd 66 * 11              ; encoder

window_active           dd windows
window_print            dd windows

scroll                  dd 1
                        dd 12

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
        edit_box_draw    ,'edit_box'            ,\
        edit_box_key     ,'edit_box_key'        ,\
        edit_box_mouse   ,'edit_box_mouse'      ,\
        scrollbar_v_draw ,'scrollbar_v_draw'    ,\
        scrollbar_v_mouse,'scrollbar_v_mouse'

I_END:

        ;         width, left, top
edit1   edit_box  0, 0, 0, 0xffffff, 0x6f9480, 0, 0, 0, USERCMD_MAX_SIZE, usercommand, mouse_dd, ed_focus, 25, 25
        ;         xsize, xpos, ysize, ypos, max, cur, pos, bgcol, frcol, linecol
scroll1 scrollbar SCROLLBAR_WIDTH, 300, 150, TOP_Y, 10, 100, 0, 0, 0, 0, 0, 1
scroll2 scrollbar SCROLLBAR_WIDTH, 300, 150, TOP_Y, 10, 100, 0, 0, 0, 0, 0, 1

usercommand     db '/server chat.freenode.net', 0
                rb MAX_COMMAND_LEN

main_PID        dd ?            ; identifier of main thread
utf8_bytes_rest dd ?            ; bytes rest in current UTF8 sequence
utf8_char       dd ?            ; first bits of current UTF8 character
gai_reqdata     rb 32           ; buffer for getaddrinfo_start/process
ip_list         dd ?            ; will be filled as pointer to addrinfo list
packetbuf       rb 1024         ; buffer for packets to server
path            rb 1024
param           rb 1024

socketnum       dd ?

servercommand   rb 600

thread_info     rb 1024
xsize           dd ?
ysize           dd ?

colors          system_colors

irc_server_name rb MAX_SERVER_NAME

user_nick       rb MAX_NICK_LEN
user_real_name  rb MAX_REAL_LEN
quit_msg        rb 250

windows         rb MAX_WINDOWS*sizeof.window

mouse_dd        dd ?

IM_END:







