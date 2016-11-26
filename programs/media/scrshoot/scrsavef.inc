save_file:
pusha
        bt      dword [flags],0
        jnc     .not_shooted

        cmp     byte [fname_buf],0
        je      .no_file_name

        mov     edi,sign_n_input
        call    zstr_to_int
        mov     [sign_n],al

        ; генерируем имя файла
        xor     eax,eax
        mov     ecx,1056
        mov     edi,file_name
        rep     stosb   ; заполняем нулями

        mov     esi,fname_buf
        mov     edi,file_name
.next:
        mov     ah,[esi]
        test    ah,ah
        jz      .str_end
        cmp     ah,'*'
        jne     .no_insert_number
        jmp     .insert_number
.no_insert_number:
        mov     [edi],ah
        inc     edi
        inc     esi
        jmp     .next
.insert_number:
        bt      word [ch6.flags],1
        jnc     @f
        mov     eax,[cur_number]
        movsx   ebx,byte [sign_n]
        push    ebx
        call    int_to_str
        pop     ebx
        add     edi,ebx
@@:
        inc     esi
        jmp     .next
.str_end:

        mcall   SF_SYS_MISC, SSF_MEM_FREE, [sf_buf.bmp_header]

	;create image struct
	movsx ebx,word[scr_buf.width]
	movsx edx,word[scr_buf.height]
	stdcall [img_create], ebx, edx, Image.bpp24
	mov [sf_buf.bmp_area],eax
	test eax,eax
	jz @f
		;copy foto to image buffer
		mov edi,[eax+Image.Data]
		mov esi,[scr_buf.ptr]
		movsx ecx,word[scr_buf.width]
		movsx edx,word[scr_buf.height]
		imul ecx,edx
		imul ecx,3
		rep movsb

		;encode image
		stdcall [img_encode], eax, LIBIMG_FORMAT_PNG, 0
		test eax,eax
		jz @f

		mov [sf_buf.bmp_header],eax
		mov [sf_buf.end],ecx
	@@:
	stdcall [img_destroy], [sf_buf.bmp_area]

        ; заполняем структуру для сохранения файла
        mov     ebp,fs_struc
        mov     [ebp],dword SSF_CREATE_FILE

        mov     eax,[sf_buf.bmp_header]
        mov     ecx,[sf_buf.end]
        mov     [ebp+12],ecx ;file size
        mov     [ebp+16],eax
        mov     [ebp+21],dword file_name

        draw_status saving
        mcall   SF_FILE, fs_struc

        test    eax,eax
        jne     save_error

        inc     dword [cur_number]
        call    draw_number

        draw_status saved_ok

popa
ret

.no_file_name:
        draw_status no_file_name
popa
ret

.not_shooted:
        draw_status not_shooted
popa
ret

save_error:
        cmp     al,5
        jne     @f
        mov     [status.text],dword bad_file_name
@@:
        cmp     al,8
        jne     @f
        mov     [status.text],dword disk_filled
@@:

        cmp     al,9
        jne     @f
        mov     [status.text],dword bad_fat_table
@@:

        cmp     al,10
        jne     @f
        mov     [status.text],dword ac_den
@@:

        cmp     al,11
        jne     @f
        mov     [status.text],dword device_er
@@:
        call    send_draw_status
popa
ret