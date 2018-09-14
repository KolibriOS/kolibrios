;
; в этом файле собраны функции нужные для создания и
; работы окна с координатами вершин
;

wnd_prop_width  equ 320 ;ширина окна со свойствами объекта
wnd_prop_height equ 150 ;высота окна со свойствами объекта

align 4
wnd_run_prop db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна со свойствами одновременно
txt_p db '.',0

align 4
wnd_pObj dd 0 ;obj
wnd_pFig dd 0 ;фигура
wnd_FigN dd 0 ;fig number
wnd_pPoi dd 0 ;select point
wnd_PoiN dd 0 ;select point number


;даное окно (процесс) вызываеться функцией but_wnd_coords
align 4
start_prop:
pushad
	mcall SF_SET_EVENTS_MASK,0x27 ;маска ожидаемых событий
	inc byte[wnd_run_prop]

	;имя объекта
	stdcall mem_copy,capt_p,txt_obj,1+txt_obj.end-txt_obj
	mov edi,capt_p+txt_obj.end-txt_obj
	mov eax,[wnd_pObj]
	or eax,eax
	jz @f
		add eax,Object.Caption
		stdcall str_cat,edi,eax
	@@:
	stdcall str_cat,edi,txt_39
	;имя фигуры
	mov edi,capt_fig.name
	mov byte[edi],0 ;clear pervios name
	mov eax,[wnd_pFig]
	or eax,eax
	jz @f
		add eax,Figure.Caption
		stdcall str_cat,edi,eax
	@@:
	stdcall str_cat,edi,txt_39
	;номер точки
	mov edi,capt_poi.numb
	mov byte[edi],0
	mov eax,[wnd_PoiN]
	stdcall convert_int_to_str, 20

	edit_boxes_set_sys_color edit1,editboxes_end,sc ;установка системных цветов
	init_checkboxes2 check_boxes,check_boxes_end
	check_boxes_set_sys_color2 check_boxes,check_boxes_end,sc
	call get_point_coords
popad
	call red_win_prop

;не отрывать эту функцию от предыдущей
align 4
still_prop:
	pushad

	mcall SF_WAIT_EVENT_TIMEOUT,10
	or eax,eax
	jnz @f
		call timer_funct_prop
		jmp .end
	@@:

	cmp al,1 ;изм. положение окна
	jne @f
		call red_win_prop
		jmp .end
	@@:
	cmp al,2
	jne @f
		call key_prop
		jmp .end
	@@:
	cmp al,3
	jz button_prop
	cmp al,6
	jne @f 
		call mouse_prop
	@@:
.end:
	popad
	jmp still_prop

align 4
red_win_prop:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mcall SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
	mov edi,capt_p ;children window caption
	mov bx,word[procinfo.box.left]
	add bx,word[buf_0.l]
	add bx,5 ;ширина боковой рамки
	shl ebx,16
	mov bx,wnd_prop_width
	mov cx,word[procinfo.box.top]
	add cx,ax ;add skin height
	add cx,word[buf_0.t]
	shl ecx,16
	mov cx,wnd_prop_height
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
	mcall SF_DRAW_TEXT, (5 shl 16)+5,, capt_fig
	mcall , (5 shl 16)+15,, capt_poi

	mcall , (5 shl 16)+32,, capt_coord_x
	mcall , (5 shl 16)+50,, capt_coord_y

	mov ecx,[sc.work_button_text]
	bts ecx,31
	mcall , (9 shl 16)+101,, txt_but_cancel
	mov edx,txt_but_apply
	cmp dword[wnd_pPoi],0
	jne @f
		mov edx,txt_but_create
	@@:
	mcall ,(79 shl 16)+101

	stdcall [edit_box_draw], edit1
	stdcall [edit_box_draw], edit2
	checkboxes_draw2 check_boxes,check_boxes_end

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
proc key_prop uses eax ebx
	mcall SF_GET_KEY

	test word [edit1.flags],10b ;ed_focus
	je @f
		stdcall [edit_box_key], edit1
		jmp .end
	@@:
	test word [edit2.flags],10b
	je @f
		stdcall [edit_box_key], edit2
		;jmp .end
	@@:

	.end:
	ret
endp

align 4
mouse_prop:
	stdcall [edit_box_mouse], edit1
	stdcall [edit_box_mouse], edit2
	checkboxes_mouse2 check_boxes,check_boxes_end
	ret

align 4
button_prop:
	mcall SF_GET_BUTTON

	cmp ah,3
	je .exit
	cmp ah,4
	jne .end_save
		push eax ebx ecx edi esi
		mov ebx,[wnd_pPoi]
		or ebx,ebx
		jnz .select_1
			; *** create new point ***
			mov ebx,[wnd_pFig]
			mov eax,[ebx+Figure.PoiCount]
			inc eax
			imul eax,sizeof.Point
			stdcall mem.ReAlloc,[ebx+Figure.PoiData],eax
			mov [ebx+Figure.PoiData],eax
			inc dword[ebx+Figure.PoiCount]
			mov ebx,[ebx+Figure.PoiCount]
			dec ebx
			imul ebx,sizeof.Point
			add ebx,eax
		.select_1:
			finit

			mov esi,string1
			mov edi,Data_String
			cld
			mov ecx,8
			rep movsd
			call String_to_DoubleFloat
			fld  qword[Data_Double] ;считываем как double
			fstp qword[ebx+Point.CoordX] ;сохраняем как double

			mov esi,string2
			mov edi,Data_String
			cld
			mov ecx,8
			rep movsd
			call String_to_DoubleFloat
			fld  qword[Data_Double]
			fstp qword[ebx+Point.CoordY]

			stdcall figure_update_coords,[wnd_pObj],[wnd_FigN]
			;выставляем свойства
			and dword[ebx+Point.Prop], not 7 ;7 = 2^PROP_BIT_START +2^PROP_BIT_LINE +2^PROP_BIT_LINE_CLOSE)
			mov dword[offs_last_timer],0 ;для обновления блока в таймере
		pop esi edi ecx ebx eax
		jmp .exit
	.end_save:

	cmp ah,1
	jne still_prop.end
.exit:
	mov byte[wnd_run_prop],0 ;обнуляем счетчик окон
	mcall SF_TERMINATE_PROCESS ;выход из программы

align 4
proc get_point_coords uses eax ebx
	;загрузка координат в editbox-ы
	mov ebx,[wnd_pPoi]
	btr dword[ch1.flags],1
	btr dword[ch2.flags],1
	btr dword[ch3.flags],1
	or ebx,ebx
	jz .end0
		mov word[NumberSymbolsAD],8
		finit
		fld qword[ebx+Point.CoordX]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		stdcall [edit_box_set_text], edit1, Data_String
		fld qword[ebx+Point.CoordY]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		stdcall [edit_box_set_text], edit2, Data_String
		;
		jmp .end1
	.end0:
		stdcall [edit_box_set_text], edit1, txt_p
		stdcall [edit_box_set_text], edit2, txt_p
	.end1:
		;stdcall [edit_box_draw], edit1
		;stdcall [edit_box_draw], edit2
	ret
endp

align 4
proc timer_funct_prop
	;просматриваем выделенную координату
	ret
endp

align 4
capt_p rb 64
if lang eq ru
capt_fig: db 'Команда: ',39
.name: rb 55
capt_poi: db 'Точка: '
.numb: rb 25
else
capt_fig: db 'Command: ',39
.name: rb 55
capt_poi: db 'Point: '
.numb: rb 25
end if

capt_coord_x db 'X:',0
capt_coord_y db 'Y:',0

edit1 edit_box 80, 18, 28, 0xffd0d0, 0xff, 0x80ff, 0, 0x8000, 34, string1, mouse_dd, 0
edit2 edit_box 80, 18, 46, 0xd0d0ff, 0xff, 0x80ff, 0, 0x8000, 34, string2, mouse_dd, 0
editboxes_end:

string1 rb 34
string2 rb 34

check_boxes:
ch1 check_box2 (115 shl 16+15),(28 shl 16 +11),5,0xffffff,0,0x80000000,ch_text.1,ch_flag_en+ch_flag_middle
ch2 check_box2 (115 shl 16+15),(43 shl 16 +11),5,0xffffff,0,0x80000000,ch_text.2,ch_flag_en+ch_flag_middle
ch3 check_box2 (115 shl 16+15),(58 shl 16 +11),5,0xffffff,0,0x80000000,ch_text.3,ch_flag_en+ch_flag_middle
check_boxes_end:

ch_text:
.1 db 'X',0
.2 db 'Y',0
.3 db 'Z',0
