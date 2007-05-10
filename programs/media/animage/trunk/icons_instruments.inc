;-----------------------------------------------------------
;-----instruments of panel(icon's instruments)--------------
;-----------------------------------------------------------
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

	jmp still
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

	jmp still
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

	jmp still
	no_brush3:

	;************************brush 4************************
	cmp eax,43
	jne no_brush4

	mov [Brush_SizeX],14
	mov [Brush_SizeY],14
	mov [Number_Brush],3
	mov ebx,[Last_instrument]
	mov [Current_instrument],ebx

	mov [line_width],4

	jmp still
	no_brush4:

	;************************brush 5************************
	cmp eax,44
	jne no_brush5

	mov [Brush_SizeX],6
	mov [Brush_SizeY],6
	mov [Number_Brush],4
	mov ebx,[Last_instrument]
	mov [Current_instrument],ebx

	mov [line_width],5

	jmp still
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

	mov eax,[MaxWorkScreen_SizeX]
	mov ebx,[MaxWorkScreen_SizeY]

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
	jmp still
	no_pensil:

	;**********************pipetka**************************
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

	mov eax,13
	mov ebx,136*65536+20
	mov ecx,65*65536+21
	mov edx,[Color]
	mcall

	jmp still
	no_pipetka:

	;**********************draw brush***********************
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

	jmp still
	no_brush:

	;************************Flood Fill*******************
	cmp eax,14
	jne no_FloodFill

	mov eax,[PointerToPicture]
	mov ebx,[PointerToEditBufer]

	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	shl ecx,1

	cmp ecx,500000
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

	jmp still
	no_FloodFill:

	;************************lastik*************************
	cmp eax,13
	jne no_lastik

	 mov [Last_instrument],eax
	 mov [lastik_is_active],1
	jmp lastik_in

	no_lastik:

	;******************************************************
	cmp eax,12
	jne  no_spray

	cmp [Activate_instrument],0
	jne no_null_spray
	mov [Activate_instrument],1
	jmp still
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
	jmp still

	no_spray:

	;***********************palette*************************
	cmp eax,51
	jne no_palette

	mov eax,20
	mov ebx,100
	mov ecx,32*10+32*5+7
	mov edx,8*10+7*10
	mov  esi,1
	call draw_volume_rectangle
	mov [x],20+5
	mov [y],100+10
	mov edi,256
	xor esi,esi
	next_rectangle:
	mov eax,13
	mov ebx,[x]
	mov ecx,[y]
	mov edx,[palitra+esi]
	and edx,0xffffff
	shl ebx,16
	shl ecx,16
	add ebx,13
	add ecx,13
	mcall
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
	call event

	cmp eax,1
	je still

	cmp eax,2
	jne no_keys

	mov eax,2
	mcall

	no_keys:

	cmp eax,3
	jne no_buttons
	mov eax,17
	mcall

	no_buttons:

	cmp eax,6
	jne wait_events

	call GetMouseClick

	test eax,eax
	jz wait_events
	call GetMouseCoordinats
	mov [x],20+5
	mov [y],100+10
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

	mov eax,5
	mov ebx,10
	mcall

	call drawwin

	mov eax,[Last_instrument]
	mov [Current_instrument],eax

	jmp still
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
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_line:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_line
	jmp still
	no_new_line_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_line
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_line:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put:
	   and [counter],0

	   horizontal_width_put:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_line

	no_put_line_to_screen_line:

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
	;call calculate_rectangle
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_line:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save:
	   and [counter],0

	   horizontal_width_save:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_line

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_line:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width:
	   and [counter],0

	   horizontal_width:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_line
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_line:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_line

	call MovePictureToWorkScreen

	jmp still
	no_line:

	;*********************************DRAW RECTANGLE****************************
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
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_rectangle:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_rectangle
	jmp still
	no_new_rectangle_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_rectangle_to_screen_line
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_rectangle:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_rectangle:
	   and [counter],0

	   horizontal_width_put_rectangle:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_rectangle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_rectangle
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_rectangle

	no_put_rectangle_to_screen_line:

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
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_rectangle:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_rectangle:
	   and [counter],0

	   horizontal_width_save_rectangle:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_rectangle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_rectangle
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_rectangle

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_rectangle:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_rectangle:
	   and [counter],0

	   horizontal_width_rectangle:
	   xor ecx,ecx
	   mov cl,byte[width_pixels_rectangle+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_rectangle
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_rectangle:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_rectangle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_rectangle

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_rectangle

	call MovePictureToWorkScreen
	jmp still
	no_rectangle:

	;*********************************DRAW CIRCLE****************************
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
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_circle:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_circle
	jmp still
	no_new_circle_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_circle
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_circle:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_circle:
	   and [counter],0

	   horizontal_width_put_circle:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_circle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_circle
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_circle

	no_put_line_to_screen_circle:

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

	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_circle:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_circle:
	   and [counter],0

	   horizontal_width_save_circle:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_circle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_circle
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_circle

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_circle:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_circle:
	   and [counter],0

	   horizontal_width_circle:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_circle
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_circle:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_circle

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_circle

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_circle

	call MovePictureToWorkScreen
	jmp still
	no_circle:

	;************************zoom 1*************************
	cmp eax,45
	jne no_1_
	mov [k],1
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],89
	and [Current_instrument],0
	call drawwin
	jmp still
	no_1_:

	;*************************zoom 2************************
	cmp eax,46
	jne no_2_
	mov [k],2
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],89
	and [Current_instrument],0
	jmp still
	no_2_:


	;*************************zoom 3************************
	cmp eax,47
	jne no_3_
	mov [k],3
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],89
	and [Current_instrument],0
	jmp still
	no_3_:

	;*************************zoom 4************************
	cmp eax,48
	jne no_4_
	mov [k],4
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],89
	and [Current_instrument],0
	jmp still
	no_4_:

	;************************zoom 8*************************
	cmp eax,49
	jne no_8_
	mov [k],8
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],89
	and [Current_instrument],0
	jmp still
	no_8_:

	;************************zoom 16************************
	cmp eax,50
	jne no_16_
	mov [k],16
	call drawwin
	mov [Scroll1CoordinatX],9
	mov [Scroll2CoordinatY],89
	and [Current_instrument],0
	jmp still
	no_16_:

	;***************allocation of a countour*********************
	cmp eax,21
	jne no_allocation

	cmp [instrument_used],0
	jnz instrument_not_finished_work

	mov al,[Activate_instrument]
	test al,al
	jnz no_new_allocation_xy
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [OldX],eax
	mov [OldY],ebx
	inc eax
	inc ebx
	mov [rectangular_shade_x],eax
	mov [rectangular_shade_y],ebx
	mov [Activate_instrument],1
	mov [instrument_used],1
	and [crossing],0
	and [finishing_crossing],0
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_allocation:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_allocation
	jmp still
	no_new_allocation_xy:
	instrument_not_finished_work:

	mov al,[instrument_used]
	test al,al
	jz no_change_coordinats

	cmp [Activate_instrument],0
	jnz no_change_coordinats

	;save coordinates as old if crossing=0
	cmp [crossing],0
	jnz no_save_coordinate_of_crossing

	mov eax,[OldX]
	mov ebx,[OldY]

	cmp eax,[rectangular_shade_x]
	jl no_remove_x
	mov ecx,[rectangular_shade_x]
	mov [OldX],ecx			  ; OldX <-----> rectangulare_shade_x
	mov [rectangular_shade_x],eax
	no_remove_x:

	cmp ebx,[rectangular_shade_y]
	jl no_remove_y
	mov ecx,[rectangular_shade_y]
	mov [OldY],ecx			  ; OldY <-----> rectangulare_shade_y
	mov [rectangular_shade_y],ebx
	no_remove_y:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [crossing_old_x],eax
	mov [crossing_old_y],ebx
	mov [crossing],1


	mov eax,[OldX]
	mov ebx,[OldY]
	inc eax
	inc ebx
	mov [SpriteCoordinatX],eax
	mov [SpriteCoordinatY],ebx
	mov [SpriteOldCoordinatX],eax
	mov [SpriteOldCoordinatY],ebx
	mov esi,[rectangular_shade_x]
	mov edi,[rectangular_shade_y]

	sub esi,eax
	jns no_sign_sprite_size_x
	neg esi
	no_sign_sprite_size_x:

	sub edi,ebx
	jns no_sign_sprite_size_y
	neg edi
	no_sign_sprite_size_y:

	test esi,esi
	jnz no_null_sprite_x
	mov esi,1
	no_null_sprite_x:

	test edi,edi
	jnz no_null_sprite_y
	mov edi,1
	no_null_sprite_y:

	mov [SpriteSizeX],esi
	mov [SpriteSizeY],edi

	call SaveFonForSprite

	no_save_coordinate_of_crossing:

	cmp [crossing],0
	je no_test_crossing_with_work_arrea
	;if mouse situatad after allocation than exit
	push [ScreenX]
	push [ScreenY]

	call GetScreenCordinats

	mov eax,[OldX]
	mov ebx,[OldY]
	mov ecx,[ScreenX]
	mov edx,[ScreenY]
	mov esi,[SpriteSizeX]
	mov edi,[SpriteSizeY]
	add ecx,[PosX]
	add edx,[PosY]
	inc esi;eax
	inc edi;ebx
	call columnus

	test eax,eax
	jnz crossing_with_work_area
	mov [finishing_crossing],1
	mov [register],1
	crossing_with_work_area:

	pop [ScreenY]
	pop [ScreenX]

	no_test_crossing_with_work_arrea:

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov ecx,eax
	mov edx,ebx
	sub eax,[crossing_old_x]  ;dx=(x-oldx)
	sub ebx,[crossing_old_y]  ;dy=(y-oldy)
	mov [crossing_old_x],ecx
	mov [crossing_old_y],edx

	add [OldX],eax		       ;x1=x1+dx
	add [OldY],ebx		       ;y1=y1+dy
	add [rectangular_shade_x],eax  ;x2=x2+dx
	add [rectangular_shade_y],ebx  ;y2+y2+dy

	mov eax,[OldX]
	mov ebx,[OldY]
	inc eax
	inc ebx
	mov [SpriteCoordinatX],eax
	mov [SpriteCoordinatY],ebx

	cmp [SpriteCoordinatX],0
	jns no_null_sprite_coordinat_x
	mov [SpriteCoordinatX],1
	no_null_sprite_coordinat_x:

	cmp [SpriteCoordinatY],0
	jns no_null_sprite_coordinat_y
	mov [SpriteCoordinatY],1
	no_null_sprite_coordinat_y:

	mov esi,[rectangular_shade_x]
	mov edi,[rectangular_shade_y]

	sub esi,[OldX]
	jns no_znak_size_of_rectangulare_crossing_x
	neg esi
	no_znak_size_of_rectangulare_crossing_x:

	sub edi,[OldY]
	jns no_znak_size_of_rectangulare_crossing_y
	neg edi
	no_znak_size_of_rectangulare_crossing_y:

	mov ecx,[OldX]
	mov edx,[OldY]

	sub ecx,[PosX]
	jns no_minimum_x_crossing
	mov ecx,0
	add ecx,[PosX]
	mov [OldX],ecx
	add ecx,esi
	mov [rectangular_shade_x],ecx
	no_minimum_x_crossing:

	sub edx,[PosY]
	jns no_minimum_y_crossing
	mov edx,0
	add edx,[PosY]
	mov [OldY],edx
	add edx,edi
	mov [rectangular_shade_y],edx
	no_minimum_y_crossing:

	mov ecx,[Picture_SizeX]
	sub ecx,esi
	cmp [OldX],ecx
	jl no_maximum_x_crossing
	dec ecx
	mov [OldX],ecx
	add ecx,esi
	mov [rectangular_shade_x],ecx
	no_maximum_x_crossing:

	mov edx,[Picture_SizeY]
	sub edx,edi
	cmp [OldY],edx
	jl no_maximum_y_crossing
	dec edx
	mov [OldY],edx
	add edx,edi
	mov [rectangular_shade_y],edx
	no_maximum_y_crossing:

	mov eax,[rectangular_shade_x]
	mov ebx,[rectangular_shade_y]
	sub eax,[PosX]
	sub ebx,[PosY]
	mov [ScreenX],eax
	mov [ScreenY],ebx
	no_change_coordinats:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_allocation
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_allocation:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_allocation:
	   and [counter],0

	   horizontal_width_put_allocation:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],1;5
	   jne horizontal_width_put_allocation

	   mov ecx,[Picture_SizeX]
	   sub ecx,1;5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],1;5
	jne vertical_width_put_allocation
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_allocation

	no_put_line_to_screen_allocation:

	cmp [DrawSprite_flag],1
	jne no_activate_put_fon_

	cmp [Paste_flag],1
	je no_put_fon___
	call PutFonForSprite
	no_put_fon___:
	and [Paste_flag],0
	no_activate_put_fon_:

	cmp [finishing_crossing],0
	jz not_finish_from_instrument_crossing
	and [Activate_instrument],0
	and [crossing],0
	and [instrument_used],0
	and [DrawSprite_flag],0
	call MovePictureToWorkScreen
	jmp still
	not_finish_from_instrument_crossing:

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
	dec ebp
	cmp esi,ebp
	jl no_minimum_x_allocation
	mov esi,ebp
	no_minimum_x_allocation:

	mov ebp,[Picture_SizeY]
	dec ebp
	cmp edi,ebp
	jl no_minimum_y_allocation
	mov edi,ebp
	no_minimum_y_allocation:

	call calculate_rectangle
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_allocation:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_allocation:
	   and [counter],0

	   horizontal_width_save_allocation:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],1;5
	   jne horizontal_width_save_allocation

	   mov ecx,[Picture_SizeX]
	   sub ecx,1;5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],1;5
	jne vertical_width_save_allocation
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_allocation

	cmp [DrawSprite_flag],1
	jne no_save_fon_for_sprite_
	;save current coordinats as old
	mov eax,[SpriteCoordinatX]
	mov ebx,[SpriteCoordinatY]
	mov [SpriteOldCoordinatX],eax
	mov [SpriteOldCoordinatY],ebx

	call SaveFonForSprite

	no_save_fon_for_sprite_:

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	add ebx,4
	next_pixel_put_allocation:

	mov edx,0x1f3fff
	mov esi,edx
	shr edx,16

	mov ebp,ecx
	and ebp,8
	cmp ebp,8
	jne black_color
	mov si,0xffff
	mov dl,0xff
	black_color:

	mov eax,[ebx]
	mov [eax],si
	mov [eax+2],dl

	add ebx,4
	dec ecx
	jnz next_pixel_put_allocation

	cmp [DrawSprite_flag],1
	jne no_activate_draw_sprite_

	call DrawSprite

	no_activate_draw_sprite_:

	mov al,[instrument_used]
	test al,al
	jz no_change_coordinats
	cmp [Activate_instrument],0
	jz no_save_shades
	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [rectangular_shade_x],eax
	mov [rectangular_shade_y],ebx
	no_save_shades:

	call MovePictureToWorkScreen
	jmp still
	no_allocation:

	;*************reflection from left to right******************
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
	xor eax,eax
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
	jmp still
	no_reflection_from_left_to_right:

	;*************reflection from up to down******************
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
	jmp still
	no_reflection_from_up_to_down:

	;*********************draw hard contour*******************
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
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_konture:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_line

	no_new_konture_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_konture
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_konture:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_konture:
	   and [counter],0

	   horizontal_width_put_konture:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_konture

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_konture
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_konture

	no_put_line_to_screen_konture:

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
	;call calculate_rectangle
	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_konture:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_konture:
	   and [counter],0

	   horizontal_width_save_konture:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_konture

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_konture
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_konture

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_konture:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_konture:
	   and [counter],0

	   horizontal_width_konture:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_konture
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_konture:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_konture

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_konture

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_konture

	mov eax,[ScreenX]
	mov ebx,[ScreenY]
	add eax,[PosX]
	add ebx,[PosY]
	mov [used_OldX],eax
	mov [used_OldY],ebx

	call MovePictureToWorkScreen

	jmp still

	no_kontur_:
	;************************Draw ellips***********************
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
	mov eax,[ReserveArray]
	mov ecx,60000
	clear_array_ellips:
	mov [eax],dword 0
	add eax,4
	dec ecx
	jnz clear_array_ellips
	jmp still
	no_new_ellips_xy:

	;put saved pixels
	mov ebx,[ReserveArray]
	mov eax,[ebx]
	test eax,eax
	jz no_put_line_to_screen_ellips
	mov ecx,[ebx]
	add ebx,4
	xor ebp,ebp
	next_color_put_ellips:
	;put saved pixels in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	push edi
	vertical_width_put_ellips:
	   and [counter],0

	   horizontal_width_put_ellips:
	      mov edx,[ebx+4807*4+ebp]
	      and edx,0xffffff
	      mov [edi],dx
	      shr edx,16
	      mov [edi+2],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_put_ellips

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_put_ellips
	pop edi
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_put_ellips

	no_put_line_to_screen_ellips:

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

	mov [counter],eax

	;save color pixels in ReserveArray
	mov eax,[counter]
	mov ebx,[ReserveArray]
	mov [ebx],eax

	mov ecx,[ebx]
	add ebx,4

	xor ebp,ebp
	next_color_save_ellips:
	;save color of pixel in ReserveArray
	push ecx
	and [counter2],0
	mov edi,[ebx]
	vertical_width_save_ellips:
	   and [counter],0

	   horizontal_width_save_ellips:
	      mov eax,edi
	      mov edx,[eax]
	      and edx,0xffffff
	      mov [ebx+4807*4+ebp],dx
	      shr edx,16
	      mov [ebx+4807*4+2+ebp],dl

	   add edi,3
	   add ebp,4
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_save_ellips

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add edi,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_save_ellips
	pop ecx
	add ebx,4
	dec ecx
	jnz next_color_save_ellips

	;draw calculated pixels on work arrea
	mov ebx,[ReserveArray]
	mov ecx,[ebx]
	mov edx,[Color]
	mov esi,[Color]
	shr edx,16
	add ebx,4
	mov edi,[line_width]
	dec edi
	imul edi,25
	next_pixel_put_ellips:

	mov eax,[ebx]
	push eax
	push ecx
	xor ebp,ebp
	and [counter2],0

	vertical_width_ellips:
	   and [counter],0

	   horizontal_width_ellips:
	   xor ecx,ecx
	   mov cl,byte[width_pixels+edi+ebp]
	   test cl,cl
	     jz no_draw_pixel_ellips
	      mov [eax],si
	      mov [eax+2],dl
	     no_draw_pixel_ellips:
	   add eax,3
	   inc ebp
	   inc [counter]
	   cmp [counter],5
	   jne horizontal_width_ellips

	   mov ecx,[Picture_SizeX]
	   sub ecx,5
	   lea ecx,[ecx+ecx*2]
	   add eax,ecx
	inc [counter2]
	cmp [counter2],5
	jne vertical_width_ellips

	pop ecx
	pop eax
	add ebx,4
	dec ecx
	jnz next_pixel_put_ellips

	call MovePictureToWorkScreen
	jmp still
	no_ellips:


	jmp still
	ret