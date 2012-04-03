;
; Kolibrios FTP Daemon
;
; hidnplayr@gmail.com
;
; GPLv2
;

BUFFERSIZE      equ 4096


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
        push    25
        push    80
        push    25
        push    80
        call    [con_init]

        mcall   40, 1 shl 7     ; we only want network events

        push    str1
        call    [con_write_asciiz]

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      sock_err

        mov     [socketnum], eax

;;        mcall   setsockopt, [socketnum], SOL_SOCKET, SO_REUSEADDR, &yes,
;;        cmp     eax, -1
;;        je      opt_err

        invoke  ini.get_int, path, str_ftpd, str_port, 21
        mov     [sockaddr1.port], ax

        mcall   bind, [socketnum], sockaddr1, sockaddr1.length
        cmp     eax, -1
        je      bind_err

        invoke  ini.get_int, path, str_ftpd, str_conn, 1        ; Backlog (max connections)
        mov     edx, eax
        mcall   listen, [socketnum]
        cmp     eax, -1
        je      listen_err

        push    str2
        call    [con_write_asciiz]

        mcall   10

        mcall   accept, [socketnum], sockaddr1, sockaddr1.length
        cmp     eax, -1
        je      acpt_err

        mov     [socketnum2], eax

;;        mcall   close, [socketnum]

        mcall   send, [socketnum2], str220, str220.length       ; send welcome string

  .loop:
        mcall   10

        mcall   recv, [socketnum2], buffer, buffer.length

        push    buffer
        call    [con_write_asciiz]

        mov     esi, buffer
        call    parse_cmd

        jmp     .loop

acpt_err:
        push    str8
        call    [con_write_asciiz]
        jmp     done

listen_err:
        push    str3
        call    [con_write_asciiz]
        jmp     done

bind_err:
        push    str4
        call    [con_write_asciiz]
        jmp     done

sock_err:
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
title   db      'KolibriOS FTP daemon 1.0',0
str1    db      'Opening socket',10, 0
str2    db      'Listening for incoming connections...',10,0
str3    db      'Listen error',10,10,0
str4    db      'Bind error',10,10,0
str5    db      'Setsockopt error.',10,10,0
str6    db      'Could not open socket',10,10,0
str7    db      'Got data!',10,10,0
str8    db      'Error accepting connection',10,10,0

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
        con_set_cursor_pos, 'con_set_cursor_pos'

import  libini,         \
        ini.get_str,    'ini_get_str',\
        ini.get_int,    'ini_get_int'

import  libio,          \
        libio.init , 'lib_init'   , \
        file.size  , 'file_size'  , \
        file.open  , 'file_open'  , \
        file.read  , 'file_read'  , \
        file.close , 'file_close'


i_end:

socketnum       dd ?
socketnum2      dd ?

buffer          rb BUFFERSIZE
.length = BUFFERSIZE

path    rb 1024
mem:
