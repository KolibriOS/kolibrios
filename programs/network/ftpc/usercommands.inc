cmd_help:

        push    str_help
        call    [con_write_asciiz]

        jmp     wait_for_usercommand


cmd_bye:

; Send BYE message to the server
        mov     dword[buf_cmd], "BYE" + 13 shl 24
        mov     byte[buf_cmd+4], 10
        mcall   send, [controlsocket], buf_cmd, 5, 0

; Close the control connection
        mcall   close, [controlsocket]
        ijmp    eax, interface_addr, interface.server_addr


cmd_pwd:

        mov     dword[buf_cmd], "PWD" + 13 shl 24
        mov     byte[buf_cmd+4], 10
        mcall   send, [controlsocket], buf_cmd, 5, 0

        jmp     wait_for_servercommand


cmd_cwd:

        mov     dword[buf_cmd], "CWD "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [controlsocket], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_dele:

        mov     dword[buf_cmd], "DELE"
        mov     byte[buf_cmd+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [controlsocket], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_list:
        call    open_dataconnection

        mov     [operation], OPERATION_LIST

        mov     dword[buf_cmd], "LIST"
        mov     word[buf_cmd+4], 0x0a0d
        mcall   send, [controlsocket], buf_cmd, 6, 0

        jmp     wait_for_servercommand


cmd_retr:
        call    open_dataconnection
        
; Create/open the file
; TODO: check beforehand if the disk has enough free space available to store the file

        mov     esi, buf_cmd+5
        mov     ecx, 256-5
        call    set_filename

        mov     [filestruct.subfn], 2   ; create/rewrite file
        mov     [filestruct.offset], 0
        mov     [filestruct.offset+4], 0
        mov     [filestruct.size], 0
        mov     [filestruct.ptr], 0

        mcall   70, filestruct
        test    eax, eax
        jz      @f
        call    error_fs
        jmp     close_datacon
      @@:
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
        mcall   send, [controlsocket], buf_cmd, , 0

        icall   eax, interface_addr, interface.print, buf_cmd
        jmp     wait_for_servercommand

cmd_rdir:        

        mov     [operation], OPERATION_RDIR

; Request filename list from the server

        call    open_dataconnection

        mov     [ptr_fname], 0
        mov     [size_fname], 0
        mov     dword[buf_cmd], "NLST"
        mov     word[buf_cmd+4], 0x0a0d
        mcall   send, [controlsocket], buf_cmd, 6, 0

        jmp     wait_for_servercommand  
        
cmd_stor:

        call    open_dataconnection

        mov     [operation], OPERATION_STOR

        ; get file size
        mov     [filestruct.subfn], 5
        mov     [filestruct.offset], 0
        mov     [filestruct.offset+4], 0
        mov     [filestruct.size], 0
        mov     [filestruct.ptr], folder_buf

        mov     esi, buf_cmd+5
        mov     ecx, 256-5
        call    set_filename

        mcall   70, filestruct

        mov     eax, dword[folder_buf+32] ; supports file size upto 4GB
        mov     [file_size], eax

        mov     [filestruct.subfn], 0   ; read file
        ; mov     [filestruct.offset], 0
        ; mov     [filestruct.offset+4], 0
        mov     [filestruct.size], BUFFERSIZE
        mov     [filestruct.ptr], buf_buffer2

        mov     dword[buf_cmd], "STOR"
        mov     byte[buf_cmd+4], " "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d
        mcall   send, [controlsocket], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_lcwd:

        mov     esi, buf_cmd+5
        cmp     byte[esi], 10
        je      .print
        mov     ecx, 256-5
  .loop:
        lodsb
        cmp     al, 10
        je      .check
        test    al, al
        je      .check
        loop    .loop

  .check:
        mov     byte[esi-1], 0

        ; check whether entered path is valid (folder exists)
        mov     [filestruct2.subfn], 5
        mov     [filestruct2.offset], 0
        mov     [filestruct2.size], 0
        mov     [filestruct2.ptr], folder_buf
        mov     [filestruct2.name], buf_cmd+5
        mcall   70, filestruct2
        test    eax, eax
        jz      @f
        cmp     eax, 2
        je      @f
        call    error_fs
        jmp     wait_for_usercommand

  @@:
        mcall   30, 1, buf_cmd+5              ; set working directory

  .print:
        mcall   30, 2, buf_cmd, 256           ; and read it again
        icall   eax, interface_addr, interface.print, str_lcwd, buf_cmd, str_newline

        jmp     wait_for_usercommand


cmd_cdup:

        mov     dword[buf_cmd], "CDUP"
        mov     word[buf_cmd+4], 0x0a0d
        mcall   send, [controlsocket], buf_cmd, 6, 0

        jmp     wait_for_servercommand


cmd_rmd:

        mov     dword[buf_cmd], "RMD "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [controlsocket], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_mkd:

        mov     dword[buf_cmd], "MKD "

        mov     ecx, 256
        xor     al, al
        mov     edi, buf_cmd
        repne scasb
        lea     esi, [edi - buf_cmd]
        mov     word [edi - 2], 0x0a0d

        mcall   send, [controlsocket], buf_cmd, , 0

        jmp     wait_for_servercommand


cmd_abor:

        mcall   close, [datasocket]
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