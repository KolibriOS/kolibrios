;-----------------------------------------------------------------------------
func save_file ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------

	mov	esi,tb_opensave.text
	mov	edi,f_info.path
	movzx	ecx,[tb_opensave.length]
	mov	[f_info.length],ecx
	cld
	rep	movsb
	mov	byte[edi],0

	mov	esi,AREA_EDIT	  ; 0x70000 = 448 Kbytes (maximum)
	mov	edi,AREA_TEMP

  .new_string:
	call	save_string
	cmp	dword[esi],0
	jne	.new_string
	sub	edi,AREA_TEMP+2   ; minus last CRLF
;!      mov     [filelen],edi
	cmp	byte[f_info.path],'/'
	je	.systree_save
	mcall	33,f_info.path,AREA_TEMP,edi,0;[filelen],0
	or	eax,eax
	jz	.exit
;       call    file_not_found
	jmp	.exit.2

  .systree_save:
;!      mov     eax,[filelen]
;       mov     [f_info+8],edi ;! eax
;       mov     [f_info+0],1
;        mov     esi,s_fname
;        mov     edi,f_info.path
;        mov     ecx,PATHL
;        cld
;        rep     movsb
	;mcall   58,f_info

	mov	[f_info70+0],2
	mov	[f_info70+12],edi
	mov	[f_info70+16],AREA_TEMP
	mov	byte[f_info70+20],0
	mov	[f_info70+21],f_info.path
	mcall	70,f_info70

	call	set_status_fs_error

	or	eax,eax
	jnz	.exit.2

  .exit:
	mov	[modified],0
	clc
	ret

  .exit.2:
	stc
	ret
endf

;-----------------------------------------------------------------------------
func save_string ;////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	movzx	ecx,word[esi]
	test	dword[esi],0x00010000
	jz	@f
	or	dword[esi],0x00020000
    @@: add	esi,4
;       mov     ecx,eax

    @@: cmp	byte[esi+ecx-1],' '
	jne	@f
	loop	@b
    @@: jecxz	.endcopy
	xor	edx,edx
	mov	ebx,edx
	mov	ah,dl

  .next_char:
	mov	al,[esi+ebx]
	inc	ebx
;        cmp     [asm_mode],0
;        je      .put
	test	[options],OPTS_OPTIMSAVE
	jz	.put
	test	ah,00000001b
	jnz	.char
	cmp	al,'"'
	jne	@f
	xor	ah,00000100b
	jmp	.char
    @@: cmp	al,"'"
	jne	@f
	xor	ah,00000010b
	jmp	.char
    @@: test	ah,00000110b
	jnz	.char
	cmp	al,';'
	jne	@f
	test	ah,00000001b
	jnz	.char
	xor	ah,00000001b
	jmp	.char
    @@: cmp	al,' '
	jne	.char
	inc	edx
	test	ebx,ATABW-1
	jnz	@f
	dec	edx
	jle	.put
	mov	al,9
	xor	edx,edx
	jmp	.put
  .char:
	or	edx,edx
	jz	.put
	push	ecx eax
	mov	ecx,edx
	mov	al,' '
	rep	stosb
	pop	eax ecx
	xor	edx,edx
  .put:
	stosb
    @@: loop	.next_char

  .endcopy:
	mov	eax,0x0A0D
	stosw
	movzx	eax,word[esi-4]
	add	esi,eax;[esi-4]
	ret
endf

func set_status_fs_error
	push	eax
	mov	esi,s_fs_error
    @@: dec	eax
	js	@f
	movzx	ecx,byte[esi]
	lea	esi,[esi+ecx+1]
	jmp	@b
    @@: inc	esi
	mov	[s_status],esi
	pop	eax
	call	writepos
	ret
endf

;-----------------------------------------------------------------------------
func load_hd_file ;///////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[f_info+0],0
	mov	[f_info+8],300000/512
;        mov     esi,s_fname
;        mov     edi,f_info.path;pathfile_read
;        mov     ecx,PATHL
;        cld
;        rep     movsb
	;mcall   58,f_info ; fileinfo_read

	mov	[f_info70+0],0
	mov	[f_info70+4],0
	mov	[f_info70+8],0
	mov	[f_info70+12],300000;/512
	mov	[f_info70+16],AREA_TEMP
	mov	byte[f_info70+20],0
	mov	[f_info70+21],f_info.path
	mcall	70,f_info70

	call	set_status_fs_error

	xchg	eax,ebx
	inc	eax
	test	ebx,ebx
	je	load_file.file_found
	cmp	ebx,6		 ;// ATV driver fix (6 instead of 5)
	je	load_file.file_found
;       jmp     file_not_found
	stc
	ret
endf

;-----------------------------------------------------------------------------
func load_file ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------

	mov	esi,tb_opensave.text
	mov	edi,f_info.path
	movzx	ecx,[tb_opensave.length]
	mov	[f_info.length],ecx
	cld
	rep	movsb
	mov	byte[edi],0

	cmp	byte[f_info.path],'/'
	je	load_hd_file

	mcall	6,f_info.path,0,16800,AREA_TEMP ; 6 = open file
	inc	eax	     ; eax = -1 -> file not found
	jnz	.file_found
;       jmp     file_not_found
	stc
	ret

  .file_found:
	dec	eax
	mov	[filesize],eax
	mov	[lines],1
	mov	[columns],0
	mov	esi,AREA_TEMP
	mov	edi,AREA_EDIT
	mov	edx,eax

  .next_line:
	mov	ebx,edi
	add	edi,4
  .next_char:
	or	edx,edx
	jle	.exit
	lodsb
	dec	edx
	cmp	al,13
	je	.CR
	cmp	al,10
	je	.LF
	cmp	al,9
	je	.TB
	cmp	al,0
	je	.exit
	stosb
	jmp	.next_char

  .exit:
	mov	ecx,10
	mov	al,' '
	rep	stosb
	lea	eax,[edi-4]
	sub	eax,ebx
	mov	[ebx],eax
	mov	dword[ebx+eax+4],0
	sub	eax,10
	jnz	@f
	inc	eax
    @@: cmp	eax,[columns]
	jbe	@f
	mov	[columns],eax
    @@: mov	[modified],0
	clc
	ret

  .CR:	cmp	byte[esi],10
	jne	.LF
	lodsb
	dec	edx
  .LF:	mov	ecx,10
	mov	al,' '
	rep	stosb
	lea	eax,[edi-4]
	sub	eax,ebx
	mov	[ebx],eax
	inc	[lines]
	add	eax,-10
	cmp	eax,[columns]
	jbe	.next_line
	mov	[columns],eax
	jmp	.next_line

  .TB:	lea	eax,[edi-4]
	sub	eax,ebx
	mov	ecx,eax
	add	ecx,ATABW
	and	ecx,not(ATABW-1)
	sub	ecx,eax
	mov	al,' '
	rep	stosb
	jmp	.next_char
endf

;-----------------------------------------------------------------------------
func new_file ;///////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
;       mcall   55,eax,error_beep   ; beep
	mov	[lines],1	    ; open empty document
	mov	[columns],1
	xor	eax,eax
	mov	[top_line],eax
	mov	[pos.x],eax
	mov	[pos.y],eax
	mov	edi,AREA_EDIT+4
	mov	ecx,10
	mov	[edi-4],ecx
	mov	[edi+10],eax
	mov	al,' '
	cld
	rep	stosb

	mov	[f_info.length],0
	mov	[modified],0
	mov	[asm_mode],0
	call	update_caption
	call	drawwindow

	ret
endf