

struct  thread_data
                        rb 1024
        stack           rb 0

        home_dir        rb 1024         ; home directory in wich the user is locked, asciiz
        work_dir        rb 1024         ; working directory, must at all times begin and end with a '/', asciiz
        fpath           rb 1024*3       ; file path, combination of home_dir, work_dir and filename
                                        ; Will also be used to temporarily store username

        type            db ?    ; ASCII/EBDIC/IMAGE/..
        mode            db ?    ; active/passive
        socketnum       dd ?    ; Commands socket
        state           dd ?    ; disconnected/logging in/logged in/..
        passivesocknum  dd ?    ; when in passive mode, this is the listening socket
        datasocketnum   dd ?    ; socket used for data transfers
        permissions     dd ?    ; read/write/execute/....
        buffer_ptr      dd ?
        pid             dd ?    ; Process id of the current thread

        datasock        sockaddr_in

        buffer          rb BUFFERSIZE
ends

;------------------------------------------------
; parse_cmd
;
; Internal function wich uses the 'commands'
;  table to call an appropriate cmd_xx function.
;
; input: esi = ptr to ascii commands
;        ecx = number of bytes input
;        ebp = pointer to thread_data structure
;
; output: none
;
;------------------------------------------------
align 4
parse_cmd:                              ; esi must point to command

        cmp     byte [esi], 0x20        ; skip all leading characters
        ja      .ok
        inc     esi
        dec     ecx
        cmp     ecx, 3
        jb      .error
        jmp     parse_cmd
  .ok:
        cmp     byte [esi+3], 0x20
        ja      @f
        mov     byte [esi+3], 0
       @@:

        mov     eax, [esi]
        and     eax, not 0x20202020     ; convert to upper case
        mov     edi, commands           ; list of commands to scan
  .scanloop:
        cmp     eax, [edi]
        je      .got_it

        add     edi, 5*4
        cmp     byte [edi], 0
        jne     .scanloop

  .error:
        cmp     [ebp + thread_data.state], STATE_ACTIVE
        jb      login_first
        sendFTP "500 Unsupported command"
        ret

  .got_it:
        mov     eax, [ebp + thread_data.state]
        jmp     dword [edi + 4 + eax]


align 4
iglobal
commands:               ; all commands must be in uppercase

        dd 'ABOR', login_first, login_first, login_first, cmdABOR
;        dd 'ACCT', login_first, login_first, login_first, cmd_ACCT
;        dd 'APPE', login_first, login_first, login_first, cmd_APPE
        dd 'CDUP', login_first, login_first, login_first, cmdCDUP
        dd 'CWD',  login_first, login_first, login_first, cmdCWD
        dd 'DELE', login_first, login_first, login_first, cmdDELE
;        dd 'HELP', login_first, login_first, login_first, cmd_HELP
        dd 'LIST', login_first, login_first, login_first, cmdLIST
;        dd 'MDTM', login_first, login_first, login_first, cmd_MDTM
;        dd 'MKD',  login_first, login_first, login_first, cmd_MKD
;        dd 'MODE', login_first, login_first, login_first, cmd_MODE
        dd 'NLST', login_first, login_first, login_first, cmdNLST
        dd 'NOOP', login_first, login_first, login_first, cmdNOOP
        dd 'PASS', cmdPASS.0,   cmdPASS    , cmdPASS.2,   cmdPASS.3
        dd 'PASV', login_first, login_first, login_first, cmdPASV
        dd 'PORT', login_first, login_first, login_first, cmdPORT
        dd 'PWD',  login_first, login_first, login_first, cmdPWD
        dd 'QUIT', cmdQUIT,     cmdQUIT,     cmdQUIT,     cmdQUIT
;        dd 'REIN', login_first, login_first, login_first, cmd_REIN
;        dd 'REST', login_first, login_first, login_first, cmd_REST
        dd 'RETR', login_first, login_first, login_first, cmdRETR
;        dd 'RMD', login_first, login_first, login_first, cmd_RMD
;        dd 'RNFR', login_first, login_first, login_first, cmd_RNFR
;        dd 'RNTO', login_first, login_first, login_first, cmd_RNTO
;        dd 'SITE', login_first, login_first, login_first, cmd_SITE
;        dd 'SIZE', login_first, login_first, login_first, cmd_SIZE
;        dd 'STAT', login_first, login_first, login_first, cmd_STAT
        dd 'STOR', login_first, login_first, login_first, cmdSTOR
;        dd 'STOU', login_first, login_first, login_first, cmd_STOU
;        dd 'STRU', login_first, login_first, login_first, cmd_STRU
        dd 'SYST', login_first, login_first, login_first, cmdSYST
        dd 'TYPE', login_first, login_first, login_first, cmdTYPE
        dd 'USER', cmdUSER,     cmdUSER,     cmdUSER,     cmdUSER.2
        db 0    ; end marker
endg

align 4
login_first:
        sendFTP "530 Please login with USER and PASS"
        ret

align 4
permission_denied:
        sendFTP "550 Permission denied"
        ret

align 4
socketerror:
        invoke  con_set_flags, 0x0c
        invoke  con_write_asciiz, str_sockerr
        invoke  con_set_flags, 0x07

        sendFTP "425 Can't open data connection"
        ret

align 4
abort_transfer:
        and     [ebp + thread_data.permissions], not ABORT
        mov     [ebp + thread_data.mode], MODE_NOTREADY
        invoke  file.close, ebx
        mcall   close, [ebp + thread_data.datasocketnum]

        sendFTP "530 Transfer aborted"
        ret

align 4
ip_to_dword:    ; esi = ptr to str, cl = separator ('.', ',')

        call    ascii_to_byte
        mov     bl, al
        cmp     byte [esi], cl
        jne     .err
        inc     esi

        call    ascii_to_byte
        mov     bh, al
        cmp     byte [esi], cl
        jne     .err
        inc     esi

        shl     ebx, 16

        call    ascii_to_byte
        mov     bl, al
        cmp     byte [esi], cl
        jne     .err
        inc     esi

        call    ascii_to_byte
        mov     bh, al

        ror     ebx, 16
        ret

  .err:
        xor     ebx, ebx
        ret

align 4         ; esi = ptr to str, output in eax
ascii_to_byte:

        xor     eax, eax
        push    ebx

  .loop:
        movzx   ebx, byte[esi]
        sub     bl, '0'
        jb      .done
        cmp     bl, 9
        ja      .done
        lea     eax, [eax*4 + eax]      ;
        shl     eax, 1                  ; eax = eax * 10
        add     eax, ebx
        inc     esi

        jmp     .loop

  .done:
        pop     ebx
        ret

align 4
dword_to_ascii: ; edi = ptr where to write, eax is number

        push    edx ebx ecx
        mov     ebx, 10
        xor     ecx, ecx

       @@:
        xor     edx, edx
        div     ebx
        add     edx, '0'
        pushw   dx
        inc     ecx
        test    eax, eax
        jnz     @r

       @@:
        popw    ax
        stosb
        dec     ecx
        jnz     @r

        pop     ecx ebx edx
        ret

align 4
create_path:            ; combine home_dir and work_dir strings into fpath

        lea     edi, [ebp + thread_data.fpath]
        lea     esi, [ebp + thread_data.home_dir]
        mov     ecx, 1024
  .loop1:
        lodsb
        cmp     al, 0x20
        jb      .next
        stosb
        loop    .loop1
  .next:

        cmp     byte[edi-1], '/'
        jne     @f
        dec     edi
       @@:

        lea     esi, [ebp + thread_data.work_dir]
        mov     ecx, 1024
  .loop2:
        lodsb
        cmp     al, 0x20
        jb      .done
        stosb
        loop    .loop2

  .done:
        xor     al, al
        stosb
        ret


align 4
nextpasvport:

        inc     [pasv_port]

        mov     ax, [pasv_port]
        cmp     ax, [pasv_start]
        jb      .restart
        cmp     ax, [pasv_end]
        ja      .restart

        ret

  .restart:
        pushw   [pasv_start]
        popw    [pasv_port]

        ret


align 4
open_datasock:

        cmp     [ebp + thread_data.mode], MODE_PASSIVE_OK
        je      .start

; If we are in active mode, it's time to open a data socket..
        cmp     [ebp + thread_data.mode], MODE_ACTIVE
        jne     .not_active
        mov     ecx, [ebp + thread_data.datasocketnum]
        lea     edx, [ebp + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   connect
        cmp     eax, -1
        jne     .start

  .socketerror:
        add     esp, 4
        jmp     socketerror

; If we are still in passive_wait, it's time we accept an incomming call..
  .not_active:
        cmp     [ebp + thread_data.mode], MODE_PASSIVE_WAIT
        jne     .socketerror

  .try_now:
        mov     ecx, [ebp + thread_data.passivesocknum]
        lea     edx, [ebp + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   accept
        cmp     eax, -1
        jne     .pasv_ok
        mov     [ebp + thread_data.mode], MODE_PASSIVE_FAILED                   ; assume that we will fail
        mcall   23, 200
        mcall   accept
        cmp     eax, -1
        je      .socketerror
  .pasv_ok:
        mov     [ebp + thread_data.datasocketnum], eax
        mov     [ebp + thread_data.mode], MODE_PASSIVE_OK
        mcall   close   ; [ebp + thread_data.passivesocknum]
        mov     [ebp + thread_data.passivesocknum], -1
        invoke  con_write_asciiz, str_datasock

  .start:
        ret


;------------------------------------------------
; "ABOR"
;
; This command aborts the current filetransfer.
;
;------------------------------------------------
align 4
cmdABOR:

        or      [ebp + thread_data.permissions], ABORT
        sendFTP "250 Command succesul"
        ret

;------------------------------------------------
; "CDUP"
;
; Change the directory to move up one level.
;
;------------------------------------------------
align 4
cmdCDUP:

        test    [ebp + thread_data.permissions], PERMISSION_CD
        jz      permission_denied

        cmp     byte [ebp + thread_data.work_dir+1], 0  ; are we in "/" ?
        je      .done                                   ; if so, we cant go up..

; find the end of asciiz string work_dir
        mov     ecx, 1024
        xor     al, al
        lea     edi, [ebp + thread_data.work_dir]
        repne   scasb
; return 2 characters (right before last /)
        sub     edi, 3
; and now search backwards, for a '/'
        mov     al,'/'
        neg     ecx
        add     ecx, 1024
        std
        repne   scasb
        cld
; terminate the string here
        mov     byte[edi+2], 0

  .done:
; Print the new working dir on the console
        lea     eax, [ebp + thread_data.work_dir]
        invoke  con_write_asciiz, eax
        invoke  con_write_asciiz, str_newline

        sendFTP "250 Command succesul"
        ret

;------------------------------------------------
; "CWD"
;
; Change Working Directory.
;
;------------------------------------------------
align 4
cmdCWD:

        test    [ebp + thread_data.permissions], PERMISSION_CD
        jz      permission_denied

; do we have enough parameters?
        sub     ecx, 4
        jbe     .err

; get ready to copy the path
        add     esi, 4
        mov     ecx, 1024
        lea     edi, [ebp + thread_data.work_dir]

; if received dir starts with '/', we will simply copy it
; If not, we will append the current path with the received path.
        cmp     byte [esi], '/'
        je      .copyloop

; Find the end of work_dir string.
        xor     al, al
  .find_zero:
        repne   scasb
        dec     edi

; and now append work_dir with received string
        mov     ecx, 1024

; scan for end byte, or '.'
  .copyloop:
        lodsb
        cmp     al, 0x20
        jb      .done
;;;        cmp     al, '.'         ; '..' means we must go up one dir   TODO
;;;        je      .up
        stosb
        loop    .copyloop

; now, now make sure it ends with '/', 0
  .done:
        cmp     byte [edi-1], '/'
        je      @f
        mov     byte [edi], '/'
        inc     edi
       @@:
        mov     byte [edi], 0

; Print the new working dir on the console
        lea     eax, [ebp + thread_data.work_dir]
        invoke  con_write_asciiz, eax
        invoke  con_write_asciiz, str_newline

        sendFTP "250 Command succesful"
        ret

  .err:
        sendFTP "550 Directory does not exist"
        ret

;------------------------------------------------
; "DELE"
;
; Delete a file from the server.
;
;------------------------------------------------
align 4
cmdDELE:

        test    [ebp + thread_data.permissions], PERMISSION_DELETE
        jz      permission_denied

        ret

;------------------------------------------------
; "LIST"
;
; List the files in the current working directory.
;
;------------------------------------------------
align 4
cmdLIST:

        test    [ebp + thread_data.permissions], PERMISSION_EXEC
        jz      permission_denied

        call    open_datasock

; Create fpath from home_dir and work_dir
        call    create_path

        lea     ebx, [ebp + thread_data.fpath]
        invoke  con_write_asciiz, ebx
        invoke  con_write_asciiz, str_newline

; Start the search
        invoke  file.find.first, ebx, str_mask, FA_READONLY+FA_FOLDER+FA_NORMAL+FA_ARCHIVED
        test    eax, eax
        jz      .nosuchdir

        lea     edi, [ebp + thread_data.buffer]
  .parse_file:
        test    eax, eax        ; did we find a file?
        jz      .done
        mov     ebx, eax        ; yes, save the descripter in ebx

; first, convert the attributes
        test    [ebx + FileInfoA.Attributes], FA_FOLDER
        jnz     .folder

        test    [ebx + FileInfoA.Attributes], FA_READONLY
        jnz     .readonly

        mov     eax, '-rw-'
        stosd
        jmp     .attr

  .folder:
        mov     eax, 'drwx'
        stosd
        jmp     .attr

  .readonly:
        mov     eax, '-r--'
        stosd

  .attr:
        mov     eax, 'rw-r'
        stosd
        mov     ax, 'w-'
        stosw
        mov     al, ' '
        stosb

; now..
        mov     ax, '1 '
        stosw

; now write owner, everything is owned by FTP, woohoo!
        mov     eax, 'FTP '
        stosd
        stosd

; now the filesize in ascii
        mov     eax, [ebx + FileInfoA.FileSizeLow]
        call    dword_to_ascii
        mov     al, ' '
        stosb

; then date (month/day/year)
        movzx   eax, [ebx + FileInfoA.DateModify + FileDateTime.month]
        cmp     eax, 12
        ja      @f
        mov     eax, [months - 4 + 4*eax]
        stosd
       @@:

        movzx   eax, [ebx + FileInfoA.DateModify + FileDateTime.day]
        call    dword_to_ascii
        mov     al, ' '
        stosb

        movzx   eax, [ebx + FileInfoA.DateModify + FileDateTime.year]
        call    dword_to_ascii
        mov     al, ' '
        stosb

; and last but not least, filename
        lea     esi, [ebx + FileInfoA.FileName]
        mov     ecx, 264
  .nameloop:
        lodsb
        test    al, al
        jz      .namedone
        stosb
        loop    .nameloop

; insert a cr lf
  .namedone:
        mov     ax, 0x0a0d
        stosw

        test    [ebp + thread_data.permissions], ABORT          ; Did we receive ABOR command from client?
;;;        jnz     .abort                                         ; TODO

; check next file
        invoke  file.find.next, ebx
        jmp     .parse_file

; close file desc
  .done:
        invoke  file.find.close, ebx                            ; ebx is descriptor of last file, eax will be -1 !!

; append the string with a 0
        xor     al, al
        stosb

; Warn the client we're about to send the data
        push    edi
        sendFTP "150 Here it comes.."
        pop     esi

; and send it to the client
        mov     ecx, [ebp + thread_data.datasocketnum]          ; socket num
        lea     edx, [ebp + thread_data.buffer]                 ; buffer ptr
        sub     esi, edx                                        ; length
        xor     edi, edi                                        ; flags
        mcall   send

; close the data socket..
        mov     [ebp + thread_data.mode], MODE_NOTREADY
        mcall   close, [ebp + thread_data.datasocketnum]

        sendFTP "226 Transfer OK"
        ret

  .nosuchdir:
        sendFTP "550 Directory does not exist"
        ret

;------------------------------------------------
; "NLST"
;
; List the filenames of the files in the current working directory.
;
;------------------------------------------------
align 4
cmdNLST:

        test    [ebp + thread_data.permissions], PERMISSION_EXEC
        jz      permission_denied

        ; TODO: same as list but simpler output format

        ret

;------------------------------------------------
; "NOOP"
;
; No operation, just keep the connection alive.
;
;------------------------------------------------
align 4
cmdNOOP:

        sendFTP "200 Command OK"
        ret

;------------------------------------------------
; "PASS"
;
; Second phase of login process, client provides password.
;
;------------------------------------------------
align 4
cmdPASS:

; read the password from users.ini
        lea     edi, [ebp + thread_data.buffer + 512]           ; temp pass
        lea     ebx, [ebp + thread_data.fpath]                  ; temp username
        invoke  ini.get_str, path2, ebx, str_pass, edi, 512, str_infinity
        test    eax, eax                                        ; unable to read password? fail!
        jnz     .incorrect
        cmp     dword [edi], -1                                 ; no key, section or file found.. fail!
        je      .incorrect
;;;        cmp     byte [edi], 0                                   ; zero password? ok!
;;;        je      .ok

        add     esi, 5
        sub     ecx, 5
        jbe     .incorrect                                      ; no password given? but hey, we need one! fail..

; compare with received password
        repe    cmpsb
        cmp     byte [esi-1], 0x20                              ; printeable characters left?
        jae     .incorrect
        cmp     byte [edi-1], 0
        jne     .incorrect

  .ok:
        invoke  ini.get_int, path2, ebx, str_mode, 0
        mov     [ebp + thread_data.permissions], eax

        invoke  con_write_asciiz, str_pass_ok
        mov     [ebp + thread_data.state], STATE_ACTIVE
        sendFTP "230 You are now logged in"
        ret

  .2:
  .incorrect:
        invoke  con_write_asciiz, str_pass_err
        mov     [ebp + thread_data.state], STATE_CONNECTED      ; reset state
        sendFTP "530 Login incorrect"
        ret

  .0:
        sendFTP "503 Login with USER first"
        ret

  .3:
        sendFTP "230 Already logged in"
        ret

;------------------------------------------------
; "PASV"
;
; Initiate a passive dataconnection.
;
;------------------------------------------------
align 4
cmdPASV:

;        cmp     [ebp + thread_data.passivesocknum], -1
;        je      @f
;        mcall   close, [ebp + thread_data.passivesocknum]       ; if there is still a socket open, close it
;       @@:

; Open a new TCP socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      socketerror
        mov     [ebp + thread_data.passivesocknum], eax

; Bind it to a known local port
        mov     [ebp + thread_data.datasock.sin_family], AF_INET4
        mov     [ebp + thread_data.datasock.sin_addr], 0

        mov     ecx, eax                                        ; passivesocketnum
        lea     edx, [ebp + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock

  .next_port:                                                   ; TODO: break the endless loop
        call    nextpasvport
        pushw   [pasv_port]
        popw    [ebp + thread_data.datasock.sin_port]

        mcall   bind
        cmp     eax, -1
        je      .next_port

; And set it to listen!
        mcall   listen, , 1
        cmp     eax, -1
        je      socketerror

; Tell our thread we are ready to accept incoming calls
        mov     [ebp + thread_data.mode], MODE_PASSIVE_WAIT

; Now tell the client where to connect to in this format:
; 227 Entering Passive Mode (a1,a2,a3,a4,p1,p2)
; where a1.a2.a3.a4 is the IP address and p1*256+p2 is the port number.

; '227 ('
        lea     edi, [ebp + thread_data.buffer]
        mov     eax, '227 '
        stosd
        mov     al, '('
        stosb
; ip
        movzx   eax, byte [serverip]
        call    dword_to_ascii
        mov     al, ','
        stosb
        movzx   eax, byte [serverip+1]
        call    dword_to_ascii
        mov     al, ','
        stosb
        movzx   eax, byte [serverip+2]
        call    dword_to_ascii
        mov     al, ','
        stosb
        movzx   eax, byte [serverip+3]
        call    dword_to_ascii
        mov     al, ','
        stosb
; port
        movzx   eax, byte [ebp + thread_data.datasock.sin_port + 1]
        call    dword_to_ascii
        mov     al, ','
        stosb
        movzx   eax, byte [ebp + thread_data.datasock.sin_port]
        call    dword_to_ascii
; ')', 13, 10, 0
        mov     eax, ')' + 0x000a0d00
        stosd

        lea     esi, [edi - thread_data.buffer]
        sub     esi, ebp
        mov     ecx, [ebp + thread_data.socketnum]
        lea     edx, [ebp + thread_data.buffer]
        xor     edi, edi
        mcall   send

        ret

;------------------------------------------------
; "PWD"
;
; Print the current working directory.
;
;------------------------------------------------
align 4
cmdPWD:

        mov     dword [ebp + thread_data.buffer], '257 '
        mov     byte [ebp + thread_data.buffer+4], '"'

        lea     edi, [ebp + thread_data.buffer+5]
        lea     esi, [ebp + thread_data.work_dir]
        mov     ecx, 1024
  .loop:
        lodsb
        or      al, al
        jz      .ok
        stosb
        dec     ecx
        jnz     .loop

  .ok:
        mov     dword [edi], '"' + 0x000a0d00    ; '"',13,10,0
        lea     esi, [edi - thread_data.buffer + 4]
        sub     esi, ebp
        mov     ecx, [ebp + thread_data.socketnum]
        lea     edx, [ebp + thread_data.buffer]
        xor     edi, edi
        mcall   send

; Print the new working dir on the console
        lea     eax, [ebp + thread_data.work_dir]
        invoke  con_write_asciiz, eax
        invoke  con_write_asciiz, str_newline

        ret

;------------------------------------------------
; "PORT"
;
; Initiate an active dataconnection.
;
;------------------------------------------------
align 4
cmdPORT:

; PORT a1,a2,a3,a4,p1,p2
; IP address a1.a2.a3.a4, port p1*256+p2

; Convert the IP
        lea     esi, [esi+5]
        mov     cl, ','
        call    ip_to_dword
; And put it in datasock
        mov     [ebp + thread_data.datasock.sin_addr], ebx

; Now the same with portnumber
        inc     esi
        call    ascii_to_byte
        mov     bh, al
        inc     esi
        call    ascii_to_byte
        mov     bl, al

; Save it in datasock too
        mov     [ebp + thread_data.datasock.sin_port], bx

; We will open the socket, but do not connect yet!
        mov     [ebp + thread_data.datasock.sin_family], AF_INET4
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      socketerror

        mov     [ebp + thread_data.datasocketnum], eax
        mov     [ebp + thread_data.mode], MODE_ACTIVE

        sendFTP "225 Data connection open"
        ret

;------------------------------------------------
; "QUIT"
;
; Close the connection with client.
;
;------------------------------------------------
align 4
cmdQUIT:

        sendFTP "221 Bye!"
        mcall   close, [ebp + thread_data.datasocketnum]
        mcall   close, [ebp + thread_data.socketnum]

        add     esp, 4          ; get rid of call return address
        jmp     thread_exit     ; now close this thread


;------------------------------------------------
; "RETR"
;
; Retrieve a file from the ftp server.
;
;------------------------------------------------
align 4
cmdRETR:

        test    [ebp + thread_data.permissions], PERMISSION_READ
        jz      permission_denied

        cmp     ecx, 1024 + 5
        jae     .cannot_open

        sub     ecx, 5
        jb      .cannot_open

        call    open_datasock

        call    create_path
        dec     edi
        lea     esi, [ebp + thread_data.buffer + 5]
        mov     ecx, 1024
  .loop:
        lodsb
        cmp     al, 0x20
        jl      .done
        stosb
        loop    .loop
  .done:
        xor     al, al
        stosb

        lea     ebx, [ebp + thread_data.fpath]
        invoke  con_write_asciiz, ebx
        invoke  con_write_asciiz, str_newline

        invoke  file.open, ebx, O_READ
        test    eax, eax
        jz      .cannot_open

        push    eax
        sendFTP "150 Here it comes.."
        pop     ebx

  .read_more:
        test    [ebp + thread_data.permissions], ABORT
        jnz     abort_transfer

        lea     eax, [ebp + thread_data.buffer]                 ; FIXME: use another buffer!! if we receive something on control connection now, we screw up!
        invoke  file.read, ebx, eax, BUFFERSIZE
        cmp     eax, -1
        je      .cannot_open                                    ; FIXME: this is not the correct error

        invoke  con_write_asciiz, str2

        push    eax ebx
        mov     esi, eax
        mov     ecx, [ebp + thread_data.datasocketnum]
        lea     edx, [ebp + thread_data.buffer]
        xor     edi, edi
        mcall   send
        pop     ebx ecx
        cmp     eax, -1
        je      socketerror                                     ; FIXME: not the correct error

;        cmp     eax, ecx
;        jne     not_all_byes_sent                               ; TODO

        cmp     ecx, BUFFERSIZE
        je      .read_more

        invoke  file.close, ebx

        invoke  con_write_asciiz, str2b

        mov     [ebp + thread_data.mode], MODE_NOTREADY
        mcall   close, [ebp + thread_data.datasocketnum]

        sendFTP "226 Transfer OK, closing connection"
        ret

  .cannot_open:
        invoke  con_set_flags, 0x0c
        invoke  con_write_asciiz, str_notfound
        invoke  con_set_flags, 0x07

        sendFTP "550 No such file"
        ret



;------------------------------------------------
; "STOR"
;
; Store a file on the server.
;
;------------------------------------------------
align 4
cmdSTOR:

        test    [ebp + thread_data.permissions], PERMISSION_WRITE
        jz      permission_denied


;;;;
        test    [ebp + thread_data.permissions], ABORT
        jnz     abort_transfer

;;;;

        ret

;------------------------------------------------
; "SYST"
;
; Send information about the system.
;
;------------------------------------------------
align 4
cmdSYST:

        sendFTP "215 UNIX type: L8"
        ret

;------------------------------------------------
; "TYPE"
;
; Choose the file transfer type.
;
;------------------------------------------------
align 4
cmdTYPE:

        cmp     ecx, 6
        jb      parse_cmd.error

        mov     al, byte[esi+5]
        and     al, not 0x20

        cmp     al, 'A'
        je      .ascii
        cmp     al, 'E'
        je      .ebdic
        cmp     al, 'I'
        je      .image
        cmp     al, 'L'
        je      .local

        jmp     parse_cmd.error

  .ascii:
        mov     [ebp + thread_data.type], TYPE_ASCII
        jmp     .subtype

  .ebdic:
        mov     [ebp + thread_data.type], TYPE_EBDIC

  .subtype:
        cmp     ecx, 8
        jb      .non_print

        mov     al, byte[esi+7]
        and     al, not 0x20

        cmp     al, 'N'
        je      .non_print
        cmp     al, 'T'
        je      .telnet
        cmp     al, 'C'
        je      .asacc
        cmp     al, 0x20
        jb      .non_print

        jmp     parse_cmd.error

  .non_print:
        or      [ebp + thread_data.type], TYPE_NP
        jmp     .ok

  .telnet:
        or      [ebp + thread_data.type], TYPE_TELNET
        jmp     .ok

  .asacc:
        or      [ebp + thread_data.type], TYPE_ASA
        jmp     .ok

  .image:
        mov     [ebp + thread_data.type], TYPE_IMAGE
        jmp     .ok

  .local:
        cmp     ecx, 8
        jb      parse_cmd.error

        mov     al, byte[esi+7]
        sub     al, '0'
        jb      parse_cmd.error                         ; FIXME: this is not the correct errormessage
        cmp     al, 9
        ja      parse_cmd.error                         ; FIXME
        or      al, TYPE_LOCAL
        mov     [ebp + thread_data.type], al

  .ok:
        sendFTP "200 Command ok"
        ret

;------------------------------------------------
; "USER"
;
; Login to the server, step one of two.                         ;;; TODO: prevent buffer overflow!
;
;------------------------------------------------
align 4
cmdUSER:

        lea     esi, [esi + 5]
        lea     edi, [ebp + thread_data.fpath]                  ; temp buffer for username
  .loop:
        lodsb
        stosb
        cmp     al, 0x20
        jae     .loop
        mov     byte [edi-1], 0

        lea     esi, [ebp + thread_data.fpath]
        lea     eax, [ebp + thread_data.home_dir]
        invoke  ini.get_str, path2, esi, str_home, eax, 1024, str_infinity
        cmp     eax, -1
        je      .login_fail
        cmp     dword [esi], -1
        je      .login_fail

        mov     word [ebp + thread_data.work_dir], "/"          ; "/", 0

        invoke  con_write_asciiz, str_logged_in
        mov     [ebp + thread_data.state], STATE_LOGIN
  .sendstr:
        sendFTP "331 Please specify the password"
        ret

  .login_fail:
        invoke  con_write_asciiz, str_pass_err
        mov     [ebp + thread_data.state], STATE_LOGIN_FAIL
        jmp     .sendstr

align 4
  .2:
        sendFTP "530 Can't change to another user"
        ret
