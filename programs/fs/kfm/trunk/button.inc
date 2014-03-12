;------------------------------------------------------------------------------
button:
	mcall	17
;--------------------------------------
.1:
	cmp	ah,101
	je	f_key_f1

	cmp	ah,102
	je	f_key_f2

	cmp	ah,103
	je	f_key_f3 ;sort_name

	cmp	ah,104
	je	f_key_f4 ;sort_type

	cmp	ah,105
	je	f_key_f5 ;sort_size

	cmp	ah,106
	je	f_key_f6 ;sort_date

	cmp	ah,107
	je	f_key_f7 ;create di

	cmp	ah,108
	je	f_key_f8 ;delete_file

	cmp	ah,112
	je	f_key_f12
;--------------------------------------
.120:
	cmp	ah,120
	jb	.140
	
	cmp	ah,139
	ja	.140
	
	sub	ah,120
	movzx	eax,ah
	imul	eax,10
	add	eax,retrieved_devices_table+1
	mov	esi,eax	; selected path

	cmp	[select_panel_counter],0
	jne	@f

	call	copy_folder_name
	jmp	left_panel_read_folder
;--------------------------------------
@@:
	call	copy_folder_name_1
	jmp	right_panel_read_folder
;--------------------------------------
.140:
	cmp	ah,140
	je	draw_left_select_disk_menu

	cmp	ah,141
	je	draw_right_select_disk_menu

	cmp	ah,142
	je	left_sort_name

	cmp	ah,143
	je	left_sort_type

	cmp	ah,144
	je	left_sort_size

	cmp	ah,145
	je	left_sort_date

	cmp	ah,146
	je	right_sort_name

	cmp	ah,147
	je	right_sort_type

	cmp	ah,148
	je	right_sort_size

	cmp	ah,149
	je	right_sort_date

	cmp	ah,150
	je	exit_dir_1

	cmp	ah,151
	je	reload_dir

	cmp	ah,152
	je	create_directory

	cmp	ah,153
	je	copy_file_f5

	cmp	ah,154
	je	move_file_f6

	cmp	ah,157
	je	delete_file

	cmp	ah, 1
	jne	still
;--------------------------------------
.exit:
	mcall -1
;---------------------------------------------------------------------
reload_dir:
	cmp	[select_panel_counter],0
	jne	@f
	call	proc_read_left_folder
;	mov	[left_panel_clear_all],1
	call	draw_left_panel
	jmp	still
@@:
	call	proc_read_right_folder
;	mov	[right_panel_clear_all],1
	call	draw_right_panel
	jmp	still
;---------------------------------------------------------------------
reload_dir_all:
	call	proc_read_left_folder
	call	proc_read_right_folder
	jmp	red    
;---------------------------------------------------------------------
exit_dir_1:
	mov	[exit_dir],1
	jmp	key_28.1
;---------------------------------------------------------------------
left_panel_read_folder:
	mov	[left_start_draw_cursor_line],0
	jmp	proc_read_folder
;---------------------------------------------------------------------
right_panel_read_folder:
	mov	[right_start_draw_cursor_line],0
	jmp	proc_read_folder_1
;---------------------------------------------------------------------
left_sort_name:
	xor	eax,eax
	cmp	[left_sort_flag],eax
	je	still
	mov	[left_sort_flag],eax
	jmp	left_sort_date.1
;---------------------------------------------------------------------
left_sort_type:
	cmp	[left_sort_flag],1
	je	still
	mov	[left_sort_flag],1
	jmp	left_sort_date.1
;---------------------------------------------------------------------
left_sort_size:
	cmp	[left_sort_flag],2
	je	still
	mov	[left_sort_flag],2
	jmp	left_sort_date.1
;---------------------------------------------------------------------
left_sort_date:
	cmp	[left_sort_flag],3
	je	still
	mov	[left_sort_flag],3
.1:
	call	draw_left_sort_button
	call	left_sort_files
;	mov	[left_panel_clear_all],1
	call	draw_left_panel
	jmp	still
;---------------------------------------------------------------------
right_sort_name:
	xor	eax,eax
	cmp	[right_sort_flag],eax
	je	still
	mov	[right_sort_flag],eax
	jmp	right_sort_date.1
;---------------------------------------------------------------------
right_sort_type:
	cmp	[right_sort_flag],1
	je	still
	mov	[right_sort_flag],1
	jmp	right_sort_date.1
;---------------------------------------------------------------------
right_sort_size:
	cmp	[right_sort_flag],2
	je	still
	mov	[right_sort_flag],2
	jmp	right_sort_date.1
;---------------------------------------------------------------------
right_sort_date:
	cmp	[right_sort_flag],3
	je	still
	mov	[right_sort_flag],3
.1:
	call	draw_right_sort_button
	call	right_sort_files
	call	draw_right_panel
	jmp	still
;---------------------------------------------------------------------