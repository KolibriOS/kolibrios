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
include 'fps.inc'

@use_library

macro matr_cell c_funct,c_param,funct,param, dia
{
	dia dword[esp-4*(c_param*(c_funct-funct)+(1+c_param-param))]
}

align 4
start:
	load_library name_tgl, cur_dir_path, library_path, system_path, \
		err_message_found_lib, head_f_l, import_lib_tinygl, err_message_import, head_f_i
	cmp eax,SF_TERMINATE_PROCESS
	jz button.exit

	mcall SF_SET_EVENTS_MASK,0x27

	stdcall [kosglMakeCurrent], 0,15,400,380,ctx1
	stdcall reshape, 400,380

; *** init ***
	stdcall [glLightfv], GL_LIGHT0, GL_POSITION, pos
	stdcall [glEnable], GL_CULL_FACE
	stdcall [glEnable], GL_LIGHTING
	stdcall [glEnable], GL_LIGHT0
	stdcall [glEnable], GL_DEPTH_TEST

	; make the gears
	stdcall [glGenLists],1
	mov [gear1],eax
	stdcall [glNewList], eax, GL_COMPILE
	stdcall [glMaterialfv], GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red
	stdcall gear, 1.0, 4.0, 1.0, 20, 0.7
	call [glEndList]

	stdcall [glGenLists],1
	mov [gear2],eax
	stdcall [glNewList], eax, GL_COMPILE
	stdcall [glMaterialfv], GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green
	stdcall gear, 0.5, 2.0, 2.0, 10, 0.7
	call [glEndList]

	stdcall [glGenLists],1
	mov [gear3],eax
	stdcall [glNewList], eax, GL_COMPILE
	stdcall [glMaterialfv], GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue
	stdcall gear, 1.3, 2.0, 0.5, 10, 0.7
	call [glEndList]

	stdcall [glEnable], GL_NORMALIZE
; *** end init ***

align 4
red_win:
	call draw_window

align 16
still:
	call draw_3d

	stdcall Fps, 365,4
	mov dword[esp-4],eax
	fild dword[esp-4]
	fmul dword[a2]
	fadd dword[a1]
	fadd dword[angle]
	fstp dword[angle]

	mcall SF_CHECK_EVENT
	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	jmp still

align 4
a1 dd 0.01
a2 dd 0.3

; new window size or exposure
align 4
proc reshape uses ebx ecx, width:dword, height:dword
locals
	h dq ?
	mh dq ?
endl

	stdcall [glViewport], 0, 0, [width], [height]
	stdcall [glMatrixMode], GL_PROJECTION
	stdcall [glLoadIdentity]
	fild dword[height]
	fidiv dword[width]
	fst qword[h] ;h = height / width
	fchs
	fstp qword[mh]
	mov ebx,ebp
	sub ebx,8
	mov ecx,ebp
	sub ecx,16
	stdcall [glFrustum], dword p1, dword p2, ebx, ecx, dword p5, dword p6
	stdcall [glMatrixMode], GL_MODELVIEW
	stdcall [glLoadIdentity]
	stdcall [glTranslatef], 0.0, 0.0, -40.0
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT
	ret
endp

align 4
p1 dq -1.0
p2 dq  1.0
p5 dq  5.0
p6 dq 60.0

align 4
draw_window:
	pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mcall SF_CREATE_WINDOW,(50 shl 16)+409,(30 shl 16)+425,0x33404040,,title1
	stdcall [kosglSwapBuffers]

	;Title
	mcall SF_DRAW_TEXT,(338 shl 16)+4,0xc0c0c0,fps,   fps.end-fps
	mcall SF_DRAW_TEXT,(8 shl 16)+4,0xc0c0c0,title3,title3.end-title3
	;mcall SF_DRAW_TEXT,(180 shl 16)+4,0xc0c0c0,title2,title2.end-title2

	mcall SF_REDRAW,SSF_END_DRAW
	popad
	ret

align 4
key:
	mcall SF_GET_KEY

	cmp ah,27 ;Esc
	je button.exit

	cmp ah,43 ;+
	jne @f
		fld dword[scale]
		fdiv dword[delt_sc]
		fstp dword[scale]
		call draw_3d
	@@:
	cmp ah,61 ;=
	jne @f
		fld dword[scale]
		fdiv dword[delt_sc]
		fstp dword[scale]
		call draw_3d
	@@:
	cmp ah,45 ;-
	jne @f
		fld dword[scale]
		fmul dword[delt_sc]
		fstp dword[scale]
		call draw_3d
	@@:
	cmp ah,178 ;Up
	jne @f
		fld dword[view_rotx]
		fadd dword[delt_size]
		fstp dword[view_rotx]
		call draw_3d
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[view_rotx]
		fsub dword[delt_size]
		fstp dword[view_rotx]
		call draw_3d
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[view_roty]
		fadd dword[delt_size]
		fstp dword[view_roty]
		call draw_3d
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[view_roty]
		fsub dword[delt_size]
		fstp dword[view_roty]
		call draw_3d
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
title2: db 'F full screen'
.end: db 0
title3: db 'ESC - exit   Arrow keys - rotate   +/- zoom'
.end: db 0
fps:	db 'FPS:'
.end: db 0

align 4
ctx1 db 28 dup (0) ;TinyGLContext or KOSGLContext
;sizeof.TinyGLContext = 28

align 16
draw_3d:
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT

	stdcall [glPushMatrix]
	stdcall [glScalef],  [scale], [scale], [scale]
	stdcall [glRotatef], [view_rotx], 1.0, 0.0, 0.0
	stdcall [glRotatef], [view_roty], 0.0, 1.0, 0.0
	stdcall [glRotatef], [view_rotz], 0.0, 0.0, 1.0

	stdcall [glPushMatrix]
	stdcall [glTranslatef], -3.0, -2.0, 0.0
	stdcall [glRotatef], [angle], 0.0, 0.0, 1.0
	stdcall [glCallList],[gear1]
	stdcall [glPopMatrix]

	stdcall [glPushMatrix]
	stdcall [glTranslatef], 3.1, -2.0, 0.0
	push dword 1.0
	push dword 0.0
	push dword 0.0
	finit
	fld1
	fld1
	faddp
	fchs
	fld dword[angle]
	fmulp
	fsub dword[an_9]
	fstp dword[esp-4]
	sub esp,4
	call [glRotatef] ;, -2.0*angle-9.0, 0.0, 0.0, 1.0
	stdcall [glCallList],[gear2]
	stdcall [glPopMatrix]

	stdcall [glPushMatrix]
	stdcall [glTranslatef], -3.1, 4.2, 0.0
	push dword 1.0
	push dword 0.0
	push dword 0.0
	finit
	fld1
	fld1
	faddp
	fchs
	fld dword[angle]
	fmulp
	fsub dword[an_25]
	fstp dword[esp-4]
	sub esp,4
	call [glRotatef] ;, -2.0*angle-25.0, 0.0, 0.0, 1.0
	stdcall [glCallList],[gear3]
	stdcall [glPopMatrix]

	stdcall [glPopMatrix]

	stdcall [kosglSwapBuffers]

;   count++;
;   if (count==limit) {
;       exit(0);
	ret

align 4
an_9 dd 9.0
an_25 dd 25.0
scale dd 1.0 ;???
delt_sc dd 0.85 ;???
delt_size dd 5.0

align 4
pos dd 5.0, 5.0, 10.0, 0.0
red dd 0.8, 0.1, 0.0, 1.0
green dd 0.0, 0.8, 0.2, 1.0
blue  dd 0.2, 0.2, 1.0, 1.0

view_rotx dd 20.0
view_roty dd 30.0
view_rotz dd  0.0
gear1 dd ?
gear2 dd ?
gear3 dd ?
angle dd 0.0

limit dd ?
count dd 1

;
;  Draw a gear wheel.  You'll probably want to call this function when
;  building a display list since we do a lot of trig here.
; 
;  Input:  inner_radius - radius of hole at center
;          outer_radius - radius at center of teeth
;          width - width of gear
;          teeth - number of teeth
;          tooth_depth - depth of tooth
;
align 4
proc gear uses eax ebx, inner_radius:dword, outer_radius:dword, width:dword, teeth:dword, tooth_depth:dword
locals
	i dd ?
	r0 dd ?
	r1 dd ?
	r2 dd ?
	angle dd ?
	da dd ?
	u dd ?
	v dd ?
endl

	finit
	mov eax,[inner_radius]
	mov [r0],eax
	fld1
	fld1
	faddp
	fld dword[tooth_depth]
	fdiv st0,st1
	fld dword[outer_radius]
	fsub st0,st1
	fst  dword[r1] ;r1 = outer_radius - tooth_depth/2.0
	fadd dword[tooth_depth]
	fstp dword[r2] ;r2 = outer_radius + tooth_depth/2.0
	ffree st0 ;st0 = tooth_depth/2.0
	fincstp
	;st0 = 2.0

	fldpi
	fidiv dword[teeth]
	fdiv st0,st1
	fstp dword[da] ;da = M_PI / teeth / 2.0

	stdcall [glShadeModel], GL_FLAT

	stdcall [glNormal3f], 0.0, 0.0, 1.0

	mov ebx,[teeth]

	; draw front face
	stdcall [glBegin], GL_QUAD_STRIP
	mov dword[i],0
	@@:
	cmp dword[i],ebx
	jg @f
		finit
		fld1
		fld1
		faddp

		fldpi
		fmul st0,st1
		fimul dword[i]
		fidiv dword[teeth]
		fst dword[angle] ;angle = i * 2.0*M_PI / teeth

		fld dword[width]
		fdiv st0,st2
		matr_cell 4,3,1,3,fst
		matr_cell 4,3,2,3,fst
		matr_cell 4,3,3,3,fst
		matr_cell 4,3,4,3,fstp

		fld st0
		fsin
		fmul dword[r0]
		matr_cell 4,3,1,2,fst
		matr_cell 4,3,3,2,fstp
		fld st0
		fcos
		fmul dword[r0]
		matr_cell 4,3,1,1,fst
		matr_cell 4,3,3,1,fstp

		fld st0
		fsin
		fmul dword[r1]
		matr_cell 4,3,2,2,fstp
		fld st0
		fcos
		fmul dword[r1]
		matr_cell 4,3,2,1,fstp

		;st0 = angle
		fadd dword[da]
		fadd dword[da]
		fadd dword[da]
		fld st0
		fsin
		fmul dword[r1]
		matr_cell 4,3,4,2,fstp
		fcos
		fmul dword[r1]
		matr_cell 4,3,4,1,fstp

		sub esp,48 ;12*4

		call [glVertex3f] ;, r0*cos(angle), r0*sin(angle), width*0.5
		call [glVertex3f] ;, r1*cos(angle), r1*sin(angle), width*0.5
		call [glVertex3f] ;, r0*cos(angle), r0*sin(angle), width*0.5
		call [glVertex3f] ;, r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5
		inc dword[i]
		jmp @b
	@@:
	call [glEnd]

	; draw front sides of teeth
	stdcall [glBegin], GL_QUADS
	mov dword[i],0
	@@:
	cmp dword[i],ebx
	jg @f
		finit
		fld1
		fld1
		faddp

		fldpi
		fmul st0,st1
		fimul dword[i]
		fidiv dword[teeth]
		fst dword[angle] ;angle = i * 2.0*M_PI / teeth

		fld dword[width]
		fdiv st0,st2
		matr_cell 4,3,1,3,fst
		matr_cell 4,3,2,3,fst
		matr_cell 4,3,3,3,fst
		matr_cell 4,3,4,3,fstp

		fld st0
		fcos
		fmul dword[r1]
		matr_cell 4,3,1,1,fstp
		fld st0
		fsin
		fmul dword[r1]
		matr_cell 4,3,1,2,fstp

		fadd dword[da] ;st0 = angle+da
		fld st0
		fcos
		fmul dword[r2]
		matr_cell 4,3,2,1,fstp
		fld st0
		fsin
		fmul dword[r2]
		matr_cell 4,3,2,2,fstp

		fadd dword[da] ;st0 = angle+2*da
		fld st0
		fcos
		fmul dword[r2]
		matr_cell 4,3,3,1,fstp
		fld st0
		fsin
		fmul dword[r2]
		matr_cell 4,3,3,2,fstp

		fadd dword[da] ;st0 = angle+3*da
		fld st0
		fcos
		fmul dword[r1]
		matr_cell 4,3,4,1,fstp
		fsin
		fmul dword[r1]
		matr_cell 4,3,4,2,fstp

		sub esp,48 ;12*4

		call [glVertex3f];, r1*cos(angle),      r1*sin(angle),      width*0.5
		call [glVertex3f];, r2*cos(angle+da),   r2*sin(angle+da),   width*0.5
		call [glVertex3f];, r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5
		call [glVertex3f];, r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5
		inc dword[i]
		jmp @b
	@@:
	call [glEnd]

	stdcall [glNormal3f], 0.0, 0.0, -1.0

	; draw back face
	stdcall [glBegin], GL_QUAD_STRIP
	mov dword[i],0
	@@:
	cmp dword[i],ebx
	jg @f
		finit
		fld1
		fld1
		faddp

		fldpi
		fmul st0,st1
		fimul dword[i]
		fidiv dword[teeth]
		fst dword[angle] ;angle = i * 2.0*M_PI / teeth

		fld dword[width]
		fdiv st0,st2
		fchs
		matr_cell 4,3,1,3,fst
		matr_cell 4,3,2,3,fst
		matr_cell 4,3,3,3,fst
		matr_cell 4,3,4,3,fstp

		fld st0
		fsin
		fld st0
		fmul dword[r1]
		matr_cell 4,3,1,2,fstp
		fmul dword[r0]
		matr_cell 4,3,2,2,fst
		matr_cell 4,3,4,2,fstp

		fld st0
		fcos
		fld st0
		fmul dword[r1]
		matr_cell 4,3,1,1,fstp
		fmul dword[r0]
		matr_cell 4,3,2,1,fst
		matr_cell 4,3,4,1,fstp

		;st0 = angle
		fadd dword[da]
		fadd dword[da]
		fadd dword[da]
		fld st0
		fsin
		fmul dword[r1]
		matr_cell 4,3,3,2,fstp
		fcos
		fmul dword[r1]
		matr_cell 4,3,3,1,fstp

		sub esp,48 ;12*4

		call [glVertex3f];, r1*cos(angle), r1*sin(angle), -width*0.5
		call [glVertex3f];, r0*cos(angle), r0*sin(angle), -width*0.5
		call [glVertex3f];, r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5
		call [glVertex3f];, r0*cos(angle), r0*sin(angle), -width*0.5
		inc dword[i]
		jmp @b
	@@:
	call [glEnd]

	; draw back sides of teeth
	stdcall [glBegin], GL_QUADS
	mov dword[i],0
	@@:
	cmp dword[i],ebx
	jg @f
		finit
		fld1
		fld1
		faddp

		fldpi
		fmul st0,st1
		fimul dword[i]
		fidiv dword[teeth]
		fst dword[angle] ;angle = i * 2.0*M_PI / teeth

		fld dword[width]
		fdiv st0,st2
		fchs
		matr_cell 4,3,1,3,fst
		matr_cell 4,3,2,3,fst
		matr_cell 4,3,3,3,fst
		matr_cell 4,3,4,3,fstp

		fld st0
		fcos
		fmul dword[r1]
		matr_cell 4,3,4,1,fstp
		fld st0
		fsin
		fmul dword[r1]
		matr_cell 4,3,4,2,fstp

		fadd dword[da] ;st0 = angle+da
		fld st0
		fcos
		fmul dword[r2]
		matr_cell 4,3,3,1,fstp
		fld st0
		fsin
		fmul dword[r2]
		matr_cell 4,3,3,2,fstp

		fadd dword[da] ;st0 = angle+2*da
		fld st0
		fcos
		fmul dword[r2]
		matr_cell 4,3,2,1,fstp
		fld st0
		fsin
		fmul dword[r2]
		matr_cell 4,3,2,2,fstp

		fadd dword[da] ;st0 = angle+3*da
		fld st0
		fcos
		fmul dword[r1]
		matr_cell 4,3,1,1,fstp
		fsin
		fmul dword[r1]
		matr_cell 4,3,1,2,fstp

		sub esp,48 ;12*4

		call [glVertex3f];, r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5
		call [glVertex3f];, r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5
		call [glVertex3f];, r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5
		call [glVertex3f];, r1*cos(angle),      r1*sin(angle),      -width*0.5
		inc dword[i]
		jmp @b
	@@:
	call [glEnd]

	; draw outward faces of teeth
	stdcall [glBegin], GL_QUAD_STRIP
	mov dword[i],0
	@@:
	cmp dword[i],ebx
	jg @f
		finit
		fld1
		fld1
		faddp

		fldpi
		fmul st0,st1
		fimul dword[i]
		fidiv dword[teeth]
		fst dword[angle] ;angle = i * 2.0*M_PI / teeth

		fld dword[width]
		fdiv st0,st2
		matr_cell 12,3, 1,3,fst
		matr_cell 12,3, 4,3,fst
		matr_cell 12,3, 7,3,fst
		matr_cell 12,3,10,3,fst
		fchs
		matr_cell 12,3, 2,3,fst
		matr_cell 12,3, 5,3,fst
		matr_cell 12,3, 8,3,fst
		matr_cell 12,3,11,3,fstp
		fldz
		matr_cell 12,3, 3,3,fst
		matr_cell 12,3, 6,3,fst
		matr_cell 12,3, 9,3,fst
		matr_cell 12,3,12,3,fstp

		fld st0
		fsin
		matr_cell 12,3, 6,2,fst
		matr_cell 12,3,12,2,fst
		fmul dword[r1]
		matr_cell 12,3, 1,2,fst
		matr_cell 12,3, 2,2,fstp

		fld st0
		fcos
		matr_cell 12,3, 6,1,fst
		matr_cell 12,3,12,1,fst
		fmul dword[r1]
		matr_cell 12,3, 1,1,fst
		matr_cell 12,3, 2,1,fstp

		fadd dword[da] ;st0 = angle+da
		fld st0
		fcos
		fmul dword[r2]
		matr_cell 12,3, 4,1,fst
		matr_cell 12,3, 5,1,fst
		matr_cell 12,3, 1,1,fsub
		fstp dword[u]

		fld st0
		fsin
		fmul dword[r2]
		matr_cell 12,3, 4,2,fst
		matr_cell 12,3, 5,2,fst
		matr_cell 12,3, 1,2,fsub
		fst  dword[v]

		fmul st0,st0
		fld  dword[u]
		fmul st0,st0
		faddp
		fsqrt ;st0 = len
		fld  dword[u]
		fdiv st0,st1
		fchs
		matr_cell 12,3, 3,2,fstp
		fld  dword[v]
		fdiv st0,st1
		matr_cell 12,3, 3,1,fstp
		ffree st0 ;len
		fincstp

		fadd dword[da] ;st0 = angle+2*da
		fld st0
		fcos
		fmul dword[r2]
		matr_cell 12,3, 7,1,fst
		matr_cell 12,3, 8,1,fstp
		fld st0
		fsin
		fmul dword[r2]
		matr_cell 12,3, 7,2,fst
		matr_cell 12,3, 8,2,fstp

		fadd dword[da] ;st0 = angle+3*da
		fld st0
		fcos
		fmul dword[r1]
		matr_cell 12,3,10,1,fst
		matr_cell 12,3,11,1,fst
		matr_cell 12,3, 7,1,fsub
		fchs
		matr_cell 12,3, 9,2,fstp ;-u

		fsin
		fmul dword[r1]
		matr_cell 12,3,10,2,fst
		matr_cell 12,3,11,2,fst
		matr_cell 12,3, 7,2,fsub
		matr_cell 12,3, 9,1,fstp ;v

		sub esp,144 ;12*12

		call [glVertex3f];, r1*cos(angle), r1*sin(angle), width*0.5
		call [glVertex3f];, r1*cos(angle), r1*sin(angle), -width*0.5
;u = r2*cos(angle+da) - r1*cos(angle)
;v = r2*sin(angle+da) - r1*sin(angle)
;len = sqrt( u*u + v*v )
;u /= len
;v /= len
		call [glNormal3f];, v, -u, 0.0
		call [glVertex3f];, r2*cos(angle+da),   r2*sin(angle+da),    width*0.5
		call [glVertex3f];, r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5
		call [glNormal3f];, cos(angle), sin(angle), 0.0
		call [glVertex3f];, r2*cos(angle+2*da), r2*sin(angle+2*da),  width*0.5
		call [glVertex3f];, r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5
;u = r1*cos(angle+3*da) - r2*cos(angle+2*da)
;v = r1*sin(angle+3*da) - r2*sin(angle+2*da)
		call [glNormal3f];, v, -u, 0.0
		call [glVertex3f];, r1*cos(angle+3*da), r1*sin(angle+3*da),  width*0.5
		call [glVertex3f];, r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5
		call [glNormal3f];, cos(angle), sin(angle), 0.0
		inc dword[i]
		jmp @b
	@@:

	finit
	fld1
	fld1
	faddp

	fld dword[width]
	fdiv st0,st1
	matr_cell 2,3,1,3,fst
	fchs
	matr_cell 2,3,2,3,fstp
	fldz
	matr_cell 2,3,1,2,fst
	matr_cell 2,3,2,2,fstp
	fldz
	fcos
	fmul dword[r1]
	matr_cell 2,3,1,1,fst
	matr_cell 2,3,2,1,fstp

	sub esp,24

	call [glVertex3f];, r1*cos(0), r1*sin(0), width*0.5
	call [glVertex3f];, r1*cos(0), r1*sin(0), -width*0.5

	call [glEnd]


	stdcall [glShadeModel], GL_SMOOTH

	; draw inside radius cylinder
	stdcall [glBegin], GL_QUAD_STRIP
	mov dword[i],0
	finit
	@@:
	cmp dword[i],ebx
	jg @f
		fld1
		fld1
		faddp

		fldpi
		fmul st0,st1
		fimul dword[i]
		fidiv dword[teeth]
		fst dword[angle] ;angle = i * 2.0*M_PI / teeth

		fldz
		matr_cell 3,3,1,3,fstp
		fld dword[width]
		fdiv st0,st2
		matr_cell 3,3,3,3,fst
		fchs
		matr_cell 3,3,2,3,fstp

		fld st0
		fsin
		fchs
		matr_cell 3,3,1,2,fst
		fchs
		fmul dword[r0]
		matr_cell 3,3,2,2,fst
		matr_cell 3,3,3,2,fstp

		fcos
		fchs
		matr_cell 3,3,1,1,fst
		fchs
		fmul dword[r0]
		matr_cell 3,3,2,1,fst
		matr_cell 3,3,3,1,fstp

		sub esp,36 ;12*3

		call [glNormal3f];, -cos(angle), -sin(angle), 0.0
		call [glVertex3f];, r0*cos(angle), r0*sin(angle), -width*0.5
		call [glVertex3f];, r0*cos(angle), r0*sin(angle), width*0.5
		ffree st0 ;2.0
		fincstp
		inc dword[i]
		jmp @b
	@@:
	call [glEnd]

	ret	 
endp



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

align 16
i_end:
	rb 4096
stacktop:
cur_dir_path:
	rb 4096
library_path:
	rb 4096
mem:
