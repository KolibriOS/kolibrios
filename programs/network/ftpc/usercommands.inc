cmd_help:

        push    str_help
        call    [con_write_asciiz]

        jmp     wait_for_usercommand


cmd_bye:

; Send BYE message to the server
        mov     dword[buf_cmd], "BYE" + 13 shl 24
        mov     byte[buf_cmd+4], 10
        mcall   send, [socketnum], buf_cmd, 5, 0

; Close the control connection
        mcall   close, [socketnum]
        jmp     main


cmd_pwd:

        mov     dword[buf_cmd], "PWD" + 13 shl 24
        mov     byte[buf_cmd+4], 10
        mcall   send, [socketnum], buf_cmd, 5, 0

        jmp     wait_for_servercommand


cmd_cwd:

        mov     dword[buf_cmd], "CWD "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_dele:

        mov     dword[buf_cmd], "DELE"
        mov     byte[buf_cmd], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_list:
        call    open_dataconnection

        mov     [operation], OPERATION_LIST

        mov     dword[buf_cmd], "LIST"
        mov     word[buf_cmd+4], 0x0a0d
        mcall   send, [socketnum], buf_cmd, 6, 0 

        jmp     wait_for_servercommand


cmd_retr:
        call    open_dataconnection
        
; Create/open the file

; Create/open the file

        mov     esi, buf_cmd+5
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
    
        mov     dword[buf_cmd], "RETR"
        mov     byte[buf_cmd+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     dword[edi - 2], 0x0a0d
        mcall   send, [socketnum], buf_cmd, , 0

        invoke  con_write_asciiz, buf_cmd     ; print command
        jmp     wait_for_servercommand

cmd_rdir:        

        mov     [operation], OPERATION_RDIR

; Request filename list from the server

	call    open_dataconnection
	
	mov	[size_fname], 0
        mov     dword[buf_cmd], "NLST"
        mov     word[buf_cmd+4], 0x0a0d
        mcall   send, [socketnum], buf_cmd, 6, 0 

        jmp     wait_for_servercommand	
	
cmd_stor:

        call    open_dataconnection

        mov     [operation], OPERATION_STOR

        mov     [filestruct.subfn], 0   ; read file
        mov     [filestruct.offset], 0
        mov     [filestruct.offset+4], 0
        mov     [filestruct.size], BUFFERSIZE
        mov     [filestruct.ptr], buf_buffer2

        mov     esi, buf_cmd+5
        mov     ecx, 256-5
        call    set_filename

        mov     dword[buf_cmd], "STOR"
        mov     byte[buf_cmd+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d
        mcall   send, [socketnum], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_lcwd:

        mov     esi, buf_cmd+5
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
        mcall   30, 1, buf_cmd+5              ; set working directory
        mcall   30, 2, buf_cmd, 256           ; and read it again

        invoke  con_write_asciiz, str_lcwd
        invoke  con_write_asciiz, buf_cmd
        invoke  con_write_asciiz, str_newline

        jmp     wait_for_usercommand


cmd_cdup:

        mov     dword[buf_cmd], "CDUP"
        mov     word[buf_cmd+4], 0x0d0a
        mcall   send, [socketnum], buf_cmd, 6, 0

        jmp     wait_for_servercommand


cmd_rmd:

        mov     dword[buf_cmd], "RMD "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_mkd:

        mov     dword[buf_cmd], "MKD "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [socketnum], buf_cmd, , 0

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