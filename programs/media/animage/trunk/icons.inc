;-----------------------------------------------------------
;---load icons  in memory and draw icons on panel-----------
;-----------------------------------------------------------
load_icons:
	 mov esi,panel_picture
	 mov edi,[ScreenPointer]
	 add edi,(1200*1000*3)
	 ;mov edi,[PointerToIcons]
	 call ReadGIF

	 ret

draw_icons:

	 mov [Icon_X],7
	 mov [Icon_Y],20+15+4

	 ;draw panel picture
	 mov edx,[Icon_X]
	 shl edx,16
	 add edx,[Icon_Y]
	 mov ebx,[ScreenPointer]
	 add ebx,(1200*1000*3)+8
	 mov eax,7
	 mov ecx,417*65536+46
	 mcall


	 ;main buttons of instrumnts

	 and [counter],0
	 mov [Icon_X],9
	 mov [Icon_Y],21+15+4+1

   main_buttons:

	 mov eax,8
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
	 mcall


	 add [Icon_X],22

	 inc [counter]
	 cmp [counter],19
	 jne main_buttons


	 ;buttons of brushes(and lines width)
	 and [counter],0

	 mov [Icon_X],8+1
	 mov [Icon_Y],20+15+4+25+1

	 next_button_brush:

	 mov eax,8
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
	 mcall

	 add [Icon_X],19

	 inc [counter]
	 cmp [counter],5
	 jne next_button_brush


	 ;buttons of zoom
	 and [counter],0

	 mov [Icon_X],8+155+1
	 mov [Icon_Y],20+15+4+25+1

	 next_button_zoom:

	 mov eax,8
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
	 mcall

	 add [Icon_X],19

	 inc [counter]
	 cmp [counter],6
	 jne next_button_zoom


	 ; button of palette
	 mov [Icon_X],7+105+1
	 mov [Icon_Y],19+15+4+25+1

	 mov eax,8
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
	 mcall

	 ; button of color
	 mov [Icon_X],7+128+1
	 mov [Icon_Y],19+15+4+25+1

	 mov eax,8
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
	 mcall

	 mov eax,13
	 mov ebx,136*65536+19
	 mov ecx,64*65536+19
	 mov edx,[Color]
	 mcall

	 ret
