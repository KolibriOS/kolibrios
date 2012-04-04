;
; Kolibrios FTP Daemon
;
; hidnplayr@gmail.com
;
; GPLv2
;

BUFFERSIZE              = 4096

STATE_DISCONNECTED      = 0
STATE_CONNECTED         = 1
STATE_LOGIN             = 2
STATE_ACTIVE            = 3

TYPE_UNDEF              = 0

TYPE_ASCII              = 00000100b
TYPE_EBDIC              = 00001000b
; subtypes for ascii & ebdic (np = default)
TYPE_NP                 = 00000001b     ; non printable
TYPE_TELNET             = 00000010b
TYPE_ASA                = 00000011b

TYPE_IMAGE              = 01000000b     ; binary data
TYPE_LOCAL              = 10000000b     ; bits per byte must be specified
                                        ; lower 4 bits will hold this value

MODE_NOTREADY           = 0
MODE_ACTIVE             = 1
MODE_PASSIVE_WAIT       = 2
MODE_PASSIVE_OK         = 3

use32
        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+0x1000      ; required memory
        dd      mem+0x1000      ; stack pointer
        dd      0               ; parameters
        dd      path            ; path

include '../macros.inc'
purge mov,add,sub
include '../proc32.inc'
include '../dll.inc'
include '../struct.inc'
include '../libio.inc'

include '../network.inc'
include 'commands.inc'

align 4
start:
; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

; find path to main settings file
        mov     edi, path      ; Calculate the length of zero-terminated string
        xor     al , al
        mov     ecx, 1024
        repne   scasb
        dec     edi
        mov     esi, filename
        movsd
        movsb

; initialize console
        push    1
        call    [con_start]

        push    title
        push    -1
        push    -1
        push    -1
        push    -1
        call    [con_init]

        mcall   40, 1 shl 7     ; we only want network events

        invoke  ini.get_int, path, str_ftpd, str_port, 21
        mov     [sockaddr1.port], ax

        push    eax
        push    str1
        call    [con_printf]

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      sock_err

        mov     [socketnum], eax

        push    str2
        call    [con_write_asciiz]

;;        mcall   setsockopt, [socketnum], SOL_SOCKET, SO_REUSEADDR, &yes,
;;        cmp     eax, -1
;;        je      opt_err

        mcall   bind, [socketnum], sockaddr1, sockaddr1.length
        cmp     eax, -1
        je      bind_err

        push    str2
        call    [con_write_asciiz]

        invoke  ini.get_int, path, str_ftpd, str_conn, 1        ; Backlog (max connections)
        mov     edx, eax

        push    str2
        call    [con_write_asciiz]

        mcall   listen, [socketnum]
        cmp     eax, -1
        je      listen_err

        push    str2b
        call    [con_write_asciiz]

        mcall   10

        mcall   accept, [socketnum], sockaddr1, sockaddr1.length
        cmp     eax, -1
        je      acpt_err

        mov     [socketnum2], eax

        mcall   send, [socketnum2], str220, str220.length, 0    ; send welcome string

  .loop:
        mcall   10

        cmp     [mode], MODE_PASSIVE_WAIT
        jne     @f
        mcall   accept, [passivesocknum], datasock, datasock.length
        cmp     eax, -1
        je      @f
        mov     [datasocketnum], eax
        mov     [mode], MODE_PASSIVE_OK

        push    str_datasock
        call    [con_write_asciiz]
       @@:

        mcall   recv, [socketnum2], buffer, buffer.length
        cmp     eax, -1
        je      .loop
        or      eax, eax
        jz      .loop
        push    eax

        mov     byte[buffer+eax], 0

        pushd   0x0a
        call    [con_set_flags]
        push    buffer
        call    [con_write_asciiz]
        pushd   0x07
        call    [con_set_flags]

        pop     ecx
        mov     esi, buffer
        call    parse_cmd

        jmp     .loop

acpt_err:

        pushd   0x0c
        call    [con_set_flags]

        push    str8
        call    [con_write_asciiz]
        jmp     done

listen_err:

        pushd   0x0c
        call    [con_set_flags]

        push    str3
        call    [con_write_asciiz]
        jmp     done

bind_err:

        pushd   0x0c
        call    [con_set_flags]

        push    str4
        call    [con_write_asciiz]
        jmp     done

sock_err:

        pushd   0x0c
        call    [con_set_flags]

        push    str6
        call    [con_write_asciiz]
        jmp     done

done:
        call    [con_getch2]
        push    1
        call    [con_exit]
exit:
        mcall   -1



; data
title   db      'KolibriOS FTP daemon 0.1', 0
str1    db      'Starting FTP daemon on port %u', 0
str2    db      '.', 0
str2b   db      ' OK!',10,10,0
str3    db      'Listen error',10,10,0
str4    db      'Bind error',10,10,0
str5    db      'Setsockopt error.',10,10,0
str6    db      'Could not open socket',10,10,0
str7    db      'Got data!',10,10,0
str8    db      'Error accepting connection',10,10,0

str_logged_in   db 'Login ok',10,10,0
str_pass_ok     db 'Password ok - Logged in',10,10,0
str_pwd         db 'Current directory is "%s"\n',0
str_err1        db 'ERROR: cannot connect to remote socket',10,10,0
str_err2        db 'ERROR: cannot open directory',10,10,0
str_datasock    db 'Passive data socket connected!',10,10,0


str_mask        db '*', 0


months:
        dd     'Jan ','Feb ','Mar ','Apr ','May ','Jun '
        dd     'Jul ','Aug ','Sep ','Oct ','Nov ','Dec '

filename db '.ini', 0
str_port db 'port', 0
str_ftpd db 'ftpd', 0
str_conn db 'conn', 0

sockaddr1:
        dw AF_INET4
.port   dw 21
.ip     dd 0
        rb 10
.length = $ - sockaddr1

; import
align 4
@IMPORT:

library console, 'console.obj', \
        libini, 'libini.obj', \
        libio, 'libio.obj'

import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets',\
        con_cls,        'con_cls',\
        con_printf,     'con_printf',\
        con_getch2,     'con_getch2',\
        con_set_cursor_pos, 'con_set_cursor_pos',\
        con_set_flags,  'con_set_flags'

import  libini,         \
        ini.get_str,    'ini_get_str',\
        ini.get_int,    'ini_get_int'

import  libio,          \
        libio.init , 'lib_init'   , \
        file.size  , 'file_size'  , \
        file.open  , 'file_open'  , \
        file.read  , 'file_read'  , \
        file.close , 'file_close' , \
        file.find.first , 'file_find_first', \
        file.find.next ,  'file_find_next', \
        file.find.close , 'file_find_close'


i_end:

socketnum       dd ?


; thread specific data
socketnum2      dd ?
state           dd ?
home_dir        db '/rd/1/', 0
                rb 1024
work_dir        rb 1024
fpath           rb 2048

type            db ?
mode            db ?    ; active/passive

passivesocknum  dd ?
datasocketnum   dd ?

datasock:
        dw AF_INET4
.port   dw ?
.ip     dd ?
        rb 10
.length = $ - datasock

buffer  rb BUFFERSIZE
.length = $ - buffer

path    rb 1024
mem:
