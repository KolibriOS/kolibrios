;
; Kolibrios FTP Daemon
;
; hidnplayr@gmail.com
;
; GPLv2
;

BUFFERSIZE              = 8192

; using multiple's of 4
STATE_CONNECTED         = 4*0
STATE_LOGIN             = 4*1
STATE_LOGIN_FAIL        = 4*2           ; When an invalid username was given
STATE_ACTIVE            = 4*3

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
MODE_PASSIVE_FAILED     = 4

PERMISSION_EXEC         = 1b            ; LIST
PERMISSION_READ         = 10b
PERMISSION_WRITE        = 100b
PERMISSION_DELETE       = 1000b
PERMISSION_CD           = 10000b        ; Change Directory

ABORT                   = 1 shl 31

format binary as ""

use32
        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+0x1000      ; required memory
        dd      mem+0x1000      ; stack pointer
        dd      params          ; parameters
        dd      path            ; path

include '../macros.inc'
purge mov,add,sub
include '../proc32.inc'
include '../dll.inc'
include '../struct.inc'
include '../libio.inc'

include '../network.inc'
include 'commands.inc'

start:
        mcall   68, 11                  ; init heap
        mcall   40, 1 shl 7             ; we only want network events

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

; find path to main settings file (ftpd.ini)
        mov     edi, path               ; Calculate the length of zero-terminated string
        xor     al, al
        mov     ecx, 1024
        repne   scasb
        dec     edi
        mov     esi, str_ini            ; append it with '.ini', 0
        movsd
        movsb

; now create the second path (users.ini)
        std
        mov     al, '/'
        repne   scasb
        lea     ecx, [edi - path + 2]
        cld
        mov     esi, path
        mov     edi, path2
        rep     movsb
        mov     esi, str_users
        movsd
        movsd
        movsw

; initialize console
        invoke  con_start, 1
        invoke  con_init, -1, -1, -1, -1, title

        invoke  ini.get_str, path, str_ftpd, str_ip, ini_buf, 16, 0
        mov     esi, ini_buf
        mov     cl, '.'
        call    ip_to_dword
        mov     [serverip], ebx

        invoke  ini.get_int, path, str_ftpd, str_port, 21
        mov     [sockaddr1.port], ax

        invoke  con_printf, str1, eax
        add     esp, 8

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      sock_err
        mov     [socketnum], eax

        invoke  con_write_asciiz, str2

;        mcall   setsockopt, [socketnum], SOL_SOCKET, SO_REUSEADDR, &yes,
;        cmp     eax, -1
;        je      opt_err

        mcall   bind, [socketnum], sockaddr1, sockaddr1.length
        cmp     eax, -1
        je      bind_err

        invoke  con_write_asciiz, str2

        invoke  ini.get_int, path, str_ftpd, str_conn, 1        ; Backlog (max connections)
        mov     edx, eax

        invoke  con_write_asciiz, str2

        mcall   listen, [socketnum]
        cmp     eax, -1
        je      listen_err

        invoke  con_write_asciiz, str2b

        invoke  ini.get_int, path, str_pasv, str_start, 2000
        mov     [pasv_start], ax
        invoke  ini.get_int, path, str_pasv, str_end, 5000
        mov     [pasv_end], ax

        mov     [alive], 1

mainloop:
        mcall   23, 100                         ; Wait here for incoming connections on the base socket (socketnum)
                                                ; One second timeout, we will use this to check if console is still working

        test    eax, eax                        ; network event?
        jz      .checkconsole

        mcall   51, 1, threadstart, 0           ; Start a new thread for every incoming connection
                                                ; NOTE: upon initialisation of the thread, stack will not be available!
        jmp     mainloop

  .checkconsole:

        invoke  con_get_flags                   ; Is console still running?
        test    eax, 0x0200
        jz      mainloop
        mcall   close, [socketnum]              ; kill the listening socket
        mov     [alive], 0
        mcall   -1                              ; and exit

        diff16  "threadstart", 0, $

threadstart:
;;;        mcall   68, 11                          ; init heap
        mcall   68, 12, sizeof.thread_data      ; allocate the thread data struct
        test    eax, eax
        je      exit

        lea     esp, [eax + thread_data.stack]  ; init stack
        mov     ebp, eax

        mcall   40, 1 shl 7                     ; we only want network events for this thread

        lea     ebx, [ebp + thread_data.buffer] ; get information about the current process
        or      ecx, -1
        mcall   9
        mov     eax, dword [ebp + thread_data.buffer + 30]              ; PID is at offset 30
        mov     [ebp + thread_data.pid], eax

        invoke  con_set_flags, 0x03
        invoke  con_printf, str8, [ebp + thread_data.pid]               ; print on the console that we have created the new thread successfully
        add     esp, 8                                                  ; balance stack
        invoke  con_set_flags, 0x07

        mcall   accept, [socketnum], sockaddr1, sockaddr1.length        ; time to accept the awaiting connection..
        cmp     eax, -1
        je      thread_exit
        mov     [ebp + thread_data.socketnum], eax

        mov     [ebp + thread_data.state], STATE_CONNECTED
        mov     [ebp + thread_data.permissions], 0
        mov     [ebp + thread_data.mode], MODE_NOTREADY
        lea     eax, [ebp + thread_data.buffer]
        mov     [ebp + thread_data.buffer_ptr], eax
        mov     [ebp + thread_data.passivesocknum], -1

        sendFTP "220 Welcome to KolibriOS FTP daemon"

        diff16  "threadloop", 0, $
threadloop:
; Check if our socket is still connected
        mcall   send, [ebp + thread_data.socketnum], 0, 0       ; Try to send zero bytes, if socket is closed, this will return -1
        cmp     eax, -1
        je      thread_exit

        cmp     [alive], 0                                      ; Did main thread take a run for it?
        je      thread_exit

        mcall   10                                              ; Wait for network event

        cmp     [ebp + thread_data.mode], MODE_PASSIVE_WAIT
        jne     .not_passive
        mov     ecx, [ebp + thread_data.passivesocknum]
        lea     edx, [ebp + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   accept
        cmp     eax, -1
        je      .not_passive
        mov     [ebp + thread_data.datasocketnum], eax
        mov     [ebp + thread_data.mode], MODE_PASSIVE_OK
        mcall   close   ; [ebp + thread_data.passivesocknum]
        mov     [ebp + thread_data.passivesocknum], -1

        invoke  con_write_asciiz, str_datasock
  .not_passive:

        mov     ecx, [ebp + thread_data.socketnum]
        mov     edx, [ebp + thread_data.buffer_ptr]
        mov     esi, sizeof.thread_data.buffer    ;;; FIXME
        mcall   recv
        inc     eax                                     ; error? (-1)
        jz      threadloop
        dec     eax                                     ; 0 bytes read?
        jz      threadloop

        mov     edi, [ebp + thread_data.buffer_ptr]
        add     [ebp + thread_data.buffer_ptr], eax

; Check if we received a newline character, if not, wait for more data
        mov     ecx, eax
        mov     al, 13
        repne   scasb
        jne     threadloop

; We got a command!
        mov     byte [edi + 1], 0                       ; append string with zero byte
        lea     esi, [ebp + thread_data.buffer]
        mov     ecx, [ebp + thread_data.buffer_ptr]
        sub     ecx, esi
        mov     [ebp + thread_data.buffer_ptr], esi     ; reset buffer ptr

        invoke  con_set_flags, 0x02                     ; print received data to console (in green color)
        invoke  con_write_asciiz, str_newline
        invoke  con_write_asciiz, esi
        invoke  con_set_flags, 0x07

        push    threadloop
        jmp     parse_cmd

listen_err:
        invoke  con_set_flags, 0x0c                     ; print errors in red
        invoke  con_write_asciiz, str3
        jmp     done

bind_err:
        invoke  con_set_flags, 0x0c                     ; print errors in red
        invoke  con_write_asciiz, str4
        jmp     done

sock_err:
        invoke  con_set_flags, 0x0c                     ; print errors in red
        invoke  con_write_asciiz, str6
        jmp     done

done:
        invoke  con_exit, 0
exit:
        mcall   -1


thread_exit:
        invoke  con_set_flags, 0x03                             ; print thread info in blue
        invoke  con_printf, str_bye, [ebp + thread_data.pid]    ; print on the console that we are about to kill the thread
        add     esp, 8                                          ; balance stack
        mcall   68, 13, ebp                                     ; free the memory
        mcall   -1                                              ; and kill the thread


; initialized data

title           db 'KolibriOS FTP daemon 0.1', 0
str1            db 'Starting FTP daemon on port %u.', 0
str2            db '.', 0
str2b           db ' OK!',10,0
str3            db 'Listen error',10,0
str4            db 10,'ERROR: local port is already in use.',10,0
;str5            db 'Setsockopt error.',10,10,0
str6            db 'ERROR: Could not open socket.',10,0
str7            db 'Got data!',10,10,0
str8            db 10,'Thread %d created',10,0
str_bye         db 10,'Thread %d killed',10,0

str_logged_in   db 'Login ok',10,0
str_pass_ok     db 'Password ok',10,0
str_pass_err    db 'Password/Username incorrect',10,0
str_pwd         db 'Current directory is "%s"\n',0
str_err2        db 'ERROR: cannot open the directory.',10,0
str_datasock    db 'Passive data socket connected.',10,0
str_notfound    db 'ERROR: file not found.',10,0
str_sockerr     db 'ERROR: socket error.',10,0

str_newline     db 10, 0
str_mask        db '*', 0
str_infinity    db 0xff, 0xff, 0xff, 0xff, 0

months          dd 'Jan '
                dd 'Feb '
                dd 'Mar '
                dd 'Apr '
                dd 'May '
                dd 'Jun '
                dd 'Jul '
                dd 'Aug '
                dd 'Sep '
                dd 'Oct '
                dd 'Nov '
                dd 'Dec '

str_users       db 'users'
str_ini         db '.ini', 0
str_port        db 'port', 0
str_ftpd        db 'ftpd', 0
str_conn        db 'conn', 0
str_ip          db 'ip', 0
str_pass        db 'pass', 0
str_home        db 'home', 0
str_mode        db 'mode', 0
str_pasv        db 'pasv', 0
str_start       db 'start', 0
str_end         db 'end', 0


sockaddr1:
                dw AF_INET4
  .port         dw 21
  .ip           dd 0
                rb 10
  .length       = $ - sockaddr1

; import

align 4
@IMPORT:

diff16 "import", 0, $

library console,                'console.obj',\
        libini,                 'libini.obj', \
        libio,                  'libio.obj'

import  console,\
        con_start,              'START',\
        con_init,               'con_init',\
        con_write_asciiz,       'con_write_asciiz',\
        con_exit,               'con_exit',\
        con_gets,               'con_gets',\
        con_cls,                'con_cls',\
        con_printf,             'con_printf',\
        con_getch2,             'con_getch2',\
        con_set_cursor_pos,     'con_set_cursor_pos',\
        con_set_flags,          'con_set_flags',\
        con_get_flags,          'con_get_flags'

import  libini,\
        ini.get_str,            'ini_get_str',\
        ini.get_int,            'ini_get_int'

import  libio,\
        libio.init,             'lib_init',\
        file.size,              'file_size',\
        file.open,              'file_open',\
        file.read,              'file_read',\
        file.close,             'file_close',\
        file.find.first,        'file_find_first',\
        file.find.next,         'file_find_next',\
        file.find.close,        'file_find_close'


i_end:

diff16 "i_end", 0, $

; uninitialised data

        socketnum       dd ?
        path            rb 1024
        path2           rb 1024
        params          rb 1024
        serverip        dd ?
        pasv_start      dw ?
        pasv_end        dw ?
        pasv_port       dw ?

        ini_buf         rb 3*4+3+1

        alive           db ?

mem:


