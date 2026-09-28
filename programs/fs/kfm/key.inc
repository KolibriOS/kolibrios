align 4
key:
    mcall SF_GET_KEY
    cmp   [extended_key],1
    je    .extended_key
    test  al, al
    jnz   still
    cmp   ah, 0xE0
    jne   @f
    mov   [extended_key],1
    jmp   still
@@:
    cmp   ah,15
    jne   .no_tab
    xor   [fb_left.select_panel_counter],1
    xor   [fb_right.select_panel_counter],1
    call  draw_left_panel
    call  draw_right_panel
    jmp   still
.no_tab:
    cmp   ah,72   ; arrow up
    je    key_72.1
    cmp   ah,80   ; arrow down
    je    key_80.1
    cmp   ah,28   ; Enter
    je    key_28.1
    cmp   ah,17   ; W
    je    set_background
    cmp   ah,19   ; R
    je    reload_dir_1
    cmp   ah,42
    je    key_shift_up
    cmp   ah,54
    je    key_shift_up
    cmp   ah,142
    je    exit_dir_1    ; BackSpace Up
    cmp   ah,170
    je    key_shift_down
    cmp   ah,182
    je    key_shift_down
    cmp   ah,29
    je    key_ctrl_up.1
    cmp   ah,157
    je    key_ctrl_down.1
    cmp   ah,56
    je    key_alt_up.1
    cmp   ah,184
    je    key_alt_down.1
    cmp   ah,206 ; NumPad+ Up
    je    NumPad_plus_Up
    cmp   ah,202 ; NumPad- Up
    je    NumPad_minus_Up
    cmp   ah,183 ; NumPad* Up
    je    NumPad_sneg_Up
    cmp   ah,158
    je    symbol_a_up
    cmp   ah,59    ; down F1
    je    f_key_f1
    cmp   ah,60    ; down F2
    je    f_key_f2
    cmp   ah,61    ; down F3
    je    f_key_f3 ;sort_name
    cmp   ah,62    ; down F4
    je    f_key_f4 ;sort_type
    cmp   ah,63    ; down F5
    je    f_key_f5 ;sort_size
    cmp   ah,64    ; down F6
    je    f_key_f6 ; sort_date
    cmp   ah,65    ; down F7
    je    f_key_f7 ;
    cmp   ah,66    ; down F8
    je    f_key_f8 ; delete_file
    cmp   ah,67
    je    f_key_f9
    cmp   ah,88    ; down F12
    je    f_key_f12
    jmp   search_by_key
;-------------------------------------------------------------
.extended_key:
    mov  [extended_key],0
key_80:
    cmp  ah,80  ; arrow down
    jne  key_72
.1:
    mov  bx,1
    call fb_on_key
    jmp  still
;-------------------------------------------------------------
key_72:
    cmp  ah,72  ;arrow up
    jne  key_81
.1:
    mov  bx,2
    call fb_on_key
    jmp  still
;-------------------------------------------------------------
key_81:
    cmp  ah,81 ; PageDown
    jne  key_73
    mov  bx,3
    call fb_on_key
    jmp  still
;-------------------------------------------------------------
key_73:
    cmp  ah,73 ; PageUp
    jne  key_71
    mov  bx,4
    call fb_on_key
    jmp  still
;-------------------------------------------------------------
key_71:
    cmp  ah,71 ; Home
    jne  key_79
    mov  bx,5
    call fb_on_key
    jmp  still
;-------------------------------------------------------------
key_79:
    cmp  ah,79 ; End
    jne  key_28
    mov  bx,6
    call fb_on_key
    je   still   
;-------------------------------------------------------------
key_28:
    cmp  ah,28 ; Enter
    jne  key_ctrl_up
.1:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   still
    cmp  [alt_flag],1
    je   files_information
    
    mov  [open_param],0
.2:
    cmp  [fb_right.select_panel_counter],1
    je   .right_1
    mov  ebp,read_folder_name    ;read_folder.name
    call  calculate_left_data_1
    add  esi,32
    cmp  [exit_dir],1
    je   .up_patch
    test [esi],byte 0x10
    jz   file_no_folder
    cmp  [open_param],0
    jne  still
    cmp  [esi+40],word 2Eh
    je   still
    cmp  [esi+40],word '..'
    jne  @f
    cmp  [esi+40+2],byte 0
    je   .up_patch
@@:
    call calculate_key_28_1
    mov  [fb_left.start_draw_cursor_line],0
    mov  [fb_left.start_draw_line],0
    mov  [sb_left.position],0
    jmp  proc_read_folder
.up_patch:
    call calculate_key_28_2
    jmp  proc_read_folder
.right_1:
    mov  ebp,read_folder_1_name   ;read_folder_1.name
    call calculate_right_data_1
    add  esi,32
    cmp  [exit_dir],1
    je   .up_patch_1
    test [esi],byte 0x10
    jz   file_no_folder
    cmp  [esi+40],word 2Eh
    je   still
    cmp  [esi+40],word '..'
    jne  @f
    cmp  [esi+40+2],byte 0
    je   .up_patch_1
@@:
    call calculate_key_28_1
    mov  [fb_right.start_draw_cursor_line],0
    mov  [fb_right.start_draw_line],0
    mov  [sb_right.position],0
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
    je   @f
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
    jne  search_by_key
    mov  [open_param],background_stretch
    jmp  key_28.2
.shift:
    mov  [open_param],background_tile
    jmp  key_28.2
;---------------------------------------------------------------------    
reload_dir_1:
    cmp  [ctrl_flag],0
    je   search_by_key
    jmp  reload_dir
;---------------------------------------------------------------------
symbol_a_up:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   .ctrl
    ;cmp  [alt_flag],1
    ;je   still
    jmp  still
.ctrl:
    mov  bx,9
    call fb_on_key
    jmp  still
;---------------------------------------------------------------------
;in:
;  ah - key symbol
align 4
search_by_key:
    mov  bx,12
    call fb_on_key
    jmp  still
;---------------------------------------------------------------------
NumPad_plus_Up:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   .ctrl
    ;cmp  [alt_flag],1
    ;je   still
    jmp  still
.ctrl:
    mov  bx,9
    call fb_on_key
    jmp  still
;---------------------------------------------------------------------
NumPad_minus_Up:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   .ctrl
    ;cmp  [alt_flag],1
    ;je   still
    jmp  still
.ctrl:
    mov  bx,10
    call fb_on_key
    jmp  still
;---------------------------------------------------------------------
NumPad_sneg_Up:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   .ctrl
    ;cmp  [alt_flag],1
    ;je   still
    jmp  still
.ctrl:
    mov  bx,11
    call fb_on_key
    jmp  still
;---------------------------------------------------------------------
f_key_f1:
;sort_name:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   still
    cmp  [alt_flag],1
    je   draw_left_select_disk_menu
    jmp  help_start
;---------------------------------------------------------------------
f_key_f2:
;sort_name:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   still
    cmp  [alt_flag],1
    je   draw_right_select_disk_menu
    jmp  still
;---------------------------------------------------------------------
f_key_f3:
;sort_name:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   .ctrl
    ;cmp  [alt_flag],1
    ;je   still
    jmp  still
.ctrl:
    cmp  [fb_right.select_panel_counter],1
    je   right_sort_name
    jmp  left_sort_name
;---------------------------------------------------------------------
f_key_f4:
;sort_type:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   .ctrl
    cmp  [alt_flag],1
    je   still
    jmp  open_file_f4
.ctrl:
    cmp  [fb_right.select_panel_counter],1
    je   right_sort_type
    jmp  left_sort_type
;---------------------------------------------------------------------
f_key_f5:
;sort_size:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   .ctrl
    cmp  [alt_flag],1
    je   still
    jmp  copy_file_f5
.ctrl:
    cmp  [fb_right.select_panel_counter],1
    je   right_sort_size
    jmp  left_sort_size
;---------------------------------------------------------------------
f_key_f6:
;sort_date:
    cmp  [shift_flag],1
    je   .shift
    cmp  [ctrl_flag],1
    je   .ctrl
    cmp  [alt_flag],1
    je   still
    jmp  move_file_f6
.shift:
    jmp  still
.ctrl:
    cmp  [fb_right.select_panel_counter],1
    je   right_sort_date
    jmp  left_sort_date
;---------------------------------------------------------------------
f_key_f7:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   still
    cmp  [alt_flag],1
    je   still
    jmp  create_directory
;---------------------------------------------------------------------
f_key_f8:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   still
    cmp  [alt_flag],1
    je   still
    jmp  delete_file
;---------------------------------------------------------------------
f_key_f9:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   still
    cmp  [alt_flag],1
    je   still
    jmp  create_new_file; create_directory;
;---------------------------------------------------------------------
f_key_f12:
    cmp  [shift_flag],1
    je   still
    cmp  [ctrl_flag],1
    je   still
    cmp  [alt_flag],1
    je   still
    jmp  exit_apl
;---------------------------------------------------------------------
key_shift_up:
    cmp  [shift_flag],1
    je   still
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
    je   still
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
    je   still
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
    jne   @f
.1:
    mov   bx,8 ; mark file
    call  fb_on_key
    jmp   still
@@:
    cmp   ah,83
    je    f_key_f8      ;delete_file
key_end:
    jmp  still
;---------------------------------------------------------------------
;in:
;  ax - key
;  bx - 1...12
align 4
fb_on_key:
    shr  ax,8
    cmp  [fb_right.select_panel_counter],1
    je   .right_1
    mov  [fb_left.key_action],bx
    mov  [fb_left.key_action_num],ax
    stdcall [FileBrowser_key], fb_left
    call sb_left_update
    ret
.right_1:
    mov  [fb_right.key_action],bx
    mov  [fb_right.key_action_num],ax
    stdcall [FileBrowser_key], fb_right
    call sb_right_update
    ret
;---------------------------------------------------------------------
;in:
;  ax - key
;  bx - 1...12
;  esi
align 4
fb_on_key_1:
    shr  ax,8
    or   esi,esi
    jnz  .right_1
    mov  [fb_left.key_action],bx
    mov  [fb_left.key_action_num],ax
    stdcall [FileBrowser_key], fb_left
    call sb_left_update
    ret
.right_1:
    mov  [fb_right.key_action],bx
    mov  [fb_right.key_action_num],ax
    stdcall [FileBrowser_key], fb_right
    call sb_right_update
    ret
;---------------------------------------------------------------------
align 4
sb_left_update:
    cmp  [fb_left.draw_scroll_bar],0
    je   @f
    mov  eax,[fb_left.start_draw_line]
    mov  [sb_left.position],eax
    ;mov  eax,[sb_left.max_area]
    ;cmp  eax,[sb_left.cur_area]
    ;jbe  @f
    ;cmp  [sb_left.cur_area],0
    ;je   @f
    stdcall [scrollbar_v_draw], sb_left
    mov  [fb_left.draw_scroll_bar],0
@@:
    ret
;---------------------------------------------------------------------
align 4
sb_right_update:
    cmp  [fb_right.draw_scroll_bar],0
    je   @f
    mov  eax,[fb_right.start_draw_line]
    mov  [sb_right.position],eax
    ;mov  eax,[sb_right.max_area]
    ;cmp  eax,[sb_right.cur_area]
    ;jbe  @f
    ;cmp  [sb_right.cur_area],0
    ;je   @f
    stdcall [scrollbar_v_draw], sb_right
    mov  [fb_right.draw_scroll_bar],0
@@:
    ret
;---------------------------------------------------------------------