mouse:
    mcall SF_SYSTEM, SSF_GET_ACTIVE_WINDOW
    cmp  [active_process],eax
    jne  still
    mcall SF_MOUSE_GET, SSF_SCROLL_DATA
    mov   [mouse_scroll_data],eax
    mcall SF_MOUSE_GET, SSF_BUTTON
    cmp  [mouse_keys],eax
    je	 @f
    mov  [mouse_keys],eax
    mov  [mouse_keys_delta],0
    jmp  .continue_2
@@:
    mov  [mouse_keys_delta],1
.continue_2:
    mcall SF_MOUSE_GET, SSF_WINDOW_POSITION
    mov  [mouse_position],eax
;    mcall SF_SYSTEM, SSF_GET_ACTIVE_WINDOW
;    cmp  [active_process],eax
;    jne  still
    mov  eax,[mouse_position]
    test eax,0x80000000
    jnz   still
    test eax,0x8000
    jnz   still
    mov   ebx,eax
    shr   ebx,16
    shl   eax,16
    shr   eax,16
    cmp   [mouse_keys_delta],0
    je	  @f
    cmp   [scroll_pointer],0
    jne   .check_scroll_bar_left
@@:
    cmp   ebx,[window_width]
    ja	  still
    cmp   eax,[window_high]
    ja	  still
.check_scroll_bar_left:
    xor   ecx,ecx
    mov   cx,[left_panel_y.start_y]
    cmp   [mouse_keys_delta],0
    je	  @f
    cmp   [scroll_pointer],1
    je	  .check_scroll_bar_left_event
    cmp   [scroll_pointer],2
    je	 .check_scroll_bar_right_event
@@:
    cmp   ax,cx
    jbe    .other
    add   cx,[left_panel_y.size_y]
    cmp   ax,cx
    jae    .other
    mov   cx,[left_panel_x.start_x]
    cmp   bx,cx
    jbe    .other
    add   cx,[left_panel_x.size_x]
    cmp   bx,cx
    jae    .check_scroll_bar_right
    mov   edx,[left_folder_block]
    cmp   edx,[max_panel_line]
    jbe   @f
.check_scroll_bar_left_event:
    mov   [scroll_bar_event],0
    call  check_scroll_left_event
    cmp   [scroll_bar_event],0
    jne    still
    mov   [scroll_bar_event],0
    call  check_scroll_bar_left_event
    cmp   [scroll_bar_event],0
    jne    still  ;.continue
@@:
    call  check_left_panel_mouse_event
.continue:
    jmp   still

.check_scroll_bar_right:
    xor   ecx,ecx
    mov   cx,[right_panel_x.start_x]
    cmp   bx,cx
    jbe    .other
    add   cx,[right_panel_x.size_x]
    cmp   bx,cx
    jae    .other
    mov   edx,[right_folder_block]
    cmp   edx,[max_panel_line]
    jbe   @f
.check_scroll_bar_right_event:
    mov   [scroll_bar_event],0
    call  check_scroll_right_event
    cmp   [scroll_bar_event],0
    jne    still
    mov   [scroll_bar_event],0
    call  check_scroll_bar_right_event
    cmp   [scroll_bar_event],0
    jne    still   ;.continue_1
@@:
    call  check_right_panel_mouse_event
.continue_1:
    jmp   still

.other:
    jmp   still

;.draw_data:
;    pusha
;    mov   [temp_eax],eax
;    mcall SF_DRAW_RECT, <190,400>,<3,15>,0x0
;    xor   ecx,ecx
;    mov   cx,[mouse_position.x]
;    mcall SF_DRAW_NUMBER, 0x80000, ,<200, 5>,0xffffff
;    mov   cx,[mouse_position.y]
;    mcall SF_DRAW_NUMBER, 0x80000, ,<300, 5>,0xffffff
;    mcall SF_DRAW_NUMBER, 0x80000,[mouse_keys],<400, 5>,0xffffff
;    mcall SF_DRAW_TEXT, <500,5>,0xffffff,[temp_edx],5
;    mcall SF_DRAW_NUMBER, 0x80000,[mouse_keys],<200, 5>,0xffffff
;    popa
;    ret
;---------------------------------------------------------------------
check_left_panel_mouse_event:
    mov  [scroll_pointer],0
@@:
    cmp  [mouse_keys_delta],0
    jz	 .start
    cmp  [mouse_keys],0
    jz	.end
.start:
    sub  ax,[left_panel_y.start_y]
    mov  ebx,10
    xor  edx,edx
    div  ebx
    imul eax,10
    cmp  [select_panel_counter],0
    je	 @f
    mov  [select_panel_counter],0
    mov  [mouse_keys_delta_1],0
    push  eax
    call draw_right_panel
    pop   eax
    jmp  .continue
@@:
    cmp  [left_start_draw_cursor_line],ax
    jne   @f
    cmp  [mouse_keys],0
    jnz  .continue
    cmp  [mouse_keys_delta],0
    je	 .enter
@@:
    mov  [mouse_keys_delta_1],0
.continue:
    imul  ebx,[left_folder_block],10
    cmp  eax,ebx
    jb	 @f
    sub  ebx,10
    mov  [left_start_draw_cursor_line],bx
    jmp  .continue_1
@@:
    mov  [left_start_draw_cursor_line],ax
.continue_1:
    call draw_left_panel
.end:
    ret
.enter:
    cmp  [mouse_keys_delta_1],1
    je	 .enter_1
    mcall SF_SYSTEM_GET, SSF_SHUTDOWN
    add  eax,[mouse_key_delay]
    mov  [mouse_keys_tick],eax
    mov  [mouse_keys_delta_1],1
    jmp  .continue_1
.enter_1:
    mov  [mouse_keys_delta_1],0
    mcall SF_SYSTEM_GET, SSF_SHUTDOWN
    cmp  eax,[mouse_keys_tick]
    jbe  @f
    ret
@@:
    add  esp,4
    jmp  key_28.1
;---------------------------------------------------------------------
check_right_panel_mouse_event:
    mov  [scroll_pointer],0
@@:
    cmp  [mouse_keys_delta],0
    jz	 .start
    cmp [mouse_keys],0
    jz	.end
.start:
    sub  ax,[right_panel_y.start_y]
    mov  ebx,10
    xor  edx,edx
    div  ebx
    imul eax,10
    cmp  [select_panel_counter],1
    je	 @f
    mov  [select_panel_counter],1
    mov  [mouse_keys_delta_1],0
    push eax
    call draw_left_panel
    pop  eax
    jmp  .continue
@@:
    cmp  [right_start_draw_cursor_line],ax
    jne   @f
    cmp  [mouse_keys],0
    jnz  .continue
    cmp  [mouse_keys_delta],0
    je	 .enter
@@:
    mov  [mouse_keys_delta_1],0
.continue:
    imul  ebx,[right_folder_block],10
    cmp  eax,ebx
    jb	 @f
    sub  ebx,10
    mov  [right_start_draw_cursor_line],bx
    jmp  .continue_1
@@:
    mov  [right_start_draw_cursor_line],ax
.continue_1:
    call draw_right_panel
.end:
    ret
.enter:
    cmp  [mouse_keys_delta_1],1
    je	 .enter_1
    mcall SF_SYSTEM_GET, SSF_SHUTDOWN
    add  eax,[mouse_key_delay]
    mov  [mouse_keys_tick],eax
    mov  [mouse_keys_delta_1],1
    jmp  .continue_1
.enter_1:
    mov  [mouse_keys_delta_1],0
    mcall SF_SYSTEM_GET, SSF_SHUTDOWN
    cmp  eax,[mouse_keys_tick]
    jbe  @f
    ret
@@:
    add  esp,4
    jmp  key_28.1
;---------------------------------------------------------------------