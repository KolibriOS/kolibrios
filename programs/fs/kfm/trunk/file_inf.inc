files_information:
	xor	eax,eax
	mov	[alt_flag],al
	mov	[files_counter],eax
	mov	[directories_counter],eax
	mov	[files_size.low],eax
	mov	[files_size.high],eax
	call	get_files_information
	call	start_files_information_thread
	jmp	still
;------------------------------------------------------------------------------
start_files_information_thread:
	mcall	9,procinfo,-1
	mov	eax,[ebx+46]
	shr	eax,1
	add	eax,[ebx+38]
	sub	eax,INF_WINH/2
	mov	[files_information_window_y+2],ax
	mov	eax,[ebx+42]
	shr	eax,1
	add	eax,[ebx+34]
	sub	eax,INF_WINW/2
	mov	[files_information_window_x+2],ax
	mcall	51,1,thread_files_information_start,thread_stack
	ret
;------------------------------------------------------------------------------
draw_files_information_window:
	mcall	12,1
	mcall	0,[files_information_window_x],[files_information_window_y],0x33ffffcc,,infowin_title
	mov	edx,INF_DATAX shl 16+INF_DATAY+INF_LINEH
	mov	ebx,0x800a0000
	mov	esi,0x10000000
	cmp	[inf_marked_yes],1
	je	.continue_4

	cmp	[nesting_direction],1
	je	@f

	add	edx,INF_LINEH
;--------------------------------------
.continue_4:
; show files counter
	mcall	47,,[files_counter]
;--------------------------------------
@@:
	cmp	[inf_marked_yes],1
	je	.continue_2

	cmp	[nesting_direction],0
	je	.continue_2

	cmp	[directories_counter],0
	je	@f
;--------------------------------------
.continue_2:
; show directories counter
	add	edx,INF_LINEH
	mcall	,,[directories_counter]
;--------------------------------------
@@:
; show file or directory size
	add	edx,INF_LINEH
	mcall	47,0xc0140001,files_size
;--------------------------------------
	mov	ebp,date.create
	call	show_date_and_time_of_files_information
;--------------------------------------
	mov	ebp,date.change
	call	show_date_and_time_of_files_information
;--------------------------------------
	mov	ebp,date.access
	call	show_date_and_time_of_files_information
;--------------------------------------
	cmp	[select_panel_counter],0
	jne	.right_1

	mov	edx,read_folder_name
	jmp	@f
;--------------------------------------
.right_1:
	mov	edx,read_folder_1_name
;--------------------------------------
@@:
	mcall	4,<INF_DATAX,INF_DATAY>,0x90000000,	;file_name
	mcall	,<10,INF_DATAY>,0x90000000,type_location
	cmp	[inf_marked_yes],1
	je	.continue

	mcall	,<INF_DATAX,INF_DATAY+INF_LINEH>,0x90000000,type_name
	mov	ecx,0x90000000
	cmp	[nesting_direction],1
	je	@f

	mcall	,<10,INF_DATAY+INF_LINEH>,,type_dir_name
	jmp	.continue
;--------------------------------------
@@:
	mcall	,<10,INF_DATAY+INF_LINEH>,,type_file_name
;--------------------------------------
.continue:
	cmp	[inf_marked_yes],1
	je	.continue_3

	cmp	[nesting_direction],1
	je	@f
;--------------------------------------
.continue_3:
	add	ebx,INF_LINEH
	mcall	,,,type_files
;--------------------------------------
@@:
	cmp	[inf_marked_yes],1
	je	.continue_1

	cmp	[nesting_direction],0
	je	.continue_1

	cmp	[directories_counter],0
	je	@f
;--------------------------------------
.continue_1:
	add	ebx,INF_LINEH
	mcall	,,,type_dir
;--------------------------------------
@@:
	add	ebx,INF_LINEH
	mcall	,,,type_size
	add	ebx,INF_LINEH
	mcall	,,,type_create
	add	ebx,INF_LINEH
	mcall	,,,type_change
	add	ebx,INF_LINEH
	mcall	,,,type_access
	sub	ebx,INF_LINEH*2
	add	ebx,115 shl 16
	mcall	,,0x90000000,type_limiters_data_time
	add	ebx,INF_LINEH
	mcall
	add	ebx,INF_LINEH
	mcall

	mcall	12,2
	ret
;------------------------------------------------------------------------------
show_date_and_time_of_files_information:
; show date
	add	edx,INF_LINEH
	push	edx
	movzx	ecx,byte [ebp+0]
	mcall	,0x00020000
	add	edx,23 shl 16
	movzx	ecx,byte [ebp+1]
	mcall
	add	edx,23 shl 16
	movzx	ecx,word [ebp+2]
	mcall	,0x00040000
	sub	ebp,12
; show time
	add	edx,41 shl 16
	movzx	ecx,byte [ebp+2]
	mcall	,0x00020000
	add	edx,24 shl 16
	movzx	ecx,byte [ebp+1]
	mcall
	add	edx,23 shl 16
	movzx	ecx,byte [ebp+0]
	mcall
	pop	edx
	ret
;------------------------------------------------------------------------------
thread_files_information_start:
.red:
	call	draw_files_information_window
;--------------------------------------
.still:
	mcall	10
	cmp	eax,1
	je	.red

	cmp	eax,2
	je	.key

	cmp	eax,3
	je	.button

	jmp	.still
;--------------------------------------
.key:
	mcall	2
;--------------------------------------
.button:
	mcall	-1
	jmp	.still
;------------------------------------------------------------------------------
; деление 64-битного числа divident на 16-битное число divisor.
; Частное помещается в 64-битную переменную quotent,
; а остаток - в 16-битную переменную modulo
Division_64bit:
    	mov	ax,[ebp+6]
    	xor	dx,dx
    	mov	bx,[divisor]
	div	bx
    	mov	[quotent+6],ax
    	mov	ax,[ebp+4]
	div	bx
    	mov	[quotent+4],ax
    	mov	ax,[ebp+2]
	div	bx
    	mov	[quotent+2],ax
    	mov	ax,[ebp]
	div	bx
    	mov	[quotent],ax
;    	mov	[modulo],dx
        ret
;--------------------------------------       
;divident:
;      rb 8
divisor:
      rb 2
quotent:
      rb 8
;modulo:
;      rb 2
;------------------------------------------------------------------------------
copy_type_name:
	push	esi
	mov	esi,ebx
	mov	edi,type_name
	cld
;--------------------------------------
@@:
	lodsb
	stosb
	test	al,al
	jnz	@b

	pop	esi
	ret
;------------------------------------------------------------------------------
get_files_information:
	call	get_information_marked_files
	cmp	[inf_marked_yes],1
	je	.end_2

	xor	eax,eax
	xor	edx,edx
	cmp	[select_panel_counter],0
	jne	.right_1

	mov	ax,[left_start_draw_cursor_line]
	mov	ebx,10
	div	ebx
	mov	ebx,[left_start_draw_line]
	add	ebx,eax
	imul	ebx,304
	add	ebx,[left_folder_data]
	call	prepare_data_8
	mov	esi,read_folder_name
	jmp	.end
;--------------------------------------
.right_1:
	mov	ax,[right_start_draw_cursor_line]
	mov	ebx,10
	div	ebx
	mov	ebx,[right_start_draw_line]
	add	ebx,eax
	imul	ebx,304
	add	ebx,[right_folder_data]
	call	prepare_data_8
	mov	esi,read_folder_1_name
;--------------------------------------
.end:
	mov	[error_pointer],ebx
;--------------------------------------
	push	eax ebx
	sub	ebx,40
; время создания файла
	mov	eax,[ebx+8]
	mov	[time.create],eax
; дата создания файла
	mov	eax,[ebx+12]
	mov	[date.create],eax  
; время последнего доступа (чтение или запись)
	mov	eax,[ebx+16]
	mov	[time.access],eax
; дата последнего доступа
	mov	eax,[ebx+20]
	mov	[date.access],eax    
; время последней модификации
	mov	eax,[ebx+24]
	mov	[time.change],eax
; дата последней модификации
	mov	eax,[ebx+28]
	mov	[date.change],eax
	pop	ebx eax
;--------------------------------------
	call	copy_type_name
	mov	edi,file_name
	call	copy_path_1

	test	[ebx-40],byte 0x10
	jz	@f

	push	eax ebx
	call	get_inf_dir
	mov	[nesting_direction],0
	pop	ebx eax
	dec	[directories_counter]
	jmp	.end_2
;--------------------------------------
@@:
	mov	[nesting_direction],1
	call	get_file_size
	test	eax,eax
	jz	.end_1

	call	start_error_window_thread
	add	esp,4
	jmp	reload_dir_all
;--------------------------------------
.end_1:
	mov	eax,[file_features_temp_area+32]
	add	[files_size.low],eax
	mov	eax,[file_features_temp_area+32+4]
	adc	[files_size.high],eax
;--------------------------------------
.end_2:
	ret
;------------------------------------------------------------------------------
get_information_marked_files:
	xor	eax,eax
	mov	[inf_marked_yes],al
	cmp	[select_panel_counter],al
	jne	.right_1

	mov	ebp,[left_folder_block]
;--------------------------------------
.start:
	mov	ebx,ebp
	dec	ebx
	imul	ebx,304
	add	ebx,[left_folder_data]
	add	ebx,32+40
	cmp	[ebx+299-40],byte 0
	je	@f

	cmp	[ebx],word '..'
	jne	.continue

	cmp	[ebx+2],byte 0
	je	.continue_1
;--------------------------------------
.continue:
	mov	esi,read_folder_name
	call	prepare_data_9
;--------------------------------------
.continue_1:
	mov	[inf_marked_yes],1
;--------------------------------------
@@:
	dec	ebp
	jnz	.start

	ret
;--------------------------------------
.right_1:
	mov	ebp,[right_folder_block]
;--------------------------------------
.start_1:
	mov	ebx,ebp
	dec	ebx
	imul	ebx,304
	add	ebx,[right_folder_data]
	add	ebx,32+40
	cmp	[ebx+299-40],byte 0
	je	@f

	cmp	[ebx],word '..'
	jne	.continue_2

	cmp	[ebx+2],byte 0
	je	.continue_3
;--------------------------------------
.continue_2:
	mov	esi,read_folder_1_name
	call	prepare_data_9
;--------------------------------------
.continue_3:
	mov	[inf_marked_yes],1
;--------------------------------------
@@:
	dec	ebp
	jnz	.start_1

	ret
;------------------------------------------------------------------------------
prepare_data_9:
	mov	[error_pointer],ebx
	mov	edi,file_name
	call	copy_path_1
	test	[ebx-40],byte 0x10
	jz	@f

	call	get_inf_dir
	ret
;--------------------------------------
@@:
	inc	[files_counter]
	call	get_file_size
	test	eax,eax
	jz	.continue

	call	start_error_window_thread
	add	esp,8+4
	ret
;--------------------------------------
.continue:
	mov	eax,[file_features_temp_area+32]
	add	[files_size.low],eax
	mov	eax,[file_features_temp_area+32+4]
	adc	[files_size.high],eax
;--------------------------------------
.continue_1:
	ret
;------------------------------------------------------------------------------
prepare_data_8:
	add	ebx,32+40
	cmp	[ebx],word '..'
	jne	.continue

	cmp	[ebx+2],byte 0
	jne	.continue

	add	esp,4+4
	cmp	[select_panel_counter],0
	jne	@f

	call	proc_read_left_folder
	jmp	red
;--------------------------------------
@@:
	call	proc_read_right_folder
	jmp	red
;--------------------------------------
.continue:
	ret
;------------------------------------------------------------------------------
get_inf_dir:
	mov	[nesting_direction],0
	pusha
	mov	[recursive_pointer],0
	mov	eax,[appl_memory]
	mov	[appl_memory_old],eax
;--------------------------------------
.1:
	mov	[read_delete_dir_data.size],0
	mov	[read_delete_dir_data.return],delete_dir_data
	call	read_delete_dir
	test	eax,eax
	jz	@f

	mov	[error_pointer],file_name
	call	start_error_window_thread
	popa
	add	esp,8+8
	jmp	still
;--------------------------------------
@@:
	mov	eax,[delete_dir_data+8]
	imul	eax,304
	add	eax,32
	mov	ebx,eax
	add	ebx,[appl_memory_old]
	cmp	ebx,[appl_memory]
	jbe	@f

	mov	ecx,[appl_memory]
	mov	ecx,ebx
	mov	[appl_memory],ecx
	mcall	64,1
;--------------------------------------
@@:
	mov	eax,[delete_dir_data+8]
	mov	[read_delete_dir_data.size],eax
	mov	eax,[appl_memory_old]
	mov	[read_delete_dir_data.return],eax
	call	read_delete_dir
	test	eax,eax
	jz	@f

	mov	[error_pointer],file_name
	call	start_error_window_thread
	popa
	add	esp,8+8
	jmp	still
;--------------------------------------
@@:
	cmp	[nesting_direction],0
	je	@f

	call	get_inf_dir_2
	jnc	.end

	dec	ebp
	push	ebp
	jmp	.1
;--------------------------------------
@@:
	call	get_inf_dir_1
	jnc	.end
	dec	ebp
	push	ebp
	jmp	.1
;--------------------------------------
.end:
	call	return_delete_path
	dec	[recursive_pointer]
	cmp	[recursive_pointer],0
	jge	.1

	mov	ecx,[appl_memory_old]
	cmp	[appl_memory],ecx
	je	@f

	mov	[appl_memory],ecx
	mcall	64,1
;--------------------------------------
@@:
	popa
	ret
;------------------------------------------------------------------------------
get_inf_dir_2:
	pop	eax
	pop	ebp
	push	eax
	jmp	get_inf_dir_1.start
;--------------------------------------
get_inf_dir_1:
	inc	[directories_counter]
	mov	ebp,[appl_memory_old]
	mov	ebp,[ebp+8]
;--------------------------------------
.start:
	test	ebp,ebp
	jz	.continue_4
	mov	ebx,ebp
	dec	ebx
	imul	ebx,304
	add	ebx,[appl_memory_old]
	add	ebx,32+40
	cmp	[ebx],byte '.'
	jne	@f

	cmp	[ebx+1],byte 0
	je	.continue_4
;--------------------------------------
@@:
	cmp	[ebx],word '..'
	jne	.continue

	cmp	[ebx+2],byte 0
	je	.continue_4
;--------------------------------------
.continue:
	mov	esi,file_name
	inc	[recursive_pointer]
	call	copy_path_2
	test	[ebx-40],byte 0x10
	jnz	.continue_3
;--------------------------------------
@@:
	inc	[files_counter]
;--------------------------------------
.continue_1:
	mov	eax,[ebx+32-40]
	add	[files_size.low],eax
	mov	eax,[ebx+32+4-40]
	adc	[files_size.high],eax
	dec	[recursive_pointer]
	call	return_delete_path
	dec	ebp
	jnz	.start
;--------------------------------------
.continue_4:
	clc
	mov	[nesting_direction],1
	ret
;--------------------------------------
.continue_3:
	stc
	mov	[nesting_direction],0
	ret
;------------------------------------------------------------------------------
;type_patch_test:
;	pusha
;	mcall	5,200
;	mcall	13,<290,300>,<3,13>,0x0
;	mcall	4,<300,5>,0x80FFFFFF,file_name  ;[error_pointer]
;	mcall	47,0x160100,[recursive_pointer],<300,5>,0xffffff
;	mcall	47,0x160100,ebp,<300,5>,0xffffff
;	mcall	5,1
;	popa
;	ret
;temp_value_1	db '1',0
;temp_value_2	db '2',0
;temp_value_3	db '3',0
;temp_value_4	db '4',0
;temp_value_5	db '5',0
;------------------------------------------------------------------------------