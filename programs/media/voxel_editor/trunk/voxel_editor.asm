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

include 'd:/kolibri/svn/programs/macros.inc'
include 'd:/kolibri/svn/programs/proc32.inc'
include 'd:/kolibri/svn/programs/develop/libraries/box_lib/load_lib.mac'
include 'mem.inc'
include 'dll.inc'
include 'vox_draw.inc'

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
caption db 'Voxel editor 11.06.12',0 ;подпись окна

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
IMAGE_TOOLBAR_SIZE equ IMAGE_TOOLBAR_ICON_SIZE*14
image_data_toolbar dd 0

max_open_file_size equ 64*1024 ;64 Kb


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
TILE_SIZE equ 10 ;размер квадратика на плоскости с сечением
OT_CAPT_X_COLOR equ  5 ;отступ для подписи цвета
OT_CAPT_Y_COLOR equ 30

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

	stdcall [buf2d_create], buf_0 ;создание буфера изображения
	stdcall [buf2d_create], buf_0z ;создание буфера глубины
	stdcall [buf2d_create], buf_pl ;создание буфера для сечения

	stdcall [buf2d_vox_brush_create], buf_vox, vox_6_7_z

	load_image_file fn_toolbar, image_data_toolbar,IMAGE_TOOLBAR_SIZE

	stdcall mem.Alloc,max_open_file_size
	mov dword[open_file_vox],eax

	call but_new_file

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

		cmp dword[v_pen_mode],2 ;select color
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
		stdcall get_mouse_ev_scale, buf_vox, ebx, eax, [scaled_zoom],edx
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
			mov ecx,TILE_SIZE ;H
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
				mov ecx,TILE_SIZE ;W
				div ecx
				mov [n_plane],eax
				jmp .end_1
			.end_0:
			mov [v_cur_y],eax ;Y-coord
			sub ebx,edx
			mov eax,ebx
			xor edx,edx
			mov ecx,TILE_SIZE ;W
			div ecx
			mov [v_cur_x],eax ;X-coord

			cmp dword[v_pen_mode],0
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

				;вызов общих параметров для функций
				push dword[v_zoom]
				push edx
				push ebx
				push eax
				push dword[open_file_vox]

				;вызов функций
				cmp dword[v_pen_mode],1
				jne @f
					call buf2d_vox_obj_create_node
					;stdcall buf2d_vox_obj_create_node, [open_file_vox], eax,ebx,edx, [v_zoom], [v_color]
					jmp .end_1
				@@:
					call buf2d_vox_obj_delete_node
					;stdcall buf2d_vox_obj_delete_node, [open_file_vox], eax,ebx,edx, [v_zoom]

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
		;stdcall [buf2d_filled_rect_by_size], buf_0, 5,3, 8,8,eax
		;stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
		mov ebx,((OT_CAPT_X_COLOR+35) shl 16)+16 ;по оси x
		mov ecx,(OT_CAPT_Y_COLOR shl 16)+12 ;по оси y
		mov edx,[v_color]
		mcall 13
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
	xor eax,eax
	mov ebx,(20 shl 16)+550
	mov ecx,(20 shl 16)+415
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x30000000
	mov edi,caption
	int 0x40

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
	add edx,(30 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40

	; *** рисование буферов ***
	call draw_objects
	call draw_pok

	mcall 12,2
popad
	ret

align 4
draw_pok:
	mov eax,4 ;рисование текста
	mov ebx,(365 shl 16)+5
	mov ecx,[sc.work_text]
	or  ecx,0x80000000 ;or (1 shl 30)
	mov edx,txt_zoom
	;mov edi,[sc.work]
	int 0x40
	add bx,9
	mov edx,txt_cur_x
	int 0x40
	add bx,9
	mov edx,txt_cur_y
	int 0x40
	add bx,9
	mov edx,txt_n_plane
	int 0x40

	mov eax,47
	mov ecx,[v_zoom]
	mov ebx,(2 shl 16)
	mov edx,((365+6*9) shl 16)+5
	mov esi,[sc.work_button_text]
	or  esi,(1 shl 30)
	mov edi,[sc.work_button]
	int 0x40 ;масштаб
	mov ebx,(5 shl 16)
	mov ecx,[v_cur_x]
	add edx,(6*2)*65536+9
	int 0x40 ;
	mov ebx,(5 shl 16)
	mov ecx,[v_cur_y]
	add edx,(6*0)*65536+9
	int 0x40 ;
	mov ebx,(5 shl 16)
	mov ecx,[n_plane]
	add edx,(6*0)*65536+9
	int 0x40 ;

	mov eax,4 ;рисование текста
	mov ebx,(OT_CAPT_X_COLOR shl 16)+OT_CAPT_Y_COLOR+2
	mov ecx,[sc.work_text]
	or  ecx,0x80000000 ;or (1 shl 30)
	mov edx,txt_color
	int 0x40

	mov ebx,((OT_CAPT_X_COLOR+35) shl 16)+16 ;по оси x
	mov ecx,(OT_CAPT_Y_COLOR shl 16)+12 ;по оси y
	mov edx,[v_color]
	mcall 13
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
		call but_1
	@@:
	cmp ah,7
	jne @f
		call but_2
	@@:
	cmp ah,8
	jne @f
		call but_3
	@@:
	cmp ah,9
	jne @f
		call but_4
	@@:
	cmp ah,10
	jne @f
		call but_5
	@@:
	cmp ah,11
	jne @f
		call but_6
	@@:
	cmp ah,12
	jne @f
		call but_7
	@@:
	cmp ah,13
	jne @f
		call but_8
	@@:
	cmp ah,14
	jne @f
		mov dword[v_pen_mode],2 ;select color
		call draw_palete
	@@:
	cmp ah,15
	jne @f
		call but_light
	@@:
	cmp ah,16
	jne @f
		call but_rend_2_2
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
	stdcall [buf2d_vox_brush_delete], buf_vox
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_vox]
	mcall -1

align 4
vox_new_data:
	db 2,0,0,0
	db 000b,001b,010b,011b, 100b,101b,110b,111b ;default table
	dd 0 ;null node

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
	mov dword[run_file_70.Count], max_open_file_size
	m2m [run_file_70.Buffer], [open_file_vox]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je .end_open_file

	add ebx,[open_file_vox]
	mov byte[ebx],0 ;на случай если ранее был открыт файл большего размера чистим конец буфера с файлом
	mcall 71,1,openfile_path

	;---
	;
	mov eax,[open_file_vox]
	movzx eax,byte[eax]
	and eax,0xff ;берем масштаб по умолчанию
	mov dword[v_zoom],eax ;берем масштаб по умолчанию
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

;увеличение масштаба
align 4
but_1:
	cmp dword[v_zoom],10 ;максимальный размер, до которого можно увеличить 2^10=1024
	jge @f
		inc dword[v_zoom]
		shl dword[n_plane],1
		push eax
		mov eax,[v_zoom]
		cmp eax,[scaled_zoom]
		jl .end_0
			shl dword[cam_x],1
			shl dword[cam_y],1
			shl dword[cam_z],1
		.end_0:
		pop eax
		call draw_objects
		call draw_pok
	@@:
	ret

;уменьшение масштаба
align 4
but_2:
	cmp dword[v_zoom],-1
	jl @f
		dec dword[v_zoom]
		shr dword[n_plane],1
		push eax
		mov eax,[v_zoom]
		cmp eax,[scaled_zoom]
		jl .end_0
			shr dword[cam_x],1
			shr dword[cam_y],1
			shr dword[cam_z],1
		.end_0:
		pop eax
		call draw_objects
		call draw_pok
	@@:
	ret

align 4
but_3:
	stdcall vox_obj_rot_z, [open_file_vox]
	call draw_objects
	ret

align 4
but_4:
	stdcall vox_obj_rot_x, [open_file_vox]
	call draw_objects
	ret

align 4
but_5:
	inc dword[n_plane]
	call draw_objects
	call draw_pok
	ret

align 4
but_6:
	cmp dword[n_plane],0
	jle @f
		dec dword[n_plane]
		call draw_objects
		call draw_pok
	@@:
	ret

align 4
but_7:
	push eax
	mov eax,dword[v_pen_mode]
	mov dword[v_pen_mode],1 ;draw
	cmp eax,2
	jne @f
		call draw_objects
	@@:
	pop eax
	ret

align 4
but_8:
	push eax
	mov eax,dword[v_pen_mode]
	mov dword[v_pen_mode],0 ;clear
	cmp eax,2
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
			stdcall [buf2d_vox_obj_draw_3g_scaled], buf_r_img, buf_r_z, buf_vox, [open_file_vox], 0,0, 0, ebx, [cam_x],[cam_y],[cam_z],eax, 0xd080d0
			bt dword[mode_light],0
			jnc @f
				stdcall [buf2d_vox_obj_draw_3g_shadows], buf_r_img, buf_r_z, buf_vox, 0,0, 0, ebx, 3
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
		stdcall [buf2d_vox_obj_draw_3g], buf_r_img, buf_r_z, buf_vox, [open_file_vox], 0,0, 0, eax
		bt dword[mode_light],0
		jnc @f
			stdcall [buf2d_vox_obj_draw_3g_shadows], buf_r_img, buf_r_z, buf_vox, 0,0, 0, eax, 3
		@@:

		mov edi,buf_r_img
		stdcall [buf2d_img_hdiv2], edi
		shr buf2d_h,1
		stdcall [buf2d_img_wdiv2], edi
		shr buf2d_w,1

		stdcall [buf2d_vox_obj_get_img_w_3g], buf_vox,[v_zoom]
		mov ebx,[buf_0.w]
		sub ebx,eax
		shr ebx,1 ;ebx - для центровки маленьких изображений по горизонтали
		stdcall [buf2d_vox_obj_get_img_h_3g], buf_vox,[v_zoom]
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

align 4
draw_palete:
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
	stdcall buf2d_draw_palete, buf_0, 5,3, 9,6, 18, 512
	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	ret

v_zoom dd 3 ;текущий масштаб
v_cur_x dd 0 ;координата курсора x
v_cur_y dd 0 ;координата курсора y (но ось в объекте z)
n_plane dd 0 ;плоскость сечения
v_color dd 0xff ;цвет карандаша
v_pen_mode dd 1 ;режим: 0-стирания, 1-рисования
mode_light dd 1 ;режим освещения
cam_x dd 0
cam_y dd 0
cam_z dd 0
scaled_zoom dd 5 ;масштаб после которого начинается рисование части изображения

txt_zoom db 'Масштаб:',0
txt_cur_x db 'x:',0
txt_cur_y db 'y:',0
txt_n_plane db 'Сечение:',0
txt_color db 'Цвет:',0

align 4
draw_objects:
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
	stdcall [buf2d_clear], buf_0z, 0 ;чистим буфер
	stdcall [buf2d_clear], buf_pl, [buf_pl.color] ;чистим буфер

	cmp dword[v_pen_mode],2
	jne @f
		call draw_palete
		jmp .end_f
	@@:
	push eax ebx ecx
	stdcall [buf2d_vox_obj_get_img_w_3g], buf_vox,[v_zoom]
	mov ebx,[buf_0.w]
	sub ebx,eax
	shr ebx,1 ;ebx - для центровки маленьких изображений по горизонтали

	xor ecx,ecx
	stdcall [buf2d_vox_obj_get_img_h_3g], buf_vox,[v_zoom]
	cmp eax,[buf_0.h]
	jg @f
		mov ecx,[buf_0.h]
		sub ecx,eax
		shr ecx,1 ;ecx - для центровки маленьких изображений по
	@@:

	mov eax,[v_zoom]
	cmp eax,[scaled_zoom]
	jg @f
		;обычный режим изображения
		stdcall [buf2d_vox_obj_draw_3g], buf_0, buf_0z, buf_vox, [open_file_vox], ebx,ecx, 0, eax
		stdcall [buf2d_vox_obj_draw_pl], buf_pl, [open_file_vox], OT_MAP_X,OT_MAP_Y,TILE_SIZE, [v_zoom], [n_plane], 0xd0d0d0
		bt dword[mode_light],0
		jnc .end_1
			stdcall [buf2d_vox_obj_draw_3g_shadows], buf_0, buf_0z, buf_vox, ebx,ecx, 0, eax, 3
		.end_1:
		jmp .end_0
	@@:
		;режим масштабирования изображения
		sub eax,[scaled_zoom]
		stdcall [buf2d_vox_obj_draw_3g_scaled], buf_0, buf_0z, buf_vox,\
			[open_file_vox], 0,0, 0, [scaled_zoom], [cam_x],[cam_y],[cam_z],eax, [sc.work_graph] ;scroll -> 2^eax
		stdcall [buf2d_vox_obj_draw_pl_scaled], buf_pl, [open_file_vox],\
			OT_MAP_X,OT_MAP_Y,TILE_SIZE, [scaled_zoom], [n_plane], [sc.work_graph],[cam_x],[cam_y],[cam_z],eax
		bt dword[mode_light],0
		jnc .end_2
			stdcall [buf2d_vox_obj_draw_3g_shadows], buf_0, buf_0z, buf_vox, 0,0, 0, [scaled_zoom], 3
		.end_2:
	.end_0:

	pop ecx ebx eax

	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	stdcall [buf2d_draw], buf_pl ;обновляем буфер на экране
	.end_f:
	ret

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

l_libs_start:
	lib0 l_libs lib_name_0, sys_path, file_name, system_dir_0,\
		err_message_found_lib_0, head_f_l, proclib_import,err_message_import_0, head_f_i
	lib1 l_libs lib_name_1, sys_path, file_name, system_dir_1,\
		err_message_found_lib_1, head_f_l, import_libimg, err_message_import_1, head_f_i
	lib_2 l_libs lib_name_2, sys_path, library_path, system_dir_2,\
		err_msg_found_lib_2,head_f_l,import_buf2d,err_msg_import_2,head_f_i
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
	buf2d_vox_obj_draw_3g dd sz_buf2d_vox_obj_draw_3g
	buf2d_vox_obj_draw_3g_scaled dd sz_buf2d_vox_obj_draw_3g_scaled
	buf2d_vox_obj_draw_pl dd sz_buf2d_vox_obj_draw_pl
	buf2d_vox_obj_draw_pl_scaled dd sz_buf2d_vox_obj_draw_pl_scaled
	buf2d_vox_obj_draw_3g_shadows dd sz_buf2d_vox_obj_draw_3g_shadows
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
	sz_buf2d_vox_obj_draw_3g db 'buf2d_vox_obj_draw_3g',0
	sz_buf2d_vox_obj_draw_3g_scaled db 'buf2d_vox_obj_draw_3g_scaled',0
	sz_buf2d_vox_obj_draw_pl db 'buf2d_vox_obj_draw_pl',0
	sz_buf2d_vox_obj_draw_pl_scaled db 'buf2d_vox_obj_draw_pl_scaled',0
	sz_buf2d_vox_obj_draw_3g_shadows db 'buf2d_vox_obj_draw_3g_shadows',0

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

;данные для создания минимального единичного вокселя
align 4
vox_6_7_z:
dd 0,0,1,1,0,0,\
   0,2,2,2,2,0,\
   2,2,2,2,2,2,\
   2,3,2,2,3,2,\
   2,3,3,3,3,2,\
   0,3,3,3,3,0,\
   0,0,3,3,0,0

align 4
buf_vox:
	db 6,7,4,3 ;w,h,h_osn,n
	rb BUF_STRUCT_SIZE*(2+1)


i_end:
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
