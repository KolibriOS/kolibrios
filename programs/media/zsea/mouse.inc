;---------------------------------------------------------------------
mouse:
    mcall 18,7
    cmp  [active_process],eax
    jne  still
	
    mcall 37,7
    mov   [mouse_scroll_data],eax
	
	mov  eax,[mouse_position]
	mov  [mouse_position_old],eax
	mcall 37,1
    mov  [mouse_position],eax
	
	mov  eax,[mouse_keys]
	mov  [mouse_keys_old],eax
    mcall 37,2
    mov  [mouse_keys],eax
	
	cmp [soi],dword 0
	je  .menu_bar
	cmp [scaling_mode],dword 0
	je  .menu_bar
	
	cmp  [capture_image_area_flag],1
	je   capture_image_area
;-----------------------------------------------------
    cmp   [scroll_bar_data_vertical.delta2],0
	jne   .vertical
.horizontal:
    mov   eax,[scroll_bar_data_horizontal.max_area]
    cmp   eax,[scroll_bar_data_horizontal.cur_area]
    jbe   .vertical
	
	push    dword scroll_bar_data_horizontal
	call    [scrollbar_hor_mouse]
	
	;    mouse_scroll_bar_horizontal scroll_bar_data_horizontal
    cmp   [scroll_bar_data_horizontal.redraw],0	
    je    @f
	
    mov   [scroll_bar_data_horizontal.redraw],0
    jmp   .draw
@@:
    cmp   [scroll_bar_data_horizontal.delta2],0
	jne   still
;-----------------------------------------------------
.vertical:
    mov   eax,[scroll_bar_data_vertical.max_area]
    cmp   eax,[scroll_bar_data_vertical.cur_area]
    jbe   .menu_bar
	
	push    dword scroll_bar_data_vertical
	call    [scrollbar_ver_mouse]
	
;    mouse_scroll_bar_vertical scroll_bar_data_vertical
    cmp   [scroll_bar_data_vertical.redraw],0	
    je    .menu_bar   ;still
    mov   [scroll_bar_data_vertical.redraw],0
.draw:
    call  drawimage
    jmp   still
;---------------------------------------------------
.clear_mouse_keys_old:
	xor  eax,eax
    mov  [mouse_keys],eax
	mov  [mouse_keys_old],eax
	ret
;---------------------------------------------------
.menu_bar:
	cmp  [scroll_bar_data_vertical.delta2],0
	jne   still
	cmp  [scroll_bar_data_horizontal.delta2],0
	jne   still
;	xor   eax,eax
;	mov   [menu_data_1.get_mouse_flag],eax
;	mov   [menu_data_2.get_mouse_flag],eax
;	mov   [menu_data_3.get_mouse_flag],eax
;	mov   [menu_data_4.get_mouse_flag],eax
;	mov   [menu_data_5.get_mouse_flag],eax
;	mov   [menu_data_6.get_mouse_flag],eax
	
.menu_bar_1:
	call   .set_mouse_flag
@@:
	push    dword menu_data_1
	call    [menu_bar_mouse]
;    mouse_menubar menu_data_1
    cmp   [menu_data_1.click],dword 1
	jne   .menu_bar_2
	call  .clear_mouse_keys_old
	cmp   [menu_data_1.cursor_out],dword 0
	jne    analyse_out_menu_1
	jmp    .menu_bar_1
;--------------------------------------------
.menu_bar_2:
; Edit
	push    dword menu_data_2
	call    [menu_bar_mouse]
;    mouse_menubar menu_data_2
    cmp   [menu_data_2.click],dword 1
	jne   .menu_bar_3
	call  .clear_mouse_keys_old
	cmp   [menu_data_2.cursor_out],dword 0
	jne    analyse_out_menu_2
    jmp    .menu_bar_1
;--------------------------------------------
.menu_bar_3:
; View
	push    dword menu_data_3
	call    [menu_bar_mouse]
;    mouse_menubar menu_data_3
    cmp   [menu_data_3.click],dword 1
	jne   .menu_bar_4
	call  .clear_mouse_keys_old
	cmp   [menu_data_3.cursor_out],dword 0
	jne    analyse_out_menu_3 ;still
    jmp    .menu_bar_1
;--------------------------------------------
.menu_bar_4:
; Zoom
	push    dword menu_data_4
	call    [menu_bar_mouse]
;    mouse_menubar menu_data_4
    cmp   [menu_data_4.click],dword 1
	jne   .menu_bar_5
	call  .clear_mouse_keys_old
	cmp   [menu_data_4.cursor_out],dword 0
	jne    analyse_out_menu_4
    jmp    .menu_bar_1
;--------------------------------------------
.menu_bar_5:
; Tools
	push    dword menu_data_5
	call    [menu_bar_mouse]
;    mouse_menubar menu_data_5
    cmp   [menu_data_5.click],dword 1
	jne   .menu_bar_6
	call  .clear_mouse_keys_old
	cmp   [menu_data_5.cursor_out],dword 0
	jne    analyse_out_menu_5
    jmp    .menu_bar_1
;--------------------------------------------
.menu_bar_6:
; Help
	push    dword menu_data_6
	call    [menu_bar_mouse]
;    mouse_menubar menu_data_6
    cmp   [menu_data_6.click],dword 1
	jne   .mouse_dinamic_button
	call  .clear_mouse_keys_old
	cmp   [menu_data_6.cursor_out],dword 0
	jne    analyse_out_menu_6
	jmp   .menu_bar_1
;---------------------------------------------------
.mouse_dinamic_button:
; open file
	push    dword dinamic_button_data_1
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_1
	mov   eax,dinamic_button_data_1.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   kfile
;-------------------------------------------------
@@:
; reload file
	push    dword dinamic_button_data_2
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_2
	mov   eax,dinamic_button_data_2.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   kopen
;-------------------------------------------------
@@:
; left limit
	push    dword dinamic_button_data_3
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_3
	mov   eax,dinamic_button_data_3.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   left_limit
;-------------------------------------------------
@@:
; left arrow
	push    dword dinamic_button_data_4
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_4
	mov   eax,dinamic_button_data_4.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   move_to_left
;-------------------------------------------------
@@:
; slide show
	push    dword dinamic_button_data_5
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_5
	mov   eax,dinamic_button_data_5.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   slide_show.3
;-------------------------------------------------
@@:
; right arrow
	push    dword dinamic_button_data_6
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_6
	mov   eax,dinamic_button_data_6.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   move_to_right
;-------------------------------------------------
@@:
; right limit
	push    dword dinamic_button_data_7
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_7
	mov   eax,dinamic_button_data_7.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   right_limit
;-------------------------------------------------
@@:
; zoom +
	push    dword dinamic_button_data_8
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_8
	mov   eax,dinamic_button_data_8.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   zoom_plus  ;still
;-------------------------------------------------
@@:
; zoom -
	push    dword dinamic_button_data_9
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_9
	mov   eax,dinamic_button_data_9.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   zoom_minus  ;still
;-------------------------------------------------
@@:
; zoom 100%
	push    dword dinamic_button_data_10
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_10
	mov   eax,dinamic_button_data_10.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   no_scaling  ;still
;-------------------------------------------------
@@:
; rotate clockwise
	push    dword dinamic_button_data_11
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_11
	mov   eax,dinamic_button_data_11.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   rotate_clockwise
;-------------------------------------------------
@@:
; rotate counter-clockwise
	push    dword dinamic_button_data_12
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_12
	mov   eax,dinamic_button_data_12.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   rotate_counter_clockwise
;-------------------------------------------------
@@:
; file information
	push    dword dinamic_button_data_13
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_13
	mov   eax,dinamic_button_data_13.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   kinfo
;-------------------------------------------------
@@:
; wallpaper
	push    dword dinamic_button_data_14
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_14
	mov   eax,dinamic_button_data_14.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   k_background
;-------------------------------------------------
@@:
; settings
	push    dword dinamic_button_data_15
	call    [dinamic_button_mouse]
;    mouse_dinamic_button dinamic_button_data_15
	mov   eax,dinamic_button_data_15.click
    cmp   [eax],dword 1
	jne   @f
    mov   [eax],dword 0
	jmp   k_option  ;still
;-------------------------------------------------
@@:
	jmp  check_scroll_event
;    jmp   still
;---------------------------------------------------------------------
.set_mouse_flag:
	xor   eax,eax
	inc   eax
	mov   [menu_data_1.get_mouse_flag],eax
	mov   [menu_data_2.get_mouse_flag],eax
	mov   [menu_data_3.get_mouse_flag],eax
	mov   [menu_data_4.get_mouse_flag],eax
	mov   [menu_data_5.get_mouse_flag],eax
	mov   [menu_data_6.get_mouse_flag],eax
	ret
;---------------------------------------------------------------------
analyse_out_menu_1:
; File
	mov eax,[menu_data_1.cursor_out]
	cmp eax,dword 1
	je   kfile
	cmp eax,dword 2
	je   kinfo	
	cmp eax,dword 3
	je   START.exit	 
    jmp   still
;---------------------------------------------------------------------
analyse_out_menu_2:
; Edit
	mov eax,[menu_data_2.cursor_out]
	cmp eax,dword 1
	je   rotate_clockwise
	cmp eax,dword 2
	je   rotate_counter_clockwise	
	cmp eax,dword 3
	je   invert_left_to_right
	cmp eax,dword 4
	je   invert_up_to_down	
    jmp   still
;---------------------------------------------------------------------
analyse_out_menu_3:
; View
	mov  eax,[menu_data_3.cursor_out]
	cmp eax,dword 1
	je   k_sort_directory
	cmp eax,dword 2
	je   full_screen
	cmp eax,dword 3
	je   kopen
    jmp   still
;---------------------------------------------------------------------
analyse_out_menu_4:
; Zoom
	mov eax,[menu_data_4.cursor_out]
	cmp eax,dword 1
	je   no_scaling
	cmp eax,dword 2
	je   zoom_plus
	cmp eax,dword 3
	je   zoom_minus
	cmp eax,dword 4
	je   scaling
    jmp   still
;---------------------------------------------------------------------
analyse_out_menu_5:
; Tools
	mov eax,[menu_data_5.cursor_out]
	cmp eax,dword 1
	je  image_editor
	cmp eax,dword 2
	je  k_background
	cmp eax,dword 3
	je   slide_show.3
	cmp eax,dword 4
	je  k_option
    jmp still
;---------------------------------------------------------------------
analyse_out_menu_6:
; Help
	cmp [menu_data_6.cursor_out],dword 1
	je   open_help
	cmp [menu_data_6.cursor_out],dword 2
	je   kabout
    jmp   still
;---------------------------------------------------------------------
check_scroll_event:
;	jmp  capture_image_area
	mov  eax,[mouse_scroll_data]
	test eax,eax
	jz   capture_image_area ;still
	test ax,0x8000
	jnz  .decr
	shr  eax,16
	test ax,0x8000
	jnz  .decr_1
	pusha
	mov  eax,[scaling_mode]
	test eax,eax	
	jnz  .continue_1
	call  calculation_next_value_minus
	mov  [calc_next_value_minus],1
.continue_1:
	popa
	mov  eax,zoom_factors
	cmp  [eax],dword zoom_factors.min
	jbe  still
	xor  ecx,ecx
	mov  cx,[mouse_scroll_data.vertical]
	test ecx,ecx
	jnz  @f
	mov  cx,[mouse_scroll_data.horizontal]
	test ecx,ecx
	jz   still
@@:
;----------------------------------------
	cmp  [calc_next_value_minus],1
	jne  @f
	mov  [calc_next_value_minus],0
	dec  ecx
	jz   zoom_plus.1
@@:
	mov  ebx,4
@@:
	cmp  [eax],dword zoom_factors.min
	jbe  @f
	sub  [eax],ebx
	dec  ecx
	jnz  @r
	jmp  zoom_plus.1
@@:
	mov  [eax],dword zoom_factors.min
	jmp  zoom_plus.1
;----------------------------------------
.decr:
	xor  ebx,ebx
	mov  bx,[mouse_scroll_data.vertical]
	jmp    @f
.decr_1:
	xor  ebx,ebx
	mov  bx,[mouse_scroll_data.horizontal]
@@:
	pusha
	mov  eax,[scaling_mode]
	test eax,eax	
	jnz  .continue_2
	call  calculation_next_value_plus
	mov  [calc_next_value_plus],1
.continue_2:
	popa
	mov  eax,zoom_factors
	cmp  [eax],dword zoom_factors.max
	jae  still
	mov  ecx,0xffff
	sub  ecx,ebx
	inc  ecx
;---------------------------------------
	cmp  [calc_next_value_plus],1
	jne  @f
	mov  [calc_next_value_plus],0
	dec  ecx
	jz   zoom_plus.1
@@:
	mov  ebx,4
@@:
	cmp  [eax],dword zoom_factors.max
	jae  @f
	add  [eax],ebx
	dec  ecx
	jnz  @r
	jmp  zoom_plus.1
@@:
	mov  [eax],dword zoom_factors.max
	jmp  zoom_plus.1
;---------------------------------------------------------------------
capture_image_area:
	cmp  [scroll_bar_data_vertical.delta2],0
	jne   .no_capture
	cmp  [scroll_bar_data_horizontal.delta2],0
	jne   .no_capture
	xor   eax,eax
	mov   [draw_scrollbar_counter],eax
    mov   eax,[scroll_bar_data_vertical.max_area]
    cmp   eax,[scroll_bar_data_vertical.cur_area]
    jbe   @f
	inc  [draw_scrollbar_counter]
@@:	
    mov   eax,[scroll_bar_data_horizontal.max_area]
    cmp   eax,[scroll_bar_data_horizontal.cur_area]
    jbe   @f
	inc  [draw_scrollbar_counter]
@@:
	cmp  [draw_scrollbar_counter],0
	je   still

	mov  eax,[mouse_position]
	xor  ebx,ebx
	mov  bx,ax  ; EBX mouse y
	shr  eax,16 ; EAX mouse x
	
	mov  ecx,[image_start_coordinates]
	xor  edx,edx
	mov  dx,cx  ; EDX start y
	shr  ecx,16 ; ECX start x
	
	cmp  ax,cx
	jb   still ; min x

	cmp  bx,dx
	jb   still ; min y

	add  cx,[wa_width_2]
	cmp  ax,cx	
	ja   still ; max x

	add  dx,[wa_high_2]
	cmp  bx,dx
	ja   still ; max y

	mov  ecx,[mouse_keys]
	test ecx,1b  ; left button 
	jz   .no_capture ;still

	mov  ecx,[mouse_keys_old]
	test ecx,1b  ; left button old
	jz   .no_capture ;still

	mov  [capture_image_area_flag],1
	
	mov  ecx,[mouse_position_old]
	xor  edx,edx
	mov  dx,cx  ; EDX mouse y old
	shr  ecx,16 ; ECX mouse x old
	
	cmp  eax,ecx
	je   .y
	ja   @f
	sub  ecx,eax
	add  [scroll_bar_data_horizontal.position],ecx
	jmp  .y
@@:
	sub  eax,ecx
	cmp  eax,[scroll_bar_data_horizontal.position]
	jb   @f
	xor  eax,eax
	mov  [scroll_bar_data_horizontal.position],eax
	jmp  .y
@@:
	sub  [scroll_bar_data_horizontal.position],eax
	
.y:
	cmp  ebx,edx
	je   .draw
	ja   @f
	sub  edx,ebx
	add  [scroll_bar_data_vertical.position],edx
	jmp  .draw
@@:
	sub  ebx,edx
	
	cmp  ebx,[scroll_bar_data_vertical.position]
	jb   @f
	xor  eax,eax
	mov  [scroll_bar_data_vertical.position],eax
	jmp  .draw
@@:
	sub  [scroll_bar_data_vertical.position],ebx
	
.draw:
	call  drawimage
	call  draw_scrollbars
	jmp   still
	
.no_capture:
	mov  [capture_image_area_flag],0
.end:
	jmp   still
;---------------------------------------------------------------------