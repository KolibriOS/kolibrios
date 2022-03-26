;------------------------------------------------------------------------------
draw_buttons_panel:
	mov	ecx,[skin_high]
	add	ecx,19
	shl	ecx,16
	mov	cx,18
	mov	ebx,[window_width]
	sub	ebx,9
	add	ebx,5 shl 16
	mcall	13,,,0xffffff
	
	mov	edx,15 shl 16+20
	add	edx,[skin_high]
	mcall	7,buttons_file_data,<198,16>
	
	mov ecx, 20
	add ecx, [skin_high]
	shl ecx, 16
	add ecx, 16
	mcall	8,<15,16>,,150+0x40000000
	
_new_but:
	add	ebx,26 shl 16
	inc edx
	mcall
	cmp edx, 157+0x40000000
	jl  _new_but
	
	ret
;--------------------------------------
.calculate_button:
	mov	ebx,edx
	mov	bx,15
	mov	ecx,edx
	shl	ecx,16
	mov	cx,bx
	ret
;------------------------------------------------------------------------------
draw_left_sort_button:
	mov	bx,[left_panel_x.start_x]
	add	bx,[left_panel_x.size_x]
	sub	ebx,50+15+12
	mov	cx,[left_panel_y.start_y]
	call	calclulate_sort_button_1
	mov	edx,145
;--------------------------------------
.start:
	mov	esi,0xffffff
	dec	ebp
	cmp	[left_sort_flag],ebp
	jne	@f

	mov	esi,0xccccff
;--------------------------------------
@@:
	call	calclulate_sort_button_2
	jnz	.start

	ret
;------------------------------------------------------------------------------
calclulate_sort_button_1:
	shl	ebx,16
	mov	bx,50
	sub	cx,14
	shl	ecx,16
	mov	cx,12
	mov	eax,8
	mov	ebp,4
	ret
;------------------------------------------------------------------------------
calclulate_sort_button_2:
	inc	ebp
	mcall
	pusha
	push	ebx
	mov	eax,ebx
	inc	eax
	shl	eax,16
	add	ebx,eax
	cmp	ebp,4
	jne	@f

	mov	bx,15+12
	jmp	.continue
;--------------------------------------
@@:
	mov	bx,1
;--------------------------------------
.continue:
	inc	ecx
	mcall	13,,,0xcccccc
	pop	ebx
	cmp	ebp,1
	jne	@f

	mov	eax,[temp_panel_end]
	shl	eax,16
	add	eax,[temp_panel_end]
	shr	eax,16
	shr	ebx,16
	sub	ebx,eax
	shl	ebx,16
	mov	bx,ax
	rol	ebx,16
	mcall	13
	mov	[temp_panel_end],ebx
;--------------------------------------
@@:
	popa
	pusha
	shr	ecx,16
	mov	bx,cx
	add	ebx,3 shl 16+2
	mov	edx,ebp
	dec	edx
	shl	edx,2
	add	edx,sort_button_name
	mov	eax,4
	xor	ecx,ecx
	mov	esi,eax
	mcall
	popa
	dec	edx
	sub	ebx,38 shl 16
	mov	bx,36
	dec	ebp
	ret
;------------------------------------------------------------------------------
draw_right_sort_button:
	mov	bx,[right_panel_x.start_x]
	add	bx,[right_panel_x.size_x]
	sub	ebx,50+15+12
	mov	cx,[right_panel_y.start_y]
	call	calclulate_sort_button_1
	mov	edx,149
;--------------------------------------
.start:
	mov	esi,0xffffff
	dec	ebp
	cmp	[right_sort_flag],ebp
	jne	@f

	mov	esi,0xccccff
;--------------------------------------
@@:
	call	calclulate_sort_button_2
	jnz	.start

	ret
;------------------------------------------------------------------------------
draw_left_select_disk_button:
	mov	bx,[left_panel_x.start_x]
	mov	cx,[left_panel_y.start_y]
	mov	ebp,read_folder_name	      ;read_folder.name
	mov	edx,0x40000000+140
;--------------------------------------
.1:
	shl	ebx,16
	mov	bx,60
	sub	cx,14
	shl	ecx,16
	mov	cx,13
	mcall	8,,,,0xffffff
	mcall	13,,,0xffffff
	pusha
	mov	esi,ebp
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
	sub	esi,ebp  ;read_folder.name
	add	ebx,1 shl 16
	shr	ecx,16
	mov	bx,cx
	add	ebx,3
	mov	edx,ebp
	mcall	4,,0  ;,read_folder.name
	popa
	add	ebx,50 shl 16
	mov	bx,10
	mcall	,,,0x6060ff
	mov	[temp_panel_end],ebx
	add	ebx,1 shl 16
	shr	ecx,16
	mov	bx,cx
	add	ebx,3
	mcall	4, ,0x80ffffff,select_disk_char
	add	ebx,1 shl 16
	mcall
	ret
;------------------------------------------------------------------------------
draw_right_select_disk_button:
	mov	bx,[right_panel_x.start_x]
	mov	cx,[right_panel_y.start_y]
	mov	ebp,read_folder_1_name   ;read_folder_1.name
	mov	edx,0x40000000+141
	jmp	draw_left_select_disk_button.1
;------------------------------------------------------------------------------
;draw_ATAPI_tray_control:
;	ret
;------------------------------------------------------------------------------
calc_max_panel_line:
	xor	eax,eax
	mov	ax,[left_panel_y.size_y]
	mov	ebx,10
	xor	edx,edx
	div	ebx
	mov	[max_panel_line],eax
	ret
;------------------------------------------------------------------------------
draw_icon:
	pusha
	test	[edx-40],byte 0x10
	jnz	.draw_dir_pic

	call	get_icon_number
	imul	ebx,270
	add	ebx,icons_file_data
	jmp	.draw
;--------------------------------------
.draw_dir_pic:
	mov	ebx,icons_file_data
	cmp	[edx],word '..'
	jne	 .draw

	cmp	[edx+2],byte 0
	jne	.draw

	add	ebx,270   ;*17
;--------------------------------------
.draw:
	mov	edx,[esp+16]	;ebx
	ror	edx,16
	sub	edx,12
	rol	edx,16
	dec edx
	mcall	7, ,<10,9>
	popa
	ret
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
draw_type_size_date:
	pusha
	test	[edx-40],byte 0x10
	jz	.copy_type

	mov	[type_table],dword '<DIR'
	mov	[type_table+4],word '> '
	mov	[file_name_length],0
	mov	[extension_size],0
	jmp	.start  ;.size
;--------------------------------------
.copy_type:
	mov	[type_table],dword '    '
	mov	[type_table+4],word '  '
;--------------------------------------
.start:
	mov	esi,edx
	xor	eax,eax
;--------------------------------------
@@:
	cld
	lodsb
	test	eax,eax
	jnz	@b

	mov	[file_name_length],esi
	sub	[file_name_length],edx
	mov	[temp_counter_1],esi
	test	[edx-40],byte 0x10
	jnz	 .size

	dec	esi
	dec	edx
;--------------------------------------
@@:
	std
	lodsb
	cmp	esi,edx
	je	.extension_size_0

	cmp	al,'.'
	jnz	@b

	add	esi,2
	mov	[extension_start],esi
	mov	ecx,[temp_counter_1]
	sub	ecx,esi
	inc	ecx
	mov	[extension_size],ecx
	sub	[file_name_length],ecx
	cmp	ecx,2
	ja	@f

	inc	[file_name_length]
;--------------------------------------
@@:
	sub	ecx,2
	cmp	ecx,4
	jbe	@f

	mov	ecx,4
	mov	[type_table+4],word '..'
;--------------------------------------
@@:
	mov	edi,type_table  ;+1
	cld
	rep	movsb
	mov	[edi],byte ' '
	inc	edx
	jmp	.size
;--------------------------------------
.extension_size_0:
	inc	edx
	mov	[extension_size],0
;--------------------------------------
.size:
	test	[edx-40],byte 0x10
	jz	.copy_size

	mov	[size_table],dword '----'
	mov	[size_table+4],word '- '
	jmp	.date
;--------------------------------------
.qword_div:
	mov	eax,[edx-40+32]
	mov	ebx,[edx-40+32+4]
@@:	; /1024
	shrd	eax,ebx,5 ; /32
	shr	ebx,5 ; /32
	shrd	eax,ebx,5 ; /32
	shr	ebx,5 ; /32
	dec	ecx
	jnz	@b
; /(1024*1024)
	shr	eax,20
	test	eax,eax
	ret
;--------------------------------------
.copy_size:
;/0x1000000000000000 - EB
;/0x4000000000000 - PB
;/0x10000000000 - TB
;/0x40000000 - GB
;/0x100000 - MB
;/0x400 - KB
	mov	[size_table],dword '    '
	mov	[size_table+4],word '  '
	push	ebx edx
	push	ecx
	mov	ecx,4
	call	.qword_div
	pop	ecx
	jz	@f

	mov	ebx,size_table+3
	call	decimal_string
	mov	[ebx+1],byte 'E' ; Exa Byte
	jmp	.size_convert_end
;--------------------------------------
@@:
	push	ecx
	mov	ecx,3
	call	.qword_div
	pop	ecx
	jz	@f

	mov	ebx,size_table+3
	call	decimal_string
	mov	[ebx+1],byte 'P' ; Peta Byte
	jmp	.size_convert_end
;--------------------------------------
@@:
	push	ecx
	mov	ecx,2
	call	.qword_div
	pop	ecx
	jz	@f

	mov	ebx,size_table+3
	call	decimal_string
	mov	[ebx+1],byte 'T' ; Tera Byte
	jmp	.size_convert_end
;--------------------------------------
@@:
	push	ecx
	mov	ecx,1
	call	.qword_div
	pop	ecx
	jz	@f

	mov	ebx,size_table+3
	call	decimal_string
	mov	[ebx+1],byte 'G' ; Giga Byte
	jmp	.size_convert_end
;--------------------------------------
@@:
	mov	eax,[edx-40+32]
	mov	ebx,eax
	shr	eax,20 ; /(1024*1024)
	test	eax,eax
	jz	@f

	mov	ebx,size_table+3
	call	decimal_string
	mov	[ebx+1],byte 'M' ; Mega Byte
	jmp	.size_convert_end
;--------------------------------------
@@:
	mov	eax,ebx
	shr	eax,10 ; /1024
	test	eax,eax
	jz	@f

	mov	ebx,size_table+3
	call	decimal_string
	mov	[ebx+1],byte 'K' ; Kilo Byte
	jmp	.size_convert_end
;--------------------------------------
@@:
	mov	eax,ebx
;.size_convert:
	mov	ebx,size_table+4
	call	decimal_string
;--------------------------------------
.size_convert_end:
	pop	edx ebx
;--------------------------------------
.date:
	cmp	[edx],word '..'
	jne	@f
	
	cmp	[edx+2],byte 0
	je	.not_show_date
@@:
	xor	eax,eax
	mov	al,[edx-40+28]
	push	ebx
	mov	ebx,date_table
	call	decimal_string_2
	mov	al,[edx-40+29]
	mov	ebx,date_table+3
	call	decimal_string_2
	mov	ax,[edx-40+30]
	lea	ebx,[year_table+4]
	mov	[ebx-4], dword '0000'
	mov	[ebx], byte '0'
	call	decimal_string
	mov	eax,[year_table+1]
	mov	[date_table+6],eax
	pop	ebx
	ror	ebx,16
	add	bx,[left_panel_x.size_x]
	sub	ebx,161	;122+12+15
	rol	ebx,16
	mov	ecx,0
	cmp	[marked_file],byte 0
	je	@f

	mov	ecx,0xff0000
;--------------------------------------
@@:
	mcall	4,,,type_table,22
;--------------------------------------
.not_show_date:
	popa
	ret
;------------------------------------------------------------------------------
draw_folder_data:
	cmp	[drawing_panel_folder],1
	jne	.right_panel

	mov	eax,[left_folder_block]
	sub	eax,[max_panel_line]
	test	eax,0x8000000
	jz	.ok_left

	mov	[left_start_draw_line],0
	jmp	@f
;--------------------------------------
.ok_left:
	cmp	eax,[left_start_draw_line]
	jae	@f

	mov	[left_start_draw_line],eax
;--------------------------------------
@@:
	mov	bx,[left_panel_x.start_x]
	add	bx,12
	shl	ebx,16
	mov	bx,[left_panel_y.start_y]
	inc	bx
	mov	edx,[left_start_draw_line]
	imul	edx,304
	add	edx,[left_folder_data]
	add	edx,32+40
	mov	[temp_counter],0
	jmp	.type_folder
;--------------------------------------
.right_panel:
	mov	eax,[right_folder_block]
	sub	eax,[max_panel_line]
	test	eax,0x8000000
	jz	.ok_right

	mov	[right_start_draw_line],0
	jmp	@f
;--------------------------------------
.ok_right:
	cmp	eax,[right_start_draw_line]
	jae	@f

	mov	[right_start_draw_line],eax
;--------------------------------------
@@:
	mov	bx,[right_panel_x.start_x]
	add	bx,12
	shl	ebx,16
	mov	bx,[right_panel_y.start_y]
	inc	bx
	mov	edx,[right_start_draw_line]
	imul	edx,304
	add	edx,[right_folder_data]
	add	edx,32+40
	mov	[temp_counter],1
;--------------------------------------
.type_folder:
	xor	eax,eax
;--------------------------------------
.start:
	pusha
	mov	eax,edx
	cmp	[temp_counter],1
	je	@f

	sub	eax,[left_folder_data]
	sub	eax,32+40
	xor	edx,edx
	mov	ebx,304
	div	ebx
	cmp	eax,[left_folder_block]
	je	.end_1

	jmp	.continue_1
;--------------------------------------
@@:
	sub	eax,[right_folder_data]
	sub	eax,32+40
	xor	edx,edx
	mov	ebx,304
	div	ebx
	cmp	eax,[right_folder_block]
	je	.end_1
;--------------------------------------
.continue_1:
	popa
	cmp	eax,[max_panel_line]	;  20
	je	.end_2

	mov	[marked_file],0
	cmp	[edx+299-40],byte 0
	je	@f

	mov	[marked_file],1
;--------------------------------------
@@:
	call	clear_line
	call	draw_type_size_date
	call	draw_icon
	push	eax
	xor	eax,eax
	mov	ax,[left_panel_x.size_x]
	push	ebx edx
	mov	ebx,6
	xor	edx,edx
	div	ebx
	pop	edx ebx
	sub	eax,23+2+2+2
	mov	esi,[file_name_length]
	mov	[temp_counter_1],0
	cmp	esi,eax
	jbe	@f

	mov	esi,eax
	mov	[temp_counter_1],1
;--------------------------------------
@@:
	mov	ecx,0
	cmp	[marked_file],byte 0
	je	@f

	mov	ecx,0xff0000
;--------------------------------------
@@:
	mcall	4
	cmp	[temp_counter_1],0
	je	.continue

	mov	eax,esi ;[file_name_length]
	imul	eax,6
	shl	eax,16
	push	ebx edx
	add	ebx,eax
	mcall	4,,0xff0000,truncated_filename_char,2
	add	ebx,1 shl 16
	mcall
	pop	edx ebx
;--------------------------------------
.continue:
	add	ebx,10
	add	edx,304 ;10
	pop	eax
	inc	eax
	jmp	.start
;--------------------------------------
.end_1:
	popa
;--------------------------------------
.end_2:    
	mov	ax,[left_panel_y.start_y]
	add	ax,[left_panel_y.size_y]
	mov	cx,bx
	dec	cx
	rol	ecx,16
	mov	cx,ax
	mov	eax,ecx
	shr	eax,16
	sub	cx,ax
	cmp	cx,0
	jbe	@f

	mov	bx,[left_panel_x.size_x]
	sub	ebx,12 shl 16
	mcall	13,,,0xffffff
;--------------------------------------
@@:
.end:
	cmp	[drawing_panel_folder],1
	jne	@f

	call	draw_scroll_bar_left
	ret
;--------------------------------------
@@:
	call	draw_scroll_bar_right
	ret
;------------------------------------------------------------------------------
clear_line:
	cmp	[not_clear_lines],0
	jne	.end_1

	pusha
	shl	ebx,16
	shr	ebx,16
	dec	ebx
	cmp	[drawing_panel_folder],1
	jne	.right

	cmp	[select_panel_counter],0
	jne	.continue

	mov	ax,[left_start_draw_cursor_line]
	add	ax,[left_panel_y.start_y]
	cmp	bx,ax
	je	.end

	jmp	.continue
;--------------------------------------
.right:
	cmp	[select_panel_counter],0
	je	.continue

	mov	ax,[right_start_draw_cursor_line]
	add	ax,[right_panel_y.start_y]
	cmp	bx,ax
	je	.end
;--------------------------------------
.continue:
	mov	ebx,[esp+16]
	mov	cx,bx
	dec	cx
	rol	ecx,16
	mov	cx,10
	mov	bx,[left_panel_x.size_x]
	cmp	[drawing_panel_folder],1
	je	@f

	dec	ebx
;--------------------------------------
@@:
	sub	ebx,12 shl 16+15
	mcall	13,,,0xffffff
;--------------------------------------
.end:
	popa
;--------------------------------------
.end_1:
	ret
;------------------------------------------------------------------------------
draw_left_panel:
	call	clear_left_panel
	call	calc_max_panel_line
	call	draw_left_panel_selection
	mov	[drawing_panel_folder],1
	call	draw_folder_data
	call	draw_left_panel_patch
	ret
;------------------------------------------------------------------------------
draw_right_panel:
	call	clear_right_panel
	call	calc_max_panel_line
	call	draw_right_panel_selection
	mov	[drawing_panel_folder],2
	call	draw_folder_data
	call	draw_right_panel_patch
	ret
;------------------------------------------------------------------------------
draw_left_panel_patch:
	cmp	[select_panel_counter],0
	jne	.end

	mov	edx,read_folder_name	   ;read_folder.name
;--------------------------------------
.1:
	call	clear_patch_area
	mov	ebx,[window_high]
	sub	ebx,33
	add	ebx,8 shl 16
	mcall	4,,0x80000000  ; ,read_folder.name
;--------------------------------------
.end:
	ret
;------------------------------------------------------------------------------
draw_right_panel_patch:
	cmp	[select_panel_counter],1
	jne	draw_left_panel_patch.end

	mov	edx,read_folder_1_name    ;read_folder_1.name
	jmp	draw_left_panel_patch.1
;------------------------------------------------------------------------------
clear_patch_area:
	push	edx
	mov	ebx,[window_width]
	sub	ebx,9
	add	ebx,5 shl 16
	mov	ecx,[window_high]
	sub	ecx,35
	shl	ecx,16
	mov	cx,12
	mcall	13,,,0xffffcc
	pop	edx
	ret
;------------------------------------------------------------------------------
draw_left_panel_selection:
	cmp	[select_panel_counter],0
	jne	.end
	
	mov	eax,[left_folder_block]
	sub	eax,[left_start_draw_line]
	mov	edx,left_start_draw_cursor_line
	mov	cx,[edx]
	call	for_all_panel_selection
	mov	eax,[left_folder_block]
	dec	eax
	imul	eax,10
	cmp	ax,cx
	jae	 @f

	mov	cx,ax
;--------------------------------------
@@:
	mov	[edx],cx
	mov	ebx,[left_panel_x]
	add	cx,[left_panel_y.start_y]
;--------------------------------------
.1:
	shl	ecx,16
	mov	cx,10
	sub	ebx,15
	mcall	13,,,0xbbddff
;--------------------------------------
.end:
	ret
;------------------------------------------------------------------------------
draw_right_panel_selection:
	cmp	[select_panel_counter],1
	jne	draw_left_panel_selection.end

	mov	eax,[right_folder_block]
	sub	eax,[right_start_draw_line]
	mov	edx,right_start_draw_cursor_line
	mov	cx,[edx]
	call	for_all_panel_selection
	mov	eax,[right_folder_block]
	dec	eax
	imul	eax,10
	cmp	ax,cx
	jae	 @f

	mov	cx,ax
;--------------------------------------
@@:
	mov	[edx],cx
	mov	ebx,[right_panel_x]
	add	cx,[right_panel_y.start_y]
	jmp	draw_left_panel_selection.1
;------------------------------------------------------------------------------
for_all_panel_selection:
	imul	eax,10
	cmp	ax,cx
	jae	@f

	mov	cx,ax
;--------------------------------------
@@:
	mov	eax,[max_panel_line]
	dec	eax
	imul	eax,10
	cmp	ax,cx
	jae	@f

	mov	cx,ax
;--------------------------------------
@@:
	ret
;------------------------------------------------------------------------------
clear_left_panel:
	mov	ebx,7 shl 16
	add	ebx,[window_width]
	shr	bx,1
	sub	ebx,6+1
	mov	[left_panel_x],ebx
	call	calculate_panel_1
	mov	[left_panel_y],ecx
	xor	eax,eax
	mov	ax,cx
	xor	edx,edx
	mov	ebx,10
	div	ebx
; умножение на 10
	shl	eax,1      ; умножение на 2
	lea	eax,[eax+eax*4] ; умножение на 5
	mov	[left_panel_y_correct_size_y],ax
	ret
;------------------------------------------------------------------------------
calculate_panel_1:
	mov	ecx,[skin_high]
	add	ecx,70
	shl	ecx,16
	add	ecx,[window_high]
	sub	ecx,129
	ret
;------------------------------------------------------------------------------
clear_right_panel:
	mov	ebx,[window_width]
	shr	ebx,1
	add	ebx,3
	shl	ebx,16
	add	ebx,[window_width]
	shr	bx,1
	sub	bx,2+6
	mov	[right_panel_x],ebx
	call	calculate_panel_1
	mov	[right_panel_y],ecx
	xor	eax,eax
	mov	ax,cx
	xor	edx,edx
	mov	ebx,10
	div	ebx
; умножение на 10
	shl	eax,1      ; умножение на 2
	lea	eax,[eax+eax*4] ; умножение на 5
	mov	[right_panel_y_correct_size_y],ax
	ret
;------------------------------------------------------------------------------
erase_fbutton:
	mov	edx,0x80000065
	mov	eax,8
	mov	ebp,12
;--------------------------------------
.start:
	mcall
	dec	ebp
	jnz	.start

	ret
;------------------------------------------------------------------------------
draw_fbutton:
	test	[window_status],100b	; window is rolled up
	jnz	.exit

	test	[window_status],10b	; window is minimized to panel
	jnz	.exit

	mov	ecx,[window_high]
	sub	ecx,20
	shl	ecx,16
	add	ecx,15
	mov	eax,[window_width]
	mov	ebx,12
	xor	edx,edx
	div	ebx
	mov	[fbutton_width],eax
	mov	ebx,5 shl 16
	add	ebx,eax
	sub	ebx,2
	mov	esi,0xaa00
	mov	edx,101
	mov	eax,8
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
	sub	ax,5
	mov	bx,ax
	mov	eax,8
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
	mcall	13
	cmp	ebp,1
	je	@f

	ror	ebx,16
	add	ebx,[fbutton_width]
	dec	ebx
	rol	ebx,16
	mov	bx,1
	inc	ecx
	mcall	,,,0xcccccc
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
	mov	ebx, 7 shl 16
	add	ebx,[window_high]
	sub	ebx,19
	mov	eax,4
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
