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

align 4
start:
; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        mcall   68, 11                  ; init heap

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
        push    1
        call    [con_start]

        push    title
        push    -1
        push    -1
        push    -1
        push    -1
        call    [con_init]

        mcall   40, 1 shl 7             ; we only want network events

        invoke  ini.get_str, path, str_ftpd, str_ip, ini_buf, 16, 0
        mov     esi, ini_buf
        call    ip_to_dword
        mov     [serverip], ebx

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

;        mcall   setsockopt, [socketnum], SOL_SOCKET, SO_REUSEADDR, &yes,
;        cmp     eax, -1
;        je      opt_err

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

mainloop:
        mcall   10                              ; Wait here for incoming connections on the base socket (socketnum)

        mcall   51, 1, threadstart, 0           ; Start a new thread for every incoming connection
                                                ; NOTE: upon initialisation of the thread, stack will not be available!
        jmp     mainloop

        diff16  "threadstart", 0, $
threadstart:
        mcall   68, 12, sizeof.thread_data      ; allocate the thread data struct
        cmp     eax, -1
        je      exit

        lea     esp, [eax + thread_data.stack]  ; init stack
        push    eax                             ; save pointer to thread_data on stack
        mov     ebp, esp

        mcall   40, 1 shl 7                     ; we only want network events for this thread

        pushd   0x03
        call    [con_set_flags]
        push    str8
        call    [con_write_asciiz]                                              ; print on the console that we have created the new thread successfully
        pushd   0x07
        call    [con_set_flags]

        mcall   accept, [socketnum], sockaddr1, sockaddr1.length                ; time to accept the awaiting connection..
        cmp     eax, -1
        je      thread_exit
        mov     edx, [ebp]                                                      ; pointer to thread_data
        mov     [edx + thread_data.socketnum], eax

        mov     [edx + thread_data.state], STATE_CONNECTED
        mov     [edx + thread_data.permissions], 0
        mov     [edx + thread_data.mode], MODE_NOTREADY
        lea     eax, [edx + thread_data.buffer]
        mov     [edx + thread_data.buffer_ptr], eax

        sendFTP "220 Welcome to KolibriOS FTP daemon"

threadloop:
        mcall   10

        mov     edx, [ebp]                                                      ; pointer to thread_data

        cmp     [edx + thread_data.mode], MODE_PASSIVE_WAIT
        jne     .not_passive
        mov     ecx, [edx + thread_data.passivesocknum]
        lea     edx, [edx + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   accept
        mov     edx, [ebp]                                                      ; pointer to thread_data
        cmp     eax, -1
        je      .not_passive
        mov     [edx + thread_data.datasocketnum], eax
        mov     [edx + thread_data.mode], MODE_PASSIVE_FAILED

        push    str_datasock
        call    [con_write_asciiz]                                              ; print on the console that the datasock is now ready
  .not_passive:

        mov     ecx, [edx + thread_data.socketnum]
        mov     edx, [edx + thread_data.buffer_ptr]
        mov     esi, sizeof.thread_data.buffer    ;;; FIXME
        mcall   recv
        inc     eax                                                             ; error? (-1)
        jz      threadloop
        dec     eax                                                             ; 0 bytes read?
        jz      threadloop

        mov     edx, [ebp]                                                      ; pointer to thread_data
        mov     edi, [edx + thread_data.buffer_ptr]
        add     [edx + thread_data.buffer_ptr], eax

; Check if we received a newline character, if not, wait for more data
        mov     ecx, eax
        mov     al, 13
        repne   scasb
        jne     threadloop

; We got a command!
        lea     eax, [edx + thread_data.buffer]
        mov     ecx, [edx + thread_data.buffer_ptr]
        sub     ecx, eax
        push    ecx                                                             ; push full data size on stack
        mov     [edx + thread_data.buffer_ptr], eax                             ; reset buffer ptr

        push    eax;;;;
        pushd   0x02                                                            ; print received data to console (in green color)
        call    [con_set_flags]
        push    str_newline
        call    [con_write_asciiz]
;;;;        push    eax
        call    [con_write_asciiz]
        pushd   0x07
        call    [con_set_flags]

        mov     edx, [ebp]
        pop     ecx                                                             ; number of bytes read
        lea     esi, [edx + thread_data.buffer]
        call    parse_cmd

        jmp     threadloop

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


thread_exit:
        push    str_bye
        call    [con_write_asciiz]      ; say bye bye
        pop     ecx                     ; get the thread_data pointer from stack
        mcall   68, 13                  ; free the memory
        mcall   -1                      ; and kill the thread



; initialized data

title           db 'KolibriOS FTP daemon 0.1', 0
str1            db 'Starting FTP daemon on port %u', 0
str2            db '.', 0
str2b           db ' OK!',10,0
str3            db 'Listen error',10,0
str4            db 'Bind error',10,0
;str5            db 'Setsockopt error.',10,10,0
str6            db 'Could not open socket',10,0
str7            db 'Got data!',10,10,0
str8            db 10,'New thread created!',10,0
str_bye         db 10,'Closing thread!',10,0

str_logged_in   db 'Login ok',10,0
str_pass_ok     db 'Password ok - Logged in',10,0
str_pwd         db 'Current directory is "%s"\n',0
str_err2        db 'ERROR: cannot open directory',10,0
str_datasock    db 'Passive data socket connected!',10,0
str_notfound    db 'ERROR: file not found',10,0
str_sockerr     db 'ERROR: socket error',10,0

str_login_invalid db 'Login invalid',10,0

str_newline     db 10, 0
str_mask        db '*', 0

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


sockaddr1:
                dw AF_INET4
  .port         dw 21
  .ip           dd 0
                rb 10
  .length       = $ - sockaddr1

; import

align 4
@IMPORT:

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
        con_set_flags,          'con_set_flags'

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

; uninitialised data

        socketnum       dd ?
        path            rb 1024
        path2           rb 1024
        params          rb 1024
        serverip        dd ?

        ini_buf         rb 3*4+3+1

mem:


