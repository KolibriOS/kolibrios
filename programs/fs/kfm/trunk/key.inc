key:
    mcall 2
    cmp   [extended_key],1
    je	  .extended_key
    test  al, al
    jnz   still
    cmp   ah, 0xE0
    jne   @f
    mov   [extended_key],1
    jmp   still
@@:
    cmp   ah,15
    jne   .no_tab
    inc   [select_panel_counter]
    and   [select_panel_counter],1b
    call  draw_left_panel
    call  draw_right_panel
    jmp   still
.no_tab:
    cmp   ah,72   ; arrow up
    je	  key_72.1
    cmp   ah,80   ; arrow down
    je	  key_80.1
    cmp   ah,28   ; Enter
    je	  key_28.1
    cmp   ah,17   ; W
    je    set_background
    cmp   ah,19   ; R
    je	  reload_dir_1
    cmp   ah,42
    je	  key_shift_up
    cmp   ah,54
    je	  key_shift_up
    cmp   ah,142
    je	  exit_dir_1	; BackSpace Up
    cmp   ah,170
    je	  key_shift_down
    cmp   ah,182
    je	  key_shift_down
    cmp   ah,29
    je	  key_ctrl_up.1
    cmp   ah,157
    je	  key_ctrl_down.1
    cmp   ah,56
    je	  key_alt_up.1
    cmp   ah,184
    je	  key_alt_down.1
    cmp   ah,206 ; NumPad+ Up
    je    NumPad_plus_Up
    cmp   ah,202 ; NumPad- Up
    je    NumPad_minus_Up
    cmp   ah,183 ; NumPad* Up
    je    NumPad_sneg_Up
    cmp   ah,158
    je    symbol_a_up
    cmp   ah,59    ; down F1
    je	  f_key_f1
    cmp   ah,60    ; down F2
    je	  f_key_f2
    cmp   ah,61    ; down F3
    je	  f_key_f3 ;sort_name
    cmp   ah,62    ; down F4
    je	  f_key_f4 ;sort_type
    cmp   ah,63    ; down F5
    je	  f_key_f5 ;sort_size
    cmp   ah,64    ; down F6
    je	  f_key_f6 ; sort_date
    cmp   ah,65    ; down F7
    je	  f_key_f7 ;
    cmp   ah,66    ; down F8
    je	  f_key_f8 ; delete_file
    cmp   ah,88    ; down F12
    je	  f_key_f12
    jmp  still
;-------------------------------------------------------------
.extended_key:
    mov  [extended_key],0
key_80:
    cmp  ah,80	; arrow down
    jne  key_72
.1:
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  eax,[left_folder_block]
    dec  eax
    imul eax,10
    cmp  ax,[left_start_draw_cursor_line]
    jbe  .marked_action   ;still
    mov  ax,[left_start_draw_cursor_line]
    add  ax,10
    cmp  ax,[left_panel_y_correct_size_y]
    jae  @f
.add_1:
    add  [left_start_draw_cursor_line],10
    call draw_left_panel_selection
    mov  [drawing_panel_folder],1
.all:
    sub  ecx,10 shl 16
    mov  edx,0xffffff
	mcall
    mov  [not_clear_lines],1
    call draw_folder_data
    mov  [not_clear_lines],0
    jmp  still
.marked_action:
    cmp  [marked_action],0
    je   still
    mov  [marked_action],0
    call draw_left_panel
    jmp  still
.marked_action_1:
    cmp  [marked_action],0
    je   still
    mov  [marked_action],0
    call draw_right_panel
    jmp  still
@@:
    mov  eax,[left_folder_block]
    sub  eax,[max_panel_line]
    test eax,0x80000000
    jz	 @f
    xor  eax,eax
@@:
    cmp  [marked_action],0
    jne   @f
    cmp  [left_start_draw_line],eax
    je	 still
@@:
    mov  [marked_action],0
    inc  [left_start_draw_line]
    call draw_left_panel
    jmp  still
.right_1:
    mov  eax,[right_folder_block]
    dec  eax
    imul eax,10
    cmp  ax,[right_start_draw_cursor_line]
    jbe  .marked_action_1   ;still
    mov  ax,[right_start_draw_cursor_line]
    add  ax,10
    cmp  ax,[right_panel_y_correct_size_y]
    jae  @f
.add_2:
    add  [right_start_draw_cursor_line],10
    call draw_right_panel_selection
    mov  [drawing_panel_folder],2
    jmp  key_80.all
@@:
    mov  eax,[right_folder_block]
    sub  eax,[max_panel_line]
    test eax,0x80000000
    jz	 @f
    xor  eax,eax
@@:
    cmp  [marked_action],0
    jne  @f
    cmp  [right_start_draw_line],eax
    je	 still
@@:
    mov  [marked_action],0
    inc  [right_start_draw_line]
    call draw_right_panel
    jmp  still
;-------------------------------------------------------------
key_72:
    cmp  ah,72	;arrow up
    jne  key_81
.1:
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  ax,[left_start_draw_cursor_line]
    add  ax,[left_panel_y.start_y]
    cmp  [left_panel_y.start_y],ax
    je	 @f
    sub  [left_start_draw_cursor_line],10
    call draw_left_panel_selection
    mov  [drawing_panel_folder],1
.all:    
    add  ecx,10 shl 16
    mov  edx,0xffffff
	mcall
    mov  [not_clear_lines],1
    call draw_folder_data
    mov  [not_clear_lines],0
    jmp  still
@@:
    cmp  [left_start_draw_line],0
    je	 still
    dec  [left_start_draw_line]
    call draw_left_panel
    jmp  still
.right_1:
    mov  ax,[right_start_draw_cursor_line]
    add  ax,[right_panel_y.start_y]
    cmp  [right_panel_y.start_y],ax
    je	 @f
    sub  [right_start_draw_cursor_line],10
    call draw_right_panel_selection
    mov  [drawing_panel_folder],2
    jmp  key_72.all
@@:
    cmp  [right_start_draw_line],0
    je	 still
    dec  [right_start_draw_line]
    call draw_right_panel
    jmp  still
;-------------------------------------------------------------
key_81:
    cmp  ah,81 ; PageDown
    jne  key_73
    mov  eax,[max_panel_line]
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  ebx,[left_folder_block]
    sub  ebx,eax
    test ebx,0x80000000
    jnz  .left_2
    sub  ebx,[left_start_draw_line]
    cmp  ebx,eax
    ja  .ok_1
    mov  ebx,[left_folder_block]
    sub  ebx,eax
    call prepare_data_left_down
    ja   @f
    cmp  ebx,[left_start_draw_line]
    je   still
@@:
    mov  [left_start_draw_line],ebx
.left_3:
    dec  eax
    imul eax,10
.left_1:
    mov  [left_start_draw_cursor_line],ax
    call draw_left_panel
    jmp  still
.left_2:
    mov  eax,[left_folder_block]
    sub  eax,[left_start_draw_line]
    dec  eax
    imul eax,10
    cmp  ax,[left_start_draw_cursor_line]
    jbe  still
    jmp  .left_1
.ok_1:
    add  [left_start_draw_line],eax
    call draw_left_panel
    jmp  still
.right_1:
    mov  ebx,[right_folder_block]
    sub  ebx,eax
    test ebx,0x80000000
    jnz  .right_3
    sub  ebx,[right_start_draw_line]
    cmp  ebx,eax
    jae  .ok_2
    mov  ebx,[right_folder_block]
    sub  ebx,eax
    call prepare_data_right_down
    ja   @f
    cmp  ebx,[right_start_draw_line]
    je   still
@@:
    mov  [right_start_draw_line],ebx
.right_4:
    dec  eax
    imul eax,10
.right_2:
    mov  [right_start_draw_cursor_line],ax
    call draw_right_panel
    jmp  still
.right_3:
    mov  eax,[right_folder_block]
    sub  eax,[right_start_draw_line]
    dec  eax
    imul eax,10
    cmp  ax,[right_start_draw_cursor_line]
    jbe  still
    jmp  .right_2
.ok_2:
    add  [right_start_draw_line],eax
    call draw_right_panel
    jmp  still
;-------------------------------------------------------------    
prepare_data_right_down:
    mov  ecx,[right_folder_block]
    sub  ecx,[right_start_draw_line]
    dec  ecx
    imul ecx,10
    cmp  cx,[right_start_draw_cursor_line]
    ret
;-------------------------------------------------------------    
prepare_data_left_down:
    mov  ecx,[left_folder_block]
    sub  ecx,[left_start_draw_line]
    dec  ecx
    imul ecx,10
    cmp  cx,[left_start_draw_cursor_line]
    ret
;-------------------------------------------------------------
key_73:
    cmp  ah,73 ; PageUp
    jne  key_71
    mov  eax,[max_panel_line]
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  ebx,[left_start_draw_line]
    sub  ebx,eax
    test ebx,0x8000000
    jz	 @f
    cmp  [left_start_draw_line],0
    jne  .draw_line_1
    cmp  [left_start_draw_cursor_line],0
    je	 still
    mov  [left_start_draw_cursor_line],0
.draw_line_1:
    mov  [left_start_draw_line],0
    call draw_left_panel
    jmp  still
@@:
    sub  [left_start_draw_line],eax
    call draw_left_panel
    jmp  still
.right_1:
    mov  ebx,[right_start_draw_line]
    sub  ebx,eax
    test ebx,0x80000000
    jz	 @f
    cmp  [right_start_draw_line],0
    jne  .draw_line_2
    cmp  [right_start_draw_cursor_line],0
    je	 still
    mov  [right_start_draw_cursor_line],0
.draw_line_2:
    mov  [right_start_draw_line],0
    call draw_right_panel
    jmp  still
@@:
    sub  [right_start_draw_line],eax
    call draw_right_panel
    jmp  still
;-------------------------------------------------------------
key_71:
    cmp  ah,71 ; Home
    jne  key_79
    cmp  [select_panel_counter],0
    jne  .right_1
    cmp  [left_start_draw_line],0
    jne   .draw_left_panel
    cmp  [left_start_draw_cursor_line],0
    je	 still
.draw_left_panel:
    mov  [left_start_draw_line],0
    mov  [left_start_draw_cursor_line],0
    call draw_left_panel
    jmp  still
.right_1:
    cmp  [right_start_draw_line],0
    jne   .draw_right_panel
    cmp  [right_start_draw_cursor_line],0
    je	 still
.draw_right_panel:
    mov  [right_start_draw_line],0
    mov  [right_start_draw_cursor_line],0
    call draw_right_panel
    jmp  still
;-------------------------------------------------------------
key_79:
    cmp  ah,79 ; End
    jne  key_28
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  eax,[left_folder_block]
    sub  eax,[max_panel_line]
    test eax,0x80000000
    jnz  key_81.left_2
    call prepare_data_left_down
    ja   @f
    cmp  eax,[left_start_draw_line]
    je   still
@@:
    mov  [left_start_draw_line],eax
    mov  eax,[max_panel_line]
    jmp  key_81.left_3
.right_1:
    mov  eax,[right_folder_block]
    sub  eax,[max_panel_line]
    test eax,0x80000000
    jnz  key_81.right_3
    call prepare_data_right_down
    ja   @f
    cmp  eax,[right_start_draw_line]
    je   still
@@:
    mov  [right_start_draw_line],eax
    mov  eax,[max_panel_line]
    jmp  key_81.right_4    
;-------------------------------------------------------------
key_28:
    cmp  ah,28 ; Enter
    jne  key_ctrl_up
.1:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 still
    cmp  [alt_flag],1
    je	 files_information
    
    mov  [open_param],0
.2:
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  ebp,read_folder_name    ;read_folder.name
    call  calculate_left_data_1
    add  esi,32
    cmp  [exit_dir],1
    je	 .up_patch
    test [esi],byte 0x10
    jz	 file_no_folder
    cmp  [open_param],0
    jne  still
    cmp  [esi+40],word 2Eh
    je	 still
    cmp  [esi+40],word '..'
    je	 .up_patch
    call calculate_key_28_1
    mov  [left_start_draw_cursor_line],0
    mov  [left_start_draw_line],0
    jmp  proc_read_folder
.up_patch:
    call calculate_key_28_2
    jmp  proc_read_folder
.right_1:
    mov  ebp,read_folder_1_name   ;read_folder_1.name
    call calculate_right_data_1
    add  esi,32
    cmp  [exit_dir],1
    je	 .up_patch_1
    test [esi],byte 0x10
    jz	 file_no_folder
    cmp  [esi+40],word 2Eh
    je	 still
    cmp  [esi+40],word '..'
    je	 .up_patch_1
    call calculate_key_28_1
    mov  [right_start_draw_cursor_line],0
    mov  [right_start_draw_line],0
    jmp  proc_read_folder_1
.up_patch_1:
    call calculate_key_28_2
    jmp  proc_read_folder_1
;---------------------------------------------------------------------
calculate_key_28_1:    
;    mov  edi,read_folder.name-1
    mov  edi,ebp
    dec  edi
    inc  ebp
@@:
    inc  edi
    cmp  [edi],byte 0
    jne  @r
    cmp  edi,ebp ;read_folder.name+1
    je	 @f
    mov  al,'/'
    mov [edi],al
    inc  edi
@@:
    add  esi,40
@@:
    cld
    lodsb
    stosb
    cmp al,0
    jnz   @r
    ret
;---------------------------------------------------------------------
calculate_key_28_2:
    mov  [exit_dir],0
    mov  edi,ebp ;read_folder.name
@@:
    inc  edi
    cmp  [edi],byte 0
    jne  @r
    mov  [extension_size],edi
    dec  edi
@@:
    dec  edi
    cmp  [edi],byte '/'
    jne  @r
    mov  eax,ebp ;read_folder.name
    cmp  edi,eax
    jbe  @f
    mov  [edi],byte 0
@@:
    inc  edi
    mov  [temp_counter_dword],edi
    dec  edi
    sub  [extension_size],edi
    mov  [search_pre_folder],1
    ret
;---------------------------------------------------------------------
set_background:
    cmp  [shift_flag],1
    je   .shift
    cmp  [ctrl_flag],1
    jne  still
    mov  [open_param],background_stretch
    jmp  key_28.2
.shift:
    mov  [open_param],background_tile
    jmp  key_28.2    
;---------------------------------------------------------------------    
reload_dir_1:
    cmp  [ctrl_flag],0
    je	 still
    jmp  reload_dir
;---------------------------------------------------------------------
symbol_a_up:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  still
.ctrl:
    mov  [mark_action_counter],0
    jmp  mark_all_file
;---------------------------------------------------------------------
NumPad_plus_Up:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  still
.ctrl:
    mov  [mark_action_counter],0
    jmp  mark_all_file
;---------------------------------------------------------------------
NumPad_minus_Up:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  still
.ctrl:
    mov  [mark_action_counter],1
    jmp  mark_all_file
;---------------------------------------------------------------------
NumPad_sneg_Up:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  still
.ctrl:
    mov  [mark_action_counter],2
    jmp  mark_all_file
;---------------------------------------------------------------------
f_key_f1:
;sort_name:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 still
    cmp  [alt_flag],1
    je	 draw_left_select_disk_menu
    jmp  help_start
;---------------------------------------------------------------------
f_key_f2:
;sort_name:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 still
    cmp  [alt_flag],1
    je	 draw_right_select_disk_menu
    jmp  still
;---------------------------------------------------------------------
f_key_f3:
;sort_name:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  still
.ctrl:
    cmp  [select_panel_counter],1
    je	 right_sort_name
    jmp  left_sort_name
;---------------------------------------------------------------------
f_key_f4:
;sort_type:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  open_file_f4
.ctrl:
    cmp  [select_panel_counter],1
    je	 right_sort_type
    jmp  left_sort_type
;---------------------------------------------------------------------
f_key_f5:
;sort_size:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  copy_file_f5
.ctrl:
    cmp  [select_panel_counter],1
    je	 right_sort_size
    jmp  left_sort_size
;---------------------------------------------------------------------
f_key_f6:
;sort_date:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 .ctrl
    cmp  [alt_flag],1
    je	 still
    jmp  move_file_f6
.ctrl:
    cmp  [select_panel_counter],1
    je	 right_sort_date
    jmp  left_sort_date
;---------------------------------------------------------------------
f_key_f7:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 still
    cmp  [alt_flag],1
    je	 still
    jmp  create_directory
;---------------------------------------------------------------------
f_key_f8:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 still
    cmp  [alt_flag],1
    je	 still
    jmp  delete_file
;---------------------------------------------------------------------
f_key_f12:
    cmp  [shift_flag],1
    je	 still
    cmp  [ctrl_flag],1
    je	 still
    cmp  [alt_flag],1
    je	 still
    jmp  exit_apl
;---------------------------------------------------------------------
key_shift_up:
    cmp  [shift_flag],1
    je	 still
    mov  [shift_flag],1
    jmp  key_alt_down.for_all
;---------------------------------------------------------------------
key_shift_down:
    mov  [shift_flag],0
    jmp  key_alt_down.for_all
;---------------------------------------------------------------------
key_ctrl_up:
    cmp   ah,29
    jne   key_ctrl_down
.1:
    cmp  [ctrl_flag],1
    je	 still
    mov  [ctrl_flag],1
    jmp  key_alt_down.for_all
;---------------------------------------------------------------------
key_ctrl_down:
    cmp   ah,157
    jne   key_alt_up
.1:
    mov  [ctrl_flag],0
    jmp  key_alt_down.for_all
;---------------------------------------------------------------------
key_alt_up:
    cmp   ah,56
    jne   key_alt_down
.1:
    cmp  [alt_flag],1
    je	 still
    mov  [alt_flag],1
    jmp  key_alt_down.for_all
;---------------------------------------------------------------------
key_alt_down:
    cmp   ah,184
    jne   key_insert
.1:
    mov  [alt_flag],0
.for_all:
    call erase_fbutton
    call draw_fbutton
    jmp  still
;---------------------------------------------------------------------
key_insert:
    cmp   ah,82   ; Insert
    je	  mark_file
    cmp   ah,83
    je	  f_key_f8	;delete_file
key_end:
    jmp  still
;---------------------------------------------------------------------