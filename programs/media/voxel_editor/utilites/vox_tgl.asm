use32
	org 0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 1, start, i_end, mem, stacktop, openfile_path, sys_path

include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../KOSfuncs.inc'
include '../../../load_img.inc'
include '../../../load_lib.mac'
include '../../../develop/libraries/TinyGL/asm_fork/opengl_const.inc'
include '../../../develop/libraries/TinyGL/asm_fork/zbuffer.inc'
include 'vox_3d.inc'
include '../trunk/str.inc'
include 'lang.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
if lang eq ru
caption db 'Просмотр вокселей 11.11.20',0 ;подпись окна
else
caption db 'Voxel viewer 11.11.20',0
end if

3d_wnd_l equ   5 ;отступ для tinygl буфера слева
3d_wnd_t equ  30 ;отступ для tinygl буфера сверху
3d_wnd_w equ 512
3d_wnd_h equ 512

IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
IMAGE_TOOLBAR_SIZE equ IMAGE_TOOLBAR_ICON_SIZE*10

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

	include_image_file 'toolbar_t.png', image_data_toolbar,,,6 ;6 - for gray icons
	mov eax,[image_data_toolbar]
	add eax,IMAGE_TOOLBAR_SIZE
	stdcall img_to_gray, [image_data_toolbar],eax,(IMAGE_TOOLBAR_SIZE)/3

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	stdcall [kosglMakeCurrent], 3d_wnd_l,3d_wnd_t,3d_wnd_w,3d_wnd_h,ctx1
	stdcall [glEnable], GL_DEPTH_TEST
	stdcall [glEnable], GL_NORMALIZE ;делам нормали одинаковой величины во избежание артефактов
	stdcall [glClearColor], 0.0,0.0,0.0,0.0
	stdcall [glShadeModel], GL_SMOOTH

	call but_new_file
	;проверка командной строки
	cmp dword[openfile_path],0
	je @f
		call but_open_file_cmd_lin
	@@:

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov ebx,[last_time]
	add ebx,50 ;задержка
	cmp ebx,eax
	jge @f
		mov ebx,eax
	@@:
	sub ebx,eax
	mcall SF_WAIT_EVENT_TIMEOUT
	bt word[opt_auto_rotate],0
	jnc @f
		or eax,eax
		jz timer_funct
	@@:

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6
	jne @f 
		call mouse
	@@:

	jmp still

align 4
timer_funct:
	pushad
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	;автоматическое изменение угла обзора
	fld dword[angle_y]
	fsub dword[delt_size]
	fstp dword[angle_y]
	call draw_3d
	call [kosglSwapBuffers]

	popad
	jmp still

align 4
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x30000000
	mcall SF_CREATE_WINDOW,(20 shl 16)+800,(20 shl 16)+570,,,caption

	; *** создание кнопок на панель ***
	mcall SF_DEFINE_BUTTON,(5 shl 16)+20,(5 shl 16)+20,3,[sc.work_button]

	mov ebx,(30 shl 16)+20
	mcall ,,,4
	add ebx,25 shl 16
	mcall ,,,5
	add ebx,30 shl 16
	mcall ,,,6
	add ebx,25 shl 16
	mcall ,,,7
	add ebx,25 shl 16
	mcall ,,,8
	add ebx,25 shl 16
	mcall ,,,9
	add ebx,25 shl 16
	mcall ,,,10
	add ebx,25 shl 16
	mcall ,,,11
	add ebx,25 shl 16
	mcall ,,,12

	call draw_toolbar_i

	stdcall [buf2d_draw], buf_0
	call [kosglSwapBuffers]

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret


align 4
draw_toolbar_i:
	; *** рисование иконок на кнопках ***
	mov edx,(7 shl 16)+7 ;icon new
	mcall SF_PUT_IMAGE,[image_data_toolbar],(16 shl 16)+16

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon open
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon save
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;zoom +
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;zoom -
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	cmp word[opt_light],0
	jne @f
		add ebx,IMAGE_TOOLBAR_SIZE ;make gray icon
	@@:
	add edx,(25 shl 16) ;light on/off
	int 0x40
	cmp word[opt_light],0
	jne @f
		sub ebx,IMAGE_TOOLBAR_SIZE
	@@:

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	cmp word[opt_cube_box],0
	jne @f
		add ebx,IMAGE_TOOLBAR_SIZE ;make gray icon
	@@:
	add edx,(25 shl 16) ;box on/off
	int 0x40
	cmp word[opt_cube_box],0
	jne @f
		sub ebx,IMAGE_TOOLBAR_SIZE
	@@:

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	cmp word[opt_auto_rotate],0
	jne @f
		add ebx,IMAGE_TOOLBAR_SIZE ;make gray icon
	@@:
	add edx,(25 shl 16) ;auto rotate on/off
	int 0x40
	cmp word[opt_auto_rotate],0
	jne @f
		sub ebx,IMAGE_TOOLBAR_SIZE
	@@:

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;info voxels
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;refresh
	int 0x40
	ret


align 4
key:
	mcall SF_GET_KEY

	cmp ah,178 ;Up
	jne @f
		fld dword[angle_x]
		fadd dword[delt_size]
		fstp dword[angle_x]
		jmp .end0
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[angle_x]
		fsub dword[delt_size]
		fstp dword[angle_x]
		jmp .end0
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[angle_y]
		fadd dword[delt_size]
		fstp dword[angle_y]
		jmp .end0
	@@:
	cmp ah,179 ;Right
	jne still  ;@f
		fld dword[angle_y]
		fsub dword[delt_size]
		fstp dword[angle_y]
	.end0:
		call draw_3d
		call [kosglSwapBuffers]
	;@@:
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
		mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
		mov ebx,eax
		shr ebx,16 ;mouse.x
		cmp ebx,3d_wnd_l
		jg @f
			mov ebx,3d_wnd_l
		@@:
		sub ebx,3d_wnd_l
		cmp ebx,3d_wnd_w
		jle @f
			mov ebx,3d_wnd_w
		@@:
		and eax,0xffff ;mouse.y
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

		call draw_3d
		call [kosglSwapBuffers]
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
		shr ebx,16 ;mouse.x
		cmp ebx,3d_wnd_l
		jl .end_d
		sub ebx,3d_wnd_l
		cmp ebx,3d_wnd_w
		jg .end_d
		and eax,0xffff ;mouse.y
		cmp eax,3d_wnd_t
		jl .end_d
		sub eax,3d_wnd_t
		cmp eax,3d_wnd_h
		jg .end_d
		mov dword[mouse_drag],1
		mov dword[mouse_x],ebx
		mov dword[mouse_y],eax
	.end_d:

	;stdcall [kmainmenu_dispatch_cursorevent], [main_menu]
	pop ebx eax
	ret

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
		call but_zoom_p
		jmp still
	@@:
	cmp ah,7
	jne @f
		call but_zoom_m
		jmp still
	@@:
	cmp ah,8
	jne @f
		call but_light
		jmp still
	@@:
	cmp ah,9
	jne @f
		call but_4
		jmp still
	@@:
	cmp ah,10
	jne @f
		call but_5
		jmp still
	@@:
	cmp ah,11
	jne @f
		call but_info
		jmp still
	@@:
	cmp ah,12
	jne @f
		call but_draw_cadr
		jmp still
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_data]
	stdcall mem.Free,[open_file_ogl]
	mcall SF_TERMINATE_PROCESS


align 4
but_new_file:
	mov dword[angle_x], -30.0
	mov dword[angle_y], 180.0
	mov dword[angle_z], 180.0
	ret

align 4
open_file_data dd 0 ;указатель на память для открытия файлов
open_file_size dd 0 ;размер открытого файла
open_file_ogl dd 0 ;для записи координат шраней вокселей в показе opengl
v_zoom dd 0

align 4
but_open_file:
	copy_path open_dialog_name,communication_area_default_path,file_name,0
pushad
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je @f
		;код при удачном открытии диалога
		call but_open_file_cmd_lin
	@@:
popad
	ret

align 4
but_open_file_cmd_lin:
pushad
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
	mcall SF_FILE,run_file_70
	cmp ebx,0xffffffff
	je .end_open_file

	mov [open_file_size],ebx
	mcall SF_SET_CAPTION,1,openfile_path

	mov eax,[open_file_data]
	movzx eax,byte[eax]
	mov dword[v_zoom],eax ;берем масштаб по умолчанию
	mov ecx,[open_file_size]
	sub ecx,vox_offs_data
	shr ecx,2
	imul ecx,vox_ogl_size
	add ecx,4 ;ecx = размер памяти необходимой для распаковки координат
	stdcall mem.ReAlloc,[open_file_ogl],ecx
	or eax,eax
	jz .end_open_file
		mov [open_file_ogl],eax
		stdcall buf_vox_obj_create_3d,[open_file_data],eax,0,0,[v_zoom]
		call draw_cadr_8
	.end_open_file:
popad
	ret

;description:
; рисование 8-ми кадров под разными углами поворота
align 4
draw_cadr_8:
	call but_new_file ;установка углов поворота по умолчанию
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер

	;рисование 8-ми кадров
	fild dword[rot_angles+4]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 128, 0, buf_1

	fild dword[rot_angles+8]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 0, 128, buf_1
	
	fild dword[rot_angles+12]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 128, 128, buf_1

	fild dword[rot_angles+16]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 0, 256, buf_1
	
	fild dword[rot_angles+20]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 128, 256, buf_1

	fild dword[rot_angles+24]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 0, 384, buf_1
	
	fild dword[rot_angles+28]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 128, 384, buf_1

	; *** последний кадр ***
	fild dword[rot_angles]
	fstp dword[angle_y]
	call draw_3d
	call draw_cadr
	stdcall [buf2d_bit_blt], buf_0, 0, 0, buf_1
	
	call draw_3d
	; ***

	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	ret

align 4
rot_angles dd 180,225,270,315,0,45,90,135

align 4
draw_cadr:
	mov eax,dword[ctx1] ;eax -> TinyGLContext.GLContext
	mov eax,[eax] ;eax -> ZBuffer
	mov eax,[eax+ZBuffer.pbuf]
	mov dword[buf_1],eax

	mov dword[buf_1.w],512
	mov dword[buf_1.h],512
	stdcall [buf2d_img_hdiv2],buf_1
	mov dword[buf_1.h],256
	stdcall [buf2d_img_hdiv2],buf_1
	mov dword[buf_1.h],128
	stdcall [buf2d_img_wdiv2],buf_1
	mov dword[buf_1.w],256
	stdcall [buf2d_img_wdiv2],buf_1
	mov dword[buf_1.w],128
	ret

align 4
but_save_file:
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	pushad
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_save_file
	;код при удачном открытии диалога

	mov [run_file_70.Function], SSF_CREATE_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov ebx, dword[open_file_data]
	;пишем в файл новый масштаб
	mov edx,dword[v_zoom]
	mov byte[ebx],dl
	mov [run_file_70.Buffer], ebx
	mov ebx,[open_file_size]
	mov dword[run_file_70.Count], ebx ;размер файла
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70
	;cmp ebx,0xffffffff
	;je .end_save_file
	; ... сообщение о неудачном сохранении ...

	.end_save_file:
	popad
	ret

align 4
proc but_zoom_p uses eax
    cmp dword[v_zoom],11 ;max=2^11=2048
    jge @f
        inc dword[v_zoom]
        stdcall buf_vox_obj_create_3d,[open_file_data],[open_file_ogl],0,0,[v_zoom]
		call draw_3d
		call [kosglSwapBuffers]
    @@:
    ret
endp

align 4
proc but_zoom_m uses eax
    cmp dword[v_zoom],1
    jle @f
        dec dword[v_zoom]
        stdcall buf_vox_obj_create_3d,[open_file_data],[open_file_ogl],0,0,[v_zoom]
		call draw_3d
		call [kosglSwapBuffers]
    @@:
    ret
endp

align 4
proc but_light uses eax ebx ecx edx
	xor word[opt_light],1
	cmp word[opt_light],0
	je @f
		stdcall [glEnable], GL_LIGHTING
		stdcall [glEnable], GL_LIGHT0
		jmp .end_light
	@@:
		stdcall [glDisable], GL_LIGHTING
		stdcall [glDisable], GL_LIGHT0
	.end_light:
	call draw_toolbar_i
	call draw_3d
	call [kosglSwapBuffers]
	ret
endp

align 4
proc but_4 uses eax ebx ecx edx
	xor word[opt_cube_box],1
	call draw_toolbar_i
	call draw_3d
	call [kosglSwapBuffers]
	ret
endp

align 4
proc but_5 uses eax ebx ecx edx
	xor word[opt_auto_rotate],1
	call draw_toolbar_i
	ret
endp

align 4
proc but_info uses eax ebx ecx edx edi
	;вычисление статистики по вокселям
	mov eax,[open_file_ogl]
	or eax,eax
	jz .end_stat
		mov ebx,[eax]
		mov ecx,ebx
		mov edx,ebx
		imul ebx,6
		add eax,4
align 4
		.cycle_0:
			bt word[eax+vox_ogl_planes],vox_ogl_gran_z0
			jc @f
			dec ebx
			@@:
			bt word[eax+vox_ogl_planes],vox_ogl_gran_z1
			jc @f
			dec ebx
			@@:
			bt word[eax+vox_ogl_planes],vox_ogl_gran_y0
			jc @f
			dec ebx
			@@:
			bt word[eax+vox_ogl_planes],vox_ogl_gran_y1
			jc @f
			dec ebx
			@@:
			bt word[eax+vox_ogl_planes],vox_ogl_gran_x0
			jc @f
			dec ebx
			@@:
			bt word[eax+vox_ogl_planes],vox_ogl_gran_x1
			jc @f
			dec ebx
			@@:
			add eax,vox_ogl_size
		loop .cycle_0

		mov eax,edx
		mov edi,txt_stat_m1.v
		stdcall convert_int_to_str,20

		mov eax,ebx
		mov edi,txt_stat_m2.v
		stdcall convert_int_to_str,20

		stdcall str_n_cat,txt_stat_m1.v,txt_stat_m2,50
		notify_window_run txt_stat_m1
	.end_stat:
	ret
endp

align 4
txt_stat_m1:
if lang eq ru
	db 'Статистика',13,10,'Вокселей: '
.v: rb 70
txt_stat_m2:
	db 13,10,'Отображаемых граней: '
else
	db 'Statistics',13,10,'Voxels: '
.v: rb 70
txt_stat_m2:
	db 13,10,'Facets displayed: '
end if
.v: rb 20

align 4
proc but_draw_cadr uses eax ebx ecx edx
	mov ebx,[angle_x]
	mov ecx,[angle_y]
	mov edx,[angle_z]
	call draw_cadr_8
	mov [angle_x],ebx
	mov [angle_y],ecx
	mov [angle_z],edx
	cmp word[opt_auto_rotate],0
	jne @f
		call draw_3d
		;call [kosglSwapBuffers]
	@@:
	ret
endp

align 4
draw_3d:
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины
	stdcall [glPushMatrix]

	cmp word[opt_light],0
	je @f
		call SetLight
	@@:
	stdcall [glTranslatef], 0.0,0.0,0.5 ;координаты по оси z должны быть в пределах от 0.0 до 1.0, иначе изображение будет отсекаться
		;воксельный объект имеет координаты по осям от -0.5 до 0.5, потому его двигаем на +0.5
		;но все же при поворотах будут отсекатся края, которые вылезут за пределы плоскостей отсечения
		;в версии opengl под Win координаты идут от -1.0 до 1.0 потому там этого делать не нужно
	stdcall [glScalef], [scale], [scale], [scale] ;увеличиваем воксельный объект, что-бы не был очень маленьким
	stdcall [glScalef], 1.0, 1.0, 0.25 ;что-бы края объекта не вылазили за грани отсечения
	stdcall [glRotatef], [angle_x],1.0,0.0,0.0
	stdcall [glRotatef], [angle_y],0.0,1.0,0.0
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0
	stdcall draw_voxels_3d,[open_file_ogl]

	call [glPopMatrix]
ret

align 4
proc SetLight
    stdcall [glLightfv], GL_LIGHT0, GL_POSITION, light_position
    stdcall [glLightfv], GL_LIGHT0, GL_SPOT_DIRECTION, light_dir

    stdcall [glLightfv], GL_LIGHT0, GL_DIFFUSE, white_light
    stdcall [glLightfv], GL_LIGHT0, GL_SPECULAR, white_light

    stdcall [glEnable], GL_COLOR_MATERIAL
    stdcall [glColorMaterial], GL_FRONT, GL_AMBIENT_AND_DIFFUSE
    stdcall [glMaterialfv], GL_FRONT, GL_SPECULAR, mat_specular
    stdcall [glMaterialf], GL_FRONT, GL_SHININESS, mat_shininess
    stdcall [glLightModelfv], GL_LIGHT_MODEL_AMBIENT, lmodel_ambient
  
    stdcall [glEnable], GL_LIGHTING
    stdcall [glEnable], GL_LIGHT0
    
    ;;;stdcall [glLightModeli], GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE
    ret
endp

align 4
proc img_to_gray, buf_rgb:dword, buf_g24:dword, pixels:dword
pushad
	mov esi,[buf_rgb]
	mov edi,[buf_g24]
	mov ecx,[pixels]
	mov ebx,3
align 4
	@@:
		movzx eax,byte[esi]
		movzx edx,byte[esi+1]
		add eax,edx
		movzx edx,byte[esi+2]
		add eax,edx
		xor edx,edx
		div ebx ;shr eax,2
		mov ah,al
		mov word[edi],ax
		mov byte[edi+2],al
		add esi,3
		add edi,3
		loop @b
popad
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
.1:
db 'VOX',0
db 'TXT',0
.end:
db 0


system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'tinygl.obj',0

l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, file_name, system_dir_2, import_buf2d
	lib_3 l_libs lib_name_3, file_name, system_dir_3, import_tinygl
l_libs_end:

align 4
import_libimg:
	dd alib_init1
;	img_is_img  dd aimg_is_img
;	img_info    dd aimg_info
;	img_from_file dd aimg_from_file
;	img_to_file dd aimg_to_file
;	img_from_rgb dd aimg_from_rgb
;	img_to_rgb  dd aimg_to_rgb
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
;	img_encode  dd aimg_encode
;	img_create  dd aimg_create
	img_destroy dd aimg_destroy
;	img_destroy_layer dd aimg_destroy_layer
;	img_count   dd aimg_count
;	img_lock_bits dd aimg_lock_bits
;	img_unlock_bits dd aimg_unlock_bits
;	img_flip    dd aimg_flip
;	img_flip_layer dd aimg_flip_layer
;	img_rotate  dd aimg_rotate
;	img_rotate_layer dd aimg_rotate_layer
;	img_draw    dd aimg_draw
;	img_convert dd aimg_convert

	dd 0,0
	alib_init1   db 'lib_init',0
;	aimg_is_img  db 'img_is_img',0 ;определяет по данным, может ли библиотека сделать из них изображение
;	aimg_info    db 'img_info',0
;	aimg_from_file db 'img_from_file',0
;	aimg_to_file db 'img_to_file',0
;	aimg_from_rgb db 'img_from_rgb',0
;	aimg_to_rgb  db 'img_to_rgb',0 ;преобразование изображения в данные RGB
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;автоматически определяет формат графических данных
;	aimg_encode  db 'img_encode',0
;	aimg_create  db 'img_create',0
	aimg_destroy db 'img_destroy',0
;	aimg_destroy_layer db 'img_destroy_layer',0
;	aimg_count   db 'img_count',0
;	aimg_lock_bits db 'img_lock_bits',0
;	aimg_unlock_bits db 'img_unlock_bits',0
;	aimg_flip    db 'img_flip',0
;	aimg_flip_layer db 'img_flip_layer',0
;	aimg_rotate  db 'img_rotate',0
;	aimg_rotate_layer db 'img_rotate_layer',0
;	aimg_draw    db 'img_draw',0
;	aimg_convert db 'img_convert',0

align 4
import_proclib:
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

;--------------------------------------------------
align 4
import_tinygl:

macro E_LIB n
{
	n dd sz_#n
}
include '../../../develop/libraries/TinyGL/asm_fork/export.inc'
	dd 0,0
macro E_LIB n
{
	sz_#n db `n,0
}
include '../../../develop/libraries/TinyGL/asm_fork/export.inc'

last_time dd 0

align 4
buf_0: dd 0 ;указатель на буфер изображения
	dw 530 ;+4 left
	dw 30 ;+6 top
.w: dd 256 ;+8 w
.h: dd 512 ;+12 h
.color: dd 0xffffd0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_1: dd 0 ;указатель на буфер изображения
	dw 0 ;+4 left
	dw 0 ;+6 top
.w: dd 512 ;+8 w
.h: dd 512 ;+12 h
.color: dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

scale dd 1.414213562
angle_x dd 0.0
angle_y dd 0.0
angle_z dd 0.0
delt_size dd 3.0
angle_dxm dd 2.8444 ;~ 3d_wnd_w/180 - прибавление углов поворота сцены при вращении мышей
angle_dym dd 2.8444 ;~ 3d_wnd_h/180

opt_light dw 0 ;опция для включения/выключения света
opt_cube_box dw 1 ;опция для рисования рамки вокруг объекта
opt_auto_rotate dw 0 ;опция для автоматического поворота объекта

light_position dd 0.0, 0.0, 2.0, 1.0 ; Расположение источника [0][1][2]
	;[3] = (0.0 - бесконечно удаленный источник, 1.0 - источник света на определенном расстоянии)
light_dir dd 0.0,0.0,0.0 ;направление лампы

mat_specular dd 0.3, 0.3, 0.3, 1.0 ; Цвет блика
mat_shininess dd 3.0 ; Размер блика (обратная пропорция)
white_light dd 0.8, 0.8, 0.8, 1.0 ; Цвет и интенсивность освещения, генерируемого источником
lmodel_ambient dd 0.3, 0.3, 0.3, 1.0 ; Параметры фонового освещения


align 16
i_end:
	ctx1 rb 28 ;sizeof.TinyGLContext = 28
	image_data_toolbar dd ?
	mouse_drag dd ? ;режим поворота сцены от перемещении курсора мыши
	mouse_x dd ?
	mouse_y dd ?
	rb 4096
stacktop:
	sys_path rb 1024
	file_name rb 2048 
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
	sc system_colors
	procinfo process_information
	run_file_70 FileInfoBlock
mem:
