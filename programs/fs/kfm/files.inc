
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
proc p_read_folder uses ebx ecx edx esi edi, fb_panel:dword
locals
   sort_flag dd ?
   data_size dd ?
   sb_max_area dd ?
endl
   mov   edi,[fb_panel]
   cmp   edi,fb_left
   jne   @f
   mov   esi,read_folder
   mov   eax,[left_sort_flag]
   mov   ebx,lfd_size
   mov   [sb_max_area],sb_left.max_area
   jmp   .end
@@:
   mov   esi,read_folder_1
   mov   eax,[right_sort_flag]
   mov   ebx,rfd_size
   mov   [sb_max_area],sb_right.max_area
.end:
   mov   [sort_flag],eax
   mov   [data_size],ebx

   push  ebx ecx
   mov   ecx,[data_size]
   mov   ecx,[ecx]
   add   ecx,304
   mcall SF_SYS_MISC, SSF_MEM_REALLOC,, [edi+FileBrowser.folder_data]
   mov   [edi+FileBrowser.folder_data],eax
   mov   ebx,[data_size]
   mov   [ebx],ecx
   stdcall mark_file_clear_all, eax, ecx
   pop   ecx ebx
   xor   eax,eax
   mov   [edi+FileBrowser.marked_counter],eax
   mov	 [esi+FileInfoBlock.Position],eax    ;dword 0
   inc   eax
   mov	 [esi+FileInfoBlock.Count],eax     ;dword 1
   mov   eax,[edi+FileBrowser.folder_data]
   mov   [esi+FileInfoBlock.Buffer],eax

   mcall SF_FILE, esi
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
@@:
   mov   eax,[edi+FileBrowser.folder_data]
   cmp   word[eax+32+FileInfoA.FileName],2Eh
   jne   @f
   mov   [esi+FileInfoBlock.Position],dword 1
@@:
   mov   ecx,[eax+FileInfoHeader.FilesCount]
   mov   [esi+FileInfoBlock.Count],ecx
   imul  ecx,304
   add   ecx,32

   mcall SF_SYS_MISC, SSF_MEM_REALLOC,, [edi+FileBrowser.folder_data]
   mov   [edi+FileBrowser.folder_data],eax
   mov   ebx,[data_size]
   mov   [ebx],ecx
   mov   [esi+FileInfoBlock.Buffer],eax
   stdcall mark_file_clear_all, eax, ecx

    mcall SF_FILE, esi
    mov   edx,[sb_max_area]
    mov   [edx],ebx
    mov   [edi+FileBrowser.folder_block],ebx
    test  eax,eax
    jz    @f
    cmp   eax,6
    jne   .error
align 4
@@:
   push  eax
   cmp   [edi+FileBrowser.folder_block], 0
   jz    @f
   stdcall sort_directory, [edi+FileBrowser.folder_data], [sort_flag]
@@:
   pop   eax
   ret
align 4
.error:
   mov   [edi+FileBrowser.folder_block],dword 2
   mov   edi,[edi+FileBrowser.folder_data]
   call  error_handler
   ret
 endp
;---------------------------------------------------------------------
align 4
error_handler:
   push  eax
   xor   eax,eax
   add   edi,32+FileInfoA.FileName
   push  edi
   sub   edi,FileInfoA.FileName
   mov   ecx,304/4
   cld
   rep   stosd
   pop   edi
   mov   [edi-FileInfoA.FileName],byte FA_FOLDER
   mov   [edi],word '..'
   mov   [edi+2],al
   add   edi,304
   push  edi
   sub   edi,FileInfoA.FileName
   mov   ecx,304/4
   cld
   rep   stosd
   pop   edi
   mov   eax,[esp]
   mov   dword[edi+32-FileInfoA.FileName],eax
   mov   esi,error_type
   mov   ecx,19
   cld
   rep   movsb
   pop   eax
   ret
;---------------------------------------------------------------------
; in:
; out:
align 4
proc p_read_folder_1 uses ebx ecx edx esi edi, fb_panel:dword
locals
    sb_position dd ?
endl
    mov  edx,[fb_panel]
    cmp  edx,fb_left
	jne  @f
	mov  eax,sb_left.position
	jmp  .end0
@@:
	mov  eax,sb_right.position
.end0:
    mov [sb_position],eax

    stdcall p_read_folder,edx
    cmp  [search_pre_folder],0
    jne  @f
    mov  [edx+FileBrowser.start_draw_cursor_line],0
    mov  [edx+FileBrowser.start_draw_line],0
    mov  eax,[sb_position]
	mov  dword[eax],0
    jmp  .continue
@@:
    mov  edi,[edx+FileBrowser.folder_data]
    add  edi,32+FileInfoA.FileName
    sub  edi,304
    xor  ebx,ebx
.search_file:
    cmp   [edx+FileBrowser.folder_block],ebx
    jbe   .end_folder
    mov   esi,[temp_counter_dword]
    mov   ecx,[extension_size]
    add   edi,304
    inc   ebx
    cld
    push   edi
    rep  cmpsb
    pop    edi
    jne   .search_file
    dec  ebx
    mov  eax,[edx+FileBrowser.folder_block]
    cmp  [edx+FileBrowser.max_panel_line],eax
    js  @f
    movzx eax,[edx+FileBrowser.line_size_y]
    imul eax,ebx
    mov  [edx+FileBrowser.start_draw_cursor_line],ax
    mov  [edx+FileBrowser.start_draw_line],0
    mov  eax,[sb_position]
	mov  dword[eax],0
    jmp  .end_folder
@@:
    mov  [edx+FileBrowser.start_draw_cursor_line],0
    mov  [edx+FileBrowser.start_draw_line],ebx
    mov  eax,[sb_position]
	mov  [eax],ebx
.end_folder:
    mov  [search_pre_folder],0
.continue:
    cmp  edx,fb_left
	jne  @f
    call  draw_left_panel
	jmp  .end1
@@:
    call  draw_right_panel
.end1:
    stdcall draw_select_disk_button,[fb_panel] ;edx
    ret
endp
;---------------------------------------------------------------------
align 4
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