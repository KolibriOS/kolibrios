;---------------------------------------------------------------------
progress_bar_delete:
    pusha
    cmp  [max_progress_value],0
    je   .end
    call calculate_progress_data_4
    sub  ecx,30
    call calculate_progress_data_5
    mov   cx,60
    call calculate_progress_data_3
    add  ecx,35 shl 16
    mov  cx,15
    mov  si,bx
    mov  edx,0xff0000
    call calculate_progress_data_6
    shr  ecx,16
    mov  bx,cx
    sub  ebx,13
    call  clear_text_area_1
    mcall 4, ,0x90ffffff,file_name
    cmp  [draw_all_progressbar],0
    je   .end
    sub  ebx,13
    mov  edx,delete_type
	mcall
.end:
    popa
    mov  [draw_all_progressbar],0
    ret
;---------------------------------------------------------------------
progress_bar_copy:
    pusha
    call calculate_progress_data_4
    sub  ecx,43
    call calculate_progress_data_5
    mov   cx,86
    call calculate_progress_data_3
    add  ecx,30 shl 16
    mov  cx,15
    mov  si,bx
    pusha
    mov  edx,0xff00
    call calculate_progress_data_6
    popa
    mov  ebp,[current_progress_value_1]
    call calculate_progress_data_1
    add  ecx,30 shl 16
    mov  bx,[temp_progress_value]
    cmp  [move_file_flag],1
    je	@f
    mov  edx,0xff7700
    jmp  .continue_1
@@:
    mov  edx,0xff0000
.continue_1:
	mcall
    call calculate_progress_data_2
    shr  ecx,16
    mov  bx,cx
    sub  ebx,11
    call  clear_text_area_1
    mcall 4, ,0x80ffffff,start_parameter
    sub  ebx,30
    call  clear_text_area_1
    mcall 4, ,0x80ffffff,file_name
    cmp  [draw_all_progressbar],0
    je   .end
    sub  ebx,13
    cmp  [move_file_flag],1
    je	@f
    mov  edx,copy_type_1
    jmp  .continue
@@:
    mov  edx,move_type_1
.continue:

	mcall
.end:
    popa
    mov  [draw_all_progressbar],0
    ret
;---------------------------------------------------------------------
clear_text_area_1:
    cmp  [draw_all_progressbar],1
    je   @f
    push ebx
    mov  ecx,ebx
    shl  ecx,16
    mov  cx,9
    mov  eax,[window_width]
    sub  eax,70
    mov  bx,ax
    mov  eax,13
    mov  edx, 0x6060ff
	mcall
    pop  ebx
@@:
    ret
;---------------------------------------------------------------------
calculate_progress_data_1:
    pusha
    xor  eax,eax
    mov  ax,bx
    shl  eax,10
    mov  ebx,[max_progress_value]
    xor  edx,edx
    div  ebx
    imul eax,ebp
    shr  eax,10
    mov  [temp_progress_value],eax
    popa
    ret
;---------------------------------------------------------------------
calculate_progress_data_2: 
    cmp  [draw_all_progressbar],0
    je   @f    
    push ebx
    rol  esi,16
    mov  si,bx
    rol  ebx,16
    add  bx,si
    ror  ebx,16
    ror  esi,16
    sub  si,bx
    mov  bx,si
    mov  edx,0xffffff
	mcall
    pop  ebx
@@:
    ret
;---------------------------------------------------------------------
calculate_progress_data_3: 
    mov   bx,ax
    mov  eax,13
    cmp  [draw_all_progressbar],0
    je   @f
    mov  edx, 0x6060ff
	mcall
@@:
    add  ebx,10 shl 16
    sub  ebx,20
    ret
;---------------------------------------------------------------------
calculate_progress_data_4:
    mcall 11
    cmp  eax,1
    jne   .1
    call draw_window
    mov  [draw_all_progressbar],1
.1:
    mov  ecx,[window_high]
    mov  ebx,30
    shr  ecx,1
    ret
;---------------------------------------------------------------------
calculate_progress_data_5:
    shl  ecx,16
    shl  ebx,16
    mov  eax,[window_width]
    sub  eax,60
    ret
;---------------------------------------------------------------------
calculate_progress_data_6:
    mov  ebp,[current_progress_value]
    call calculate_progress_data_1
    mov  bx,[temp_progress_value]
	mcall
    call calculate_progress_data_2
    ret
;---------------------------------------------------------------------