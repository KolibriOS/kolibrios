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
	ret

draw_icons:

	 mov [Icon_X],7
	 mov [Icon_Y],20+15+4

	 ;draw panel picture
	 mov edx,[Icon_X]
	 shl edx,16
	 add edx,[Icon_Y]
	 mov ebx,[ScreenPointer]
	 add ebx,mem_screen
	 mov ecx,417*65536+46
	 mcall SF_PUT_IMAGE


	 ;main buttons of instrumnts

	 and [counter],0
	 mov [Icon_X],9
	 mov [Icon_Y],21+15+4+1

   main_buttons:

	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
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

	 mov [Icon_X],8+1
	 mov [Icon_Y],20+15+4+25+1

	 next_button_brush:

	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
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

	 mov [Icon_X],8+155+1
	 mov [Icon_Y],20+15+4+25+1

	 next_button_zoom:

	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
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
	 mov [Icon_X],7+105+1
	 mov [Icon_Y],19+15+4+25+1

	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
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
	 mov [Icon_X],7+128+1
	 mov [Icon_Y],19+15+4+25+1

	 mov ebx,[Icon_X]
	 mov ecx,[Icon_Y]
	 dec ebx
	 dec ecx
	 shl ebx,16
	 shl ecx,16
	 add ebx,20
	 add ecx,20
	 mov edx,52
	 add edx,1000000000000000000000000000000b
	 mcall SF_DEFINE_BUTTON

	 mcall SF_DRAW_RECT, 136*65536+19,64*65536+19,[Color]

	 ret
