use32
	org 0x0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,0,cur_dir_path

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../develop/libraries/box_lib/load_lib.mac'
include '../../../../../dll.inc'
include '../opengl_const.inc'

@use_library

align 4
start:
	load_library name_tgl, cur_dir_path, library_path, system_path, \
		err_message_found_lib, head_f_l, import_lib_tinygl, err_message_import, head_f_i
	cmp eax,SF_TERMINATE_PROCESS
	jz button.exit

	mcall SF_SET_EVENTS_MASK,0x27

	;заполняем массив индексов из файла house.3ds (который вшит внутрь данной программы)
	mov esi,house_3ds
	add esi,0x1798 ;смещение по которому идет информация о гранях в файле 3ds (получено с использованием программы info_3ds)
	mov edi,Indices
	mov eax,0x1a6 ;число граней в файле 3ds (получено с использованием программы info_3ds)
	@@:
		movsd
		movsw
		add esi,2 ;пропускаем свойства грани
		dec eax
		or eax,eax
	jnz @b

	;первоначальные настройки контекста tinygl
	stdcall [kosglMakeCurrent], 10,10,400,350,ctx1
	stdcall [glEnable], GL_DEPTH_TEST
	stdcall [glClearColor], 0.0,0.0,0.0,0.0
	stdcall [glShadeModel], GL_SMOOTH

	call draw_3d

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_WAIT_EVENT
	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	jmp still

align 4
draw_window:
	pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mov edx,0x33ffffff
	mcall SF_CREATE_WINDOW,(50 shl 16)+430,(30 shl 16)+400,,,caption
	stdcall [kosglSwapBuffers]

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
		fld dword[scale]
		fadd dword[delt_sc]
		fstp dword[scale]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:
	cmp ah,45 ;-
	jne @f
		fld dword[scale]
		fsub dword[delt_sc]
		fstp dword[scale]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:
	cmp ah,178 ;Up
	jne @f
		fld dword[angle_y]
		fadd dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[angle_y]
		fsub dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[angle_x]
		fadd dword[delt_size]
		fstp dword[angle_x]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_x]
		fsub dword[delt_size]
		fstp dword[angle_x]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:

	jmp still

align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,1
	jne still
.exit:
	mcall SF_TERMINATE_PROCESS


align 4
caption db 'Test opengl 1.1 arrays, [Esc] - exit, [<-],[->],[Up],[Down] - rotate',0
align 4
ctx1 db 28 dup (0) ;TinyGLContext or KOSGLContext
;sizeof.TinyGLContext = 28

align 4
draw_3d:
stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины
stdcall [glPushMatrix]

	;масштаб и повороты
	stdcall [glTranslatef], 0.0,0.0,0.5
	stdcall [glScalef], [scale], [scale], [scale]
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0
	stdcall [glRotatef], [angle_y],0.0,1.0,0.0
	stdcall [glRotatef], [angle_x],1.0,0.0,0.0

	;рисование через тндексный массив
	mov eax,house_3ds ;начало внедренного файла 3ds
	add eax,0xeb ;смещение по которому идут координаты вершин (получено с использованием программы info_3ds)
	stdcall [glVertexPointer], 3, GL_FLOAT, 0, eax ;задаем массив для вершин, 3 - число координат для одной вершины
	stdcall [glEnableClientState], GL_VERTEX_ARRAY ;включаем режим рисования вершин
	stdcall [glDrawElements], GL_TRIANGLES, 0x1a6*3, GL_UNSIGNED_SHORT, Indices ;mode, count, type, *indices
	stdcall [glDisableClientState], GL_VERTEX_ARRAY ;отключаем режим рисования вершин

stdcall [glPopMatrix]
ret

align 4
scale dd 0.0065 ;начальный масштаб (в идеальном случае должен вычислятся, но для даного примера подобран в ручную на глаз)
delt_sc dd 0.0005
angle_z dd 90.0
angle_y dd 90.0
angle_x dd 0.0
delt_size dd 3.0

align 4
house_3ds: ;внедряем файл внутрь программы (в идеальном случае должен открыватся через окно диалога, но для облегчения примера вшит внутрь)
file '../../../../../demos/3DS/3ds_objects/House.3ds'
align 4
Indices rb 0x1a6*6 ;0x1a6 - число граней, на каждую грань по 3 точки, индекс точки 2 байта

;--------------------------------------------------
align 4
import_lib_tinygl:

macro E_LIB n
{
	n dd sz_#n
}
include '../export.inc'
	dd 0,0
macro E_LIB n
{
	sz_#n db `n,0
}
include '../export.inc'

;--------------------------------------------------
system_path db '/sys/lib/'
name_tgl db 'tinygl.obj',0
err_message_found_lib db 'Sorry I cannot load library ',39,'tinygl.obj',39,0
head_f_i:
head_f_l db 'System error',0
err_message_import db 'Error on load import library ',39,'tinygl.obj',39,0
;--------------------------------------------------

i_end:
	rb 4096
stacktop:
cur_dir_path:
	rb 4096
library_path:
	rb 4096
mem:
