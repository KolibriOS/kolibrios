struct GLUquadricObj
	DrawStyle   dd ? ; GLU_FILL, LINE, SILHOUETTE, or POINT
	Orientation dd ? ; GLU_INSIDE or GLU_OUTSIDE
	TextureFlag dd ? ; Generate texture coords?
	Normals     dd ? ; GLU_NONE, GLU_FLAT, or GLU_SMOOTH
	ErrorFunc   dd ? ; Error handler callback function
ends

offs_qobj_DrawStyle equ 0
offs_qobj_Orientation equ 4
offs_qobj_TextureFlag equ 8
offs_qobj_Normals equ 12
offs_qobj_ErrorFunc equ 16

;void drawTorus(float rc, int numc, float rt, int numt)
;{
;}

;static void normal3f(GLfloat x, GLfloat y, GLfloat z )
;{
;}

;void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar )
;{
;}

;void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
;	  GLdouble centerx, GLdouble centery, GLdouble centerz,
;	  GLdouble upx, GLdouble upy, GLdouble upz)
;{
;}

align 16
gluNewQuadric:
	stdcall gl_malloc, sizeof.GLUquadricObj
	or eax,eax
	jz @f
		mov dword[eax+offs_qobj_DrawStyle],GLU_FILL
		mov dword[eax+offs_qobj_Orientation],GLU_OUTSIDE
		mov dword[eax+offs_qobj_TextureFlag],GL_FALSE
		mov dword[eax+offs_qobj_Normals],GLU_SMOOTH
		mov dword[eax+offs_qobj_ErrorFunc],0 ;NULL
	@@:
	ret

align 16
proc gluDeleteQuadric, state:dword
	cmp dword[state],0
	je @f
		stdcall gl_free,[state]
	@@:
	ret
endp

;
; Set the drawing style to be GLU_FILL, GLU_LINE, GLU_SILHOUETTE,
; or GLU_POINT.
;
align 16
proc gluQuadricDrawStyle uses eax ebx, qobj:dword, drawStyle:dword
	mov eax,[qobj]
	or eax,eax
	jz .err_q
	mov ebx,[drawStyle]
	cmp ebx,GLU_FILL
	je @f
	cmp ebx,GLU_LINE
	je @f
	cmp ebx,GLU_SILHOUETTE
	je @f
	cmp ebx,GLU_POINT
	je @f
	jmp .err_q
align 4
	@@:
		mov dword[eax+offs_qobj_DrawStyle],ebx
		jmp @f
align 4
	.err_q:
		stdcall dbg_print,sz_gluQuadricDrawStyle,err_9
	@@:
	ret
endp

;
; Set the orientation to GLU_INSIDE or GLU_OUTSIDE.
;
align 16
proc gluQuadricOrientation uses eax ebx, qobj:dword, orientation:dword
	mov eax,[qobj]
	or eax,eax
	jz .err_q
	mov ebx,[orientation]
	cmp ebx,GLU_INSIDE
	je @f
	cmp ebx,GLU_OUTSIDE
	je @f
	jmp .err_q
align 4
	@@:
		mov dword[eax+offs_qobj_Orientation],ebx
		jmp @f
align 4
	.err_q:
		stdcall dbg_print,sz_gluQuadricOrientation,err_9
	@@:
	ret
endp

align 16
proc gluQuadricTexture uses eax ebx, qobj:dword, texture:dword
	mov eax,[qobj]
	or eax,eax
	jz .err_q
	mov ebx,[texture]
	cmp ebx,GL_TRUE
	je @f
	cmp ebx,GL_FALSE
	je @f
	@@:
		mov dword[eax+offs_qobj_TextureFlag],ebx
		jmp @f
align 4
	.err_q:
		stdcall dbg_print,sz_gluQuadricTexture,err_9
	@@:
	ret
endp

align 16
proc gluCylinder qobj:dword, baseRadius:dword, topRadius:dword, height:dword,\
	slices:dword, stacks:dword
locals
	da dq ? ;double
	r dq ? ;double
	dr dq ? ;double
	dz dq ? ;double
	x dd ? ;float
	y dd ? ;float
	z dd ? ;float
	nz dd ? ;float
	nsign dd ? ;float
	i dd ? ;GLint
	j dd ? ;GLint
	du dd ? ;float
	dv dd ? ;float
	tcx dd ? ;float
	tcy dd ? ;float
	x2 dd ? ;float
	y2 dd ? ;float
endl
pushad
	mov edx,[qobj]
	fld1
	cmp dword[edx+offs_qobj_Orientation],GLU_INSIDE
	jne @f
		fchs
	@@:
	fstp dword[nsign]

	fldpi
	fadd st0,st0
	fidiv dword[slices]
	fstp qword[da] ;da = 2.0*M_PI / slices
	mov ebx,[topRadius]
	fld qword[ebx]
	mov ecx,[baseRadius]
	fsub qword[ecx]
	fld st0 ;copy: topRadius-baseRadius
	fidiv dword[stacks]
	fstp qword[dr] ;dr = (topRadius-baseRadius) / stacks
	mov eax,[height]
	fld qword[eax]
	fidiv dword[stacks]
	fstp qword[dz] ;dz = height / stacks
	fchs
	fdiv qword[eax]
	fstp dword[nz] ;nz = (baseRadius-topRadius) / height ; Z component of normal vectors

	cmp dword[edx+offs_qobj_DrawStyle],GLU_POINT
	jne .else0
		stdcall glBegin,GL_POINTS
		mov ebx,[slices]
		mov dword[i],0
align 4
		.cycle_0: ;for (i=0;i<slices;i++)
			cmp [i],ebx
			jge .cycle_0_end
			fild dword[i]
			fmul qword[da]
			fld st0
			fcos
			fst dword[x] ;x = cos(i*da)
			fmul dword[nsign]
			fstp dword[esp-12]
			fsin
			fst dword[y] ;y = sin(i*da)
			fmul dword[nsign]
			fstp dword[esp-8]
			fld dword[nz]
			fmul dword[nsign]
			fstp dword[esp-4]
			sub esp,12
			call glNormal3f ;x*nsign, y*nsign, nz*nsign

			mov dword[z],0.0
			mov ecx,[baseRadius]
			fld qword[ecx]
			fstp qword[r] ;r = baseRadius
			mov ecx,[stacks]
			inc ecx
align 4
			.cycle_1: ;for (j=0;j<=stacks;j++)
				mov eax,[z]
				mov [esp-4],eax
				fld qword[r]
				fld st0
				fmul dword[y]
				fstp dword[esp-8]
				fmul dword[x]
				fstp dword[esp-12]
				sub esp,12
				call glVertex3f ;x*r, y*r, z
				fld dword[z]
				fadd qword[dz]
				fstp dword[z] ;z += dz
				fld qword[r]
				fadd qword[dr]
				fstp qword[r] ;r += dr
				dec ecx
				jnz .cycle_1
			inc dword[i]
			jmp .cycle_0
align 4
		.cycle_0_end:
		call glEnd
		jmp .end_f
	.else0:
	cmp dword[edx+offs_qobj_DrawStyle],GLU_LINE
	je @f
	cmp dword[edx+offs_qobj_DrawStyle],GLU_SILHOUETTE
	je .else2
		jmp .else1
	@@:
		; Draw rings
		mov dword[z],0.0
		mov ecx,[baseRadius]
		fld qword[ecx]
		fstp qword[r] ;r = baseRadius
		mov ecx,[stacks]
		inc ecx
align 4
		.cycle_2: ;for (j=0;j<=stacks;j++)
			stdcall glBegin,GL_LINE_LOOP
			mov ebx,[slices]
			mov dword[i],0
align 4
			.cycle_3: ;for (i=0;i<slices;i++)
				cmp [i],ebx
				jge .cycle_3_end
				fild dword[i]
				fmul qword[da]
				fld st0
				fcos
				fst dword[x] ;x = cos(i*da)
				fmul dword[nsign]
				fstp dword[esp-12]
				fsin
				fst dword[y] ;y = sin(i*da)
				fmul dword[nsign]
				fstp dword[esp-8]
				fld dword[nz]
				fmul dword[nsign]
				fstp dword[esp-4]
				sub esp,12
				call glNormal3f ;x*nsign, y*nsign, nz*nsign
				mov eax,[z]
				mov [esp-4],eax
				fld qword[r]
				fld st0
				fmul dword[y]
				fstp dword[esp-8]
				fmul dword[x]
				fstp dword[esp-12]
				sub esp,12
				call glVertex3f ;x*r, y*r, z
				inc dword[i]
				jmp .cycle_3
align 4
			.cycle_3_end:
			call glEnd
			fld dword[z]
			fadd qword[dz]
			fstp dword[z] ;z += dz
			fld qword[r]
			fadd qword[dr]
			fstp qword[r] ;r += dr
			dec ecx
			jnz .cycle_2
			jmp .else2_end
align 4
		.else2:
		; draw one ring at each end
;     if (baseRadius!=0.0) {
			stdcall glBegin,GL_LINE_LOOP
align 4
			.cycle_4: ;for (i=0;i<slices;i++)
;		{
;           x = cos(i*da);
;           y = sin(i*da);
;           normal3f( x*nsign, y*nsign, nz*nsign );
;           glVertex3f( x*baseRadius, y*baseRadius, 0.0 );
;        }
			call glEnd
			stdcall glBegin,GL_LINE_LOOP
align 4
			.cycle_5: ;for (i=0;i<slices;i++)
;		{
;           x = cos(i*da);
;           y = sin(i*da);
;           normal3f( x*nsign, y*nsign, nz*nsign );
;           glVertex3f( x*topRadius, y*topRadius, height );
;        }
			call glEnd
;     }
		.else2_end:
		; draw length lines
		stdcall glBegin,GL_LINES
		mov ebx,[slices]
		mov dword[i],0
align 4
		.cycle_6: ;for (i=0;i<slices;i++)
			cmp [i],ebx
			jge .cycle_6_end
			fild dword[i]
			fmul qword[da]
			fld st0
			fcos
			fst dword[x] ;x = cos(i*da)
			fmul dword[nsign]
			fstp dword[esp-12]
			fsin
			fst dword[y] ;y = sin(i*da)
			fmul dword[nsign]
			fstp dword[esp-8]
			fld dword[nz]
			fmul dword[nsign]
			fstp dword[esp-4]
			sub esp,12
			call glNormal3f ;x*nsign, y*nsign, nz*nsign
			mov dword[esp-4],0.0
			mov ecx,[baseRadius]
			fld qword[ecx]
			fld st0
			fmul dword[y]
			fstp dword[esp-8]
			fmul dword[x]
			fstp dword[esp-12]
			sub esp,12
			call glVertex3f ;x*baseRadius, y*baseRadius, 0.0
			mov eax,[height]
			fld qword[eax]
			fstp dword[esp-4]
			mov ecx,[topRadius]
			fld qword[ecx]
			fld st0
			fmul dword[y]
			fstp dword[esp-8]
			fmul dword[x]
			fstp dword[esp-12]
			sub esp,12
			call glVertex3f ;x*topRadius, y*topRadius, height
			inc dword[i]
			jmp .cycle_6
align 4
		.cycle_6_end:
		call glEnd
		jmp .end_f
align 4
	.else1:
	cmp dword[edx+offs_qobj_DrawStyle],GLU_FILL
	jne .end_f
		fld1
		fidiv dword[slices]
		fstp dword[du] ;du = 1.0 / slices
		fld1
		fidiv dword[stacks]
		fstp dword[dv] ;dv = 1.0 / stacks
		mov dword[tcx],0.0
		mov dword[tcy],0.0
		mov ebx,[slices]
		mov dword[i],0
align 4
		.cycle_7: ;for (i=0;i<slices;i++)
			cmp [i],ebx
			jge .cycle_7_end
			fild dword[i]
			fmul qword[da]
			fld st0
			fcos
			fstp dword[y] ;y1 = cos(i*da)
			fsin
			fchs
			fstp dword[x] ;x1 = -sin(i*da)
			inc dword[i]
			fild dword[i]
			fmul qword[da]
			fld st0
			fcos
			fstp dword[y2] ;y2 = cos((i+1)*da)
			fsin
			fchs
			fstp dword[x2] ;x2 = -sin((i+1)*da)
			mov dword[z],0.0
			mov ecx,[baseRadius]
			fld qword[ecx]
			fstp qword[r] ;r = baseRadius
			mov dword[tcy],0.0
			stdcall glBegin,GL_QUAD_STRIP
			mov ecx,[stacks]
			inc ecx
align 4
			.cycle_8: ;for (j=0;j<=stacks;j++)

				fld dword[nsign]
				ftst
				fstsw ax
				sahf
				jbe .else3
					;if (nsign>0.0)
					fld st0
					fmul dword[nz]
					fstp dword[esp-4]
					fld st0
					fmul dword[y]
					fstp dword[esp-8]
					fmul dword[x]
					fstp dword[esp-12]
					sub esp,12
					call glNormal3f ;x1*nsign, y1*nsign, nz*nsign
					cmp dword[edx+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
					je @f
						stdcall glTexCoord2f, [tcx],[tcy]
					@@:
					mov eax,[z]
					mov [esp-4],eax
					fld qword[r]
					fld st0
					fmul dword[y]
					fstp dword[esp-8]
					fmul dword[x]
					fstp dword[esp-12]
					sub esp,12
					call glVertex3f ;x1*r, y1*r, z
					fld dword[nsign]
					fld st0
					fmul dword[nz]
					fstp dword[esp-4]
					fld st0
					fmul dword[y2]
					fstp dword[esp-8]
					fmul dword[x2]
					fstp dword[esp-12]
					sub esp,12
					call glNormal3f ;x2*nsign, y2*nsign, nz*nsign
					cmp dword[edx+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
					je @f
						push dword[tcy]
						fld dword[tcx]
						fadd dword[du]
						fstp dword[esp-4]
						sub esp,4
						call glTexCoord2f ;tcx+du, tcy
					@@:
					mov eax,[z]
					mov [esp-4],eax
					fld qword[r]
					fld st0
					fmul dword[y2]
					fstp dword[esp-8]
					fmul dword[x2]
					fstp dword[esp-12]
					sub esp,12
					call glVertex3f ;x2*r, y2*r, z
					jmp .else3_end
				.else3:
					fld st0
					fmul dword[nz]
					fstp dword[esp-4]
					fld st0
					fmul dword[y2]
					fstp dword[esp-8]
					fmul dword[x2]
					fstp dword[esp-12]
					sub esp,12
					call glNormal3f ;x2*nsign, y2*nsign, nz*nsign
					cmp dword[edx+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
					je @f
						stdcall glTexCoord2f, [tcx],[tcy]
					@@:
					mov eax,[z]
					mov [esp-4],eax
					fld qword[r]
					fld st0
					fmul dword[y2]
					fstp dword[esp-8]
					fmul dword[x2]
					fstp dword[esp-12]
					sub esp,12
					call glVertex3f ;x2*r, y2*r, z
					fld dword[nsign]
					fld st0
					fmul dword[nz]
					fstp dword[esp-4]
					fld st0
					fmul dword[y]
					fstp dword[esp-8]
					fmul dword[x]
					fstp dword[esp-12]
					sub esp,12
					call glNormal3f ;x1*nsign, y1*nsign, nz*nsign
					cmp dword[edx+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
					je @f
						push dword[tcy]
						fld dword[tcx]
						fadd dword[du]
						fstp dword[esp-4]
						sub esp,4
						call glTexCoord2f ;tcx+du, tcy
					@@:
					mov eax,[z]
					mov [esp-4],eax
					fld qword[r]
					fld st0
					fmul dword[y]
					fstp dword[esp-8]
					fmul dword[x]
					fstp dword[esp-12]
					sub esp,12
					call glVertex3f ;x1*r, y1*r, z
				.else3_end:
				fld dword[z]
				fadd qword[dz]
				fstp dword[z] ;z += dz
				fld qword[r]
				fadd qword[dr]
				fstp qword[r] ;r += dr
				fld dword[tcy]
				fadd dword[dv]
				fstp dword[tcy] ;tcy += dv
				dec ecx
				jnz .cycle_8
			call glEnd
			fld dword[tcx]
			fadd dword[du]
			fstp dword[tcx] ;tcx += du
			jmp .cycle_7
align 4
		.cycle_7_end:
	.end_f:
popad
	ret
endp

; Disk (adapted from Mesa)

;void gluDisk(GLUquadricObj *qobj, GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint loops )
;{
;}

;
; Sphere (adapted from Mesa)
;

;input:
; float radius, int slices, int stacks
align 16
proc gluSphere qobj:dword, radius:dword, slices:dword, stacks:dword
locals
	rho dd ? ;float
	drho dd ? 
	theta dd ? 
	dtheta dd ? 
	x dd ? ;float
	y dd ? ;float
	z dd ? ;float
	s dd ? ;float
	t dd ? ;float
	d_s dd ? ;float
	d_t dd ? ;float
	i dd ? ;int
	j dd ? ;int
	imax dd ? ;int
	normals dd ? ;int
	nsign dd ? ;float
endl
pushad

	mov eax,[qobj]
	cmp dword[eax+offs_qobj_Normals],GLU_NONE ;if (qobj.Normals==GLU_NONE)
	jne .els_0
		mov dword[normals],GL_FALSE
		jmp @f
align 4
	.els_0:
		mov dword[normals],GL_TRUE
	@@:
	cmp dword[eax+offs_qobj_Orientation],GLU_INSIDE ;if (qobj.Orientation==GLU_INSIDE)
	jne .els_1
		mov dword[nsign],-1.0
		jmp @f
align 4
	.els_1:
		mov dword[nsign],1.0
	@@:

	fldpi
	fidiv dword[stacks]
	fstp dword[drho]
	fld1
	fldpi
	fscale
	fidiv dword[slices]
	fstp dword[dtheta]
	ffree st0
	fincstp

	cmp dword[eax+offs_qobj_DrawStyle],GLU_FILL ;if (qobj.DrawStyle==GLU_FILL)
	jne .if_glu_line

	; draw +Z end as a triangle fan
	stdcall glBegin,GL_TRIANGLE_FAN
	cmp dword[normals],GL_TRUE
	jne @f
		stdcall glNormal3f, 0.0, 0.0, 1.0
	@@:
	cmp dword[eax+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
	je @f
		stdcall glTexCoord2f, 0.5,1.0
	@@:
	sub esp,4
	fld dword[nsign]
	fmul dword[radius]
	fstp dword[esp]
	stdcall glVertex3f, 0.0, 0.0 ;, nsign * radius
	fld dword[drho]
	fcos
	fmul dword[nsign]
	fstp dword[z] ;z = nsign * cos(drho)
	mov dword[j],0
	mov ecx,[slices]
align 4
	.cycle_0: ;for (j=0;j<=slices;j++)
		cmp dword[j],ecx
		jg .cycle_0_end
		fld dword[drho]
		fsin
		je @f
			fild dword[j]
			fmul dword[dtheta]
			jmp .t0_end
align 4
		@@:
			fldz
		.t0_end:
		fst dword[theta] ;theta = (j==slices) ? 0.0 : j * dtheta
		fsin
		fchs
		fmul st0,st1
		fstp dword[x] ;x = -sin(theta) * sin(drho)
		fld dword[theta]
		fcos
		fmulp
		fstp dword[y] ;y = cos(theta) * sin(drho)
		cmp dword[normals],GL_TRUE
		jne @f
			fld dword[nsign]
			fld dword[z]
			fmul st0,st1
			fstp dword[esp-4]
			fld dword[y]
			fmul st0,st1
			fstp dword[esp-8]
			fld dword[x]
			fmulp
			fstp dword[esp-12]
			sub esp,12
			stdcall glNormal3f ;x*nsign, y*nsign, z*nsign
		@@:
		fld dword[radius]
		fld dword[z]
		fmul st0,st1
		fstp dword[esp-4]
		fld dword[y]
		fmul st0,st1
		fstp dword[esp-8]
		fld dword[x]
		fmulp
		fstp dword[esp-12]
		sub esp,12
		call glVertex3f ;x*radius, y*radius, z*radius
		inc dword[j]
		jmp .cycle_0
align 4
	.cycle_0_end:
	call glEnd

	fld1
	fidiv dword[slices]
	fstp dword[d_s] ;1.0 / slices
	fld1
	fidiv dword[stacks]
	fstp dword[d_t] ;1.0 / stacks
	mov dword[t],1.0 ; because loop now runs from 0
	mov ebx,[stacks]
	cmp dword[eax+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
	je .els_2
		mov dword[i],0
		mov [imax],ebx
		jmp @f
align 4
	.els_2:
		mov dword[i],1
		dec ebx
		mov [imax],ebx
	@@:

	; draw intermediate stacks as quad strips
	mov ebx,[imax]
align 4
	.cycle_1: ;for (i=imin;i<imax;i++)
	cmp dword[i],ebx
	jge .cycle_1_end
	fild dword[i]
	fmul dword[drho]
	fstp dword[rho] ;rho = i * drho
	stdcall glBegin,GL_QUAD_STRIP
	mov dword[s],0.0
	mov dword[j],0
align 4
	.cycle_2: ;for (j=0;j<=slices;j++)
		cmp dword[j],ecx
		jg .cycle_2_end
		fld dword[rho]
		fsin
		je @f
			fild dword[j]
			fmul dword[dtheta]
			jmp .t1_end
align 4
		@@:
			fldz
		.t1_end:
		fst dword[theta] ;theta = (j==slices) ? 0.0 : j * dtheta
		fsin
		fchs
		fmul st0,st1
		fstp dword[x] ;x = -sin(theta) * sin(rho)
		fld dword[theta]
		fcos
		fmulp
		fstp dword[y] ;y = cos(theta) * sin(rho)
		fld dword[rho]
		fcos
		fmul dword[nsign]
		fstp dword[z] ;z = nsign * cos(rho)
		cmp dword[normals],GL_TRUE
		jne @f
			fld dword[nsign]
			fld dword[z]
			fmul st0,st1
			fstp dword[esp-4]
			fld dword[y]
			fmul st0,st1
			fstp dword[esp-8]
			fld dword[x]
			fmulp
			fstp dword[esp-12]
			sub esp,12
			stdcall glNormal3f ;x*nsign, y*nsign, z*nsign
		@@:
		cmp dword[eax+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
		je @f
			stdcall glTexCoord2f, [s],[t]
		@@:
		fld dword[radius]
		fld dword[z]
		fmul st0,st1
		fstp dword[esp-4]
		fld dword[y]
		fmul st0,st1
		fstp dword[esp-8]
		fld dword[x]
		fmulp
		fstp dword[esp-12]
		sub esp,12
		call glVertex3f ;x*radius, y*radius, z*radius
		fld dword[rho]
		fadd dword[drho]
		fsin ;st0 = sin(rho+drho)
		fld dword[theta]
		fsin
		fchs
		fmul st0,st1
		fstp dword[x] ;x = -sin(theta) * sin(rho+drho)
		fld dword[theta]
		fcos
		fmulp
		fstp dword[y] ;y = cos(theta) * sin(rho+drho)
		fld dword[rho]
		fadd dword[drho]
		fcos
		fmul dword[nsign]
		fstp dword[z] ;z = nsign * cos(rho+drho)
		cmp dword[normals],GL_TRUE
		jne @f
			fld dword[nsign]
			fld dword[z]
			fmul st0,st1
			fstp dword[esp-4]
			fld dword[y]
			fmul st0,st1
			fstp dword[esp-8]
			fld dword[x]
			fmulp
			fstp dword[esp-12]
			sub esp,12
			stdcall glNormal3f ;x*nsign, y*nsign, z*nsign
		@@:
		cmp dword[eax+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
		je @f
			fld dword[t]
			fsub dword[d_t]
			fstp dword[esp-4]
			sub esp,4
			stdcall glTexCoord2f, [s] ;,t-dt
			fld dword[s]
			fadd dword[d_s]
			fstp dword[s]
		@@:
		fld dword[radius]
		fld dword[z]
		fmul st0,st1
		fstp dword[esp-4]
		fld dword[y]
		fmul st0,st1
		fstp dword[esp-8]
		fld dword[x]
		fmulp
		fstp dword[esp-12]
		sub esp,12
		call glVertex3f ;x*radius, y*radius, z*radius
		inc dword[j]
		jmp .cycle_2
align 4
		.cycle_2_end:
	call glEnd
	fld dword[t]
	fsub dword[d_t]
	fstp dword[t]
	inc dword[i]
	jmp .cycle_1
align 4
	.cycle_1_end:

	; draw -Z end as a triangle fan
	stdcall glBegin,GL_TRIANGLE_FAN
	cmp dword[normals],GL_TRUE
	jne @f
		stdcall glNormal3f, 0.0, 0.0, -1.0
	@@:
	cmp dword[eax+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
	je @f
		stdcall glTexCoord2f, 0.5,0.0
		mov dword[s],1.0
		mov ebx,[d_t]
		mov [t],ebx
	@@:
	sub esp,4
	fld dword[radius]
	fchs
	fmul dword[nsign]
	fstp dword[esp]
	stdcall glVertex3f, 0.0, 0.0 ;, -radius*nsign
	fldpi
	fsub dword[drho]
	fst dword[rho]
	fcos
	fmul dword[nsign]
	fstp dword[z] ;z = nsign * cos(rho)
	mov [j],ecx
align 4
	.cycle_3: ;for (j=slices;j>=0;j--)
		cmp dword[j],0
		jl .cycle_3_end
		fld dword[rho]
		fsin
		je @f
			fild dword[j]
			fmul dword[dtheta]
			jmp .t2_end
align 4
		@@:
			fldz
		.t2_end:
		fst dword[theta] ;theta = (j==slices) ? 0.0 : j * dtheta
		fsin
		fchs
		fmul st0,st1
		fstp dword[x] ;x = -sin(theta) * sin(rho)
		fld dword[theta]
		fcos
		fmulp
		fstp dword[y] ;y = cos(theta) * sin(rho)
		cmp dword[normals],GL_TRUE
		jne @f
			fld dword[nsign]
			fld dword[z]
			fmul st0,st1
			fstp dword[esp-4]
			fld dword[y]
			fmul st0,st1
			fstp dword[esp-8]
			fld dword[x]
			fmul st0,st1
			fstp dword[esp-12]
			sub esp,12
			ffree st0
			fincstp
			stdcall glNormal3f ;x*nsign, y*nsign, z*nsign
		@@:
		cmp dword[eax+offs_qobj_TextureFlag],0 ;if (qobj.TextureFlag)
		je @f
			stdcall glTexCoord2f, [s],[t]
			fld dword[s]
			fsub dword[d_s]
			fstp dword[s]
		@@:
		fld dword[radius]
		fld dword[z]
		fmul st0,st1
		fstp dword[esp-4]
		fld dword[y]
		fmul st0,st1
		fstp dword[esp-8]
		fld dword[x]
		fmulp
		fstp dword[esp-12]
		sub esp,12
		call glVertex3f ;x*radius, y*radius, z*radius
		dec dword[j]
		jmp .cycle_3
align 4
	.cycle_3_end:
	call glEnd
	jmp .end_f

align 4
	.if_glu_line:
	cmp dword[eax+offs_qobj_DrawStyle],GLU_LINE ;if (qobj.DrawStyle==GLU_LINE)
	je @f
	cmp dword[eax+offs_qobj_DrawStyle],GLU_SILHOUETTE ;if (qobj.DrawStyle==GLU_SILHOUETTE)
	je @f
	jmp .if_glu_point
align 4
	@@:

	; draw stack lines
	mov dword[i],1
	mov ebx,[stacks]
	mov ecx,[slices]
align 4
	.cycle_4: ;for (i=1;i<stacks;i++)
		cmp dword[i],ebx
		jge .cycle_4_end
		; stack line at i==stacks-1 was missing here

		fild dword[i]
		fmul dword[drho]
		fstp dword[rho] ;rho = i * drho
		stdcall glBegin,GL_LINE_LOOP
		mov dword[j],0
align 4
		.cycle_5: ;for (j=0;j<slices;j++)
			cmp dword[j],ecx
			jge .cycle_5_end
			fild dword[j]
			fmul dword[dtheta]
			fst dword[theta] ;theta = j * dtheta;
			fcos
			fld dword[rho]
			fsin
			fxch ;толкаем sin(rho) в st1
			fmul st0,st1
			fstp dword[x] ;x = cos(theta) * sin(rho)
			fld dword[theta]
			fsin
			fmulp ;множим на sin(rho) ранее записанный в st1
			fstp dword[y] ;y = sin(theta) * sin(rho)
			fld dword[rho]
			fcos
			fstp dword[z] ;z = cos(rho)
			cmp dword[normals],GL_TRUE
			jne @f
				fld dword[nsign]
				fld dword[z]
				fmul st0,st1
				fstp dword[esp-4]
				fld dword[y]
				fmul st0,st1
				fstp dword[esp-8]
				fld dword[x]
				fmulp
				fstp dword[esp-12]
				sub esp,12
				stdcall glNormal3f ;x*nsign, y*nsign, z*nsign
			@@:
			fld dword[radius]
			fld dword[z]
			fmul st0,st1
			fstp dword[esp-4]
			fld dword[y]
			fmul st0,st1
			fstp dword[esp-8]
			fld dword[x]
			fmulp
			fstp dword[esp-12]
			sub esp,12
			call glVertex3f ;x*radius, y*radius, z*radius
			inc dword[j]
			jmp .cycle_5
align 4
		.cycle_5_end:
		call glEnd
		inc dword[i]
		jmp .cycle_4
align 4
	.cycle_4_end:

	; draw slice lines
	mov dword[j],0
align 4
	.cycle_6: ;for (j=0;j<slices;j++)
		cmp dword[j],ecx
		jge .cycle_6_end
		fild dword[j]
		fmul dword[dtheta]
		fstp dword[theta] ;theta = j * dtheta;
		stdcall glBegin,GL_LINE_STRIP
		mov dword[i],0
align 4
		.cycle_7: ;for (i=0;i<=stacks;i++)
			cmp dword[i],ebx
			jg .cycle_7_end
			fild dword[i]
			fmul dword[drho]
			fst dword[rho] ;rho = i * drho
			fsin
			fld dword[theta]
			fcos
			fmul st0,st1
			fstp dword[x] ;x = cos(theta) * sin(rho)
			fld dword[theta]
			fsin
			fmulp
			fstp dword[y] ;y = sin(theta) * sin(rho)
			fld dword[rho]
			fcos
			fstp dword[z] ;z = cos(rho)
			cmp dword[normals],GL_TRUE
			jne @f
				fld dword[nsign]
				fld dword[z]
				fmul st0,st1
				fstp dword[esp-4]
				fld dword[y]
				fmul st0,st1
				fstp dword[esp-8]
				fld dword[x]
				fmulp
				fstp dword[esp-12]
				sub esp,12
				stdcall glNormal3f ;x*nsign, y*nsign, z*nsign
			@@:
			fld dword[radius]
			fld dword[z]
			fmul st0,st1
			fstp dword[esp-4]
			fld dword[y]
			fmul st0,st1
			fstp dword[esp-8]
			fld dword[x]
			fmulp
			fstp dword[esp-12]
			sub esp,12
			call glVertex3f ;x*radius, y*radius, z*radius
			inc dword[i]
			jmp .cycle_7
align 4
		.cycle_7_end:
		call glEnd
		inc dword[j]
		jmp .cycle_6
align 4
	.cycle_6_end:
	jmp .end_f

align 4
	.if_glu_point:
	cmp dword[eax+offs_qobj_DrawStyle],GLU_POINT ;if (qobj.DrawStyle==GLU_POINT)
	jne .end_f

	; top and bottom-most points
	stdcall glBegin,GL_POINTS
	cmp dword[normals],GL_TRUE
	jne @f
		stdcall glNormal3f, 0.0,0.0,dword[nsign]
	@@:
	stdcall glVertex3f, 0.0,0.0,dword[radius]
	cmp dword[normals],GL_TRUE
	jne @f
		sub esp,4
		fld dword[nsign]
		fchs
		fstp dword[esp]
		stdcall glNormal3f, 0.0,0.0 ;,-nsign
	@@:
	sub esp,4
	fld dword[radius]
	fchs
	fstp dword[esp]
	stdcall glVertex3f, 0.0,0.0 ;,-radius

	; loop over stacks
	mov dword[i],1
	mov ebx,[stacks]
	mov ecx,[slices]
align 4
	.cycle_8: ;for (i=1;i<stacks;i++)
		cmp dword[i],ebx
		jge .cycle_8_end
		fild dword[i]
		fmul dword[drho]
		fstp dword[rho] ;rho = i * drho

		mov dword[j],0
align 4
		.cycle_9: ;for (j=0;j<slices;j++)
			cmp dword[j],ecx
			jge .cycle_9_end
			fild dword[j]
			fmul dword[dtheta]
			fst dword[theta] ;theta = j * dtheta;
			fcos
			fld dword[rho]
			fsin
			fxch ;толкаем sin(rho) в st1
			fmul st0,st1
			fstp dword[x] ;x = cos(theta) * sin(rho)
			fld dword[theta]
			fsin
			fmulp ;множим на sin(rho) ранее записанный в st1
			fstp dword[y] ;y = sin(theta) * sin(rho)
			fld dword[rho]
			fcos
			fstp dword[z] ;z = cos(rho)
			cmp dword[normals],GL_TRUE
			jne @f
				fld dword[nsign]
				fld dword[z]
				fmul st0,st1
				fstp dword[esp-4]
				fld dword[y]
				fmul st0,st1
				fstp dword[esp-8]
				fld dword[x]
				fmulp
				fstp dword[esp-12]
				sub esp,12
				stdcall glNormal3f ;x*nsign, y*nsign, z*nsign
			@@:
			fld dword[radius]
			fld dword[z]
			fmul st0,st1
			fstp dword[esp-4]
			fld dword[y]
			fmul st0,st1
			fstp dword[esp-8]
			fld dword[x]
			fmulp
			fstp dword[esp-12]
			sub esp,12
			call glVertex3f ;x*radius, y*radius, z*radius
			inc dword[j]
			jmp .cycle_9
align 4
		.cycle_9_end:
		inc dword[i]
		jmp .cycle_8
align 4
	.cycle_8_end:
	call glEnd

	.end_f:
popad
	ret
endp
