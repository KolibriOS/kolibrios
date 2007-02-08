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

	mov	esi,[cur_editor.Lines]

	xor	ebx,ebx
	mov	ecx,[cur_editor.Lines.Count]
    @@: call	get_real_length
	add	ebx,eax
	movzx	eax,word[esi]
	lea	esi,[esi+eax+4]
	loop	@b
	mov	eax,[cur_editor.Lines.Count]
	shl	eax,1
	lea	eax,[eax+ebx+1024]
	call	mem.Alloc
	push	eax
	mov	esi,[cur_editor.Lines]
	mov	edi,eax

  .new_string:
	call	save_string
	cmp	dword[esi],0
	jne	.new_string
	pop	eax
	sub	edi,eax
	add	edi,-2			; minus last CRLF

  .systree_save:
	mov	[f_info70+0],2
	mov	[f_info70+12],edi
	mov	[f_info70+16],eax
	mov	byte[f_info70+20],0
	mov	[f_info70+21],f_info.path
	mcall	70,f_info70

	call	set_status_fs_error

	or	eax,eax
	jnz	.exit.2

  .exit:
	mov	[cur_editor.Modified],0
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
	call	draw_statusbar
	ret
endf

;-----------------------------------------------------------------------------
func load_file ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[tb_opensave.length],0
	jne	@f
	stc
	ret

    @@: mov	esi,tb_opensave.text
	mov	edi,f_info.path
	movzx	ecx,[tb_opensave.length]
	mov	[f_info.length],ecx
	cld
	rep	movsb
	mov	byte[edi],0

	xor	eax,eax
	mov	[f_info70+0],5
	mov	[f_info70+4],eax
	mov	[f_info70+8],eax
	mov	[f_info70+12],eax
	mov	[f_info70+16],file_info
	mov	byte[f_info70+20],al
	mov	[f_info70+21],f_info.path
	mcall	70,f_info70
	mov	[f_info70+0],0
	mov	eax,dword[file_info.Size]
	mov	[f_info70+12],eax
	call	mem.Alloc
	mov	[f_info70+16],eax
	mcall	70,f_info70

	call	set_status_fs_error

	mov	esi,[f_info70+16]

	xchg	eax,ebx
	test	ebx,ebx
	je	.file_found
	cmp	ebx,6		 ;// ATV driver fix (6 instead of 5)
	je	.file_found

	mov	eax,[f_info70+16]
	call	mem.Free
	stc
	ret

  .file_found:
	mov	ecx,eax
	cmp	[tab_bar.Items.Count],1
	jne	@f
	cmp	[cur_editor.FilePath],'/'
	je	@f
	cmp	[cur_editor.Modified],0
	jne	@f
	mov	ebp,cur_editor
	jmp	.lp1
    @@: inc	[do_not_draw]
	call	create_tab
	dec	[do_not_draw]
  .lp1: push	ecx esi edi
	mov	esi,tb_opensave.text
	lea	edi,[ebp+TABITEM.Editor.FilePath]
	movzx	ecx,[tb_opensave.length]
	cld
	rep	movsb
	mov	byte[edi],0
	lea	edi,[ebp+TABITEM.Editor.FilePath]
	movzx	ecx,[tb_opensave.length]
	inc	ecx
    @@: dec	ecx
	jz	@f
	cmp	byte[edi+ecx-1],'/'
	jne	@b
    @@: mov	[ebp+TABITEM.Editor.FileName],ecx
	call	flush_cur_tab
	pop	edi esi ecx
	call	load_from_memory
	mov	eax,[f_info70+16]
	call	mem.Free

	xor	eax,eax
	mov	[cur_editor.TopLeft.Y],eax
	mov	[cur_editor.TopLeft.X],eax
	mov	[cur_editor.Caret.X],eax
	mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.SelStart.X],eax
	mov	[cur_editor.SelStart.Y],eax
	mov	[cur_editor.Modified],al
	mov	[cur_editor.AsmMode],al

	lea	ebx,[cur_editor.FilePath]
	mov	eax,ebx
	call	strlen
	mov	ecx,dword[ebx+eax-3]
	or	ecx,0x202020
	cmp	ecx,'asm'
	jne	@f
	inc	[cur_editor.AsmMode]
	jmp	.nocol
    @@: cmp	ecx,'inc'
	jne	.nocol
	inc	[cur_editor.AsmMode]
    .nocol:
	call	update_caption

	clc
	ret
endf

;-----------------------------------------------------------------------------
func load_from_memory ;///////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; ECX = data length
; ESI = data pointer
; EBP = EDITOR*
;-----------------------------------------------------------------------------
	call	get_lines_in_file
	mov	[ebp+EDITOR.Lines.Count],eax
	lea	edx,[ebx+ecx]
	imul	ebx,eax,14
	add	ebx,edx
	mov	eax,[ebp+EDITOR.Lines]
	mov	[ebp+EDITOR.Lines.Size],ebx
	call	mem.ReAlloc
	mov	[ebp+EDITOR.Lines],eax

	mov	[ebp+EDITOR.Columns.Count],0
	mov	edi,eax
	mov	edx,ecx

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
    @@: cmp	eax,[ebp+EDITOR.Columns.Count]
	jbe	@f
	mov	[ebp+EDITOR.Columns.Count],eax
    @@: mov	[ebp+EDITOR.Modified],0
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
	add	eax,-10
	cmp	eax,[ebp+EDITOR.Columns.Count]
	jbe	.next_line
	mov	[ebp+EDITOR.Columns.Count],eax
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
