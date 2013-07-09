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