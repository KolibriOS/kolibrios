use32
	org 0x0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 0x1
	dd start
	dd i_end ;размер приложения
	dd mem
	dd stacktop
	dd 0
	dd sys_path

include '../../../../programs/macros.inc'
include '../../../../programs/proc32.inc'
include '../../../../programs/develop/libraries/box_lib/load_lib.mac'
include '../../../dll.inc'
include 'vox_draw.inc'
include 'vox_rotate.inc'

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
caption db 'Voxel editor 03.10.13',0 ;подпись окна

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

run_file_70 FileInfoBlock
image_data dd 0 ;указатель на временную память. для нужен преобразования изображения

fn_toolbar db 'toolbar.png',0
IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
IMAGE_TOOLBAR_SIZE equ IMAGE_TOOLBAR_ICON_SIZE*24
image_data_toolbar dd 0
cursors_count equ 4
IMAGE_CURSORS_SIZE equ 4096*cursors_count ;размер картинки с курсорами

;значения задаваемые по умолчанию, без ini файла
ini_def_window_t equ 10
ini_def_window_l equ 10
ini_def_window_w equ 550
ini_def_window_h equ 415
ini_def_buf_w equ 198 ;=192+6
ini_def_buf_h equ 231 ;=224+7
ini_def_s_zoom equ 5
ini_def_t_size equ 10
ini_def_color_b equ 0xffffff
;описание параматров для ini файла
ini_name db 'vox_editor.ini',0
ini_sec_window db 'Window',0
key_window_t db 't',0
key_window_l db 'l',0
key_window_w db 'w',0
key_window_h db 'h',0
key_buf_w db 'buf_w',0
key_buf_h db 'buf_h',0
ini_sec_options db 'Options',0
key_s_zoom db 's_zoom',0
key_t_size db 'tile_size',0
key_f_size db 'file_size',0
key_col_b db 'c_background',0

macro load_image_file path,buf,size { ;макрос для загрузки изображений
	;path - может быть переменной или строковым параметром
	if path eqtype '' ;проверяем задан ли строкой параметр path
		jmp @f
			local .path_str
			.path_str db path ;формируем локальную переменную
			db 0
		@@:
		;32 - стандартный адрес по которому должен быть буфер с системным путем
		copy_path .path_str,[32],file_name,0x0
	else
		copy_path path,[32],file_name,0x0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой
	end if

	stdcall mem.Alloc, dword size ;выделяем память для изображения
	mov [buf],eax

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword size
	m2m [run_file_70.Buffer], [buf]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], file_name
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je @f
		;определяем вид изображения и переводим его во временный буфер image_data
		stdcall dword[img_decode], dword[buf],ebx,0
		mov dword[image_data],eax
		;преобразуем изображение к формату rgb
		stdcall dword[img_to_rgb2], dword[image_data],dword[buf]
		;удаляем временный буфер image_data
		stdcall dword[img_destroy], dword[image_data]
	@@:
}

OT_MAP_X  equ  0
OT_MAP_Y  equ  0
OT_CAPT_X_COLOR equ  5 ;отступ для подписи цвета
OT_CAPT_Y_COLOR equ 30
PEN_MODE_NONE equ -1
PEN_MODE_CLEAR equ 0 ;режим стирания
PEN_MODE_SELECT_COLOR equ 2 ;режим выбора цвета
PEN_MODE_BRUSH equ 3 ;режим работы с кистью

align 4
start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась библиотека
	mov	ebp,lib_2
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mcall 48,3,sc,sizeof.system_colors
	mcall 40,0x27
	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

;--- load ini file ---
	copy_path ini_name,sys_path,file_name,0
	;window startup pozition
	stdcall dword[ini_get_int],file_name,ini_sec_window,key_window_l,ini_def_window_l
	mov word[wnd_s_pos+2],ax
	stdcall dword[ini_get_int],file_name,ini_sec_window,key_window_w,ini_def_window_w
	mov word[wnd_s_pos],ax
	stdcall dword[ini_get_int],file_name,ini_sec_window,key_window_t,ini_def_window_t
	mov word[wnd_s_pos+6],ax
	stdcall dword[ini_get_int],file_name,ini_sec_window,key_window_h,ini_def_window_h
	mov word[wnd_s_pos+4],ax
	;image buffer size
	stdcall dword[ini_get_int],file_name,ini_sec_window,key_buf_w,ini_def_buf_w
	mov [buf_0.w],eax
	mov [buf_0z.w],eax
	add ax,15
	mov [buf_pl.l],ax ;отступ для правого окна
	stdcall dword[ini_get_int],file_name,ini_sec_window,key_buf_h,ini_def_buf_h
	mov [buf_0.h],eax
	mov [buf_0z.h],eax
	;цвет фона
	stdcall dword[ini_get_color],file_name,ini_sec_window,key_col_b,ini_def_color_b
	mov [buf_0.color],eax
	mov [buf_pl.color],eax

	;мастаб, после которого будет увеличение
	stdcall dword[ini_get_int],file_name,ini_sec_options,key_s_zoom,ini_def_s_zoom
	mov [scaled_zoom],eax
	;размер квадратика на плоскости сечения
	stdcall dword[ini_get_int],file_name,ini_sec_options,key_t_size,ini_def_t_size
	mov [tile_size],eax

	stdcall dword[ini_get_int],file_name,ini_sec_options,key_f_size,64
	shl eax,10
	mov [max_open_file_size],eax

	mov ecx,[scaled_zoom]
	xor eax,eax
	inc eax
	shl eax,cl
	imul eax,[tile_size]
	mov [buf_pl.w],eax
	add eax,[tile_size]
	mov [buf_pl.h],eax

	;*** загрузка курсоров
	load_image_file 'cursors_gr.png',image_data_toolbar,IMAGE_CURSORS_SIZE
	stdcall [buf2d_create_f_img], buf_curs_8,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память

	load_image_file 'cursors.png',image_data_toolbar, IMAGE_CURSORS_SIZE
	stdcall [buf2d_create_f_img], buf_curs,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память

	stdcall [buf2d_conv_24_to_8], buf_curs_8,1 ;делаем буфер прозрачности 8бит
	stdcall [buf2d_conv_24_to_32],buf_curs,buf_curs_8 ;делаем буфер rgba 32бит


	stdcall [buf2d_create], buf_0 ;создание буфера изображения
	stdcall [buf2d_create], buf_0z ;создание буфера глубины
	stdcall [buf2d_create], buf_pl ;создание буфера для сечения

	stdcall [buf2d_vox_brush_create], buf_vox_g3, vox_6_7_z
	stdcall [buf2d_vox_brush_create], buf_vox_g2, vox_6_4_z

	load_image_file fn_toolbar, image_data_toolbar,IMAGE_TOOLBAR_SIZE

	stdcall mem.Alloc,[max_open_file_size]
	mov dword[open_file_vox],eax

	call but_new_file

	;первоначальная установка курсора
	stdcall set_pen_mode,1,0,((9 shl 8)+9) shl 16 ;pen

align 4
red_win:
	call draw_window

align 4
still:
	mcall 10

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jne @f
		mcall 9,procinfo,-1
		cmp ax,word[procinfo+4]
		jne @f ;окно не активно
		call mouse
	@@:
	jmp still

align 4
mouse:
	pushad
	mcall 37,2
	bt eax,1 ;right button
	jnc @f
		mcall 37,1 ;get mouse coords
		mov ebx,eax
		shr ebx,16
		and eax,0xffff
		stdcall get_buf_color, buf_0
		stdcall get_buf_color, buf_pl
		jmp .end_f
	@@:
	bt eax,0 ;left button
	jnc .end_f
		mcall 37,1 ;get mouse coords
		mov ebx,eax
		shr ebx,16
		and eax,0xffff

		cmp dword[v_pen_mode],PEN_MODE_SELECT_COLOR
		jne .end_2
			stdcall get_buf_color, buf_0
			stdcall get_buf_color, buf_pl
			jmp .end_f
		.end_2:


		push eax ebx
		mov edx,[v_zoom]
		cmp edx,[scaled_zoom]
		jle @f
		;режим масштабирования изображения
		sub edx,[scaled_zoom]
		sub ax,word[buf_0.t]
		sub bx,word[buf_0.l]
		stdcall get_mouse_ev_scale, [buf_vox], ebx, eax, [scaled_zoom],edx
		cmp eax,0
		je @f
			mov ebx,eax
			and eax,0x3fffffff
			rol ebx,2
			and ebx,3
			dec ebx

			shl ebx,2
			add ebx,cam_x
			mov dword[ebx],eax ;change [cam_x] or [cam_y] or [cam_z]
			call draw_objects

			pop ebx eax
			jmp .end_f
		@@:
		pop ebx eax


		;eax - mouse coord y
		;ebx - mouse coord x
		movzx edx,word[buf_pl.t]
		add edx,OT_MAP_Y
		cmp eax,edx
		jl .end_f
			sub eax,edx
			xor edx,edx
			mov ecx,[tile_size] ;H
			div ecx
		movzx edx,word[buf_pl.l]
		add edx,OT_MAP_X
		cmp ebx,edx
		jl .end_f
			call convert_y ;преобразование координаты y
			cmp eax,0
			jge .end_0 ;ограничение по нижней координате y
				cmp eax,-1
				jne .end_f
				;меняем сечение, попали на квадратик
				sub ebx,edx
				mov eax,ebx
				xor edx,edx
				mov ecx,[tile_size] ;W
				div ecx
				mov [n_plane],eax
				jmp .end_1
			.end_0:
			mov [v_cur_y],eax ;Y-coord
			sub ebx,edx
			mov eax,ebx
			xor edx,edx
			mov ecx,[tile_size] ;W
			div ecx
			mov [v_cur_x],eax ;X-coord

			cmp dword[v_pen_mode],PEN_MODE_CLEAR
			jl .end_1
			cmp dword[v_pen_mode],1
			jg .end_1
				mov eax,[v_cur_x]
				mov ebx,[n_plane]
				mov edx,[v_cur_y]

				mov ecx,[v_zoom]
				cmp ecx,[scaled_zoom]
				jle .no_c_coord_0
					;преобразование координат, с учетом увеличения
					;sub ecx,[scaled_zoom] ;в ecx прирост масштаба (ecx>0)
					mov ecx,[scaled_zoom]

					mov edi,[cam_x]
					shl edi,cl
					add eax,edi
					mov edi,[cam_y]
					shl edi,cl
					add ebx,edi
					mov edi,[cam_z]
					shl edi,cl
					add edx,edi
				.no_c_coord_0:

				;отличающийся параметр для функции создания вокселя
				cmp dword[v_pen_mode],1
				jne @f
					push dword[v_color]
				@@:

				mov ecx,dword[v_zoom]
				mov edi,eax
				mov esi,ebx
				mcall 66,3
				and eax,3 ;3 -> бит 0 левый Shift нажат, бит 1 правый Shift нажат
                jz .shift_end
                    ;если нажат Shift, то редактируем на верхнем уровне
                    ;для этого все координаты делим на 2 а от мастаба отнимаем 1
                    shr edx,1
                    shr esi,1
                    shr edi,1
                    dec ecx
                .shift_end:
                
                ;вызов общих параметров для функций
				push ecx edx esi edi
				push dword[open_file_vox]

				;вызов функций
				cmp dword[v_pen_mode],1
				jne @f
					call buf2d_vox_obj_create_node
					;stdcall buf2d_vox_obj_create_node, [open_file_vox], edi,esi,edx, [v_zoom], [v_color]
					jmp .end_1
				@@:
					call buf2d_vox_obj_delete_node
					;stdcall buf2d_vox_obj_delete_node, [open_file_vox], edi,esi,edx, [v_zoom]

			.end_1:
			call draw_objects
			call draw_pok
	.end_f:
	popad
	ret

;input:
; eax - coord y
; ebx - coord x
align 4
proc get_buf_color, buf:dword
pushad
	mov edi,[buf]
	cmp ax,buf2d_t
	jl .end_f
	sub ax,buf2d_t
	cmp eax,buf2d_h
	jg .end_f
	cmp bx,buf2d_l
	jl .end_f
	sub bx,buf2d_l
	cmp ebx,buf2d_w
	jg .end_f
		stdcall [buf2d_get_pixel], edi,ebx,eax
		mov [v_color],eax
		call on_change_color ;отображаем изменения цвета
	.end_f:
popad
	ret
endp

;преобразовываем координату y (значение должно увеличиваться с низу вверх)
align 4
convert_y:
	push ecx edx
	mov ecx,[v_zoom]
	cmp ecx,[scaled_zoom]
	jle @f
		mov ecx,[scaled_zoom]
	@@:
	mov edx,1
	cmp ecx,1
	jl @f
		shl edx,cl
	@@:
	sub edx,eax
	dec edx
	mov eax,edx
	pop edx ecx
	ret

align 4
draw_window:
pushad
	mcall 12,1

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x30000000
	mov edi,caption
	mcall 0,dword[wnd_s_pos],dword[wnd_s_pos+4]

	; *** создание кнопок на панель ***
	mov eax,8
	mov ebx,(5 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,3
	mov esi,[sc.work_button]
	int 0x40

	mov ebx,(30 shl 16)+20
	mov edx,4
	int 0x40
	add ebx,25 shl 16
	mov edx,5
	int 0x40
	add ebx,30 shl 16
	mov edx,6
	int 0x40
	add ebx,25 shl 16
	mov edx,7
	int 0x40
	add ebx,25 shl 16
	mov edx,8
	int 0x40
	add ebx,25 shl 16
	mov edx,9
	int 0x40
	add ebx,25 shl 16
	mov edx,10
	int 0x40
	add ebx,25 shl 16
	mov edx,11
	int 0x40
	add ebx,25 shl 16
	mov edx,12
	int 0x40
	add ebx,25 shl 16
	mov edx,13
	int 0x40
	add ebx,25 shl 16
	mov edx,14
	int 0x40
	add ebx,25 shl 16
	mov edx,15
	int 0x40
	add ebx,25 shl 16
	mov edx,16
	int 0x40
	add ebx,25 shl 16
	mov edx,17
	int 0x40
	add ebx,25 shl 16
	mov edx,18
	int 0x40
	add ebx,25 shl 16
	mov edx,19
	int 0x40
	add ebx,25 shl 16
	mov edx,20
	int 0x40
	add ebx,25 shl 16
	mov edx,21
	int 0x40
	add ebx,25 shl 16
	mov edx,22
	int 0x40
	add ebx,25 shl 16
	mov edx,23
	int 0x40
	add ebx,25 shl 16
	mov edx,24
	int 0x40
	add ebx,25 shl 16
	mov edx,25
	int 0x40
	add ebx,25 shl 16
	mov edx,26
	int 0x40

	; *** рисование иконок на кнопках ***
	mov eax,7
	mov ebx,[image_data_toolbar]
	mov ecx,(16 shl 16)+16
	mov edx,(7 shl 16)+7 ;icon new
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon open
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon save
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;увелич. масштаб
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;уменьш. масштаб
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;camera 3g 2g
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;поворот z
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;поворот x
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;поворот y
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;сдвиг плоскости +
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;сдвиг плоскости -
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;инструмент карандаш
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;инструмент кисть
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;инструмент затирачка
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;выбор цвета
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;освещение
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;рендер 2*2
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;создать кисть
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;нарисовать кисть
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;удалить кисть
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;ширина кисти -1
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;ширина кисти +1
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;высота кисти -1
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;высота кисти +1
	int 0x40

	; *** рисование буферов ***
	call draw_objects
	call draw_pok

	mcall 12,2
popad
	ret

align 4
draw_pok:
	mov eax,47
	mov ecx,[v_zoom]
	mov ebx,(3 shl 16)+(1 shl 31)
	mov edx,((350+6*9) shl 16)+OT_CAPT_Y_COLOR+2
	mov esi,[sc.work_button_text]
	or  esi,(1 shl 30)
	mov edi,[sc.work_button]
	int 0x40 ;масштаб
	mov ecx,[n_plane]
	add edx,115 shl 16 ;9
	int 0x40 ;номер сечения

	;обновление параметров курсора
	mov dword[txt_curor.size],0
	mov eax,dword[v_cur_x]
	mov edi,txt_curor.size
	call convert_int_to_str
	stdcall str_cat, edi,txt_mull
	mov eax,dword[v_cur_y]
	mov edi,txt_buf
	call convert_int_to_str
	stdcall str_cat, txt_curor.size,edi
	stdcall str_cat, txt_curor.size,txt_space ;завершающий пробел

	;обновление параметров кисти
	mov dword[txt_brush.size],0
	mov eax,dword[brush_w]
	mov edi,txt_brush.size
	call convert_int_to_str
	stdcall str_cat, edi,txt_mull
	mov eax,dword[brush_h]
	mov edi,txt_buf
	call convert_int_to_str
	stdcall str_cat, txt_brush.size,edi
	stdcall str_cat, txt_brush.size,txt_space ;завершающий пробел

	mov eax,4 ;рисование текста
	mov ebx,(OT_CAPT_X_COLOR shl 16)+OT_CAPT_Y_COLOR+2
	mov ecx,[sc.work_text]
	or  ecx,0x80000000 ;or (1 shl 30)
	mov edx,txt_color
	int 0x40

	mov edx,txt_curor
	add ebx,115 shl 16
	or  ecx,(1 shl 30)
	mov edi,[sc.work]
	int 0x40

	mov edx,txt_brush
	add ebx,115 shl 16
	int 0x40

	mov edx,txt_zoom
	add ebx,115 shl 16
	int 0x40

	mov edx,txt_n_plane
	add ebx,115 shl 16
	int 0x40

	call on_change_color
	ret

align 4
on_change_color:
pushad
	mov ebx,((OT_CAPT_X_COLOR+35) shl 16)+16 ;по оси x
	mov ecx,(OT_CAPT_Y_COLOR shl 16)+12 ;по оси y
	mov edx,[v_color]
	mcall 13

	mov ebx,(1 shl 8)+(6 shl 16)
	mov ecx,edx
	mov edx,((OT_CAPT_X_COLOR+55) shl 16)+OT_CAPT_Y_COLOR+2
	mov esi,[sc.work_text]
	add esi,(1 shl 30)
	mov edi,[sc.work]
	mcall 47
popad
	ret

align 4
key:
	mcall 2
	jmp still


align 4
button:
	mcall 17
	cmp ah,3
	jne @f
		call but_new_file
		call draw_objects
		call draw_pok
	@@:
	cmp ah,4
	jne @f
		call but_open_file
	@@:
	cmp ah,5
	jne @f
		call but_save_file
	@@:
	cmp ah,6
	jne @f
		call but_camera_3g_2g
	@@:
	cmp ah,7
	jne @f
		call but_zoom_p
	@@:
	cmp ah,8
	jne @f
		call but_zoom_m
	@@:
	cmp ah,9
	jne @f
		call but_r_z
	@@:
	cmp ah,10
	jne @f
		call but_r_x
	@@:
	cmp ah,11
	jne @f
		call but_r_y
	@@:
	cmp ah,12
	jne @f
		call but_plane_inc
	@@:
	cmp ah,13
	jne @f
		call but_plane_dec
	@@:
	cmp ah,14
	jne @f
		call but_mode_pen
	@@:
	cmp ah,15
	jne @f
		call but_mode_brush
	@@:
	cmp ah,16
	jne @f
		call but_mode_clear
	@@:
	cmp ah,17
	jne @f
		stdcall set_pen_mode,PEN_MODE_SELECT_COLOR,3,((9 shl 8)+9) shl 16
		call draw_palete
	@@:
	cmp ah,18
	jne @f
		call but_light
	@@:
	cmp ah,19
	jne @f
		call but_rend_2_2
	@@:
	cmp ah,20
	jne @f
		call but_brush_copy
	@@:
	cmp ah,21
	jne @f
		call but_brush_draw
	@@:
	cmp ah,22
	jne @f
		call but_brush_clear
	@@:
	cmp ah,23
	jne @f
		call but_bru_w_m
	@@:
	cmp ah,24
	jne @f
		call but_bru_w_p
	@@:
	cmp ah,25
	jne @f
		call but_bru_h_m
	@@:
	cmp ah,26
	jne @f
		call but_bru_h_p
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall [buf2d_delete],buf_0z
	cmp dword[buf_r_img],0
	je @f
		stdcall [buf2d_delete],buf_r_img
		stdcall [buf2d_delete],buf_r_z
	@@:
	stdcall [buf2d_vox_brush_delete], buf_vox_g3
	stdcall [buf2d_vox_brush_delete], buf_vox_g2
	stdcall [buf2d_delete],buf_curs
	stdcall [buf2d_delete],buf_curs_8
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_vox]
	mcall -1

;данные для инициализации воксельного объекта
align 4
vox_new_data:
	db 2,0,0,0
	db 0,1,2,3,4,5,6,7 ;default table
	dd 0 ;null node

;инициализация воксельного объекта
align 4
proc but_new_file uses ecx edi esi
	mov ecx,vox_offs_data+4
	mov esi,vox_new_data
	mov edi,[open_file_vox]
	cld
	rep movsb
	ret
endp

align 4
open_file_vox dd 0 ;указатель на область для открытия файлов

align 4
but_open_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;код при удачном открытии диалога

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	m2m dword[run_file_70.Count], dword[max_open_file_size]
	m2m [run_file_70.Buffer], [open_file_vox]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je .end_open_file

	;add ebx,[open_file_vox]
	;mov byte[ebx],0 ;на случай если ранее был открыт файл большего размера чистим конец буфера с файлом
	mcall 71,1,openfile_path

	;---
	;
	mov eax,[open_file_vox]
	movzx eax,byte[eax]
	and eax,0xff ;берем масштаб по умолчанию
	mov dword[v_zoom],eax ;берем масштаб по умолчанию
	mov dword[cam_x],0
	mov dword[cam_y],0
	mov dword[cam_z],0
	call draw_objects
	.end_open_file:
	popad
	ret

align 4
but_save_file:
	pushad
		copy_path open_dialog_name,communication_area_default_path,file_name,0
		mov [OpenDialog_data.type],1
		stdcall [OpenDialog_Start],OpenDialog_data
		cmp [OpenDialog_data.status],2
		je .end_save_file
		;код при удачном открытии диалога

		mov eax,dword[v_zoom] ;задаем масштаб по умолчанию
		mov ebx,[open_file_vox]
		mov byte[ebx],al

		stdcall buf2d_vox_obj_get_size, ebx
		mov dword[run_file_70.Count], eax ;размер файла
		mov eax,70 ;70-я функция работа с файлами
		mov [run_file_70.Function], 2
		mov [run_file_70.Position], 0
		mov [run_file_70.Flags], 0
		mov ebx, dword[open_file_vox]
		mov [run_file_70.Buffer], ebx
		mov byte[run_file_70+20], 0
		mov dword[run_file_70.FileName], openfile_path
		mov ebx,run_file_70
		int 0x40 ;загружаем файл изображения
		cmp ebx,0xffffffff
		je .end_save_file

		.end_save_file:
	popad
	ret

;смена вида камеры на просмотр 3 и 2 граней
align 4
but_camera_3g_2g:
	cmp dword[buf_vox],buf_vox_g3
	je @f
		mov dword[buf_vox],buf_vox_g3
		jmp .end_0
	@@:
		mov dword[buf_vox],buf_vox_g2
	.end_0:
	call draw_objects
	ret

;увеличение масштаба
align 4
but_zoom_p:
	cmp dword[v_zoom],10 ;максимальный размер, до которого можно увеличить 2^10=1024
	jge @f
		inc dword[v_zoom]
		shl dword[n_plane],1
		push eax
		mov eax,[v_zoom]
		cmp eax,[scaled_zoom]
		jl .end_0
			push ecx
			mov ecx,[scaled_zoom]
			xor eax,eax
			inc eax
			shl eax,cl
			shl dword[cam_x],1
			shl dword[cam_y],1
			shl dword[cam_z],1
			cmp eax,[n_plane]
			jg .end_1
				;коректировка ползунка
				sub [n_plane],eax
				inc dword[cam_y]
			.end_1:
			pop ecx
		.end_0:
		pop eax
		call draw_objects
		call draw_pok
	@@:
	ret

;уменьшение масштаба
align 4
but_zoom_m:
	cmp dword[v_zoom],1
	jl @f
		dec dword[v_zoom]
		shr dword[n_plane],1
		push eax
		mov eax,[v_zoom]
		cmp eax,[scaled_zoom]
		jl .end_0
			shr dword[cam_x],1
			shr dword[cam_y],1
			jnc .end_1
				;коректировка ползунка
				push ecx
				mov ecx,[scaled_zoom]
				dec ecx
				xor eax,eax
				inc eax
				shl eax,cl
				add [n_plane],eax
				pop ecx
			.end_1:
			shr dword[cam_z],1
		.end_0:
		pop eax
		call draw_objects
		call draw_pok
	@@:
	ret

align 4
but_r_z:
	stdcall vox_obj_rot_z, [open_file_vox]
	call draw_objects
	ret

align 4
but_r_x:
	stdcall vox_obj_rot_x, [open_file_vox]
	call draw_objects
	ret

align 4
but_r_y:
	stdcall vox_obj_rot_y, [open_file_vox]
	call draw_objects
	ret

;сдвигаем плоскость среза
align 4
but_plane_inc:
push eax ecx
	inc dword[n_plane]
	mov eax,[v_zoom]
	cmp eax,[scaled_zoom]
	jle .end_0
		;происходит масштабирование
		mov ecx,[scaled_zoom]
		xor eax,eax
		inc eax
		shl eax,cl
		cmp eax,[n_plane]
		jg @f
			mov dword[n_plane],0
			inc dword[cam_y] ;переходим в соседний куб
			neg ecx
			;inc ecx
			add ecx,[v_zoom]
			xor eax,eax
			inc eax
			shl eax,cl
			cmp eax,[cam_y]
			jg @f
				mov dword[cam_y],0 ;зацикливаем если вылезли за пределы последнего куба
		@@:
		jmp .end_1
	.end_0:
		;масштабирование не происходит
		mov ecx,eax
		xor eax,eax
		inc eax
		shl eax,cl
		cmp eax,[n_plane]
		jg .end_1
			mov dword[n_plane],0
	.end_1:
	call draw_objects
	call draw_pok
pop ecx eax
	ret

;сдвигаем плоскость среза
align 4
but_plane_dec:
	dec dword[n_plane]
	cmp dword[n_plane],0
	jge .end_f
push eax ecx
	mov ecx,[scaled_zoom]
	xor eax,eax
	inc eax
	shl eax,cl
	dec eax
	mov dword[n_plane],eax

	mov eax,[v_zoom]
	cmp eax,[scaled_zoom]
	jle .end_0
		;происходит масштабирование
		dec dword[cam_y] ;переходим в соседний куб
		cmp dword[cam_y],0
		jge .end_0

		mov ecx,eax
		sub ecx,[scaled_zoom]
		xor eax,eax
		inc eax
		shl eax,cl
		dec eax
		mov dword[cam_y],eax ;если номер куба оказался меньше 0 исправляем на максимальное значение
	.end_0:
pop ecx eax
	.end_f:
	call draw_objects
	call draw_pok
	ret

align 4
but_mode_pen:
	push eax
	mov eax,dword[v_pen_mode]
	stdcall set_pen_mode,1,0,((9 shl 8)+9) shl 16 ;pen
	cmp eax,PEN_MODE_SELECT_COLOR
	jne @f
		call draw_objects
	@@:
	pop eax
	ret

align 4
but_mode_brush:
	push eax
	mov eax,dword[v_pen_mode]
	stdcall set_pen_mode,3,1,((9 shl 8)+9) shl 16 ;brush
	cmp eax,PEN_MODE_SELECT_COLOR
	jne @f
		call draw_objects
	@@:
	pop eax
	ret

align 4
but_mode_clear:
	push eax
	mov eax,dword[v_pen_mode]
	stdcall set_pen_mode,PEN_MODE_CLEAR,2,((15 shl 8)+9) shl 16
	cmp eax,PEN_MODE_SELECT_COLOR
	jne @f
		call draw_objects
	@@:
	pop eax
	ret

align 4
but_light:
	xor dword[mode_light],1
	call draw_objects
	ret

align 4
but_rend_2_2:
push edi
	cmp dword[buf_r_img],0
	jne @f
		;создание буфера для рендера
		push ecx esi
		mov edi,buf_r_img
		mov esi,buf_0
		mov ecx,BUF_STRUCT_SIZE
		cld
		rep movsb ;копируем все параметры с основного буфера
		mov edi,buf_r_img
		mov buf2d_data,0
		shl buf2d_w,1 ;увеличиваем размер буфера
		shl buf2d_h,1
		stdcall [buf2d_create],buf_r_img

		mov edi,buf_r_z
		mov esi,buf_0z
		mov ecx,BUF_STRUCT_SIZE
		cld
		rep movsb ;копируем все параметры с основного буфера
		mov edi,buf_r_z
		mov buf2d_data,0
		shl buf2d_w,1 ;увеличиваем размер буфера
		shl buf2d_h,1
		stdcall [buf2d_create],buf_r_z
		pop esi ecx
	@@:
	stdcall [buf2d_clear], buf_r_img, [buf_0.color] ;чистим буфер
	stdcall [buf2d_clear], buf_r_z, 0 ;чистим буфер

	push eax ebx ecx
		mov eax,[v_zoom]
		cmp eax,[scaled_zoom]
		jle .end_scaled
			;рендер увеличенной части объекта
			mov ebx,[scaled_zoom]
			sub eax,ebx
			inc ebx
			stdcall [buf2d_vox_obj_draw_3g_scaled], buf_r_img, buf_r_z, [buf_vox],\
				[open_file_vox], 0,0, 0, ebx, [cam_x],[cam_y],[cam_z],eax, [sc.work_graph]
			bt dword[mode_light],0
			jnc @f
				stdcall [buf2d_vox_obj_draw_3g_shadows], buf_r_img, buf_r_z, [buf_vox], 0,0, 0, ebx, 3
			@@:
			xor ebx,ebx
			xor ecx,ecx
			mov edi,buf_r_img
			stdcall [buf2d_img_hdiv2], edi
			shr buf2d_h,1
			stdcall [buf2d_img_wdiv2], edi
			shr buf2d_w,1
			jmp .show
		.end_scaled:

		inc eax
		stdcall [buf2d_vox_obj_draw_3g], buf_r_img, buf_r_z, [buf_vox], [open_file_vox], 0,0, 0, eax
		stdcall [buf2d_vox_obj_draw_1g], buf_r_img, buf_r_z, [open_file_vox], 0,0, eax
		bt dword[mode_light],0
		jnc @f
			stdcall [buf2d_vox_obj_draw_3g_shadows], buf_r_img, buf_r_z, [buf_vox], 0,0, 0, eax, 3
		@@:

		mov edi,buf_r_img
		stdcall [buf2d_img_hdiv2], edi
		shr buf2d_h,1
		stdcall [buf2d_img_wdiv2], edi
		shr buf2d_w,1

		stdcall [buf2d_vox_obj_get_img_w_3g], [buf_vox],[v_zoom]
		mov ebx,[buf_0.w]
		sub ebx,eax
		shr ebx,1 ;ebx - для центровки маленьких изображений по горизонтали
		stdcall [buf2d_vox_obj_get_img_h_3g], [buf_vox],[v_zoom]
		cmp eax,[buf_0.h]
		jg @f
			mov ecx,[buf_0.h]
			sub ecx,eax
			shr ecx,1 ;ecx - для центровки маленьких изображений по вертикали
		@@:
		.show:
		stdcall [buf2d_bit_blt], buf_0, ebx,ecx, edi
		shl buf2d_h,1
		shl buf2d_w,1
	pop ecx ebx eax
pop edi
	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	ret

;уменьшаем ширину кисти
align 4
but_bru_w_m:
	cmp dword[brush_w],1
	jle @f
		dec dword[brush_w]
		pushad
		call draw_pok
		call draw_plane
		popad
	@@:
	ret
;увеличиваем ширину кисти
align 4
but_bru_w_p:
	cmp dword[brush_w],32
	jge @f
		inc dword[brush_w]
		pushad
		call draw_pok
		call draw_plane
		popad
	@@:
	ret
;уменьшаем высоту кисти
align 4
but_bru_h_m:
	cmp dword[brush_h],1
	jle @f
		dec dword[brush_h]
		pushad
		call draw_pok
		call draw_plane
		popad
	@@:
	ret
;увеличиваем высоту кисти
align 4
but_bru_h_p:
	cmp dword[brush_h],32
	jge @f
		inc dword[brush_h]
		pushad
		call draw_pok
		call draw_plane
		popad
	@@:
	ret

align 4
but_brush_copy:
	cmp dword[v_pen_mode],PEN_MODE_BRUSH
	jne .end_f
pushad
	mov eax,[v_cur_x]
	mov ebx,[n_plane]
	mov edx,[v_cur_y]
		
	mov ecx,[v_zoom]
	cmp ecx,[scaled_zoom]
	jle @f
		;преобразование координат, с учетом увеличения
		;sub ecx,[scaled_zoom] ;в ecx прирост масштаба (ecx>0)
		mov ecx,[scaled_zoom]

		mov edi,[cam_x]
		shl edi,cl
		add eax,edi
		mov edi,[cam_y]
		shl edi,cl
		add ebx,edi
		mov edi,[cam_z]
		shl edi,cl
		add edx,edi
	@@:

	cld
	mov edi,brush_data
	mov esi,edx
	sub esi,[brush_h]
	.cycle_0:
	mov ecx,[brush_w]
	@@:
		push eax
		stdcall buf2d_vox_obj_node_get_color, [open_file_vox], eax,ebx,edx, [v_zoom]
		cmp eax,[v_color]
		jne .end_0
			mov eax,1 shl 30
		.end_0:
		mov dword[edi],eax ;color
		pop eax
		inc eax
		add edi,4
		loop @b
	dec edx
	sub eax,[brush_w]
	cmp edx,esi
	jg .cycle_0

	call draw_objects
popad
	.end_f:
	ret

align 4
but_brush_draw:
	cmp dword[v_pen_mode],PEN_MODE_BRUSH
	jne .end_f
pushad
	mov eax,[v_cur_x]
	mov ebx,[n_plane]
	mov edx,[v_cur_y]
		
	mov ecx,[v_zoom]
	cmp ecx,[scaled_zoom]
	jle @f
		;преобразование координат, с учетом увеличения
		;sub ecx,[scaled_zoom] ;в ecx прирост масштаба (ecx>0)
		mov ecx,[scaled_zoom]

		mov edi,[cam_x]
		shl edi,cl
		add eax,edi
		mov edi,[cam_y]
		shl edi,cl
		add ebx,edi
		mov edi,[cam_z]
		shl edi,cl
		add edx,edi
	@@:

	cld
	mov edi,brush_data
	mov esi,edx
	sub esi,[brush_h]
	.cycle_0:
	mov ecx,[brush_w]
	@@:
		bt dword[edi],31 ;бит прозрачности
		jc .end_2
		bt dword[edi],30 ;бит текущего цвета
		jnc .end_0
			push dword[v_color]
			jmp .end_1
		.end_0:
			push dword[edi]
		.end_1:
		stdcall buf2d_vox_obj_create_node, [open_file_vox], eax,ebx,edx, [v_zoom] ;, color
		.end_2:
		inc eax
		add edi,4
		loop @b
	dec edx
	sub eax,[brush_w]
	cmp edx,esi
	jg .cycle_0

	call draw_objects
popad
	.end_f:
	ret

align 4
but_brush_clear:
	cmp dword[v_pen_mode],PEN_MODE_BRUSH
	jne .end_f
pushad
	mov eax,[v_cur_x]
	mov ebx,[n_plane]
	mov edx,[v_cur_y]
		
	mov ecx,[v_zoom]
	cmp ecx,[scaled_zoom]
	jle @f
		;преобразование координат, с учетом увеличения
		;sub ecx,[scaled_zoom] ;в ecx прирост масштаба (ecx>0)
		mov ecx,[scaled_zoom]

		mov edi,[cam_x]
		shl edi,cl
		add eax,edi
		mov edi,[cam_y]
		shl edi,cl
		add ebx,edi
		mov edi,[cam_z]
		shl edi,cl
		add edx,edi
	@@:

	cld
	mov edi,brush_data
	mov esi,edx
	sub esi,[brush_h]
	.cycle_0:
	mov ecx,[brush_w]
	@@:
		bt dword[edi],31 ;бит прозрачности
		jc .end_2
		stdcall buf2d_vox_obj_delete_node, [open_file_vox], eax,ebx,edx, [v_zoom]
		.end_2:
		inc eax
		add edi,4
		loop @b
	dec edx
	sub eax,[brush_w]
	cmp edx,esi
	jg .cycle_0

	call draw_objects
popad
	.end_f:
	ret

align 4
draw_palete:
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
	stdcall buf2d_draw_palete, buf_0, 5,3, 9,6, 18, 512
	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	ret

align 4
v_zoom dd 3 ;текущий масштаб
v_cur_x dd 0 ;координата курсора x
v_cur_y dd 0 ;координата курсора y (но ось в объекте z)
n_plane dd 0 ;плоскость сечения
v_color dd 0xff ;цвет карандаша
v_pen_mode dd PEN_MODE_NONE ;режим работы курсора (см. константы PEN_MODE_...)
mode_light dd 1 ;режим освещения
cam_x dd 0
cam_y dd 0
cam_z dd 0
scaled_zoom dd 5 ;масштаб после которого начинается рисование части изображения
tile_size dd ? ;размер квадратика на плоскости с сечением
max_open_file_size dd ?
brush_w dd 5 ;ширина кисти
brush_h dd 5 ;высота кисти
brush_data dd 1 shl 31,1 shl 30,1 shl 30,1 shl 30,1 shl 31
dd 1 shl 30,1 shl 30,1 shl 30,1 shl 30,1 shl 30
dd 1 shl 30,1 shl 30,1 shl 30,1 shl 30,1 shl 30
dd 1 shl 30,1 shl 30,1 shl 30,1 shl 30,1 shl 30
dd 1 shl 31,1 shl 30,1 shl 30,1 shl 30,1 shl 31
rd 999 ;32*32-25

txt_zoom db 'Масштаб:',0
txt_curor: db 'Курсор: '
.size: rb 10
txt_n_plane db 'Сечение:',0
txt_color db 'Цвет:',0
txt_brush: db 'Кисть: '
.size: rb 10
txt_mull db '*',0
txt_space db ' ',0
txt_buf rb 16

;рисование буфера с воксельными объектами
align 4
draw_objects:
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
	stdcall [buf2d_clear], buf_0z, 0 ;чистим буфер

	cmp dword[v_pen_mode],PEN_MODE_SELECT_COLOR
	jne @f
		call draw_palete
		jmp .end_f
	@@:
	push eax ebx ecx
	stdcall [buf2d_vox_obj_get_img_w_3g], [buf_vox],[v_zoom]
	mov ebx,[buf_0.w]
	sub ebx,eax
	shr ebx,1 ;ebx - для центровки маленьких изображений по горизонтали

	xor ecx,ecx
	stdcall [buf2d_vox_obj_get_img_h_3g], [buf_vox],[v_zoom]
	cmp eax,[buf_0.h]
	jg @f
		mov ecx,[buf_0.h]
		sub ecx,eax
		shr ecx,1 ;ecx - для центровки маленьких изображений по вертикали
	@@:

	mov eax,[v_zoom]
	cmp eax,[scaled_zoom]
	jg @f
		;обычный режим изображения
		stdcall [buf2d_vox_obj_draw_3g], buf_0, buf_0z, [buf_vox],\
			[open_file_vox], ebx,ecx, 0, eax
		stdcall [buf2d_vox_obj_draw_1g], buf_0, buf_0z,\
			[open_file_vox], 0,0, eax
		bt dword[mode_light],0
		jnc .end_1
			stdcall [buf2d_vox_obj_draw_3g_shadows], buf_0, buf_0z, [buf_vox], ebx,ecx, 0, eax, 3
		.end_1:
		jmp .end_0
	@@:
		;режим масштабирования изображения
		sub eax,[scaled_zoom]
		stdcall [buf2d_vox_obj_draw_3g_scaled], buf_0, buf_0z, [buf_vox],\
			[open_file_vox], 0,0, 0, [scaled_zoom], [cam_x],[cam_y],[cam_z],eax, [sc.work_graph] ;scroll -> 2^eax
		bt dword[mode_light],0
		jnc .end_2
			stdcall [buf2d_vox_obj_draw_3g_shadows], buf_0, buf_0z, [buf_vox], 0,0, 0, [scaled_zoom], 3
		.end_2:
	.end_0:
	pop ecx ebx eax

	call draw_plane
	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	.end_f:
	ret

;рисование плоскости с сечением объекта
align 4
proc draw_plane uses eax
	stdcall [buf2d_clear], buf_pl, [buf_pl.color] ;чистим буфер

	mov eax,[v_zoom]
	cmp eax,[scaled_zoom]
	jg @f
		;обычный режим изображения
		stdcall [buf2d_vox_obj_draw_pl], buf_pl, [open_file_vox],\
			OT_MAP_X,OT_MAP_Y,[tile_size], eax, [n_plane], [sc.work_graph]
		jmp .end_0
	@@:
		;режим масштабирования изображения
		sub eax,[scaled_zoom]
		stdcall [buf2d_vox_obj_draw_pl_scaled], buf_pl, [open_file_vox],\
			OT_MAP_X,OT_MAP_Y,[tile_size], [scaled_zoom], [n_plane], [sc.work_graph],[cam_x],[cam_y],[cam_z],eax
	.end_0:

	call draw_vox_cursor
	stdcall [buf2d_draw], buf_pl ;обновляем буфер на экране
	.end_f:
	ret
endp

;рисование курсора
align 4
draw_vox_cursor:
pushad
	mov ecx,[v_zoom]
	cmp ecx,[scaled_zoom]
	jle @f
		mov ecx,[scaled_zoom]
	@@:
	xor edx,edx
	inc edx
	shl edx,cl

	mov eax,[v_cur_x]
	cmp eax,edx
	jge .end_f ;курсор за пределами поля
	mov edi,[tile_size]
	imul eax,edi
	add eax,OT_MAP_X
	mov ebx,edx
	dec ebx
	sub ebx,[v_cur_y]
	imul ebx,edi
	add ebx,OT_MAP_Y
	inc eax
	cmp dword[v_pen_mode],PEN_MODE_BRUSH
	je .brush
	inc ebx
    sub edi,2
	stdcall [buf2d_rect_by_size], buf_pl, eax,ebx, edi,edi,[sc.work_graph]
	dec ebx
	add edi,2

	;горизонтальные линии
	sub eax,2
	mov ecx,edi
	imul edi,edx
	shr ecx,1
	add ebx,ecx ;центровка по середине клетки
	mov ecx,OT_MAP_X
	add edi,ecx
	stdcall [buf2d_line], buf_pl, ecx,ebx, eax,ebx,[sc.work_graph]
	add eax,[tile_size]
	inc eax
	cmp eax,edi
	jge @f ;если курсор на краю поля
		dec edi
		stdcall [buf2d_line], buf_pl, eax,ebx, edi,ebx,[sc.work_graph]
	@@:
	jmp .end_f

	;рамка для кисти
	.brush:
	dec eax
	mov ecx,[brush_w]
	imul ecx,[tile_size]
	;mov edi,eax
	mov edi,ecx
	mov esi,[brush_h]
	imul esi,[tile_size]
	stdcall [buf2d_rect_by_size], buf_pl, eax,ebx, edi,esi,[sc.work_graph]

    ;рисование точек для кисти
    mov ecx,[brush_w]
    mov edx,[tile_size]
    sub eax,edx ;eax-=tile_size
    shr edx,2
    add eax,edx ;eax+=tile_size/4
    add ebx,edx ;ebx+=tile_size/4
    mov edx,eax
    mov esi,[tile_size]
    imul esi,ecx
    imul ecx,[brush_h]
    add esi,edx
    ;eax = from edx to esi
    mov edi,brush_data
    cld
    @@:
        add eax,[tile_size]
        bt dword[edi],31
        jc .no_pixel
            bt dword[edi],30
            jc .sel_color
                push dword[edi]
                jmp .set_pixel
            .sel_color:
                push [v_color]
            .set_pixel:
            stdcall [buf2d_rect_by_size], buf_pl, eax,ebx, 3,3 ;, [edi]
        .no_pixel:
        add edi,4
        cmp eax,esi
        jl .end_line        
            mov eax,edx
            add ebx,[tile_size]
        .end_line:
    loop @b

	.end_f:
popad
	ret

;hot_p - координаты горячей точки курсора, смещенные на бит 16 ((cx shl 8) + cy) shl 16
align 4
proc set_pen_mode uses eax ebx ecx edx, mode:dword, icon:dword, hot_p:dword
	mov eax,[mode]
	cmp [v_pen_mode],eax
	je @f
		mov [v_pen_mode],eax
		mov edx,[hot_p]
		mov dx,2 ;LOAD_INDIRECT
		mov ecx,[icon]
		shl ecx,12 ;умножаем на 4 кб
		add ecx,[buf_curs.data]
		mcall 37,4

		cmp eax,0
		je @f
			mov [cursor_pointer],eax
			mcall 37,5,[cursor_pointer]
	@@:
	ret
endp

if 0
;input:
; buf - указатель на строку, число должно быть в 10 или 16 ричном виде
;output:
; eax - число
align 4
proc conv_str_to_int, buf:dword
	xor eax,eax
	push ebx ecx esi
	xor ebx,ebx
	mov esi,[buf]
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
	pop esi ecx ebx
	ret
endp
end if

;input:
; eax = value
; edi = string buffer
;output:
align 4
convert_int_to_str:
	pushad
		mov dword[edi+1],0
		mov word[edi+5],0
		call .str
	popad
	ret

align 4
.str:
	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
    ;преревод числа в ASCII строку взодные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
	cmp eax,ecx  ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx  ;очистить edx
		div ecx      ;разделить - остаток в edx
		push edx     ;положить в стек
		;dec edi             ;смещение необходимое для записи с конца строки
		call .str ;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
	or al,0x30  ;данная команда короче  чем две выше
	stosb	    ;записать элемент из регистра al в ячеку памяти es:edi
	ret	      ;вернуться чень интересный ход т.к. пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться

align 4
proc str_cat, str1:dword, str2:dword
	push eax ecx edi esi
	mov esi,dword[str2]
	stdcall str_len,esi
	mov ecx,eax
	inc ecx
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	pop esi edi ecx eax
	ret
endp

;output:
; eax = strlen
align 4
proc str_len, str1:dword
	mov eax,[str1]
	@@:
		cmp byte[eax],0
		je @f
		inc eax
		jmp @b
	@@:
	sub eax,[str1]
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

default_dir db '/rd/1',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/rd/1/File managers/',0

Filter:
dd Filter.end - Filter ;.1
.1:
db 'VOX',0
.end:
db 0



head_f_i:
head_f_l db 'Системная ошибка',0

system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
err_message_found_lib_0 db 'Не найдена библиотека ',39,'proc_lib.obj',39,0
err_message_import_0 db 'Ошибка при импорте библиотеки ',39,'proc_lib.obj',39,0

system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
err_message_found_lib_1 db 'Не найдена библиотека ',39,'libimg.obj',39,0
err_message_import_1 db 'Ошибка при импорте библиотеки ',39,'libimg.obj',39,0

system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
err_msg_found_lib_2 db 'Не найдена библиотека ',39,'buf2d.obj',39,0
err_msg_import_2 db 'Ошибка при импорте библиотеки ',39,'buf2d',39,0

system_dir_3 db '/sys/lib/'
lib_name_3 db 'libini.obj',0
err_msg_found_lib_3 db 'Не найдена библиотека ',39,'libini.obj',39,0
err_msg_import_3 db 'Ошибка при импорте библиотеки ',39,'libini',39,0

l_libs_start:
	lib0 l_libs lib_name_0, sys_path, file_name, system_dir_0,\
		err_message_found_lib_0, head_f_l, proclib_import,err_message_import_0, head_f_i
	lib1 l_libs lib_name_1, sys_path, file_name, system_dir_1,\
		err_message_found_lib_1, head_f_l, import_libimg, err_message_import_1, head_f_i
	lib_2 l_libs lib_name_2, sys_path, library_path, system_dir_2,\
		err_msg_found_lib_2,head_f_l,import_buf2d,err_msg_import_2,head_f_i
	lib_3 l_libs lib_name_3, sys_path, library_path, system_dir_3,\
		err_msg_found_lib_3,head_f_l,import_libini,err_msg_import_3,head_f_i
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
proclib_import: ;описание экспортируемых функций
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

align 4
import_buf2d:
	init dd sz_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_line dd sz_buf2d_line
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
	buf2d_offset_h dd sz_buf2d_offset_h
	buf2d_flood_fill dd sz_buf2d_flood_fill
	buf2d_set_pixel dd sz_buf2d_set_pixel
	buf2d_get_pixel dd sz_buf2d_get_pixel
	buf2d_vox_brush_create dd sz_buf2d_vox_brush_create
	buf2d_vox_brush_delete dd sz_buf2d_vox_brush_delete
	buf2d_vox_obj_get_img_w_3g dd sz_buf2d_vox_obj_get_img_w_3g
	buf2d_vox_obj_get_img_h_3g dd sz_buf2d_vox_obj_get_img_h_3g
	buf2d_vox_obj_draw_1g dd sz_buf2d_vox_obj_draw_1g
	buf2d_vox_obj_draw_3g dd sz_buf2d_vox_obj_draw_3g
	buf2d_vox_obj_draw_3g_scaled dd sz_buf2d_vox_obj_draw_3g_scaled
	buf2d_vox_obj_draw_3g_shadows dd sz_buf2d_vox_obj_draw_3g_shadows
	buf2d_vox_obj_draw_pl dd sz_buf2d_vox_obj_draw_pl
	buf2d_vox_obj_draw_pl_scaled dd sz_buf2d_vox_obj_draw_pl_scaled

	dd 0,0
	sz_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_line db 'buf2d_line',0
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
	sz_buf2d_offset_h db 'buf2d_offset_h',0
	sz_buf2d_flood_fill db 'buf2d_flood_fill',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0
	sz_buf2d_get_pixel db 'buf2d_get_pixel',0
	sz_buf2d_vox_brush_create db 'buf2d_vox_brush_create',0
	sz_buf2d_vox_brush_delete db 'buf2d_vox_brush_delete',0
	sz_buf2d_vox_obj_get_img_w_3g db 'buf2d_vox_obj_get_img_w_3g',0
	sz_buf2d_vox_obj_get_img_h_3g db 'buf2d_vox_obj_get_img_h_3g',0
	sz_buf2d_vox_obj_draw_1g db 'buf2d_vox_obj_draw_1g',0
	sz_buf2d_vox_obj_draw_3g db 'buf2d_vox_obj_draw_3g',0
	sz_buf2d_vox_obj_draw_3g_scaled db 'buf2d_vox_obj_draw_3g_scaled',0
	sz_buf2d_vox_obj_draw_3g_shadows db 'buf2d_vox_obj_draw_3g_shadows',0
	sz_buf2d_vox_obj_draw_pl db 'buf2d_vox_obj_draw_pl',0
	sz_buf2d_vox_obj_draw_pl_scaled db 'buf2d_vox_obj_draw_pl_scaled',0

align 4
import_libini:
	dd alib_init2
	ini_get_str   dd aini_get_str
	ini_get_int   dd aini_get_int
	ini_get_color dd aini_get_color
dd 0,0
	alib_init2     db 'lib_init',0
	aini_get_str   db 'ini_get_str',0
	aini_get_int   db 'ini_get_int',0
	aini_get_color db 'ini_get_color',0

mouse_dd dd 0x0
sc system_colors 

align 16
procinfo process_information 

;буфер основного изображения
align 4
buf_0: dd 0 ;указатель на дaные изображения
.l: dw 5 ;+4 left
.t: dw 45 ;+6 top
.w: dd 192+6 ;+8 w
.h: dd 224+7 ;+12 h
.color: dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

;буфер глубины основного изображения
align 4
buf_0z: dd 0
	dw 0 ;+4 left
	dw 0 ;+6 top
.w: dd 192+6 ;+8 w
.h: dd 224+7 ;+12 h
.color: dd 0 ;+16 color
	db 32 ;+20 bit in pixel

;буфер для рисования среза объекта
align 4
buf_pl: dd 0
.l: dw 15+192+6 ;+4 left
.t: dw 45 ;+6 top
.w: dd 320 ;+8 w
.h: dd 330 ;+12 h
.color: dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

;буфер для улучшеного рендера
align 4
buf_r_img:
	rb BUF_STRUCT_SIZE
align 4
buf_r_z:
	rb BUF_STRUCT_SIZE

align 4
cursor_pointer dd 0 ;указатель на данные для курсора

buf_curs: ;буфер с курсорами
.data: dd 0 ;указатель на буфер изображения
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd 32 ;+8 w
	dd 32*cursors_count ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_curs_8: ;буфер с прозрачностью для курсоров
.data: dd 0 ;указатель на буфер изображения
	dw 0 ;+4 left
	dw 0 ;+6 top
	dd 32 ;+8 w
	dd 32*cursors_count ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

;данные для создания минимального единичного вокселя
align 4
vox_6_7_z: ;воксель где видно 3 грани
dd 0,0,1,1,0,0,\
   0,2,2,2,2,0,\
   2,2,2,2,2,2,\
   2,3,2,2,3,2,\
   2,3,3,3,3,2,\
   0,3,3,3,3,0,\
   0,0,3,3,0,0

align 4
vox_6_4_z: ;воксель, где видно 2 боковые грани а верхней грани не видно
dd 1,2,3,3,2,1,\
   1,2,3,3,2,1,\
   1,2,3,3,2,1,\
   1,2,3,3,2,1

align 4
buf_vox dd buf_vox_g3

buf_vox_g3:
	db 6,7,4,3 ;w,h,h_osn,n
	rb BUF_STRUCT_SIZE*(3+1)

align 4
buf_vox_g2:
	db 6,4,0,3 ;w,h,h_osn,n
	rb BUF_STRUCT_SIZE*(3+1)

i_end:
	wnd_s_pos: ;место для настроек стартовой позиции окна
		rq 0
	rb 4096 ;2048
stacktop:
	sys_path rb 1024
	file_name:
		rb 1024 ;4096 
	library_path rb 1024
	plugin_path rb 1024 ;4096
	openfile_path rb 1024 ;4096
	filename_area rb 256
mem:
