read_delete_dir:
   mcall 70, read_delete_dir_data
   ret
;---------------------------------------------------------------------
create_directory_f70:
   mcall 70, create_dir_file_data 
   ret
;---------------------------------------------------------------------
del_file:
   mcall 70, delete_file_data 
   ret
;---------------------------------------------------------------------
start_appl:
   mcall 70, start_file_data
   ret
;---------------------------------------------------------------------
get_file_size:
   mcall 70, read_file_features
   ret
;---------------------------------------------------------------------
load_file:
   mcall 70, read_file
   ret
;---------------------------------------------------------------------
write_file:
   mcall 70, save_file
   ret
;---------------------------------------------------------------------
add_write_file:
   mcall 70, add_save_file
   ret
;---------------------------------------------------------------------
proc_read_left_folder:
   xor   eax,eax
   mov   [left_marked_counter],eax
   mov	 [read_folder.start],eax    ;dword 0
   inc   eax
   mov	 [read_folder.size],eax     ;dword 1
   mov   eax,[left_folder_data]
   push  eax
   mov   [read_folder.return],eax
   mcall 70, read_folder
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error_1
@@:
   mov   eax,[esp]
   cmp	 [eax+32+40],word 2Eh
   jne	 @f
   mov	 [read_folder.start],dword 1
@@:
   mov   ebx,[eax+8]
   mov	 [read_folder.size],ebx  ;dword 10  ;5000
   imul  ebx,304
   add   ebx,32
   mov   ecx,[right_folder_data]
   sub   ecx,[left_folder_data]
   push  ebx ecx
   cmp   ebx,ecx
   je    .continue
   sub   ebx,ecx
   js    .sub_mem
.add_mem:
   mov   ecx,[appl_memory]
   add   ecx,ebx
   mov   [esp],ebx
   mov   ebx,[appl_memory]
   mov   [esp+4],ebx
   mov   [appl_memory],ecx
   mcall 64,1
   call  move_up_right_table
;   mov   [temp_znak],1
   jmp   .continue
.sub_mem:
   mov   ebx,[esp]
   mov   ecx,[esp+4]
   sub   ebx,ecx
   mov   ecx,[appl_memory]
   sub   ecx,ebx
   mov   [esp],ebx
   mov   ebx,[appl_memory]
   mov   [esp+4],ebx
   mov   [appl_memory],ecx
   call  move_down_right_table
   mcall 64,1
;   mov   [temp_znak],2
.continue:
;   mov	 edi,eax  ;folder_data
;   call  clear_folder_data_area
   mcall 70, read_folder
   mov	 [left_folder_block],ebx
   add   esp,4*3
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
@@:
   push  eax
;   call  left_files_name_normalize
   call  left_prepare_extension_start
   call  left_sort_files
   pop   eax
;   pusha
;   call  draw_window
;   popa
   ret
.error_1:
   add   esp,4
.error:
   mov	 [left_folder_block],dword 2
   mov   edi,[left_folder_data]
   call  error_handler
   ret
;---------------------------------------------------------------------
error_handler:
   push  eax
   xor   eax,eax
   add   edi,32+40
   push  edi
   sub   edi,40
   mov   ecx,304/4
   cld
   rep   stosd
   pop   edi
   mov   [edi-40],byte 0x10
   mov   [edi],word '..'
   mov   [edi+2],al
   add   edi,304
   push  edi
   sub   edi,40
   mov   ecx,304/4
   cld
   rep   stosd
   pop   edi
   mov   eax,[esp]
   mov   [edi+32-40],eax
   mov   esi,error_type
   mov   ecx,19
   cld
   rep   movsb
   pop   eax
   ret
;---------------------------------------------------------------------
proc_read_right_folder:
   xor   eax,eax
   mov   [right_marked_counter],eax
   mov	 [read_folder_1.start],eax      ;dword 0
   inc   eax
   mov	 [read_folder_1.size],eax       ;dword 1
   mov   eax,[right_folder_data]
   push  eax
   mov   [read_folder_1.return],eax
   mcall 70, read_folder_1
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error_1
@@:
   mov   eax,[esp]
   cmp	 [eax+32+40],word 2Eh
   jne	 @f
   mov	 [read_folder_1.start],dword 1
@@:
   mov   ebx,[eax+8]
   mov	 [read_folder_1.size],ebx  ;dword 10  ;5000
   imul  ebx,304
   add   ebx,32
   mov   ecx,[appl_memory]
   sub   ecx,[right_folder_data]
   push  ebx ecx
   cmp   ebx,ecx
   je    .continue
   sub   ebx,ecx
   js    .sub_mem
.add_mem:
   mov   ecx,[appl_memory]
   add   ecx,ebx
   mov   [appl_memory],ecx
   mcall 64,1
;   mov   [temp_znak],1
   jmp   .continue
.sub_mem:
   mov   ebx,[esp]
   mov   ecx,[esp+4]
   sub   ebx,ecx
   mov   ecx,[appl_memory]
   sub   ecx,ebx
   mov   [appl_memory],ecx
   mcall 64,1
;   mov   [temp_znak],2
.continue:
;   mov	 edi,eax  ;folder_data_1
;   call  clear_folder_data_area
   mcall 70, read_folder_1
   mov	 [right_folder_block],ebx
   add   esp,4*3
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
@@:
   push  eax
;   call  right_files_name_normalize
   call  right_prepare_extension_start
   call  right_sort_files
   pop   eax
;   pusha
;   call  draw_window
;   popa
   ret
.error_1:
   add   esp,4
.error:
   mov	 [right_folder_block],dword 2
   mov   edi,[right_folder_data]
   call  error_handler
   ret
;---------------------------------------------------------------------
move_up_right_table:
   mov  esi,[esp+8]
   mov  ecx,esi
   sub  ecx,[right_folder_data]
   mov  edi,esi
   add  edi,[esp+4]
   shr  ecx,2
   std
;   rep  movsb
   rep  movsd
   mov  eax,[esp+4]
   add  [right_folder_data],eax
   ret
;---------------------------------------------------------------------
move_down_right_table:
   mov  esi,[right_folder_data]
   mov  edi,esi
   sub  edi,[esp+4]
   push ecx
   mov  ecx,[esp+12]
   sub  ecx,[right_folder_data]
   shr  ecx,2
   cld
;   rep  movsb
   rep  movsd
   pop  ecx
   mov  eax,[esp+4]
   sub  [right_folder_data],eax
   ret
;---------------------------------------------------------------------
;clear_folder_data_area:
;   mov ecx,4096*10/4
;   xor eax,eax
;   cld
;   rep stosd
;   ret
;---------------------------------------------------------------------
proc_read_folder:
    call  proc_read_left_folder
;    mov   [left_panel_clear_all],1
    cmp  [search_pre_folder],0
    jne   @f
    mov  [left_start_draw_cursor_line],0
    mov  [left_start_draw_line],0    
    jmp  .continue
@@:
    mov  edi,[left_folder_data]
    add  edi,32+40
    sub  edi,304
    xor  ebp,ebp
.search_file:
    cmp   [left_folder_block],ebp
    jbe   .end_folder
    mov   esi,[temp_counter_dword]
    mov   ecx,[extension_size]
    add   edi,304
    inc   ebp
    cld
;    mov   [temp_counter_dword_1],edi
    push   edi
    rep  cmpsb
    pop    edi
    jne   .search_file
    dec  ebp
    mov  eax,[left_folder_block]
    cmp  [max_panel_line],eax
    js  @f
    mov  eax,ebp
    imul eax,10
    mov  [left_start_draw_cursor_line],ax
    mov  [left_start_draw_line],0
    jmp  .end_folder
@@:
;    sub  eax,ebp
;    push eax
;    sub  eax,[max_panel_line]
;    jns  @f
;    mov  eax,[max_panel_line]
;    sub  eax,[esp]
;    imul eax,10
;    mov  [left_start_draw_cursor_line],ax
;@@:
    mov  [left_start_draw_cursor_line],0
    mov  [left_start_draw_line],ebp
.end_folder:
    mov  [search_pre_folder],0
.continue:
    call  draw_left_panel
    call  draw_left_select_disk_button
;    call  draw_window
;    pop   eax
    jmp   still
;---------------------------------------------------------------------
proc_read_folder_1:
    call  proc_read_right_folder
;    mov   [right_panel_clear_all],1
    cmp  [search_pre_folder],0
    jne   @f
    mov  [right_start_draw_cursor_line],0
    mov  [right_start_draw_line],0    
    jmp  .continue
@@:
    mov  edi,[right_folder_data]
    add  edi,32+40
    sub  edi,304
    xor  ebp,ebp
.search_file:
    cmp   [right_folder_block],ebp
    jbe   .end_folder
    mov   esi,[temp_counter_dword]
    mov   ecx,[extension_size]
    add   edi,304
    inc   ebp
    cld
;    mov   [temp_counter_dword_1],edi
    push   edi
    rep  cmpsb
    pop    edi
    jne   .search_file
    dec  ebp
    mov  eax,[right_folder_block]
    cmp  [max_panel_line],eax
    js  @f
    mov  eax,ebp
    imul eax,10
    mov  [right_start_draw_cursor_line],ax
    mov  [right_start_draw_line],0
    jmp  .end_folder
@@:
;    sub  eax,ebp
;    push eax
;    sub  eax,[max_panel_line]
;    jns  @f
;    mov  eax,[max_panel_line]
;    sub  eax,[esp]
;    imul eax,10
;    mov  [right_start_draw_cursor_line],ax
;@@:
    mov  [right_start_draw_cursor_line],0
    mov  [right_start_draw_line],ebp
.end_folder:
    mov  [search_pre_folder],0
.continue:
    call  draw_right_panel
    call  draw_right_select_disk_button
    jmp   still
;---------------------------------------------------------------------
copy_folder_name:
    mov  edi,read_folder_name+1
.1:
    cld
@@:
    lodsb
    stosb
    cmp al,0
    jne  @r
    ret
;---------------------------------------------------------------------
copy_folder_name_1:
    mov  edi,read_folder_1_name+1
    jmp  copy_folder_name.1
;---------------------------------------------------------------------
copy_folder_name_2:
    mov  edi,read_folder_2_name+1
    jmp  copy_folder_name.1
;---------------------------------------------------------------------
copy_folder_name_3:
    mov  edi,read_folder_3_name+1
    jmp  copy_folder_name.1
;---------------------------------------------------------------------
read_folder:
    .subfunction dd   1
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   0
    db 0
    .name: dd read_folder_name
;---------------------------------------------------------------------
read_folder_1:
    .subfunction dd   1
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   0
    db 0
    .name: dd read_folder_1_name
;---------------------------------------------------------------------
read_folder_2:
    .subfunction dd   1
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   left_folder_data_1
    db 0
    .name: dd read_folder_2_name
;---------------------------------------------------------------------
read_folder_3:
    .subfunction dd   1
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   right_folder_data_1
    db 0
    .name: dd read_folder_3_name
;---------------------------------------------------------------------
read_file_features:
    .subfunction dd   5
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .return	 dd   file_features_temp_area
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
read_file:
    .subfunction dd   0
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .return	 dd   mem
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
save_file:
    .subfunction dd   2
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .pointer	 dd   mem
    db 0
    .name: dd start_parameter
;---------------------------------------------------------------------
add_save_file:
    .subfunction dd   3
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .pointer	 dd   mem
    db 0
    .name: dd start_parameter
;---------------------------------------------------------------------
create_dir_file_data:
    .subfunction dd   9
    .flags	 dd   0
    .param	 dd   0
    .rezerv	 dd   0
    .rezerv1	 dd   0
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
start_file_data:
    .subfunction dd   7
    .flags	 dd   0
    .param	 dd   0
    .rezerv	 dd   0
    .rezerv1	 dd   0
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
delete_file_data:
    .subfunction dd   8
    .flags	 dd   0
    .param	 dd   0
    .rezerv	 dd   0
    .rezerv1 dd   0
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
read_delete_dir_data:
    .subfunction dd   1
    .start	 dd   0
    .flags	 dd   0
    .size	     dd   0
    .return	 dd   delete_dir_data
    db 0
    .name: dd file_name
;---------------------------------------------------------------------