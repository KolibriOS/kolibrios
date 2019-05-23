;
; в этом файле собраны функции нужные для создания и
; работы окна создания нового файла
;

wnd_n_file_width  equ 300 ;ширина окна создания нового файла
wnd_n_file_height equ 200 ;высота окна создания нового файла

align 4
wnd_n_file db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна создания нового файла

align 4
NOptCoordX0 dq 35.0
NOptCoordY0 dq 25.0
NOptCoordX1 dq 20.0 ;for circle Radius
NOptCoordY1 dq 16.0 ;S
NOptCoordZ0 dq  1.0
NOptCoordZ1 dq  5.0

;даное окно (процесс) вызываеться функцией but_wnd_coords
align 4
start_n_file:
pushad
	mcall SF_SET_EVENTS_MASK,0x27 ;маска ожидаемых событий
	inc byte[wnd_n_file]

	edit_boxes_set_sys_color edit4,editboxes_end_nf,sc ;установка системных цветов
	option_boxes_set_sys_color sc,opt_grlist1

	call get_n_file_opt
popad
	call red_win_n_file

;не отрывать эту функцию от предыдущей
align 4
still_n_file:
	pushad

	mcall SF_WAIT_EVENT_TIMEOUT,10
	or eax,eax
	jnz @f
		call timer_funct_n_file
		jmp .end
	@@:

	cmp al,1 ;изм. положение окна
	jne @f
		call red_win_n_file
		jmp .end
	@@:
	cmp al,2
	jne @f
		call key_n_file
		jmp .end
	@@:
	cmp al,3
	jz button_n_file
	cmp al,6
	jne @f 
		call mouse_n_file
	@@:
.end:
	popad
	jmp still_n_file

align 4
red_win_n_file:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mcall SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
	mov edi,capt_n_file ;children window caption
	mov bx,word[procinfo.box.left]
	add bx,word[buf_0.l]
	add bx,5 ;ширина боковой рамки
	shl ebx,16
	mov bx,wnd_n_file_width
	mov cx,word[procinfo.box.top]
	add cx,ax ;add skin height
	add cx,word[buf_0.t]
	shl ecx,16
	mov cx,wnd_n_file_height
	mov edx,[sc.work]
	or  edx,0x33000000
	xor eax,eax
	int 0x40

	mov esi,[sc.work_button]
	;ebx = (l<:)+w, ecx = (t<:)+h
	mcall SF_DEFINE_BUTTON, (5 shl 16)+59, (145 shl 16)+20, 3
	mcall , (75 shl 16)+59, (145 shl 16)+20, 4

	mov ecx,[sc.work_text]
	bts ecx,31 ;рисуемая строка заканчивается нулём
	mcall SF_DRAW_TEXT, (5 shl 16)+5,, capt_sel_fig

	mcall , (5 shl 16)+32,, capt_coord_x0
	mcall , (5 shl 16)+50,, capt_coord_y0
	cmp [opt_gr1],opt1
	jne @f
		;circle
		mcall , (5 shl 16)+68,, capt_coord_r
		mcall , (5 shl 16)+86,, capt_coord_s
		jmp .end0
	@@:
		;rect
		mcall , (5 shl 16)+68,, capt_coord_x1
		mcall , (5 shl 16)+86,, capt_coord_y1
	.end0:
	mcall , (5 shl 16)+104,,capt_coord_z0
	mcall , (5 shl 16)+122,,capt_coord_z1

	mov ecx,[sc.work_button_text]
	bts ecx,31
	mcall , (9 shl 16)+151,, txt_but_cancel
	mcall ,(79 shl 16)+151,, txt_but_create

	stdcall [option_box_draw], opt_grlist1

	stdcall [edit_box_draw], edit4
	stdcall [edit_box_draw], edit5
	stdcall [edit_box_draw], edit6
	stdcall [edit_box_draw], edit7
	stdcall [edit_box_draw], edit8
	stdcall [edit_box_draw], edit9

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
proc key_n_file uses eax ebx
	mcall SF_GET_KEY

	test word [edit4.flags],10b ;ed_focus
	je @f
		stdcall [edit_box_key], edit4
		jmp .end
	@@:
	test word [edit5.flags],10b
	je @f
		stdcall [edit_box_key], edit5
		jmp .end
	@@:
	test word [edit6.flags],10b
	je @f
		stdcall [edit_box_key], edit6
		jmp .end
	@@:
	test word [edit7.flags],10b
	je @f
		stdcall [edit_box_key], edit7
		jmp .end
	@@:
	test word [edit8.flags],10b
	je @f
		stdcall [edit_box_key], edit8
		jmp .end
	@@:
	test word [edit9.flags],10b
	je @f
		stdcall [edit_box_key], edit9
		;jmp .end
	@@:
	.end:
	ret
endp

align 4
mouse_n_file:
	stdcall [option_box_mouse], opt_grlist1

	stdcall [edit_box_mouse], edit4
	stdcall [edit_box_mouse], edit5
	stdcall [edit_box_mouse], edit6
	stdcall [edit_box_mouse], edit7
	stdcall [edit_box_mouse], edit8
	stdcall [edit_box_mouse], edit9
	ret

align 4
button_n_file:
	mcall SF_GET_BUTTON

	cmp ah,3
	je .exit
	cmp ah,4
	jne .end_save
		push eax ecx edi esi
		finit

		mov esi,string4
		mov edi,Data_String
		cld
		mov ecx,8
		rep movsd
		call String_to_DoubleFloat
		fld  qword[Data_Double] ;считываем как double
		fstp qword[NOptCoordX0] ;сохраняем как double

		mov esi,string5
		mov edi,Data_String
		cld
		mov ecx,8
		rep movsd
		call String_to_DoubleFloat
		fld  qword[Data_Double]
		fstp qword[NOptCoordY0]

		mov esi,string6
		mov edi,Data_String
		cld
		mov ecx,8
		rep movsd
		call String_to_DoubleFloat
		fld  qword[Data_Double]
		fstp qword[NOptCoordX1]

		mov esi,string7
		mov edi,Data_String
		cld
		mov ecx,8
		rep movsd
		call String_to_DoubleFloat
		fld  qword[Data_Double]
		fstp qword[NOptCoordY1]

		mov esi,string8
		mov edi,Data_String
		cld
		mov ecx,8
		rep movsd
		call String_to_DoubleFloat
		fld  qword[Data_Double]
		fstp qword[NOptCoordZ0]

		mov esi,string9
		mov edi,Data_String
		cld
		mov ecx,8
		rep movsd
		call String_to_DoubleFloat
		fld  qword[Data_Double]
		fstp qword[NOptCoordZ1]

		mov word[NumberSymbolsAD],3
		;fld qword[NOptCoordZ1]
		;fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		mov dword[txt_01.z],0
		mov dword[txt_05.z],0
		mov dword[txt_06.z],0
		stdcall str_cat, txt_01.z, Data_String
		stdcall str_cat, txt_01.z, txt_nl
		stdcall str_cat, txt_01.z, txt_nl
		stdcall str_cat, txt_05.z, Data_String
		stdcall str_cat, txt_05.z, txt_nl
		stdcall str_cat, txt_05.z, txt_nl
		stdcall str_cat, txt_06.z, Data_String
		
		fld qword[NOptCoordZ0]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		mov dword[txt_03.z],0
		stdcall str_cat, txt_03.z, Data_String

		fld qword[NOptCoordX0]
		cmp [opt_gr1],opt1
		jne @f
			;circle
			fadd qword[NOptCoordX1]
		@@:
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		mov dword[txt_04.x],0
		stdcall str_cat, txt_04.x, Data_String
		stdcall str_cat, txt_04.x, txt_s_poi_Y
		fld qword[NOptCoordY0]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		stdcall str_cat, txt_04.x, Data_String
		mov dword[txt_02.x],0
		stdcall str_cat, txt_02.x, txt_04.x
		stdcall str_cat, txt_02.x, txt_s_poi_Z
		fld qword[NOptCoordZ1]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		stdcall str_cat, txt_02.x, Data_String

		;clear old file
		stdcall [tl_node_poi_get_info], tree1,0
		or eax,eax
		jz @f
			mov ebx,eax
			stdcall [tl_node_poi_get_data], tree1,ebx
			stdcall clear_object_in_list, eax
		@@:
		stdcall [tl_info_clear], tree1 ;очистка списка объектов
		and dword[tree1.style],not tl_cursor_pos_limited
		mov dword[offs_last_timer],0

		stdcall ObjectCreate,ObjData
		or dword[tree1.style], tl_cursor_pos_limited
		stdcall [tl_cur_beg], tree1

		mov dword[offs_last_timer],0 ;для обновления блока в таймере
		pop esi edi ecx eax
		jmp .exit
	.end_save:

	cmp ah,1
	jne still_n_file.end
.exit:
	mov byte[wnd_n_file],0 ;обнуляем счетчик окон
	mcall SF_TERMINATE_PROCESS ;выход из программы

align 4
proc ObjectCreate, pobj:dword
pushad
	mov ebx,[pobj]
	mov [ebx+Object.OType],'Obj'
	mov edi,ebx
	add edi,Object.Caption
	mov dword[edi],'CNC'

	mov eax,7 ;max lines
	mov [ebx+Object.FigCount],eax
	mov ecx,eax

	shl eax,2
	stdcall mem.Alloc,eax
	mov [ebx+Object.FigData],eax
	push ecx
		mov edi,eax
		xor eax,eax
		rep stosd ;clear memory
	pop ecx

	stdcall add_object_in_list,0,0,ebx,0
	;eax -> pointer to object node
	mov edi,[ebx+Object.FigData]

	mov esi,txt_00
	stdcall FigureInit,FigData,edi
	add edi,4
	mov esi,txt_01
	stdcall FigureInit,FigData,edi
	add edi,4
	mov esi,txt_02
	stdcall FigureInit,FigData,edi
	add edi,4
	mov esi,txt_03
	stdcall FigureInit,FigData,edi
	add edi,4
	mov esi,txt_04 ;X__ Y__
	stdcall FigureInit,FigData,edi
	; *** create new points ***
	mov edx,[edi]
	cmp [opt_gr1],opt1
	jne .rect
		;circle
		fld qword[NOptCoordY1]
		fistp dword[edx+Figure.PoiCount]
		inc dword[edx+Figure.PoiCount]
		mov eax,[edx+Figure.PoiCount]
		imul eax,sizeof.Point
		stdcall mem.ReAlloc,[edx+Figure.PoiData],eax
		mov [edx+Figure.PoiData],eax

		;init points
		fldpi
		fadd st0,st0
		fdiv qword[NOptCoordY1]
		fldz
		;st0 - angle (=0)
		;st1 - delta angle (=pi/slices)
		
		mov ecx,[edx+Figure.PoiCount]
		cmp ecx,3
		jge @f
			mov ecx,3
		@@:
		mov edx,[edx+Figure.PoiData]
		add edx,Point.CoordX
		.cycle0:
			add edx,sizeof.Point
			fadd st0,st1 ;angle += delta angle
		
			fld st0
			fcos
			fmul qword[NOptCoordX1] ;st0 = cos(angle) * radius
			fadd qword[NOptCoordX0]
			fstp qword[edx]
			fld st0
			fsin
			fmul qword[NOptCoordX1] ;st0 = sin(angle) * radius
			fadd qword[NOptCoordY0]
			fstp qword[edx+8]
			loop .cycle0
		ffree st0
		fincstp
		ffree st0
		fincstp
		jmp .end0
	.rect:
		;rect
		mov eax,5
		mov dword[edx+Figure.PoiCount],eax
		imul eax,sizeof.Point
		stdcall mem.ReAlloc,[edx+Figure.PoiData],eax
		mov [edx+Figure.PoiData],eax
		;init points
		push edi
		mov ecx,[edx+Figure.PoiData]
		;p1
		add ecx,sizeof.Point
		lea edi,[ecx+Point.CoordX]
		mov esi,NOptCoordX1
		movsd
		movsd
		mov esi,NOptCoordY0
		movsd
		movsd
		;p2
		add ecx,sizeof.Point
		lea edi,[ecx+Point.CoordX]
		mov esi,NOptCoordX1
		movsd
		movsd
		mov esi,NOptCoordY1
		movsd
		movsd
		;p3
		add ecx,sizeof.Point
		lea edi,[ecx+Point.CoordX]
		mov esi,NOptCoordX0
		movsd
		movsd
		mov esi,NOptCoordY1
		movsd
		movsd
		;p4
		add ecx,sizeof.Point
		lea edi,[ecx+Point.CoordX]
		mov esi,NOptCoordX0
		movsd
		movsd
		mov esi,NOptCoordY0
		movsd
		movsd
		pop edi
	.end0:
	stdcall found_parent_obj,[edi] ;get figure number in ecx
	stdcall figure_update_coords,ObjData,ecx

	add edi,4
	mov esi,txt_05
	stdcall FigureInit,FigData,edi
	add edi,4
	mov esi,txt_06
	stdcall FigureInit,FigData,edi

	stdcall ObjCalculateScale,ebx
popad
	ret
endp

align 4
proc get_n_file_opt uses eax
	;загрузка координат в editbox-ы
	mov word[NumberSymbolsAD],3
	finit
	fld qword[NOptCoordX0]
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	call String_crop_0
	stdcall [edit_box_set_text], edit4, Data_String
	fld qword[NOptCoordY0]
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	call String_crop_0
	stdcall [edit_box_set_text], edit5, Data_String
	fld qword[NOptCoordX1]
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	call String_crop_0
	stdcall [edit_box_set_text], edit6, Data_String
	fld qword[NOptCoordY1]
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	call String_crop_0
	stdcall [edit_box_set_text], edit7, Data_String
	fld qword[NOptCoordZ0]
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	call String_crop_0
	stdcall [edit_box_set_text], edit8, Data_String
	fld qword[NOptCoordZ1]
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	call String_crop_0
	stdcall [edit_box_set_text], edit9, Data_String
	;
	;stdcall [edit_box_draw], edit4
	;stdcall [edit_box_draw], edit5
	ret
endp

align 4
proc timer_funct_n_file uses eax
	;просматриваем выделенную координату
	mov eax,[opt_gr1]
	cmp eax,[last_opt]
	je @f
		mov eax,[opt_gr1]
		mov [last_opt],eax
		call red_win_n_file
	@@:
	ret
endp

capt_coord_x0 db 'X0:',0
capt_coord_y0 db 'Y0:',0
capt_coord_z0 db 'Z0:',0
capt_coord_x1 db 'X1:',0
capt_coord_y1 db 'Y1:',0
capt_coord_z1 db 'Z1:',0
capt_coord_r  db 'R:',0
capt_coord_s  db 'S:',0

edit4 edit_box 80, 24, 28, 0xffd0d0, 0xff, 0x80ff, 0, 0x8000, 34, string4, mouse_dd, 0
edit5 edit_box 80, 24, 46, 0xd0d0ff, 0xff, 0x80ff, 0, 0x8000, 34, string5, mouse_dd, 0
edit6 edit_box 80, 24, 64, 0xffd0d0, 0xff, 0x80ff, 0, 0x8000, 34, string6, mouse_dd, 0
edit7 edit_box 80, 24, 82, 0xd0d0ff, 0xff, 0x80ff, 0, 0x8000, 34, string7, mouse_dd, 0
edit8 edit_box 80, 24, 100, 0xffd0d0, 0xff, 0x80ff, 0, 0x8000, 34, string8, mouse_dd, 0
edit9 edit_box 80, 24, 118, 0xd0d0ff, 0xff, 0x80ff, 0, 0x8000, 34, string9, mouse_dd, 0
editboxes_end_nf:

string4 rb 34
string5 rb 34
string6 rb 34
string7 rb 34
string8 rb 34
string9 rb 34

align 4
last_opt dd opt1 ;for timer
opt_gr1 dd opt1

opt1 option_box opt_gr1, 115,28,6,12,0xd0d0ff, 0xff, 0x80ff,txt_circle,txt_circle.end-txt_circle
opt2 option_box opt_gr1, 115,46,6,12,0xd0d0ff, 0xff, 0x80ff,txt_rect,txt_rect.end-txt_rect

align 4
opt_grlist1 dd opt1,opt2,0 ;end option group

txt_00 db ';PCB graving',0
txt_01: db 'G0 M3 X0 Y0 Z'
.z: rb 12
txt_02: db 'G0 X'
.x: rb 50 ;X__ Y__ Z__
txt_03: db 'G1 F100 Z'
.z: rb 12
txt_04: db 'X'
.x: rb 30 ;X__ Y__
txt_05: db 'G0 Z'
.z: rb 12
txt_06: db 'G0 M5 X0 Y0 Z'
.z: rb 12

if lang eq ru
capt_n_file db 'Создать новый файл',0
capt_sel_fig db 'Выберите фигуру:',0
txt_circle: db 'Окружность'
.end: db 0
txt_rect: db 'Прямоугольник'
.end: db 0
else
capt_n_file db 'Create new file',0
capt_sel_fig db 'Select figure:',0
txt_circle: db 'Circle'
.end: db 0
txt_rect: db 'Rectangle'
.end: db 0
end if