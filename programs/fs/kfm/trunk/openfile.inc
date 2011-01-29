;---------------------------------------------------------------------
open_file_f4:
    cmp  [select_panel_counter],0
    jne  .right_1
.left_1:
    xor  eax,eax
    mov  ax,[left_start_draw_cursor_line]
    xor  edx,edx
    mov  ebx,10
    div  ebx
    mov  esi,[left_start_draw_line]
    add  esi,eax
    imul esi,304
    add  esi,[left_folder_data]
    add  esi,32
    test [esi],byte 0x10
    jz	 file_no_folder_1
    jmp  still
.right_1:
    xor  eax,eax
    mov  ax,[right_start_draw_cursor_line]
    xor  edx,edx
    mov  ebx,10
    div  ebx
    mov  esi,[right_start_draw_line]
    add  esi,eax
    imul esi,304
    add  esi,[right_folder_data]
    add  esi,32
    test [esi],byte 0x10
    jz	 file_no_folder_1
    jmp  still
;---------------------------------------------------------------------
file_no_folder_1:
    call  calculate_file_no_folder_1
@@:
    std
    lodsb
    cmp   esi,edx
    je	  still
    cmp   al,'.'
    jnz   @b
    call  calculate_file_no_folder_2
    call  open_file_with_appl_1
    jmp  still
;---------------------------------------------------------------------
calculate_file_no_folder_1:
    add   esi,40
    mov   edx,esi
    xor   eax,eax
@@:
    cld
    lodsb
    test  eax,eax
    jnz   @b
    mov   [temp_counter_1],esi
    dec   esi
    dec   edx
    ret
;---------------------------------------------------------------------
calculate_file_no_folder_2:
    add   esi,2
    mov   ecx,[temp_counter_1]
    sub   ecx,esi
    dec   ecx
    mov   [extension_size],ecx
    inc   edx
    ret
;---------------------------------------------------------------------
file_no_folder:
    call  calculate_file_no_folder_1
@@:
    std
    lodsb
    cmp   esi,edx
    je	  start_file
    cmp   al,'.'
    jnz   @b
    call  calculate_file_no_folder_2
    call  open_file_with_appl
    cmp   eax,1
    je	  start_file.1
    jmp  still
;---------------------------------------------------------------------
start_file:
    inc   edx
.1:
    cmp   [select_panel_counter],0
    jne   @f
    mov   esi,read_folder_name    ;read_folder.name
    jmp   .start
@@:
    mov   esi,read_folder_1_name   ;read_folder_1.name
.start:
    mov   edi,file_name  ;start_file_data.name
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
    mov   [start_file_data.param],eax
    call  start_appl
    cmp   eax,0
    jge    @f
    mov  ebx,0
    sub  ebx,eax
    mov  eax,ebx
    call start_error_window_thread
@@:
    jmp  still
;---------------------------------------------------------------------