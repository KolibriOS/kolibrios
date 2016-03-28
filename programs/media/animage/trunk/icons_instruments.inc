macro square_width_put
{
local .no_put_to_screen
local .next_color_put
local .vertical_width_put
local .horizontal_width_put

	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz .no_put_to_screen
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp

	.next_color_put:
	;put saved pixels in ReserveArray
	push ecx
	mov [counter2],5
	mov edi,[ebx]
	push edi
	.vertical_width_put:
		mov [counter],5

		.horizontal_width_put:
			mov edx,[ebx+mem_offs_shape+ebp]
			;and edx,0xffffff
			mov [edi],dx
			shr edx,16
			mov [edi+2],dl

		add edi,3
		add ebp,4
		dec [counter]
		jnz .horizontal_width_put

		mov ecx,[Picture_SizeX]
		sub ecx,5
		lea ecx,[ecx+ecx*2]
		add edi,ecx
	dec [counter2]
	jnz .vertical_width_put
	pop edi
	pop ecx

	add ebx,4
	dec ecx
	jnz .next_color_put

	.no_put_to_screen:
}

macro square_width_save
{
local .next_color_save
local .vertical_width_save
local .horizontal_width_save
	mov ebx,[ReserveArray]
	mov [ebx],eax
	mov ecx,eax
	add ebx,4
	xor ebp,ebp

	.next_color_save:
	;save color of pixel in ReserveArray
	push ecx
	mov [counter2],5
	mov edi,[ebx]
	.vertical_width_save:
		mov [counter],5

		.horizontal_width_save:
			mov eax,edi
			mov edx,[eax]
			;and edx,0xffffff
			mov [ebx+mem_offs_shape+ebp],dx
			shr edx,16
			mov [ebx+mem_offs_shape+2+ebp],dl

		add edi,3
		add ebp,4
		dec [counter]
		jnz .horizontal_width_save

		mov ecx,[Picture_SizeX]
		sub ecx,5
		lea ecx,[ecx+ecx*2]
		add edi,ecx
	dec [counter2]
	jnz .vertical_width_save
	pop ecx

	add ebx,4
	dec ecx
	jnz .next_color_save
}

macro draw_calc_pixels brush
{
local .next_pixel_put
local .vertical_width
local .horizontal_width
local .no_draw_pixel

	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	add edi,brush
	.next_pixel_put:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	mov [counter2],5

	.vertical_width:
		mov [counter],5

		.horizontal_width:
		xor ecx,ecx
		mov cl,byte[edi+ebp]
		test cl,cl
		jz .no_draw_pixel
			mov [eax],si
			mov [eax+2],dl
		.no_draw_pixel:
		add eax,3
		inc ebp
		dec [counter]
		jnz .horizontal_width

		mov ecx,[Picture_SizeX]
		sub ecx,5
		lea ecx,[ecx+ecx*2]
		add eax,ecx
	dec [counter2]
	jnz .vertical_width

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz .next_pixel_put
}

;-----------------------------------------------------------
;-----instruments of panel(icon's instruments)--------------
;-----------------------------------------------------------
;input:
; [Current_instrument] - instrument code
;
; *** instrument codes ***
;10 - pensil
;11 - draw brush
;12 - spray
;13 - lastik
;14 - flood fill
;15 - pipetka
;16 - draw line
;17 - draw rectangle
;18 - draw circle
;19 - draw ellips
;20 - draw hard contour
;21 - allocation of a countour
;23 - reflection from left to right
;24 - reflection from up to down
;40 - brush 1
;41 - brush 2
;42 - brush 3
;43 - brush 4
;44 - brush 5
;45 - zoom 1
;46 - zoom 2
;47 - zoom 3
;48 - zoom 4
;49 - zoom 8
;50 - zoom 16
;51 - palette

align 4
TakeButtonInstruments:
	mov eax,[Current_instrument]

	;*************************brush 1***********************
	cmp eax,40
	jne no_brush1

	mov [Brush_SizeX],4
	mov [Brush_SizeY],4
	mov [Number_Brush],0
	mov ebx,[Last_instrument]
	mov [Current_instrument],ebx

	mov [line_width],1
	ret
	no_brush1:

	;*************************brush 2***********************
	cmp eax,41
	jne no_brush2

	mov [Brush_SizeX],6
	mov [Brush_SizeY],4
	mov [Number_Brush],1
	mov ebx,[Last_instrument]
	mov [Current_instrument],ebx

	mov [line_width],2
	ret
	no_brush2:

	;*************************brush 3***********************
	cmp eax,42
	jne no_brush3

	mov [Brush_SizeX],8
	mov [Brush_SizeY],7
	mov [Number_Brush],2
	mov ebx,[Last_instrument]
	mov [Current_instrument],ebx

	mov [line_width],3
	ret
	no_brush3:

	;*************************brush 4***********************
	cmp eax,43
	jne no_brush4

	mov [Brush_SizeX],14
	mov [Brush_SizeY],14
	mov [Number_Brush],3
	mov ebx,[Last_instrument]
	mov [Current_instrument],ebx

	mov [line_width],4
	ret
	no_brush4:

	;*************************brush 5***********************
	cmp eax,44
	jne no_brush5

	mov [Brush_SizeX],6
	mov [Brush_SizeY],6
	mov [Number_Brush],4
	mov ebx,[Last_instrument]
	mov [Current_instrument],ebx

	mov [line_width],5
	ret
	no_brush5:

	;*************************pensil************************
	cmp eax,10
	jne no_pensil

	mov al,[exit_from_work_arrea]

	test al,al
	jz no_exit_from_work_arrea

	and [Activate_instrument],0
	and [exit_from_work_arrea],0

	no_exit_from_work_arrea:

	mov al,[Activate_instrument]

	test al,al
	jnz no_activated_later

	call GetScreenCordinats
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx

	no_activated_later:

	mov eax,[PointerToPicture]
	mov ebx,[ReserveArray]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]
	call calculate_line

	mov ecx,eax
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	xor ebx,ebx
	mov eax,[ReserveArray]

	pensil_next_pixel_draw:

	  mov ebx,[eax]
	  mov [ebx],si
	  mov [ebx+2],dl
	  add eax,4

	dec ecx
	jnz pensil_next_pixel_draw

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx

	call MovePictureToWorkScreen
	mov [Activate_instrument],1
	ret
	no_pensil:

	;*************************draw brush********************
	cmp eax,11
	jne no_brush

	mov [Last_instrument],eax

	jmp no_lastik_

	lastik_in:
	mov eax,[Color]
	mov [SColor],eax
	mov [Color],0xffffff
	no_lastik_:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	mov ecx,[Brush_SizeX]
	mov edx,[Brush_SizeY]
	add eax,[PosX]
	add ebx,[PosY]
	add eax,ecx
	add ebx,edx

	cmp eax,[Picture_SizeX]
	jl no_max_pos_x
	mov eax,[Picture_SizeX]
	no_max_pos_x:

	cmp ebx,[Picture_SizeY]
	jl no_max_pos_y
	mov ebx,[Picture_SizeY]
	no_max_pos_y:

	cmp eax,ecx
	ja no_min_pos_x
	mov eax,[Brush_SizeX]
	no_min_pos_x:

	cmp ebx,edx
	ja no_min_pos_y
	mov ebx,[Brush_SizeY]
	no_min_pos_y:

	sub eax,[Brush_SizeX]
	sub ebx,[Brush_SizeY]

	mov [x],eax
	mov [y],ebx

	mov al,[exit_from_work_arrea]
	test al,al
	jz no_exit_from_work_arrea_brush
	and [Activate_instrument],0
	and [exit_from_work_arrea],0
	no_exit_from_work_arrea_brush:

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_brush_xy
	mov eax,[x]
	mov ebx,[y]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	no_new_brush_xy:

	mov eax,[PointerToPicture]
	mov ebx,[ReserveArray]
	add ebx,4
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[x]
	mov edi,[y]

	call calculate_line

	mov ebx,[ReserveArray]
	mov [ebx],eax

	;procedure drawing of brush
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[Number_Brush]
	imul edi,20*20

	next_pixel_put_brush:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_brush:
	   and [counter],0

	   horizontal_width_brush:
	   xor ecx,ecx
	   mov cl,byte[Brush_color+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_brush
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_brush:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],20
	   jne horizontal_width_brush

	   mov ecx,[Picture_SizeX]
	   sub ecx,20
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],20
	jne vertical_width_brush

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_brush

	mov eax,[x]
	mov ebx,[y]
	mov [OldX],eax
	mov [OldY],ebx

	mov al,[lastik_is_active]
	test al,al
	jz no_lastik_active

	mov eax,[SColor]
	mov [Color],eax
	and [lastik_is_active],0
	no_lastik_active:

	call MovePictureToWorkScreen
	mov eax,[Current_instrument]
	mov [Last_instrument],eax

	ret
	no_brush:

	;*************************pipetka***********************
	cmp eax,15
	jne no_pipetka

	mov eax,[ScreenY]
	mov ebx,[Picture_SizeX]
	add eax,[PosY]
	imul eax,ebx
	add eax,[ScreenX]
	add eax,[PosX]
	lea eax,[eax+eax*2]
	add eax,[PointerToPicture]
	mov ebx,[eax]
	and ebx,0xffffff
	mov [Color],ebx

	mcall SF_DRAW_RECT, ((ci_panel_x_pos+130) shl 16)+21,\
		((ci_panel_y_pos+28) shl 16)+21, [Color]

	ret
	no_pipetka:

	;************************lastik*************************
	cmp eax,13
	jne no_lastik

	mov [Last_instrument],eax
	mov [lastik_is_active],1
	jmp lastik_in
	no_lastik:

	;************************Flood Fill*********************
	cmp eax,14
	jne no_FloodFill

	mov eax,[PointerToPicture]
	mov ebx,[PointerToEditBufer]

	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	shl ecx,1

	cmp ecx,mem_flood_f
	ja normal_size_of_bufer

	mov ebx,[ReserveArray]

	normal_size_of_bufer:

	add ebx,4
	mov ecx,[Picture_SizeX]
	mov edx,[ScreenX]
	add edx,[PosX]
	shl edx,16
	add edx,[ScreenY]
	add edx,[PosY]
	mov esi,[Picture_SizeX]
	dec esi
	shl esi,16
	add esi,[Picture_SizeY]
	dec esi
	mov edi,[Color]

	call flood_fill
	call MovePictureToWorkScreen
	ret
	no_FloodFill:

	;************************spray**************************
	cmp eax,12
	jne no_spray

	cmp [Activate_instrument],0
	jne no_null_spray
	mov [Activate_instrument],1
	ret
	no_null_spray:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	mov ecx,[Brush_SizeX]
	mov edx,[Brush_SizeY]
	add eax,[PosX]
	add ebx,[PosY]
	add eax,ecx
	add ebx,edx

	cmp eax,[Picture_SizeX]
	jl no_max_pos_x_spray
	mov eax,[Picture_SizeX]
	no_max_pos_x_spray:

	cmp ebx,[Picture_SizeY]
	jl no_max_pos_y_spray
	mov ebx,[Picture_SizeY]
	no_max_pos_y_spray:

	cmp eax,ecx
	ja no_min_pos_x_spray
	mov eax,[Brush_SizeX]
	no_min_pos_x_spray:

	cmp ebx,edx
	ja no_min_pos_y_spray
	mov ebx,[Brush_SizeY]
	no_min_pos_y_spray:

	sub eax,[Brush_SizeX]
	sub ebx,[Brush_SizeY]
	mov edi,0;[Number_Brush]
	imul edi,20*20

	mov [x],eax
	mov [y],ebx
	mov ebp,[Picture_SizeX]
	xor edx,edx
	brush_y_spray:
	  xor ecx,ecx
	  brush_x_spray:
	   ;calculate position in array of spray
	   mov esi,edx
	   lea esi,[esi+esi*4] ;esi=esi*5
	   shl esi,2	       ;esi=(esi*3)*4
	   add esi,ecx
	   add esi,edi
	   ;read byte from array
	   xor eax,eax
	   mov al,[Spray_color+esi]
	   test eax,eax
	   jz no_color_spray
	   mov eax,[x]
	   mov ebx,[y]
	   add eax,ecx
	   add ebx,edx
	   imul ebx,ebp; ebp=[Picture_SizeX]
	   add eax,ebx
	   lea eax,[eax+eax*2]
	   add eax,[PointerToPicture]
	   mov ebx,[Color]
	   mov [eax],bx
	   shr ebx,16
	   mov [eax+2],bl
	   no_color_spray:
	   inc ecx
	   cmp ecx,20
	  jl brush_x_spray
	inc edx
	cmp edx,20
	jl brush_y_spray

	call MovePictureToWorkScreen
	ret
	no_spray:

	;************************palette************************
	cmp eax,51
	jne no_palette

	mov eax,20
	mov ebx,ci_palete_y_pos
	mov ecx,32*10+32*5+7
	mov edx,140
	mov  esi,1
	call draw_volume_rectangle
	mov [x],20+5
	mov [y],ci_palete_y_pos+10
	mov edi,256
	xor esi,esi
	next_rectangle:
	mov ebx,[x]
	mov ecx,[y]
	mov edx,[palitra+esi]
	and edx,0xffffff
	shl ebx,16
	shl ecx,16
	add ebx,13
	add ecx,13
	mcall SF_DRAW_RECT
	add [x],15
	cmp [x],20+15*32
	jl no_new_line
	mov [x],20+5
	add [y],15
	no_new_line:
	add esi,4
	dec edi
	jnz next_rectangle

	wait_events:
	mcall SF_WAIT_EVENT

	cmp eax,1
	jne @f
		ret
	@@:
	cmp eax,2
	jne no_keys
		mcall SF_GET_KEY
		jmp wait_events
	no_keys:
	cmp eax,3
	jne no_buttons
		mcall SF_GET_BUTTON
		jmp wait_events
	no_buttons:
	cmp eax,6
	jne wait_events

	call GetMouseClick

	test eax,eax
	jz wait_events
	call GetMouseCoordinats
	mov [x],20+5
	mov [y],ci_palete_y_pos+10
	mov [counter],0
	next_rectangle_column:
	mov eax,[x]
	mov ebx,[y]
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,13
	mov edi,13
	call columnus
	test eax,eax
	jz no_columnus_color

	mov eax,[counter]
	shl eax,2
	mov ebx,[palitra+eax]
	and ebx,0xffffff
	mov [Color],ebx
	no_columnus_color:

	add [x],15

	cmp [x],20+15*32
	jl no_new_line_column

	mov [x],20+5
	add [y],15
	no_new_line_column:

	inc [counter]

	cmp [counter],257
	jne next_rectangle_column

	mcall SF_SLEEP,10

	call drawwin

	mov eax,[Last_instrument]
	mov [Current_instrument],eax
	ret
	no_palette:

	;************************draw line**********************
	cmp eax,16
	jne no_line

	mov [Last_instrument],eax

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_line_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	xor eax,eax
	mov ecx,mem_flood_f/4
	mov edi,[ReserveArray]
	cld
	rep stosd
	ret
	no_new_line_xy:

	;put saved pixels
	square_width_put

	;calculate line
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	mov ebp,[Picture_SizeX]
	sub ebp,[line_width]
	cmp esi,ebp
	jl no_minimum_x_line
	mov esi,ebp
	no_minimum_x_line:

	mov ebp,[Picture_SizeY]
	sub ebp,[line_width]
	cmp edi,ebp
	jl no_minimum_y_line
	mov edi,ebp
	no_minimum_y_line:

	call calculate_line

	;save color pixels in ReserveArray
	square_width_save

	;draw calculated pixels on work arrea
	draw_calc_pixels width_pixels

	call MovePictureToWorkScreen
	ret
	no_line:

	;************************draw rectangle*****************
	cmp eax,17
	jne no_rectangle

	mov [Last_instrument],eax

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_rectangle_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	xor eax,eax
	mov ecx,mem_flood_f/4
	mov edi,[ReserveArray]
	cld
	rep stosd
	ret
	no_new_rectangle_xy:

	;put saved pixels
	square_width_put

	;calculate line
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	mov ebp,[Picture_SizeX]
	sub ebp,[line_width]
	cmp esi,ebp
	jl no_minimum_x_rectangle
	mov esi,ebp
	no_minimum_x_rectangle:

	mov ebp,[Picture_SizeY]
	sub ebp,[line_width]
	cmp edi,ebp
	jl no_minimum_y_rectangle
	mov edi,ebp
	no_minimum_y_rectangle:

	call calculate_rectangle

	;save color pixels in ReserveArray
	square_width_save

	;draw calculated pixels on work arrea
	draw_calc_pixels width_pixels_rectangle

	call MovePictureToWorkScreen
	ret
	no_rectangle:

	;************************draw circle********************
	cmp eax,18
	jne no_circle

	mov [Last_instrument],eax

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_circle_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	xor eax,eax
	mov ecx,mem_flood_f/4
	mov edi,[ReserveArray]
	cld
	rep stosd
	ret
	no_new_circle_xy:

	;put saved pixels
	square_width_put

	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_circle
	neg esi
	shr esi,1
	neg esi
	add esi,[OldX]
	jmp no_plus_sign_x_circle
	no_sign_x_circle:

	shr esi,1
	add esi,[OldX]
	no_plus_sign_x_circle:

	sub edi,[OldY]
	jns no_sign_y_circle
	neg edi
	shr edi,1
	neg edi
	add edi,[OldY]
	jmp no_plus_sign_y_circle
	no_sign_y_circle:

	shr edi,1
	add edi,[OldY]
	no_plus_sign_y_circle:

	mov [x],esi
	mov [y],edi

	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_circle_r
	neg esi
	no_sign_x_circle_r:

	sub edi,[OldY]
	jns no_sign_y_circle_r
	neg edi
	no_sign_y_circle_r:

	mov [Dx_],esi
	mov [Dy_],edi

	;finit
	fild [Dx_]
	fmul st0,st0
	fild [Dy_]
	fmul st0,st0
	fadd st0,st1
	fsqrt
	fistp [Radius]
	fistp [Icon_X]
	mov esi,[Radius]
	shr esi,1
	cmp esi,0
	jne no_null_radius
	mov [Radius],1
	no_null_radius:

	mov [Radius],esi

	mov edi,[x]
	mov ebp,[y]
	add edi,esi
	add ebp,esi

	mov edx,[Picture_SizeX]
	sub edx,[line_width]
	cmp edi,edx
	jl no_limit_x_circle
	sub edi,edx
	sub [Radius],edi
	no_limit_x_circle:

	mov edx,[Picture_SizeY]
	sub edx,[line_width]
	cmp ebp,edx
	jl no_limit_y_circle
	sub ebp,edx
	sub [Radius],ebp
	no_limit_y_circle:


	mov edi,[x]
	mov ebp,[y]

	sub edi,[Radius]
	jns no_minimum_x_circle
	add [Radius],edi
	no_minimum_x_circle:

	sub ebp,[Radius]
	jns no_minimum_y_circle
	add [Radius],ebp
	no_minimum_y_circle:

	;calculate circle
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[x]
	shl edx,16
	add edx,[y]
	mov esi,[Radius]
	call calculate_circle

	;save color pixels in ReserveArray
	square_width_save

	;draw calculated pixels on work arrea
	draw_calc_pixels width_pixels

	call MovePictureToWorkScreen
	ret
	no_circle:

	;************************zoom 1*************************
	cmp eax,45
	jne no_1_
	mov [k],1
	call scrollbar_move_00
	and [Current_instrument],0 ;???
	call drawwin
	ret
	no_1_:

	;************************zoom 2*************************
	cmp eax,46
	jne no_2_
	mov [k],2
	call drawwin
	call scrollbar_move_00
	and [Current_instrument],0
	ret
	no_2_:

	;************************zoom 3*************************
	cmp eax,47
	jne no_3_
	mov [k],3
	call drawwin
	call scrollbar_move_00
	and [Current_instrument],0
	ret
	no_3_:

	;************************zoom 4*************************
	cmp eax,48
	jne no_4_
	mov [k],4
	call drawwin
	call scrollbar_move_00
	and [Current_instrument],0
	ret
	no_4_:

	;************************zoom 8*************************
	cmp eax,49
	jne no_8_
	mov [k],8
	call drawwin
	call scrollbar_move_00
	and [Current_instrument],0
	ret
	no_8_:

	;************************zoom 16************************
	cmp eax,50
	jne no_16_
	mov [k],16
	call drawwin
	call scrollbar_move_00
	and [Current_instrument],0
	ret
	no_16_:

	;***************allocation of a countour****************
	cmp eax,21
	jne no_allocation

	mov [Activate_instrument],1

	call GetScreenCordinats

	bt [MouseBut],8 ;left button down
	jnc .left_but_down
		cmp [crossing],2
		jne @f
			;move old selection
			mov eax,[crossing_old_x]
			mov ebx,[crossing_old_y]
			mov ecx,[ScreenX]
			mov edx,[ScreenY]
			add ecx,[PosX]
			add edx,[PosY]
			mov esi,[rectangular_shade_x]
			mov edi,[rectangular_shade_y]
			sub esi,eax
			sub edi,ebx
			call columnus

			test eax,eax
			jz @f
				mov [crossing],3
				mov [OldX],ecx
				mov [OldY],edx
				jmp selection_update
		@@:
		;start new selection
		mov eax,[ScreenX]
		add eax,[PosX]
		mov [crossing_old_x],eax
		mov [rectangular_shade_x],eax
		mov eax,[ScreenY]
		add eax,[PosY]
		mov [crossing_old_y],eax
		mov [rectangular_shade_y],eax
		mov [crossing],1
		jmp selection_update
	.left_but_down:

	bt [MouseBut],0 ;left button press
	jnc .no_mouse_move
		cmp [crossing],3
		jne .no_sel_move
			;move selection
			mov eax,[ScreenX]
			add eax,[PosX]
			sub eax,[OldX]
			add [OldX],eax
			mov ebx,[crossing_old_x]
			add ebx,eax
			cmp ebx,0
			jge @f
				;exit from left border
				sub eax,ebx 
			@@:
			add [crossing_old_x],eax
			add [rectangular_shade_x],eax
			mov eax,[ScreenY]
			add eax,[PosY]
			sub eax,[OldY]
			add [OldY],eax
			mov ebx,[crossing_old_y]
			add ebx,eax
			cmp ebx,0
			jge @f
				;exit from top border
				sub eax,ebx 
			@@:
			add [crossing_old_y],eax
			add [rectangular_shade_y],eax
			jmp selection_update
		.no_sel_move:
		;resize selection
		mov eax,[ScreenX]
		add eax,[PosX]
		mov [rectangular_shade_x],eax
		mov eax,[ScreenY]
		add eax,[PosY]
		mov [rectangular_shade_y],eax
		jmp selection_update
	.no_mouse_move:

	bt [MouseBut],16 ;left button up
	jnc .left_but_up
		;finish selection
		mov eax,[crossing_old_x]
		mov ebx,[rectangular_shade_x]
		cmp eax,ebx
		jle @f
			mov [crossing_old_x],ebx
			mov [rectangular_shade_x],eax
		@@:
		mov eax,[crossing_old_y]
		mov ebx,[rectangular_shade_y]
		cmp eax,ebx
		jle @f
			mov [crossing_old_y],ebx
			mov [rectangular_shade_y],eax
		@@:
		mov [crossing],2
		ret
	.left_but_up:

	bt [MouseBut],9 ;right button down
	jnc .right_but_down
		;delete select
		mov [crossing],0
		jmp selection_update
	.right_but_down:

if 0
	cmp [Paste_flag],1
	je @f
		call PutFonForSprite
	@@:
	cmp [DrawSprite_flag],1
	jne @f
		call SaveFonForSprite
		call DrawSprite
	@@:
end if

	selection_update:
	call MovePictureToWorkScreen
	ret
	no_allocation:

	;*************reflection from left to right*************
	cmp eax,23
	jne no_reflection_from_left_to_right

	mov ebp,[PointerToPicture]
	mov edx,[ReserveArray]
	mov esi,[Picture_SizeX]
	mov ebx,[Picture_SizeX]
	lea ebx,[ebx+ebx*2]
	shr esi,1
	next_line_reflection_x:
	;copy vertical line to array
	mov ecx,[Picture_SizeX]
	shr ecx,1
	sub ecx,esi
	lea ecx,[ecx+ecx*2]
	add ecx,[PointerToPicture]
	mov edx,[ReserveArray]
	and edi,0
	copy_to_array_line_reflection_y:
	mov eax,[ecx]
	mov ebp,edi
	lea ebp,[ebp+ebp*2]
	mov [edx+ebp],ax
	shr eax,16
	mov [edx+ebp+2],al
	add ecx,ebx
	inc edi
	cmp edi,[Picture_SizeY]
	jne copy_to_array_line_reflection_y

	mov ecx,[Picture_SizeX]
	shr ecx,1
	mov ebp,ecx
	add ecx,esi
	dec ecx
	lea ecx,[ecx+ecx*2]
	add ecx,[PointerToPicture]
	sub ebp,esi
	lea ebp,[ebp+ebp*2]
	add ebp,[PointerToPicture]
	and edi,0
	next_line_reflection_y:
	mov eax,[ecx]
	and eax,0xffffff
	mov [ebp],ax
	shr eax,16
	mov [ebp+2],al
	add ebp,ebx
	add ecx,ebx
	inc edi
	cmp edi,[Picture_SizeY]
	jnz next_line_reflection_y

	;copy vertical line  from array to screen
	mov ecx,[Picture_SizeX]
	shr ecx,1
	add ecx,esi
	dec ecx
	lea ecx,[ecx+ecx*2]
	add ecx,[PointerToPicture]
	mov edx,[ReserveArray]
	and edi,0
	copy_from_array_to_screen_reflection_y:
	mov ebp,edi
	lea ebp,[ebp+ebp*2]
	xor eax,eax
	mov eax,[edx+ebp]
	mov [ecx],ax
	shr eax,16
	mov [ecx+2],al
	add ecx,ebx
	inc edi
	cmp edi,[Picture_SizeY]
	jne copy_from_array_to_screen_reflection_y

	dec esi
	jnz next_line_reflection_x

	call MovePictureToWorkScreen
	ret
	no_reflection_from_left_to_right:

	;*************reflection from up to down****************
	cmp eax,24
	jne no_reflection_from_up_to_down
	mov esi,[Picture_SizeX]
	mov edi,esi
	lea esi,[esi+esi*2]
	lea edi,[edi+edi*2]
	imul edi,[Picture_SizeY]
	mov edx,[ReserveArray]
	mov ecx,[Picture_SizeY]
	shr ecx,1
	add edi,[PointerToPicture]
	sub edi,esi
	mov ebp,[PointerToPicture]
	next_lines_reflection:
	;copy line
	xor ebx,ebx
	copy_line_1:
	xor eax,eax
	mov al,[edi+ebx]
	mov [edx+ebx],al
	inc ebx
	cmp ebx,esi
	jne copy_line_1

	xor ebx,ebx
	copy_line_2:
	xor eax,eax
	mov al,[ebp+ebx]
	mov [edi+ebx],al
	inc ebx
	cmp ebx,esi
	jne copy_line_2

	xor ebx,ebx
	copy_line_3:
	xor eax,eax
	mov al,[edx+ebx]
	mov [ebp+ebx],al
	inc ebx
	cmp ebx,esi
	jne copy_line_3

	add ebp,esi
	sub edi,esi
	dec ecx
	jnz next_lines_reflection

	call MovePictureToWorkScreen
	ret
	no_reflection_from_up_to_down:

	;*********************draw hard contour*****************
	cmp eax,20
	jne no_kontur_

	mov [Last_instrument],eax

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_konture_xy

	mov al,[instrument_used]
	test al,al
	jz instrument_not_used
	mov eax,[used_OldX]
	mov ebx,[used_OldY]
	mov [OldX],eax
	mov [OldY],ebx
	jmp exit_used_instrument
	instrument_not_used:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	exit_used_instrument:

	mov al,[instrument_used]
	test al,al
	jnz instrument_used_true
	mov [instrument_used],byte 1
	instrument_used_true:

	mov [Activate_instrument],1
	xor eax,eax
	mov ecx,mem_flood_f/4
	mov edi,[ReserveArray]
	rep stosd

	no_new_konture_xy:

	;put saved pixels
	square_width_put

	;calculate line
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[OldX]
	shl edx,16
	add edx,[OldY]
	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	mov ebp,[Picture_SizeX]
	sub ebp,[line_width]
	cmp esi,ebp
	jl no_minimum_x_konture
	mov esi,ebp
	no_minimum_x_konture:

	mov ebp,[Picture_SizeY]
	sub ebp,[line_width]
	cmp edi,ebp
	jl no_minimum_y_konture
	mov edi,ebp
	no_minimum_y_konture:

	call calculate_line

	;save color pixels in ReserveArray
	square_width_save

	;draw calculated pixels on work arrea
	draw_calc_pixels width_pixels

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [used_OldX],eax
	mov [used_OldY],ebx

	call MovePictureToWorkScreen
	ret
	no_kontur_:

	;************************draw ellips********************
	cmp eax,19
	jne no_ellips

	mov [Last_instrument],eax

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_ellips_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	mov [Activate_instrument],1
	xor eax,eax
	mov ecx,mem_flood_f/4
	mov edi,[ReserveArray]
	cld
	rep stosd
	ret
	no_new_ellips_xy:

	;put saved pixels
	square_width_put


	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_ellips
	neg esi
	shr esi,1
	neg esi
	add esi,[OldX]
	jmp no_plus_sign_x_ellips
	no_sign_x_ellips:

	shr esi,1
	add esi,[OldX]
	no_plus_sign_x_ellips:

	sub edi,[OldY]
	jns no_sign_y_ellips
	neg edi
	shr edi,1
	neg edi
	add edi,[OldY]
	jmp no_plus_sign_y_ellips
	no_sign_y_ellips:

	shr edi,1
	add edi,[OldY]
	no_plus_sign_y_ellips:

	mov [x],esi
	mov [y],edi

	mov esi,[ScreenX]
	mov edi,[ScreenY]
	add esi,[PosX]
	add edi,[PosY]

	sub esi,[OldX]
	jns no_sign_x_ellips_r
	neg esi
	no_sign_x_ellips_r:

	sub edi,[OldY]
	jns no_sign_y_ellips_r
	neg edi
	no_sign_y_ellips_r:

	cmp edi,0
	jnz no_null_a_ellips
	mov edi,1
	no_null_a_ellips:

	shr esi,1
	shr edi,1
	mov [a_ellips],esi
	mov [b_ellips],edi

	mov edi,[x]
	mov ebp,[y]
	add edi,esi
	add ebp,esi

	mov edx,[Picture_SizeX]
	sub edx,[line_width]
	cmp edi,edx
	jl no_limit_x_ellips
	sub edi,edx
	sub [a_ellips],edi
	no_limit_x_ellips:

	mov edx,[Picture_SizeY]
	sub edx,[line_width]
	cmp ebp,edx
	jl no_limit_y_ellips
	sub ebp,edx
	sub [b_ellips],ebp
	no_limit_y_ellips:


	mov edi,[x]
	mov ebp,[y]

	sub edi,[a_ellips]
	jns no_minimum_x_ellips
	add [a_ellips],edi
	no_minimum_x_ellips:

	sub ebp,[b_ellips]
	jns no_minimum_y_ellips
	add [b_ellips],ebp
	no_minimum_y_ellips:

	;calculate circle
	mov ebx,[ReserveArray]
	add ebx,4
	mov eax,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	mov edx,[x]
	shl edx,16
	add edx,[y]
	mov esi,[a_ellips]
	shl esi,16
	add esi,[b_ellips]
	call calculate_ellips

	;save color pixels in ReserveArray
	square_width_save

	;draw calculated pixels on work arrea
	draw_calc_pixels width_pixels

	call MovePictureToWorkScreen
	;ret
	no_ellips:

	ret