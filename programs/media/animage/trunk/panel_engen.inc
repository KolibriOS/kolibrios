;-------------------------------------------------
;-----interraction panel with user----------------
;-------------------------------------------------
panel_interraction:
	;>>>>>>>>>>>>>>>>>>>>>>>>>>
	mov eax,18
	mov ebx,14
	;mcall

	and [Panel_flag],0
	;collision with text on panel
      next_columnus_text:

	mov eax,[edi]
	mov ebx,[edi+4]
	sub eax,10
	sub ebx,3
	mov [Icon_text_x],eax
	mov [Icon_text_y],ebx

	mov esi,[edi+8]
	imul esi,10
	mov [button_menu_size_x],esi
	mov [button_menu_size_y],dword 13

	mov esi,[edi+8]
	add edi,4*3
	add edi,esi

	mov ecx,[MouseX]
	mov edx,[MouseY]

	push edi
	mov edi,[button_menu_size_y]
	mov esi,[button_menu_size_x]
	call columnus

	pop edi
	mov esi,7

	test eax,eax
	jz no_columnus_text

	mov esi,1
	mov [Panel_flag],1
	call GetMouseClick

	test eax,eax
	jz no_mouse_pressed

	mov eax,18
	mov ebx,14
	;mcall

	call print_panel_menu

	no_mouse_pressed:

	no_columnus_text:

	mov eax,[Icon_text_x]
	mov ebx,[Icon_text_y]
	mov ecx,[button_menu_size_x]		;-------------------------
	mov edx,[button_menu_size_y]
	call draw_volume_rectangle

	push [counter]
	push edi
	mov [counter],7
	mov edi,panel_text
	call print_panel_text

	pop edi
	pop [counter]
	dec [counter]
	jnz next_columnus_text

	ret
;---------------------------------------------------------
;-----draw panel menu(main engin of panel)----------------
;---------------------------------------------------------
print_panel_menu:


	push [counter]
	;delit main buttons(some time)

	mov [counter],10

	next_icon_delit1:

	mov eax,8
	mov edx,[counter]
	add edx,11100000000000000000000000000000b
	mcall
	inc [counter]

	cmp [counter],30
	jl next_icon_delit1

	;delit buttons of brushes
	mov [counter],40

	next_icon_delit2:

	mov eax,8
	mov edx,[counter]
	add edx,11100000000000000000000000000000b
	mcall
	inc [counter]

	cmp [counter],45
	jl next_icon_delit2

	;delit buttons of zoom
	mov [counter],45

	next_icon_delit3:

	mov eax,8
	mov edx,[counter]
	add edx,11100000000000000000000000000000b
	mcall
	inc [counter]

	cmp [counter],50
	jl next_icon_delit3

	;delit buttons of palette
	mov eax,8
	mov edx,51
	add edx,11100000000000000000000000000000b
	mcall


	;delit buttons of color
	mov eax,8
	mov edx,52
	add edx,11100000000000000000000000000000b
	mcall

	pop [counter]

	push esi
	mov esi,[counter]
	dec esi
	shl esi,4

	mov eax,[menu_rectangles+esi]
	mov ebx,[menu_rectangles+esi+4]
	mov ecx,[menu_rectangles+esi+8]
	mov edx,[menu_rectangles+esi+12]
	mov esi,1
	mov [menu_coordinat_x],eax
	mov [menu_size_x],ecx
	call draw_volume_rectangle
	pop esi

    calculate_counter_menu:


	;calculate menu counter
	mov eax,[counter]
	dec eax
	shl eax,2
	mov ebx,[menu_counters+eax]
	mov [counter_menu],ebx

	call GetMouseCoordinats

	and [menu_counter],0
	push esi
	push edi
	;сглаживание анимации >>>>>>>>>>>>>>>>>>
	mov eax,18
	mov ebx,14
	;mcall

    menu_loop:

	mov eax,[menu_coordinat_x]
	mov ebx,36
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[menu_size_x]
	mov edi,18
	add ebx,[menu_counter]
	add eax,2
	sub esi,3
	inc ebx
	call columnus
	mov esi,7

	test eax,eax
	jz no_columnus_menu_text

	mov esi,1

	no_columnus_menu_text:

	mov eax,[menu_coordinat_x]
	mov ebx,36
	mov ecx,[menu_size_x]
	mov edx,18
	add ebx,[menu_counter]
	add eax,2
	sub ecx,3
	inc ebx
	call draw_volume_rectangle
	add [menu_counter],20


	dec [counter_menu]
	jnz menu_loop


	;print menu text
	push [counter]
	mov esi,[counter]
	dec esi
	shl esi,2
	mov edi,[menu_text_en+esi]
	mov eax,[menu_counters+esi]
	mov [counter],eax
	call print_panel_text
	pop [counter]
	pop edi
	pop esi


	menu_still:

	mov eax,10
	mcall
	mov eax,2
	mcall
	mov eax,17
	mcall

	call GetMouseClick

	test eax,eax
	jz calculate_counter_menu

	call GetMouseCoordinats

	;calculate menu counter
	mov eax,[counter]
	dec eax
	shl eax,2
	mov ebx,[menu_counters+eax]
	mov [counter_menu],ebx

	and [menu_counter],0
	and [counter_11],0

	push esi
	push edi

    menu_loop2:


	mov eax,[menu_coordinat_x]
	mov ebx,36
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[menu_size_x]
	mov edi,18
	add ebx,[menu_counter]
	add eax,2
	sub esi,3
	inc ebx
	call columnus

	inc [counter_11]

	test eax,eax
	jz no_columnus_menu_text2

	mov esi,[counter_11]
	mov [number_menu],esi

	no_columnus_menu_text2:

	add [menu_counter],20

	dec [counter_menu]
	jnz menu_loop2

	pop edi
	pop esi

	mov eax,5
	mov ebx,15
	mcall

	push [counter]
	pushad

	call drawwin

	popad
	pop [counter]

	mov eax,[counter]
	mov [number_panel],eax

	call TakeInstruments
	jmp still

;---------------------------------------------------------
;------print mouse position on panel----------------------
;---------------------------------------------------------
PrintMousePos:

	pushad

	mov eax,[Window_SizeX]
	sub eax,75
	mov ebx,20+15+6
	mov ecx,52
	mov edx,18
	mov esi,4
	call draw_volume_rectangle

	mov eax,[Window_SizeX]
	sub eax,75
	mov ebx,20+15+6+18+2
	mov ecx,52
	mov edx,18
	mov esi,4
	call draw_volume_rectangle

	mov eax,[Window_SizeX]
	sub eax,70
	mov ebx,20+15+6+6
	and ecx,0
	mov edx,mouse_pos_x
	mov esi,2
	call print_text

	mov eax,[Window_SizeX]
	sub eax,70
	mov ebx,20+15+6+6+18+1
	and ecx,0
	mov edx,mouse_pos_y
	mov esi,2
	call print_text

	mov eax,47
	mov ebx,4*65536
	mov ecx,[ScreenX]
	add ecx,[PosX]
	mov edx,[Window_SizeX]
	sub edx,55
	shl edx,16
	add edx,20+15+6+6
	and esi,0
	mcall

	mov eax,47
	mov ebx,4*65536
	mov ecx,[ScreenY]
	add ecx,[PosY]
	mov edx,[Window_SizeX]
	sub edx,55
	shl edx,16
	add edx,20+15+6+18+1+6
	and esi,0
	mcall

	popad
	ret
;----------------------------------------------------------
;---------------draw panel in window of program------------
;----------------------------------------------------------
draw_panel:

	mov eax,5
	mov ebx,20
	mov ecx,[Window_SizeX]
	mov edx,15
	mov esi,6
	sub ecx,10
	call draw_volume_rectangle ; top menu - file and etc.

	;mov eax,5
	;mov ebx,20
	;mov ecx,[Window_SizeX]
	;mov edx,15
	;mov esi,6
	;sub ecx,10
	;call draw_volume_rectangle

	mov eax,5
	mov ebx,20+15+1
	mov ecx,[Window_SizeX]
	mov edx,50
	mov esi,1
	sub ecx,10
	call draw_volume_rectangle ; tools panel

	mov eax,5
	mov ebx,20+15+1+50+1
	mov ecx,[Window_SizeX]
	mov edx,[Window_SizeY]
	mov esi,1
	sub ecx,10+20
	sub edx,20+15+1+50+1+5+20
	call draw_volume_rectangle	; picure area

	mov eax,5
	mov ebx,[Window_SizeY]
	mov ecx,[Window_SizeX]
	mov edx,20+10
	mov esi,3
	sub ecx,30
	sub ebx,25+10
	call draw_volume_rectangle	; horizontal scrollbar

	mov eax,[Window_SizeX]
	mov ebx,20+15+1+49+2
	mov ecx,20+10
	mov edx,[Window_SizeY]
	mov esi,3
	sub eax,25+10
	sub edx,20+15+1+49+5+20
	call draw_volume_rectangle	; vertical scrollbar

	mov eax,[Window_SizeX]
	mov ebx,[Window_SizeY]
	mov ecx,20+10
	mov edx,20+10
	mov esi,6
	sub eax,25+10
	sub ebx,25+10
	call draw_volume_rectangle	; down and right small area

	;mov eax,445
	;mov ebx,20+15+6
	;mov ecx,37
	;mov edx,37
	;mov esi,4
	;call draw_volume_rectangle

	ret


;----------------------------------------------------------
;----------print text on the panel and menu----------------
;----------------------------------------------------------
print_panel_text:
       next_panel_text:

	mov eax,[edi]
	mov ebx,[edi+4]
	;xor ecx,ecx
	mov ecx,0x10000000
	mov edx,edi
	add edx,12
	mov esi,[edi+8]
	add edi,esi
	add edi,3*4

	push edi
	call print_text
	pop edi

	dec [counter]
	jnz next_panel_text
	ret
;----------------------------------------------------------
;------------draw lines of scoll---------------------------
;----------------------------------------------------------
draw_scrollers:

	 mov edi,[CounterX]
	 mov eax,[Scroll1CoordinatX]
	 mov ebx,[Window_SizeY]
	 sub ebx,22+10
	 mov ecx,[Window_SizeX]
	 sub ecx,10+5+20+10+1
	 mov edx,14+10
	 mov esi,1
	 mov [Scroll1CoordinatX],eax
	 mov [Scroll1CoordinatY],ebx
	 mov [Scroll1MaxSizeY],edx
	 mov [Scroll1MaxSizeX],ecx
	 imul ecx,edi
	 push eax
	 push ebx
	 push edx
	 mov eax,ecx
	 mov ebx,[Picture_SizeX]
	 cdq
	 idiv ebx
	 mov ecx,eax
	 pop edx
	 pop ebx
	 pop eax
	 mov [Scroll1SizeX],ecx
	 mov edi,[Scroll1MaxSizeX]
	 sub edi,ecx
	 mov [Scroll1FreeX],edi
	 call draw_volume_rectangle

	 mov eax,[Window_SizeX]
	 sub eax,22+10
	 mov ebx,[Scroll2CoordinatY]
	 mov ecx,14+10
	 mov edx,[Window_SizeY]
	 sub edx,20+15+1+49+10+20+11
	 mov esi,1
	 mov [Scroll2CoordinatX],eax
	 mov [Scroll2CoordinatY],ebx
	 mov [Scroll2MaxSizeX],ecx
	 mov [Scroll2MaxSizeY],edx

	 imul edx,[CounterY]
	 push eax
	 push ebx
	 mov eax,edx
	 mov ebx,[Picture_SizeY]
	 cdq
	 idiv ebx
	 mov edx,eax
	 pop ebx
	 pop eax
	 mov [Scroll2SizeY],edx
	 mov edi,[Scroll2MaxSizeY]
	 sub edi,edx
	 mov [Scroll2FreeY],edi

	 call draw_volume_rectangle

	 ret
	 ret

button_menu_size_x  dd 0
button_menu_size_y  dd 0