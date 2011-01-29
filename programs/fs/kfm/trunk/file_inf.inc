files_information:
    xor  eax,eax
    mov  [alt_flag],al
    mov  [files_counter],eax
    mov  [directories_counter],eax
    mov  [files_size.low],eax
    mov  [files_size.high],eax
    call get_files_information
    mov  eax,files_size
    mov  ebx,type_size_string
    call decimal_string_20
    call start_files_information_thread
   jmp still
;---------------------------------------------------------------------
start_files_information_thread:
    mcall 9, procinfo, -1
    mov   eax,[ebx+46]
    shr   eax,1
    add   eax,[ebx+38]
    sub   eax,150
    mov   [files_information_window_y+2],ax
    mov   eax,[ebx+42]
    shr   eax,1
    add   eax,[ebx+34]
    sub   eax,125
    mov   [files_information_window_x+2],ax
    mcall 51,1,thread_files_information_start,thread_stack
    ret
;---------------------------------------------------------------------
draw_files_information_window:
    mcall 12, 1
    mcall 0,[files_information_window_x] ,[files_information_window_y], 0x03ffffcc
    mov   edx,90 shl 16+50
    mov   ebx,0xa0000
    mov   esi,0x10000000
    cmp  [inf_marked_yes],1
    je	 .continue_4
    cmp  [nesting_direction],1
    je    @f
    add  edx,15
.continue_4:
    mcall 47, ,[files_counter]
@@:
    cmp  [inf_marked_yes],1
    je	 .continue_2
    cmp  [nesting_direction],0
    je    .continue_2
    cmp  [directories_counter],0
    je   @f
.continue_2:
    add  edx,15
    mcall 47, ,[directories_counter]
@@:
    add  edx,15
    mov  ebx,edx
    mcall 4, ,0x90000000,type_size_string
    cmp  [select_panel_counter],0
    jne  .right_1
    mov   edx,read_folder_name
    jmp  @f
.right_1:
    mov   edx,read_folder_1_name
@@:
    mcall 4,<75,35>,0x80000000,   ;file_name
    mcall 4,<10,35>,0x90000000,type_location
    cmp  [inf_marked_yes],1
    je	 .continue
    mcall 4,<90,50>,0x80000000,type_name
    mov   ecx,0x90000000
    cmp  [nesting_direction],1
    je    @f
    mcall 4,<10,50>, ,type_dir_name
    jmp   .continue
@@:
    mcall 4,<10,50>, ,type_file_name
.continue:
    cmp  [inf_marked_yes],1
    je	 .continue_3
    cmp  [nesting_direction],1
    je    @f
.continue_3:
    add   ebx,15
    mcall 4, , ,type_files
@@:
    cmp  [inf_marked_yes],1
    je	 .continue_1
    cmp  [nesting_direction],0
    je    .continue_1
    cmp  [directories_counter],0
    je   @f
.continue_1:
    add   ebx,15
    mcall 4, , ,type_dir
@@:
    add   ebx,15
    mcall 4, , ,type_size
    mcall 12, 2
    ret
;---------------------------------------------------------------------
thread_files_information_start:
.red:
    call  draw_files_information_window
.still:
    mcall 10
    cmp   eax,1
    je	  .red
    cmp   eax,2
    je	  .key
    cmp   eax,3
    je	  .button
    jmp   .still
.key:
.button:
    mcall -1
    jmp   .still
;---------------------------------------------------------------------
;       ПЕРЕВОД 10-НЫХ ЧИСЕЛ СО ЗНАКОМ В СТРОКОВЫЙ ВИД
;       Вход:
;               EAX - указатель на число
;               EBX - адрес строки
;---------------------------------------------------------------------
decimal_string_20:
	push  eax ebx ecx edx
	mov   [divisor],word 10
	mov   ebp,eax
	xor	  ecx,ecx
;	mov	  [ebx],byte '0'
;	inc	  ebx
.p3:
	xor	  edx,edx
	push  ebx
	call  Division_64bit
	mov   eax,[quotent]
	mov   [ebp],eax
	mov   eax,[quotent+4]
	mov   [ebp+4],eax	
	pop   ebx
	add	  edx,48
	push  edx
	inc	  ecx
	cmp   [ebp],dword 0
	jne	  .p3
	cmp   [ebp+4],dword 0
	jne	  .p3
	cmp   ecx,19
	jbe   .p4
	mov   ecx,20
	dec   ebx
.p4:
	pop	  edx
	mov	  [ebx],dl
	inc	  ebx
	loop  .p4
	mov   [ebx], byte 0
	pop	  edx ecx ebx eax
	ret
;----------------------------------------------------------
; деление 64-битного числа divident на 16-битное число divisor.
; Частное помещается в 64-битную переменную quotent,
; а остаток - в 16-битную переменную modulo
Division_64bit:
        mov  ax,[ebp+6]
        xor  dx,dx
        mov  bx,[divisor]
        div  bx
        mov  [quotent+6],ax
        mov  ax,[ebp+4]
        div  bx
        mov  [quotent+4],ax
        mov  ax,[ebp+2]
        div  bx
        mov  [quotent+2],ax
        mov  ax,[ebp]
        div  bx
        mov  [quotent],ax
;        mov  [modulo],dx
        ret
        
;divident:
;      rb 8
divisor:
      rb 2
quotent:
      rb 8
;modulo:
;      rb 2
;---------------------------------------------------------------------
copy_type_name:
    push  esi
    mov  esi,ebx
    mov  edi,type_name
@@:
    cld
    lodsb
    stosb
    test  al,al
    jnz   @b
    pop  esi
    ret
;---------------------------------------------------------------------
get_files_information:
    call get_information_marked_files
    cmp  [inf_marked_yes],1
    je	 .end_2
    xor  eax,eax
    xor  edx,edx
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  ax,[left_start_draw_cursor_line]
    mov  ebx,10
    div  ebx
    mov  ebx,[left_start_draw_line]
    add  ebx,eax
    imul ebx,304
    add  ebx,[left_folder_data]
    call prepare_data_8
    mov   esi,read_folder_name
    jmp  .end
.right_1:
    mov  ax,[right_start_draw_cursor_line]
    mov  ebx,10
    div  ebx
    mov  ebx,[right_start_draw_line]
    add  ebx,eax
    imul ebx,304
    add  ebx,[right_folder_data]
    call prepare_data_8
    mov   esi,read_folder_1_name
.end:
    mov   [error_pointer],ebx
    call  copy_type_name
    mov   edi,file_name
    call  copy_path_1
    test [ebx-40],byte 0x10
    jz	 @f
    push eax ebx
    call get_inf_dir
    mov  [nesting_direction],0
    pop  ebx eax
    dec  [directories_counter]
    jmp  .end_2
@@:
    mov  [nesting_direction],1
    call get_file_size
    test  eax,eax
    jz	 .end_1
    call start_error_window_thread
    add  esp,4
    jmp  reload_dir_all
.end_1:
;    inc  [files_counter]
    mov  eax,[file_features_temp_area+32]
    add  [files_size.low],eax
    mov  eax,[file_features_temp_area+32+4]
    adc  [files_size.high],eax
.end_2:
    ret
;---------------------------------------------------------------------
get_information_marked_files:
    xor  eax,eax
    mov  [inf_marked_yes],al
    cmp  [select_panel_counter],al
    jne  .right_1
    mov  ebp,[left_folder_block]
.start:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[left_folder_data]
    add  ebx,32+40
    cmp  [ebx+299-40],byte 0
    je	 @f
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    je	 .continue_1
.continue:
    mov  esi,read_folder_name
    call prepare_data_9
.continue_1:
    mov  [inf_marked_yes],1
@@:
    dec  ebp
    jnz  .start
    ret
.right_1:
    mov  ebp,[right_folder_block]
.start_1:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[right_folder_data]
    add  ebx,32+40
    cmp  [ebx+299-40],byte 0
    je	 @f
    cmp  [ebx],word '..'
    jne  .continue_2
    cmp  [ebx+2],byte 0
    je	 .continue_3
.continue_2:
    mov   esi,read_folder_1_name
    call prepare_data_9
.continue_3:
    mov  [inf_marked_yes],1
@@:
    dec  ebp
    jnz  .start_1
    ret
;---------------------------------------------------------------------
prepare_data_9:
    mov   [error_pointer],ebx
    mov  edi,file_name
    call copy_path_1
    test [ebx-40],byte 0x10
    jz	 @f
    call get_inf_dir
    ret
@@:
    inc  [files_counter]
    call get_file_size
    test eax,eax
    jz	 .continue
    call start_error_window_thread
    add  esp,8+4
    ret
.continue:
    mov  eax,[file_features_temp_area+32]
    add  [files_size.low],eax
    mov  eax,[file_features_temp_area+32+4]
    adc  [files_size.high],eax
.continue_1:
    ret
;---------------------------------------------------------------------
prepare_data_8:
    add  ebx,32+40
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    jne  .continue
    add  esp,4+4
    jmp  reload_dir
.continue:
    ret
;---------------------------------------------------------------------
get_inf_dir:
    mov  [nesting_direction],0
    pusha
    mov   [recursive_pointer],0
    mov   eax,[appl_memory]
    mov   [appl_memory_old],eax
.1:
    mov  [read_delete_dir_data.size],0
    mov  [read_delete_dir_data.return],delete_dir_data
    call read_delete_dir
    test eax,eax
    jz	 @f
    mov   [error_pointer],file_name
    call start_error_window_thread
    popa
    add  esp,8+8
    ret
@@:
    mov  eax,[delete_dir_data+8]
    imul  eax,304
    add   eax,32
    mov   ebx,eax
    add   ebx,[appl_memory_old]
    cmp   ebx,[appl_memory]
    jbe   @f
    mov   ecx,[appl_memory]
    mov   ecx,ebx
    mov   [appl_memory],ecx
    mcall 64,1
@@:
    mov  eax,[delete_dir_data+8]
    mov  [read_delete_dir_data.size],eax
    mov   eax,[appl_memory_old]
    mov  [read_delete_dir_data.return],eax
    call read_delete_dir
    test eax,eax
    jz	 @f
    mov   [error_pointer],file_name
    call start_error_window_thread
    popa
    add  esp,8+8
    ret
@@:
    cmp  [nesting_direction],0
    je	  @f
    call get_inf_dir_2
    jnc  .end
    dec   ebp
    push  ebp
    jmp   .1
@@:
    call get_inf_dir_1
    jnc  .end
    dec   ebp
    push  ebp
    jmp   .1
.end:
    call  return_delete_path
    dec   [recursive_pointer]
    cmp   [recursive_pointer],0
    jge   .1
    mov   ecx,[appl_memory_old]
    cmp   [appl_memory],ecx
    je	  @f
    mov   [appl_memory],ecx
    mcall 64,1
@@:
    popa
    ret
;---------------------------------------------------------------------
get_inf_dir_2:
    pop  eax
    pop  ebp
    push eax
    jmp  get_inf_dir_1.start
get_inf_dir_1:
    inc  [directories_counter]
    mov  ebp,[appl_memory_old]
    mov  ebp,[ebp+8]
.start:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[appl_memory_old]
    add  ebx,32+40
    cmp  [ebx],byte '.'
    jne  @f
    cmp  [ebx+1],byte 0
    je	 .continue_4
@@:
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    je	 .continue_4
.continue:
    mov  esi,file_name
    inc  [recursive_pointer]
    call copy_path_2
    test [ebx-40],byte 0x10
    jnz  .continue_3
@@:
    inc  [files_counter]
.continue_1:
    mov  eax,[ebx+32-40]
    add  [files_size.low],eax
    mov  eax,[ebx+32+4-40]
    adc  [files_size.high],eax
    dec  [recursive_pointer]
    call return_delete_path
    dec  ebp
    jnz  .start

.continue_4:
    clc
    mov  [nesting_direction],1
    ret
.continue_3:
    stc
    mov  [nesting_direction],0
    ret
;---------------------------------------------------------------------
;type_patch_test:
;    pusha
;    mcall 5,200
;    mcall 13,<290,300>,<3,13>,0x0
;    mcall 4,<300, 5>,0x80FFFFFF,file_name  ;[error_pointer]
;    mcall 47,0x160100,[recursive_pointer],<300, 5>,0xffffff
;    mcall 47,0x160100,ebp,<300, 5>,0xffffff
;    mcall 5,1
;    popa
;    ret
;temp_value_1 db '1',0
;temp_value_2 db '2',0
;temp_value_3 db '3',0
;temp_value_4 db '4',0
;temp_value_5 db '5',0
;---------------------------------------------------------------------