;---------------------------------------------------------------------
  key:				; key
    mcall 2
    cmp   [extended_key],1
    je	  .extended_key
    test  al, al
    jnz   still
    cmp   ah, 0xE0
    jne   @f
    mov   [extended_key],1
    jmp   still
@@:
    cmp   ah,151  ; i - file information
	je    kinfo
    cmp   ah,152  ; o - application options
	je    check_key_O ;k_option
    cmp   ah,159  ; s - sort mode
	je    k_sort_directory
    cmp   ah,145  ; w - wallpaper
	je    k_background
    cmp   ah,183  ; * - scaling to window
	je    scaling
    cmp   ah,139  ; 0 - scaling to window
	je    scaling
    cmp   ah,202  ; "-" - zoom minus
	je    zoom_minus
    cmp   ah,140  ; "-" - zoom minus
	je    zoom_minus
    cmp   ah,206  ; "+" - zoom plus
	je    zoom_plus
    cmp   ah,141  ; "+" - zoom plus
	je    zoom_plus
    cmp   ah,171  ; "\" - zoom 100%
	je    no_scaling
	
	cmp   ah,72 ; up arrow
	je    key_up_arrow
	cmp   ah,80 ; down arrow
	je    key_down_arrow
	cmp   ah,75 ; left arrow
	je    key_left_arrow
	cmp   ah,77 ; right arrow
	je    key_right_arrow
	
	cmp   ah,28 ; Enter
	je    full_screen
	cmp	ah,187	; F1
	je	select_menu ;key_alt_down

	cmp	ah,42
	je	key_shift_up
	cmp	ah,54
	je	key_shift_up
	cmp	ah,170
	je	key_shift_down
	cmp	ah,182
	je	key_shift_down
	cmp	ah,29
	je	key_ctrl_up
	cmp	ah,157
	je	key_ctrl_down
	cmp	ah,56
	je	key_alt_up
	cmp	ah,184
	je	key_alt_down
	
    jmp  still	
.extended_key:
    mov  [extended_key],0
	mov  [current_key_code],ah
;	push  eax
;	mcall 26,9
;	mov  [timer_counter],eax
;	pop   eax

;    mov  al,ah
;    cmp  al,130  ; 1
;    je	 kfile
;    cmp  al,131  ; 2
;    je	 kopen
;    cmp  al,132  ; 3
;    je	 kinfo
;    cmp  al,133  ; 4
;    je	 kbgrd
	cmp  ah,73
	je   move_to_left
	cmp  ah,81
	je   move_to_right
	cmp  ah,71
	je   left_limit
	cmp  ah,79
	je   right_limit
    cmp   ah,181  ; / - zoom 100%
	je    no_scaling
	
	cmp   ah,72 ; up arrow
	je    key_up_arrow
	cmp   ah,80 ; down arrow
	je    key_down_arrow
	cmp   ah,75 ; left arrow
	je    key_left_arrow
	cmp   ah,77 ; right arrow
	je    key_right_arrow
	
	cmp   ah,28 ; Enter
	je    full_screen
	
	cmp   ah,183 ; Print Screen  - Slides Show
	je    slide_show.3

	cmp	ah,29
	je	key_ctrl_up
	cmp	ah,157
	je	key_ctrl_down
	cmp	ah,56
	je	key_alt_up
	cmp	ah,184
	je	key_alt_down
    jmp  still
;---------------------------------------------------------------------
key_shift_up:
	mov	[shift_flag],1
	jmp	still
;---------------------------------------------------------------------
key_shift_down:
	mov	[shift_flag],0
	jmp	still
;---------------------------------------------------------------------
key_ctrl_up:
	mov	[ctrl_flag],1
	jmp	still
;---------------------------------------------------------------------
key_ctrl_down:
	mov	[ctrl_flag],0
	jmp	still
;---------------------------------------------------------------------
key_alt_up:
	mov	[alt_flag],1
	jmp	still
;---------------------------------------------------------------------
key_alt_down:
	mov	[alt_flag],0
	jmp	still
;---------------------------------------------------------------------
check_key_O:
	cmp	[ctrl_flag],0
	jne	kfile
	jmp	k_option
;---------------------------------------------------------------------
move_to_left:
	xor   eax,eax
	cmp   [load_file_number],eax
	je    still
    dec   [load_file_number]
	jmp  right_limit.2
;----------------------------------------    
move_to_right:
	mov   eax,[load_directory_pointer]
	mov   eax,[eax+4]
	test  eax,eax
	jz    @f
	dec   eax
@@:
	cmp   [load_file_number],eax
	je    still
    inc   [load_file_number]
	jmp  right_limit.2
;----------------------------------------
left_limit:
	xor  eax,eax
	jmp  right_limit.1
;----------------------------------------
right_limit:
	mov   eax,[load_directory_pointer]
	mov   eax,[eax+4]
	test  eax,eax
	jz    @f
	dec   eax
@@:
.1:
	cmp   [load_file_number],eax
	je    still
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

;	jmp   kopen
    mov  [open_file_flag],0
    mov ecx,-1
    call  getappinfo
    call  load_image
	test  eax,eax
	jnz    kopen_1.err
	
    call  convert
	
	call load_directory.copy_dir_path	
	
    call  get_window_param
    test  [window_status],1b
    jz    @f ;draw_still
	mov   [no_draw_window],1
    call  draw_other
	mov   [no_draw_window],0
	jmp   .3
	
@@:
    call draw_window
.3:
	mov  al,[old_key_code]
	mov  ah,[current_key_code]
	mov  [old_key_code],ah
	cmp  al,ah
	jne  still  ;key.extended_key  ;still
.still:
    mcall 2
    cmp   [extended_key],1
    je	  .extended_key
    test  al, al
    jnz   still
    cmp   ah, 0xE0
    jne   @f
    mov   [extended_key],1
    jmp   .still
@@:
    jmp   still	
.extended_key:
    mov  [extended_key],0
	mov  [current_key_code],ah
	jmp  .3
.4:
	mov  ah,[current_key_code]
	mov  [old_key_code],ah
	jmp  key.extended_key
;---------------------------------------------------------------------
slide_show:
	mov   eax,[load_directory_pointer]
	cmp   [eax+4],dword 1
	jbe   still
    mcall 26,9
	cmp   [timer],eax
	jbe   .1
	jmp   .still
.red:
    call draw_window
.still:
	mcall 23,10
    cmp  eax,1	      ; перерисовать окно ?
    je	   .red
    cmp  eax,2	      ; нажата клавиша ?
    je	   .key	      ; если да - на key
    cmp  eax,3	      ; нажата кнопка ?
    je	   .button     ; если да - на button
;	mcall 68,1
	jmp   slide_show
;-------------------------------------------
.key:
    mcall 2
    cmp   [extended_key],1
    je	  .extended_key
    test  al, al
    jnz   .still
    cmp   ah, 0xE0
    jne   @f
    mov   [extended_key],1
    jmp   .still
@@:
    cmp   ah,1  ; Esc
	je    still
    cmp   ah,185  ; Space
	je    still
	jmp  .still
.extended_key:
    mov  [extended_key],0
	mov  [current_key_code],ah
	jmp   .still
;-------------------------------------------
.button:			; button
    mov  eax,17 		; get id
    mcall
    cmp  ah,1		  ; button id=1 ?
	je  START.exit
.noclose:
    cmp  ah,2
	je   still
	jmp  slide_show
;-------------------------------------------
.1:
;    mcall 26,9
;	add  eax,[slide_show_delay]   ;500
;	mov   [timer],eax
	
	mov   eax,[load_directory_pointer]
	mov   eax,[eax+4]
	test  eax,eax
	jz    @f
	dec   eax
@@:
	cmp   [load_file_number],eax
	je    still
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
	call load_directory.copy_dir_path
	
    call  get_window_param
    test  [window_status],1b
    jz    @f ;draw_still
	mov   [no_draw_window],1
    call  draw_other
	mov   [no_draw_window],0
	jmp   .3  ;slide_show
@@:
    call draw_window
;	jmp   slide_show
.3:
    mcall 26,9
	add  eax,[slide_show_delay]  ;500
	mov   [timer],eax
    jmp  slide_show
;---------------------------------------------------------------------
key_up_arrow:
	mov  eax,[scroll_bar_data_vertical.ar_offset]
	mov  ebx,scroll_bar_data_vertical.position
	cmp  [ebx],eax
	jbe  @f
	sub  [ebx],eax
	jmp  .1
@@:
	xor   eax,eax
	cmp  [ebx],eax
	je   still
	mov   [ebx],eax
.1:
	call  drawimage
	call  draw_scrollbars
	jmp   still
;---------------------------------------------------------------------
key_down_arrow:
	mov  ecx,[scroll_bar_data_vertical.ar_offset]
	mov  ebx,scroll_bar_data_vertical.position
	mov  eax,[scroll_bar_data_vertical.max_area]
	sub  eax,[scroll_bar_data_vertical.cur_area]
	cmp  [ebx],eax
	je   still
	push eax
	sub  eax,ecx
	cmp  [ebx],eax
	pop  eax
	jae  @f
	add  [ebx],ecx
	jmp  key_up_arrow.1
@@:
	mov  [ebx],eax
	jmp  key_up_arrow.1
;---------------------------------------------------------------------
key_left_arrow:
	mov  eax,[scroll_bar_data_horizontal.ar_offset]
	mov  ebx,scroll_bar_data_horizontal.position
	cmp  [ebx],eax
	jbe  @f
	sub  [ebx],eax
	jmp  key_up_arrow.1
@@:
	xor   eax,eax
	cmp  [ebx],eax
	je   still
	mov   [ebx],eax
	jmp  key_up_arrow.1
;---------------------------------------------------------------------
key_right_arrow:
	mov  ecx,[scroll_bar_data_horizontal.ar_offset]
	mov  ebx,scroll_bar_data_horizontal.position
	mov  eax,[scroll_bar_data_horizontal.max_area]
	sub  eax,[scroll_bar_data_horizontal.cur_area]
	cmp  [ebx],eax
	je   still
	push eax
	sub  eax,ecx
	cmp  [ebx],eax
	pop  eax
	jae  @f
	add  [ebx],ecx
	jmp  key_up_arrow.1
@@:
	mov  [ebx],eax
	
;	xor   eax,eax
;	mov   ax,[scroll_bar_data_horizontal.size_x]
;	sub   eax,[scroll_bar_data_horizontal.btn_high]
;	sub   eax,[scroll_bar_data_horizontal.btn_high]
;	sub   eax,[scroll_bar_data_horizontal.run_size]
;	mov   [scroll_bar_data_horizontal.position2],eax
	jmp  key_up_arrow.1
;---------------------------------------------------------------------