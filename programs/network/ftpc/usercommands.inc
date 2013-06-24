cmd_list:

        call    open_dataconnection

        mov     dword[s], "LIST"
        mov     word[s+4], 0x0d0a
        mcall   send, [socketnum], s, 6

        jmp     wait_for_serverdata


cmd_help:

        push    str_help
        call    [con_write_asciiz]

        jmp     wait_for_usercommand