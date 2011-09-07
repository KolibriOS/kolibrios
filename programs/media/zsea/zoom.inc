;---------------------------------------------------------------------
invert_left_to_right:
	mov ebx,3
	jmp  rotate_counter_clockwise.1
;---------------------------------------------------------------------
invert_up_to_down:
	mov ebx,4
	jmp  rotate_counter_clockwise.1
;---------------------------------------------------------------------
rotate_clockwise:
	mov ebx,1
	jmp  rotate_counter_clockwise.1
;---------------------------------------------------------------------
rotate_counter_clockwise:
	mov ebx,2
.1:
	mov  eax,[soi]
	test eax,eax
	jz   still
	
	mov  eax,[RAW1_pointer]
	test eax,eax
	jz   @f
	cmp  [Total_Repeat_N],1
	ja   still
@@:

	push  ebx
	call  proc_position_correct
	mov  eax,[vertical_position_coorect]
	mov  ebx,[horizontal_position_coorect]
	mov  [horizontal_position_coorect],eax
	mov  [vertical_position_coorect],ebx
	pop  ebx
;	mov eax,image_file
; ebx - direction
; 1 - clockwise, 2 - counter clockwise
; 3 - Left&Right, 4 - Up&Down
	push	ebx
	push	dword image_file
;	call [plugin_rotate]
	call [rotate_Start]
	mov  ebx,[raw_pointer]
	mov  eax,[ebx+4]
	mov  [img_width],eax
	mov  eax,[ebx+8]
	mov  [img_high],eax
	call convert.img_resolution_ok
	jmp  kopen.1
;---------------------------------------------------------------------
calculate_arrows_offset:
	mov  eax,[arrows_offset]
	imul eax,[scaling_mode]
	mov  ebx,100
	xor  edx,edx
	div  ebx
	mov  [scroll_bar_data_vertical.ar_offset],eax
	mov  [scroll_bar_data_horizontal.ar_offset],eax
	ret
;---------------------------------------------------------------------
no_scaling:
	mov  eax,scaling_mode
	cmp  [eax],dword 100
	je   still
.1:
	mov  [eax],dword 100
	mov  [zoom_factors],dword zoom_factors.100
	mov  eax,[eax]
	jmp   zoom_plus.2
;---------------------------------------------------------------------
scaling:
	mov  eax,[soi]
	test eax,eax
	jz   still
	mov  eax,scaling_mode
	xor  ebx,ebx
	cmp  [eax],ebx  ;dword 0
	je   still
	mov  [eax],ebx  ;dword 0
	mov  [zoom_factors],dword zoom_factors.1
	call .1
	call draw_zoom_factors
	jmp  still
;---------------------------
.compens_x:
	call .get_x_y
	xor  ecx,ecx
	mov  cx,[wa_high_1]
	imul eax,ecx
	xor  edx,edx
	div  ebx
	ret
;---------------------------
.compens_y:
	call .get_x_y
	xchg eax,ebx
	xor  ecx,ecx
	mov  cx,[wa_width_1]
	imul eax,ecx
	xor  edx,edx
	div  ebx
	ret
;---------------------------
.get_x_y:
    mov  ebx,[raw_pointer]
    mov  eax,[ebx+4]
	mov  ebx,[ebx+8]
	ret
;---------------------------
.fill_for_L_R:
	cmp   [no_fill],1
	je   @f
	pusha
	mov   ebx,[image_start_coordinates]
	mov   ecx,ebx
	shl   ecx,16
	mov   bx,ax
	mov   cx,[wa_high_1]
	mcall 13,,,[fill_color] ;0xffffff
	mov   ax,bx
	ror   ebx,16
	add   bx,ax
	add   bx,[wa_width_2]
	mov   ax,[wa_width_1]
	sub   ax,bx
	rol   ebx,16
	mov   bx,ax
	mcall 13,,,[fill_color]  ;0xffffff
	popa
@@:
	ret
;---------------------------
.fill_for_U_D:
	cmp   [no_fill],1
	je   @f
	pusha
	mov   ebx,[image_start_coordinates]
	mov   ecx,ebx
	shl   ecx,16
	mov   cx,ax
	mov   bx,[wa_width_1]
	mcall 13,,,[fill_color]  ;0xffffff
	mov   ax,cx
	ror   ecx,16
	add   cx,ax
	add   cx,[wa_high_2]
	mov   ax,[wa_high_1]
	add   ax,28  ;50
	sub   ax,cx
	rol   ecx,16
	mov   cx,ax
	mcall 13,,,[fill_color]  ;0xffffff
	popa
@@:
	ret
;---------------------------
.1:
	call .get_x_y
	cmp  eax,ebx
	jb  .y
;---------------------------
.x:
	call .compens_y
	cmp  ax,[wa_high_1]
	jbe @f
	call .compens_x
	mov  [wa_width_2],ax
	jmp  .4
@@:
	mov  [wa_high_2],ax
	jmp  .4
;---------------------------
.y:
	call .compens_x
	cmp  ax,[wa_width_1]
	jbe  @f
	call .compens_y
	mov  [wa_high_2],ax
	jmp  .4
@@:
	mov  [wa_width_2],ax
;---------------------------
.4:
	mov bx,[wa_width_2]
	shl ebx,16
	mov bx,[wa_high_2]

	xor  ecx,ecx
.5:

;	mov  eax,ebx
;	and  eax,0xffff
;	mov  [test1],eax

;	mov  eax,ebx
;	shr  eax,16
;	mov  [test2],eax

;	mov  eax,ecx
;	and  eax,0xffff
;	mov  [test3],eax

;	mov  eax,ecx
;	shr  eax,16
;	mov  [test4],eax

;	mov eax,image_file
;	mov  edx,[scaling_mode]
	movzx  esi,byte [filtering_flag]
;	mov  edi,[background_color]

	push	dword 0				; eax - crop size
	push	dword [background_color]	; edi - background color
	push	esi				; esi - filtering
	push	dword [scaling_mode]		; edx - scaling mode
	push	ecx				; ecx - start_coordinates
	push	ebx				; ebx - new_size
	push	dword image_file		; eax -  convert data table

;	pusha
;	mcall 26,9
;	mov  [test1],eax
;	popa

	call [Scaling_Start]

;	pusha
;	mcall 26,9
;	mov  ebx,[test1]
;	sub  eax,ebx
;	cmp  [test2],0
;	je  @f
;	add  eax,[test2]
;	shr  eax,1
;	cmp  eax,[test3]
;	jbe  @f
;	mov  [test3],eax
;@@:
;	mov  [test2],eax
;	popa

	mov  ecx,ebx

	mov  edx,[image_start_coordinates]

	mov  eax,edx
	shl  eax,16
	mov  ax,cx
	mov  [scroll_bar_data_vertical.y],eax
	
	mov  eax,edx
	mov  ebx,ecx
	shr  ebx,16
	mov  ax,bx
	mov  [scroll_bar_data_horizontal.x],eax

;	call correct_cur_area

;	movzx eax,[wa_width_1]
;	mov  [test1],eax
;	movzx eax,[wa_width_2]
;	mov  [test2],eax

;	movzx eax,[wa_high_1]
;	mov  [test3],eax
;	movzx eax,[wa_high_2]
;	mov  [test4],eax

	xor  eax,eax
	mov  ax,[wa_width_1]
	sub  ax,[wa_width_2]
	shr  ax,1
	test ax,ax
	jz  @f
	shl  eax,16
	add  edx,eax
	shr  eax,16
	call .fill_for_L_R
@@:
	mov  ax,[wa_high_1]
	sub  ax,[wa_high_2]
	shr  ax,1
	test ax,ax
	jz  @f
	add  edx,eax
	call .fill_for_U_D
@@:
;	mov   esi,[img_resolution]
;	cmp   [img_resolution_2],15
;	jne  @f
;	mov  esi,15
;@@:
	mov	eax,[raw_pointer_2]
	mov	esi,[eax+12]
	
	mov  eax,[raw_pointer]
	mov  edi,[eax+20]
	add  edi,eax
;	mov  edi,[img_palette]

	xor  ebp,ebp

	mov  ebx,[raw_pointer_2]
	mov	eax,[ebx+28]
	add	ebx,eax

	mov  eax,ecx
	test ax,ax
	jz   .no_draw  ;@f
	shr  eax,16
	test eax,eax
	jz   .no_draw ;@f

;	cmp  [filtering_flag],0
;	je   @f
;	cmp  esi,8
;	jne  @f
;	mov  esi,24
;@@:
	mcall 65
.no_draw:
;	mov  eax,ecx
;	shr  eax,16
;	mov  [test1],eax
;	
;	mov  eax,ecx
;	and  eax,0xffff
;	mov  [test2],eax
;
;	mov  eax,edx
;	shr  eax,16
;	mov  [test3],eax
;	
;	mov  eax,edx
;	and  eax,0xffff
;	mov  [test4],eax

	call draw_file_name_to_work_area
	mcall 68,13,[raw_pointer_2]
	
	
	xor  eax,eax
	mov  ax,[wa_width_2]
	
	mov  ebx,[img_width]
;	imul eax,100
	call	calculate_relation
	mov  [zoom_auto_factor],eax
	
	ret
;---------------------------------------------------------------------
calculate_relation:
; relation = X*100/Y
; eax = eax*100/ebx
; ЕАХ = ЕАХ * 100
	shl	eax,2      ; умножение на 4
	lea	eax,[eax+eax*4] ; умножение на 5
	lea	eax,[eax+eax*4] ; умножение на 5
integer_division:
; eax = eax/ebx
	test	ebx,ebx
	jnz	@f
	inc	ebx
@@:
	xor	edx,edx
	div	ebx
;	shl	edx,1
;	cmp	ebx,edx
;	jb	@f
;	inc	eax
;@@:
	ret
;---------------------------------------------------------------------
calculation_next_value_plus:
	mov  ebx,[zoom_auto_factor]
	mov  ecx,zoom_factors.min
	sub  ecx,4
@@:
	add  ecx,4
	cmp  ecx,dword zoom_factors.max
	jae  .max	
	mov  eax,[ecx]
	cmp  eax,ebx
	jb   @r
@@:
	mov  eax,zoom_factors
	mov  [eax],ecx
;	jmp  zoom_plus.1
	ret
.max:
	mov  ecx,dword zoom_factors.max
	jmp  @r
;---------------------------------------------------------------------
calculation_next_value_minus:
	mov  ebx,[zoom_auto_factor]
	mov  ecx,zoom_factors.max
	add  ecx,4
@@:
	sub  ecx,4
	cmp  ecx,dword zoom_factors.min
	jbe  .min	
	mov  eax,[ecx]
	cmp  eax,ebx
	ja   @r
@@:
	mov  eax,zoom_factors
	mov  [eax],ecx
;	jmp  zoom_plus.1
	ret
.min:
	mov  ecx,dword zoom_factors.min
	jmp  @r
;---------------------------------------------------------------------
proc_position_correct:
	mov  [position_coorect],1
	mov  eax,[scroll_bar_data_vertical.max_area]
	xor  ebx,ebx
	mov  bx,[wa_high_1]
	shr  ebx,1
	shl  eax,8
	add  ebx,[scroll_bar_data_vertical.position]
	test ebx,ebx
	jnz  @f
	inc  ebx
@@:
	xor  edx,edx
	div  ebx
	mov  [vertical_position_coorect],eax

	mov  eax,[scroll_bar_data_horizontal.max_area]
	xor  ebx,ebx
	mov  bx,[wa_width_1]
	shr ebx,1
	shl  eax,8
	add  ebx,[scroll_bar_data_horizontal.position]
	test ebx,ebx
	jnz  @f
	inc  ebx
@@:
	xor  edx,edx
	div  ebx
	mov  [horizontal_position_coorect],eax
	ret
;---------------------------------------------------------------------
zoom_plus:
;	cmp  [scaling_mode],dword 0
	mov  eax,[scaling_mode]
	test eax,eax
;	jz   no_scaling.1
;	jz   calculation_next_value_plus
	jnz  @f
	call  calculation_next_value_plus
	jmp  .1
@@:
;	jne  @f
;	mov  [scaling_mode],dword 100
;	mov  [zoom_factors],dword zoom_factors.100
;	call  draw_zoom_factors
;	jmp  red
;@@:
	mov  eax,zoom_factors
	cmp  [eax],dword zoom_factors.max
	jae  still
	add  [eax],dword 4
.1:
	mov  eax,[eax]
	mov  eax,[eax]
.2:	
	mov  [scaling_mode],eax

	mov  [correct_scrollbar_100_flag],1
	call calculate_arrows_offset
	
	call  proc_position_correct

    call  drawimage
	
	call  set_draw_scrollbar_all
    call  draw_scrollbars
	call  set_nodraw_scrollbar_all
	
	call  draw_zoom_factors
	jmp  still

zoom_all:
;******************************
	mov  eax,wa_width_1
	mov  eax,[eax]
	push eax
	mov  eax,wa_high_1
	mov  eax,[eax]
	push eax
;******************************
	cmp  [position_coorect],0
	je   .correct_end
	mov  [position_coorect],0
	mov  eax,[scroll_bar_data_vertical.max_area]
;	xor  ebx,ebx
;	mov  bx,[wa_high_1]
;	shr  ebx,1
;	add  eax,ebx  ;[scroll_bar_data_vertical.cur_area]
	shl  eax,8
	mov  ebx,[vertical_position_coorect]
	test ebx,ebx
	jnz  @f
	inc  ebx
@@:
	xor  edx,edx
	div  ebx
;	shr  eax,8
	xor  ebx,ebx
	mov  bx,[wa_high_1]
	shr  ebx,1
	cmp  eax,ebx
	jae  @f
	xor  eax,eax
	jmp  .position_vertical_correct
@@:
	sub  eax,ebx
.position_vertical_correct:
	mov  [scroll_bar_data_vertical.position],eax
	
	mov  eax,[scroll_bar_data_horizontal.max_area]
;	xor  ebx,ebx
;	mov  bx,[wa_width_1]
;	shr  ebx,1
;	add  eax,ebx  ;[scroll_bar_data_horizontal.cur_area]
	shl  eax,8
	mov  ebx,[horizontal_position_coorect]
	test ebx,ebx
	jnz  @f
	inc  ebx
@@:
	xor  edx,edx
	div  ebx
;	shr  eax,8
	xor  ebx,ebx
	mov  bx,[wa_width_1]
	shr  ebx,1
	cmp  eax,ebx
	jae  @f
	xor  eax,eax
	jmp  .position_horizontal_correct
@@:
	sub  eax,ebx
.position_horizontal_correct:
	mov  [scroll_bar_data_horizontal.position],eax
	
.correct_end:

;****************************************************************************
; Vertical SCROLLBAR
; At a case of resizing of the window
; Check the position of a runner has crossed a limit
;****************************************************************************
    mov  eax,[scroll_bar_data_vertical.max_area]
	xor  ebx,ebx
	mov  bx,[wa_high_1]
	cmp  eax,ebx
	ja   @f
	xor  eax,eax
	jmp  .p_5
@@:
    sub  eax,ebx  ;[scroll_bar_data_vertical.cur_area]
    cmp  eax,[scroll_bar_data_vertical.position]
    jae  @f
.p_5:
    mov  [scroll_bar_data_vertical.position],eax
@@:
;****************************************************************************
; Horizontal SCROLLBAR
; At a case of resizing of the window
; Check the position of a runner has crossed a limit
;****************************************************************************
    mov  eax,[scroll_bar_data_horizontal.max_area]
	xor  ebx,ebx
	mov  bx,[wa_width_1]
	cmp  eax,ebx
	ja   @f
	xor  eax,eax
	jmp  .p_6
@@:
    sub  eax,ebx  ;[scroll_bar_data_horizontal.cur_area]
    cmp  eax,[scroll_bar_data_horizontal.position]
    jae  @f
.p_6:
    mov  [scroll_bar_data_horizontal.position],eax
@@:
;****************************************************************************

	mov   eax,[scroll_bar_data_horizontal.max_area]
	mov   ebx,[scroll_bar_data_vertical.max_area]


	sub   eax,[scroll_bar_data_horizontal.position]   ;100
	sub   ebx,[scroll_bar_data_vertical.position]    ;100

	xor   ecx,ecx
	mov   cx,word [wa_width_1]
	cmp   eax,ecx
	jbe   @f
	xor   eax,eax
	mov   ax,[wa_width_1]
@@:
	mov   [wa_width_2],ax
	mov   ecx,eax
	shl   ecx,16
	xchg  eax,ebx

	xor   edx,edx
	mov   dx,word [wa_high_1]
	cmp   eax,edx
	jbe   @f
	xor   eax,eax
	mov   ax,[wa_high_1]
@@:
	mov   [wa_high_2],ax
	mov   cx,ax

	call  correct_cur_area

	mov   ebx,ecx

	push eax
	mov  eax,[scroll_bar_data_vertical.max_area]
	cmp  eax,[scroll_bar_data_vertical.cur_area]
	jbe  .horizontal  ;@f
	mov  ax,[wa_width_1]
	sub  ax,[scroll_bar_data_vertical.size_x]
	mov  [wa_width_1],ax
	cmp  ax,[wa_width_2]
	jae  .horizontal
	mov  [wa_width_2],ax
.horizontal:
	mov  eax,[scroll_bar_data_horizontal.max_area]
	cmp  eax,[scroll_bar_data_horizontal.cur_area]
	jbe  .finish   ;@f
	mov  ax,[wa_high_1]
	sub  ax,[scroll_bar_data_horizontal.size_y]
	mov  [wa_high_1],ax
	cmp  ax,[wa_high_2]
	jae  .finish
	mov  [wa_high_2],ax
.finish:   ;@@:
	pop  eax

	mov  cx,[wa_width_2]
	shl ecx,16
	mov  cx,[wa_high_2]

	call  correct_cur_area

	mov   ebx,ecx

	mov  eax,[scroll_bar_data_horizontal.position]

;	imul eax,100
; ЕАХ = ЕАХ * 100
    shl eax,2      ; умножение на 4
    lea eax,[eax+eax*4] ; умножение на 5
    lea eax,[eax+eax*4] ; умножение на 5
	push ebx
	mov  ebx,[scaling_mode]
	xor  edx,edx
	div  ebx

;	shl  edx,1
;	cmp  edx,ebx
;	jb  @f
;	inc  eax
;@@:

	pop  ebx

	mov  ecx,eax
	shl  ecx,16

	mov  eax,[scroll_bar_data_vertical.position]

;	imul eax,100
; ЕАХ = ЕАХ * 100
    shl eax,2      ; умножение на 4
    lea eax,[eax+eax*4] ; умножение на 5
    lea eax,[eax+eax*4] ; умножение на 5
	push ebx
	mov  ebx,[scaling_mode]
	xor  edx,edx
	div  ebx

;	shl  edx,1
;	cmp  edx,ebx
;	jb  @f
;	inc  eax
;@@:

	pop  ebx

	mov  cx,ax

	call   scaling.5
;******************************
	pop  eax
	mov  ebx,wa_high_1
	mov  [ebx],eax

	pop  eax
	mov  ebx,wa_width_1
	mov  [ebx],eax
;******************************

	ret
;---------------------------------------------------------------------
zoom_minus:
;	cmp  [scaling_mode],dword 0
	mov  eax,[scaling_mode]
	test eax,eax
;	jz   no_scaling.1
;	jz   calculation_next_value_minus
	jnz  @f
	call  calculation_next_value_minus
	jmp  zoom_plus.1
@@:
;	je   no_scaling.1
;	jne  @f
;	mov  [scaling_mode],dword 100
;	mov  [zoom_factors],dword zoom_factors.100
;	call  draw_zoom_factors
;	jmp  red
;@@:
	mov  eax,zoom_factors
	cmp  [eax],dword zoom_factors.min
	jbe  still
	sub  [eax],dword 4
	jmp   zoom_plus.1
;---------------------------------------------------------------------