;---------------------------------------------------------------------
full_screen:
	xor  eax,eax
	mov  [Repeat_N],eax
	
	xor  eax,eax
	mov  [press_Enter_flag],al

	mov  eax,[zoom_factors]
	mov  [zoom_factors_old],eax

	mov  eax,[scaling_mode]
	mov  [scaling_mode_old],eax

	call get_window_param_1
	mov  eax,[window_start_x]
	mov  [window_start_x_old],eax
	mov  eax,[window_start_y]
	mov  [window_start_y_old],eax
	mov  eax,[window_high]
	mov  [window_high_old],eax
	mov  eax,[window_width]
	mov  [window_width_old],eax
;----------------------------------------
; get current work area
	mcall 48,5
	mov  [work_area_LR_old],eax
	mov  [work_area_TB_old],ebx
;----------------------------------------
	mov  eax,[soi]
	test eax,eax
	jz   still
	mov  eax,scaling_mode
	xor  ebx,ebx
	mov  [eax],ebx  ;dword 0
	mov  [zoom_factors],dword zoom_factors.1
	
	mcall 14
	xor  ebx,ebx
	xor  ecx,ecx
	mov  edx,eax
	shr  edx,16
	xor  esi,esi
	mov  si,ax
	inc  ax
	mov  [wa_high_1],ax
	mov  [wa_high_2],ax
	shr  eax,16
	inc  ax
	mov  [wa_width_1],ax
	mov  [wa_width_2],ax
	mcall 67
;----------------------------------------
; set max work area
	mov  ecx,edx
	mov  dx,si
	mcall 48,6
;----------------------------------------
	mov  eax,[fill_color]
	mov  [fill_color_old],eax
	xor  eax,eax
	mov  [fill_color],eax

	jmp  .still
;----------------------------------------
.red:
	call .draw_window
.still:
	cmp  [RAW1_flag],1
	je   .animation_handler
	mcall 10
	cmp  eax,1  ; перерисовать окно ?
	je     .red       ; если да - на метку red
	cmp  eax,2  ; нажата клавиша ?
	je     .key       ; если да - на key
	cmp  eax,3  ; нажата кнопка ?
	je     .button     ; если да - на button
	jmp  .still       ; если другое событие - в начало цикла
;----------------------------------------
.animation_handler:
	mcall 23,1
;	pusha
;	mcall 26,9
;	cmp   [Animation_timer],eax
;	ja   @f
;	call animation_handler.draw_image
;@@:
;	popa
	cmp  eax,1	; перерисовать окно ?
	je     .red       ; если да - на метку red
	cmp  eax,2	; нажата клавиша ?
	je     .key       ; если да - на key
	cmp  eax,3	; нажата кнопка ?
	je     .button     ; если да - на button
	mcall 26,9
	cmp   [Animation_timer],eax
	ja   .animation_handler
	
	mov al,[Disposal_Method]
	cmp al,2
	jne  @f
	call restore_background
	jmp  .first_frame
@@:
	cmp al,3
	jne  @f
	call restore_previous_frame
@@:
.first_frame:
	
	call copy_next_frame
	mov  [no_fill],1
	call .draw_image
	mov  [no_fill],0
;	call animation_handler.draw_image
	jmp  .still
;----------------------------------------
.key:
    mcall 2
	cmp   [extended_key],1
	je    .extended_key
	test  al, al
	jnz   .still
	cmp   ah, 0xE0
	jne   @f
	mov   [extended_key],1
	jmp   .still
@@:
	cmp   ah,1  ; Esc
	je    .restore_main_window
	cmp   ah,156 ; Enter
	je    .restore_main_window_1
	jmp  .still
;----------------------------------------
.extended_key:
    mov  [extended_key],0
	mov  [current_key_code],ah
	cmp  ah,73
	je   move_to_left_1
	cmp  ah,81
	je   move_to_right_1
	cmp  ah,71
	je   left_limit_1
	cmp  ah,79
	je   right_limit_1

	cmp   ah,156 ; Enter
	je    .restore_main_window_1

	cmp   ah,183 ; Print Screen  - Slides Show
	je    slide_show_1.3
	jmp  .still
;----------------------------------------
.button:
	mcall  17
	cmp  ah,1	     ; button id=1 ?
;       je  START.exit
;       jmp  .still  ;red_1
	jne  .still
.exit:
	call restore_work_area
	jmp  START.exit
;---------------------------------------------------------------------
.restore_main_window_1:
	cmp  [press_Enter_flag],1
	je   .restore_main_window
	inc  [press_Enter_flag]
	jmp  .still
;---------------------------------------------------------------------
.restore_main_window:
	call restore_work_area
	
	mov  eax,[zoom_factors_old]
	mov  [zoom_factors],eax

	mov  eax,[scaling_mode_old]
	mov  [scaling_mode],eax

	mov	ebx,[window_start_x_old]
	mov	ecx,[window_start_y_old]
	mov	edx,[window_width_old]
	mov	esi,[window_high_old]
	mcall	67

;	call get_window_param
	mov  eax,[fill_color_old]
	mov  [fill_color],eax

	xor  eax,eax
	mov  [Repeat_N],eax
	
;	mcall 15,3

	mov  [redraw_wallpaper_flag],1
	jmp  still  ;red_1
;---------------------------------------------------------------------
.draw_window:
	mcall 12,1
	mcall 0,<0,800>,<0,600>,0x41000000,0,0
;	call get_window_param_1
;       mcall 8,0,0,1,0
	call  .draw_image
	mcall 12,2
	ret
;---------------------------------------------------------------------
.draw_image:
	mcall 14
	inc  ax
	mov  [wa_high_1],ax
	mov  [wa_high_2],ax
	shr  eax,16
	inc  ax
	mov  [wa_width_1],ax
	mov  [wa_width_2],ax
	mov  [image_start_coordinates],0
	mov  [background_color],0
	call scaling.1
	mov  [background_color],0xffffff
	ret
;---------------------------------------------------------------------
get_window_param_1:
	mcall 9, procinfo, -1
	mov	eax,[ebx+34]
	mov	[window_start_x],eax
	mov	eax,[ebx+38]
	mov	[window_start_y],eax
	mov	eax,[ebx+46]
	mov	[window_high],eax
	mov	eax,[ebx+42]
	mov	[window_width],eax
	mov	eax,[ebx+70]
	mov	[window_status],eax
	ret
;----------------------------------------
restore_work_area:
; restore old work area
	mov  ecx,[work_area_LR_old]
	mov  edx,[work_area_TB_old]
	mcall 48,6
	ret
;---------------------------------------------------------------------
move_to_left_1:
	xor   eax,eax
	cmp   [load_file_number],eax
	je    full_screen.still
	dec   [load_file_number]
	jmp  right_limit_1.2
;----------------------------------------
move_to_right_1:
	mov   eax,[load_directory_pointer]
	mov   eax,[eax+4]
	test  eax,eax
	jz    @f
	dec   eax
@@:
	cmp   [load_file_number],eax
	je    full_screen.still
	inc   [load_file_number]
	jmp  right_limit_1.2
;----------------------------------------
left_limit_1:
	xor  eax,eax
	jmp  right_limit_1.1
;----------------------------------------
right_limit_1:
	mov   eax,[load_directory_pointer]
	mov   eax,[eax+4]
	test  eax,eax
	jz    @f
	dec   eax
@@:
.1:
	cmp   [load_file_number],eax
	je    full_screen.still
	mov  [load_file_number],eax
.2:
	imul  ebx,[load_file_number],304
	add   ebx,32+40
	add   ebx,[load_directory_pointer]

	mov   esi,string
	mov   edi,plugin_pach
	call  copy_file_path

	mov  esi,plugin_pach
	mov  edi,string
	mov  ecx,4096/4
	cld
	rep  movsd

;       jmp   kopen
	mov  [open_file_flag],0
	mov ecx,-1
	call  getappinfo
	call  load_image
	test  eax,eax
	jnz    kopen_1.err

	call  convert

	call load_directory.copy_dir_path

;	call  get_window_param
;	test  [window_status],1b
;	jz    @f ;draw_still
;	mov   [no_draw_window],1
;***    call  draw_other
;	mov   [no_draw_window],0
;	jmp   .3
;
;@@:
;	call .draw_window
	call  full_screen.draw_image
.3:
	mov  al,[old_key_code]
	mov  ah,[current_key_code]
	mov  [old_key_code],ah
	cmp  al,ah
	jne  full_screen.still  ;key.extended_key  ;still
.still:
	mcall 2
	cmp   [extended_key],1
	je    .extended_key
	test  al, al
	jnz   full_screen.still
	cmp   ah, 0xE0
	jne   @f
	mov   [extended_key],1
	jmp   .still
@@:
	jmp   full_screen.still
.extended_key:
	mov  [extended_key],0
	mov  [current_key_code],ah
	jmp  .3
.4:
	mov  ah,[current_key_code]
	mov  [old_key_code],ah
	jmp  full_screen.extended_key  ;key.extended_key
;---------------------------------------------------------------------
slide_show_1:
	mov   eax,[load_directory_pointer]
	cmp   [eax+4],dword 1
	jbe   full_screen.still
	mcall 26,9
	cmp   [timer],eax
	jbe   .1
	jmp   .still
.red:
	call full_screen.draw_window
.still:
	mcall 23,10
	cmp  eax,1
	je     .red
	cmp  eax,2
	je     .key
	cmp  eax,3
	je     .button
;	mcall 68,1
	jmp   slide_show_1
;-------------------------------------------
.key:
	mcall 2
	cmp   [extended_key],1
	je    .extended_key
	test  al, al
	jnz   .still
	cmp   ah, 0xE0
	jne   @f
	mov   [extended_key],1
	jmp   .still
@@:
	cmp   ah,1  ; Esc
	je    full_screen.still
	cmp   ah,185  ; Space
	je    full_screen.still
	jmp  .still
.extended_key:
	mov  [extended_key],0
	mov  [current_key_code],ah
	jmp   .still
;-------------------------------------------
.button:			; button
	mov  eax,17	  ; get id
	mcall
	cmp  ah,1	     ; button id=1 ?
	je  full_screen.exit  ;START.exit
.noclose:
	cmp  ah,2
	je   full_screen.still
	jmp  slide_show_1
;-------------------------------------------
.1:
;	mcall 26,9
;	add  eax,[slide_show_delay]   ;500
;	mov   [timer],eax

	mov   eax,[load_directory_pointer]
	mov   eax,[eax+4]
	test  eax,eax
	jz    @f
	dec   eax
@@:
	cmp   [load_file_number],eax
	je    full_screen.still
	inc   [load_file_number]
	imul  ebx,[load_file_number],304
	add   ebx,32+40
	add   ebx,[load_directory_pointer]

	mov   esi,string
	mov   edi,plugin_pach
	call  copy_file_path

	mov  esi,plugin_pach
	mov  edi,string
	mov  ecx,4096/4
	cld
	rep  movsd
	mov  [open_file_flag],0
	mov ecx,-1
	call  getappinfo
	call  load_image

	test  eax,eax
	jnz    kopen_1.err

	call  convert
;	call  get_window_param
;	test  [window_status],1b
;	jz    @f ;draw_still
;       mov   [no_draw_window],1
;	call  draw_other
;       mov   [no_draw_window],0
;       jmp   slide_show_1  ;.3
;@@:
;	call draw_window
	call load_directory.copy_dir_path
	call  full_screen.draw_image
;	jmp   slide_show_1
.3:
	mcall 26,9
	add  eax,[slide_show_delay]  ;500
	mov   [timer],eax
	jmp  slide_show_1
;---------------------------------------------------------------------
