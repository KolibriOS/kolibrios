

struct  thread_data
                        rb 1024
        stack           rb 0

        home_dir        rb 1024
        work_dir        rb 1024
        fpath           rb 1024*3

        type            db ?    ; ASCII/EBDIC/IMAGE/..
        mode            db ?    ; active/passive
        socketnum       dd ?    ; Commands socket
        state           dd ?    ; disconnected/logging in/logged in/..
        passivesocknum  dd ?    ; when in passive mode, this is the listening socket
        datasocketnum   dd ?    ; socket used for data transfers

        datasock        sockaddr_in

        buffer          rb BUFFERSIZE
ends




align 4
parse_cmd:                              ; esi must point to command

        cmp     byte [esi], 0x20        ; skip all leading characters
        ja      .ok
        inc     esi
        dec     ecx
        cmp     ecx, 3
        ja      parse_cmd
        ret
  .ok:

        cmp     byte [esi+3], 0x20
        jae     @f
        mov     byte [esi+3], 0
       @@:

        mov     eax, [esi]
        and     eax, not 0x20202020     ; convert to upper case
        mov     edi, commands           ; list of commands to scan
  .scanloop:
        cmp     eax, [edi]
        jne     .try_next

        jmp     dword [edi+4]

  .try_next:
        add     edi, 8
        cmp     byte [edi], 0
        jne     .scanloop

  .error:
        mcall   send, [edx + thread_data.socketnum], str500, str500.length, 0

        ret


align 4
commands:               ; all commands must be in uppercase

        db 'ABOR'
        dd cmdABOR
        db 'CDUP'
        dd cmdCDUP
        db 'CWD', 0
        dd cmdCWD
        db 'DELE'
        dd cmdDELE
        db 'LIST'
        dd cmdLIST
        db 'NLST'
        dd cmdNLST
        db 'NOOP'
        dd cmdNOOP
        db 'PASS'
        dd cmdPASS
        db 'PASV'
        dd cmdPASV
        db 'PWD', 0
        dd cmdPWD
        db 'PORT'
        dd cmdPORT
        db 'QUIT'
        dd cmdQUIT
        db 'RETR'
        dd cmdRETR
        db 'STOR'
        dd cmdSTOR
        db 'SYST'
        dd cmdSYST
        db 'TYPE'
        dd cmdTYPE
        db 'USER'
        dd cmdUSER
        db 'XPWD'
        dd cmdPWD
        db 0            ; end marker


align 4
cmdABOR:

        ; TODO: abort the current filetransfer

        ret

align 4
cmdCDUP:

        cmp     byte [edx + thread_data.work_dir+1], 0
        je      .done

        mov     ecx, 1024
        xor     al, al
        lea     edi, [edx + thread_data.work_dir+1024]
        repne   scasb
        std
        dec     edi
        mov     al,'/'
        scasb
        cld
        mov     byte[edi], 0

  .done:
        mcall   send, [edx + thread_data.socketnum], str250, str250.length, 0    ; command successful
        ret

align 4
cmdCWD:                 ; Change Working Directory

        sub     ecx, 4
        jb      .err
        add     esi, 4

  .scan:
        lea     edi, [edx + thread_data.work_dir + 1]
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

        mcall   send, [edx + thread_data.socketnum], str250, str250.length, 0

        ret

  .up:
        lodsb
        cmp     al, '.'
        jne     .continue

;;;;        TODO: find second last '\' in work_dir and make next char zero
;;;;        point edi to that 0

        jmp     .scan2

  .err:
        ; TODO: print correct error message (550?)

        ret

align 4
cmdDELE:

        ret

align 4
cmdLIST:

; If we are in active mode, it's time to open a data socket..
        cmp     [edx + thread_data.mode], MODE_ACTIVE
        jne     @f
        mov     ecx, [edx + thread_data.datasocketnum]
        lea     edx, [edx + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   connect
        mov     edx, [esp+4]                                    ; thread_data pointer
        cmp     eax, -1
        je      socketerror
  @@:

; Create fpath from home_dir and work_dir
        call    create_path

        lea     eax, [edx + thread_data.fpath]
        push    eax
        call    [con_write_asciiz]
        push    str_newline
        call    [con_write_asciiz]

; Start the search
        push    FA_ANY
        push    str_mask
        lea     eax, [edx + thread_data.fpath]
        push    eax
        call    [file.find.first]

        test    eax, eax
        jz      .nosuchdir

        lea     edi, [edx + thread_data.buffer]
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
        mov     eax, [months + 4*eax]
        stosd

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

; check next file
        push    ebx
        call    [file.find.next]
        jmp     .parse_file

; close file desc
  .done:
        push    ebx
        call    [file.find.close]

; append the string with a 0
        xor     al, al
        stosb

; Warn the client we're about to send the data
        push    edi edx
        mcall   send, [edx + thread_data.socketnum], str150, str150.length, 0    ; here it comes..
        pop     edx esi

; and send it to the client
        mov     ecx, [edx + thread_data.datasocketnum]
        lea     edx, [edx + thread_data.buffer]
        sub     esi, edx
        xor     edi, edi
        mcall   send

; close the data socket..
        mov     edx, [esp+4]                                    ; thread_data pointer
        mcall   close, [edx + thread_data.datasocketnum]

        cmp     [edx + thread_data.mode], MODE_PASSIVE_OK
        jne     @f
        mov     [edx + thread_data.mode], MODE_NOTREADY
      @@:

; And send "transfer ok" on the base connection
        mcall   send, [edx + thread_data.socketnum], str226, str226.length, 0

        ret

  .nosuchdir:
        mcall   send, [edx + thread_data.socketnum], str550, str550.length, 0

        ret


align 4
cmdNLST:

        ; TODO: same as list but simpler output format

        ret

align 4
cmdNOOP:

        ret

align 4
cmdPASS:

        ; TODO: verify password

        mcall   send, [edx + thread_data.socketnum], str230, str230.length, 0

        push    str_pass_ok
        call    [con_write_asciiz]

        mov     edx, [esp+4]                                    ; thread_data pointer
        mov     [edx + thread_data.state], STATE_ACTIVE

        ret

align 4
cmdPASV:

; Open a new TCP socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        mov     edx, [esp+4]                                    ; thread_data pointer
        cmp     eax, -1
        je      socketerror
        mov     [edx + thread_data.passivesocknum], eax

; Bind it to a known local port
        mov     [edx + thread_data.datasock.sin_family], AF_INET4
        mov     [edx + thread_data.datasock.sin_port], 2000
        mov     [edx + thread_data.datasock.sin_addr], 0

        mov     ecx, eax ;[edx + thread_data.passivesocknum]
        lea     edx, [edx + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   bind
        mov     edx, [esp+4]                                    ; thread_data pointer
        cmp     eax, -1
        je      bind_err

; And set it to listen!
        mcall   listen, [edx + thread_data.passivesocknum], 10    ;;;;; FIXME

; Tell our thread we are ready to accept incoming calls
        mov     edx, [esp+4]                                    ; thread_data pointer
        mov     [edx + thread_data.mode], MODE_PASSIVE_WAIT

; Now tell the client where to connect to in this format:
; 227 Entering Passive Mode (a1,a2,a3,a4,p1,p2)
; where a1.a2.a3.a4 is the IP address and p1*256+p2 is the port number.
        lea     edi, [edx + thread_data.buffer]
        mov     eax, '227 '     ; FIXME (now hardcoded to 127.0.0.1:2000)
        stosd
        mov     eax, '(127'
        stosd
        mov     eax, ',0,0'
        stosd
        mov     eax, ',1,7'
        stosd
        mov     eax, ',208'
        stosd
        mov     al, ')'
        stosb
        mov     ax, 0x0a0d
        stosw
        xor     al, al
        stosb

        lea     esi, [edi - thread_data.buffer]
        sub     esi, edx
        mov     ecx, [edx + thread_data.socketnum]
        lea     edx, [edx + thread_data.buffer]
        xor     esi, esi
        mcall   send

        ret

align 4
cmdPWD:         ; Print Working Directory

        mov     dword [edx + thread_data.buffer], '257 '
        mov     byte [edx + thread_data.buffer+4], '"'

        lea     edi, [edx + thread_data.buffer+5]
        lea     esi, [edx + thread_data.work_dir]
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
        sub     esi, edx
        mov     ecx, [edx + thread_data.socketnum]
        lea     edx, [edx + thread_data.buffer]
        mcall   send, , , , 0

;        push    work_dir
;        push    str_pwd
;        call    [con_printf]

        ret

align 4
cmdPORT:

; PORT a1,a2,a3,a4,p1,p2
; IP address a1.a2.a3.a4, port p1*256+p2

        mov     [edx + thread_data.mode], MODE_ACTIVE

        lea     esi, [esi+5]
; Convert the IP
        call    ascii_to_byte
        mov     bl, al
        inc     esi             ; skip past ','
        call    ascii_to_byte
        mov     bh, al
        shl     ebx, 16
        inc     esi
        call    ascii_to_byte
        mov     bl, al
        inc     esi
        call    ascii_to_byte
        mov     bh, al
        inc     esi
        rol     ebx, 16

; And put it in datasock
        mov     [edx + thread_data.datasock.sin_addr], ebx

; Now the same with portnumber
        call    ascii_to_byte
        mov     bh, al
        inc     esi
        call    ascii_to_byte
        mov     bl, al

; Save it in datasock too
        mov     [edx + thread_data.datasock.sin_port], bx

; We will open the socket, but do not connect yet!
        mov     [edx + thread_data.datasock.sin_family], AF_INET4
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        mov     edx, [esp+4]                                    ; thread_data pointer
        cmp     eax, -1
        je      socketerror
        mov     [edx + thread_data.datasocketnum], eax

; Tell the client we are ready
        mov     edx, [esp+4]                                    ; thread_data pointer
        mcall   send, [edx + thread_data.socketnum], str225, str225.length, 0
        ret


align 4
cmdQUIT:

        mcall   close, [edx + thread_data.datasocketnum]
        mcall   send, [edx + thread_data.socketnum], str221, str221.length, 0    ; 221 - bye!
        mcall   close;, [edx + thread_data.socketnum]

        add     esp, 4          ; get rid of call return address
        jmp     thread_exit     ; now close this thread

align 4
cmdRETR:

        sub     ecx, 5
        jb      .cannot_open

        cmp     [edx + thread_data.mode], MODE_ACTIVE
        jne     @f
        push    esi
        mov     ecx, [edx + thread_data.datasocketnum]
        lea     edx, [edx + thread_data.datasock]
        mov     esi, sizeof.thread_data.datasock
        mcall   connect
        pop     esi
        mov     edx, [esp+4]                                    ; thread_data pointer
        cmp     eax, -1
        je      socketerror
  @@:

        push    esi
        call    create_path
        pop     esi
        dec     edi
        add     esi, 5
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

        lea     eax, [edx + thread_data.fpath]
        push    eax
        call    [con_write_asciiz]
        push    str_newline
        call    [con_write_asciiz]

        push    O_READ
        lea     eax, [edx + thread_data.fpath]
        push    eax
        call    [file.open]
        test    eax, eax
        jz      .cannot_open

        push    eax
        mcall   send, [edx + thread_data.socketnum], str150, str150.length, 0    ; here it comes..
        pop     ebx

        mov     edx, [esp+4]                                    ; thread_data pointer
  .read_more:
        push    BUFFERSIZE
        lea     eax, [edx + thread_data.buffer]
        push    eax
        push    ebx
        call    [file.read]
        cmp     eax, -1
        je      .cannot_open    ; fixme: this is not the correct error

        push    eax
        push    ebx
        mov     esi, eax
        mov     ecx, [edx + thread_data.datasocketnum]
        lea     edx, [edx + thread_data.buffer]
        xor     esi, esi
        mcall   send
        pop     ebx
        pop     ecx
        mov     edx, [esp+4]                                    ; thread_data pointer
        cmp     eax, -1
        je      socketerror

        cmp     ecx, BUFFERSIZE
        je      .read_more

        mcall   close, [edx + thread_data.datasocketnum]

        cmp     [edx + thread_data.mode], MODE_PASSIVE_OK
        jne     @f
        mov     [edx + thread_data.mode], MODE_PASSIVE_WAIT
      @@:

        mcall   send, [edx + thread_data.socketnum], str226, str226.length, 0    ; transfer ok

        ret

  .cannot_open:
        pushd   0x0c
        call    [con_set_flags]
        push    str_notfound
        call    [con_write_asciiz]
        pushd   0x07
        call    [con_set_flags]

        mcall   send, [edx + thread_data.socketnum], str550, str550.length, 0    ; file not found

        ret

align 4
cmdSTOR:

        ; TODO: check if user has write permission, and write file if so

        ret

align 4
cmdSYST:

        mcall   send, [edx + thread_data.socketnum], str215, str215.length, 0

        ret

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
        mov     [edx + thread_data.type], TYPE_ASCII
        jmp     .subtype

  .ebdic:
        mov     [edx + thread_data.type], TYPE_EBDIC

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
        or      [edx + thread_data.type], TYPE_NP
        jmp     .ok

  .telnet:
        or      [edx + thread_data.type], TYPE_TELNET
        jmp     .ok

  .asacc:
        or      [edx + thread_data.type], TYPE_ASA
        jmp     .ok

  .image:
        mov     [edx + thread_data.type], TYPE_IMAGE
        jmp     .ok

  .local:
        cmp     ecx, 8
        jb      parse_cmd.error

        mov     al, byte[esi+7]
        sub     al, '0'
        jb      parse_cmd.error
        cmp     al, 9
        ja      parse_cmd.error
        or      al, TYPE_LOCAL
        mov     [edx + thread_data.type], al

  .ok:
        mcall   send, [edx + thread_data.socketnum], str200, str200.length, 0

        ret

align 4
cmdUSER:

        ; TODO: check user and set home directory (and permissions)

        mcall   send, [edx + thread_data.socketnum], str331, str331.length, 0
        mov     edx, [esp+4]                                    ; thread_data pointer
        mov     [edx + thread_data.state], STATE_LOGIN

        mov     byte [edx + thread_data.work_dir], "/"
        mov     byte [edx + thread_data.work_dir+1], 0

        push    str_logged_in
        call    [con_write_asciiz]

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

        mov     eax, '1'
        stosb

        ret

align 4
create_path:            ; combine home_dir and work_dir strings into fpath

        lea     edi, [edx + thread_data.fpath]
        lea     esi, [edx + thread_data.home_dir]
        mov     ecx, 1024
  .loop1:
        lodsb
        or      al, al
        jz      .next
        stosb
        loop    .loop1
  .next:

        cmp     byte[edi-1], '/'
        jne     @f
        dec     edi
       @@:

        lea     esi, [edx + thread_data.work_dir]
        mov     ecx, 1024
  .loop2:
        lodsb
        or      al, al
        jz      .done
        stosb
        loop    .loop2

  .done:
        stosb

        ret


align 4
socketerror:

        pushd   0x0c
        call    [con_set_flags]
        push    str_sockerr
        call    [con_write_asciiz]
        pushd   0x07
        call    [con_set_flags]

        mcall   send, [edx + thread_data.socketnum], str425, str425.length, 0    ; data connection error

        ret




str150  db '150 Here it comes...', 13, 10
.length = $ - str150
str200  db '200 Command OK.', 13, 10
.length = $ - str200
str215  db '215 UNIX type: L8', 13, 10
.length = $ - str215
str220  db '220 KolibriOS FTP Daemon 1.0', 13, 10
.length = $ - str220
str221  db '221 Bye!', 13, 10
.length = $ - str221
str225  db '225 Data connection open', 13, 10
.length = $ - str225
str226  db '226 Transfer OK, Closing connection', 13, 10
.length = $ - str226
str230  db '230 You are now logged in.', 13, 10
.length = $ - str230
str250  db '250 command successful', 13, 10
.length = $ - str250
str331  db '331 Please specify the password.', 13, 10
.length = $ - str331
str421  db '421 Timeout!', 13, 10
.length = $ - str421
str425  db '425 Cant open data connection.', 13, 10
.length = $ - str425
str500  db '500 Unsupported command', 13, 10
.length = $ - str500
str550  db '550 No such file', 13, 10
.length = $ - str550