;;****************************************************************************
;****************************************************************************
; Draw output part of the image and SCROLLBAR's
;****************************************************************************
;****************************************************************************
  drawimage:

  
    call get_window_param
    ;mov  edx,5*65536+50
	mov  edx,0*65536+28
    mov  esi,[img_resolution]    ;24
    cmp  esi,32
    ja   .exit_error ;.exit
    mov  edi,[img_palette]
;****************************************************************************
; Calculation of sizes of an output part of the image
;****************************************************************************
    xor  eax,eax
    mov  [draw_SB],al
    mov  [draw_SB1],al
    mov  eax,[window_width] 
;    sub  eax,9
    mov  [wa_width],ax
    mov  [wa_width_1],ax
    mov  [wa_width_2],ax
    mov  ebx,[img_width]
    test ebx,ebx
    jz   .exit_error ;.exit
    cmp  ax,bx
    jae   @f
	sub   ax,[scroll_bar_data_vertical.size_x]
    mov  [draw_SB],1
    jmp  .size_y
@@:
    mov  ax,bx

    
.size_y:
    mov  ebx,[img_high]
    test ebx,ebx
    jz   .exit_error ;.exit
    shl  eax,16
    add  eax,[window_high]
    cmp  ax,50-22
    jbe  @f
    sub  ax,50-22
@@:
    mov [wa_high],ax
    mov [wa_high_1],ax
    mov [wa_high_2],ax
    cmp  ax,bx
    jae   @f
	sub   ax,[scroll_bar_data_horizontal.size_y]
    mov  [draw_SB1],1
    jmp  .continue
@@: 
    mov  ax,bx
.continue:
    mov  ecx,eax
;****************************************************************************
;
;****************************************************************************
    cmp  [draw_SB1],1
    je   @f
    cmp  [draw_SB],0
    je   @f
	ror  ecx,16
	add  cx,[scroll_bar_data_vertical.size_x]
	rol  ecx,16
@@:    
    cmp  [draw_SB],1
    je  @f
    cmp  [draw_SB1],0
    je  @f
	add  cx,[scroll_bar_data_horizontal.size_y]
@@:
;    call  check_8bit_less
;.continue_1:
;****************************************************************************
; Prepare of data for SCROLLBAR's
;****************************************************************************
    call correct_cur_area
    
    mov  eax,[img_width]
	push edx
	imul eax,[scaling_mode]
	mov  ebx,100
	xor  edx,edx
	div  ebx
    mov  [scroll_bar_data_horizontal.max_area],eax
	pop  edx

	
    mov  ebx,[img_high]
	push eax edx
	mov  eax,ebx
	imul eax,[scaling_mode]
	mov  ebx,100
	xor  edx,edx
	div  ebx
	mov  ebx,eax
	pop  edx eax
    mov  [scroll_bar_data_vertical.max_area],ebx
	
;	cmp [scaling_mode],100
;	jne  .continue_1
 
    call correct_width
    call correct_cur_area
    
    cmp [correct_w],0
    je  @f
        
    call correct_high
    call correct_cur_area
    jmp  .continue_1
    
@@:
    call correct_high
    call correct_cur_area
    
    cmp [correct_h],0
    je  @f
    
    call correct_width
    call correct_cur_area
@@:

;    call  check_8bit_less

.continue_1:
;	mov  eax,[window_high]
;	sub  eax,55
;	ror  eax,16
;	mov  ax,dx
;	rol  eax,16
    mov  eax,edx
    shl  eax,16
    mov  ax,cx
    mov  [scroll_bar_data_vertical.y],eax

;	mov  eax,[window_width]
;	sub  eax,10
;	ror  eax,16
;	ror  edx,16
;	mov  ax,dx
;	rol  edx,16
;	rol  eax,16
    mov  eax,edx
    mov  ebx,ecx
    shr  ebx,16
    mov  ax,bx
    mov  [scroll_bar_data_horizontal.x],eax
    
    
;    mov  eax,edx
;    mov  ebx,ecx
;    shr  ebx,16
;    shr  eax,16
;    add  ax,bx
	mov  eax,[window_width]
	sub  ax,[scroll_bar_data_vertical.size_x]
;	sub  ax,4
	mov  [scroll_bar_data_vertical.start_x],ax

;    mov  eax,edx
;    add  ax,cx
	mov  eax,[window_high]
	sub  ax,[scroll_bar_data_horizontal.size_y]
;	sub  ax,4
    mov  [scroll_bar_data_horizontal.start_y],ax
	
;****************************************************************************
; Check necessity prepare of data
;****************************************************************************
    mov   eax,[scroll_bar_data_horizontal.max_area]
    mov   ebx,[scroll_bar_data_vertical.max_area]
    cmp   ebx,[scroll_bar_data_vertical.cur_area]
    ja    @f
    cmp   eax,[scroll_bar_data_horizontal.cur_area]
    ja    @f
    
    mov  ebp,0
    mov  ebx,[soi]
    jmp  .end
@@:
;****************************************************************************
; Calculate offset of next row data relative to previous row data
;****************************************************************************
    mov  ebp,eax
    mov  ebx,ecx
    shr  ebx,16
    sub  ebp,ebx
    imul ebp,esi
    mov  eax,ebp
    shr  ebp,3
;****************************************************************************
; Vertical SCROLLBAR
; At a case of resizing of the window
; Check the position of a runner has crossed a limit
;****************************************************************************
    mov  eax,[scroll_bar_data_vertical.max_area]
    sub  eax,[scroll_bar_data_vertical.cur_area]
    cmp  eax,[scroll_bar_data_vertical.position]
    jae  @f
    mov  [scroll_bar_data_vertical.position],eax
@@:
;****************************************************************************
; Horizontal SCROLLBAR
; At a case of resizing of the window
; Check the position of a runner has crossed a limit
;****************************************************************************
    mov  eax,[scroll_bar_data_horizontal.max_area]
    sub  eax,[scroll_bar_data_horizontal.cur_area]
    cmp  eax,[scroll_bar_data_horizontal.position]
    jae  @f
    mov  [scroll_bar_data_horizontal.position],eax
@@:
;****************************************************************************
; Calculate offset from a beginning of a image
;****************************************************************************
    mov  ebx,[scroll_bar_data_horizontal.max_area]
    imul ebx,esi
    mov  eax,ebx    
    shr  ebx,3
    imul ebx,[scroll_bar_data_vertical.position]
    mov  eax,[scroll_bar_data_horizontal.position]
    imul eax,esi
    shr  eax,3
    add  ebx,eax
    add  ebx,[soi]
.end:
    cmp  [soi],0
    je   .exit_error ;.exit
;****************************************************************************
; Draw image
;****************************************************************************
    cmp   [img_resolution_2],15
    jne  @f
	mov  esi,15
@@:
	mov  [image_start_coordinates],edx
	mov  [image_draw_size],ecx
	cmp [scaling_mode],dword 100
	je  .100
	cmp [scaling_mode],dword 0
	jne  @f
	call scaling.1
	ret
@@:
	mov  [correct_scrollbar_100_flag],0
	call zoom_all
	ret	
.100:
	mov  al,[correct_scrollbar_100_flag]
	test al,al
	jnz  @r
;******************************
	mov  eax,wa_width_1
	mov  eax,[eax]
	push eax
	mov  eax,wa_high_1
	mov  eax,[eax]
	push eax	
;******************************
	rol  ecx,16
    mov  eax,[scroll_bar_data_vertical.max_area]
    cmp  eax,[scroll_bar_data_vertical.cur_area]
	mov  ax,[wa_width_1]
    jbe  @f
	sub  ax,[scroll_bar_data_vertical.size_x]
	mov  [wa_width_1],ax
@@:
	sub  ax,cx
	mov  [wa_width_2],cx
	ror  ecx,16
	shr  ax,1
	shl  eax,16
	add  edx,eax
	shr  eax,16
	test eax,eax
	jz   @f
	call scaling.fill_for_L_R
@@:
    mov  eax,[scroll_bar_data_horizontal.max_area]
    cmp  eax,[scroll_bar_data_horizontal.cur_area]
	mov  ax,[wa_high_1]
    jbe  @f
	sub  ax,[scroll_bar_data_horizontal.size_y]
	mov  [wa_high_1],ax
@@:
	sub  ax,cx
	mov  [wa_high_2],cx
	shr  ax,1
	add  edx,eax
	test eax,eax
	jz   @f
	call scaling.fill_for_U_D
@@:
;******************************
    mcall 65,  ;[soi]
;******************************
	pop  eax
	mov  ebx,wa_high_1
	mov  [ebx],eax
	
	pop  eax
	mov  ebx,wa_width_1
	mov  [ebx],eax
;******************************
.exit:
	call draw_file_name_to_work_area
	ret

.exit_error:
	xor  eax,eax
	mov  [scroll_bar_data_vertical.cur_area],eax
	mov  [scroll_bar_data_horizontal.cur_area],eax
	ret
;****************************************************************************	
draw_file_name_to_work_area:
	mov  eax,[check1+32]
	test  eax,10b
	jz  @f
	mov  ebx,[file_name_length]
	mov  eax,ebx
	lea  ebx,[ebx*5]
	add  ebx,eax
	add  ebx,13 shl 16
	mcall 13,,<59,11>,0xe5e5e5
	mcall 4,<16,60>,0x80000000,file_name,,0xe5e5e5
@@:
	ret
;****************************************************************************
;check_8bit_less:
;    cmp  esi,8
;    jae  .no_8bit_less
;    mov  eax,ecx
;    ror  eax,16
;;    mov  ebx,eax
;    xor  ecx,ecx
;    
;    inc  ecx
;    shr  ax,1
;    
;    cmp  esi,4
;    je   @f
;    
;    inc  ecx
;    shr  ax,1
;    
;    cmp  esi,2
;    je   @f
;    
;    inc  ecx
;    shr  ax,1
;@@: 
;;    test ebx,ecx
;;    jz  @f
;;    inc  eax
;;@@:
;    shl  ax,cl
;    rol  eax,16
;    mov  ecx,eax
;.no_8bit_less:
;    ret
;****************************************************************************
correct_width:
    mov   [correct_w],0
    mov   ebx,[scroll_bar_data_vertical.max_area]
    cmp   ebx,[scroll_bar_data_vertical.cur_area]
    jbe   @f
    mov   eax,ecx
    shr   eax,16
	push  eax
	mov   ax,[scroll_bar_data_vertical.size_x]
	sub   [wa_width],ax
	pop   eax
    cmp   [wa_width],ax
    jae   @f
    ror   ecx,16
    mov   cx,[wa_width]
    rol   ecx,16
    mov   [correct_w],1
@@:
    ret
;****************************************************************************
correct_high:
    mov   [correct_h],0
    mov   eax,[scroll_bar_data_horizontal.max_area]
    cmp   eax,[scroll_bar_data_horizontal.cur_area]
    jbe   @f
	push  eax
	mov   ax,[scroll_bar_data_horizontal.size_y]
	sub   [wa_high],ax
	pop   eax
    cmp   [wa_high],cx
    jae   @f
    mov   cx,[wa_high]
    mov   [correct_h],1
@@:
    ret
;****************************************************************************
correct_cur_area:
    mov  eax,ecx
    xor  ebx,ebx
    mov  bx,ax
    mov  [scroll_bar_data_vertical.cur_area],ebx
    shr  eax,16
    mov  [scroll_bar_data_horizontal.cur_area],eax
    ret
;****************************************************************************
; Check draw vertical scrollbar
;****************************************************************************
draw_scrollbars:
	xor   eax,eax
	mov   [draw_scrollbar_counter],eax
    mov   eax,[scroll_bar_data_vertical.max_area]
    cmp   eax,[scroll_bar_data_vertical.cur_area]
    jbe   @f
	cmp   [scroll_bar_data_vertical.cur_area],0
	je   @f
	push    dword scroll_bar_data_vertical
	call    [scrollbar_ver_draw]
;    draw_scroll_bar_vertical scroll_bar_data_vertical
	inc  [draw_scrollbar_counter]
@@:
;****************************************************************************
; Check draw horizontal scrollbar
;****************************************************************************
    mov   eax,[scroll_bar_data_horizontal.max_area]
    cmp   eax,[scroll_bar_data_horizontal.cur_area]
    jbe   @f
	cmp   [scroll_bar_data_horizontal.cur_area],0
	je   @f
	push    dword scroll_bar_data_horizontal
	call    [scrollbar_hor_draw]
;    draw_scroll_bar_horizontal scroll_bar_data_horizontal
	inc  [draw_scrollbar_counter]
@@:
;  nodrawimage:
	cmp  [draw_scrollbar_counter],2
	jne  @f
	mov  ebx,[scroll_bar_data_vertical.x]
	mov  ecx,[scroll_bar_data_horizontal.y]
	mcall 13, , ,0xffffff
@@:
    ret
;****************************************************************************
;debug_1:    
;     pushad
;     mov   eax,ecx
;     shr   eax,16
;     mov   ebx,ecx
;     shl   ebx,16
;     shr   ebx,16
;     mov   eax,[image_file]
;     mov   ebx,edi
;     mov   [temp_eax],eax
;     mov   [temp_ebx],ebx
;     mcall 13,<90,500>,<2,20>,0
;     mcall 47,0xa0000,[temp_eax],<100, 5>,0xffffff
;     mcall 47,0xa0000,[temp_ebx],<300, 5>,0xffffff
;     popad
;     ret
;temp_eax dd 0
;temp_ebx dd 0
;****************************************************************************