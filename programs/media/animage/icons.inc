;-----------------------------------------------------------
;---load icons  in memory and draw icons on panel-----------
;-----------------------------------------------------------
load_icons:
	stdcall [img_decode], panel_picture, panel_picture.end-panel_picture, 0
	mov ebx,eax
	mov edi,[ScreenPointer]
	add edi,mem_screen
	stdcall [img_to_rgb2], ebx,edi ;преобразуем изображение к формату rgb
	stdcall [img_destroy], ebx ;удаляем временный буфер с параметрами изображения

	stdcall [img_decode], panel_zoom, panel_zoom.end-panel_zoom, 0
	mov ebx,eax
	add edi,mem_panel_but
	stdcall [img_to_rgb2], ebx,edi
	stdcall [img_destroy], ebx
	ret

draw_icons:
	mov [Icon_X],ci_panel_x_pos+2
	mov [Icon_Y],ci_panel_y_pos+3

	;draw panel picture
	mov edx,[Icon_X]
	shl edx,16
	add edx,[Icon_Y]
	mov ebx,[ScreenPointer]
	add ebx,mem_screen
	mov ecx,417*65536+46
	mcall SF_PUT_IMAGE

	;draw active zoom button
	add ebx,mem_panel_but
	xor eax,eax
	cmp [k],1
	je .end_calc
	inc eax
	cmp [k],2
	je @f
	inc eax
	cmp [k],3
	je @f
	inc eax
	cmp [k],4
	je @f
	inc eax
	cmp [k],8
	je @f
	inc eax
	cmp [k],16
	je @f
	inc eax
	@@:
		mov edx,eax
		imul edx,18*21*3
		add ebx,edx
	.end_calc:
	mov edx,eax
	imul edx,18+1
	add edx,ci_panel_zoom_x-1
	shl edx,16
	add edx,ci_panel_but_y2-2
	mov ecx,18*65536+21
	mcall SF_PUT_IMAGE

	;main buttons of instrumnts

	and [counter],0
	mov [Icon_X],ci_panel_x_pos+4

   main_buttons:

	 mov ebx,[Icon_X]
	 mov ecx,ci_panel_but_y1
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,18
	 add ecx,19
	 mov edx,[counter]
	 add edx,10
	 add edx,1000000000000000000000000000000b
	 mcall SF_DEFINE_BUTTON


	 add [Icon_X],22

	 inc [counter]
	 cmp [counter],19
	 jne main_buttons


	 ;buttons of brushes(and lines width)
	 and [counter],0

	 mov [Icon_X],ci_panel_x_pos+4

	 next_button_brush:

	 mov ebx,[Icon_X]
	 mov ecx,ci_panel_but_y2
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,18
	 add ecx,19
	 mov edx,[counter]
	 add edx,40
	 add edx,1000000000000000000000000000000b
	 mcall SF_DEFINE_BUTTON

	 add [Icon_X],19

	 inc [counter]
	 cmp [counter],5
	 jne next_button_brush


	 ;buttons of zoom
	 and [counter],0

	 mov [Icon_X],ci_panel_x_pos+ci_panel_zoom_x

	 next_button_zoom:

	 mov ebx,[Icon_X]
	 mov ecx,ci_panel_but_y2
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,18
	 add ecx,19
	 mov edx,[counter]
	 add edx,45
	 add edx,1000000000000000000000000000000b
	 mcall SF_DEFINE_BUTTON

	 add [Icon_X],19

	 inc [counter]
	 cmp [counter],6
	 jne next_button_zoom


	 ; button of palette
	 mov [Icon_X],ci_panel_x_pos+108

	 mov ebx,[Icon_X]
	 mov ecx,ci_panel_but_y2-1
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,20
	 add ecx,21
	 mov edx,51
	 add edx,1000000000000000000000000000000b
	 mcall SF_DEFINE_BUTTON

	 ; button of color
	 mov [Icon_X],ci_panel_x_pos+131

	 mov ebx,[Icon_X]
	 mov ecx,ci_panel_but_y2-1
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,20
	 add ecx,20
	 mov edx,52
	 add edx,1000000000000000000000000000000b
	 mcall SF_DEFINE_BUTTON

	mcall SF_DRAW_RECT, ((ci_panel_x_pos+130) shl 16)+21,\
		((ci_panel_y_pos+28) shl 16)+21, [Color]

	ret
