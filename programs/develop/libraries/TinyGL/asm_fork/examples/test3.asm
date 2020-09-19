use32
	org 0
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

stdcall [kosglMakeCurrent], 10,10,300,225,ctx1
;stdcall [glEnable], GL_DEPTH_TEST
stdcall [glClearColor], 0.2,0.0,0.2,0.0
;stdcall [glShadeModel],GL_FLAT

call draw_3d

align 4
red_win:
	call draw_window

align 16
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

	mov edx,0x33ffffff ;0x73ffffff
	mcall SF_CREATE_WINDOW,(50 shl 16)+330,(30 shl 16)+275,,,caption
	call [kosglSwapBuffers]

	mcall SF_REDRAW,SSF_END_DRAW
	popad
	ret

align 4
key:
	mcall SF_GET_KEY

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
		call [kosglSwapBuffers]
		jmp still
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_z]
		fsub dword[delt_size]
		fstp dword[angle_z]
		call draw_3d
		call [kosglSwapBuffers]
		;jmp still
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
caption db 'Test tinygl library, [Esc] - exit, [<-] and [->] - rotate',0

align 4
draw_3d:
stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины

call [glPushMatrix]
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0

	stdcall [glBegin],GL_TRIANGLES

	stdcall [glColor3f],0.0, 0.0, 1.0
	stdcall [glVertex3f], 0.0,   0.5,   0.1
	stdcall [glVertex3f], 0.475, 0.823, 0.1
	stdcall [glVertex3f], 0.433, 0.25,  0.1

	stdcall [glColor3f],0.0, 1.0, 0.0
	stdcall [glVertex3f], 0.5,   0.0,   0.1
	stdcall [glVertex3f], 0.823,-0.475, 0.1
	stdcall [glVertex3f], 0.25, -0.433, 0.1

	stdcall [glColor3f],1.0, 0.0, 0.0
	stdcall [glVertex3f], 0.0,  -0.5,   0.1
	stdcall [glVertex3f], -0.475,-0.823,0.1
	stdcall [glVertex3f], -0.433,-0.25, 0.1

	stdcall [glVertex3f], -0.5,   0.0,   0.1
	stdcall [glColor3f],1.0, 1.0, 0.0
	stdcall [glVertex3f], -0.823, 0.475, 0.1
	stdcall [glColor3f],1.0, 1.0, 1.0
	stdcall [glVertex3f], -0.25,  0.433, 0.1

	call [glEnd]

call [glPopMatrix]
ret

angle_z dd 15.0
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

head_f_i:
head_f_l db '"System error',0
err_message_import db 'Error on load import library ',39,'tinygl.obj',39,'" -tE',0
err_message_found_lib db 'Sorry I cannot load library ',39,'tinygl.obj',39,'" -tE',0
;--------------------------------------------------

align 16
i_end:
ctx1 db 28 dup (0) ;TinyGLContext or KOSGLContext
;sizeof.TinyGLContext = 28
	rb 1024
stacktop:
cur_dir_path:
	rb 4096
library_path:
	rb 4096
mem:
