;---------------------------------------------------------------------
move_file_f6:
    mov  [move_file_flag],1
copy_file_f5:
    call clear_file_buffer
    xor  edx,edx
    xor  eax,eax
;    mov  byte [copy_many_files],al
    mov  [current_progress_value],eax
    mov  [current_progress_value_1],eax
    cmp  [select_panel_counter],al
    jne  .right_1
    cmp  [left_marked_counter],eax
    jne  .confirmation_action
    mov  ax,[left_start_draw_cursor_line]
    mov  ebx,10
    div  ebx
    mov  ebx,[left_start_draw_line]
    add  ebx,eax
    imul ebx,304
    add  ebx,[left_folder_data]
    call prepare_data_7
    call .copy_name
    jmp  editbox_copy_name
.right_1:
    cmp  [right_marked_counter],eax
    jne  .confirmation_action
    mov  ax,[right_start_draw_cursor_line]
    mov  ebx,10
    div  ebx
    mov  ebx,[right_start_draw_line]
    add  ebx,eax
    imul ebx,304
    add  ebx,[right_folder_data]
    call prepare_data_7
    call .copy_name
    jmp  editbox_copy_name
    
.confirmation_action:
    cmp  [move_file_flag],0
    je	 .copy
    mov  [confirmation_type],move_type
    jmp  @f
.copy:
    mov  [confirmation_type],copy_type
@@:
    call confirmation_action
    cmp  [work_confirmation_yes],1
    je	 copy_marked_files
    mov  [move_file_flag],0
    jmp  red

.copy_name:
    mov  esi,ebx
    mov  edi,create_dir_name
@@:
    cld
    lodsb
    stosb
    test al,al
    jne  @r
    sub  edi,create_dir_name
    mov  eax,edi
    dec  eax
    mov edi, edit1
    mov ed_size, eax
    mov ed_pos, eax
default_box
    cmp  [select_panel_counter],0
    jne  .right_2
    mov esi,read_folder_name
    jmp  @f
.right_2:
    mov esi,read_folder_1_name
@@:
    mov  edi,file_name
    call copy_path_1
    ret
;-------------------------------------------------------------
editbox_copy_name:
    mcall 66, 1, 0
    jmp  .red
.red_1:
    call  draw_window
.red:
    call  draw_editbox_copy_name
.still:
    mcall 10
    cmp  eax,1
    je	 .red_1
    cmp  eax,2
    je	 .key_copy
    cmp  eax,3
    je	 .button_copy
	mouse_edit_box copy_editboxes
	jmp .still
;-------------------------------------------------------------
.button_copy:
    mov eax,17
    int 0x40
    test ah,ah
    jz	.still
    cmp  ah,160
    je	.exit_with_copy
    cmp  ah,161
    je	.exit_without_copy
    cmp   ah,1
    jne   .still
    mcall -1
.exit_with_copy:
    mov  [max_progress_value],1
    mov  [draw_all_progressbar],1
    call  copy_file_1
    cmp   [move_file_flag],0
    je	  @f
    call  del_file
    test  eax,eax
    jz	  @f
    call  start_error_window_thread
@@:
    call proc_read_left_folder
    call proc_read_right_folder
.exit_without_copy:
    mcall 66, 1, 1
    mov  [move_file_flag],0
    jmp  red
;-------------------------------------------------------------
.key_copy:
    mov eax,2
    int 0x40
    cmp ah,13
    je	.exit_with_copy
    cmp ah,27
    je	.exit_without_copy
    key_edit_box copy_editboxes
    jmp .still
;-------------------------------------------------------------
draw_editbox_copy_name:
    mov  ecx,[window_high]
    mov  ebx,[window_width]
    shr  ecx,1
    shr  ebx,1
    sub  ecx,30
    sub  ebx,87
    shl  ecx,16
    shl  ebx,16
    mov   cx,60
    mov   bx,174
    mcall 13, , ,0x6060ff ;0xff
    pusha
    shr ebx,16
    add  ebx,10
    shr ecx,16
    add ecx,17
    mov eax,copy_editboxes
    mov dword [eax+4],ebx
    mov dword [eax+8],ecx
	draw_edit_box copy_editboxes
	popa
	pusha
	add   ebx,70 shl 16
	add   ecx,37 shl 16
    mov   cx,15
    mov   bx,40
    mov   esi,0xffffff
    mov   edx,0x40000000+160
    mov   eax,8
    int   0x40
    add   ebx,50 shl 16
    inc   edx
    int   0x40
    popa

    push  ebx ecx
    add   ebx,70 shl 16
    add   ecx,37 shl 16
    mov   bx,40
    mov   cx,15
    mov   edx,0
    push  ebx ecx
    sub   ecx,1 shl 16
    sub   ebx,1 shl 16
    mov   bx,42
    mov   cx,17
    int   0x40
    add   ebx,50 shl 16
    int   0x40
    pop   ecx ebx
    mov   edx,0xff7700
    int   0x40
    mov   edx,0xaa00
    add   ebx,50 shl 16
    int   0x40
    pop   ecx ebx

    shr   ecx,16
    mov   bx,cx
    add   ebx,60 shl 16+5
    cmp   [move_file_flag],0
    je	  .copy
    mov   edx,move_type_1
    jmp   @f
.copy:
    mov   edx,copy_type_1
@@:
    mcall 4, ,0x90ffffff
    add   ebx,18 shl 16+36
    mov   edx,type_yes
    int   0x40
    add   ebx,56 shl 16
    mov   edx,type_no
    int   0x40
	ret
;-------------------------------------------------------------
copy_file_1:
    mov  ebx,create_dir_name
.1:
    mov   [error_pointer],ebx
    cmp  [select_panel_counter],0
    je	.right_2
    mov esi,read_folder_name
    jmp  @f
.right_2:
    mov esi,read_folder_1_name
@@:
    mov  edi,start_parameter
    call copy_path_1
    call get_file_size
    test  eax,eax
    jnz   .error
    mov   [read_file.Offset],eax
    mov   [add_save_file.Offset],eax
    mov   eax,[appl_memory]
    mov   [read_file.return],eax
    mov   [save_file.pointer],eax
    mov   [add_save_file.pointer],eax
    mcall 18,16
    mov   ebx,[file_features_temp_area+32]
    shr   ebx,10
    shr   eax,2
    cmp   ebx,eax
    jb	 .copy_for_one
    shl   eax,10
    mov   ecx,eax
    mov   [read_file.size],eax
    mov   [save_file.size],eax
    mov   [add_save_file.size],eax

    cmp  [copy_many_files], byte 1
    je	.continue_5
    pusha
    mov  eax,[file_features_temp_area+32]
    mov  ebx,[save_file.size]
    xor  edx,edx
    mov  [current_progress_value],edx
    mov  [current_progress_value_1],edx
    div  ebx
    test edx,edx
    jz	 @f
    inc  eax
@@:
    mov  [max_progress_value],eax
    popa
.continue_5:
    call  progress_bar_copy    ;_1
    call  add_application_memory.1
    cmp   [copy_many_files],byte 0
    jne   @f
    inc  [current_progress_value]
    call  progress_bar_copy   ;_1
@@:
    call  load_file
    test  eax,eax
    jnz   .error
    cmp   [copy_many_files],byte 0
    jne   @f
    inc  [current_progress_value_1]
    call  progress_bar_copy   ;_1
@@:
    call  write_file
    test  eax,eax
    jnz   .error
.continue:
    mov   eax,[file_features_temp_area+32]
    mov   ebx,[save_file.size]
    sub   eax,ebx
    cmp   eax,ebx
    jbe   .end_copy_write
    mov   [file_features_temp_area+32],eax
    add   [add_save_file.Offset],ebx
    add   [read_file.Offset],ebx
    cmp   [copy_many_files],byte 0
    jne   @f
    inc  [current_progress_value]
    call  progress_bar_copy   ;_1
@@:
    call  load_file
    test  eax,eax
    jnz   .error_1
    cmp   [copy_many_files],byte 0
    jne   @f
    inc  [current_progress_value_1]
    call  progress_bar_copy   ;_1
@@:
    call  add_write_file
    test  eax,eax
    jnz   .error_1
    jmp   .continue

.end_copy_write:
    mov   [add_save_file.size],eax
    add   [add_save_file.Offset],ebx
    mov   [read_file.size],eax
    add   [read_file.Offset],ebx
    inc  [current_progress_value]
    call  progress_bar_copy
    call  load_file
    test  eax,eax
    jnz   .error_1
    inc  [current_progress_value_1]
    call  progress_bar_copy
    call  add_write_file
    test  eax,eax
    jnz   .error_1
    mov   ecx,[appl_memory]
    sub   ecx,[save_file.size]
    call  sub_application_memory.1
.exit:
    ret


.copy_for_one:
    call  add_application_memory
    mov   eax,[file_features_temp_area+32]
    mov   [read_file.size],eax
    inc  [current_progress_value]
    call  progress_bar_copy
    call  load_file
    test  eax,eax
    jnz   .error
    mov   [save_file.size],ebx
    inc  [current_progress_value_1]
    call  progress_bar_copy
    call  write_file
    test  eax,eax
    jnz   .error
    call  sub_application_memory
    ret

.error:
    call start_error_window_thread
    call  sub_application_memory
    add  esp,4
    mov  [move_file_flag],0
    jmp  editbox_copy_name.exit_without_copy

.error_1:
    call start_error_window_thread
    mov   ecx,[appl_memory]
    sub   ecx,[save_file.size]
    call  sub_application_memory.1
    add  esp,4
    mov  [move_file_flag],0
    jmp  editbox_copy_name.exit_without_copy
;-------------------------------------------------------------
copy_marked_files:
    mov  [copy_many_files],byte 1
    mov  [draw_all_progressbar],1
    xor  eax,eax
    mov  [current_progress_value],eax
    mov  [current_progress_value_1],eax
    cmp  [select_panel_counter],al
    jne  .right_1
    mov  eax,[left_marked_counter]
    mov  [max_progress_value],eax
    mov  ebp,[left_folder_block]
.start:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[left_folder_data]
    add  ebx,32+40
    test [ebx-40],byte 0x10
    jnz  @f
    cmp  [ebx+299-40],byte 0
    je	 @f
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    je	 .continue_1
.continue:
    push  ebx
    mov   esi,read_folder_name
    mov   edi,file_name
    call  copy_path_1
    pop   ebx
    call  copy_file_1.1
    cmp   [move_file_flag],0
    je	  .continue_1
    call  del_file
    test  eax,eax
    jz	  .continue_1
    mov  [move_file_flag],0
    call  start_error_window_thread
    jmp   reload_dir_all
.continue_1:
@@:
    dec  ebp
    jnz  .start
    cmp  [move_file_flag],0
    je	 reload_dir_all
    mov  [left_start_draw_cursor_line],0
    mov  [move_file_flag],0
    mov  [copy_many_files],byte 0
    jmp  reload_dir_all

.right_1:
    mov  eax,[right_marked_counter]
    mov  [max_progress_value],eax
    mov  ebp,[right_folder_block]
.start_1:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[right_folder_data]
    add  ebx,32+40
    test [ebx-40],byte 0x10
    jnz  @f
    cmp  [ebx+299-40],byte 0
    je	 @f
    cmp  [ebx],word '..'
    jne  .continue_2
    cmp  [ebx+2],byte 0
    je	 .continue_3
.continue_2:
    push  ebx
    mov   esi,read_folder_1_name
    mov   edi,file_name
    call  copy_path_1
    pop   ebx
    call copy_file_1.1
    cmp   [move_file_flag],0
    je	  .continue_3
    call  del_file
    test  eax,eax
    jz	  .continue_3
    mov  [move_file_flag],0
    call  start_error_window_thread
    jmp   reload_dir_all
.continue_3:
@@:
    dec  ebp
    jnz  .start_1
    cmp  [move_file_flag],0
    je	 reload_dir_all
    mov  [right_start_draw_cursor_line],0
    mov  [move_file_flag],0
    mov  [copy_many_files],byte 0
    jmp  reload_dir_all
;---------------------------------------------------------------------