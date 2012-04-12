

struct  thread_data
                        rb 1024
        stack           rb 0

        home_dir        rb 1024
        work_dir        rb 1024
        fpath           rb 1024*3       ; Will also be used to temporarily store username

        type            db ?    ; ASCII/EBDIC/IMAGE/..
        mode            db ?    ; active/passive
        socketnum       dd ?    ; Commands socket
        state           dd ?    ; disconnected/logging in/logged in/..
        passivesocknum  dd ?    ; when in passive mode, this is the listening socket
        datasocketnum   dd ?    ; socket used for data transfers
        permissions     dd ?
        buffer_ptr      dd ?

        datasock        sockaddr_in

        buffer          rb BUFFERSIZE
ends


macro sendFTP str {
local .string, .length, .label
        xor     edi, edi
        mcall   send, [ebp + thread_data.socketnum], .string, .length
        jmp     @f
.string db str, 13, 10
.length = $ - .string
@@:

}

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

        cmp     byte [ebp + thread_data.work_dir+1], 0                          ; are we in "/" ?
        je      .done

        mov     ecx, 1024
        xor     al, al
        lea     edi, [ebp + thread_data.work_dir]
        repne   scasb
        sub     edi, 3
        mov     al,'/'
        std
        neg     ecx
        add     ecx, 1024
        repne   scasb
        cld
        mov     byte[edi+1], 0

  .done:
; Print the new working dir on the console
        lea     eax, [ebp + thread_data.work_dir]
        push    eax
        call    [con_write_asciiz]
        push    str_newline
        call    [con_write_asciiz]

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

        sub     ecx, 4
        jb      .err
        add     esi, 4

        mov     ecx, 1024
        lea     edi, [ebp + thread_data.work_dir]
        cmp     byte [esi], '/'
        je      .loop

  .scan:
        lea     edi, [ebp + thread_data.work_dir + 1]
        push    ecx
        mov     ecx, 1024
  .find_zero:
        cmp     byte [edi], 0
        je      .found_zero
        inc     edi
        loop    .find_zero
  .found_zero:
        pop     ecx
  .scan2:

        cmp     byte [esi], '/'
        jne     @f
        inc     esi
        dec     ecx
        jz      .done
       @@:

  .loop:
        lodsb
        cmp     al, 0x20
        jb      .done
        cmp     al, '.'
        je      .up
  .continue:
        stosb
        loop    .loop
  .done:
        cmp     byte [edi-1], '/'
        je      @f
        mov     byte [edi], '/'
        inc     edi
       @@:
        mov     byte [edi], 0

; Print the new working dir on the console
        lea     eax, [ebp + thread_data.work_dir]
        push    eax
        call    [con_write_asciiz]
        push    str_newline
        call    [con_write_asciiz]

        sendFTP "250 Command succesful"
        ret

  .up:
        lodsb
        cmp     al, '.'
        jne     .continue

;;;;        TODO: find second last '\' in work_dir and make next char zero
;;;;        point edi to that 0

        jmp     .scan2

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

; If we are in active mode, it's time to open a data socket..
        cmp     [ebp + thread_data.mode], MODE_ACTIVE
        jne     @f
        mov     ecx, [ebp + thread_data.datasocketnum]
        lea     edx, [ebp + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   connect
        cmp     eax, -1
        je      socketerror
  @@:

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
;;;        mov     eax, ebx        ;;;;;

; close file desc
  .done:
        invoke  file.find.close, ebx                            ; ebx is the

; append the string with a 0
        xor     al, al
        stosb

; Warn the client we're about to send the data
        push    edi
        sendFTP "150 Here it comes.."
        pop     esi

; and send it to the client
        mov     ecx, [ebp + thread_data.datasocketnum]
        lea     edx, [ebp + thread_data.buffer]
        sub     esi, edx
        xor     edi, edi
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
        lea     esi, [esi + 5]

; read the password from users.ini
        lea     edi, [ebp + thread_data.buffer + 512]           ; temp pass
        lea     ebx, [ebp + thread_data.fpath]                  ; temp username
        invoke  ini.get_str, path2, ebx, str_pass, edi, 512, str_infinity
        test    eax, eax
        jnz     .incorrect
        cmp     dword [edi], -1
        je      .incorrect
        cmp     byte[edi], 0
        je      .pass_ok

; compare with received password
        repe    cmpsb
        cmp     byte [esi], 0x20
        jae     .incorrect
        cmp     byte [edi], 0
        jne     .incorrect

  .pass_ok:
        invoke  ini.get_int, path2, ebx, str_mode, 0
        mov     [ebp + thread_data.permissions], eax

        invoke  con_write_asciiz, str_pass_ok
        mov     [ebp + thread_data.state], STATE_ACTIVE
        sendFTP "230 You are now logged in"
        ret

  .2:
  .incorrect:
        mov     [ebp + thread_data.state], STATE_CONNECTED
        sendFTP "530 Login incorrect"
        ret

align 4
  .0:
        sendFTP "503 Login with USER first"
        ret

align 4
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

; Open a new TCP socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      socketerror
        mov     [ebp + thread_data.passivesocknum], eax

; Bind it to a known local port
        mov     [ebp + thread_data.datasock.sin_family], AF_INET4
        pushw   [pasvport]
        popw    [ebp + thread_data.datasock.sin_port]
        inc     [pasvport]
        mov     [ebp + thread_data.datasock.sin_addr], 0

        mov     ecx, eax                                        ; passivesocketnum
        lea     edx, [ebp + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   bind
        cmp     eax, -1
;        je      bind_err       ; TODO

; And set it to listen!
        mcall   listen, , 1
        cmp     eax, -1
;        je      listen_err     ; TODO

; Tell our thread we are ready to accept incoming calls
        mov     [ebp + thread_data.mode], MODE_PASSIVE_WAIT

; Now tell the client where to connect to in this format:
; 227 Entering Passive Mode (a1,a2,a3,a4,p1,p2)
; where a1.a2.a3.a4 is the IP address and p1*256+p2 is the port number.

; '227 ('
        lea     edi, [ebp + thread_data.buffer]
        mov     eax, '227 '     ; FIXME (now hardcoded to 127.0.0.1:2000)
        stosd
        mov     al, '('
        stosb
; ip
        mov     eax, 127
        call    dword_to_ascii
        mov     al, ','
        stosb
        mov     eax, 0
        call    dword_to_ascii
        mov     al, ','
        stosb
        mov     eax, 0
        call    dword_to_ascii
        mov     al, ','
        stosb
        mov     eax, 1
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

        cmp     [ebp + thread_data.mode], MODE_ACTIVE
        jne     @f
        push    ecx esi
        mov     ecx, [ebp + thread_data.datasocketnum]
        lea     edx, [ebp + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   connect
        pop     esi ecx
        cmp     eax, -1
        je      socketerror
  @@:

        push    ecx esi
        call    create_path
        pop     esi ecx
        dec     edi
        add     esi, 5

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

        mov     [ebp + thread_data.mode], MODE_NOTREADY
        mcall   close, [ebp + thread_data.datasocketnum]

        sendFTP "226 Transfer OK, closing connection"
        ret

  .cannot_open:
        invoke  con_set_flags, 0x0c
        invoke  con_write_asciiz, str_notfound
        invoke  con_set_flags, 0x07

        mov     edx, [ebp]
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
        invoke  con_write_asciiz, str_login_invalid
        mov     [ebp + thread_data.state], STATE_LOGIN_FAIL
        jmp     .sendstr

align 4
  .2:
        sendFTP "530 Can't change to another user"
        ret
