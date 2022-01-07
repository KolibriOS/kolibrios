use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,openfile_path,sys_path

include 'lang.inc'
include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../load_img.inc'
include '../../load_lib.mac'
include '../../develop/libraries/TinyGL/asm_fork/opengl_const.inc'
include '../../develop/libraries/TinyGL/asm_fork/zbuffer.inc'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'
include '../../develop/info3ds/info_fun_float.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
caption db 'Image transform 08.12.20',0 ;подпись окна

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;данные буфера изображения
buf2d_w equ dword[edi+8] ;ширина буфера
buf2d_h equ dword[edi+12] ;высота буфера
buf2d_l equ word[edi+4]
buf2d_t equ word[edi+6] ;отступ сверху
buf2d_size_lt equ dword[edi+4] ;отступ слева и справа для буфера
buf2d_color equ dword[edi+16] ;цвет фона буфера
buf2d_bits equ byte[edi+20] ;количество бит в 1-й точке изображения

NAV_WND_L equ 145
NAV_WND_T equ 1

include 'select_points.inc'

IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
image_data_toolbar dd 0

align 4
start:
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

	include_image_file '../../../programs/fs/kfar/trunk/font6x9.bmp', image_data_toolbar, buf_font.w,buf_font.h
	stdcall [buf2d_create_f_img], buf_font,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память
	stdcall [buf2d_conv_24_to_8], buf_font,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_font

	include_image_file 'toolbar.png', image_data_toolbar

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	;настройка точек
	call points_init

	;open file from cmd line
	cmp dword[openfile_path],0
	je @f
		call but_open_file.no_dlg
	@@:

align 4
red_win:
	call draw_window

align 16
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
	je timer_funct

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
timer_funct:
	push eax ebx
		mcall SF_SYSTEM_GET,SSF_TIME_COUNT
		mov [last_time],eax
	pop ebx eax
	cmp byte[calc],0
	je still
		call draw_nav_wnd
		call draw_buffers
	jmp still

align 16
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x30000000
	mcall SF_CREATE_WINDOW,(20 shl 16)+410,(20 shl 16)+520,,,caption

	mcall SF_THREAD_INFO,procinfo,-1
	mov eax,dword[procinfo.box.height]
	cmp eax,120
	jge @f
		mov eax,120 ;min size
	@@:
	sub eax,65
	mov ebx,dword[procinfo.box.width]
	cmp ebx,270
	jge @f
		mov ebx,270
	@@:
	sub ebx,19
	cmp eax,dword[buf_0.h] ;смотрим размер буфера
	jne @f
	cmp ebx,dword[buf_0.w]
	jne @f
		jmp .end0
	@@:
		stdcall [buf2d_resize],buf_0,ebx,eax,1
		call calc_nav_params
		mov eax,[nav_x]
		call nav_x_corect
		mov [nav_x],eax
		mov eax,[nav_y]
		call nav_y_corect
		mov [nav_y],eax
		mov byte[calc],1
	.end0:

	; *** создание кнопок на панель ***
	mcall SF_DEFINE_BUTTON,(5 shl 16)+20,(5 shl 16)+20,3, [sc.work_button]

	add ebx,25 shl 16
	mcall ,,,4
	add ebx,30 shl 16
	mcall ,,,5
	add ebx,25 shl 16
	mcall ,,,6
	add ebx,25 shl 16
	mcall ,,,7

	; *** рисование иконок на кнопках ***
	mov edx,(7 shl 16)+7 ;icon open
	mcall SF_PUT_IMAGE,[image_data_toolbar],(16 shl 16)+16

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon save
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;icon view
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon mode
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40

	call draw_nav_wnd
	call draw_buffers

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

;рисование навигационного окна
align 4
proc draw_nav_wnd
	cmp dword[buf_i0],0
	je .end0
	bt dword[nav_x_min],31
	jnc .end0
	bt dword[nav_y_min],31
	jnc .end0
pushad
		mov ebx,(NAV_WND_L shl 16)
		add ebx,[nav_wnd_w]
		mov ecx,(NAV_WND_T shl 16)
		add ecx,[nav_wnd_h]
		mcall SF_DRAW_RECT,,,0 ;изображение
		mov ecx,[nav_wnd_zoom]
		mov ebx,[nav_x]
		neg ebx
		sar ebx,cl
		add ebx,NAV_WND_L
		shl ebx,16
		add ebx,[buf_0.w]
		shr bx,cl
		mov edx,[nav_y]
		neg edx
		sar edx,cl
		add edx,NAV_WND_T
		shl edx,16
		add edx,[buf_0.h]
		shr dx,cl
		mov ecx,edx
		mcall ,,,0x404080 ;часть изображения попадающая в окно
		
		mov edi,sel_pt
		@@:
			mov ecx,[nav_wnd_zoom]
			mov ebx,[edi+point2d.x]
			shr ebx,cl
			add ebx,NAV_WND_L
			mov edx,[edi+point2d.y]
			shr edx,cl
			add edx,NAV_WND_T
			mov ecx,edx
			mcall SF_PUT_PIXEL,,,0xffff00
			add edi,sizeof.point2d
			cmp edi,sel_pt+4*sizeof.point2d
			jl @b
popad
	.end0:
	ret
endp

align 4
proc draw_buffers
pushad
	cmp byte[calc],0
	je .end0
		; *** обновление буфера ***
		bt dword[nav_x_min],31
		jnc .beg0
		bt dword[nav_y_min],31
		jc @f
		.beg0:
			stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
		@@:
		cmp byte[view_b],1
		je .end1
			push buf_i0
			jmp .end2
		.end1:
			push buf_ogl
		.end2:
		stdcall [buf2d_bit_blt], buf_0, [nav_x],[nav_y] ;,...

		call points_draw
		xor eax,eax
		cmp [u_line_v],eax
		je @f
			mov eax,[u_line_v]
			add eax,[nav_x]
			stdcall [buf2d_line], buf_0, eax,0, eax,[buf_0.h], 0xffffff
			inc eax
			stdcall [buf2d_line], buf_0, eax,0, eax,[buf_0.h], 0
		@@:
		xor eax,eax
		cmp [u_line_h],eax
		je @f
			mov eax,[u_line_h]
			add eax,[nav_y]
			stdcall [buf2d_line], buf_0, 0,eax, [buf_0.w],eax, 0xffffff
			inc eax
			stdcall [buf2d_line], buf_0, 0,eax, [buf_0.w],eax, 0
		@@:

		; *** обновление подписи размера файла ***
		mov edi,txt_f_size.size
		mov eax,[open_file_size]
		mov ebx,txt_pref
		.cycle:
			cmp eax,1024
			jl @f
			shr eax,10
			add ebx,4
			jmp .cycle
		@@:
		stdcall convert_int_to_str, 16
		stdcall str_cat, edi,ebx
		stdcall str_cat, edi,txt_space ;завершающий пробел
		;ширина и высота изображения
		mov eax,[buf_i0.w]
		mov edi,txt_img_w.size
		stdcall convert_int_to_str, 16
		mov eax,[buf_i0.h]
		mov edi,txt_img_h.size
		stdcall convert_int_to_str, 16
		mov byte[calc],0
	.end0:
	; *** рисование буфера ***
	stdcall [buf2d_draw], buf_0
	; *** рисование подписи размера файла ***
	mov ecx,[sc.work_text]
	or  ecx,0x80000000 or (1 shl 30)
	mov edi,[sc.work] ;цвет фона окна
	mcall SF_DRAW_TEXT,(275 shl 16)+4,,txt_f_size
	add ebx,9
	mcall ,,,txt_img_w
	add ebx,9
	mcall ,,,txt_img_h
popad
	ret
endp

align 16
key:
	push eax ebx ecx
	mcall SF_GET_KEY
	cmp byte[view_b],1
	je .end1
	cmp ah,49 ;1
	jne @f
		mov edi,sel_pt
		call set_point_coords
		jmp .end0
	@@:
	cmp ah,50 ;2
	jne @f
		mov edi,sel_pt+sizeof.point2d
		call set_point_coords
		jmp .end0
	@@:
	cmp ah,51 ;3
	jne @f
		mov edi,sel_pt+sizeof.point2d*2
		call set_point_coords
		jmp .end0
	@@:
	cmp ah,52 ;4
	jne @f
		mov edi,sel_pt+sizeof.point2d*3
		call set_point_coords
		jmp .end0
	@@:
	.end1:

	cmp ah,178 ;Up
	jne @f
		call but_img_move_up
		jmp .end0
	@@:
	cmp ah,177 ;Down
	jne @f
		call but_img_move_down
		jmp .end0
	@@:
	cmp ah,176 ;Left
	jne @f
		call but_img_move_left
		jmp .end0
	@@:
	cmp ah,179 ;Right
	jne @f
		call but_img_move_right
		jmp .end0
	@@:
	cmp ah,104 ;H
	jne @f
		call set_user_line_h
		mov byte[calc],1
		jmp .end0
	@@:
	cmp ah,118 ;V
	jne @f
		call set_user_line_w
		mov byte[calc],1
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
		cmp ch,15 ;Ctrl+O
		jne @f
			call but_open_file
		@@:
		cmp ch,19 ;Ctrl+S
		jne @f
			call but_save_file
		@@:

	.end0:
	pop ecx ebx eax
	jmp still

;input:
; edi - указатель на структуру point2d
align 4
set_point_coords:
	call buf_get_mouse_coord
	cmp eax,-1
	je .end0
		sub eax,[nav_x]
		cmp eax,0
		jge @f
			xor eax,eax
		@@:
		cmp eax,[buf_i0.w]
		jle @f
			mov eax,[buf_i0.w]
		@@:
		mov [edi+point2d.x],eax
		sub ebx,[nav_y]
		cmp ebx,0
		jge @f
			xor ebx,ebx
		@@:
		cmp ebx,[buf_i0.h]
		jle @f
			mov ebx,[buf_i0.h]
		@@:
		mov [edi+point2d.y],ebx
		call points_update_prop
		mov byte[calc],1
	.end0:
	ret

align 16
button:
	mcall SF_GET_BUTTON
	cmp ah,3
	jne @f
		call but_open_file
		jmp still
	@@:
	cmp ah,4
	jne @f
		call but_save_file
		jmp still
	@@:
	cmp ah,5
	jne @f
		call but_2
		jmp still
	@@:
	cmp ah,6
	jne @f
		call but_3
		jmp still
	@@:
	cmp ah,7
	jne @f
		call but_about
		jmp still
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall [buf2d_delete],buf_i0
	;stdcall [buf2d_delete],buf_ogl
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_img]
	mcall SF_TERMINATE_PROCESS

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
	;bt eax,9
	;jnc @f
		;mouse r. but. press
		;call mouse_right_d
		;jmp .end_r
	;@@:
	;bt eax,1
	;jnc @f
		;mouse r. but. move
		;call mouse_right_m
		;jmp .end_r
	;@@:
	;.end_r:

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

;output:
; eax - buffer coord X (если курсор за буфером -1)
; ebx - buffer coord Y (если курсор за буфером -1)
align 4
proc nav_wnd_get_mouse_coord
	mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
	cmp ax,NAV_WND_T
	jl .no_buf ;не попали в окно буфера по оси y
	cmp eax,NAV_WND_L shl 16
	jl .no_buf ;не попали в окно буфера по оси x
	mov ebx,eax
	shr ebx,16

	and eax,0xffff ;оставляем координату y
	sub ax,NAV_WND_T
	cmp eax,[nav_wnd_h]
	jg .no_buf
	sub bx,NAV_WND_L
	cmp ebx,[nav_wnd_w]
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
mouse_left_d:
pushad
	call buf_get_mouse_coord
	cmp eax,-1
	je .end0
		mov [mouse_down_x],eax
		mov [mouse_down_y],ebx
		sub eax,[nav_x]
		sub ebx,[nav_y]
		
		mov edi,sel_pt
		xor ecx,ecx
		.cycle0:
			mov edx,[edi+point2d.x]
			sub edx,eax
			bt edx,31
			jnc @f
				neg edx
			@@:
			cmp edx,5 ;размер для выделения точки по оси x
			jg .end1
			mov edx,[edi+point2d.y]
			sub edx,ebx
			bt edx,31
			jnc @f
				neg edx
			@@:
			cmp edx,5 ;размер для выделения точки по оси y
			jg .end1
				mov [sel_act],ecx
				jmp .end0
			.end1:
			add edi,sizeof.point2d
			inc ecx
			cmp ecx,4
			jl .cycle0
			mov dword[sel_act],-1
		jmp .end2
	.end0:
	call nav_wnd_get_mouse_coord
	cmp eax,-1
	je .end2
		mov ecx,[nav_wnd_zoom]
		shl eax,cl
		shl ebx,cl
		stdcall nav_to_point, eax,ebx
		mov byte[calc],1
	.end2:
popad
	ret

align 4
proc mouse_left_u uses eax ebx
	call buf_get_mouse_coord
	cmp eax,-1
	je .end0
		sub [mouse_down_x],eax
		sub [mouse_down_y],ebx

		cmp dword[sel_act],-1
		je .end1
			;двигаем точки
			mov eax,[sel_act]
			imul eax,sizeof.point2d
			add eax,sel_pt
			;coord x
			mov ebx,dword[eax+point2d.x]
			sub ebx,[mouse_down_x]
			cmp ebx,0
			jge @f
				xor ebx,ebx
			@@:
			cmp ebx,[buf_i0.w]
			jle @f
				mov ebx,[buf_i0.w]
			@@:
			mov dword[eax+point2d.x],ebx
			;coord y
			mov ebx,dword[eax+point2d.y]
			sub ebx,[mouse_down_y]
			cmp ebx,0
			jge @f
				xor ebx,ebx
			@@:
			cmp ebx,[buf_i0.h]
			jle @f
				mov ebx,[buf_i0.h]
			@@:
			mov dword[eax+point2d.y],ebx
			call points_update_prop
			jmp .end2
		.end1:

		;двигаем изображение
		mov eax,[nav_y]
		sub eax,[mouse_down_y]
		call nav_y_corect
		mov [nav_y],eax

		mov eax,[nav_x]
		sub eax,[mouse_down_x]
		call nav_x_corect
		mov [nav_x],eax
	.end2:
		mov byte[calc],1
	.end0:
	ret
endp

align 4
set_user_line_h:
pushad
	call buf_get_mouse_coord
	cmp eax,-1
	je .end0
		mov [mouse_down_y],ebx
		sub ebx,[nav_y]
		cmp eax,[buf_i0.h]
		jle @f
			mov eax,[buf_i0.h]
		@@:
		cmp [u_line_h],ebx
		jne @f
			xor ebx,ebx ;line on/off
		@@:
		mov [u_line_h],ebx
	.end0:
popad
	ret

align 4
set_user_line_w:
pushad
	call buf_get_mouse_coord
	cmp eax,-1
	je .end0
		mov [mouse_down_x],eax
		sub eax,[nav_x]
		cmp eax,[buf_i0.w]
		jle @f
			mov eax,[buf_i0.w]
		@@:
		cmp [u_line_v],eax
		jne @f
			xor eax,eax ;line on/off
		@@:
		mov [u_line_v],eax
	.end0:
popad
	ret

align 4
proc but_new_file uses eax edi esi
	xor eax,eax
	mov [open_file_size],eax
	mov edi,[open_file_img]
	stosd
	ret
endp

align 4
open_file_img dd 0 ;указатель на память для открытия текстур
open_file_size dd 0 ;размер

;вычисление параметров для навигации по изображению
align 4
proc calc_nav_params uses eax ecx edi
	mov dword[nav_x_max],0
	mov edi,buf_0
	mov eax,buf2d_w
	mov [nav_sx],eax
	mov edi,buf_i0
	sub eax,buf2d_w
	bt eax,31
	jc @f
		mov [nav_x_max],eax
		xor eax,eax
	@@:
	mov [nav_x_min],eax
	mov edi,buf2d_w
	cmp [nav_sx],edi
	jle @f
		mov [nav_sx],edi
	@@:
	shr dword[nav_sx],1

	mov dword[nav_y_max],0
	mov edi,buf_0
	mov eax,buf2d_h
	mov [nav_sy],eax
	mov edi,buf_i0
	sub eax,buf2d_h
	bt eax,31
	jc @f
		mov [nav_y_max],eax
		xor eax,eax
	@@:
	mov [nav_y_min],eax
	mov edi,buf2d_h
	cmp [nav_sy],edi
	jle @f
		mov [nav_sy],edi
	@@:
	shr dword[nav_sy],1

	xor ecx,ecx
	mov [u_line_v],ecx
	mov eax,[buf_i0.w]
	@@:
		inc ecx
		shr eax,1
		cmp eax,100
		jg @b
	mov [nav_wnd_zoom],ecx
	xor ecx,ecx
	mov [u_line_h],ecx
	mov eax,[buf_i0.h]
	@@:
		inc ecx
		shr eax,1
		cmp eax,32
		jg @b
	cmp [nav_wnd_zoom],ecx
	jg @f
		mov [nav_wnd_zoom],ecx
	@@:
	mov ecx,[nav_wnd_zoom]
	mov eax,[buf_i0.w]
	shr eax,cl
	mov [nav_wnd_w],eax
	mov eax,[buf_i0.h]
	shr eax,cl
	mov [nav_wnd_h],eax
	ret
endp

align 4
proc but_open_file
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	pushad
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
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
	mov [open_file_size],ecx
	stdcall mem.ReAlloc,[open_file_img],ecx ;выделяем память для изображения
	mov [open_file_img],eax

	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	m2m [run_file_70.Count], dword[open_file_size]
	m2m [run_file_70.Buffer],dword[open_file_img]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70
	cmp ebx,0xffffffff
	je .end_0
		;определяем вид изображения
		stdcall [img_decode], [open_file_img],ebx,0
		or eax,eax
		jz .end_0 ;если нарушен формат файла
		mov ebx,eax
		
		mov ecx,[ebx+4] ;+4 = image width
		mov dword[buf_cop.w],ecx
		imul ecx,[ebx+8] ;+8 = image height
		lea ecx,[ecx+ecx*2]
		mov [open_file_size],ecx
		stdcall mem.ReAlloc,[open_file_img],ecx
		mov [open_file_img],eax
		mov dword[buf_cop],eax
		mov dword[buf_cop.l],0 ;left = 0, top = 0
		m2m dword[buf_cop.h],dword[ebx+8]

		;преобразуем изображение к формату rgb
		stdcall [img_to_rgb2], ebx,[open_file_img]
		mov edi,buf_i0
		cmp buf2d_data,0
		jne .end3
			stdcall getNextPowerOfTwo,[ebx+8]
			mov buf2d_h,eax
			mov edx,eax
			stdcall getNextPowerOfTwo,[ebx+4]
			mov buf2d_w,eax
			cmp edx,[ebx+8]
			jne @f
			cmp eax,[ebx+4]
			jne @f
				;создание нового изображения по исходным размерам
				stdcall [buf2d_create_f_img], edi,[open_file_img]
				jmp .end_1
			@@:
				;создание нового изображения по преобразованным размерам
				cmp eax,[ebx+4]
				jge @f
					mov eax,[ebx+4]
					mov buf2d_w,eax
				@@:
				sub eax,[ebx+4]
				shr eax,1
				mov esi,eax
				cmp edx,[ebx+8]
				jge @f
					mov edx,[ebx+8]
					mov buf2d_h,edx
				@@:
				sub edx,[ebx+8]
				shr edx,1
				stdcall [buf2d_create], edi
				mov [buf_cop.l],si
				mov [buf_cop.t],dx
				stdcall [buf2d_bit_blt], edi, esi,edx, buf_cop
				jmp .end_1
		.end3:
			;преобразование созданного изображения
			stdcall getNextPowerOfTwo,[ebx+4]
			cmp eax,[ebx+4]
			jg @f
				mov eax,[ebx+4]
			@@:
			mov ecx,eax ;размер по x
			stdcall getNextPowerOfTwo,[ebx+8]
			cmp eax,[ebx+8]
			jg @f
				mov eax,[ebx+8]
			@@:
			mov edx,eax ;размер по y
			stdcall [buf2d_resize], edi, ecx,edx,1 ;изменяем размеры буфера
			stdcall [buf2d_clear], edi, buf2d_color
			sub ecx,[ebx+4]
			shr ecx,1
			mov [buf_cop.l],cx
			sub edx,[ebx+8]
			shr edx,1
			mov [buf_cop.t],dx
			stdcall [buf2d_bit_blt], edi, ecx,edx, buf_cop
		.end_1:
		
		;создаем буфер для преобразованного изображения
		mov edi,buf_ogl
		mov eax,[buf_i0.w]
		mov buf2d_w,eax
		mov eax,[buf_i0.h]
		mov buf2d_h,eax
		cmp buf2d_data,0
		jne @f
			stdcall [kosglMakeCurrent], 0,35,buf2d_w,buf2d_h,ctx1
			stdcall [glEnable], GL_DEPTH_TEST
			stdcall [glEnable], GL_NORMALIZE ;делам нормали одинаковой величины во избежание артефактов
			stdcall [glShadeModel], GL_SMOOTH
			stdcall [glScalef], 2.0, -2.0, 1.0
			stdcall [glTranslatef], -0.5, -0.5, 0.0
			jmp .end_2
		@@:
			stdcall reshape, buf2d_w,buf2d_h ;изменяем размеры буфера buf_ogl
		.end_2:
		mov eax,dword[ctx1] ;eax -> TinyGLContext.GLContext
		mov eax,[eax] ;eax -> ZBuffer
		mov eax,[eax+ZBuffer.pbuf]
		mov buf2d_data,eax

		stdcall [buf2d_bit_blt], edi, 0,0, buf_i0 ;копируем изображение для востановления

		;* Setup texturing *
		stdcall [glTexEnvi], GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL
  
		;* generate texture object IDs *
		stdcall [glGenTextures], 1, TexObj
		stdcall [glBindTexture], GL_TEXTURE_2D, [TexObj]
		stdcall [glTexImage2D], GL_TEXTURE_2D, 0, 3, [buf_i0.w], [buf_i0.h],\
			0, GL_RGB, GL_UNSIGNED_BYTE, [buf_i0] ;делаем текстуру на основе буфера, само изображение в буфере может испортится при подгонке размеров текстуры

		stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST
		stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST
		stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT
		stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT
		stdcall [glBindTexture], GL_TEXTURE_2D, [TexObj]
		stdcall [glEnable], GL_TEXTURE_2D

		stdcall [buf2d_bit_blt], buf_i0, 0,0, edi ;востанавливаем изображение испорченое при установке текстуры

		;удаляем временный буфер в ebx
		stdcall [img_destroy], ebx
		
		movzx eax,word[buf_cop.l]
		movzx ebx,word[buf_cop.t]
		stdcall points_init_2,eax,ebx

		call calc_nav_params
		stdcall nav_to_point, dword[sel_pt+point2d.x],dword[sel_pt+point2d.y]
		mov byte[view_b],0
		mov dword[sel_act],-1 ;снимаем выделение с точек
		mov byte[calc],1
	.end_0:

	.end_open_file:
	popad
	ret
endp

align 4
proc nav_to_point, coord_x:dword, coord_y:dword
	;coord x
	mov eax,[nav_x_max]
	cmp eax,0
	jle @f
		;если маленькое изображение то ставим по центру
		shr eax,1
		jmp .end0
	@@:
	mov eax,[buf_0.w]
	shr eax,1
	sub eax,[coord_x]
	call nav_x_corect
	.end0:
	mov [nav_x],eax
	;coord y
	mov eax,[nav_y_max]
	cmp eax,0
	jle @f
		;если маленькое изображение то ставим по центру
		shr eax,1
		jmp .end1
	@@:
	mov eax,[buf_0.h]
	shr eax,1
	sub eax,[coord_y]
	call nav_y_corect
	.end1:
	mov [nav_y],eax
	ret
endp

;input:
; eax - navigation coord x
;output:
; eax - valid coord x
align 4
nav_x_corect:
	cmp eax,[nav_x_min]
	jge @f
		mov eax,[nav_x_min]
	@@:
	cmp eax,[nav_x_max]
	jle @f
		mov eax,[nav_x_max]
	@@:
	ret

;input:
; eax - navigation coord y
;output:
; eax - valid coord y
align 4
nav_y_corect:
	cmp eax,[nav_y_min]
	jge @f
		mov eax,[nav_y_min]
	@@:
	cmp eax,[nav_y_max]
	jle @f
		mov eax,[nav_y_max]
	@@:
	ret

align 4
proc getNextPowerOfTwo uses ebx, n:dword
	mov ebx,[n]
	mov eax,8 ;min size
	cmp ebx,eax
	jle .set
	@@:
		shl eax,1
		cmp ebx,eax
		jg @b
	cmp eax,4096 ;max size
	jle .set
		mov eax,4096
	.set:
	ret
endp

; new window size or exposure
align 4
proc reshape uses ebx ecx, width:dword, height:dword
	stdcall [glViewport], 0, 0, [width], [height]
	stdcall [glMatrixMode], GL_MODELVIEW
	stdcall [glLoadIdentity]
	stdcall [glScalef], 2.0, -2.0, 1.0
	stdcall [glTranslatef], -0.5, -0.5, 0.0
	ret
endp

align 4
proc but_save_file
locals
	png_data dd 0
	png_size dd 0
endl
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Set_file_ext],OpenDialog_data,Filter.1 ;.png
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],1
	jne .end_save_file
		;код при удачном открытии диалога
		mov dword[png_data],0

		;create image struct
		stdcall [img_create], [buf_ogl.w], [buf_ogl.h], Image.bpp24
		mov ebx,eax
		test eax,eax
		jz @f
			;copy foto to image buffer
			mov edi,[eax+Image.Data]
			mov esi,[buf_ogl]
			mov ecx,[buf_ogl.w]
			mov edx,[buf_ogl.h]
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
endp

align 4
but_2:
	xor byte[view_b],1
	mov byte[calc],1
	ret

align 4
but_3:
	xor byte[trans_a],1
	call points_update_prop
	mov byte[calc],1
	ret

align 4
but_about:
	notify_window_run txt_about
	ret

align 4
calc db 0 ;0 - не пересчитывать буфер, 1 - пересчитать и обновить буфер
view_b db 0 ;0 - исходный буфер, 1 - просмотр результата
trans_a db 0 ;0 - преобразовать по заданному размеру, 1 - преобразовать на весь буфер

align 4
proc but_img_move_up uses eax
	cmp dword[sel_act],-1
	je .end0
		mov eax,[sel_act]
		imul eax,sizeof.point2d
		add eax,sel_pt
		cmp dword[eax+point2d.y],0
		je .end2
		dec dword[eax+point2d.y]
		call points_update_prop
		jmp .end1
	.end0:
	mov eax,[nav_y]
	sub eax,[nav_sy]
	cmp eax,[nav_y_min]
	jge @f
		mov eax,[nav_y_min]
	@@:
	mov [nav_y],eax
	.end1:
	mov byte[calc],1
	.end2:
	ret
endp

align 4
proc but_img_move_down uses eax edi
	cmp dword[sel_act],-1
	je .end0
		mov eax,[sel_act]
		imul eax,sizeof.point2d
		add eax,sel_pt
		mov edi,buf_ogl
		mov edi,buf2d_h
		cmp dword[eax+point2d.y],edi
		jge .end2
		inc dword[eax+point2d.y]
		call points_update_prop
		jmp .end1
	.end0:
	mov eax,[nav_y]
	add eax,[nav_sy]
	cmp eax,[nav_y_max]
	jle @f
		mov eax,[nav_y_max]
	@@:
	mov [nav_y],eax
	.end1:
	mov byte[calc],1
	.end2:
	ret
endp

align 4
proc but_img_move_left uses eax
	cmp dword[sel_act],-1
	je .end0
		mov eax,[sel_act]
		imul eax,sizeof.point2d
		add eax,sel_pt
		cmp dword[eax+point2d.x],0
		je .end2
		dec dword[eax+point2d.x]
		call points_update_prop
		jmp .end1
	.end0:
	mov eax,[nav_x]
	sub eax,[nav_sx]
	cmp eax,[nav_x_min]
	jge @f
		mov eax,[nav_x_min]
	@@:
	mov [nav_x],eax
	.end1:
	mov byte[calc],1
	.end2:
	ret
endp

align 4
proc but_img_move_right uses eax edi
	cmp dword[sel_act],-1
	je .end0
		mov eax,[sel_act]
		imul eax,sizeof.point2d
		add eax,sel_pt
		mov edi,buf_ogl
		mov edi,buf2d_w
		cmp dword[eax+point2d.x],edi
		jge .end2
		inc dword[eax+point2d.x]
		call points_update_prop
		jmp .end1
	.end0:
	mov eax,[nav_x]
	add eax,[nav_sx]
	cmp eax,[nav_x_max]
	jle @f
		mov eax,[nav_x_max]
	@@:
	mov [nav_x],eax
	.end1:
	mov byte[calc],1
	.end2:
	ret
endp

;input:
; eax - число
; edi - буфер для строки
; len - длинна буфера
;output:
align 4
proc convert_int_to_str, len:dword
pushad
	mov esi,[len]
	add esi,edi
	dec esi
	bt eax,31
	jae @f
		;если число отрицательное
		neg eax
		mov byte[edi],'-'
		inc edi
	@@:
	call .str
popad
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
.1:
db 'PNG',0
db 'JPG',0
db 'JPEG',0
db 'BMP',0
db 'GIF',0
.end:
db 0

txt_space db ' ',0

if lang eq ru
	txt_err_save_img_file db 'Не могу сохранить *.png файл.',0
	txt_about db '"О программе',13,10,\
	'Данная программа сделана для преобразования изображений.',13,10,\
	'После открытия файла изображения нужно указать 4 точки,',13,10,\
	'которые станут углами преобразованого изображения." -tI',0
	txt_pref db ' б ',0,' Кб',0,' Мб',0,' Гб',0 ;приставки: кило, мега, гига
	txt_f_size db 'Размер: '
.size: rb 16
	txt_img_w db 'Ширина: '
.size: rb 16
	txt_img_h db 'Высота: '
else
	txt_err_save_img_file db 'Can',39,'t save *.png file.',0
	txt_about db '"About',13,10,\
	'This program is designed to convert images.',13,10,\
	'After opening the image file, you need to specify 4 points',13,10,\
	'that will become the corners of the converted image." -tI',0
	txt_pref db ' b ',0,' Kb',0,' Mb',0,' Gb',0 ;приставки: кило, мега, гига
	txt_f_size db 'Size: '
.size: rb 16
	txt_img_w db 'Width: '
.size: rb 16
	txt_img_h db 'Height: '
end if
.size: rb 16


system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'tinygl.obj',0

align 4
l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, library_path, system_dir_2, import_buf2d
	lib_3 l_libs lib_name_3, library_path, system_dir_3, import_lib_tinygl
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
	dd 0,0
	sz_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
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

align 4
import_lib_tinygl:

macro E_LIB n
{
	n dd sz_#n
}
include '../../../programs/develop/libraries/TinyGL/asm_fork/export.inc'
	dd 0,0
macro E_LIB n
{
	sz_#n db `n,0
}
include '../../../programs/develop/libraries/TinyGL/asm_fork/export.inc'

align 4
buf_0: dd 0
.l: dw 5 ;+4 left
.t: dw 35 ;+6 top
.w: dd 6*64 ;+8 w
.h: dd 7*64 ;+12 h
.color: dd 0x808080 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_font:
	dd 0 ;указатель на буфер изображения
	dd 0 ;+4 left,top
.w: dd 0
.h: dd 0,0,24

align 4
buf_cop: ;буфер для копирования текстуры
	dd 0
.l: dw 0 ;+4 left
.t: dw 0 ;+6 top
.w: dd 0
.h: dd 0,0,24

;исходное изображение
align 4
buf_i0: dd 0,0
.w: dd 0
.h: dd 0
.color: dd 0,24

;преобразованое изображение
align 4
buf_ogl: dd 0,0
.w: dd 0
.h: dd 0
.color: dd 0,24

align 16
i_end:
ctx1 rb 28 ;sizeof.TinyGLContext = 28
TexObj dd 0 ;массив указателей на текстуры (в данном случае 1 шт.)
nav_x_min dd 0 ;мин. коорд. x для навигации
nav_y_min dd 0 ;мин. коорд. y для навигации
nav_x_max dd 0 ;макс. коорд. x
nav_y_max dd 0 ;макс. коорд. y
nav_x dd 0 ;текущ. коорд. x для навигации
nav_y dd 0 ;текущ. коорд. y для навигации
nav_sx dd 0 ;скрол по x
nav_sy dd 0 ;скрол по y
nav_wnd_w dd 0 ;ширина окна навигации
nav_wnd_h dd 0 ;высоата окна навигации
nav_wnd_zoom dd 0
mouse_down_x dd ?
mouse_down_y dd ?
sel_act dd ? ;точка выбранная для редактирования с клавиатуры
sel_pt rb 8*sizeof.point2d ;точки для выбора 4-х углов
last_time dd 0
u_line_v dd 0 ;вертикальная линия
u_line_h dd 0 ;горизонтальная линия
txt_buf rb 8
procinfo process_information 
sc system_colors 
run_file_70 FileInfoBlock
		rb 4096
align 16
stacktop:
	sys_path rb 1024
	file_name rb 1024 ;4096
	library_path rb 1024
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:
