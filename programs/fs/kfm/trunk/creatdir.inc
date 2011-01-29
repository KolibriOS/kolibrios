;-------------------------------------------------------------
create_directory:
    call clear_file_buffer
    mov edi, edit1
    mov ed_size, dword 0 
    mov ed_pos, dword 0
default_box
    mov [create_dir_name],byte 0
    mcall 66, 1, 0
    jmp  .red
.red_1:
    call  draw_window
.red:
    call  draw_create_dir
.still:
    mcall 10
    cmp  eax,1
    je	 .red_1
    cmp  eax,2
    je	 .key_create
    cmp  eax,3
    je	 .button_create
	mouse_edit_box create_editboxes
	jmp .still
;-------------------------------------------------------------
.button_create:
    mov eax,17
    int 0x40
    test ah,ah
    jz	.still
    cmp  ah,160
    je	.exit_with_create
    cmp  ah,161
    je	.exit_without_create
    cmp   ah,1
    jne   .still
    mcall -1
.exit_with_create:
    call  create_directory_1
    cmp   [select_panel_counter],0
    jne   @f
    call proc_read_left_folder
    jmp  .exit_without_create
@@:
    call proc_read_right_folder
.exit_without_create:
    mcall 66, 1, 1
    jmp  red
;-------------------------------------------------------------
.key_create:
    mov eax,2
    int 0x40
    cmp ah,13
    je	.exit_with_create
    cmp ah,27
    je	.exit_without_create
    key_edit_box create_editboxes
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
    mcall 13, , ,0x6060ff ;0xff
    pusha
    shr ebx,16
    add  ebx,10
    shr ecx,16
    add ecx,17
    mov eax,create_editboxes
    mov dword [eax+4],ebx
    mov dword [eax+8],ecx
	draw_edit_box create_editboxes
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
    add   ebx,30 shl 16+5
    mcall 4, ,0x90ffffff,create_dir  ;[confirmation_type]
    add   ebx,48 shl 16+36
    mov   edx,type_yes
    int   0x40
    add   ebx,56 shl 16
    mov   edx,type_no
    int   0x40
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
    call  create_directory_f70
    cmp   eax,0
    je	  @f
;    mov  ebx,0
;    sub  ebx,eax
;    mov  eax,ebx
    call start_error_window_thread
@@:
;    call create_directory_f70
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