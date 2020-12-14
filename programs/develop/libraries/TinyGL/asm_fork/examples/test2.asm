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
	load_library name_tgl, library_path, system_path, import_lib_tinygl
	cmp eax,SF_TERMINATE_PROCESS
	jz button.exit

	mcall SF_SET_EVENTS_MASK,0x27

stdcall [kosglMakeCurrent], 10,10,300,225,ctx1
stdcall [glEnable], GL_DEPTH_TEST

stdcall [glGenLists],1
mov [obj1],eax
stdcall [glNewList],eax,GL_COMPILE
;---
	stdcall [glColor3f],1.0, 0.0, 0.0
	stdcall [glBegin],GL_LINE_LOOP
		stdcall [glVertex3f], 0,      0.9, 0.1
		stdcall [glVertex3f], 0.636,  0.636, 0.1
		stdcall [glVertex3f], 0.9,    0.0, 0.1
		stdcall [glVertex3f], 0.636, -0.636, 0.1
		stdcall [glVertex3f], 0.0,   -0.9, 0.1
		stdcall [glVertex3f], -0.636,-0.636, 0.1
		stdcall [glVertex3f], -0.9,   0.0, 0.1
		stdcall [glVertex3f], -0.636, 0.636, 0.1
	call [glEnd]

	stdcall [glColor3f],0.0, 0.0, 1.0
	stdcall [glBegin],GL_LINE_LOOP
		stdcall [glVertex3f], 0.0, 1.1, 0.1
		stdcall [glVertex3f], 0.778, 0.778, 0.1
		stdcall [glVertex3f], 2.1, 0.0, 0.1
		stdcall [glVertex3f], 0.778, -0.778, 0.1
		stdcall [glVertex3f], 0.0, -1.1, 0.1
		stdcall [glVertex3f], -0.778, -0.778, 0.1
		stdcall [glVertex3f], -2.1, 0.0, 0.1
		stdcall [glVertex3f], -0.778, 0.778, 0.1
	call [glEnd]
;---
call [glEndList]

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
	stdcall [glScalef], 0.3,0.3,0.3

	stdcall [glCallList],[obj1]

call [glPopMatrix]
ret

align 4
angle_z dd 0.0
delt_size dd 3.0
obj1 dd ?

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
;--------------------------------------------------

align 16
i_end:
ctx1 rb 28 ;TinyGLContext or KOSGLContext
;sizeof.TinyGLContext = 28
cur_dir_path rb 4096
library_path rb 4096
	rb 1024
stacktop:
mem:
