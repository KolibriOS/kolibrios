;-------------------------------------------------------------
create_directory:
    call clear_file_buffer
    mov edi, edit1
    mov ed_size, dword 0 
    mov ed_pos, dword 0
    and word ed_flags,ed_shift_cl
    mov [create_dir_name],byte 0
    mcall SF_KEYBOARD, SSF_SET_INPUT_MODE, 0
    jmp  .red
.red_1:
    call  draw_window
.red:
    call  draw_create_dir
.still:
    mcall SF_WAIT_EVENT
    cmp  eax,EV_REDRAW
    je	 .red_1
    cmp  eax,EV_KEY
    je	 .key_create
    cmp  eax,EV_BUTTON
    je	 .button_create
if USE_STATIC_LIBS eq 0
    stdcall [edit_box_mouse], create_editboxes
else
    stdcall edit_box_mouse, create_editboxes
end if
    jmp .still
;-------------------------------------------------------------
.button_create:
    mcall SF_GET_BUTTON
    test ah,ah
    jz	.still
    cmp  ah,160
    je	.exit_with_create
    cmp  ah,161
    je	.exit_without_create
    cmp   ah,1
    jne   .still
    mcall SF_TERMINATE_PROCESS
.exit_with_create:
    call  create_directory_1
    cmp   [select_panel_counter],0
    jne   @f
    call proc_read_left_folder
    jmp  .exit_without_create
@@:
    call proc_read_right_folder
.exit_without_create:
    mcall SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
    jmp  red
;-------------------------------------------------------------
.key_create:
    mcall SF_GET_KEY
    cmp ah,13
    je	.exit_with_create
    cmp ah,27
    je	.exit_without_create
if USE_STATIC_LIBS eq 0
    stdcall [edit_box_key], create_editboxes
else
    stdcall edit_box_key, create_editboxes
end if
    jmp .still
;-------------------------------------------------------------
draw_create_dir:
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
    mcall SF_DRAW_RECT, , ,0x6060ff ;0xff
    pusha
    shr ebx,16
    add  ebx,10
    shr ecx,16
    add ecx,17
    mov eax,create_editboxes
    mov dword [eax+4],ebx
    mov dword [eax+8],ecx
if USE_STATIC_LIBS eq 0
	stdcall [edit_box_draw], create_editboxes
else
    stdcall edit_box_draw, create_editboxes
end if
	popa
	pusha
	add   ebx,70 shl 16
	add   ecx,37 shl 16
    mov   cx,15
    mov   bx,40
    mov   esi,0xffffff
    mov   edx,0x40000000+160
    mcall SF_DEFINE_BUTTON
    add   ebx,50 shl 16
    inc   edx
	mcall
    popa

    push  ebx ecx
    add   ebx,70 shl 16
    add   ecx,37 shl 16
    mov   bx,40
    mov   cx,15
    xor   edx,edx
    push  ebx ecx
    sub   ecx,1 shl 16
    sub   ebx,1 shl 16
    mov   bx,42
    mov   cx,17
	mcall
    add   ebx,50 shl 16
	mcall
    pop   ecx ebx
    mov   edx,0xff7700
	mcall
    mov   edx,0xaa00
    add   ebx,50 shl 16
	mcall
    pop   ecx ebx

    shr   ecx,16
    mov   bx,cx
    add   ebx,30 shl 16+1
    mcall SF_DRAW_TEXT, ,0x90ffffff,sz_Create_dir  ;[confirmation_type]
    add   ebx,48 shl 16+36
    mov   edx,type_yes
	mcall
    add   ebx,56 shl 16
    mov   edx,type_no
	mcall
	ret
;-------------------------------------------------------------
create_directory_1:
    mov   edx,create_dir_name
    cmp   [select_panel_counter],0
    jne   @f
    mov   esi,read_folder_name
    jmp   .start
@@:
    mov   esi,read_folder_1_name
.start:
    mov   edi,file_name
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   [edi-1], byte '/'
    mov   esi,edx
    mov   [error_pointer], edx	;start_file_data.name
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mcall SF_FILE, create_dir_file_data
    cmp   eax,0
    je	  @f
    call start_error_window_thread
@@:
    ret
;-------------------------------------------------------------
clear_file_buffer:
    xor  eax,eax
    mov edi,create_dir_name
    mov ecx,256/4
    cld
    rep stosd
    ret
;-------------------------------------------------------------