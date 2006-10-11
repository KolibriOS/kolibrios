button:
	mcall	17
	cmp	al,0
	jne	still
	shr	eax,8

	cmp	[bot_mode],0
	je	@f
	mov	ebx,eax
	mov	al,3
	call	[bot_dlg_handler]
	jmp	still

    @@: mov	esi,accel_table2
  .acc: cmp	eax,[esi]
	jne	@f
	call	dword[esi+4]
	jmp	still.skip_write
    @@: add	esi,8
	cmp	byte[esi],0
	jne	.acc

	jmp	still.skip_write

;        cmp     eax,BUTTON_SCRLUP
;        jne     not_up
  btn.scroll_up:
	dec	[top_line]
	jns	@f
	inc	[top_line]
	ret;jmp     still.skip_write
    @@: call	check_inv_all.skip_check
	ret
;  not_up:

;        cmp     eax,BUTTON_SCRLDN
;        jne     not_down
  btn.scroll_down:
	inc	[top_line]
	mov	eax,[lines]
	sub	eax,[lines.scr]
	cmp	eax,[top_line]
	jge	@f
	dec	[top_line]
	;dec     eax
	;mov     [top_line],eax
	ret;jmp     still.skip_write
    @@: call	check_inv_all.skip_check
	ret
;  not_down:

;        cmp     eax,BUTTON_SCRLLT
;        jne     not_left
  btn.scroll_left:
	dec	[left_col]
	jns	@f
	inc	[left_col]
	ret;jmp     still.skip_write
    @@: call	check_inv_all.skip_check
	ret
;  not_left:

;        cmp     eax,BUTTON_SCRLRT
;        jne     not_right
  btn.scroll_right:
	inc	[left_col]
	mov	eax,[columns]
	sub	eax,[columns.scr]
	cmp	eax,[left_col]
	jge	@f
	dec	[left_col]
	;dec     eax
	;mov     [left_col],eax
	ret;jmp     still.skip_write
    @@: call	check_inv_all.skip_check
	ret
;  not_right:

; SEARCH {
;  search:
;        cmp     al,BUTTON_SEARCH
;        jne     no_search
  btn.search:
  key.f3:
	call	search
	jc	@f
	call	check_inv_all
    @@: ret


func search
	cld
	mov	ecx,[pos.y]
	mov	edx,ecx
	call	get_line_offset
	cmp	word[esi],0
	je	.exit
	call	get_real_length
	add	esi,4
	or	eax,eax
	jz	.end_line.2
	mov	ecx,eax
	sub	ecx,[pos.x]
	push	esi
	add	esi,[pos.x]
	;dec     ecx
	;inc     esi
	jmp	@f

  .next_line:
	push	esi
    @@: sub	ecx,[s_search.size]
	inc	ecx

  .next_char:
	dec	ecx
	js	.end_line
	xor	edi,edi

  .next_ok:
	movzx	eax,byte[edi+esi]
	movzx	ebx,byte[edi+s_search]

	cmp	al,$61
	jb	@f
	add	al,[eax+add_table-$61]
    @@: cmp	bl,$61
	jb	@f
	add	bl,[ebx+add_table-$61]
    @@:
	cmp	al,bl
	je	@f

	inc	esi
	jmp	.next_char
    @@:
	inc	edi
	cmp	edi,[s_search.size]
	jne	.next_ok

  .found:
	add	esp,4
	mov	[pos.y],edx
	mov	[sel.y],edx
	mov	ecx,edx
	lea	eax,[esi-4]
	call	get_line_offset
	sub	eax,esi
	mov	[sel.x],eax
	add	eax,[s_search.size]
	mov	[pos.x],eax
	mov	[s_status],0
	clc
	ret

  .end_line:
	pop	esi
  .end_line.2:
	movzx	eax,word[esi-4]
	add	esi,eax;[esi-4]
	inc	edx
	call	get_real_length
	mov	ecx,eax
	lodsd
	or	eax,eax
	jnz	.next_line
  .exit:
	mov	[s_status],s_text_not_found
	stc
	ret
endf

; SEARCH }

;  no_search:

; TOOLBAR {
;        cmp     eax,10000
;        jb      no_toolbar

;        add     eax,-10000
;        jnz     @f
  btn.compile:
  key.ctrl_f9:
	mov	bl,0;[run_outfile],0
	call	start_fasm
	ret;jmp     still
;    @@: dec     eax
;        jnz     @f
  btn.compile_run:
  key.f9:
	mov	bl,1;[run_outfile],1
	call	start_fasm
	ret;jmp     still
;    @@: dec     eax
;        jnz     @f
  btn.debug_board:
	call	open_debug_board
	ret;jmp     still
;    @@: dec     eax
;        jnz     still
  btn.sysfuncs_txt:
	call	open_sysfuncs_txt
	ret;jmp     still
; TOOLBAR }

;  no_toolbar:

;        cmp     al,4
;        jne     noid4

; LOAD_FILE {
;  do_load_file:
  btn.load_file:
  key.ctrl_l:
;        cmp     [s_fname],'/'
;        jne     @f
;        call    load_hd_file
;        jmp     .restorecursor
;    @@: call    load_file
	call	load_file
	jnc	@f
	ret
;    .restorecursor:
    @@:
	xor	eax,eax
	mov	[top_line],eax
	mov	[left_col],eax
	mov	[pos.x],eax
	mov	[pos.y],eax
	mov	[sel.x],eax
	mov	[sel.y],eax

	mov	[modified],al

; enable color syntax for ASM and INC files:
	mov	[asm_mode],al

	mov	eax,[f_info.length] ; [s_fname.size]
	add	eax,f_info.path ; s_fname
	mov	byte[eax],0
	mov     ecx, dword [eax-3]
	or      ecx, 0x202020
	cmp     ecx, 'asm'
	jne	@f
	inc	[asm_mode]
	jmp	.nocol
    @@: cmp	ecx, 'inc'
	jne	.nocol
	inc	[asm_mode]
    .nocol:

  update_caption:
macro unused {
	movzx	ecx,[f_info.length] ; [s_fname.size]
	add	ecx,10		   ; strlen(" - TINYPAD");
	cmp	ecx,[s_title.size]
	jne	@f
	add	ecx,-10
	mov	esi,f_info.path ; s_fname       ; strcmp(s_fname,header);
	mov	edi,s_title
	repe	cmpsb
	jne	@f
;       call    draw_file
	clc
	ret;jmp     still
    @@:
}
; set window title:
	mov	esi,f_info.path ; s_fname
	mov	edi,s_title

	cld
	mov	ecx,[f_info.length] ; [s_fname.size]
	jecxz	@f
	;lea     eax,[ecx+10]
	;mov     [s_title.size],eax
	;cld
	rep	movsb

	mov	dword[edi],' - '
	add	edi,3
    @@: mov	esi,htext
	mov	ecx,htext.size
	rep	movsb

	mov	al,0
	stosb

;       call    drawwindow
	clc
	ret;jmp     still
; LOAD_FILE }

;  noid4:

;        cmp     al, 2
;        jz      yessave

;        dec     al       ; close if butid == 1
;        jnz     nosave
; EXIT:
  btn.close_main_window:
  key.alt_x:
	mov	esi,self_path
	mov	byte[esi+PATHL-1],0
	mov	edi,f_info.path
	cld
    @@: lodsb
	stosb
	or	al,al
	jnz	@b
	mov	ebx,f_info
	mov	dword[ebx+0],1
	mov	dword[ebx+8],self_path
	mov	dword[ebx+12],0
	mcall	58

;       test    eax,eax
;       je      .close
;       cmp     eax,6
;       je      .close
;       ret

  .close:
	mov	[main_closed],1
	mcall	-1

; SAVE_FILE {
;  yessave:
;  btn.save_file:
;  key.ctrl_s:
;        mov     [bot_mode],1
;        mov     [bot_save_mode],1
;        mov     [bot_dlg_height],16*2+4*2-1
;        mov     [bot_dlg_handler],osdlg_handler
;        call    drawwindow
;        call    save_file
;        ret;jmp     still
; SAVE_FILE }

;  nosave:
;  btn.save_enter_name:
;        inc     al
;        call    read_string
;        ret;jmp     still