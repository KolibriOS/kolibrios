cmd_list:

        call    open_dataconnection

        mov     dword[s], "LIST"
        mov     byte[s+4], 0x0a
        mcall   send, [socketnum], s, 5

        jmp     wait_for_servercommand


cmd_help:

        push    str_help
        call    [con_write_asciiz]

        jmp     wait_for_usercommand

cmd_cwd:

        mov     dword[s], "CWD "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s - 1]

        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand


cmd_retr:

        call    open_dataconnection

        mov     dword[s], "RETR"
        mov     byte[s+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s - 1]

        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand


cmd_stor:

        call    open_dataconnection

        mov     dword[s], "STOR"
        mov     byte[s+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s - 1]

        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand