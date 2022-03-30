;---------------------------------------------------------------------
delete_file:
;    call progress_bar
;    jmp  still
    mov  [confirmation_type],delete_type
    call confirmation_action
    cmp  [work_confirmation_yes],1
    jne  red  ;reload_dir

    xor  eax,eax
    mov  [files_counter],eax
    mov  [directories_counter],eax
    mov  [files_size.low],eax
    mov  [files_size.high],eax
    call get_files_information

    call delete_marked_files
    cmp  [delete_marked_yes],1
    je	 .end_1   ;reload_dir_all

    xor  eax,eax
    xor  edx,edx
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  ax,[left_start_draw_cursor_line]
    mov  ebx,10
    div  ebx
    mov  ebx,[left_start_draw_line]
    add  ebx,eax
    imul ebx,304
    add  ebx,[left_folder_data]
    call prepare_data_7
    mov   esi,read_folder_name	  ;read_folder.name
    jmp  .end
.right_1:
    mov  ax,[right_start_draw_cursor_line]
    mov  ebx,10
    div  ebx
    mov  ebx,[right_start_draw_line]
    add  ebx,eax
    imul ebx,304
    add  ebx,[right_folder_data]
    call prepare_data_7
    mov   esi,read_folder_1_name   ;read_folder_1.name
.end:
    mov   [error_pointer],ebx
    mov   edi,file_name   ;delete_file_data.name
    call  copy_path_1
    test [ebx-40],byte 0x10
    jz	 @f
    push eax ebx
    xor  eax,eax
    mov  [current_progress_value],eax
;    mov  [delete_marked_yes],al
    mov  [draw_all_progressbar],1
    call delete_dir
    pop  ebx eax
    jmp  .end_1
@@:
    call del_file
    test  eax,eax
    jz	 .end_1
    call start_error_window_thread
    jmp  reload_dir_all
.end_1:
    cmp  [select_panel_counter],0
    jne  .right
    call proc_read_left_folder
    jmp  red
.right:
    call proc_read_right_folder
    jmp  red
;---------------------------------------------------------------------
delete_marked_files:
    mov  eax,[directories_counter]
    add  eax,[files_counter]
    mov  [max_progress_value],eax
    
    xor  eax,eax
    mov  [current_progress_value],eax
    mov  [delete_marked_yes],al
    mov  [draw_all_progressbar],1
    
   cmp  [select_panel_counter],al
    jne  .right_1
;    mov  eax,[left_marked_counter]
;    mov  [max_progress_value],eax
    mov  ebp,[left_folder_block]
.start:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[left_folder_data]
    add  ebx,32+40
    cmp  [ebx+299-40],byte 0
    je	 @f
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    je	 .continue_1
.continue:
    mov  esi,read_folder_name	;read_folder.name
    call prepare_data_6
.continue_1:
    mov  [delete_marked_yes],1
@@:
    dec  ebp
    jnz  .start
    cmp  [delete_marked_yes],0
    je	 @f
    mov  [left_start_draw_cursor_line],0
@@:
    ret
.right_1:
;    mov  eax,[right_marked_counter]
;    mov  [max_progress_value],eax
    mov  ebp,[right_folder_block]
.start_1:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[right_folder_data]
    add  ebx,32+40
    cmp  [ebx+299-40],byte 0
    je	 @f
    cmp  [ebx],word '..'
    jne  .continue_2
    cmp  [ebx+2],byte 0
    je	 .continue_3
.continue_2:
    mov   esi,read_folder_1_name   ;read_folder_1.name
    call prepare_data_6
.continue_3:
    mov  [delete_marked_yes],1
@@:
    dec  ebp
    jnz  .start_1
    cmp  [delete_marked_yes],0
    je	 @f
    mov  [right_start_draw_cursor_line],0
@@:
    ret
;---------------------------------------------------------------------
prepare_data_6:
    mov   [error_pointer],ebx
    mov  edi,file_name	 ;delete_file_data.name
    call copy_path_1
;    inc  [current_progress_value]
;    call progress_bar_delete
    test [ebx-40],byte 0x10
    jz	 @f
    call delete_dir
    jmp  .continue
@@:
    inc  [current_progress_value]
    call progress_bar_delete
    call del_file
    test eax,eax
    jz	 .continue  ;  .continue_1
    call start_error_window_thread
    add  esp,8
.continue:
    ret
;---------------------------------------------------------------------
prepare_data_7:
    add  ebx,32+40
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    jne  .continue
    add  esp,4
    jmp  reload_dir
.continue:
    ret
;---------------------------------------------------------------------
delete_dir:
    pusha
    mov   [recursive_pointer],0
    mov   eax,[appl_memory]
    mov   [appl_memory_old],eax
.1:
    mov  [read_delete_dir_data.size],0
    mov  [read_delete_dir_data.return],delete_dir_data
    call read_delete_dir
    test eax,eax
    jz	 @f
    mov   [error_pointer],file_name
    call start_error_window_thread
    popa
    add  esp,8+4
    jmp  still
@@:
    mov  eax,[delete_dir_data+8]
    test eax,eax
    jnz  @f
    inc  [current_progress_value]
    call progress_bar_delete
    call del_file
    test eax,eax
    jz	 .end
    mov   [error_pointer],file_name
    call start_error_window_thread
    popa
    add  esp,8+4
    jmp  still
@@:
    imul  eax,304
    add   eax,32
    mov   ebx,eax
    add   ebx,[appl_memory_old]
    cmp   ebx,[appl_memory]
    jbe   @f
    mov   ecx,[appl_memory]
    mov   ecx,ebx
    mov   [appl_memory],ecx
    mcall 64,1
@@:
    mov  eax,[delete_dir_data+8]
    mov  [read_delete_dir_data.size],eax
    mov   eax,[appl_memory_old]
    mov  [read_delete_dir_data.return],eax
    call read_delete_dir
    test eax,eax
    jz	 @f
    mov   [error_pointer],file_name
    call start_error_window_thread
    popa
    add  esp,8+4
    jmp  still
@@:
    call delete_dir_1
    jc	.1
.end:
    call  return_delete_path
    dec   [recursive_pointer]
    cmp   [recursive_pointer],0
    jge	  .1
@@:
    mov   ecx,[appl_memory_old]
    cmp   [appl_memory],ecx
    je	  @f
    mov   [appl_memory],ecx
    mcall 64,1
@@:
    popa
    ret
;---------------------------------------------------------------------
delete_dir_1:
    mov  ebp,[appl_memory_old]
    mov  ebp,[ebp+8]
.start:
    test ebp,ebp
    jz   .continue_4
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[appl_memory_old]
    add  ebx,32+40
    cmp  [ebx],byte '.'
    jne  @f
    cmp  [ebx+1],byte 0
    je	 .continue_4
@@:
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    je	 .continue_4
.continue:
    mov  esi,file_name
    inc  [recursive_pointer]
    call copy_path_2
    test [ebx-40],byte 0x10
    jnz  .continue_3
@@:
    inc  [current_progress_value]
    call progress_bar_delete
    call del_file
    test eax,eax
    jz	 .continue_1
    mov   [error_pointer],file_name
    call start_error_window_thread
    add  esp,4
    popa
    add  esp,8+4
    ret
.continue_1:
    dec  [recursive_pointer]
    call return_delete_path
    dec  ebp
    jnz  .start
    
.continue_4:
    inc  [current_progress_value]
    call progress_bar_delete
    call del_file
    test eax,eax
    jz	 .continue_2
    call start_error_window_thread
    add  esp,4
    popa
    add  esp,8+4
    ret
.continue_2:
    clc
    ret
.continue_3:
    stc
    ret
;---------------------------------------------------------------------
return_delete_path:
    mov  edi,file_name
@@:
    inc  edi
    cmp  [edi],byte 0
    jne  @r
    dec  edi
@@:
    dec  edi
    cmp  [edi],byte '/'
    jne  @r
    cmp  edi,file_name
    jbe  @f
    mov  [edi],byte 0
@@:
    ret
;---------------------------------------------------------------------
copy_path_2:
    xor   eax,eax
@@:
    cld
    lodsb
    test  al,al
    jnz   @b
    mov   edi,esi
    mov   [edi-1],byte '/'
    mov   esi,ebx
@@:
    cld
    lodsb
    stosb
    test  al,al
    jnz   @b
    ret
;---------------------------------------------------------------------