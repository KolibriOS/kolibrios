;---------------------------------------------------------------------
align 4
mark_file:
    mov  [marked_action],1
    cmp  [select_panel_counter],0
    jne  .right_1
    call  calculate_left_data_1
    jmp  @f
.right_1:
    call  calculate_right_data_1
@@:
    add  esi,32+299
    mov  al,[esi]
    and  al,1 ; get mark
    cmp  [select_panel_counter],0
    jne  .right_2    
    test al,al
    jnz  @f
    inc  [left_marked_counter]
    jmp  .for_all_1
@@:
    dec  [left_marked_counter]
    jmp  .for_all_1
.right_2:
    test al,al
    jnz  @f
    inc  [right_marked_counter]
    jmp  .for_all_1
@@:
    dec  [right_marked_counter]
.for_all_1:
    inc  al
    and  al,1 ; invert mark
    mov  [esi],al ; set mark
    jmp   key_80.1
;---------------------------------------------------------------------
;out:
;  esi = left_folder_data + 304 *
;      * (left_start_draw_line + left_start_draw_cursor_line / 10)
align 4
calculate_left_data_1:
    movzx eax,[left_start_draw_cursor_line]
    xor  edx,edx
    mov  ebx,10
    div  ebx
    mov  esi,[left_start_draw_line]
    add  esi,eax
    imul esi,304
    add  esi,[left_folder_data]
    ret
;---------------------------------------------------------------------
;out:
;  esi = left_folder_data + 304 *
;      * (right_start_draw_line + right_start_draw_cursor_line / 10)
align 4
calculate_right_data_1:
    movzx eax,[right_start_draw_cursor_line]
    xor  edx,edx
    mov  ebx,10
    div  ebx
    mov  esi,[right_start_draw_line]
    add  esi,eax
    imul esi,304
    add  esi,[right_folder_data]
    ret
;---------------------------------------------------------------------
align 4
mark_all_file:
    cmp  [select_panel_counter],0
    jne  .right_1
    mov  ebp,[left_folder_block]
.start:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[left_folder_data]
    add  ebx,32+40
    cmp  [ebx],word '..'
    jne  .continue
    cmp  [ebx+2],byte 0
    je   .continue_1
.continue:
    call  select_mark_action
.continue_1:
@@:
    dec  ebp
    jnz  .start
    call  draw_left_panel
    cmp  [mark_action_counter],0
    jne  @f
    mov  eax,[left_folder_block]
;    dec  eax
;    mov  [left_marked_counter],eax
;    jmp  still
    jmp   .end
@@:
    cmp  [mark_action_counter],1
    jne  @f
    mov  [left_marked_counter],0
    jmp  still
@@:
    mov  eax,[left_folder_block]
    sub  eax,[left_marked_counter]
.end:
    dec  eax
    mov  [left_marked_counter],eax
    jmp  still
.right_1:
    mov  ebp,[right_folder_block]
.start_1:
    mov  ebx,ebp
    dec  ebx
    imul ebx,304
    add  ebx,[right_folder_data]
    add  ebx,32+40
    cmp  [ebx],word '..'
    jne  .continue_2
    cmp  [ebx+2],byte 0
    je   .continue_3
.continue_2:
    call  select_mark_action
.continue_3:
@@:
    dec  ebp
    jnz  .start_1
    call  draw_right_panel
    cmp  [mark_action_counter],0
    jne  @f
    mov  eax,[right_folder_block]
;    dec  eax
;    mov  [right_marked_counter],eax
;    jmp  still
    jmp   .end_1
@@:
    cmp  [mark_action_counter],1
    jne  @f
    mov  [right_marked_counter],0
    jmp  still
@@:
    mov  eax,[right_folder_block]
    sub  eax,[right_marked_counter]
.end_1:
    dec  eax
    mov  [right_marked_counter],eax    
    jmp  still
;---------------------------------------------------------------------
align 4
select_mark_action:
    add  ebx,299-40
    cmp  [mark_action_counter],0
    jne  @f
    mov  [ebx],byte 1
    jmp  .continue
@@:
    cmp  [mark_action_counter],1
    jne  @f
    mov  [ebx],byte 0
    jmp  .continue
@@:
    mov  al,[ebx]
    inc  al
    and  al,1
    mov  [ebx],al
.continue:
    ret
;---------------------------------------------------------------------
align 4
proc mark_file_clear_all uses eax ecx edi, tmem:dword, tsize:dword
    xor  al,al
    mov  edi,[tmem]
    mov  ecx,edi
    add  ecx,[tsize]
    add  edi,32+299
    cld
@@:
    stosb
    add  edi,304-1
    cmp  edi,ecx
    jl   @b
    ret
endp
;---------------------------------------------------------------------