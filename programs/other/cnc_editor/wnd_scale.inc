;
; в этом файле собраны функции нужные для создания и
; работы окна с настройками
;

wnd_scale_width  equ 320 ;ширина окна с настройками
wnd_scale_height equ 150 ;высота окна с настройками

align 4
wnd_run_scale db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна со свойствами одновременно

;даное окно (процесс) вызываеться функцией but_wnd_coords
align 4
start_scale:
pushad
	mcall SF_SET_EVENTS_MASK,0x27 ;маска ожидаемых событий
	inc byte[wnd_run_scale]

	edit_boxes_set_sys_color edit3,editboxes_end_sc,sc ;установка системных цветов
	call get_scale
popad
	call red_win_scale

;не отрывать эту функцию от предыдущей
align 4
still_scale:
	pushad

	mcall SF_WAIT_EVENT_TIMEOUT,10
	or eax,eax
	jnz @f
		call timer_funct_scale
		jmp .end
	@@:

	cmp al,1 ;изм. положение окна
	jne @f
		call red_win_scale
		jmp .end
	@@:
	cmp al,2
	jne @f
		call key_scale
		jmp .end
	@@:
	cmp al,3
	jz button_scale
	cmp al,6
	jne @f 
		call mouse_scale
	@@:
.end:
	popad
	jmp still_scale

align 4
red_win_scale:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mcall SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
	mov edi,capt_opt ;children window caption
	mov bx,word[procinfo.box.left]
	add bx,word[buf_0.l]
	add bx,5 ;ширина боковой рамки
	shl ebx,16
	mov bx,wnd_scale_width
	mov cx,word[procinfo.box.top]
	add cx,ax ;add skin height
	add cx,word[buf_0.t]
	shl ecx,16
	mov cx,wnd_scale_height
	mov edx,[sc.work]
	or  edx,0x33000000
	xor eax,eax
	int 0x40

	mov esi,[sc.work_button]
	;ebx = (l<:)+w, ecx = (t<:)+h
	mcall SF_DEFINE_BUTTON, (5 shl 16)+59, (95 shl 16)+20, 3
	mcall , (75 shl 16)+59, (95 shl 16)+20, 4

	mov ecx,[sc.work_text]
	bts ecx,31 ;рисуемая строка заканчивается нулём
	mcall SF_DRAW_TEXT, (5 shl 16)+15,, capt_sc

	mov ecx,[sc.work_button_text]
	bts ecx,31
	mcall , (9 shl 16)+101,, txt_but_cancel
	mov edx,txt_but_apply
	mcall ,(79 shl 16)+101

	stdcall [edit_box_draw], edit3

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
proc key_scale uses eax ebx
	mcall SF_GET_KEY

	test word [edit3.flags],10b ;ed_focus
	je @f
		stdcall [edit_box_key], edit3
		;jmp .end
	@@:

	.end:
	ret
endp

align 4
mouse_scale:
	stdcall [edit_box_mouse], edit3
	ret

align 4
button_scale:
	mcall SF_GET_BUTTON

	cmp ah,3
	je .exit
	cmp ah,4
	jne .end_save
		push eax ecx edi esi
			finit

			mov esi,string3
			mov edi,Data_String
			cld
			mov ecx,8
			rep movsd
			call String_to_DoubleFloat
			fld  qword[Data_Double] ;считываем как double
			fstp qword[Scale1mm] ;сохраняем как double

		pop esi edi ecx eax
		jmp .exit
	.end_save:

	cmp ah,1
	jne still_scale.end
.exit:
	mov byte[wnd_run_scale],0 ;обнуляем счетчик окон
	mcall SF_TERMINATE_PROCESS ;выход из программы

align 4
get_scale:
	;загрузка координат в editbox-ы
	push eax
	mov word[NumberSymbolsAD],8
	finit
	fld qword[Scale1mm]
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	call String_crop_0
	stdcall [edit_box_set_text], edit3, Data_String
	;stdcall [edit_box_draw], edit3
	pop eax
	ret

align 4
proc timer_funct_scale
	;просматриваем выделенную координату
	ret
endp

align 4
if lang eq ru
capt_opt db 'Настройки',0
capt_sc db 'Масштаб:',0
else
capt_opt db 'Options',0
capt_sc db 'Scale:',0
end if

edit3 edit_box 80, 54, 11, 0xffd0d0, 0xff, 0x80ff, 0, 0x8000, 34, string3, mouse_dd, 0
editboxes_end_sc:

string3 rb 34
