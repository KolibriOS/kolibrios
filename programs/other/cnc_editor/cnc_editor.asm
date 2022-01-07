use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,file_name,sys_path

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../load_img.inc'
include '../../load_lib.mac'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'lang.inc'
include 'cnc_editor.inc'
include '../../develop/info3ds/info_fun_float.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
caption db 'CNC editor 23.05.19',0 ;подпись окна

run_file_70 FileInfoBlock

offs_last_timer dd 0 ;последний сдвиг показаный в функции таймера

IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
image_data_toolbar dd 0 ;указатель на временную память. для нужен преобразования изображения
icon_tl_sys dd 0 ;указатель на память для хранения системных иконок
icon_toolbar dd 0 ;указатель на память для хранения иконок объектов

include 'wnd_point_coords.inc'
include 'wnd_scale.inc'
include 'wnd_new_file.inc'

align 4
start:
	;--- copy cmd line ---
	mov esi,file_name
	mov edi,openfile_path
@@:
	lodsd
	or eax,eax
	jz @f ;выход, если 0
	stosd
	jmp @b
@@:
	stosd

	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась библиотека
	mov	ebp,lib_0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0xC0000027
	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

	stdcall [buf2d_create], buf_0 ;создание буфера

	;шрифт делаем до создания панели (для экономии указателя image_data_toolbar)
	include_image_file '..\..\fs\kfar\trunk\font6x9.bmp', image_data_toolbar, buf_1.w,buf_1.h
	stdcall [buf2d_create_f_img], buf_1,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память
	stdcall [buf2d_conv_24_to_8], buf_1,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_1
	mov eax,[buf_1.h]
	shr eax,8
	mov [font_h],eax

	include_image_file 'toolbar.png', image_data_toolbar

	mov dword[w_scr_t1.type],1
	stdcall dword[tl_data_init], tree1
	;системные иконки 16*16 для tree_list
	include_image_file 'tl_sys_16.png', icon_tl_sys
	mov eax,dword[icon_tl_sys]
	mov dword[tree1.data_img_sys],eax

	include_image_file 'objects.png', icon_toolbar
	mov eax,dword[icon_toolbar]
	mov dword[tree1.data_img],eax

	;*** установка времени для таймера
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	;open file from cmd line
	cmp dword[openfile_path],0
	je @f
		call but_open_file.no_dlg
	@@:

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov ebx,[last_time]
	add ebx,10 ;задержка
	cmp ebx,eax
	jge @f
		mov ebx,eax
	@@:
	sub ebx,eax
	mcall SF_WAIT_EVENT_TIMEOUT
	cmp eax,0
	jne @f
		call timer_funct
		jmp still
	@@:

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jne @f
		mcall SF_THREAD_INFO,procinfo,-1
		cmp ax,word[procinfo.window_stack_position]
		jne @f ;окно не активно
		call mouse
	@@:
	jmp still

align 4
mouse:
	push eax ebx ecx
	mcall SF_MOUSE_GET,SSF_BUTTON_EXT
	bt eax,8
	jnc @f
		;mouse l. but. press
		call mouse_left_d
		jmp .end_l
	@@:
	;bt eax,0
	;jnc @f
		;mouse l. but. move
		;call mouse_left_m
		;jmp .end_l
	;@@:
	bt eax,16
	jnc .end_l
		;mouse l. but. up
		call mouse_left_u
		;jmp .end_l
	.end_l:
	bt eax,9
	jnc @f
		;mouse r. but. press
		call mouse_right_d
		;jmp .end_r
	@@:
	;bt eax,1
	;jnc @f
		;mouse r. but. move
		;call mouse_right_m
		;jmp .end_r
	;@@:
	;.end_r:

	call buf_get_mouse_coord
	cmp eax,-1
	je .end0
		shl eax,1
		sub eax,[buf_0.w]
		sar eax,1
		mov [mouse_prop_x],eax
		stdcall [tl_node_get_data],tree1
		or eax,eax
		jz .end0
		cmp [eax+Figure.OType],'Fig'
		je @f
		cmp [eax+Figure.OType],'Obj'
		je .end1
			jmp .end0
		.end1:
			mov eax,ObjData
		@@:
		mov ecx,eax
		shl ebx,1
		sub ebx,[buf_0.h]
		sar ebx,1
		mov [mouse_prop_y],ebx

		mcall SF_MOUSE_GET,SSF_SCROLL_DATA
		test ax,ax
		jz .end0
		finit
		fld qword[zoom_plus]
		fld1
		fsubp
		fld st0 ;for Y coord

		;for X coord
		fild dword[mouse_prop_x]
		fmulp st1,st0

		mov ebx,eax
		test ax,0x8000
		jnz .decr
			;увеличение масштаба
			fchs
			fild dword[ecx+Figure.MCentrX] ;add old value
			fmul qword[zoom_plus]
			faddp

			fld qword[ecx+Figure.MScale]
			fmul qword[zoom_plus]
			;if (Figure.MScale>16.0)
			;...
			jmp @f
		.decr:
			;уменьшение масштаба
			fild dword[ecx+Figure.MCentrX] ;add old value
			fdiv qword[zoom_plus]
			faddp

			fld qword[ecx+Figure.MScale]
			fdiv qword[zoom_plus]
			fld1
			fcomp
			fstsw ax
			sahf
			jbe @f
				;if (Figure.MScale<1.0)
				ffree st0
				fincstp
				ffree st0
				fincstp
				fldz ;default Figure.MCentrX
				fld1 ;default Figure.MScale
				mov dword[ecx+Figure.MCentrY],0
		@@:
		fstp qword[ecx+Figure.MScale]
		fistp dword[ecx+Figure.MCentrX]

		;for Y coord
		fild dword[mouse_prop_y]
		fmulp st1,st0
		test bx,0x8000
		jnz .decr_y
			;увеличение масштаба
			fild dword[ecx+Figure.MCentrY] ;add old value
			fmul qword[zoom_plus]
			faddp
			jmp @f
		.decr_y:
			;уменьшение масштаба
			fchs
			fild dword[ecx+Figure.MCentrY] ;add old value
			fdiv qword[zoom_plus]
			faddp
		@@:
		fistp dword[ecx+Figure.MCentrY]

		mov dword[offs_last_timer],0
	.end0:

	stdcall [tl_mouse], tree1
	pop ecx ebx eax
	ret

;output:
; eax - buffer coord X (если курсор за буфером -1)
; ebx - buffer coord Y (если курсор за буфером -1)
align 4
proc buf_get_mouse_coord
	mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
	cmp ax,word[buf_0.t]
	jl .no_buf ;не попали в окно буфера по оси y
	mov ebx,eax
	shr ebx,16
	cmp bx,word[buf_0.l]
	jl .no_buf ;не попали в окно буфера по оси x

	and eax,0xffff ;оставляем координату y
	sub ax,word[buf_0.t]
	cmp eax,[buf_0.h]
	jg .no_buf
	sub bx,word[buf_0.l]
	cmp ebx,[buf_0.w]
	jg .no_buf
	xchg eax,ebx
	jmp .end_f
	.no_buf:
		xor eax,eax
		not eax
		xor ebx,ebx
		not ebx
	.end_f:
	ret
endp

align 4
proc timer_funct
	pushad
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	;просматриваем выделенный блок данных
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_f
	cmp dword[offs_last_timer],eax
	je .end_f
		;если выделенный блок данных не совпадает с последним запомненным
		mov dword[offs_last_timer],eax

		stdcall get_object_data,eax
		or ecx,ecx
		jz .end_oblo
			stdcall draw_obj2d,ecx
			stdcall [buf2d_draw], buf_0
			jmp .end_f
		.end_oblo:

		cmp [eax+Figure.OType],'Fig'
		jne .end_fblo
			stdcall draw_fig2d,eax
			stdcall [buf2d_draw], buf_0
			jmp .end_f
		.end_fblo:

	.end_f:

	popad
	ret
endp

align 4
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	mov edx,[sc.work]
	or  edx,0x33000000
	mov edi,caption
	mcall SF_CREATE_WINDOW,(20 shl 16)+599,(20 shl 16)+415

	mcall SF_THREAD_INFO,procinfo,-1
	mov eax,dword[procinfo.box.height]
	cmp eax,120
	jge @f
		mov eax,120 ;min size
	@@:
	sub eax,65
	mov dword[tree1.box_height],eax
	mov word[w_scr_t1.y_size],ax ;новые размеры скроллинга
	mov ebx,dword[procinfo.box.width]
	cmp ebx,270
	jge @f
		mov ebx,270
	@@:
	sub ebx,215
	cmp eax,dword[buf_0.h] ;смотрим размер буфера
	jne @f
	cmp ebx,dword[buf_0.w]
	jne @f
		jmp .end0
	@@:
		stdcall [buf2d_resize],buf_0,ebx,eax,1
		mov eax,ObjData
		mov ecx,[eax+Object.FigCount]
		or ecx,ecx
		jz .end0
		mov eax,[eax+Object.FigData]
		xor edx,edx
		.cycle0:
			stdcall FigCalculateSizes,[eax+4*edx],0
			inc edx
			loop .cycle0
		stdcall ObjCalculateScale,ObjData
		mov dword[offs_last_timer],0
		call timer_funct
	.end0:

	; *** создание кнопок на панель ***
	mcall SF_DEFINE_BUTTON,(5 shl 16)+20,(5 shl 16)+20,3, [sc.work_button]

	add ebx,25 shl 16
	mcall ,,,4 ;open
	add ebx,25 shl 16
	mcall ,,,5 ;save

	add ebx,30 shl 16
	mcall ,,,6 ;captions on off

	add ebx,25 shl 16
	mcall ,,,7 ;figure move up
	add ebx,25 shl 16
	mcall ,,,8 ;figure move down

	add ebx,25 shl 16
	mcall ,,,9 ;sel points dlg

	add ebx,25 shl 16
	mcall ,,,10 ;sel points move up
	add ebx,25 shl 16
	mcall ,,,11 ;sel points move down

	add ebx,25 shl 16
	mcall ,,,12 ;align sel points left
	add ebx,25 shl 16
	mcall ,,,13 ;align sel points right
	add ebx,25 shl 16
	mcall ,,,14 ;align sel points top
	add ebx,25 shl 16
	mcall ,,,15 ;align sel points bottom
	add ebx,25 shl 16
	mcall ,,,16 ;optimize figure

	add ebx,30 shl 16
	mcall ,,,17 ;copy to clipboard
	add ebx,25 shl 16
	mcall ,,,18 ;paste from clipboard

	add ebx,25 shl 16
	mcall ,,,19 ;sel points del

	add ebx,30 shl 16
	mcall ,,,20 ;restore zoom

	add ebx,30 shl 16
	mcall ,,,21 ;.png

	add ebx,25 shl 16
	mcall ,,,22 ;options scale

	; *** рисование иконок на кнопках ***
	mcall SF_PUT_IMAGE,[image_data_toolbar],(16 shl 16)+16,(7 shl 16)+7 ;icon new

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon open
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon save
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;captions on off
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;figure move up
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;figure move down
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;sel points dlg
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;sel points move up
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;sel points move down
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;aling sel points left
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;aling sel points right
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;aling sel points top
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;aling sel points bottom
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;optimize figure
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;copy to clipboard
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;paste from clipboard
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;sel points del
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;restore zoom
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;.png
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;options scale
	int 0x40

	mov dword[w_scr_t1.all_redraw],1
	stdcall [tl_draw], tree1
	stdcall [buf2d_draw], buf_0

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
key:
	mcall SF_GET_KEY
	cmp dword[el_focus], tree1
	jne @f
		stdcall [tl_key], tree1
		jmp .end0
	@@:
	
	cmp ah,178 ;Up
	jne @f
		call but_selection_move_up
		jmp .end0
	@@:
	cmp ah,177 ;Down
	jne @f
		call but_selection_move_down
		jmp .end0
	@@:
	cmp ah,176 ;Left
	jne @f
		call but_selection_move_up
		jmp .end0
	@@:
	cmp ah,179 ;Right
	jne @f
		call but_selection_move_down
		jmp .end0
	@@:
	cmp ah,182 ;Delete
	jne @f
		call but_sel_points_del
		jmp .end0
	@@:

	mov ecx,eax
	mcall SF_KEYBOARD,SSF_GET_CONTROL_KEYS
	bt eax,2 ;left Ctrl
	jc .key_Ctrl
	bt eax,3 ;right Ctrl
	jc .key_Ctrl
	jmp .end0
	.key_Ctrl:
		cmp ch,15 ;111 ;Ctrl+O
		jne @f
			call but_open_file
		@@:
		cmp ch,19 ;115 ;Ctrl+S
		jne @f
			call but_save_file
		@@:
		cmp ch,14 ;110 ;Ctrl+N
		jne @f
			call but_new_file
		@@:
		cmp ch,1 ;97 ;Ctrl+A
		jne @f
			call but_sel_points_all
		@@:
	.end0:
	jmp still


align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,3
	jne @f
		call but_new_file
		jmp still
	@@:
	cmp ah,4
	jne @f
		call but_open_file
		jmp still
	@@:
	cmp ah,5
	jne @f
		call but_save_file
		jmp still
	@@:
	cmp ah,6
	jne @f
		call but_captions_on_off
		jmp still
	@@:
	cmp ah,7
	jne @f
		call but_sel_figure_move_up
		jmp still
	@@:
	cmp ah,8
	jne @f
		call but_sel_figure_move_down
		jmp still
	@@:
	cmp ah,9
	jne @f
		call but_dlg_point_coords
		jmp still
	@@:
	cmp ah,10
	jne @f
		call but_sel_points_move_up
		jmp still
	@@:
	cmp ah,11
	jne @f
		call but_sel_points_move_down
		jmp still
	@@:
	cmp ah,12
	jne @f
		call but_sel_points_align_coord_xmin
		jmp still
	@@:
	cmp ah,13
	jne @f
		call but_sel_points_align_coord_xmax
		jmp still
	@@:
	cmp ah,14
	jne @f
		call but_sel_points_align_coord_ymax
		jmp still
	@@:
	cmp ah,15
	jne @f
		call but_sel_points_align_coord_ymin
	@@:
	cmp ah,16
	jne @f
		call but_points_optimize
		jmp still
	@@:
	cmp ah,17
	jne @f
		call but_clipboard_copy_points
		jmp still
	@@:
	cmp ah,18
	jne @f
		call but_clipboard_paste_points
		jmp still
	@@:
	cmp ah,19
	jne @f
		call but_sel_points_del
		jmp still
	@@:
	cmp ah,20
	jne @f
		call but_restore_zoom
		jmp still
	@@:
	cmp ah,21
	jne @f
		call but_save_png
		jmp still
	@@:
	cmp ah,22
	jne @f
		call but_dlg_opt_scale
		jmp still
	@@:

	;cmp ah,23
	;jne @f
		;call but_...
		;jmp still
	;@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_data]
	mcall SF_TERMINATE_PROCESS


align 4
but_new_file:
	cmp byte[wnd_n_file],0
	jne .end_f
pushad
	mcall SF_CREATE_THREAD,1,start_n_file,thread_n_file
popad
	.end_f:
	ret

align 4
open_file_data dd 0 ;указатель на память для открытия файлов
open_file_size dd 0 ;размер открытого файла

align 4
but_open_file:
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	pushad
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;код при удачном открытии диалога
	jmp .end0
.no_dlg: ;если минуем диалог открытия файла
		pushad
		mov esi,openfile_path
		stdcall str_len,esi
		add esi,eax
		@@: ;цикл для поиска начала имени файла
			dec esi
			cmp byte[esi],'/'
			je @f
			cmp byte[esi],0x5c ;'\'
			je @f
			cmp esi,openfile_path
			jg @b
		@@:
		inc esi
		stdcall [OpenDialog_Set_file_name],OpenDialog_data,esi ;копируем имя файла в диалог сохранения
	.end0:

	mov [run_file_70.Function], SSF_GET_INFO
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], 0
	mov dword[run_file_70.Buffer], open_b
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70

	mov ecx,dword[open_b+32] ;+32 qword: размер файла в байтах
	inc ecx ;for text files
	stdcall mem.ReAlloc,[open_file_data],ecx
	mov [open_file_data],eax
	dec ecx ;for text files
	mov byte[eax+ecx],0 ;for text files

	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], ecx
	m2m dword[run_file_70.Buffer], dword[open_file_data]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70 ;загружаем файл изображения
	test eax,eax
	jnz .end_open_file
	cmp ebx,0xffffffff
	je .end_open_file

		mov [open_file_size],ebx
		mcall SF_SET_CAPTION,1,openfile_path

		;---
		and dword[tree1.style],not tl_cursor_pos_limited
		stdcall FileInit,[open_file_data],[open_file_size]
		or dword[tree1.style], tl_cursor_pos_limited
		stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
		stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	.end_open_file:
	popad
	ret

align 4
proc but_save_file
locals
	pCou dd ?
	pData dd ?
endl
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Set_file_ext],OpenDialog_data,Filter.1 ;.nc
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_save_file
	;код при удачном открытии диалога

	;*** определение примерного размера файла
	call get_file_save_size
	stdcall mem.ReAlloc,[open_file_data],ecx
	mov [open_file_data],eax
	mov [open_file_size],ecx

	;clear memory
	mov edi,eax
	xor eax,eax
	shr ecx,2
	rep stosd
	
	;*** пишем информацию в память
	mov edi,[open_file_data]
	mov ebx,ObjData
	mov edx,[ebx+Object.FigCount]
	or edx,edx
	jz .cycle1end
	mov esi,[ebx+Object.FigData]
align 4
	.cycle1: ;цикл по фигурам
		;param 1
		mov eax,[esi]
		or eax,eax
		jz .err_save
		mov eax,[eax+Figure.PoiCount]
		mov [pCou],eax
		or eax,eax
		jnz @f
			mov eax,[esi]
			lea eax,[eax+Figure.Caption]
			stdcall str_cat,edi,eax
			stdcall str_len,edi
			add edi,eax
			stdcall str_cat,edi,txt_nl
		@@:
		cmp dword[pCou],0
		je .cycle2end
		mov eax,[esi]
		mov eax,[eax+Figure.PoiData]
		mov [pData],eax
		mov word[NumberSymbolsAD],8
align 4
		.cycle2: ;цикл по точкам
			stdcall str_cat,edi,txt_s_poi
			;param 1
			push edi esi
			mov esi,[pData]
			lea esi,[esi+Point.CoordX]
			mov edi,Data_Double
			movsd
			movsd
			pop esi edi
			call DoubleFloat_to_String
			call String_crop_0
			stdcall str_cat,edi,Data_String
			stdcall str_cat,edi,txt_s_poi_Y
			;param 2
			push edi esi
			mov esi,[pData]
			lea esi,[esi+Point.CoordY]
			mov edi,Data_Double
			movsd
			movsd
			pop esi edi
			call DoubleFloat_to_String
			call String_crop_0
			stdcall str_cat,edi,Data_String
			;
			stdcall str_cat,edi,txt_nl
			add dword[pData],sizeof.Point
			dec dword[pCou]
			jnz .cycle2
		.cycle2end:
		add esi,4
		dec edx
		jnz .cycle1
	.cycle1end:
	
	jmp @f
	.err_save:
		notify_window_run txt_err_save_txt_file_1
	@@:

	;*** определение параметров файла
	mov edi,[open_file_data]
	stdcall str_len,edi
	;;cmp eax,[open_file_size]
	mov [run_file_70.Count],eax ;размер файла

	;*** сохраняем файл
	mov [run_file_70.Function], SSF_CREATE_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov ebx, dword[open_file_data]
	mov [run_file_70.Buffer], ebx
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70
	or eax,eax
	jz .end_save_file
	or ebx,ebx
	jnz .end_save_file
		;сообщение о неудачном сохранении
		notify_window_run txt_err_save_txt_file_0
	.end_save_file:
	popad
	ret
endp

;output:
; ecx - memory size for save file
align 4
proc get_file_save_size uses eax ebx edx esi
	mov ecx,100 ;title
	mov ebx,ObjData
	add ecx,50 ;object
	mov edx,[ebx+Object.FigCount]
	or edx,edx
	jz .cycle1end
	mov esi,[ebx+Object.FigData]
align 4
	.cycle1: ; цикл по фигурам
		add ecx,80 ;figure
		mov eax,[esi]
		or eax,eax
		jz @f
			mov eax,[eax+Figure.PoiCount]
			imul eax,70
			add ecx,eax ;points
		@@:
		add esi,4
		dec edx
		jnz .cycle1
	.cycle1end:
	ret
endp

align 4
but_captions_on_off:
	xor dword[opt_draw],1
	mov dword[offs_last_timer],0 ;для обновления по таймеру
	ret

align 4
but_sel_figure_move_up:
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_fblo
	cmp [eax+Figure.OType],'Fig'
	jne .end_fblo
		stdcall found_parent_obj,eax
		or eax,eax
		jz .end_f ;if not found
		or ecx,ecx
		jz .fig_is_0 ;если фигура в начале списка
		mov ebx,[eax+Object.FigData]
		mov edx,[ebx+4*ecx] ;передвигаемая фигура
		mov edi,[ebx+4*ecx-4]
		;меняем фигуры местами
		mov [ebx+4*ecx],edi
		mov [ebx+4*ecx-4],edx
		;меняем фигуры в списке
		stdcall [tl_node_move_up],tree1
		jmp .end_f
	.fig_is_0:
		notify_window_run txt_err_figure_is_0
		jmp .end_f
	.end_fblo:
		notify_window_run txt_err_no_figure_select
	.end_f:
	popad
	ret

align 4
but_save_png:
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_save_file

	stdcall get_object_data,eax
	or ecx,ecx
	jz .end_oblo
		stdcall draw_obj2d_png,ecx
		jmp .beg0
	.end_oblo:

;       cmp [eax+Figure.OType],'Fig'
;       jne .end_fblo
;               stdcall draw_fig2d_png,eax
;               jmp .beg0
;       .end_fblo:

	jmp .end_save_file
	.beg0:
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Set_file_ext],OpenDialog_data,Filter.2 ;.png
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],1
	jne .end_save_file
		;код при удачном открытии диалога
		mov dword[png_data],0

		;create image struct
		stdcall [img_create], [buf_png.w], [buf_png.h], Image.bpp24
		mov ebx,eax
		test eax,eax
		jz @f
			;copy foto to image buffer
			mov edi,[eax+Image.Data]
			mov esi,[buf_png]
			mov ecx,[buf_png.w]
			mov edx,[buf_png.h]
			imul ecx,edx
			imul ecx,3
			shr ecx,2 ;OpenGL buffer align to 4
			rep movsd

			;encode image
			stdcall [img_encode], eax, LIBIMG_FORMAT_PNG, 0
			test eax,eax
			jz @f
				mov [png_data],eax
				mov [png_size],ecx
		@@:
		stdcall [img_destroy],ebx

	; заполняем структуру для сохранения файла
	mov ebx,run_file_70
	mov dword[ebx],SSF_CREATE_FILE
	mov eax,[png_size]
	mov [ebx+12],eax ;file size
	mov eax,[png_data]
	mov [ebx+16],eax
	mov dword[ebx+FileInfoBlock.FileName], openfile_path

	mcall SF_FILE,run_file_70
	test eax,eax
	jnz .save_error
			;notify_window_run openfile_path
			jmp @f
		.save_error:
			;сообщение о неудачном сохранении
			notify_window_run txt_err_save_img_file
		@@:
		mcall SF_SYS_MISC, SSF_MEM_FREE, [png_data]
	.end_save_file:
	popad
	ret

;description:
; выделенные точки сдвигаются вверх
align 4
proc but_sel_points_move_up
locals
	pObj dd ?
	pFig dd ?
	idFig dd ?
endl
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov [pFig],eax
	stdcall sel_points_get_count,eax
	or eax,eax
	jz .no_point

	stdcall found_parent_obj,[pFig]
	or eax,eax
	jz .end_f ;if not found
	mov [pObj],eax
	mov [idFig],ecx

	;проверяем выделенные точки
	mov eax,[pFig]
	mov ecx,[eax+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	mov ebx,[eax+Figure.PoiData]
	xor eax,eax
	.cycle0:
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			or eax,eax
			jz .point_is_0
			stdcall point_move_up, [pObj],[pFig],[idFig],ebx;,eax
		@@:
		inc eax
		add ebx,sizeof.Point
		loop .cycle0

		;для обновления по таймеру
		mov dword[offs_last_timer],0
		jmp .end_f
	.point_is_0:
		notify_window_run txt_err_poi_is_0
		jmp .end_f
	.no_point:
		notify_window_run txt_err_no_point_sel
	.end_f:
	popad
	ret
endp

;description:
; выделенные точки сдвигаются вниз
align 4
proc but_sel_points_move_down
locals
	pObj dd ?
	pFig dd ?
	idFig dd ?
endl
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov [pFig],eax
	stdcall sel_points_get_count,eax
	or eax,eax
	jz .no_point

	stdcall found_parent_obj,[pFig]
	or eax,eax
	jz .end_f ;if not found
	mov [pObj],eax
	mov [idFig],ecx

	;проверяем выделенные точки
	mov eax,[pFig]
	mov ecx,[eax+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	mov ebx,ecx
	dec ebx
	imul ebx,sizeof.Point
	add ebx,[eax+Figure.PoiData]
	.cycle0:
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			cmp ecx,[eax+Figure.PoiCount]
			je .point_is_last
			mov edi,ebx
			add edi,sizeof.Point
			stdcall point_move_up, [pObj],[pFig],[idFig],edi;,ecx
		@@:
		sub ebx,sizeof.Point
		loop .cycle0

		;для обновления по таймеру
		mov dword[offs_last_timer],0
		jmp .end_f
	.point_is_last:
		notify_window_run txt_err_poi_is_last
		jmp .end_f
	.no_point:
		notify_window_run txt_err_no_point_sel
	.end_f:
	popad
	ret
endp

;description:
; выделение выбранной точки сдвигается вверх
align 4
proc but_selection_move_up
locals
	pFig dd ?
endl
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_fblo
	cmp [eax+Figure.OType],'Fig'
	jne .end_fblo

	mov [pFig],eax
	stdcall sel_points_get_count,eax
	cmp eax,1
	jne .no_point

	;проверяем выделенные точки
	mov eax,[pFig]
	mov ecx,[eax+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	dec ecx
	jecxz .end_f
	mov ebx,[eax+Figure.PoiData]
	.cycle0:
		add ebx,sizeof.Point
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			btr dword[ebx+Point.Prop],PROP_BIT_SELECT
			bts dword[ebx+Point.Prop-sizeof.Point],PROP_BIT_SELECT
			xor ecx,ecx
			inc ecx ;ecx = 1 - for exit from cycle
		@@:
		loop .cycle0

		;для обновления по таймеру
		mov dword[offs_last_timer],0
		jmp .end_f
	.no_point:
		notify_window_run txt_err_no_point_sel_1
		jmp .end_f
	.end_fblo:
		;notify_window_run txt_err_no_figure_select
	.end_f:
	popad
	ret
endp

;description:
; выделение выбранной точки сдвигается вниз
align 4
proc but_selection_move_down
locals
	pFig dd ?
endl
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_fblo
	cmp [eax+Figure.OType],'Fig'
	jne .end_fblo

	mov [pFig],eax
	stdcall sel_points_get_count,eax
	cmp eax,1
	jne .no_point

	;проверяем выделенные точки
	mov eax,[pFig]
	mov ecx,[eax+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	dec ecx
	jecxz .end_f
	mov ebx,[eax+Figure.PoiData]
	.cycle0:
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			btr dword[ebx+Point.Prop],PROP_BIT_SELECT
			bts dword[ebx+Point.Prop+sizeof.Point],PROP_BIT_SELECT
			xor ecx,ecx
			inc ecx ;ecx = 1 - for exit from cycle
		@@:
		add ebx,sizeof.Point
		loop .cycle0

		;для обновления по таймеру
		mov dword[offs_last_timer],0
		jmp .end_f
	.no_point:
		notify_window_run txt_err_no_point_sel_1
		jmp .end_f
	.end_fblo:
		;notify_window_run txt_err_no_figure_select
	.end_f:
	popad
	ret
endp

;output:
; eax - couunt new points
align 4
proc but_clipboard_copy_points uses ebx ecx edx edi esi
locals
	pData dd 0
	pBuf dd 0
endl
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov ecx,[eax+Figure.PoiData]
	mov [pData],ecx
	mov ecx,[eax+Figure.PoiCount]
	stdcall sel_points_get_count,eax
	or eax,eax
	jz .no_point
	push eax
		imul eax,32 ;for string 'X___.________ Y___.________^^'
		stdcall mem.Alloc,eax
		mov [pBuf],eax

		mov edx,eax
		mov dword[edx+4],0 ;text data
		mov dword[edx+8],1 ;code 866
		add edx,12 ;system buffer header size
		mov dword[edx],0
		mov word[NumberSymbolsAD],8
align 4
		.cycle2: ;цикл по точкам
			mov esi,[pData]
			bt dword[esi+Point.Prop],PROP_BIT_SELECT
			jnc .end0
			stdcall str_cat,edx,txt_s_poi
			;param 1
			add esi,Point.CoordX
			mov edi,Data_Double
			movsd
			movsd
			call DoubleFloat_to_String
			call String_crop_0
			stdcall str_cat,edx,Data_String
			stdcall str_cat,edx,txt_s_poi_Y
			;param 2
			;;mov esi,[pData]
			;;lea esi,[esi+Point.CoordY]
			mov edi,Data_Double
			movsd
			movsd
			call DoubleFloat_to_String
			call String_crop_0
			stdcall str_cat,edx,Data_String
			;
			stdcall str_cat,edx,txt_nl
			.end0:
			add dword[pData],sizeof.Point
			loop .cycle2
		.cycle2end:

		mov edx,[pBuf]
		add edx,12
		stdcall str_len,edx
		sub edx,12
		mov [edx],eax
		add eax,12
		mov ecx,eax
		mcall SF_CLIPBOARD,SSF_WRITE_CB ;,ecx,edx
		stdcall mem.Free,[pBuf]
	pop eax

	.no_point:
	ret
endp

;output:
; eax - couunt new points
align 4
proc but_clipboard_paste_points uses ebx ecx edx esi edi
locals
	pFig dd ?
	nCount dd ?
endl
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end0
	cmp [eax+Figure.OType],'Fig'
	jne .end0

	mov [pFig],eax

	mcall SF_CLIPBOARD,SSF_GET_SLOT_COUNT
	cmp eax,1
	jl .end0

	mov esi,eax
	.cycle0: ;обратный цикл по слотам
	dec esi ;номер текущего, проверяемого слота
	mcall SF_CLIPBOARD,SSF_READ_CB,esi
	cmp eax,1
	je .end0
	cmp eax,-1
	je .end0
		mov ecx,dword[eax]
		cmp ecx,5 ;min text size
		jl .end0
		cmp dword[eax+4],0 ;text
		je @f
			cmp esi,1
			jge .cycle0 ;если в буфере не текст, а слотов в буфере несколько, пробуем перейти к верхнему слоту
			jmp .end0
		@@:

	mov esi,eax
	add esi,12 ;смещение начала текста в буфере
	stdcall get_max_points
	or eax,eax
	jz .end_f
		mov [nCount],eax
		mov ebx,[pFig]
		mov edi,[ebx+Figure.PoiCount]
		add [ebx+Figure.PoiCount],eax
		add eax,edi
		imul eax,sizeof.Point
		stdcall mem.ReAlloc,[ebx+Figure.PoiData],eax
		mov [ebx+Figure.PoiData],eax

		mov ecx,edi
		mov edx,eax
align 4
		.cycle1: ;цикл для снятия выделения со старых точек
			btr dword[edx+Point.Prop],PROP_BIT_SELECT
			add edx,sizeof.Point
			loop .cycle1

		mov ecx,[nCount]
		imul edi,sizeof.Point
		add edi,eax
		finit
align 4
		.cycle2: ;цикл для добавления новых точек
			stdcall PointInit,edi
			or eax,eax
			jz .cycle2end
			bts dword[edi+Point.Prop],PROP_BIT_SELECT
			add edi,sizeof.Point
			loop .cycle2
		.cycle2end:
		or ecx,ecx
		jz .end1
			;уменьшаем объем памяти выделенный для точек
			sub [ebx+Figure.PoiCount],ecx
			mov eax,[ebx+Figure.PoiCount]
			imul eax,sizeof.Point
			stdcall mem.ReAlloc,[ebx+Figure.PoiData],eax
			mov [ebx+Figure.PoiData],eax
		.end1:
		stdcall found_parent_obj,ebx ;get figure number in ecx
		stdcall figure_update_coords,ObjData,ecx
		
		mov eax,[nCount]
		mov dword[offs_last_timer],0
		jmp .end_f
	.end0:
		xor eax,eax
	.end_f:
	ret
endp

align 4
proc but_sel_points_del
locals
	pFig dd ?
	nDel dd ? ;need delete
endl
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov [pFig],eax
	stdcall sel_points_get_count,eax
	or eax,eax
	jz .no_point
	mov [nDel],eax

	;проверяем выделенные точки
	mov ebx,[pFig]
	mov ecx,[ebx+Figure.PoiCount]
	or ecx,ecx
	jz .no_point

	cmp ecx,eax
	jle .no_all

	stdcall found_parent_obj,[pFig]
	or eax,eax
	jz .end_f ;if not found
		mov edi,[ebx+Figure.PoiData]
		mov edx,[ebx+Figure.PoiCount]
		imul edx,sizeof.Point
		add edx,edi

		.cycle0: ;1-я выделенная точка
			cmp edi,edx
			jge .end0
			bt dword[edi+Point.Prop],PROP_BIT_SELECT
			jc .cycle0end
				add edi,sizeof.Point
				jmp .cycle0
		.cycle0end:

		mov esi,edi ;следущая 1-я не выделенная точка
		add esi,sizeof.Point
		.cycle1:
			cmp esi,edx
			jge .end0
			bt dword[esi+Point.Prop],PROP_BIT_SELECT
			jnc .cycle1end
				add esi,sizeof.Point
				jmp .cycle1
		.cycle1end:

		mov ecx,sizeof.Point
		rep movsb ;замена выделенной точки
		bts dword[esi+Point.Prop-sizeof.Point],PROP_BIT_SELECT ;ставим выделение на точку, что-бы потом ее удалить

		jmp .cycle0
		.end0:

		;обновление памяти
		mov eax,[ebx+Figure.PoiCount]
		sub eax,[nDel]
		mov [ebx+Figure.PoiCount],eax
		imul eax,sizeof.Point
		stdcall mem.ReAlloc,[ebx+Figure.PoiData],eax
		mov [ebx+Figure.PoiData],eax

		;перерасчет размеров фигуры
		stdcall found_parent_obj,ebx ;get figure number in ecx
		stdcall figure_update_coords,ObjData,ecx

		;для обновления по таймеру
		mov dword[offs_last_timer],0
		jmp .end_f

	.no_all:
		notify_window_run txt_err_no_point_del
		jmp .end_f
	.no_point:
		notify_window_run txt_err_no_point_sel
	.end_f:
	popad
	ret
endp

align 4
proc but_sel_points_all uses eax ecx
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov ecx,[eax+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	mov eax,[eax+Figure.PoiData]
	.cycle0: ;цикл для выделенния точек
		bts dword[eax+Point.Prop],PROP_BIT_SELECT
		add eax,sizeof.Point
		loop .cycle0
	;для обновления по таймеру
	mov dword[offs_last_timer],0
	.no_point:
	ret
endp

align 4
proc but_sel_points_align_coord_xmin
pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov ebx,eax
	stdcall sel_points_get_count,eax
	cmp eax,1
	jle .no_point
	mov ecx,[ebx+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	cmp eax,ecx
	je .no_point ;если выделенны все точки, что-бы не портить контур

	mov edx,ecx
	imul edx,sizeof.Point
	mov ebx,[ebx+Figure.PoiData]
	add edx,ebx
align 4
	.cycle0: ;1-я выделенная точка
		cmp ebx,edx
		jge .no_point
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jc .cycle0end
		add ebx,sizeof.Point
		jmp .cycle0
	.cycle0end:
	lea esi,[ebx+Point.CoordX]
	mov edi,Data_Double
	movsd
	movsd
	finit
	fld qword[Data_Double]
align 4
	.cycle1: ;цикл для нахождения min(Point.CoordX)
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc .no_sel
			fcom qword[ebx+Point.CoordX]
			fstsw ax
			sahf
			jbe .no_sel
				;if (st0>Point.CoordX)
				ffree st0
				fincstp
				fld qword[ebx+Point.CoordX]
		.no_sel:
		add ebx,sizeof.Point
		cmp ebx,edx
		jl .cycle1
	fstp qword[Data_Double]
align 4
	.cycle2: ;цикл для присваивания всем Point.CoordX = min(Point.CoordX)
		sub edx,sizeof.Point
		bt dword[edx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			mov esi,Data_Double
			lea edi,[edx+Point.CoordX]
			movsd
			movsd
		@@:
		loop .cycle2
	;для обновления по таймеру
	mov dword[offs_last_timer],0
	.no_point:
popad
	ret
endp

align 4
proc but_sel_points_align_coord_ymin
pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov ebx,eax
	stdcall sel_points_get_count,eax
	cmp eax,1
	jle .no_point
	mov ecx,[ebx+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	cmp eax,ecx
	je .no_point ;если выделенны все точки, что-бы не портить контур

	mov edx,ecx
	imul edx,sizeof.Point
	mov ebx,[ebx+Figure.PoiData]
	add edx,ebx
align 4
	.cycle0: ;1-я выделенная точка
		cmp ebx,edx
		jge .no_point
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jc .cycle0end
		add ebx,sizeof.Point
		jmp .cycle0
	.cycle0end:
	lea esi,[ebx+Point.CoordY]
	mov edi,Data_Double
	movsd
	movsd
	finit
	fld qword[Data_Double]
align 4
	.cycle1: ;цикл для нахождения min(Point.CoordY)
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc .no_sel
			fcom qword[ebx+Point.CoordY]
			fstsw ax
			sahf
			jbe .no_sel
				;if (st0>Point.CoordY)
				ffree st0
				fincstp
				fld qword[ebx+Point.CoordY]
		.no_sel:
		add ebx,sizeof.Point
		cmp ebx,edx
		jl .cycle1
	fstp qword[Data_Double]
align 4
	.cycle2: ;цикл для присваивания всем Point.CoordY = min(Point.CoordY)
		sub edx,sizeof.Point
		bt dword[edx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			mov esi,Data_Double
			lea edi,[edx+Point.CoordY]
			movsd
			movsd
		@@:
		loop .cycle2
	;для обновления по таймеру
	mov dword[offs_last_timer],0
	.no_point:
popad
	ret
endp

align 4
proc but_sel_points_align_coord_xmax
pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov ebx,eax
	stdcall sel_points_get_count,eax
	cmp eax,1
	jle .no_point
	mov ecx,[ebx+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	cmp eax,ecx
	je .no_point ;если выделенны все точки, что-бы не портить контур

	mov edx,ecx
	imul edx,sizeof.Point
	mov ebx,[ebx+Figure.PoiData]
	add edx,ebx
align 4
	.cycle0: ;1-я выделенная точка
		cmp ebx,edx
		jge .no_point
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jc .cycle0end
		add ebx,sizeof.Point
		jmp .cycle0
	.cycle0end:
	lea esi,[ebx+Point.CoordX]
	mov edi,Data_Double
	movsd
	movsd
	finit
	fld qword[Data_Double]
align 4
	.cycle1: ;цикл для нахождения max(Point.CoordX)
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc .no_sel
			fcom qword[ebx+Point.CoordX]
			fstsw ax
			sahf
			jae .no_sel
				;if (st0<Point.CoordX)
				ffree st0
				fincstp
				fld qword[ebx+Point.CoordX]
		.no_sel:
		add ebx,sizeof.Point
		cmp ebx,edx
		jl .cycle1
	fstp qword[Data_Double]
align 4
	.cycle2: ;цикл для присваивания всем Point.CoordX = max(Point.CoordX)
		sub edx,sizeof.Point
		bt dword[edx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			mov esi,Data_Double
			lea edi,[edx+Point.CoordX]
			movsd
			movsd
		@@:
		loop .cycle2
	;для обновления по таймеру
	mov dword[offs_last_timer],0
	.no_point:
popad
	ret
endp

align 4
proc but_sel_points_align_coord_ymax
pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point

	mov ebx,eax
	stdcall sel_points_get_count,eax
	cmp eax,1
	jle .no_point
	mov ecx,[ebx+Figure.PoiCount]
	or ecx,ecx
	jz .no_point
	cmp eax,ecx
	je .no_point ;если выделенны все точки, что-бы не портить контур

	mov edx,ecx
	imul edx,sizeof.Point
	mov ebx,[ebx+Figure.PoiData]
	add edx,ebx
align 4
	.cycle0: ;1-я выделенная точка
		cmp ebx,edx
		jge .no_point
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jc .cycle0end
		add ebx,sizeof.Point
		jmp .cycle0
	.cycle0end:
	lea esi,[ebx+Point.CoordY]
	mov edi,Data_Double
	movsd
	movsd
	finit
	fld qword[Data_Double]
align 4
	.cycle1: ;цикл для нахождения max(Point.CoordY)
		bt dword[ebx+Point.Prop],PROP_BIT_SELECT
		jnc .no_sel
			fcom qword[ebx+Point.CoordY]
			fstsw ax
			sahf
			jae .no_sel
				;if (st0<Point.CoordY)
				ffree st0
				fincstp
				fld qword[ebx+Point.CoordY]
		.no_sel:
		add ebx,sizeof.Point
		cmp ebx,edx
		jl .cycle1
	fstp qword[Data_Double]
align 4
	.cycle2: ;цикл для присваивания всем Point.CoordY = max(Point.CoordY)
		sub edx,sizeof.Point
		bt dword[edx+Point.Prop],PROP_BIT_SELECT
		jnc @f
			mov esi,Data_Double
			lea edi,[edx+Point.CoordY]
			movsd
			movsd
		@@:
		loop .cycle2
	;для обновления по таймеру
	mov dword[offs_last_timer],0
	.no_point:
popad
	ret
endp

;description:
; оптимизация фигуры
align 4
proc but_points_optimize uses eax
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .no_point
	cmp [eax+Figure.OType],'Fig'
	jne .no_point
		stdcall points_optimize,eax
		mov dword[offs_last_timer],0 ;для обновления по таймеру
	.no_point:
	ret
endp

align 4
proc but_restore_zoom uses eax
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_f
	cmp [eax+Figure.OType],'Fig'
	je @f
	cmp [eax+Figure.OType],'Obj'
	je .end0
		jmp .end_f
	.end0:
		mov eax,ObjData
	@@:
		finit
		fld1
		fstp qword[eax+Figure.MScale]
		mov dword[eax+Figure.MCentrX],0
		mov dword[eax+Figure.MCentrY],0
		mov dword[offs_last_timer],0
	.end_f:
	ret
endp

align 4
but_sel_figure_move_down:
	pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_fblo
	cmp [eax+Figure.OType],'Fig'
	jne .end_fblo
		stdcall found_parent_obj,eax
		or eax,eax
		jz .end_f ;if not found
		inc ecx
		cmp ecx,[eax+Object.FigCount]
		jge .fig_is_last ;если фигура в конце списка
		mov ebx,[eax+Object.FigData]
		mov edx,[ebx+4*ecx] ;передвигаемая фигура
		mov edi,[ebx+4*ecx-4]
		;меняем фигуры местами
		mov [ebx+4*ecx],edi
		mov [ebx+4*ecx-4],edx
		;меняем фигуры в списке
		stdcall [tl_node_move_down],tree1
		jmp .end_f
	.fig_is_last:
		notify_window_run txt_err_figure_is_last
		jmp .end_f
	.end_fblo:
		notify_window_run txt_err_no_figure_select
	.end_f:
	popad
	ret

;description:
; запуск окна создания/редактирования точки
align 4
but_dlg_point_coords:
	cmp byte[wnd_run_prop],0
	jne .end_f
pushad
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_fblo
	cmp [eax+Figure.OType],'Fig'
	jne .end_fblo
	mov [wnd_pFig],eax ;фигура
	stdcall sel_points_get_count,eax
	cmp eax,1
	jne .no_select_1
		stdcall found_parent_obj,[wnd_pFig]
		or eax,eax
		jz .end0 ;if not found

		mov [wnd_pObj],eax ;obj
		mov [wnd_FigN],ecx ;fig number
		stdcall sel_points_get_first,[wnd_pFig]
		mov [wnd_pPoi],eax
		mov edx,[wnd_pFig]
		sub eax,[edx+Figure.PoiData]
		xor edx,edx
		mov ecx,sizeof.Point
		div ecx
		jmp .end1
	.no_select_1:
	or eax,eax
	jnz .end_fblo
		stdcall found_parent_obj,[wnd_pFig]
		or eax,eax
		jz .end0 ;if not found

		mov [wnd_pObj],eax ;obj
		mov [wnd_FigN],ecx ;fig number
		xor eax,eax
		mov [wnd_pPoi],eax
		mov edx,[wnd_pFig]
		mov eax,[edx+Figure.PoiCount]
	.end1:
		mov [wnd_PoiN],eax
		mcall SF_CREATE_THREAD,1,start_prop,thread_coords
		jmp .end0
	.end_fblo:
		notify_window_run txt_err_no_1_point_sel
		;jmp @f
	;.end_fblo:
		;notify_window_run txt_err_...
	.end0:
popad
	.end_f:
	ret

align 4
but_dlg_opt_scale:
	cmp byte[wnd_run_scale],0
	jne .end_f
pushad
	mcall SF_CREATE_THREAD,1,start_scale,thread_scale
popad
	.end_f:
	ret



;input:
; buf - указатель на строку, число должно быть в 10 или 16 ричном виде
;output:
; eax - число
align 4
proc conv_str_to_int uses ebx ecx esi, buf:dword
	xor eax,eax
	xor ebx,ebx
	mov esi,[buf]

	;на случай если перед числом находятся пробелы
	@@:
	cmp byte[esi],' '
	jne @f
		inc esi
		jmp @b
	@@:

	;определение отрицательных чисел
	xor ecx,ecx
	inc ecx
	cmp byte[esi],'-'
	jne @f
		dec ecx
		inc esi
	@@:

	cmp word[esi],'0x'
	je .load_digit_16

	.load_digit_10: ;считывание 10-тичных цифр
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'9'
		jg @f
			sub bl,'0'
			imul eax,10
			add eax,ebx
			inc esi
			jmp .load_digit_10
	jmp @f

	.load_digit_16: ;считывание 16-ричных цифр
		add esi,2
	.cycle_16:
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'f'
		jg @f
		cmp bl,'9'
		jle .us1
			cmp bl,'A'
			jl @f ;отсеиваем символы >'9' и <'A'
		.us1: ;составное условие
		cmp bl,'F'
		jle .us2
			cmp bl,'a'
			jl @f ;отсеиваем символы >'F' и <'a'
			sub bl,32 ;переводим символы в верхний регистр, для упрощения их последущей обработки
		.us2: ;составное условие
			sub bl,'0'
			cmp bl,9
			jle .cor1
				sub bl,7 ;convert 'A' to '10'
			.cor1:
			shl eax,4
			add eax,ebx
			inc esi
			jmp .cycle_16
	@@:
	cmp ecx,0 ;если число отрицательное
	jne @f
		sub ecx,eax
		mov eax,ecx
	@@:
	ret
endp


;данные для диалога открытия файлов
align 4
OpenDialog_data:
.type			dd 0 ;0 - открыть, 1 - сохранить, 2 - выбрать дтректорию
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd plugin_path	;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 путь к диалогу открытия файлов
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path		dd openfile_path	;+36 путь к открываемому файлу
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/sys',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/sys/File managers/',0

Filter:
dd Filter.end - Filter ;.1
.1: db 'NC',0
.2: db 'PNG',0
.end:
db 0


align 4
system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'box_lib.obj',0

align 4
l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, library_path, system_dir_2, import_buf2d
	lib_3 l_libs lib_name_3, file_name, system_dir_3, import_box_lib
l_libs_end:

align 4
import_libimg:
	dd alib_init1
	img_is_img  dd aimg_is_img
	img_info    dd aimg_info
	img_from_file dd aimg_from_file
	img_to_file dd aimg_to_file
	img_from_rgb dd aimg_from_rgb
	img_to_rgb  dd aimg_to_rgb
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_encode  dd aimg_encode
	img_create  dd aimg_create
	img_destroy dd aimg_destroy
	img_destroy_layer dd aimg_destroy_layer
	img_count   dd aimg_count
	img_lock_bits dd aimg_lock_bits
	img_unlock_bits dd aimg_unlock_bits
	img_flip    dd aimg_flip
	img_flip_layer dd aimg_flip_layer
	img_rotate  dd aimg_rotate
	img_rotate_layer dd aimg_rotate_layer
	img_draw    dd aimg_draw

	dd 0,0
	alib_init1   db 'lib_init',0
	aimg_is_img  db 'img_is_img',0 ;определяет по данным, может ли библиотека сделать из них изображение
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0 ;преобразование изображения в данные RGB
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;автоматически определяет формат графических данных
	aimg_encode  db 'img_encode',0
	aimg_create  db 'img_create',0
	aimg_destroy db 'img_destroy',0
	aimg_destroy_layer db 'img_destroy_layer',0
	aimg_count   db 'img_count',0
	aimg_lock_bits db 'img_lock_bits',0
	aimg_unlock_bits db 'img_unlock_bits',0
	aimg_flip    db 'img_flip',0
	aimg_flip_layer db 'img_flip_layer',0
	aimg_rotate  db 'img_rotate',0
	aimg_rotate_layer db 'img_rotate_layer',0
	aimg_draw    db 'img_draw',0

align 4
import_proclib:
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
	OpenDialog_Set_file_name dd aOpenDialog_Set_file_name
	OpenDialog_Set_file_ext dd aOpenDialog_Set_file_ext
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0
	aOpenDialog_Set_file_name db 'OpenDialog_set_file_name',0
	aOpenDialog_Set_file_ext db 'OpenDialog_set_file_ext',0

align 4
import_buf2d:
	init dd sz_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_resize dd sz_buf2d_resize
	buf2d_line dd sz_buf2d_line
	buf2d_line_sm dd sz_buf2d_line_sm
	buf2d_rect_by_size dd sz_buf2d_rect_by_size
	buf2d_filled_rect_by_size dd sz_buf2d_filled_rect_by_size
	buf2d_circle dd sz_buf2d_circle
	buf2d_img_hdiv2 dd sz_buf2d_img_hdiv2
	buf2d_img_wdiv2 dd sz_buf2d_img_wdiv2
	buf2d_conv_24_to_8 dd sz_buf2d_conv_24_to_8
	buf2d_conv_24_to_32 dd sz_buf2d_conv_24_to_32
	buf2d_bit_blt dd sz_buf2d_bit_blt
	buf2d_bit_blt_transp dd sz_buf2d_bit_blt_transp
	buf2d_bit_blt_alpha dd sz_buf2d_bit_blt_alpha
	buf2d_curve_bezier dd sz_buf2d_curve_bezier
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	buf2d_crop_color dd sz_buf2d_crop_color
	buf2d_flip_h dd sz_buf2d_flip_h
	buf2d_flip_v dd sz_buf2d_flip_v
	buf2d_offset_h dd sz_buf2d_offset_h
	buf2d_flood_fill dd sz_buf2d_flood_fill
	buf2d_set_pixel dd sz_buf2d_set_pixel
	dd 0,0
	sz_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_line_sm db 'buf2d_line_sm',0
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_circle db 'buf2d_circle',0
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_conv_24_to_32 db 'buf2d_conv_24_to_32',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	sz_buf2d_bit_blt_transp db 'buf2d_bit_blt_transp',0
	sz_buf2d_bit_blt_alpha db 'buf2d_bit_blt_alpha',0
	sz_buf2d_curve_bezier db 'buf2d_curve_bezier',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_flip_h db 'buf2d_flip_h',0
	sz_buf2d_flip_v db 'buf2d_flip_v',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0
	sz_buf2d_flood_fill db 'buf2d_flood_fill',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0

align 4
import_box_lib:
	dd sz_init1

	init_checkbox   dd sz_Init_checkbox
	check_box_draw  dd sz_Check_box_draw
	check_box_mouse dd sz_Check_box_mouse
	;version_ch     dd sz_Version_ch

	option_box_draw	 dd sz_Option_box_draw
	option_box_mouse dd sz_Option_box_mouse
	;version_op      dd sz_Version_op

	edit_box_draw      dd sz_edit_box_draw
	edit_box_key       dd sz_edit_box_key
	edit_box_mouse     dd sz_edit_box_mouse
	edit_box_set_text  dd sz_edit_box_set_text
	scrollbar_ver_draw dd sz_scrollbar_ver_draw
	scrollbar_hor_draw dd sz_scrollbar_hor_draw

	tl_data_init dd sz_tl_data_init
	tl_data_clear dd sz_tl_data_clear
	tl_info_clear dd sz_tl_info_clear
	tl_key dd sz_tl_key
	tl_mouse dd sz_tl_mouse
	tl_draw dd sz_tl_draw
	tl_info_undo dd sz_tl_info_undo
	tl_info_redo dd sz_tl_info_redo
	tl_node_add dd sz_tl_node_add
	tl_node_set_data dd sz_tl_node_set_data
	tl_node_get_data dd sz_tl_node_get_data
	tl_node_delete dd sz_tl_node_delete
	tl_node_move_up dd sz_tl_node_move_up
	tl_node_move_down dd sz_tl_node_move_down
	tl_cur_beg dd sz_tl_cur_beg
	tl_cur_next dd sz_tl_cur_next
	tl_cur_perv dd sz_tl_cur_perv
	tl_node_close_open dd sz_tl_node_close_open
	tl_node_lev_inc dd sz_tl_node_lev_inc
	tl_node_lev_dec dd sz_tl_node_lev_dec
	tl_node_poi_get_info dd sz_tl_node_poi_get_info
	tl_node_poi_get_next_info dd sz_tl_node_poi_get_next_info
	tl_node_poi_get_data dd sz_tl_node_poi_get_data

	dd 0,0
	sz_init1 db 'lib_init',0

	sz_Init_checkbox   db 'init_checkbox2',0
	sz_Check_box_draw  db 'check_box_draw2',0
	sz_Check_box_mouse db 'check_box_mouse2',0
	;sz_Version_ch     db 'version_ch2',0

	sz_Option_box_draw	db 'option_box_draw',0
	sz_Option_box_mouse	db 'option_box_mouse',0
	;sz_Version_op      db 'version_op',0

	sz_edit_box_draw      db 'edit_box_draw',0
	sz_edit_box_key       db 'edit_box_key',0
	sz_edit_box_mouse     db 'edit_box_mouse',0
	sz_edit_box_set_text  db 'edit_box_set_text',0
	sz_scrollbar_ver_draw db 'scrollbar_v_draw',0
	sz_scrollbar_hor_draw db 'scrollbar_h_draw',0

	sz_tl_data_init db 'tl_data_init',0
	sz_tl_data_clear db 'tl_data_clear',0
	sz_tl_info_clear db 'tl_info_clear',0
	sz_tl_key db 'tl_key',0
	sz_tl_mouse db 'tl_mouse',0
	sz_tl_draw db 'tl_draw',0
	sz_tl_info_undo db 'tl_info_undo',0
	sz_tl_info_redo db 'tl_info_redo',0
	sz_tl_node_add db 'tl_node_add',0
	sz_tl_node_set_data db 'tl_node_set_data',0
	sz_tl_node_get_data db 'tl_node_get_data',0
	sz_tl_node_delete db 'tl_node_delete',0
	sz_tl_node_move_up db 'tl_node_move_up',0
	sz_tl_node_move_down db 'tl_node_move_down',0
	sz_tl_cur_beg db 'tl_cur_beg',0
	sz_tl_cur_next db 'tl_cur_next',0
	sz_tl_cur_perv db 'tl_cur_perv',0
	sz_tl_node_close_open db 'tl_node_close_open',0
	sz_tl_node_lev_inc db 'tl_node_lev_inc',0
	sz_tl_node_lev_dec db 'tl_node_lev_dec',0
	sz_tl_node_poi_get_info db 'tl_node_poi_get_info',0
	sz_tl_node_poi_get_next_info db 'tl_node_poi_get_next_info',0
	sz_tl_node_poi_get_data db 'tl_node_poi_get_data',0

align 4
mouse_dd dd 0
last_time dd 0

align 16
sc system_colors 

align 16
procinfo process_information 

align 4
buf_0: dd 0 ;указатель на буфер изображения
.l: dw 205 ;+4 left
.t: dw 35 ;+6 top
.w: dd 384 ;+8 w
.h: dd 350 ;+12 h
.color: dd 0xffffd0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_1:
	dd 0 ;указатель на буфер изображения
	dd 0 ;+4 left,top
.w: dd 0
.h: dd 0,0,24 ;+12 color,bit in pixel

font_h dd 0 ;высота шрифта

align 4
buf_png:
	dd 0,0
.w: dd 0
.h: dd 0,0xffffff,24

align 4
el_focus dd tree1
tree1 tree_list size_one_list,1000+2, tl_key_no_edit+tl_draw_par_line,\
	16,16, 0xffffff,0xb0d0ff,0x400040, 5,35,195-16,340, 16,Figure.Caption,0,\
	el_focus,w_scr_t1,0

align 4
w_scr_t1 scrollbar 16,0, 3,0, 15, 100, 0,0, 0,0,0, 1

;input:
; eax - число
; edi - буфер для строки
; len - длинна буфера
;output:
align 4
proc convert_int_to_str uses eax ecx edx edi esi, len:dword
	mov esi,[len]
	add esi,edi
	dec esi
	call .str
	ret
endp

align 4
.str:
	mov ecx,10
	cmp eax,ecx
	jb @f
		xor edx,edx
		div ecx
		push edx
		;dec edi  ;смещение необходимое для записи с конца строки
		call .str
		pop eax
	@@:
	cmp edi,esi
	jge @f
		or al,0x30
		stosb
		mov byte[edi],0 ;в конец строки ставим 0, что-бы не вылазил мусор
	@@:
	ret

align 4
proc mem_copy uses ecx esi edi, destination:dword, source:dword, len:dword
	cld
	mov esi, dword[source]
	mov edi, dword[destination]
	mov ecx, dword[len]
	rep movsb
	ret
endp

align 16
i_end:
	rb 2048
thread_coords:
	rb 2048
thread_scale:
	rb 2048
thread_n_file:
	rb 2048
stacktop:
	sys_path rb 1024
	file_name:
		rb 1024 ;4096 
	library_path rb 1024
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:

