; SPDX-License-Identifier: GPL-2.0-only
; Textures0 - example of texture mapping on a sphere and cube
; Copyright (C) 2015-2025 KolibriOS team

use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,0,cur_dir_path

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../load_img.inc'
include '../../../../../load_lib.mac'
include '../kosgl.inc'
include '../opengl_const.inc'
include '../zbuffer.inc'
include '../../../../../develop/info3ds/info_fun_float.inc'

3d_wnd_l equ   0 ;tinygl buffer left indent
3d_wnd_t equ  30 ;tinygl buffer top indent
3d_wnd_w equ 500
3d_wnd_h equ 400

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

IMAGE_TOOLBAR_ICON_SIZE equ 21*21*3

align 4
start:
load_libraries l_libs_start,l_libs_end
	;checking how successfully the libraries were loaded
	mov	ebp,lib_0
	.test_lib_open:
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:
	add ebp,ll_struc_size
	cmp ebp,l_libs_end
	jl .test_lib_open

	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0x27

	stdcall [kosglMakeCurrent], 3d_wnd_l,3d_wnd_t,[buf_ogl.w],[buf_ogl.h],ctx1
	stdcall [glEnable], GL_DEPTH_TEST
	stdcall [glEnable], GL_NORMALIZE ;normals of the same size to avoid artifacts
	call [gluNewQuadric]
	mov [qObj],eax
	stdcall [gluQuadricTexture], eax,GL_TRUE

	stdcall [glClearColor], 0.25,0.25,0.25,0.0
	stdcall [glShadeModel], GL_SMOOTH

	mov eax,[ctx1.gl_context]
	mov eax,[eax] ;eax -> ZBuffer
	mov eax,[eax+ZBuffer.pbuf]
	mov dword[buf_ogl],eax

	load_image_file 'font8x9.bmp', image_data_toolbar, buf_1.w,buf_1.h
	stdcall [buf2d_create_f_img], buf_1,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память
	stdcall [buf2d_conv_24_to_8], buf_1,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_1

	load_image_file 'toolb_1.png', image_data_toolbar
	load_image_file 'text_1.png', texture, text_w,text_h ;открытие файла текстуры

	fld dword[angle_y]
	stdcall update_number, txt_angle_y.v
	fld dword[angle_z]
	stdcall update_number, txt_angle_z.v

	;* Setup texturing *
	stdcall [glTexEnvi], GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL
  
	;* generate texture object IDs *
	stdcall [glGenTextures], 1, TexObj
	stdcall [glBindTexture], GL_TEXTURE_2D, [TexObj]
	stdcall [glTexImage2D], GL_TEXTURE_2D, 0, 3, [text_w], [text_h],\
		0, GL_RGB, GL_UNSIGNED_BYTE, [texture]
    
	stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST
	stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST
	stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT
	stdcall [glTexParameteri], GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT
	stdcall [glBindTexture], GL_TEXTURE_2D, [TexObj]
	stdcall [glEnable], GL_TEXTURE_2D

	call draw_3d

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_WAIT_EVENT
	cmp   al,EV_REDRAW
	jz    red_win
	cmp   al,EV_KEY
	jz    key
	cmp   al,EV_BUTTON
	jz    button
	jmp   still

align 4
draw_window:
	pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mcall SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
	mov ebx,[buf_ogl.w]
	add ebx,(50 shl 16)+9
	mov ecx,[buf_ogl.h]
	add ecx,(30 shl 16)+4
	add ecx,eax
	add cx,[buf_ogl.t]
	mcall SF_CREATE_WINDOW,,,0x33ffffff,,caption

	mov esi,[sc.work_button]
	mcall SF_DEFINE_BUTTON,(6 shl 16)+19,(6 shl 16)+19,3+0x40000000 ;сферы
	mcall ,(36 shl 16)+19,,4+0x40000000 ;куб
	mcall ,(66 shl 16)+19,,5+0x40000000 ;масштаб +
	mcall ,(96 shl 16)+19,,6+0x40000000 ;масштаб -

	mcall SF_PUT_IMAGE,[image_data_toolbar],(21 shl 16)+21,(5 shl 16)+5 ;сферы
	add ebx,2*IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,(35 shl 16)+5 ;куб
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,(65 shl 16)+5 ;масштаб +
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	mcall ,,,(95 shl 16)+5 ;масштаб -

	call [kosglSwapBuffers]
	mcall SF_REDRAW,SSF_END_DRAW
	popad
	ret

align 4
key:
	mcall SF_GET_KEY

	cmp ah,27 ;Esc
	je button.exit

	cmp ah,61 ;+
	jne @f
		call but_zoom_p
		jmp still
	@@:
	cmp ah,45 ;-
	jne @f
		call but_zoom_m
		jmp still
	@@:
	cmp ah,178 ;Up
	jne @f
		finit
		fld dword[angle_y]
		fadd dword[delt_size]
		fst dword[angle_y]
		stdcall update_number, txt_angle_y.v
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,177 ;Down
	jne @f
		finit
		fld dword[angle_y]
		fsub dword[delt_size]
		fst dword[angle_y]
		stdcall update_number, txt_angle_y.v
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,176 ;Left
	jne @f
		finit
		fld dword[angle_z]
		fadd dword[delt_size]
		fst dword[angle_z]
		stdcall update_number, txt_angle_z.v
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,179 ;Right
	jne @f
		finit
		fld dword[angle_z]
		fsub dword[delt_size]
		fst dword[angle_z]
		stdcall update_number, txt_angle_z.v
		call draw_3d
		call [kosglSwapBuffers]
	@@:

	jmp still

align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,3
	jne @f
		call but_dr_0
		jmp still
	@@:
	cmp ah,4
	jne @f
		call but_dr_1
		jmp still
	@@:
	cmp ah,5
	jne @f
		call but_zoom_p
		jmp still
	@@:
	cmp ah,6
	jne @f
		call but_zoom_m
		jmp still
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [gluDeleteQuadric], [qObj]
	stdcall mem.Free,[image_data_toolbar]
	mcall SF_TERMINATE_PROCESS

;input:
; st0 - number
; txt_addr - pointer to text buffer
align 4
proc update_number uses eax, txt_addr:dword
	mov word[NumberSymbolsAD],2
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	mov eax,[txt_addr]
	mov byte[eax],0
	stdcall str_cat, eax,Data_String
	ret
endp

align 4
but_dr_0:
	mov dword[dr_figure],0
	call draw_3d
	call [kosglSwapBuffers]
	ret

align 4
but_dr_1:
	mov dword[dr_figure],1
	call draw_3d
	call [kosglSwapBuffers]
	ret

align 4
but_zoom_p:
	finit
	fld dword[scale]
	fadd dword[sc_delt]
	fcom dword[sc_max]
	fstsw ax
	sahf
	jbe @f
		ffree st0
		fincstp
		fld dword[sc_max]
	@@:
	fst dword[scale]
	mov word[NumberSymbolsAD],3
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	mov byte[txt_scale.v],0
	stdcall str_cat, txt_scale.v,Data_String
	call draw_3d
	call [kosglSwapBuffers]
	ret

align 4
but_zoom_m:
	finit
	fld dword[scale]
	fsub dword[sc_delt]
	fcom dword[sc_min]
	fstsw ax
	sahf
	ja @f
		ffree st0
		fincstp
		fld dword[sc_min]
	@@:
	fst dword[scale]
	mov word[NumberSymbolsAD],3
	fstp qword[Data_Double]
	call DoubleFloat_to_String
	mov byte[txt_scale.v],0
	stdcall str_cat, txt_scale.v,Data_String
	call draw_3d
	call [kosglSwapBuffers]
	ret


align 4
draw_3d:
stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины

call [glPushMatrix]
	stdcall [glScalef], [scale], [scale], [scale]
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0
	stdcall [glRotatef], [angle_y],0.0,1.0,0.0

cmp dword[dr_figure],0
jne @f
	; рисование сфер
	stdcall [glColor3f], 1.0, 1.0, 0.0
	push 32
	push 32
	glpush rad1
	stdcall [gluSphere], [qObj]

	stdcall [glColor3f], 1.0, 0.0, 0.0
	stdcall [glTranslatef], -1.6,0.0,0.0
	push 16
	push 16
	glpush rad2
	stdcall [gluSphere], [qObj]

	stdcall [glColor3f], 0.0, 0.0, 1.0
	stdcall [glTranslatef], 3.2,0.0,0.0
	push 16
	push 16
	glpush rad2
	stdcall [gluSphere], [qObj]
@@:
cmp dword[dr_figure],1
jne @f
	; рисование граней куба
	stdcall [glBegin],GL_QUADS

; Передняя грань
	stdcall [glTexCoord2f], 0.0f, 0.0f
	stdcall [glVertex3f], -1.0f, -1.0f,  1.0f ; Низ лево
	stdcall [glTexCoord2f], 1.0f, 0.0f
	stdcall [glVertex3f],  1.0f, -1.0f,  1.0f ; Низ право
	stdcall [glTexCoord2f], 1.0f, 1.0f
	stdcall [glVertex3f],  1.0f,  1.0f,  1.0f ; Верх право
	stdcall [glTexCoord2f], 0.0f, 1.0f
	stdcall [glVertex3f], -1.0f,  1.0f,  1.0f ; Верх лево

; Задняя грань
	stdcall [glTexCoord2f], 1.0f, 0.0f
	stdcall [glVertex3f], -1.0f, -1.0f, -1.0f ; Низ право
	stdcall [glTexCoord2f], 1.0f, 1.0f
	stdcall [glVertex3f], -1.0f,  1.0f, -1.0f ; Верх право
	stdcall [glTexCoord2f], 0.0f, 1.0f
	stdcall [glVertex3f],  1.0f,  1.0f, -1.0f ; Верх лево
	stdcall [glTexCoord2f], 0.0f, 0.0f
	stdcall [glVertex3f],  1.0f, -1.0f, -1.0f ; Низ лево

; Верхняя грань
	stdcall [glTexCoord2f], 0.0f, 1.0f
	stdcall [glVertex3f], -1.0f,  1.0f, -1.0f ; Верх лево
	stdcall [glTexCoord2f], 0.0f, 0.0f
	stdcall [glVertex3f], -1.0f,  1.0f,  1.0f ; Низ лево
	stdcall [glTexCoord2f], 1.0f, 0.0f
	stdcall [glVertex3f],  1.0f,  1.0f,  1.0f ; Низ право
	stdcall [glTexCoord2f], 1.0f, 1.0f
	stdcall [glVertex3f],  1.0f,  1.0f, -1.0f ; Верх право
	
; Нижняя грань
	stdcall [glTexCoord2f], 1.0f, 1.0f
	stdcall [glVertex3f], -1.0f, -1.0f, -1.0f ; Верх право
	stdcall [glTexCoord2f], 0.0f, 1.0f
	stdcall [glVertex3f],  1.0f, -1.0f, -1.0f ; Верх лево
	stdcall [glTexCoord2f], 0.0f, 0.0f
	stdcall [glVertex3f],  1.0f, -1.0f,  1.0f ; Низ лево
	stdcall [glTexCoord2f], 1.0f, 0.0f
	stdcall [glVertex3f], -1.0f, -1.0f,  1.0f ; Низ право

; Правая грань
	stdcall [glTexCoord2f], 1.0f, 0.0f
	stdcall [glVertex3f],  1.0f, -1.0f, -1.0f ; Низ право
	stdcall [glTexCoord2f], 1.0f, 1.0f
	stdcall [glVertex3f],  1.0f,  1.0f, -1.0f ; Верх право
	stdcall [glTexCoord2f], 0.0f, 1.0f
	stdcall [glVertex3f],  1.0f,  1.0f,  1.0f ; Верх лево
	stdcall [glTexCoord2f], 0.0f, 0.0f
	stdcall [glVertex3f],  1.0f, -1.0f,  1.0f ; Низ лево

; Левая грань
	stdcall [glTexCoord2f], 0.0f, 0.0f
	stdcall [glVertex3f], -1.0f, -1.0f, -1.0f ; Низ лево
	stdcall [glTexCoord2f], 1.0f, 0.0f
	stdcall [glVertex3f], -1.0f, -1.0f,  1.0f ; Низ право
	stdcall [glTexCoord2f], 1.0f, 1.0f
	stdcall [glVertex3f], -1.0f,  1.0f,  1.0f ; Верх право
	stdcall [glTexCoord2f], 0.0f, 1.0f
	stdcall [glVertex3f], -1.0f,  1.0f, -1.0f ; Верх лево

	call [glEnd]
@@:
call [glPopMatrix]

	stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_scale,5,5,0xffff00
	stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_angle_z,5,15,0xffff00
	stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_angle_y,5,25,0xffff00
	ret

align 4
caption db 'Test textures, [Esc] - exit, [<-],[->],[Up],[Down] - rotate',0

align 4
rad1 dq 1.0
rad2 dq 0.55
 
scale dd 0.4 ;начальный масштаб
sc_delt dd 0.05 ;изменение масштаба при нажатии
sc_min dd 0.1 ;минимальный масштаб
sc_max dd 1.1 ;максимальный масштаб
angle_z dd 0.0
angle_y dd 0.0
delt_size dd 3.0

light_position dd 3.0, 2.0, -10.0, 1.0 ; Расположение источника [0][1][2]
	;[3] = (0.0 - бесконечно удаленный источник, 1.0 - источник света на определенном расстоянии)
light_dir dd 0.0,0.0,0.0 ;направление лампы
mat_specular dd 0.1, 0.1, 0.1, 1.0 ; Цвет блика
;mat_shininess dd 3.0 ; Размер блика (обратная пропорция)
white_light dd 0.8, 0.8, 0.8, 1.0 ; Цвет и интенсивность освещения, генерируемого источником
lmodel_ambient dd 0.2, 0.2, 0.2, 1.0 ; Параметры фонового освещения

;--------------------------------------------------
include '../import.inc' ;tinygl
include '../../../buf2d/import.inc'
include '../../../libs-dev/libimg/import.inc'

;--------------------------------------------------
system_dir_0 db '/sys/lib/'
lib_name_0 db 'tinygl.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'buf2d.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'libimg.obj',0
;--------------------------------------------------

txt_scale:
db 'Scale: '
.v: rb 11

txt_angle_z:
db 'Rotate z: '
.v: rb 11

txt_angle_y:
db 'Rotate y: '
.v: rb 11

align 4
buf_ogl:
	dd 0 ;указатель на буфер изображения
	dw 3d_wnd_l ;+4 left
.t: dw 3d_wnd_t ;+6 top
.w: dd 3d_wnd_w
.h: dd 3d_wnd_h
	dd 0,24 ;+16 color,bit in pixel

align 4
buf_1:
	dd 0 ;указатель на буфер изображения
	dd 0 ;+4 left,top
.w: dd 0
.h: dd 0,0,24 ;+16 color,bit in pixel

align 4
l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_tinygl
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_buf2d
	lib_2 l_libs lib_name_2, file_name, system_dir_2, import_libimg
l_libs_end:

align 4
i_end:
	ctx1 TinyGLContext
	image_data_toolbar dd 0
	dr_figure dd 0
	qObj dd 0
	TexObj dd 0 ;массив указателей на текстуры (в данном случае 1 шт.)
	texture dd 0 ;указатель на память с текстурой
	text_w dd 0
	text_h dd 0
	run_file_70 FileInfoBlock
	sc system_colors
align 16
	cur_dir_path rb 4096
	file_name rb 4096
	rb 4096
stacktop:
mem:
