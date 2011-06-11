
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:
	
    or	 [bWasDraw],1

    mcall 12,1

				   ; DRAW WINDOW
    xor  eax,eax		   ; function 0 : define and draw window
;    mov  ebx,350                   ; [x start] *65536 + [x size]
;    mov  ecx,400                   ; [y start] *65536 + [y size]
    mov  ebx,0*65536+0
    mov  ecx,0*65536+0
    add  ebx,[wnd_width]
    add  ecx,[wnd_height]
    cmp  cx,55
    ja	 @f
    mov  cx,55
@@:
    mov  edx,0x63ffffff 	   ; color of work area RRGGBB,8->color gl
;	pusha
	xor	esi,esi
    mcall
;	popa
;	imul  ecx,[load_file_number],304
;	add   ecx,32+40
;	add   ecx,[load_directory_pointer]
	mov   ecx,Authors_text.1  ; communication_area_name 
;	mov	ecx,[communication_area]
;	add	ecx,4
;	mov	ecx,string
    mcall 71,1,  ;labelt ; plugin_pach ;file_name ;plugin_pach ;plugin_pach
	
    call get_window_param
	
    call  draw_other

	
	push    dword menu_data_1
	call    [menu_bar_draw]
	push    dword menu_data_2
	call    [menu_bar_draw]
	push    dword menu_data_3
	call    [menu_bar_draw]
	push    dword menu_data_4
	call    [menu_bar_draw]
	push    dword menu_data_5
	call    [menu_bar_draw]
	push    dword menu_data_6
	call    [menu_bar_draw]

;	draw_menu_bar menu_data_1
;	draw_menu_bar menu_data_2
;	draw_menu_bar menu_data_3
;	draw_menu_bar menu_data_4
;	draw_menu_bar menu_data_5
;	draw_menu_bar menu_data_6

	push    dword dinamic_button_data_1
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_2
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_3
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_4
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_5
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_6
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_7
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_8
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_9
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_10
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_11
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_12
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_13
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_14
	call    [dinamic_button_draw]
	push    dword dinamic_button_data_15
	call    [dinamic_button_draw]
	
;	draw_dinamic_button dinamic_button_data_1 ; open file
;	draw_dinamic_button dinamic_button_data_2 ; reload file
;	draw_dinamic_button dinamic_button_data_3 ; left limit
;	draw_dinamic_button dinamic_button_data_4 ; left arrow
;	draw_dinamic_button dinamic_button_data_5 ; slide show
;	draw_dinamic_button dinamic_button_data_6 ; right arrow
;	draw_dinamic_button dinamic_button_data_7 ; right limit
;	draw_dinamic_button dinamic_button_data_8 ; zoom +
;	draw_dinamic_button dinamic_button_data_9 ; zoom -
;	draw_dinamic_button dinamic_button_data_10 ; zoom 100%
;	draw_dinamic_button dinamic_button_data_11 ; rotate clockwise
;	draw_dinamic_button dinamic_button_data_12 ; rotate counter-clockwise
;	draw_dinamic_button dinamic_button_data_13 ; file information
;	draw_dinamic_button dinamic_button_data_14 ; wallpaper
;	draw_dinamic_button dinamic_button_data_15 ; settings

	mcall 13,<225,2>,<3,22>,0
	mcall ,<280,2>
	mcall ,<410,2>
	mcall ,<490,2>
	mcall ,<545,2>
	mcall ,<625,2>
	mcall ,<720,2>
				   ; WINDOW LABEL

;	mov   esi, file_mount - file_mount_end
    mcall 4,<630,3>,0x80000000, file_mount  ;, file_mount - file_mount.end
;    cmp  [soi],0
;    je   @f

	mcall 13,<677,40>,<13,11>,0xe5e5e5
	
	mov   ecx,[load_directory_pointer]
	mov   ecx,[ecx+4]
    mcall 47,0x80060000, ,<679, 15>,0
	
	mcall 8,[dinamic_button_data_5.x],[dinamic_button_data_5.y],0x40000002
;@@:
    mcall 12,2

    ret
;---------------------------------------------------------------------
set_draw_scrollbar_all:
	xor   eax,eax
	inc   eax
	mov   [scroll_bar_data_vertical.all_redraw],eax
	mov   [scroll_bar_data_horizontal.all_redraw],eax
	ret
;---------------------------------------------------------------------
set_nodraw_scrollbar_all:
	xor   eax,eax
	mov   [scroll_bar_data_vertical.all_redraw],eax
	mov   [scroll_bar_data_horizontal.all_redraw],eax
	ret
;---------------------------------------------------------------------
draw_other:
    call drawimage
@@:
    mov   ebx,0 ;5
    shl   ebx,16
    add   ebx,[window_width]
;    sub   ebx,9
;    push  ebx
	xor   ecx,ecx
;    mov   ecx,[skin_high]
    shl   ecx,16
    add   ecx,50-22
;    sub   ecx,[skin_high]
	mov   edx,0xffffff
	cmp   [no_draw_window],0
	jne   @f
    mcall 13   ; fill top area
@@:
    mov  eax,[soi]
    test eax,eax
    jnz   @f
	mov  ecx,[window_high]
	sub  ecx,28   ;50+4
	add  ecx,(50-22) shl 16
    mcall 13
@@:

	call  draw_number_of_file
	
	call  draw_zoom_factors
	
	cmp [scaling_mode],dword 0
	jne  @f
;	pop  ebx
	ret
@@:
;    mov  ecx,[scroll_bar_data_vertical.cur_area]
;    mov  ebx,[scroll_bar_data_horizontal.cur_area]
;    push ecx
;    add  ebx,5
;    mov  ax,bx
;    shl  ebx,16
;    add  ebx,[window_width]
;    sub  ebx,4
;    sub  bx,ax
;    cmp  bx,0
;    jbe  @f
;    add  ecx,50 shl 16
;    mcall 13, , ,0xffffff
;
;@@:
;    pop  ecx
;    pop  ebx

;    add  ecx,50
;    mov  ax,cx
;    shl  ecx,16
;    add  ecx,[window_high]
;    sub  cx,ax
;    sub  ecx,4
;    cmp  cx,0
;    jbe  @f
;    mcall 13, , ,0xffffff

;@@:
	call  set_draw_scrollbar_all
    call  draw_scrollbars
	call  set_nodraw_scrollbar_all

	ret
;---------------------------------------------------------------------	
draw_number_of_file:

	mcall 13,<630,42>,<13,11>,0xe5e5e5
	
	xor   ecx,ecx
	mov   eax,[load_directory_pointer]
	cmp   [eax+4],dword 0
	je    @f
	mov   ecx,[load_file_number]
	inc   ecx
@@:
    mcall 47,0x80060000, ,<632, 15>,0
	ret
;---------------------------------------------------------------------
draw_zoom_factors:
	mcall 13,<725,30>,<13,11>,0xe5e5e5
	
	mov   ecx,[zoom_factors]
	mov   ecx,[ecx]
	mov  [scaling_mode],ecx
	test  ecx,ecx
	jnz  @f
	mcall 4,<727,14>,0x80000000, zoom_auto_text
	ret
@@:
    mcall 47,0x80060000, ,<727, 15>,0
	ret
;---------------------------------------------------------------------