use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,0,cur_dir_path

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../load_lib.mac'
include '../../../../../dll.inc'
include '../opengl_const.inc'

@use_library

align 4
start:
	load_library name_tgl, library_path, system_path, import_tinygl
	cmp eax,-1
	jz button.exit

	mcall SF_SET_EVENTS_MASK,0x27

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

	mcall SF_CREATE_WINDOW,(50 shl 16)+430,(30 shl 16)+400,0x33ffffff,,caption
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
	    fld dword[scale]
	    fadd dword[delt_sc]
	    fstp dword[scale]
	    call draw_3d
	    call [kosglSwapBuffers]
	@@:
	cmp ah,45 ;-
	jne @f
	    fld dword[scale]
	    fsub dword[delt_sc]
	    fstp dword[scale]
	    call draw_3d
	    call [kosglSwapBuffers]
	@@:
	cmp ah,178 ;Up
	jne @f
		fld dword[angle_y]
		fadd dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[angle_y]
		fsub dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[angle_z]
		fadd dword[delt_size]
		fstp dword[angle_z]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_z]
		fsub dword[delt_size]
		fstp dword[angle_z]
		call draw_3d
		call [kosglSwapBuffers]
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
draw_3d:
stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины
call [glPushMatrix]

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

call [glPopMatrix]
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
import_tinygl:

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
;--------------------------------------------------

align 4
i_end:
	ctx1 rb 28 ;sizeof.TinyGLContext = 28
	cur_dir_path rb 4096
	library_path rb 4096
	rb 4096
stacktop:
mem:
