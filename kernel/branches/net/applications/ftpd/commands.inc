


align 4
parse_cmd:                              ; esi must point to command

        cmp     byte [esi+3], 0x20
        jae     @f
        mov     byte [esi+3], 0
       @@:

        mov     eax, [esi]
        and     eax, not 0x20202020     ; convert to upper case
                                        ; (also convert spaces to null)
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
        mcall   send, [socketnum2], str500, str500.length, 0

        ret


align 4
commands:                               ; all commands must be in uppercase

        db 'ABOR'
        dd cmdABOR

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

        db 'PWD', 0     ; Print Working Directory
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

        db 0                    ; end marker


align 4
cmdABOR:

        ret

align 4
cmdCWD:

        ret

align 4
cmdDELE:

        ret

align 4
cmdLIST:

        ret

align 4
cmdNLST:

        ret

align 4
cmdNOOP:

        ret

align 4
cmdPASS:

        mcall   send, [socketnum2], str230, str230.length, 0

        push    str_pass_ok
        call    [con_write_asciiz]

        mov     [state], STATE_ACTIVE

        ret

align 4
cmdPASV:

        mov     [mode], MODE_PASSIVE

; TODO: open the UDP socket and return our IP + port

; 227 Entering Passive Mode (a1,a2,a3,a4,p1,p2)
; where a1.a2.a3.a4 is the IP address and p1*256+p2 is the port number.

        ret

align 4
cmdPWD:

        mov     dword[buffer], '257 '
        mov     byte[buffer+4], '"'

        lea     edi, [buffer+5]
        mov     esi, work_dir
        mov     ecx, 1024
  .loop:
        lodsb
        or      al, al
        jz      .ok
        stosb
        dec     ecx
        jnz     .loop

  .ok:
        mov     dword[edi], '"' + 0x000a0d00    ; '"',13,10,0
        lea     esi, [edi - buffer + 4]

        mcall   send, [socketnum2], buffer, , 0

        ret

align 4
cmdPORT:

; PORT a1,a2,a3,a4,p1,p2
; IP address a1.a2.a3.a4, port p1*256+p2

        mov     [mode], MODE_ACTIVE

        lea     esi, [esi+5]
        xor     edx, edx

        call    ascii_to_byte
        mov     dh, bl
        inc     esi
        call    ascii_to_byte
        mov     dl, bl
        shl     edx, 16
        inc     esi
        call    ascii_to_byte
        mov     dh, bl
        inc     esi
        call    ascii_to_byte
        mov     dl, bl
        inc     esi

        mov     [datasock.ip], edx

        call    ascii_to_byte
        mov     dh, bl
        inc     esi
        call    ascii_to_byte
        mov     dl, bl

        mov     [datasock.port], dx

        mcall   socket, AF_INET4, SOCK_DGRAM, 0
        cmp     eax, -1
        je      .err
        mov     [datasocketnum], eax

        mcall   connect, [datasocketnum], datasock, datasock.length
        cmp     eax, -1
        je      .err

        mcall   send, [socketnum2], str225, str225.length, 0
        ret

  .err:

        mcall   send, [socketnum2], str425, str425.length, 0
        ret

align 4
cmdQUIT:

        mcall   send, [socketnum2], str221, str221.length, 0
        mcall   close, [socketnum2]

        ret

align 4
cmdRETR:

        ret

align 4
cmdSTOR:

        ret

align 4
cmdSYST:

        mcall   send, [socketnum2], str215, str215.length, 0

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
        mov     [type], TYPE_ASCII
        jmp     .subtype

  .ebdic:
        mov     [type], TYPE_EBDIC

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
        or      [type], TYPE_NP
        jmp     .ok

  .telnet:
        or      [type], TYPE_TELNET
        jmp     .ok

  .asacc:
        or      [type], TYPE_ASA
        jmp     .ok

  .image:
        mov     [type], TYPE_IMAGE
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
        mov     [type], al

  .ok:
        mcall   send, [socketnum2], str200, str200.length, 0

        ret

align 4
cmdUSER:

        mcall   send, [socketnum2], str331, str331.length, 0
        mov     [state], STATE_LOGIN

        mov     byte [work_dir], "/"
        mov     byte [work_dir+1], 0

        push    str_logged_in
        call    [con_write_asciiz]

        ret







align 4         ; esi = ptr to str
ascii_to_byte:

        xor     ebx, ebx

  .loop:

        movzx   eax, byte[esi]
        sub     al, '0'
        jb      .done
        cmp     al, 9
        ja      .done
        lea     ebx, [ebx*4 + ebx]
        shl     ebx, 1
        add     ebx, eax
        inc     esi

        jmp     .loop

  .done:
        ret





str150  db '150 Here it comes...', 13, 10
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
str230  db '230 You are now logged in.', 13, 10
.length = $ - str230
str250  db '250 command successful', 13, 10
;str257  db '257 "'
;.length = $ - str257
;str257b db '"', 13, 10
;.length = $ - str257b
str331  db '331 Please specify the password.', 13, 10
.length = $ - str331
str421  db '421 Timeout!', 13, 10
.length = $ - str421
str425  db '425 Cant open data connection.', 13, 10
.length = $ - str425
str500  db '500 Unsupported command', 13, 10
.length = $ - str500
str550  db '550 No such file', 13, 10