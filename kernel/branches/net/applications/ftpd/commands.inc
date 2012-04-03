


align 4
parse_cmd:                              ; esi must point to command

        mov     eax, [esi]
        and     eax, not 0x20202020     ; convert to upper case
                                        ; (also convert spaces to null)
        mov     edi, commands           ; list of commands to scan
  .scanloop:
        cmp     eax, [edi]
        jb      .error
        jl      .try_next

        jmp     dword [edi+4]

  .try_next:
        add     edi, 8
        cmp     byte [edi], 0
        jne     .scanloop

  .error:
        ret


align 4
commands:                               ; all commands must be in uppercase, and in alphabetical order.

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
cmdPWD:

        ret

align 4
cmdPORT:

        ret

align 4
cmdQUIT:

        ret

align 4
cmdRETR:

        ret

align 4
cmdSTOR:

        ret

align 4
cmdSYST:

        ret

align 4
cmdTYPE:

        ret

align 4
cmdUSER:

        ret



str150  db '150 Here it comes...', 13, 10
str200  db '200 Command OK.', 13, 10
str215  db '215 UNIX type: L8', 13, 10
str220  db '220 KolibriOS FTP Daemon 1.0', 13, 10
.length = $ - str220
str221  db '221 Bye!', 13, 10
str225  db '225 Abort successful', 13, 10
str226  db '226 Transfer OK, Closing connection', 13, 10
str230  db '230 You are now logged in.', 13, 10
str250  db '250 command successful', 13, 10
str257  db '257 ""', 13, 10
str331  db '331 Please specify the password.', 13, 10
str500  db '500 Unsupported command', 13, 10
str550  db '550 No such file', 13, 10
