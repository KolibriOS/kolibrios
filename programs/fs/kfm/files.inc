
del_file:
   mcall SF_FILE, delete_file_data 
   ret
;---------------------------------------------------------------------
get_file_size:
   mcall SF_FILE, read_file_features
   ret
;---------------------------------------------------------------------
load_file:
   mcall SF_FILE, read_file
   ret
;---------------------------------------------------------------------
write_file:
   mcall SF_FILE, save_file
   ret
;---------------------------------------------------------------------
; out:
;  eax - 0 or error code
align 4
proc_read_left_folder:
   push  ebx ecx edx
   mov   ecx,[lfd_size]
   add   ecx,304
   mcall SF_SYS_MISC, SSF_MEM_REALLOC,, [left_folder_data]
   mov   [left_folder_data],eax
   mov   [lfd_size],ecx
   stdcall mark_file_clear_all, eax, ecx
   pop   edx ecx ebx

   xor   eax,eax
   mov   [left_marked_counter],eax
   mov	 [read_folder.start],eax    ;dword 0
   inc   eax
   mov	 [read_folder.size],eax     ;dword 1
   mov   eax,[left_folder_data]
   mov   [read_folder.return],eax

   mcall SF_FILE, read_folder
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
@@:
   mov   eax,[left_folder_data]
   cmp	 [eax+32+40],word 2Eh
   jne	 @f
   mov	 [read_folder.start],dword 1
@@:
   mov   ecx,[eax+8] ; число файлов и папок
   mov	 [read_folder.size],ecx
   mov	 [sb_left.max_area],ecx
   imul  ecx,304
   add   ecx,32

   push  edx
   mcall SF_SYS_MISC, SSF_MEM_REALLOC,, [left_folder_data]
   mov   [left_folder_data],eax
   mov   [lfd_size],ecx
   mov   [read_folder.return],eax
   stdcall mark_file_clear_all, eax, ecx
   pop   edx

   mcall SF_FILE, read_folder
   mov	 [left_folder_block],ebx
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
align 4
@@:
   push  eax
   call  left_sort_files
   pop   eax
   ret
align 4
.error:
   mov	 [left_folder_block],dword 2
   mov   edi,[left_folder_data]
   call  error_handler
   ret
;---------------------------------------------------------------------
align 4
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
align 4
; out:
;  eax - 0 or error code
proc_read_right_folder:
   push  ebx ecx edx
   mov   ecx,[rfd_size]
   add   ecx,304
   mcall SF_SYS_MISC, SSF_MEM_REALLOC,, [right_folder_data]
   mov   [right_folder_data],eax
   mov   [rfd_size],ecx
   stdcall mark_file_clear_all, eax, ecx
   pop   edx ecx ebx

   xor   eax,eax
   mov   [right_marked_counter],eax
   mov	 [read_folder_1.start],eax      ;dword 0
   inc   eax
   mov	 [read_folder_1.size],eax       ;dword 1
   mov   eax,[right_folder_data]
   mov   [read_folder_1.return],eax
   mcall SF_FILE, read_folder_1
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
@@:
   mov   eax,[right_folder_data]
   cmp	 [eax+32+40],word 2Eh
   jne	 @f
   mov	 [read_folder_1.start],dword 1
@@:
   mov   ecx,[eax+8] ; число файлов и папок
   mov	 [read_folder_1.size],ecx
   mov	 [sb_right.max_area],ecx
   imul  ecx,304
   add   ecx,32

   push  edx
   mcall SF_SYS_MISC, SSF_MEM_REALLOC,, [right_folder_data]
   mov   [right_folder_data],eax
   mov   [rfd_size],ecx
   mov   [read_folder_1.return],eax
   stdcall mark_file_clear_all, eax, ecx
   pop   edx

   mcall SF_FILE, read_folder_1
   mov	 [right_folder_block],ebx
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
align 4
@@:
   push  eax
   call  right_sort_files
   pop   eax
   ret
align 4
.error:
   mov	 [right_folder_block],dword 2
   mov   edi,[right_folder_data]
   call  error_handler
   ret
;---------------------------------------------------------------------
align 4
proc_read_folder:
    call  proc_read_left_folder
    cmp  [search_pre_folder],0
    jne   @f
    mov  [left_start_draw_cursor_line],0
    mov  [left_start_draw_line],0
    mov  [sb_left.position],0
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
    mov  [sb_left.position],0
    jmp  .end_folder
@@:
    mov  [left_start_draw_cursor_line],0
    mov  [left_start_draw_line],ebp
    mov  [sb_left.position],ebp
.end_folder:
    mov  [search_pre_folder],0
.continue:
    call  draw_left_panel
    call  draw_left_select_disk_button
    jmp   still
;---------------------------------------------------------------------
proc_read_folder_1:
    call  proc_read_right_folder
;    mov   [right_panel_clear_all],1
    cmp  [search_pre_folder],0
    jne   @f
    mov  [right_start_draw_cursor_line],0
    mov  [right_start_draw_line],0
    mov  [sb_right.position],0
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
    mov  [sb_right.position],0
    jmp  .end_folder
@@:
    mov  [right_start_draw_cursor_line],0
    mov  [right_start_draw_line],ebp
    mov  [sb_right.position],ebp
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
    .subfunction dd   SSF_READ_FOLDER
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   0
    db 0
    .name: dd read_folder_name
;---------------------------------------------------------------------
read_folder_1:
    .subfunction dd   SSF_READ_FOLDER
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   0
    db 0
    .name: dd read_folder_1_name
;---------------------------------------------------------------------
read_folder_2:
    .subfunction dd   SSF_READ_FOLDER
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   left_folder_data_1
    db 0
    .name: dd read_folder_2_name
;---------------------------------------------------------------------
read_folder_3:
    .subfunction dd   SSF_READ_FOLDER
    .start	 dd   0
    .flags	 dd   0
    .size	 dd   5000
    .return	 dd   right_folder_data_1
    db 0
    .name: dd read_folder_3_name
;---------------------------------------------------------------------
read_file_features:
    .subfunction dd   SSF_GET_INFO
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .return	 dd   file_features_temp_area
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
read_file:
    .subfunction dd   SSF_READ_FILE
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .return	 dd   mem
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
save_file:
    .subfunction dd   SSF_CREATE_FILE
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .pointer	 dd   mem
    db 0
    .name: dd start_parameter
;---------------------------------------------------------------------
add_save_file:
    .subfunction dd   SSF_WRITE_FILE
    .Offset	 dd   0
    .Offset_1	 dd   0
    .size	     dd   0
    .pointer	 dd   mem
    db 0
    .name: dd start_parameter
;---------------------------------------------------------------------
create_dir_file_data:
    .subfunction dd   SSF_CREATE_FOLDER
    .flags	 dd   0
    .param	 dd   0
    .rezerv	 dd   0
    .rezerv1	 dd   0
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
start_file_data:
    .subfunction dd   SSF_START_APP
    .flags	 dd   0
    .param	 dd   0
    .rezerv	 dd   0
    .rezerv1	 dd   0
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
delete_file_data:
    .subfunction dd   SSF_DELETE
    .flags	 dd   0
    .param	 dd   0
    .rezerv	 dd   0
    .rezerv1 dd   0
    db 0
    .name: dd file_name
;---------------------------------------------------------------------
read_delete_dir_data:
    .subfunction dd   SSF_READ_FOLDER
    .start	 dd   0
    .flags	 dd   0
    .size	     dd   0
    .return	 dd   delete_dir_data
    db 0
    .name: dd file_name
;---------------------------------------------------------------------