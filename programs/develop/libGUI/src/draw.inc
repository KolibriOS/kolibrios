
DrawLine:

      mov eax,38
      mov ebx,[Line.x1]
      mov ecx,[Line.y1]
      mov edx,[Line.color]
      shl ebx,16
      shl ecx,16
      add ebx,[Line.x2]
      add ecx,[Line.y2]
      mcall

      ret

DrawPixel:

      xor eax,eax
      inc al
      mov ebx,[Pixel.x]
      mov ecx,[Pixel.y]
      mov edx,[Pixel.color]
      mcall

      ret

RectangleContour:

      mov eax,38
      mov ebx,[Rectangle.x]
      mov ecx,[Rectangle.y]
      mov edx,[Rectangle.color]
      mov esi,ebx
      mov edi,ecx
      shl ebx,16
      shl ecx,16
      add ebx,esi
      add ecx,edi
      add ebx,[Rectangle.width]
      mcall

      mov eax,38
      mov ebx,[Rectangle.x]
      mov ecx,[Rectangle.y]
      mov edx,[Rectangle.color]
      add ecx,[Rectangle.height]
      mov esi,ebx
      mov edi,ecx
      shl ebx,16
      shl ecx,16
      add ebx,esi
      add ecx,edi
      add ebx,[Rectangle.width]
      mcall

      mov eax,38
      mov ebx,[Rectangle.x]
      mov ecx,[Rectangle.y]
      mov edx,[Rectangle.color]
      mov esi,ebx
      mov edi,ecx
      shl ebx,16
      shl ecx,16
      add ebx,esi
      add ecx,edi
      add ecx,[Rectangle.height]
      mcall

      mov eax,38
      mov ebx,[Rectangle.x]
      mov ecx,[Rectangle.y]
      mov edx,[Rectangle.color]
      add ebx,[Rectangle.width]
      mov esi,ebx
      mov edi,ecx
      shl ebx,16
      shl ecx,16
      add ebx,esi
      add ecx,edi
      add ecx,[Rectangle.height]
      mcall

      ret

;eax -first color
;ebx- second color
;OUT
;eax - averege color
calculate_average_color:

      and eax,0xffffff
      and ebx,0xffffff

      mov [b_min],al
      mov [b_max],bl
      shr eax,8
      shr ebx,8
      mov [g_min],al
      mov [g_max],bl
      shr eax,8
      shr ebx,8
      mov [r_min],al
      mov [r_max],bl

      xor eax,eax
      xor ebx,ebx
      mov al,[r_max]
      mov bl,[r_min]
      add eax,ebx
      shr eax,1
      mov [r],al

      xor eax,eax
      xor ebx,ebx
      mov al,[g_max]
      mov bl,[g_min]
      add eax,ebx
      shr eax,1
      mov [g],al

      xor eax,eax
      xor ebx,ebx
      mov al,[b_max]
      mov bl,[b_min]
      add eax,ebx
      shr eax,1
      mov [b],al

      xor eax,eax
      mov al,[r]
      shl eax,8
      mov al,[g]
      shl eax,8
      mov al,[b]

      ret

;eax -color
;ebx- sub value
;OUT
;eax - sabved color
calculate_sabved_color:

      and eax,0xffffff

      mov [b],al
      shr eax,8
      mov [g],al
      shr eax,8
      mov [r],al

      sub [r],bl
      sub [g],bl
      sub [b],bl

      xor eax,eax
      mov al,[r]
      shl eax,8
      mov al,[g]
      shl eax,8
      mov al,[b]

      ret

DrawString:

      mov ebx,[Button.text]
      call GetLengthString
      mov esi,eax

      mov eax,4
      mov ebx,[Font.x]
      shl ebx,16
      add ebx,[Font.y]
      mov ecx,[Button.textcolor]
      mov edx,[Button.text]
      mcall

      ret

;eax -first color
;ebx- second color
;ecx- x coordinat of rectangle
;edx- y coordinat of rectangle
;esi- rectangle size x
;edi- rectangle size y

rectangle_gradient_right:

      mov [line_coordinat_x],ecx
      mov [line_coordinat_y],edx
      mov [line_size_x],esi
      mov [line_size_y],edi

      mov ecx,esi
      mov edx,dword pointer
      call gradient

      mov ecx,[line_coordinat_y]
      shl ecx,16
      add ecx,[line_coordinat_y]
      add ecx,[line_size_y]

      mov eax,[line_size_x]
      add [line_coordinat_x],eax

      mov esi,dword pointer
      mov edi,[line_size_x]

      next_vertical_line_draw_right:

	mov eax,38
	mov ebx,[line_coordinat_x]
	shl ebx,16
	add ebx,[line_coordinat_x]
	mov edx,[esi]
	and edx,0xffffff
	mcall

	add esi,3
	sub [line_coordinat_x],1

      dec edi
      jnz  next_vertical_line_draw_right

      ret

;eax -first color
;ebx- second color
;ecx- x coordinat of rectangle
;edx- y coordinat of rectangle
;esi- rectangle size x
;edi- rectangle size y

rectangle_gradient_left:

      mov [line_coordinat_x],ecx
      mov [line_coordinat_y],edx
      mov [line_size_x],esi
      mov [line_size_y],edi

      mov ecx,esi
      mov edx,dword pointer
      call gradient

      mov ecx,[line_coordinat_y]
      shl ecx,16
      add ecx,[line_coordinat_y]
      add ecx,[line_size_y]

      mov esi,dword pointer
      mov edi,[line_size_x]

      next_vertical_line_draw_left:

	mov eax,38
	mov ebx,[line_coordinat_x]
	shl ebx,16
	add ebx,[line_coordinat_x]
	mov edx,[esi]
	and edx,0xffffff
	mcall

	add esi,3
	add [line_coordinat_x],1

      dec edi
      jnz  next_vertical_line_draw_left

      ret

;eax -first color
;ebx- second color
;ecx- x coordinat of rectangle
;edx- y coordinat of rectangle
;esi- rectangle size x
;edi- rectangle size y

rectangle_gradient_up:

      mov [line_coordinat_x],ecx
      mov [line_coordinat_y],edx
      mov [line_size_x],esi
      mov [line_size_y],edi

      mov ecx,edi
      mov edx,dword pointer
      call gradient

      mov ebx,[line_coordinat_x]
      shl ebx,16
      add ebx,[line_coordinat_x]
      add ebx,[line_size_x]

      mov eax,[line_size_y]
      add [line_coordinat_y],eax

      mov esi,dword pointer
      mov edi,[line_size_y]

      next_horizontal_line_draw_up:

	mov eax,38
	mov ecx,[line_coordinat_y]
	shl ecx,16
	add ecx,[line_coordinat_y]
	mov edx,[esi]
	and edx,0xffffff
	mcall

	add esi,3
	sub [line_coordinat_y],1

      dec edi
      jnz  next_horizontal_line_draw_up

      ret

;eax -first color
;ebx- second color
;ecx- x coordinat of rectangle
;edx- y coordinat of rectangle
;esi- rectangle size x
;edi- rectangle size y

rectangle_gradient_down:

      mov [line_coordinat_x],ecx
      mov [line_coordinat_y],edx
      mov [line_size_x],esi
      mov [line_size_y],edi

      mov ecx,edi
      mov edx,dword pointer
      call gradient

      mov ebx,[line_coordinat_x]
      shl ebx,16
      add ebx,[line_coordinat_x]
      add ebx,[line_size_x]

      mov esi,dword pointer
      mov edi,[line_size_y]

      next_horizontal_line_draw_down:

	mov eax,38
	mov ecx,[line_coordinat_y]
	shl ecx,16
	add ecx,[line_coordinat_y]
	mov edx,[esi]
	and edx,0xffffff
	mcall

	add esi,3
	add [line_coordinat_y],1

      dec edi
      jnz  next_horizontal_line_draw_down

      ret

;eax -first color
;ebx- second color
;ecx- length of line
;edx- pointer to memory for colors of gradient

gradient:

      mov [length],ecx

      and eax,0xffffff
      and eax,0xffffff

      mov [b_min],al
      mov [b_max],bl
      shr eax,8
      shr ebx,8
      mov [g_min],al
      mov [g_max],bl
      shr eax,8
      shr ebx,8
      mov [r_min],al
      mov [r_max],bl

      mov eax,[length]
      dec eax
      mov [v],eax
      fild [v]

      xor eax,eax
      mov al,[r_max]
      sub al,[r_min]
      mov [v],eax
      fild [v]
      fdiv st0,st1
      fstp [step_r]

      xor eax,eax
      mov al,[g_max]
      sub al,[g_min]
      mov [v],eax
      fild [v]
      fdiv st0,st1
      fstp [step_g]

      xor eax,eax
      mov al,[b_max]
      sub al,[b_min]
      mov [v],eax
      fild [v]
      fdiv st0,st1
      fstp [step_b]

      fstp [v]

      xor eax,eax
      mov al,[r_min]
      mov [r],al
      mov [v],eax
      fild [v]
      fstp [r_f]

      xor eax,eax
      mov al,[g_min]
      mov [g],al
      mov [v],eax
      fild [v]
      fstp [g_f]

      xor eax,eax
      mov al,[b_min]
      mov [b],al
      mov [v],eax
      fild [v]
      fstp [b_f]

      next_gradient_color_save:

	xor ebx,ebx
	xor eax,eax
	mov al,[r]
	mov bl,al
	shl eax,8
	mov al,[g]
	shl eax,8
	mov al,[b]

	mov [edx],ax
	mov [edx+2],bl

	fld [step_r]
	fld [r_f]
	fadd st0,st1
	fst [r_f]
	fistp [v]
	xor eax,eax
	mov eax,[v]
	mov [r],al
	fstp [v]

	fld [step_g]
	fld [g_f]
	fadd st0,st1
	fst [g_f]
	fistp [v]
	xor eax,eax
	mov eax,[v]
	mov [g],al
	fstp [v]

	fld [step_b]
	fld [b_f]
	fadd st0,st1
	fst [b_f]
	fistp [v]
	xor eax,eax
	mov eax,[v]
	mov [b],al
	fstp [v]

	add edx,3

      dec ecx
      jnz  next_gradient_color_save

      ret

;IN
;NON
;OUT
;variable [skin_height]

get_skin_height:

      pushad

      mov eax,48
      mov ebx,4
      int 0x40

      mov [skin_height],eax
      popad

      ret
;eax - x
;ebx - y
;ecx - size x
;edx - size y
;esi - pointer to memory for rectangle
SaveFonForRectangle:

     mov [Rectangle.x],eax
     mov [Rectangle.y],ebx
     mov [Rectangle.width],ecx
     mov [Rectangle.height],edx
     mov [PointerToMem],esi

     ;get bytes per string
     mov eax,61
     mov ebx,2
     mcall
     mov [BitsPerPixel],eax

     mov eax,61
     mov ebx,3
     mcall
     mov [BytesPerString],eax


     ;get window coordinats
     mov eax,9
     mov ebx,dword IPC_table
     or ecx,-1
     mcall

     mov eax,dword[IPC_table+34]
     mov ebx,dword[IPC_table+38]
     ;mov ecx,dword[IPC_table+42]
     ;mov edx,dword[IPC_table+46]
     ;mov [WindowCoordinatX],eax
     ;mov [WindowCoordinatY],ebx
     ;mov [WindowSizeX],ecx
     ;mov [WindowSizeY],edx

     add eax,[Rectangle.x]
     add ebx,[Rectangle.y]

     imul ebx,[BytesPerString]
     mov esi,[Rectangle.width]

     cmp [BitsPerPixel],24
     jne no_24

     lea eax,[eax+eax*2]
     lea esi,[esi+esi*2]
     mov edi,3
     jmp exit_bits_per_pixel
     no_24:

     shl eax,2
     shl esi,2
     mov edi,4
     exit_bits_per_pixel:

     add eax,ebx

     mov ebx,[BytesPerString]
     sub ebx,esi
     mov [offset],ebx

     mov esi,[PointerToMem]
     mov edx,[Rectangle.height]
     next_string_pixels_save:

       mov ecx,[Rectangle.width]
       next_pixel_save:

	 mov ebx,[gs:eax]
	 and ebx,0xffffff
	 mov [esi],bx
	 shr ebx,16
	 mov [esi+2],bl

	 add esi,3
	 add eax,edi
       dec ecx
       jnz next_pixel_save

       add eax,[offset]
     dec edx
     jnz next_string_pixels_save

     exit_:
     ret
