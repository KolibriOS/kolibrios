; SPDX-License-Identifier: GPL-2.0-only
; Flag - demo program shows a flag with the inscription 'KolibriOS'
; Copyright (C) 2025 KolibriOS team

use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,0,cur_dir_path

include '../../proc32.inc'
include '../../macros.inc'
include '../../KOSfuncs.inc'
include '../../load_lib.mac'
include '../../dll.inc'
include '../../develop/libraries/TinyGL/asm_fork/kosgl.inc'
include '../../develop/libraries/TinyGL/asm_fork/opengl_const.inc'
include '../../develop/libraries/TinyGL/asm_fork/examples/fps.inc'

@use_library

;Macro for double type parameters (8 bytes)
macro glpush GLDoubleVar {
	push dword[GLDoubleVar+4]
	push dword[GLDoubleVar]
}

align 4
start:
	load_library name_tgl, library_path, system_path, import_tinygl
	cmp eax,SF_TERMINATE_PROCESS
	jz button.exit

	mcall SF_SET_EVENTS_MASK, 0x27

; *** init ***
	stdcall [kosglMakeCurrent], 0,15,600,380,ctx1

	stdcall [glMatrixMode], GL_MODELVIEW
	call    [glLoadIdentity]

	stdcall [glClearColor], 0.549, 0.549, 0.588, 1.0

	stdcall [glLightfv], GL_LIGHT0, GL_POSITION, lightpos
	stdcall [glLightfv], GL_LIGHT0, GL_SPOT_DIRECTION, lightdirect

	stdcall [glEnable], GL_COLOR_MATERIAL

	glpush  p3
	stdcall [glClearDepth]
	stdcall [glEnable], GL_CULL_FACE
	stdcall [glEnable], GL_DEPTH_TEST

	fninit

	stdcall reshape, 600,380
; *** end init ***


align 4
red_win:
	call draw_window
	mcall SF_THREAD_INFO, procinfo,-1
	mov eax,dword[procinfo.box.height]
	cmp eax,120
	jge @f
		mov eax,120 ;min size
	@@:
	sub eax,43
	mov ebx,dword[procinfo.box.width]
	cmp ebx,200
	jge @f
		mov ebx,200
	@@:
	sub ebx,10
	stdcall reshape, ebx,eax

align 16
still:
	call draw_3d
	cmp dword[stop],1
	je @f
		stdcall Fps, 365,4

		mov dword[esp-4],eax
		fild dword[esp-4]
		fmul dword[delt_3]
		fchs
		fadd dword[dangle]
		fstp dword[dangle] ;dangle -= 0.01*Fps(x,y)

		mcall SF_WAIT_EVENT_TIMEOUT, 1
		jmp .end0
align 4
	@@:
	mcall SF_WAIT_EVENT
	.end0:
	cmp al, EV_REDRAW
	jz red_win
	cmp al, EV_KEY
	jz key
	cmp al, EV_BUTTON
	jz button

	jmp still


; new window size or exposure
align 4
proc reshape, width:dword, height:dword
locals
	dxy dq ?
endl
	stdcall [glViewport], 0, 0, [width], [height]
	stdcall [glMatrixMode], GL_PROJECTION
	call    [glLoadIdentity]
	fild    dword[width]
	fidiv   dword[height]
	fstp    qword[dxy] ;dxy = width/height
	glpush  p4
	glpush  p3
	glpush  dxy
	glpush  p1
	call    [gluPerspective] ;28.0, width/height, 1.0, 40.0

	stdcall [glMatrixMode], GL_MODELVIEW
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT
	ret
endp

align 4
p1 dq 28.0
p3 dq  1.0
p4 dq 40.0

align 4
draw_window:
	pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mcall SF_CREATE_WINDOW, (50 shl 16)+609,(30 shl 16)+425,0x33404040,,title1
	call [kosglSwapBuffers]

	;Title
	mcall SF_DRAW_TEXT, (338 shl 16)+4, 0xc0c0c0, fps,    fps.end-fps
	mcall SF_DRAW_TEXT,   (8 shl 16)+4, 0xc0c0c0, title2, title2.end-title2

	mcall SF_REDRAW,SSF_END_DRAW
	popad
	ret

align 4
key:
	mcall SF_GET_KEY

	cmp ah,27 ;Esc
	je button.exit

	cmp ah,112 ;P
	jne @f
		xor dword[stop],1
		jmp still
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
title1: db 'TinyGL in KolibriOS'
.end: db 0
title2: db 'ESC - exit, P - pause'
.end: db 0
fps:  db 'FPS:'
.end: db 0

align 16
proc draw_3d uses ebx ecx edx esi edi
locals
	z dd ?
endl
	call    [glLoadIdentity]
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT
	stdcall [glTranslatef], 0.0, 0.0, -2.0
	stdcall [glRotatef], 10.0, 0.0, 0.0, 1.0
	stdcall [glRotatef], 43.0, 0.0, 1.0, 0.0
	stdcall [glTranslatef], -0.6, -0.2, -0.1

	mov     edi, logo+9*41 ;edi = logo[9][0]
	mov     esi, 41        ;esi = i
align 4
.cycle0: ;for(int i=0;i<41;i++)
	stdcall [glTranslatef], 0.045, 0.0, 0.0
	call    [glPushMatrix]
	fld     dword[angle]
	fsin
	fmul    dword[delt_1]
	fstp    dword[z] ;= 0.08*sin(angle)

	fld     dword[angle]
	fadd    dword[delt_2]
	fstp    dword[angle] ;angle += 0.2

	mov     ecx, 9
	mov     ebx, edi

; ecx = j
.cycle1: ;for(int j=9;j>=0;j--)
	cmp     byte[ebx],0 ;if(logo[j][i])
	je      @f
	push    0.0 ;b
	push    0.0 ;g
	push    1.0 ;r
	jmp     .end_c2
@@:
	push    0.945 ;b
	push    0.855 ;g
	push    0.859 ;r
.end_c2:
	call    [glColor3f]
	stdcall [glTranslatef], 0.0, 0.045, 0.0
	stdcall [glBegin], GL_QUADS
	stdcall [glVertex3f], 0.0, -0.04,[z]
	stdcall [glVertex3f], 0.04,-0.04,[z]
	stdcall [glVertex3f], 0.04, 0.0, [z]
	stdcall [glVertex3f], 0.0,  0.0, [z]
	call    [glEnd]
	add     ebx, -41
	dec     ecx
	jnz     .cycle1

	call    [glPopMatrix]
	inc     edi ;edi = logo[9][i]
	dec     esi
	jnz     .cycle0

	mov     edx,dword[dangle]
	mov     dword[angle],edx
	call    [kosglSwapBuffers]

	ret 
endp

align 4        
delt_1 dd 0.08
delt_2 dd 0.2
delt_3 dd 0.01

lightpos dd 2.0, 0.0, -2.5, 1.0
lightdirect dd 0.0, 0.0, -0.7
angle dd 0.0
dangle dd 0.0

stop  dd 0

;[10][41]
logo db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db 0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0,0,1,1,1,0,0,0,1,1,1,0,0,0,\
	0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,\
	0,0,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,1,0,0,0,0,\
	0,0,1,1,1,0,0,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,\
	0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,\
	0,0,1,0,0,0,1,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,1,1,1,0,0,0,1,1,1,0,0,0
db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0


;--------------------------------------------------
align 4
import_tinygl:

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

;--------------------------------------------------
system_path db '/sys/lib/'
name_tgl db 'tinygl.obj',0
;--------------------------------------------------

align 16
i_end:
ctx1 TinyGLContext
procinfo process_information 
cur_dir_path rb 4096
library_path rb 4096
	rb 4096
stacktop:
mem:
