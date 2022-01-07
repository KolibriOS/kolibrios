use32
	org 0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 1, start, i_end, mem, stacktop, file_name, sys_path

version_edit equ 0

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'
include '../../load_img.inc'
include '../../load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../develop/libraries/TinyGL/asm_fork/opengl_const.inc'
include 'lang.inc'
include 'info_fun_float.inc'
include 'info_menu.inc'
include 'data.inc'
include 'convert_stl_3ds.inc'

3d_wnd_l equ 205 ;отступ для tinygl буфера слева
3d_wnd_t equ  47 ;отступ для tinygl буфера сверху
3d_wnd_w equ 344
3d_wnd_h equ 312

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

ID_ICON_CHUNK_MAIN equ 0 ;иконка главного блока
ID_ICON_CHUNK_NOT_FOUND equ 1 ;иконка не известного блока
ID_ICON_DATA equ 2 ;иконка для данных блока, не определенной структуры
ID_ICON_POINT equ 8
ID_ICON_POINT_SEL equ 9

FILE_ERROR_CHUNK_SIZE equ -3 ;ошибка в размере блока

size_one_list equ 42+sizeof.obj_3d
list_offs_chunk_del equ 8 ;может ли блок удалятся
list_offs_chunk_lev equ 9 ;уровень блока (прописан в данные узла)
list_offs_p_data equ 10 ;указатель на подпись блока
list_offs_obj3d equ 14 ;указатель на структуру данных для 3d объекта
list_offs_text equ 14+sizeof.obj_3d ;сдвиг начала текста в листе
include 'info_o3d.inc'

align 4
fl255 dd 255.0
open_file_data dd 0 ;указатель на память для открытия файлов 3ds
open_file_size dd 0 ;размер открытого файла

;
main_wnd_height equ 460 ;высота главного окна программы
fn_toolbar db 'toolbar.png',0
IMAGE_TOOLBAR_ICON_SIZE equ 21*21*3
image_data_toolbar dd 0
;
icon_tl_sys dd 0 ;указатель на память для хранения системных иконок
icon_toolbar dd 0 ;указатель на память для хранения иконок объектов

;--------------------------------------
level_stack dd 0
offs_last_timer dd 0 ;последний сдвиг показаный в функции таймера

align 4
file_3ds: ;переменные используемые при открытии файла
.offs: dd 0 ;+0 указатель на начало блока
.size: dd 0 ;+4 размер блока (для 1-го параметра = размер файла 3ds)
rb 8*MAX_FILE_LEVEL

buffer rb size_one_list ;буфер для добавления структур в список tree1

txt_3ds_symb db 0,0
;--------------------------------------



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
	;проверка на сколько удачно загузились библиотеки
	mov	ebp,lib_0
	.test_lib_open:
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS ;exit not correct
	@@:
	add ebp,ll_struc_size
	cmp ebp,l_libs_end
	jl .test_lib_open
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0xC0000027

	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

	;kmenu initialisation
	stdcall [kmenu_init],sc
	stdcall [ksubmenu_new]
	mov [main_menu], eax

	stdcall [ksubmenu_new]
	mov [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Vertexes, 5
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Faces, 6
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Faces_Fill, 7
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Faces_Mat, 8
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Light, 9
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Smooth, 10
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_SEPARATOR, 0, 0
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_NORMAL, sz_main_menu_Veiw_Reset, 11
	stdcall [ksubmenu_add], [main_menu_view], eax
	stdcall [kmenuitem_new], KMENUITEM_SUBMENU, sz_main_menu_View, [main_menu_view]
	stdcall [ksubmenu_add], [main_menu], eax

	mov dword[w_scr_t1.type],1
	stdcall [tl_data_init], tree1
	;системные иконки 16*16 для tree_list
	include_image_file 'tl_sys_16.png', icon_tl_sys
	;если изображение не открылось, то в icon_tl_sys будут
	;не инициализированные данные, но ошибки не будет, т. к. буфер нужного размера
	mov eax,dword[icon_tl_sys]
	mov dword[tree1.data_img_sys],eax

	load_image_file 'objects.png', icon_toolbar
	mov eax,dword[icon_toolbar]
	mov dword[tree1.data_img],eax

	load_image_file 'font8x9.bmp', image_data_toolbar
	stdcall [buf2d_create_f_img], buf_1,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память
	stdcall [buf2d_conv_24_to_8], buf_1,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_1

	load_image_file fn_toolbar, image_data_toolbar

	;работа с файлом настроек
	copy_path ini_name,sys_path,file_name,0
	mov dword[def_dr_mode],0
	stdcall [ini_get_int],file_name,ini_sec_w3d,key_dv,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_vertexes
	@@:
	stdcall [ini_get_int],file_name,ini_sec_w3d,key_df,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_faces
	@@:
	stdcall [ini_get_int],file_name,ini_sec_w3d,key_dff,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_faces_fill
	@@:
	stdcall [ini_get_int],file_name,ini_sec_w3d,key_dfm,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_faces_mat
	@@:
	stdcall [ini_get_int],file_name,ini_sec_w3d,key_dl,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_light
	@@:
	stdcall [ini_get_int],file_name,ini_sec_w3d,key_ds,1
	or eax,eax
	jz @f
		or dword[def_dr_mode], 1 shl bit_smooth
	@@:
	stdcall [ini_get_color],file_name,ini_sec_w3d,key_ox,0x0000ff
	mov [color_ox],eax
	stdcall [ini_get_color],file_name,ini_sec_w3d,key_oy,0xff0000
	mov [color_oy],eax
	stdcall [ini_get_color],file_name,ini_sec_w3d,key_oz,0x00ff00
	mov [color_oz],eax
	stdcall [ini_get_color],file_name,ini_sec_w3d,key_bk,0x000000
	mov [color_bk],eax
	shr eax,8
	mov [color_bk+4],eax
	shr eax,8
	mov [color_bk+8],eax
	stdcall [ini_get_color],file_name,ini_sec_w3d,key_vert,0xffffff
	mov [color_vert],eax
	stdcall [ini_get_color],file_name,ini_sec_w3d,key_face,0x808080
	mov [color_face],eax
	stdcall [ini_get_color],file_name,ini_sec_w3d,key_select,0xffff00
	mov [color_select],eax
	finit
	fild dword[color_bk+8]
	fdiv dword[fl255]
	fstp dword[color_bk+8]
	mov eax,[color_bk+4]
	and eax,0xff
	mov [color_bk+4],eax
	fild dword[color_bk+4]
	fdiv dword[fl255]
	fstp dword[color_bk+4]
	mov eax,[color_bk]
	and eax,0xff
	mov [color_bk],eax
	fild dword[color_bk]
	fdiv dword[fl255]
	fstp dword[color_bk]

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	stdcall [kosglMakeCurrent], 3d_wnd_l,3d_wnd_t,3d_wnd_w,3d_wnd_h,ctx1
	stdcall [glEnable], GL_DEPTH_TEST
	stdcall [glEnable], GL_NORMALIZE ;делам нормали одинаковой величины во избежание артефактов
	stdcall [glClearColor], [color_bk+8],[color_bk+4],[color_bk],0.0
	stdcall [glShadeModel], GL_SMOOTH
	call [gluNewQuadric]
	mov [qObj],eax

	mov eax,dword[ctx1] ;eax -> TinyGLContext.GLContext
	mov eax,[eax] ;eax -> ZBuffer
	mov eax,[eax+ZBuffer.pbuf]
	mov dword[buf_ogl],eax

	;open file from cmd line
	cmp dword[openfile_path],0
	je @f
		call but_open_file.no_dlg
	@@:
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
	or eax,eax
	jz timer_funct

	cmp al,1
	jne @f
		call draw_window
		jmp still
	@@:
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jne @f
		mcall SF_THREAD_INFO,procinfo,-1
		cmp ax,word[procinfo+4]
		jne @f ;окно не активно
		call mouse
	@@:
	jmp still

align 4
mouse:
	push eax ebx
	mcall SF_MOUSE_GET,SSF_BUTTON_EXT
	bt eax,0
	jnc .end_m
		;mouse l. but. move
		cmp dword[mouse_drag],1
		jne .end_m

		stdcall [tl_node_get_data],tree1
		or eax,eax
		jz .end_d
			mov ebx,[eax]
			add ebx,dword[open_file_data] ;получаем значение сдвига в памяти
			cmp word[ebx],CHUNK_OBJBLOCK
			jne .end_d

		mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
		mov ebx,eax
		sar ebx,16 ;mouse.x
		cmp ebx,3d_wnd_l
		jg @f
			mov ebx,3d_wnd_l
		@@:
		sub ebx,3d_wnd_l
		cmp ebx,3d_wnd_w
		jle @f
			mov ebx,3d_wnd_w
		@@:
		movsx eax,ax ;mouse.y
		cmp eax,3d_wnd_t
		jg @f
			mov eax,3d_wnd_t
		@@:
		sub eax,3d_wnd_t
		cmp eax,3d_wnd_h
		jle @f
			mov eax,3d_wnd_h
		@@:
		finit
		fild dword[mouse_y]
		mov [mouse_y],eax
		fisub dword[mouse_y]
		fdiv dword[angle_dxm] ;если курсор движется по оси y (вверх или вниз) то поворот делаем вокруг оси x
		fadd dword[angle_x]
		fstp dword[angle_x]

		fild dword[mouse_x]
		mov [mouse_x],ebx
		fisub dword[mouse_x]
		fdiv dword[angle_dym] ;если курсор движется по оси x (вверх или вниз) то поворот делаем вокруг оси y
		fadd dword[angle_y]
		fstp dword[angle_y]

		stdcall [tl_node_get_data],tree1
		;cmp eax,0 - не надо, было сделано выше
		add eax,list_offs_obj3d
		stdcall draw_3d, eax
		jmp .end_d
	.end_m:
	bt eax,16
	jnc @f
		;mouse l. but. up
		mov dword[mouse_drag],0
		jmp .end_d
	@@:
	bt eax,8
	jnc .end_d
		;mouse l. but. press
		mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
		mov ebx,eax
		sar ebx,16 ;mouse.x
		cmp ebx,3d_wnd_l
		jl .end_d
		sub ebx,3d_wnd_l
		cmp ebx,3d_wnd_w
		jg .end_d
		movsx eax,ax ;mouse.y
		cmp eax,3d_wnd_t
		jl .end_d
		sub eax,3d_wnd_t
		cmp eax,3d_wnd_h
		jg .end_d
		mov dword[mouse_drag],1
		mov dword[mouse_x],ebx
		mov dword[mouse_y],eax
	.end_d:

	stdcall [tl_mouse], tree1
	stdcall [kmainmenu_dispatch_cursorevent], [main_menu]
	pop ebx eax
	ret

align 4
timer_funct:
	pushad
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	;просматриваем выделенный блок данных
	stdcall [tl_node_get_data],tree1
	or eax,eax
	jz .end_f
		lea edi,[eax+list_offs_obj3d]
		mov ebx,eax
		mov eax,[ebx]
		mov ecx,[ebx+4] ;размер блока
		stdcall hex_in_str, txt_3ds_offs.dig, eax,8
		stdcall hex_in_str, txt_3ds_offs.siz, ecx,8

		add eax,[open_file_data] ;получаем значение сдвига в памяти
		cmp [offs_last_timer],eax
		je .end_f
			;если выделенный блок данных не совпадает с последним запомненным
			mov [offs_last_timer],eax

			cmp word[eax],CHUNK_OBJBLOCK
			jne .end_oblo
			cmp dword[edi+obj_3d.poi_count],2
			jl .ini_oblo
				stdcall draw_3d,edi
				jmp .end_f
			.ini_oblo:
				stdcall obj_init,edi ;попытка настроить переменные объекта
				cmp dword[edi+obj_3d.poi_count],2
				jl .end_f
					call mnu_reset_settings ;сброс углов поворота и режимов рисования
				jmp .end_f
			.end_oblo:

			cmp word[eax],CHUNK_MATERIAL
			jne .end_mblo
			cmp dword[edi+material.name],0
			je .ini_mblo
				stdcall draw_material,edi
				jmp .end_f
			.ini_mblo:
				stdcall mat_init,edi,eax ;попытка настроить данные материала
				cmp dword[edi+material.name],0
				je .end_f
					stdcall draw_material,edi
				jmp .end_f
			.end_mblo:

			stdcall buf_draw_beg, buf_ogl
			stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_3ds_offs,5,35,0xb000
			mov edx,[ebx+list_offs_p_data]
			or edx,edx ;смотрим есть ли описание блока
			jz .no_info
				stdcall [buf2d_draw_text], buf_ogl, buf_1,edx,5,45,0xb000
			.no_info:
			stdcall [buf2d_draw], buf_ogl ;обновляем буфер на экране
	.end_f:
	popad
	jmp still

align 4
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW
	mov edx,[sc.work]
	or  edx,0x33000000
	mcall SF_CREATE_WINDOW, (20 shl 16)+560, (20 shl 16)+main_wnd_height,,, capt

	mcall SF_THREAD_INFO,procinfo,-1
	mov eax,dword[procinfo.box.height]
	cmp eax,250
	jge @f
		mov eax,250
	@@:
	sub eax,30
	sub eax,[tree1.box_top]
	mov [tree1.box_height],eax
	mov word[w_scr_t1.y_size],ax ;новые размеры скроллинга

	stdcall [kmainmenu_draw], [main_menu]

	mov esi,[sc.work_button]
	mcall SF_DEFINE_BUTTON,(5 shl 16)+20,(24 shl 16)+20,0x40000003
	mcall ,(30 shl 16)+20,,0x40000004 ;open
	mcall ,(3d_wnd_l shl 16)+20,,0x40000005 ;вершины вкл.
	mcall ,((3d_wnd_l+25) shl 16)+20,,0x40000006 ;каркасные грани вкл.
	mcall ,((3d_wnd_l+50) shl 16)+20,,0x40000007 ;заливка граней вкл.
	mcall ,((3d_wnd_l+75) shl 16)+20,,0x40000008 ;грани по материалам вкл.
	mcall ,((3d_wnd_l+100) shl 16)+20,,0x40000009 ;свет вкл./выкл.
	mcall ,((3d_wnd_l+125) shl 16)+20,,0x4000000a ;сглаживание
	mcall ,((3d_wnd_l+150) shl 16)+20,,0x4000000b ;сброс
	mcall ,((3d_wnd_l+175) shl 16)+20,,0x4000000c ;скрин из 3d окна

	mcall SF_PUT_IMAGE,[image_data_toolbar],(21 shl 16)+21,(5 shl 16)+24 ;new
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,(30 shl 16)+24 ;open
	add ebx,IMAGE_TOOLBAR_ICON_SIZE*6
	mcall ,,,((3d_wnd_l) shl 16)+24 ;вершины вкл.
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,((3d_wnd_l+25) shl 16)+24 ;каркасные грани вкл.
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,((3d_wnd_l+50) shl 16)+24 ;заливка граней вкл.
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,((3d_wnd_l+100) shl 16)+24 ;свет вкл./выкл.
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,((3d_wnd_l+150) shl 16)+24 ;сброс
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,((3d_wnd_l+75) shl 16)+24 ;грани по материалам вкл.
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,((3d_wnd_l+125) shl 16)+24 ;сглаживание
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,((3d_wnd_l+175) shl 16)+24 ;скрин из 3d окна

	mov dword[w_scr_t1.all_redraw],1
	stdcall [tl_draw], tree1
	stdcall [buf2d_draw], buf_ogl

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
key:
	mcall SF_GET_KEY

	cmp dword[el_focus], tree1
	jne @f
		stdcall [tl_key], tree1
		jmp .end
	@@:

	cmp ah,178 ;Up
	jne @f
		fld dword[angle_x]
		fadd dword[delt_size]
		fstp dword[angle_x]
		stdcall [tl_node_get_data],tree1
		or eax,eax
		jz .end
		add eax,list_offs_obj3d
		stdcall draw_3d, eax
		jmp .end
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[angle_x]
		fsub dword[delt_size]
		fstp dword[angle_x]
		stdcall [tl_node_get_data],tree1
		or eax,eax
		jz .end
		add eax,list_offs_obj3d
		stdcall draw_3d, eax
		jmp .end
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[angle_y]
		fadd dword[delt_size]
		fstp dword[angle_y]
		stdcall [tl_node_get_data],tree1
		or eax,eax
		jz .end
		add eax,list_offs_obj3d
		stdcall draw_3d, eax
		jmp .end
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_y]
		fsub dword[delt_size]
		fstp dword[angle_y]
		stdcall [tl_node_get_data],tree1
		or eax,eax
		jz .end
		add eax,list_offs_obj3d
		stdcall draw_3d, eax
		;jmp .end
	@@:

	.end:
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

	;menu functions
	cmp ah,5
	jne @f
		call mnu_vertexes_on
		jmp still
	@@:
	cmp ah,6
	jne @f
		call mnu_edges_on
		jmp still
	@@:
	cmp ah,7
	jne @f
		call mnu_faces_on
		jmp still
	@@:
	cmp ah,8
	jne @f
		call mnu_faces_mat
		jmp still
	@@:
	cmp ah,9
	jne @f
		call mnu_light_on_off
		jmp still
	@@:
	cmp ah,10
	jne @f
		call mnu_smooth_on_off
		jmp still
	@@:
	cmp ah,11
	jne @f
		call mnu_reset_settings
		jmp still
	@@:
	cmp ah,12
	jne @f
		call mnu_make_scrshot
		jmp still
	@@:

	cmp ah,1
	jne still
.exit:
	mov dword[tree1.data_img],0
	mov dword[tree1.data_img_sys],0
	stdcall [tl_data_clear], tree1
	stdcall [buf2d_delete],buf_1 ;удаляем буфер
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_data]
	stdcall [gluDeleteQuadric], [qObj]
	mcall SF_TERMINATE_PROCESS


align 4
but_new_file:
push eax ebx
	stdcall [tl_node_poi_get_info], tree1,0
	@@:
		or eax,eax
		jz @f
		mov ebx,eax
		stdcall [tl_node_poi_get_data], tree1,ebx
		add eax,list_offs_obj3d
		stdcall obj_clear_param, eax
		stdcall [tl_node_poi_get_next_info], tree1,ebx
		or eax,eax
		jnz @b
	@@:
pop ebx eax
	stdcall [tl_info_clear], tree1 ;очистка списка объектов
	stdcall [buf2d_clear], buf_ogl, [buf_ogl.color] ;чистим буфер
	stdcall [tl_draw], tree1
	stdcall [buf2d_draw], buf_ogl ;обновляем буфер на экране
	ret

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
    stdcall mem.ReAlloc,[open_file_data],ecx
    mov [open_file_data],eax
    
    mov [run_file_70.Function], SSF_READ_FILE
    mov [run_file_70.Position], 0
    mov [run_file_70.Flags], 0
    mov dword[run_file_70.Count], ecx
    m2m dword[run_file_70.Buffer], dword[open_file_data]
    mov byte[run_file_70+20], 0
    mov dword[run_file_70.FileName], openfile_path
    mcall SF_FILE,run_file_70 ;загружаем файл 3ds
    cmp ebx,0xffffffff
    je .end_open_file
		mov [open_file_size],ebx
		;mcall SF_SET_CAPTION,1,openfile_path

		call init_tree
		stdcall [buf2d_draw], buf_ogl ;обновляем буфер на экране
	.end_open_file:
	popad
	ret

align 4
init_tree:
	;чистим память занятую объектами
	stdcall [tl_node_poi_get_info], tree1,0
	@@:
		or eax,eax
		jz @f
		mov ebx,eax
		stdcall [tl_node_poi_get_data], tree1,ebx
		or eax,eax
		jz @f
			add eax,list_offs_obj3d
			stdcall obj_clear_param, eax
			stdcall [tl_node_poi_get_next_info], tree1,ebx
			or eax,eax
			jnz @b
	@@:
	stdcall [tl_info_clear], tree1 ;очистка списка объектов

	mov esi,[open_file_data]
	stdcall convert_stl_3ds, esi,[open_file_size] ;проверяем файл формата *.stl ?
	or eax,eax
	jz @f
		;если файл в формате *.stl
		mov [open_file_size],ecx
		mov esi,eax
		stdcall mem.Free,[open_file_data]
		mov [open_file_data],esi
		mov byte[can_save],1
	@@:
	cmp word[esi],CHUNK_MAIN
	je @f
		mov eax,[esi]
		bswap eax
		stdcall hex_in_str, txt_no_3ds.zag, eax,8
		stdcall buf_draw_beg, buf_ogl
		stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_no_3ds,5,25,0xff0000 ;рисуем строку с текстом
		jmp .end_open
	@@:
	;--- обработка открытого *.3ds файла
	mov eax,file_3ds
	mov dword[level_stack],0 ;обнуляем уровень стека
	mov dword[offs_last_timer],0
	;--- добавление главного блока в список
	stdcall add_3ds_object, ID_ICON_CHUNK_MAIN,0,dword[esi+2],0
	call block_children ;вход в дочерний блок

	mov edi,[file_3ds.offs]
	add edi,[file_3ds.size]
	.cycle_main:
		cmp dword[level_stack],0
		jle .end_cycle
		
		cmp esi,edi ;если конец файла
		jge .end_cycle

		mov edx,[esi+2] ;размер блока
		call block_analiz
		cmp word[esi],CHUNK_MATERIAL
		je @f
		cmp word[esi],CHUNK_OBJMESH
		je @f
		cmp word[esi],CHUNK_OBJBLOCK
		je @f
			mov dword[bl_found],0
		@@:
		cmp dword[bl_found],0
		jne @f
			;объект не известного вида
			call block_next
			jmp .cycle_main
		@@:
			;объект известного вида
			mov ecx,dword[bl_found]
			mov bx,word[ecx+2] ;номер иконки для объекта
			cmp word[esi],CHUNK_OBJBLOCK
			jne .pod1
				add esi,6
				push esi
				sub esi,6
				jmp .pod3
			.pod1:
			cmp word[esi],CHUNK_MATERIAL
			jne .pod2
				cmp word[esi+6],CHUNK_MATNAME
				jne .pod2
				add esi,12
				push esi
				sub esi,12
				jmp .pod3
			.pod2:
				push dword[ecx+5] ;стандартное название блока
			.pod3:
			stdcall add_3ds_object, ebx,dword[level_stack],edx
			cmp byte[ecx+4],1
			je .bl_data
				;блок содержит дочерние блоки
				call block_children ;вход в дочерний блок
				jmp .cycle_main
			.bl_data:
				;блок содержит данные
				call block_analiz_data
				jmp .cycle_main
	.end_cycle:
		stdcall [tl_cur_beg], tree1
		stdcall [tl_draw], tree1
	.end_open:
	ret

;анализ данных блока
;input:
; eax - stack pointer
; esi - memory pointer
;output:
; eax - new stack pointer
; esi - new memory pointer
align 4
proc block_analiz_data uses ebx ecx edx edi
	mov dx,[esi]
	mov ecx,[esi+2]
	sub ecx,6 ;размер данных в блоке
	add esi,6
	mov ebx,dword[level_stack]
	inc ebx
	; *** анализ блоков с разными данными и выделением подблоков
	cmp dx,CHUNK_OBJBLOCK ;объект
	jne @f
		push ax
			cld
			xor al,al
			mov edi,esi
			repne scasb
		pop ax
		sub edi,esi ;edi - strlen
		add esi,edi
		;sub ecx,edi ;уже сделано в repne
		jmp .next_bl
	@@:
	cmp dx,CHUNK_VERTLIST ;список вершин
	je .vertexes
	cmp dx,0x4111 ;флаги вершин
	je .vertexes
	cmp dx,CHUNK_MAPLIST ;текстурные координаты
	je .vertexes
	jmp @f
	.vertexes: ;обработка блоков, содержащих данные вершин
		add esi,2
		sub ecx,2
		sub esi,8 ;восстановление esi
		call block_next
		jmp .end_f		
	@@:
	cmp dx,CHUNK_FACELIST ;список граней
	jne @f
		push eax
		movzx eax,word[esi]
		shl eax,3
		add esi,2
		sub ecx,2

		sub ecx,eax
		cmp ecx,1
		jl .data_3 ;проверяем есть ли блок описывающий материал, применяемый к объекту
if 1
			add esi,eax
			mov ecx,dword[esi+2]
			stdcall add_3ds_object, 10,ebx,ecx,0 ;данные материала
			sub esi,eax
else
			add esi,eax
			pop eax
			jmp .next_bl
end if
		.data_3:

		sub esi,8 ;восстановление esi
		pop eax
		call block_next
		jmp .end_f		
	@@:
	cmp dx,CHUNK_FACEMAT ;материалы граней
	jne @f
		push ax
			cld
			xor al,al
			mov edi,esi
			repne scasb
		pop ax
		sub edi,esi ;edi - strlen
		stdcall add_3ds_object, ID_ICON_DATA,ebx,edi,0 ;название объекта
		add esi,edi
		add esi,2
		sub ecx,2
		sub esi,edi ;восстановление esi (1)
		sub esi,8   ;восстановление esi (2)
		call block_next
		jmp .end_f
	@@:
	; *** анализ блока с данными по умолчанию (без выделения подблоков)
		sub esi,6 ;восстановление esi
		call block_next
		jmp .end_f
	.next_bl:
	; *** настройки для анализа оставшихся подблоков
		mov dword[eax],esi ;указатель на начало блока
		mov ebx,dword[esi+2]
		mov dword[eax+4],ebx ;размер блока
		inc dword[level_stack]
		add eax,8
	.end_f:
	ret
endp

;вход в 1-й дочерний блок
;input:
; eax - указатель на временный стек файла file_3ds
; esi - начало родительского блока
;output:
; ebx - destroy
; esi - начало данных родительского блока
align 4
block_children:
	push ecx
		;проверка правильности размеров дочернего блока
		lea ebx,[esi+6] ;переход на начало дочернего блока
		add ebx,[ebx+2] ;добавляем размер дочернего блока
		mov ecx,esi
		add ecx,[esi+2] ;добавляем размер родительского блока
		cmp ebx,ecx ;учитывать заголовки не нужно, т. к. сравниваются только данные блоков
		jle @f
			;диагностировали ошибку файла, дочерний блок выходит за пределы родительского
			mov dword[level_stack],FILE_ERROR_CHUNK_SIZE
			jmp .end_f
		@@:
		mov [eax],esi ;указатель на начало блока
		mov ebx,[esi+2]
		mov [eax+4],ebx ;размер блока
		add esi,6 ;переходим к данным блока
		inc dword[level_stack]
		add eax,8
	.end_f:
	pop ecx
	ret

;переход к следущему блоку текущего уровня
;input:
; eax - адрес структуры с переменными
align 4
block_next:
push ebx
	add esi,dword[esi+2] ;пропускаем данные блока

	;проверка размеров родительского блока, для возможного выхода на верхний уровень если конец блока
	@@:
	mov ebx,dword[eax-8]
	add ebx,dword[eax-4]
	cmp esi,ebx
	jl @f
		dec dword[level_stack]
		sub eax,8
		cmp dword[level_stack],0
		jg @b
	@@:
pop ebx
	ret

;функция поиска структуры описывающей блок
;input:
;esi - memory pointer
;output:
;dword[bl_found] - pointer to chunk struct (= 0 if not found)
align 4
bl_found dd 0
block_analiz:
pushad
	mov dword[bl_found],0
	mov ecx,type_bloks
	@@:
		mov bx,word[ecx]
		cmp word[esi],bx
		je .found
		add ecx,sizeof.block_3ds
		cmp ecx,type_bloks.end
		jl @b
	jmp .no_found
	.found:
		mov dword[bl_found],ecx
	.no_found:
popad
	ret

;input:
; esi - указатель на анализируемые данные
; icon - номер иконки
; level - уровень вложенности узла
; size_bl - размер блока
; info_bl - строка с описанием блока
align 4
proc add_3ds_object, icon:dword, level:dword, size_bl:dword, info_bl:dword
	pushad
		mov bx,word[icon]
		shl ebx,16
		mov bx,word[level]

		mov eax,esi
		sub eax,dword[open_file_data]
		mov dword[buffer],eax ;смещение блока
		mov ecx,dword[size_bl]
		mov dword[buffer+4],ecx ;размер блока (используется в функции buf_draw_hex_table для рисования линии)
		mov ecx,dword[bl_found]
		or ecx,ecx
		jz @f
			;... здесь нужен другой алгоритм защиты от удаления
			mov cl,byte[ecx+4]
		@@:
		mov byte[buffer+list_offs_chunk_del],cl
		mov ecx,[level]
		mov byte[buffer+list_offs_chunk_lev],cl
		mov ecx,dword[info_bl]
		mov dword[buffer+list_offs_p_data],ecx
		stdcall hex_in_str, buffer+list_offs_text,dword[esi+1],2
		stdcall hex_in_str, buffer+list_offs_text+2,dword[esi],2 ;код 3ds блока
		or ecx,ecx
		jnz @f
			mov byte[buffer+list_offs_text+4],0 ;0 - символ конца строки
			jmp .no_capt
		@@:
			mov byte[buffer+list_offs_text+4],' '
			mov esi,ecx
			mov edi,buffer+list_offs_text+5
			mov ecx,size_one_list-(list_offs_text+5)
			cld
			rep movsb
			mov byte[buffer+size_one_list-1],0 ;0 - символ конца строки
		.no_capt:
		mov ecx,(sizeof.obj_3d)/4
		xor eax,eax
		mov edi,buffer+list_offs_obj3d
		rep stosd
		stdcall [tl_node_add], tree1, ebx, buffer
		stdcall [tl_cur_next], tree1
	popad
	ret
endp

;input:
; eax - value
; buf - string buffer
; len - buffer len
;output:
align 4
proc convert_int_to_str, buf:dword, len:dword
pushad
	mov edi,[buf]
	mov esi,[len]
	add esi,edi
	dec esi
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
		call .str
		pop eax
	@@:
	cmp edi,esi
	jge @f
		or al,0x30
		stosb
		mov byte[edi],0
	@@:
	ret

;данные для диалога открытия файлов
align 4
OpenDialog_data:
.type			dd 0 ;0 - открыть, 1 - сохранить, 2 - выбрать дтректорию
.procinfo		dd procinfo ;+4
.com_area_name	dd communication_area_name ;+8
.com_area		dd 0 ;+12
.opendir_path	dd plugin_path ;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 путь к диалогу открытия файлов
.draw_window	dd draw_window ;+28
.status 		dd 0 ;+32
.openfile_path	dd openfile_path ;+36 путь к открываемому файлу
.filename_area	dd filename_area ;+40
.filter_area	dd Filter
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
dd Filter.end - Filter.1
.1:
db '3DS',0
db 'STL',0
.3:
db 'PNG',0
.end:
db 0


align 4
system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'box_lib.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'buf2d.obj',0
system_dir_4 db '/sys/lib/'
lib_name_4 db 'kmenu.obj',0
system_dir_5 db '/sys/lib/'
lib_name_5 db 'tinygl.obj',0
system_dir_6 db '/sys/lib/'
lib_name_6 db 'libini.obj',0

align 4
l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, file_name, system_dir_2, import_box_lib
	lib_3 l_libs lib_name_3, file_name, system_dir_3, import_buf2d
	lib_4 l_libs lib_name_4, file_name, system_dir_4, import_libkmenu
	lib_5 l_libs lib_name_5, file_name, system_dir_5, import_lib_tinygl
	lib_6 l_libs lib_name_6, file_name, system_dir_6, import_libini	
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
	dd sz_init0
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
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	buf2d_crop_color dd sz_buf2d_crop_color
	buf2d_offset_h dd sz_buf2d_offset_h
	buf2d_set_pixel dd sz_buf2d_set_pixel
	dd 0,0
	sz_init0 db 'lib_init',0
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
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0

align 4
import_box_lib:
	dd sz_init1
	edit_box_draw dd sz_edit_box_draw
	edit_box_key dd sz_edit_box_key
	edit_box_mouse dd sz_edit_box_mouse
	edit_box_set_text dd sz_edit_box_set_text
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
	sz_edit_box_draw db 'edit_box_draw',0
	sz_edit_box_key db 'edit_box_key',0
	sz_edit_box_mouse db 'edit_box_mouse',0
	sz_edit_box_set_text db 'edit_box_set_text',0
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
import_libkmenu:
	kmenu_init		       dd akmenu_init
	kmainmenu_draw		       dd akmainmenu_draw
	kmainmenu_dispatch_cursorevent dd akmainmenu_dispatch_cursorevent
	ksubmenu_new		       dd aksubmenu_new
	ksubmenu_delete 	       dd aksubmenu_delete
	ksubmenu_draw		       dd aksubmenu_draw
	ksubmenu_add		       dd aksubmenu_add
	kmenuitem_new		       dd akmenuitem_new
	kmenuitem_delete	       dd akmenuitem_delete
	kmenuitem_draw		       dd akmenuitem_draw
dd 0,0
	akmenu_init			db 'kmenu_init',0
	akmainmenu_draw 		db 'kmainmenu_draw',0
	akmainmenu_dispatch_cursorevent db 'kmainmenu_dispatch_cursorevent',0
	aksubmenu_new			db 'ksubmenu_new',0
	aksubmenu_delete		db 'ksubmenu_delete',0
	aksubmenu_draw			db 'ksubmenu_draw',0
	aksubmenu_add			db 'ksubmenu_add',0
	akmenuitem_new			db 'kmenuitem_new',0
	akmenuitem_delete		db 'kmenuitem_delete',0
	akmenuitem_draw 		db 'kmenuitem_draw',0

align 4
import_lib_tinygl:
macro E_LIB n
{
	n dd sz_#n
}
include '../../develop/libraries/TinyGL/asm_fork/export.inc'
	dd 0,0
macro E_LIB n
{
	sz_#n db `n,0
}
include '../../develop/libraries/TinyGL/asm_fork/export.inc'

align 4
import_libini:
	dd alib_init0
	ini_get_str   dd aini_get_str
	ini_get_int   dd aini_get_int
	ini_get_color dd aini_get_color
dd 0,0
	alib_init0     db 'lib_init',0
	aini_get_str   db 'ini_get_str',0
	aini_get_int   db 'ini_get_int',0
	aini_get_color db 'ini_get_color',0

align 4
mouse_dd dd 0
last_time dd 0
angle_dxm dd 1.9111 ;~ 3d_wnd_w/180 - прибавление углов поворота сцены при вращении мышей
angle_dym dd 1.7333 ;~ 3d_wnd_h/180
ratio dd 1.1025 ;~ 3d_wnd_w/3d_wnd_h

align 4
buf_ogl:
	dd 0 ;указатель на буфер изображения
	dw 3d_wnd_l,3d_wnd_t ;+4 left,top
.w: dd 3d_wnd_w
.h: dd 3d_wnd_h
.color: dd 0xffffd0
	dd 24 ;+16 color,bit in pixel

align 4
buf_1:
	dd 0 ;указатель на буфер изображения
	dw 0,0 ;+4 left,top
	dd 128,144 ;+8 w,h
	dd 0,24 ;+16 color,bit in pixel

align 4
el_focus dd tree1
tree1 tree_list size_one_list,300+2, tl_key_no_edit+tl_draw_par_line,\
	16,16, 0xffffff,0xb0d0ff,0x400040, 5,47,195-16,250, 16,list_offs_text,0, el_focus,\
	w_scr_t1,0

align 4
w_scr_t1 scrollbar 16,0, 3,0, 15, 100, 0,0, 0,0,0, 1

align 4
qObj dd 0

light_position dd 0.0, 0.0, -2.0, 1.0 ; Расположение источника [0][1][2]
	;[3] = (0.0 - бесконечно удаленный источник, 1.0 - источник света на определенном расстоянии)
light_dir dd 0.0,0.0,0.0 ;направление лампы

mat_specular dd 0.3, 0.3, 0.3, 1.0 ; Цвет блика
mat_shininess dd 3.0 ; Размер блика (обратная пропорция)
white_light dd 0.8, 0.8, 0.8, 1.0 ; Цвет и интенсивность освещения, генерируемого источником
lmodel_ambient dd 0.3, 0.3, 0.3, 1.0 ; Параметры фонового освещения

if lang eq ru
capt db 'info 3ds [user] версия 29.09.20',0 ;подпись окна
else
capt db 'info 3ds [user] version 29.09.20',0 ;window caption
end if

align 16
i_end:
	ctx1 rb 28 ;sizeof.TinyGLContext = 28
	procinfo process_information
	run_file_70 FileInfoBlock
	sc system_colors
	angle_x rd 1 ;углы поворота сцены
	angle_y rd 1
	angle_z rd 1
	color_ox rd 1
	color_oy rd 1
	color_oz rd 1
	color_bk rd 3
	color_vert rd 1
	color_face rd 1
	color_select rd 1
align 16
	rb 4096
stacktop:
	sys_path rb 2048
	file_name rb 4096
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:
