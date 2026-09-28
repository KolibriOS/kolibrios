align 4
symbol_up   db 24,0
symbol_down db 25,0

;------------------------------------------------------------------------------
align 4
draw_buttons_panel:
	mov	ebx,[window_width]
	sub	ebx,9
	mcall	SF_DRAW_RECT, ,<19,F_BUT_PANEL_H>,[sc.work_light]
if USE_STATIC_LIBS eq 1
	mcall	SF_PUT_IMAGE, buttons_file_data,<199,19>,<12,21>
else
	mcall   SF_PUT_IMAGE_EXT, buttons_file_data,\
		<BUTTON_SIZE_W, BUTTON_SIZE_H>, <10,20>, 24,, 0
	mov     ecx,numimages-1
@@:
	push    ecx
	mov     ecx,(BUTTON_SIZE_W shl 16)+BUTTON_SIZE_H
	add     ebx,ICON_SIZE
	add     edx,BUTTON_W_SP shl 16
	mcall
	pop     ecx
	loop    @b
end if
	mcall	SF_DEFINE_BUTTON, <10,BUTTON_SIZE_W>,<20,BUTTON_SIZE_H>,150+0x40000000
	
_new_but:
	add	ebx,BUTTON_W_SP shl 16
	inc edx
	mcall
	cmp edx, 157+0x40000000
	jl  _new_but
	
	ret
;------------------------------------------------------------------------------
align 4
proc draw_sort_buttons, fb_panel:dword
	mov   edi,[fb_panel]
	cmp   edi,fb_left
	jne   @f
	push  [left_sort_flag]
	push  142
	mov   cx,[sb_left.x_size]
	jmp   .1
@@:
    push  [right_sort_flag]
	push  146
	mov   cx,[sb_right.x_size]
.1:
	movzx ebx,[edi+FileBrowser.start_x]
	add   ebx,FILE_BR_SEL_DR_W
	shl   ebx,16
	mov   bx,[edi+FileBrowser.size_x]
	add   bx,cx
	sub   bx,FILE_BR_SEL_DR_W-1
	movzx ecx,[edi+FileBrowser.start_y]
	sub   ecx,SORT_FILE_BUT_H+1
	shl   ecx,16
	mov   cx,SORT_FILE_BUT_H+1
	mcall SF_DRAW_RECT, ,,[sc.work]

	cmp   [sort_init], f_null_1
	je    @f
	movzx	ebx,[edi+FileBrowser.start_x]
	add	ebx,FILE_BR_COL_LEFT+1
	movzx	ecx,[edi+FileBrowser.start_y]
	sub	cx,SORT_FILE_BUT_H+1
	stdcall draw_sort_buttons_c, ebx, ecx
@@:
	ret
endp
;------------------------------------------------------------------------------
; in:
;   ebx - coord x
;   ecx - coord y
;   edx - button id or 0x80000000 + button id
align 4
clear_sort_buttons:
	shl	ebx,16
	mov	bx,word[file_br_col_w1]
	dec	bx
	shl	ecx,16
	mov	cx,SORT_FILE_BUT_H-1 ;buttons height
	mov	esi,[sc.work_light]
	mcall	SF_DEFINE_BUTTON
	ror	ebx,16
	add	bx,word[file_br_col_w1]
	ror	ebx,16
	mov	bx,FILE_BR_COL_W2-1
	inc edx
	mcall
	add	ebx,FILE_BR_COL_W2 shl 16
	mov	bx,FILE_BR_COL_W3-1
	inc edx
	mcall
	add	ebx,FILE_BR_COL_W3 shl 16
	mov	bx,FILE_BR_COL_W4-1
	inc edx
	mcall
	ret
;------------------------------------------------------------------------------
;in:
;  but_id - start button id
align 4
proc draw_sort_buttons_c, coord_x:dword, coord_y, but_id:dword, sort_type:dword
	mov	ebx,[coord_x]
	mov	ecx,[coord_y]
	mov	edx,[but_id]
	call	clear_sort_buttons

	;make arrows
	mov	byte[sort_button_name.name_e],' '
	mov	byte[sort_button_name.type_e],' '
	mov	byte[sort_button_name.size_e],' '
	mov	byte[sort_button_name.date_e],' '
	mov	al,byte[symbol_down]
	bt	[sort_type],0
	jnc	@f
	mov	al,byte[symbol_up]
@@:
	mov	ebx,[sort_type]
	shr	ebx,1
	cmp	ebx,3
	jne	@f
	mov	byte[sort_button_name.size_e],al
@@:
	cmp	ebx,2
	jne	@f
	mov	byte[sort_button_name.date_e],al
@@:
	cmp	ebx,1
	jne	@f
	mov	byte[sort_button_name.type_e],al
@@:
	or	ebx,ebx
	jnz	@f
	mov	byte[sort_button_name.name_e],al
@@:

	;sort captions
	mov	ebx,[coord_x]
	add	bx,4
	shl	ebx,16
	mov	bx,word[coord_y]
	add	bx,(SORT_FILE_BUT_H-9)/2
	mov	ecx,[sc.work_text]
	or  ecx,0x80000000
	mcall	SF_DRAW_TEXT,,,sort_button_name.name
	ror	ebx,16
	add	bx,word[file_br_col_w1]
	ror	ebx,16
	mcall	,,,sort_button_name.type
	add	ebx,FILE_BR_COL_W2 shl 16
	mcall	,,,sort_button_name.size
	add	ebx,FILE_BR_COL_W3 shl 16
	mcall	,,,sort_button_name.date
	ret
endp
;------------------------------------------------------------------------------
align 4
proc draw_select_disk_button, fb_panel:dword
	pushad
	mov eax,[fb_panel]
	mov	bx,[eax+FileBrowser.start_x]
	mov	cx,[eax+FileBrowser.start_y]
	mov	edx,0x40000000+140
;--------------------------------------
	cmp eax,fb_left
	je  @f
	mov	edi,read_folder_1_name
	inc edx
	jmp .1
@@:
	mov	edi,read_folder_name	      ;read_folder.name
.1:
	shl	ebx,16
	mov	bx,FILE_BR_SEL_DR_W
	sub	cx,SORT_FILE_BUT_H+1
	shl	ecx,16
	mov	cx,SORT_FILE_BUT_H
	mcall	SF_DEFINE_BUTTON, ,,,0xffffff
	mcall	SF_DRAW_RECT, ,,0xffffff
	pusha
	mov	esi,edi
	add	esi,2
;--------------------------------------
@@:
	inc	esi
	cmp	[esi],byte 0
	je	@f

	cmp	[esi],byte '/'
	jne	@r

	add	esi,2
;--------------------------------------
@@:
	sub	esi,edi  ;read_folder.name
	add	ebx,1 shl 16
	shr	ecx,16
	mov	bx,cx
	add	ebx,3
	mov	edx,edi
	mcall	SF_DRAW_TEXT, ,0  ;,read_folder.name
	popa
	add	ebx,(FILE_BR_SEL_DR_W-10) shl 16
	mov	bx,10
	mcall	,,,0x6060ff
	add	ebx,2 shl 16
	shr	ecx,16
	mov	bx,cx
	add	ebx,3
	mcall	SF_DRAW_TEXT, ,0x80ffffff,select_disk_char.but
	popad
	ret
endp
;------------------------------------------------------------------------------
;draw_ATAPI_tray_control:
;	ret
;------------------------------------------------------------------------------
;       ПЕРЕВОД 10-НЫХ ЧИСЕЛ СО ЗНАКОМ В СТРОКОВЫЙ ВИД
;       Вход:
;               AX - число
;               EBX - адрес строки
;       Выход:
;               строка содержит число, конец отмечен кодом 0
;------------------------------------------------------------------------------
decimal_string_2:
	push	eax ebx ecx edx
	xor	ecx,ecx
	mov	[ebx],byte '0'
	inc	ebx
;--------------------------------------
.p3:
	xor	edx,edx
	push	ebx
	mov	ebx,10
	div	ebx
	pop	ebx
	add	edx,48
	push	edx
	inc	ecx
	cmp	ax,0
	jne	.p3

	cmp	ecx,1
	jbe	.p4

	mov	ecx,2
	dec	ebx
;--------------------------------------
.p4:
	pop	edx
	mov	[ebx],dl
	inc	ebx
	loop	.p4
	pop	edx ecx ebx eax
	ret
;------------------------------------------------------------------------------
decimal_string:
	push	eax ebx ecx edx
	mov	ecx,10
;--------------------------------------
.p3:
	xor	edx,edx
	div	ecx
	add	edx,48
	mov	[ebx],dl
	dec	ebx
	test	eax,eax
	jnz	.p3

	pop	edx ecx ebx eax
	ret
;------------------------------------------------------------------------------
align 4
draw_left_panel:
	call	clear_left_panel
	stdcall	draw_panel_patch,fb_left
	mov	[fb_left.all_redraw],1
	stdcall	[FileBrowser_draw],fb_left
	mov	eax,[fb_left.max_panel_line]
	mov	[sb_left.cur_area],eax
	mov	[sb_left.all_redraw], 1
	stdcall	[scrollbar_v_draw], sb_left
	ret
;------------------------------------------------------------------------------
align 4
draw_right_panel:
	call	clear_right_panel
	stdcall	draw_panel_patch,fb_right
	mov	[fb_right.all_redraw],1
	stdcall	[FileBrowser_draw],fb_right
	mov	eax,[fb_right.max_panel_line]
	mov	[sb_right.cur_area],eax
	mov	[sb_right.all_redraw], 1
	stdcall	[scrollbar_v_draw], sb_right
	ret
;------------------------------------------------------------------------------
align 4
proc draw_panel_patch, fb_panel:dword
	pushad
	mov	edi,[fb_panel]
	cmp	[edi+FileBrowser.select_panel_counter],1
	jne	.end
	cmp	edi,fb_left
	jne	@f
	mov	esi,ps_left
	jmp	.1
@@:
	mov	esi,ps_right
.1:
	call	clear_patch_area
	mov	dword[esi+ps_left.start_y-ps_left],ebx
	mov	[esi+ps_left.area_size_x-ps_left],cx
if USE_STATIC_LIBS eq 0
	stdcall	[PathShow_prepare],esi
	stdcall	[PathShow_draw],esi
else
	stdcall	path_show.prepare,esi
	stdcall	path_show.draw,esi
end if
.end:
	popad
	ret
endp
;------------------------------------------------------------------------------
;out:
;  ebx = start_x<<16 + start_y
;  ecx = size_x
align 4
clear_patch_area:
	mov	ebx,[window_width]
	sub	ebx,9
	mov	ecx,[window_high]
	sub ecx,[skin_high]
	sub	ecx,43
	shl	ecx,16
	mov	cx,20
	mcall	SF_DRAW_RECT, ,,0xffffcc
	mov	ebx,[window_high]
	sub ebx,[skin_high]
	sub	ebx,40
	add	ebx,5 shl 16
	mov	ecx,[window_width]
	sub	ecx,9
	ret
;------------------------------------------------------------------------------
align 4
clear_left_panel:
	mov	ebx,2 shl 16
	add	ebx,[window_width]
	shr	bx,1
	sub	ebx,5+2+1+15 ;border_wnd +fb_left.start_x +border_mid/2 +sb_left.x_size
	mov	[fb_left.x],ebx
	;calculate left scroll
	mov [sb_left.x_pos],bx
	add [sb_left.x_pos],2 ;+fb_left.start_x
	;calculate name column width
	movzx eax,bx
	sub eax,FILE_BR_COL_LEFT+FILE_BR_COL_W2+FILE_BR_COL_W3+FILE_BR_COL_W4
	cmp eax,40
	jge @f
	mov eax,40 ;min width
@@:
	mov [file_br_col_w1],eax
	call	calculate_panel_1
	mov	[fb_left.y],ecx
	;calculate left scroll
	mov eax,ecx
	mov [sb_left.y_size],ax
	shr eax,16
	mov [sb_left.y_pos],ax
	ret
;------------------------------------------------------------------------------
calculate_panel_1:
	mov	ecx,FILE_BR_TOP_LINE
	shl	ecx,16
	add	ecx,[window_high]
	sub	ecx,FILE_BR_TOP_LINE+46
	sub ecx,[skin_high]
	ret
;------------------------------------------------------------------------------
clear_right_panel:
	mov	ebx,[window_width]
	shr	bx,1
	sub	bx,5+2+1+15
	mov	eax,[window_width]
	sub eax,ebx
	sub eax,10 ;2*border_wnd
	sub ax,[sb_right.x_size]
	shl eax,16
	add ebx,eax
	mov	[fb_right.x],ebx
	;calculate left scroll
	mov [sb_right.x_pos],bx
	mov eax,ebx
	shr eax,16
	add [sb_right.x_pos],ax

	call	calculate_panel_1
	mov	[fb_right.y],ecx
	;calculate left scroll
	mov eax,ecx
	mov [sb_right.y_size],ax
	shr eax,16
	mov [sb_right.y_pos],ax
	ret
;------------------------------------------------------------------------------
erase_fbutton:
	mov	edx,0x80000065
	mov	eax,SF_DEFINE_BUTTON
	mov	ebp,12
;--------------------------------------
.start:
	mcall
	dec	ebp
	jnz	.start

	ret
;------------------------------------------------------------------------------
align 4
draw_fbutton:
	test	[window_status],100b	; window is rolled up
	jnz	.exit

	test	[window_status],10b	; window is minimized to panel
	jnz	.exit

	mov	ecx,[window_high]
	sub	ecx,[skin_high]
	sub	ecx,20
	shl	ecx,16
	add	ecx,15 ; button height
	mov	eax,[window_width]
	mov	ebx,12
	xor	edx,edx
	div	ebx
	mov	[fbutton_width],eax
	mov	ebx,eax
	sub	ebx,2
	mov	esi,0xaa00
	mov	edx,101
	mov	eax,SF_DEFINE_BUTTON
	mov	ebp,12
;--------------------------------------
.start:
	cmp	ebp,1
	jne	@f

	mov	eax,ebx
	shr	eax,16
	mov	bx,ax
	mov	eax,[window_width]
	sub	ax,bx
	sub	ax,10 ;2*border width
	mov	bx,ax
	mov	eax,SF_DEFINE_BUTTON
;--------------------------------------
@@:
	mcall
	push	eax ebx edx
	mov	edx,0x6060ff
	mov	bx,10
	cmp	ebp,4
	jae	@f

	mov	bx,18
;--------------------------------------
@@:
	mcall	SF_DRAW_RECT
	cmp	ebp,1
	je	@f

	ror	ebx,16
	add	ebx,[fbutton_width]
	dec	ebx
	rol	ebx,16
	mov	bx,1
	inc	ecx
	mcall	,,,[sc.work]
	dec	ecx
;--------------------------------------
@@:
	pop	edx ebx eax
	call	.rol_ror
	inc	edx
	dec	ebp
	jnz	.start

	call	draw_fbutton_name
	ret
;--------------------------------------
.rol_ror:
	rol	ebx,16
	add	ebx,[fbutton_width]
	ror	ebx,16
;--------------------------------------
.exit:
	ret
;------------------------------------------------------------------------------
draw_fbutton_name:
	mov	ebx, 2 shl 16
	add	ebx,[window_high]
	sub ebx,[skin_high]
	sub	ebx,19
	mov	eax,SF_DRAW_TEXT
	mov	ecx,0x90000000
	push	ebx
	call	.type_1
	pop	ebx
	mov	ecx,0x90ffffff
	sub	ebx,1 shl 16 + 1
;--------------------------------------
.type_1:
	cmp	[shift_flag],1
	je	.type_shift

	cmp	[ctrl_flag],1
	je	.type_ctrl

	cmp	[alt_flag],1
	je	.type_alt
;--------------------------------------
.type:
	mov	edx,fbutton_name.f1
	call	.type_all
	mov	edx,fbutton_name.f2
	call	.type_all
	mov	edx,fbutton_name.f3
	call	.type_all
	mov	edx,fbutton_name.f4
	call	.type_all
	mov	edx,fbutton_name.f5
	call	.type_all
	mov	edx,fbutton_name.f6
	call	.type_all
	mov	edx,fbutton_name.f7
	call	.type_all
	mov	edx,fbutton_name.f8
	call	.type_all
	mov	edx,fbutton_name.f9
	call	.type_all
	mov	edx,fbutton_name.f10
	call	.type_all
	mov	edx,fbutton_name.f11
	call	.type_all
	mov	edx,fbutton_name.f12
	mcall
	ret
;--------------------------------------
.type_shift:
	mov	edx,fbutton_name_shift.f1
	call	.type_all
	mov	edx,fbutton_name_shift.f2
	call	.type_all
	mov	edx,fbutton_name_shift.f3
	call	.type_all
	mov	edx,fbutton_name_shift.f4
	call	.type_all
	mov	edx,fbutton_name_shift.f5
	call	.type_all
	mov	edx,fbutton_name_shift.f6
	call	.type_all
	mov	edx,fbutton_name_shift.f7
	call	.type_all
	mov	edx,fbutton_name_shift.f8
	call	.type_all
	mov	edx,fbutton_name_shift.f9
	call	.type_all
	mov	edx,fbutton_name_shift.f10
	call	.type_all
	mov	edx,fbutton_name_shift.f11
	call	.type_all
	mov	edx,fbutton_name_shift.f12
	mcall
	ret
;--------------------------------------
.type_ctrl:
	mov	edx,fbutton_name_ctrl.f1
	call	.type_all
	mov	edx,fbutton_name_ctrl.f2
	call	.type_all
	mov	edx,fbutton_name_ctrl.f3
	call	.type_all
	mov	edx,fbutton_name_ctrl.f4
	call	.type_all
	mov	edx,fbutton_name_ctrl.f5
	call	.type_all
	mov	edx,fbutton_name_ctrl.f6
	call	.type_all
	mov	edx,fbutton_name_ctrl.f7
	call	.type_all
	mov	edx,fbutton_name_ctrl.f8
	call	.type_all
	mov	edx,fbutton_name_ctrl.f9
	call	.type_all
	mov	edx,fbutton_name_ctrl.f10
	call	.type_all
	mov	edx,fbutton_name_ctrl.f11
	call	.type_all
	mov	edx,fbutton_name_ctrl.f12
	mcall
	ret
;--------------------------------------
.type_alt:
	mov	edx,fbutton_name_alt.f1
	call	.type_all
	mov	edx,fbutton_name_alt.f2
	call	.type_all
	mov	edx,fbutton_name_alt.f3
	call	.type_all
	mov	edx,fbutton_name_alt.f4
	call	.type_all
	mov	edx,fbutton_name_alt.f5
	call	.type_all
	mov	edx,fbutton_name_alt.f6
	call	.type_all
	mov	edx,fbutton_name_alt.f7
	call	.type_all
	mov	edx,fbutton_name_alt.f8
	call	.type_all
	mov	edx,fbutton_name_alt.f9
	call	.type_all
	mov	edx,fbutton_name_alt.f10
	call	.type_all
	mov	edx,fbutton_name_alt.f11
	call	.type_all
	mov	edx,fbutton_name_alt.f12
;--------------------------------------
.type_all:
	mcall
	call	draw_fbutton.rol_ror
	ret
;------------------------------------------------------------------------------