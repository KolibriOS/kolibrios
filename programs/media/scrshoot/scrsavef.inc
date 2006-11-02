save_file:
pusha
	bt	dword [flags],0
	jnc	.not_shooted

	cmp	byte [ed_buffer.1],0
	je	.no_file_name

	mov	edi,sign_n_input
	call	zstr_to_int
	mov	[sign_n],al

	; генерируем имя файла
	xor	eax,eax
	mov	ecx,1056
	mov	edi,file_name
	rep	stosb	; заполняем нулями

	mov	esi,ed_buffer.1
	mov	edi,file_name
.next:
	mov	ah,[esi]
	test	ah,ah
	jz	.str_end
	cmp	ah,'*'
	jne	.no_insert_number
	jmp	.insert_number
.no_insert_number:
	mov	[edi],ah
	inc	edi
	inc	esi
	jmp	.next
.insert_number:
	bt	word [ch6.flags],1
	jnc	@f
	mov	eax,[cur_number]
	;mov     ebx,4
	movsx	ebx,byte [sign_n]
	push	ebx
	call	int_to_str
	pop	ebx
	add	edi,ebx
@@:
	inc	esi
	jmp	.next
.str_end:

	; выделяем память
	mov	ecx,[scr_buf.end_ptr]
	mov	[sf_buf.bmp_header],ecx
	add	ecx,0x36
	mov	[sf_buf.bmp_area],ecx
	movsx	ebx,word [scr_buf.width]
	movsx	edx,word [scr_buf.height]
	imul	ebx,edx
	lea	ebx,[ebx*3]
	add	ecx,ebx
	mov	[sf_buf.end],ecx
	add	ecx,4096
	xor	ebx,ebx
	inc	ebx
	movr	 eax,64
	int	0x40

	; заполняем структуру для сохранения файла
	mov	edi,fs_struc
	mov	[edi],dword 2
	mov	eax,[sf_buf.bmp_header]
	mov	ebx,[sf_buf.end]
	sub	ebx,eax
	mov	[edi+12],ebx
	mov	[edi+16],eax
	mov	[edi+21],dword file_name

	; заполняем заголовок bmp
	mov	edi,[sf_buf.bmp_header]
	mov	[edi],word 'BM'
	mov	ebx,[sf_buf.end]
	sub	ebx,[sf_buf.bmp_area]
	mov	[edi+34],ebx	; размер картинки
	mov	[edi+10],dword 0x36	; размер заголовка
	add	ebx,0x36
	mov	[edi+2],ebx ; размер файла
	mov	[edi+14],dword 0x28
	movsx	eax,word [scr_buf.width]
	mov	[edi+18],eax ; горизонтальный размер
	movsx	eax,word [scr_buf.height]
	mov	[edi+22],eax ; вертикльный размер
	mov	[edi+26],word 1
	mov	[edi+28],word 24

	; копируем и переворачиваем
	mov	esi,[scr_buf.ptr]
	mov	edi,[sf_buf.bmp_area]
	mov	edx,[sf_buf.end]
	sub	edx,edi
	movsx	ecx,word [scr_buf.height]
.next_str:
push	ecx

	movsx	ecx,word [scr_buf.width]
	lea	ecx,[ecx*3]
	mov	ebx,ecx
.next_byte:
	mov	ah,[esi+ebx]
	mov	[edi+edx],ah
	dec	ebx
	dec	edx
	loop	.next_byte
	movsx	ecx,word [scr_buf.width]
	lea	ecx,[ecx*3]
	add	esi,ecx
pop	ecx
	loop	.next_str

	; сохраняем
	draw_status saving
	mov	eax,70
	mov	ebx,fs_struc
	int	0x40

	test	eax,eax
	jne	save_error

	inc	dword [cur_number]
	call	draw_number

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
	cmp	al,5
	jne	@f
	mov	[status.text],dword bad_file_name
@@:
	cmp	al,8
	jne	@f
	mov	[status.text],dword disk_filled
@@:

	cmp	al,9
	jne	@f
	mov	[status.text],dword bad_fat_table
@@:

	cmp	al,10
	jne	@f
	mov	[status.text],dword ac_den
@@:

	cmp	al,11
	jne	@f
	mov	[status.text],dword device_er
@@:
	call	send_draw_status
popa
ret