use32
	org 0x0
	db 'MENUET01'
	dd 0x1
	dd start
	dd i_end
	dd mem,stacktop
	dd 0,cur_dir_path

include '../../../../../../programs/proc32.inc'
include '../../../../../../programs/macros.inc'
include '../../../../../../programs/develop/libraries/box_lib/load_lib.mac'
include '../../../../../../programs/dll.inc'
include '../opengl_const.inc'

@use_library

align 4
start:
	load_library name_tgl, cur_dir_path, library_path, system_path, \
		err_message_found_lib, head_f_l, import_lib_tinygl, err_message_import, head_f_i
	cmp eax,-1
	jz button.exit

	mcall 40,0x27

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
	mcall 10
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
	mcall 12,1

	mov edx,0x33ffffff ;0x73ffffff
	mcall 0,(50 shl 16)+430,(30 shl 16)+400,,,caption
	stdcall [kosglSwapBuffers]

	mcall 12,2
	popad
	ret

align 4
key:
	mcall 2

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
		fld dword[angle_z]
		fadd dword[delt_size]
		fstp dword[angle_z]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_z]
		fsub dword[delt_size]
		fstp dword[angle_z]
		call draw_3d
		stdcall [kosglSwapBuffers]
	@@:

	jmp still

align 4
button:
	mcall 17
	cmp ah,1
	jne still
.exit:
	mcall -1


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

	;рисование через массивы
	stdcall [glVertexPointer], 2, GL_FLOAT, 0, Vertex ;задаем массив для вершин, 2 - число координат для одной вершины
	stdcall [glColorPointer],  3, GL_FLOAT, 0, Colors ;задаем массив для цветов, 3 - число параметров для одной точки
	stdcall [glEnableClientState], GL_VERTEX_ARRAY    ;включаем режим рисования вершин
	stdcall [glEnableClientState], GL_COLOR_ARRAY     ;включаем режим рисования цветов
	stdcall [glDrawArrays], GL_POLYGON, 0, 4 ;рисование полигона из 4-х вершин (равносильно вызову 8-ми функций: 4 - для вершин, 4 - для цетов)
	stdcall [glDisableClientState], GL_COLOR_ARRAY    ;отключаем режим рисования цветов
	stdcall [glDisableClientState], GL_VERTEX_ARRAY   ;отключаем режим рисования вершин

stdcall [glPopMatrix]
ret

align 4
scale dd 0.8
delt_sc dd 0.05
angle_z dd 0.0
angle_y dd 0.0
delt_size dd 3.0

align 4
Vertex dd -0.9, -0.9, -0.9, 0.9, 0.9, 0.9, 0.9, -0.9 ;4 вершины (по 2 координаты)
Colors dd 0.0, 0.5, 1.0, 1.0, 0.0, 0.5, 1.0, 1.0, 1.0, 0.5, 1.0, 0.0 ;4 цвета (по 3 параметра)

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
err_message_found_lib db 'Sorry I cannot load library tinygl.obj',0
head_f_i:
head_f_l db 'System error',0
err_message_import db 'Error on load import library tinygl.obj',0
;--------------------------------------------------

i_end:
	rb 4096
stacktop:
cur_dir_path:
	rb 4096
library_path:
	rb 4096
mem:
