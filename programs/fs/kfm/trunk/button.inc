button:
    mcall 17
.1:
    cmp   ah,101
    je   f_key_f1
    cmp   ah,102
    je   f_key_f2
    cmp   ah,103
    je   f_key_f3 ;sort_name
    cmp   ah,104
    je   f_key_f4 ;sort_type
    cmp   ah,105
    je   f_key_f5 ;sort_size
    cmp   ah,106
    je   f_key_f6 ;sort_date
    cmp   ah,107
    je   f_key_f7 ;create dir
    cmp   ah,108
    je   f_key_f8 ;delete_file
    cmp   ah,112
    je   f_key_f12
    cmp   ah,120
    jne   .121
    mov  esi,read_folder_device_name.rd
    call copy_folder_name
    jmp  left_panel_read_folder
 .121:
    cmp   ah,121
    jne   .122
    mov  esi,read_folder_device_name.fd
    call copy_folder_name
    jmp  left_panel_read_folder
 .122:
    cmp   ah,122
    jne   .123
    mov  esi,read_folder_device_name.hd0
    call copy_folder_name
    jmp  left_panel_read_folder
 .123:
    cmp   ah,123
    jne   .124
    mov  esi,read_folder_device_name.hd1
    call copy_folder_name
    jmp  left_panel_read_folder
 .124:
    cmp   ah,124
    jne   .125
    mov  esi,read_folder_device_name.hd2
    call copy_folder_name
    jmp  left_panel_read_folder
 .125:
    cmp   ah,125
    jne   .126
    mov  esi,read_folder_device_name.hd3
    call copy_folder_name
    jmp  left_panel_read_folder
 .126:
    cmp   ah,126
    jne   .127
    mov  esi,read_folder_device_name.cd0
    call copy_folder_name
    jmp  left_panel_read_folder
 .127:
    cmp   ah,127
    jne   .128
    mov  esi,read_folder_device_name.cd1
    call copy_folder_name
    jmp  left_panel_read_folder
 .128:
    cmp   ah,128
    jne   .129
    mov  esi,read_folder_device_name.cd2
    call copy_folder_name
    jmp  left_panel_read_folder
 .129:
    cmp   ah,129
    jne   .130
    mov  esi,read_folder_device_name.cd3
    call copy_folder_name
    jmp  left_panel_read_folder
 .130:
    cmp   ah,130
    jne   .131
    mov  esi,read_folder_device_name.rd
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .131:
    cmp   ah,131
    jne   .132
    mov  esi,read_folder_device_name.fd
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .132:
    cmp   ah,132
    jne   .133
    mov  esi,read_folder_device_name.hd0
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .133:
    cmp   ah,133
    jne   .134
    mov  esi,read_folder_device_name.hd1
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .134:
    cmp   ah,134
    jne   .135
    mov  esi,read_folder_device_name.hd2
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .135:
    cmp   ah,135
    jne   .136
    mov  esi,read_folder_device_name.hd3
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .136:
    cmp   ah,136
    jne   .137
    mov  esi,read_folder_device_name.cd0
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .137:
    cmp   ah,137
    jne   .138
    mov  esi,read_folder_device_name.cd1
    call copy_folder_name_1
    jmp  right_panel_read_folder
 .138:
    cmp   ah,138
    jne   .139
    mov   esi,read_folder_device_name.cd2
    call  copy_folder_name_1
    jmp   right_panel_read_folder
 .139:
    cmp   ah,139
    jne   .140
    mov   esi,read_folder_device_name.cd3
    call  copy_folder_name_1
    jmp   right_panel_read_folder
 .140:
    cmp   ah,140
    je    draw_left_select_disk_menu
    cmp   ah,141
    je    draw_right_select_disk_menu
    cmp   ah,142
    je    left_sort_name
    cmp   ah,143
    je    left_sort_type
    cmp   ah,144
    je    left_sort_size
    cmp   ah,145
    je    left_sort_date
    cmp   ah,146
    je    right_sort_name
    cmp   ah,147
    je    right_sort_type
    cmp   ah,148
    je    right_sort_size
    cmp   ah,149
    je    right_sort_date
    cmp   ah,150
    je    exit_dir_1
    cmp   ah,151
    je    reload_dir
    cmp   ah,152
    je    create_directory
    cmp   ah,153
    je    copy_file_f5
    cmp   ah,154
    je    move_file_f6
    cmp   ah,157
    je    delete_file
    cmp   ah, 1
    jne   still
  .exit:
    mcall -1
;---------------------------------------------------------------------
reload_dir:
    cmp   [select_panel_counter],0
    jne   @f
    call proc_read_left_folder
;    mov   [left_panel_clear_all],1
    call  draw_left_panel
    jmp  still
@@:
    call proc_read_right_folder
;    mov   [right_panel_clear_all],1
    call  draw_right_panel
    jmp  still
;---------------------------------------------------------------------
reload_dir_all:
    call proc_read_left_folder
    call proc_read_right_folder
    jmp  red    
;---------------------------------------------------------------------
exit_dir_1:
    mov   [exit_dir],1
    jmp   key_28.1
;---------------------------------------------------------------------
left_panel_read_folder:
    mov   [left_start_draw_cursor_line],0
    jmp   proc_read_folder
;---------------------------------------------------------------------
right_panel_read_folder:
    mov   [right_start_draw_cursor_line],0
    jmp   proc_read_folder_1
;---------------------------------------------------------------------
left_sort_name:
    xor   eax,eax
    cmp  [left_sort_flag],eax
    je   still
    mov  [left_sort_flag],eax
    jmp   left_sort_date.1
;---------------------------------------------------------------------
left_sort_type:
    cmp  [left_sort_flag],1
    je   still
    mov  [left_sort_flag],1
    jmp   left_sort_date.1
;---------------------------------------------------------------------
left_sort_size:
    cmp  [left_sort_flag],2
    je   still
    mov  [left_sort_flag],2
    jmp   left_sort_date.1
;---------------------------------------------------------------------
left_sort_date:
    cmp  [left_sort_flag],3
    je   still
    mov  [left_sort_flag],3
.1:
    call  draw_left_sort_button
    call  left_sort_files
;    mov   [left_panel_clear_all],1
    call draw_left_panel
    jmp   still
;---------------------------------------------------------------------
right_sort_name:
    xor   eax,eax
    cmp  [right_sort_flag],eax
    je   still
    mov   [right_sort_flag],eax
    jmp   right_sort_date.1
;---------------------------------------------------------------------
right_sort_type:
    cmp  [right_sort_flag],1
    je   still
    mov   [right_sort_flag],1
    jmp   right_sort_date.1
;---------------------------------------------------------------------
right_sort_size:
    cmp  [right_sort_flag],2
    je   still
    mov   [right_sort_flag],2
    jmp   right_sort_date.1
;---------------------------------------------------------------------
right_sort_date:
    cmp  [right_sort_flag],3
    je   still
    mov   [right_sort_flag],3
.1:
    call  draw_right_sort_button
    call  right_sort_files
    call draw_right_panel
    jmp   still
;---------------------------------------------------------------------