server_parser:

; Commands are always 3 numbers and followed by a space
; If a server decides it needs multiline output,
; first lines will have a dash instead of space after numbers,
; thus they are simply ignored in this simple command parser.

        cmp     dword[buf_cmd], "150 "
        je      data_loop

        cmp     dword[buf_cmd], "220 "
        je      welcome

;        cmp     dword[buf_cmd], "226 "
;        je      transfer_ok

        cmp     dword[buf_cmd], "227 "
        je      pasv_ok

        cmp     dword[buf_cmd], "230 "
        je      login_ok

;        cmp     dword[buf_cmd], "250"
;        je      op_ok

        cmp     dword[buf_cmd], "331 "
        je      pass

;        cmp     dword[buf_cmd], "421 "
;        je      timeout

        cmp     dword[buf_cmd], "503 "         ; login first
        je      welcome

        cmp     dword[buf_cmd], "530 "         ; password incorrect
        je      welcome

        cmp     dword[buf_cmd], "550 "
        je      close_datacon

        cmp     byte[buf_cmd+3], "-"
        je      wait_for_servercommand
        jmp     wait_for_usercommand


welcome:

        mov     [status], STATUS_CONNECTED
        jmp     wait_for_usercommand


pass:

        mov     [status], STATUS_NEEDPASSWORD
        jmp     wait_for_usercommand


login_ok:

        mov     [status], STATUS_LOGGED_IN
        jmp     wait_for_usercommand


pasv_ok:

        sub     ecx, 4
        jb      .fail
        mov     al, "("
        mov     edi, buf_cmd + 4
        repne   scasb

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      error_socket
        mov     [datasocket], eax

        mov     esi, edi
        call    ascii_dec
        mov     byte[sockaddr2.ip+0], bl
        call    ascii_dec
        mov     byte[sockaddr2.ip+1], bl
        call    ascii_dec
        mov     byte[sockaddr2.ip+2], bl
        call    ascii_dec
        mov     byte[sockaddr2.ip+3], bl

        call    ascii_dec
        mov     byte[sockaddr2.port+0], bl
        call    ascii_dec
        mov     byte[sockaddr2.port+1], bl

        invoke  con_write_asciiz, str_open
        mcall   connect, [datasocket], sockaddr2, 18
;        cmp     eax, -1
;        je      error_socket
        jmp     wait_for_servercommand

  .fail:
        invoke  con_write_asciiz, str_unknown
        jmp     wait_for_servercommand


data_loop:

        invoke  con_write_asciiz, str_dot

        cmp     [operation], OPERATION_STOR
        je      .stor

; we are receiving data
        mcall   recv, [datasocket], buf_buffer2, BUFFERSIZE, 0
        test    ebx, ebx
        jnz     .done
        mov     byte[buf_buffer2 + eax], 0

        cmp     [operation], OPERATION_RETR
        je      .retr

	    cmp 	[operation], OPERATION_RDIR
	    je 	    .rdir
	
; not retreiving, just print to console
        invoke  con_write_asciiz, buf_buffer2
        jmp     data_loop

; retreiving, save to file
  .retr:
        mov     [filestruct.ptr], buf_buffer2
        mov     [filestruct.size], eax
        push    eax
        mcall   70, filestruct
        pop     eax
        add     [filestruct.offset], eax
        jmp     data_loop

; storing, send all data
  .stor:
        mcall   70, filestruct
        cmp     eax, 6          ; end of file
        je      .last_call
        test    eax, eax        ; error
;        jne     .fileerror
        add     [filestruct.offset], ebx
        mov     esi, ebx
        mcall   send, [datasocket], buf_buffer2, , 0
        jmp     .stor

  .last_call:
        mov     esi, ebx
        mcall   send, [datasocket], buf_buffer2, , 0

  .done:
        invoke  con_write_asciiz, str_close
        mcall   close, [datasocket]
        mov     [operation], OPERATION_NONE
        jmp     wait_for_servercommand


  .rdir:
	cmp	[size_fname], 0
	jne	.realloc

  .malloc:					; create a new dynamic block
  	mov 	ecx, eax
	inc 	ecx	

	mcall	68,12	      			; eax now points to new buffer	
	
	test 	eax,eax
	je 	error_heap

	mov 	[ptr_fname_start], eax

	jmp 	.rdir_init

  .realloc:					; expand block created with .malloc

  	mov 	ecx, eax			; eax is size of buffer received
	inc	ecx

	add	ecx, [size_fname]		; added old size to form new required size

	mcall 	68,20,,[ptr_fname_start]

    	test	eax, eax
	je	error_heap

	mov	[ptr_fname_start], eax		; eax contains the new block now
	add	eax, [size_fname]

  .rdir_init:					; copies filenames into our buffer

	mov	esi, buf_buffer2
	mov	edi, eax
	
  .copy_buf:
  	lodsb
	cmp	al,13				; ignore any \r character
	je 	.copy_buf
	stosb

	cmp	al, 10
	jne 	.not_end
	inc 	[queued]

  .not_end:
	test	al,al
	jne 	.copy_buf
	
	dec 	edi
	dec 	edi

	mov	eax, [ptr_fname_start]
	mov 	[ptr_queue], eax

	sub 	edi, eax		; edi contains the current size now
	mov	[size_fname], edi	

	jmp	data_loop
	

; files for rdir operation are queued
transfer_queued:

	mov	esi, [ptr_queue]		; always pointing to current part of ptr_fname_start
	mov 	edi, buf_cmd+5	   		; always point to filename for retr command

  .build_filename:
	lodsb	
	stosb

	cmp 	al,10
	je 	.get_file			; filename ends with character 10
	test	al,al
	jz	.null_found			; this should be end of buffer
	jmp	.build_filename

  .null_found:					

	mov	[queued],0
	jmp 	.free

  .get_file:
	dec	[queued]
	jnz	.after_free	
	
  .free:
	mcall	68,13,[ptr_fname_start]		; freeing the buffer
	test	eax,eax
	jz 	error_heap
	jmp	wait_for_usercommand

  .after_free:
	xor	al,al				; appending 0 after retr command
	stosb
	mov	eax, esi
	mov	[ptr_queue], eax
	
	jmp	cmd_retr
		
close_datacon:
        cmp     [operation], OPERATION_NONE
        je      wait_for_usercommand
        invoke  con_write_asciiz, str_close
        mcall   close, [datasocket]
        jmp     wait_for_usercommand



ascii_dec:

        xor     ebx, ebx
        mov     cl, 4                   ; max length is 3 digits + 1 separator
  .loop:
        lodsb
        sub     al, '0'
        jb      .done
        cmp     al, 9
        ja      .done
        lea     ebx, [ebx*4+ebx]        ; ebx *5
        shl     ebx, 1                  ; ebx *2
        add     bl, al
        dec     cl
        jnz     .loop

  .done:
        ret