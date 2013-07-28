;---------------------------------------------------------------------
key_menu:
    mcall 2
    cmp  [extended_key],1
    je	 .extended_key
    test al, al
    jnz  .end_1
    cmp  ah, 0xE0
    jne  @f
    mov  [extended_key],1
.end_1:
    ret
@@:
    cmp  ah,72
    je	menu_key_72.1
    cmp  ah,80
    je	menu_key_80.1
    cmp  ah,28
    je	menu_key_28.1
.end:
    cmp  ah,208
    je	.end_1
    cmp  ah,200
    je	.end_1
    cmp  ah,156
    je	.end_1
    cmp  ah,184
    je	.end_5
    cmp  ah,187
    je	.end_1
    cmp  ah,188
    je	.end_1
    cmp  ah,199
    je	.end_1
    cmp  ah,207
    je	.end_1
    cmp  ah,201
    je	.end_1
    cmp  ah,209
    je	.end_1
    cmp  ah,42	; NumLock ON
    je	.end_1
    cmp  ah,170  ; NumLock ON
    je	.end_1
    cmp  ah,210
    je	.end_1
    cmp  ah,211
    je	.end_1
    cmp  ah,56
    je	.end_4
    cmp  ah,59
    je	.end_2
    cmp  ah,60
    je	.end_3
.for_all:
    mov [close_menu],1
    ret

.end_2:
    cmp  [alt_flag],0
    je	 .for_all
    cmp  [select_panel_counter],0
    je	.end_1
    mov [close_menu],2
    ret
.end_3:
    cmp  [alt_flag],0
    je	 .for_all
    cmp  [select_panel_counter],0
    jne  .end_1
    mov [close_menu],2
    ret
.end_4:
    cmp  [alt_flag],1
    je	 .end_1
    mov  [alt_flag],1
    jmp  @f
.end_5:
    cmp  [alt_flag],0
    je	 .end_1
    mov  [alt_flag],0
@@:
    call erase_fbutton
    call draw_fbutton
    ret
;---------------------------------------------------------------------
.extended_key:
    mov  [extended_key],0
menu_key_80:
    cmp  ah,80	; arrow down
    jne  menu_key_72
.1:
    mov  eax,[retrieved_devices_table_counter]
    dec  eax
    cmp  eax,[start_draw_menu_cursor_line]
    je	 @f
    inc  [start_draw_menu_cursor_line]
@@:
    ret
;---------------------------------------------------------------------
menu_key_72:
    cmp  ah,72	;arrow up
    jne  menu_key_71
.1:
    cmp  [start_draw_menu_cursor_line],0
    je	 @f
    dec  [start_draw_menu_cursor_line]
@@:
    ret
;---------------------------------------------------------------------
menu_key_71:
    cmp  ah,71    ;Home
    je   @f
    cmp  ah,73    ;PageUp
    jne  menu_key_79
@@:
    mov   [start_draw_menu_cursor_line],0
    ret
;---------------------------------------------------------------------
menu_key_79:
    cmp  ah,79    ; End
    je   @f
    cmp  ah,81    ; PageDown
    jne  menu_key_82
@@:
    mov  eax,[retrieved_devices_table_counter]
    dec  eax
    mov   [start_draw_menu_cursor_line],eax
    ret
;---------------------------------------------------------------------
menu_key_82:
    cmp  ah,82	; Insert
    jne  menu_key_83
    mov  esi,[start_draw_menu_cursor_line]
    imul esi,10
    add  esi,retrieved_devices_table
    inc  esi
    cmp  [esi],word 'cd'
    jne  @f
    xor  ecx,ecx
    mov  cl,[esi+2]
    sub  ecx,0x30
    mcall 24,5
@@:
    ret
;---------------------------------------------------------------------
menu_key_83:
    cmp  ah,83	; Delete
    jne  menu_key_28
    mov  esi,[start_draw_menu_cursor_line]
    imul esi,10
    add  esi,retrieved_devices_table
    inc  esi
    cmp  [esi],word 'cd'
    jne  @f
    xor  ecx,ecx
    mov  cl,[esi+2]
    sub  ecx,0x30
    mcall 24,4
@@:
    ret
;---------------------------------------------------------------------
menu_key_28:
    cmp  ah,28 ; Enter
    jne  menu_key_end
.1:
    mov [press_enter_menu],1
    ret
menu_key_end:
    jmp  key_menu.end
;---------------------------------------------------------------------
key_menu_left:
    mov  [close_menu],0
    mov [press_enter_menu],0
    call key_menu
    cmp  [press_enter_menu],1
    je	 .press_enter_menu
    cmp  [close_menu],0
    je	 draw_left_select_disk_menu.red
.1:
;    mov   [left_panel_clear_all],1
    call draw_left_panel
;    mov   [right_panel_clear_all],1
    call draw_right_panel
    cmp  [alt_flag],0
    je	 still
    cmp  [close_menu],2
    je	 draw_right_select_disk_menu
    jmp  still
.press_enter_menu:
    mov  esi,[start_draw_menu_cursor_line]
    imul esi,10
    add  esi,retrieved_devices_table
    inc  esi
    call copy_folder_name
    mov  [left_start_draw_cursor_line],0
    call proc_read_folder
    jmp  still
;---------------------------------------------------------------------
key_menu_right:
    mov  [close_menu],0
    mov [press_enter_menu],0
    call key_menu
    cmp  [press_enter_menu],1
    je	 .press_enter_menu
    cmp  [close_menu],0
    je	 draw_right_select_disk_menu.red
.1:
;    mov   [left_panel_clear_all],1
    call draw_left_panel
;    mov   [right_panel_clear_all],1
    call draw_right_panel
    cmp  [alt_flag],0
    je	 still
    cmp  [close_menu],2
    je	 draw_left_select_disk_menu
    jmp  still
.press_enter_menu:
    mov  esi,[start_draw_menu_cursor_line]
    imul esi,10
    add  esi,retrieved_devices_table
    inc  esi
    call copy_folder_name_1
    mov  [right_start_draw_cursor_line],0
    call proc_read_folder_1
    jmp  still
;---------------------------------------------------------------------
draw_left_select_disk_menu:
    call  device_detect_f70
    mov  [select_panel_counter],0
    call  draw_left_panel
    call  draw_right_panel
    mov  [start_draw_menu_cursor_line],0
.red:
    call  draw_left_menu
.still:
    mcall 10
    cmp  eax,1
    je	 red
    cmp  eax,2
    je	 key_menu_left
    cmp  eax,3
    je   @f
    cmp   eax,6
    je    mouse_menu_left
    jmp  .still
@@:
    call draw_left_panel
    jmp  button
;---------------------------------------------------------------------
draw_right_select_disk_menu:
    call  device_detect_f70
    mov  [select_panel_counter],1
    call  draw_left_panel
    call  draw_right_panel
    mov  [start_draw_menu_cursor_line],0
.red:
    call  draw_right_menu
.still:
    mcall 10
    cmp  eax,1
    je	 red
    cmp  eax,2
    je	 key_menu_right
    cmp  eax,3
    je   @f
    cmp   eax,6
    je    mouse_menu_right
    jmp  .still
@@:
    call draw_right_panel
    jmp  button
;---------------------------------------------------------------------
draw_left_menu:
    mov  ebx,[left_panel_x]
    mov  bx,60
    mov  ecx,[left_panel_y]
    mov  eax,[retrieved_devices_table_counter]
    mov  cx,ax
    imul cx,10
    mov  edx,0xffffcc
    mov  eax,13
	mcall
    call draw_menu_Selection
    call draw_menu_data
    ret
;---------------------------------------------------------------------
draw_right_menu:
    mov  ebx,[right_panel_x]
    mov  bx,60
    mov  ecx,[right_panel_y]
    mov  eax,[retrieved_devices_table_counter]
    mov  cx,ax
    imul cx,10
    mov  edx,0xffffcc
    mov  eax,13
	mcall
    call draw_menu_Selection
    call draw_menu_data
    ret
;---------------------------------------------------------------------
draw_menu_data:
    mov  edx,retrieved_devices_table
    shr  ecx,16
    mov  bx,cx
    mov  ebp,0
.start:
    mcall 4, ,0x80000000
    add   ebx,10
    add   edx,10
    inc   ebp
    cmp   ebp,[retrieved_devices_table_counter]
    jne   .start
    ret
;---------------------------------------------------------------------
draw_menu_Selection:
    push ebx ecx
    mov  eax,[start_draw_menu_cursor_line]
    imul eax,10
    shl  eax,16
    add  ecx,eax
    mov  cx,10
    mov  edx,0xbbffbb
    mov  eax,13
	mcall
    pop  ecx ebx
    ret
;---------------------------------------------------------------------
mouse_menu_left:
    mcall 37,2
    mov  [mouse_keys],eax
    mcall 37,1
    mov  [mouse_position],eax
    test eax,0x80000000
    jnz   draw_left_select_disk_menu.still
    test eax,0x8000
    jnz   draw_left_select_disk_menu.still
    mov   ebx,eax
    shr   ebx,16
    shl   eax,16
    shr   eax,16
    cmp   ebx,[window_width]
    ja    draw_left_select_disk_menu.still
    cmp   eax,[window_high]
    ja    draw_left_select_disk_menu.still
    xor   ecx,ecx
    mov   cx,[left_panel_y.start_y]
    cmp   ax,cx
    jbe    .close
    mov   edx,[retrieved_devices_table_counter]
    imul  edx,10
    add   cx,dx
    cmp   ax,cx
    jae    .close
    mov   cx,[left_panel_x.start_x]
    cmp   bx,cx
    jbe    .close
    add   cx,60
    cmp   bx,cx
    jae    .close
    sub   ax,[left_panel_y.start_y]
    mov   ebx,10
    xor   edx,edx
    div   ebx
    mov   [start_draw_menu_cursor_line],eax
    cmp   [mouse_keys],0
    je    draw_left_select_disk_menu.red
    jmp   key_menu_left.press_enter_menu
.close:
    cmp   [mouse_keys],0
    je    draw_left_select_disk_menu.still
;    mov   [left_panel_clear_all],1
    call  draw_left_panel
    jmp   still
;---------------------------------------------------------------------
mouse_menu_right:
    mcall 37,2
    mov  [mouse_keys],eax
    mcall 37,1
    mov  [mouse_position],eax
    test eax,0x80000000
    jnz   draw_right_select_disk_menu.still
    test eax,0x8000
    jnz   draw_right_select_disk_menu.still
    mov   ebx,eax
    shr   ebx,16
    shl   eax,16
    shr   eax,16
    cmp   ebx,[window_width]
    ja    draw_right_select_disk_menu.still
    cmp   eax,[window_high]
    ja    draw_right_select_disk_menu.still
    xor   ecx,ecx
    mov   cx,[right_panel_y.start_y]
    cmp   ax,cx
    jbe    .close
    mov   edx,[retrieved_devices_table_counter]
    imul  edx,10
    add   cx,dx
    cmp   ax,cx
    jae    .close
    mov   cx,[right_panel_x.start_x]
    cmp   bx,cx
    jbe    .close
    add   cx,60
    cmp   bx,cx
    jae    .close
    sub   ax,[right_panel_y.start_y]
    mov   ebx,10
    xor   edx,edx
    div   ebx
    mov   [start_draw_menu_cursor_line],eax
    cmp   [mouse_keys],0
    je    draw_right_select_disk_menu.red
    jmp   key_menu_right.press_enter_menu
.close:
    cmp   [mouse_keys],0
    je    draw_right_select_disk_menu.still
;    mov   [right_panel_clear_all],1
    call  draw_right_panel
    jmp   still
;---------------------------------------------------------------------