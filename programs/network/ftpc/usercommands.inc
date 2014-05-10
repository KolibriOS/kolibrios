cmd_help:

        push    str_help
        call    [con_write_asciiz]

        jmp     wait_for_usercommand


cmd_bye:

; Send BYE message to the server
        mov     dword[s], "BYE" + 13 shl 24
        mov     byte[s+4], 10
        mcall   send, [socketnum], s, 5, 0

; Close the control connection
        mcall   close, [socketnum]
        jmp     main


cmd_pwd:

        mov     dword[s], "PWD" + 13 shl 24
        mov     byte[s+4], 10
        mcall   send, [socketnum], s, 5, 0

        jmp     wait_for_servercommand


cmd_cwd:

        mov     dword[s], "CWD "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand


cmd_dele:

        mov     dword[s], "DELE"
        mov     byte[s], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand


cmd_list:
        call    open_dataconnection

        mov     [operation], OPERATION_LIST

        mov     dword[s], "LIST"
        mov     word[s+4], 0x0a0d
        mcall   send, [socketnum], s, 6, 0 

        jmp     wait_for_servercommand


cmd_retr:
        call    open_dataconnection
        
; Create/open the file

        mov     esi, s+5
        mov     ecx, 256-5
        call    set_filename

        mov     [filestruct.subfn], 2   ; create/rewrite file
        mov     [filestruct.offset], 0
        mov     [filestruct.offset+4], 0
        mov     [filestruct.size], 0
        mov     [filestruct.ptr], 0

        mcall   70, filestruct
        cmp     eax, -1
;        je      fileerror

; Prepare to write to the file

        mov     [filestruct.subfn], 3   ; write to file
        mov     [operation], OPERATION_RETR

; Request the file from server

        mov     dword[s], "RETR"
        mov     byte[s+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s]
        mov     dword[edi - 2], 0x0a0d
        mcall   send, [socketnum], s, , 0

        invoke  con_write_asciiz, s     ; print command
        jmp     wait_for_servercommand


cmd_stor:

        call    open_dataconnection

        mov     [operation], OPERATION_STOR

        mov     [filestruct.subfn], 0   ; read file
        mov     [filestruct.offset], 0
        mov     [filestruct.offset+4], 0
        mov     [filestruct.size], BUFFERSIZE
        mov     [filestruct.ptr], buffer_ptr2

        mov     esi, s+5
        mov     ecx, 256-5
        call    set_filename

        mov     dword[s], "STOR"
        mov     byte[s+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s]
        mov     word [edi - 2], 0x0a0d
        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand


cmd_lcwd:

        mov     esi, s+5
        mov     ecx, 256-5
  .loop:
        lodsb
        cmp     al, 10
        je      .done
        test    al, al
        je      .done
        loop    .loop
  .done:
        mov     byte[esi-1], 0
        mcall   30, 1, s+5              ; set working directory
        mcall   30, 2, s, 256           ; and read it again

        invoke  con_write_asciiz, str_lcwd
        invoke  con_write_asciiz, s
        invoke  con_write_asciiz, str_newline

        jmp     wait_for_usercommand


cmd_cdup:

        mov     dword[s], "CDUP"
        mov     word[s+4], 0x0d0a
        mcall   send, [socketnum], s, 6, 0

        jmp     wait_for_servercommand


cmd_rmd:

        mov     dword[s], "RMD "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand


cmd_mkd:

        mov     dword[s], "MKD "

        mov     ecx, 256
        xor     al, al
        mov     edi, s
        repne scasb
        lea     esi, [edi - s]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], s, , 0

        jmp     wait_for_servercommand


; esi   = source ptr
; ecx   = max length of source buffer
set_filename:

        mov     edi, filestruct.name
  .loop:
        lodsb
        test    al, al
        jz      .done
        cmp     al, 10
        je      .done
        stosb
        loop    .loop
  .done:
        xor     al, al          ; append a 0 byte
        stosb

        ret