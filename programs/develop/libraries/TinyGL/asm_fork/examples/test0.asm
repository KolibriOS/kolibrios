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

stdcall [kosglMakeCurrent], 10,10,300,225,ctx1
stdcall [glEnable], GL_DEPTH_TEST
stdcall [glClearColor], 0.2,0.0,0.2,0.0

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
	mcall 0,(50 shl 16)+330,(30 shl 16)+275,,,caption
	stdcall [kosglSwapBuffers]

	mcall 12,2
	popad
	ret

align 4
key:
	mcall 2

	cmp ah,27 ;Esc
	je button.exit

	;178 ;Up
	;177 ;Down
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
caption db 'Test tinygl library, [Esc] - exit, [<-] and [->] - rotate',0
align 4
ctx1 db 28 dup (0) ;TinyGLContext or KOSGLContext
;sizeof.TinyGLContext = 28

align 4
draw_3d:
stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины

stdcall [glPushMatrix]
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0

	stdcall [glColor3f],1.0, 0.0, 0.0
	stdcall [glBegin],GL_POINTS
	stdcall [glVertex3f], 0.0,    0.5, 0.1
	stdcall [glVertex3f], 0.354,  0.354, 0.1
	stdcall [glVertex3f], 0.5,    0.0, 0.1
	stdcall [glVertex3f], 0.354, -0.354, 0.1
	stdcall [glVertex3f], 0.0,   -0.5, 0.1
	stdcall [glVertex3f], -0.354,-0.354, 0.1
	stdcall [glVertex3f], -0.5,   0.0, 0.1
	stdcall [glVertex3f], -0.354, 0.354, 0.1
	stdcall [glEnd]

	stdcall [glBegin],GL_LINES
	stdcall [glVertex3f], 0,      0.7, 0.3
	stdcall [glVertex3f], 0.495,  0.495, 0.7
	stdcall [glVertex3f], 0.7,    0.0, 0.3
	stdcall [glColor3f],1.0, 1.0, 0.0
	stdcall [glVertex3f], 0.495, -0.495, 0.7
	stdcall [glVertex3f], 0.0,   -0.7, 0.3
	stdcall [glVertex3f], -0.495,-0.495, 0.7
	stdcall [glVertex3f], -0.7,   0.0, 0.3
	stdcall [glColor3f],1.0, 0.0, 0.0
	stdcall [glVertex3f], -0.495, 0.495, 0.7
	stdcall [glEnd]

stdcall [glPopMatrix]
ret

angle_z dd 0.0
delt_size dd 3.0

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
	rb 1024
stacktop:
cur_dir_path:
	rb 4096
library_path:
	rb 4096
mem: