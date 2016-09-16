; glVertex 

align 4
proc glVertex4f uses eax, x:dword, y:dword, z:dword, w:dword
locals
	p rd 5
endl
	mov dword[p],OP_Vertex
	mov eax,[x]
	mov dword[p+4],eax
	mov eax,[y]
	mov dword[p+8],eax
	mov eax,[z]
	mov dword[p+12],eax
	mov eax,[w]
	mov dword[p+16],eax

	mov eax,ebp
	sub eax,20 ;=sizeof(dd)*5
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glVertex2f, x:dword, y:dword
	stdcall glVertex4f,[x],[y],0.0,1.0
	ret
endp

align 4
proc glVertex2fv uses eax, v:dword
	mov eax,[v]
	stdcall glVertex4f,[eax],[eax+4],0.0,1.0
	ret
endp

align 4
proc glVertex3f, x:dword, y:dword, z:dword
	stdcall glVertex4f,[x],[y],[z],1.0
	ret
endp

align 4
proc glVertex3fv uses eax, v:dword
	mov eax,[v]
	stdcall glVertex4f,[eax],[eax+4],[eax+8],1.0
	ret
endp

align 4
proc glVertex4fv uses eax, v:dword
	mov eax,[v]
	stdcall glVertex4f,[eax],[eax+4],[eax+8],[eax+12]
	ret
endp

; glNormal

align 4
proc glNormal3f uses eax, x:dword, y:dword, z:dword
locals
	p rd 4
endl
	mov dword[p],OP_Normal
	mov eax,[x]
	mov dword[p+4],eax
	mov eax,[y]
	mov dword[p+8],eax
	mov eax,[z]
	mov dword[p+12],eax

	mov eax,ebp
	sub eax,16 ;=sizeof(dd)*4
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glNormal3fv uses eax, v:dword
	mov eax,[v]
	stdcall glNormal3f,[eax],[eax+4],[eax+8]
	ret
endp

; glColor

align 4
proc glColor4f uses eax, r:dword, g:dword, b:dword, a:dword
locals
	p rd 8
endl
	mov dword[p],OP_Color
	mov eax,[b]
	mov dword[p+4],eax
	mov eax,[g]
	mov dword[p+8],eax
	mov eax,[r]
	mov dword[p+12],eax
	mov eax,[a]
	mov dword[p+16],eax
	; direct convertion to integer to go faster if no shading
	mov eax,ebp
	sub eax,12 ;ebp-12 = &p[5]
	push eax
	add eax,4 ;ebp-8 = &p[6]
	push eax
	add eax,4 ;ebp-4 = &p[7]
	push eax
	stdcall RGBFtoRGBI,[r],[g],[b] ;call: r,g,b,&p[7],&p[6],&p[5]

	mov eax,ebp
	sub eax,32 ;=sizeof(dd)*8
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glColor4fv uses eax ebx, v:dword
	mov eax,[v]
	stdcall glColor4f,[eax],[eax+4],[eax+8],[eax+12],1.0
	ret
endp

align 4
proc glColor3f, r:dword, g:dword, b:dword
	stdcall glColor4f,[r],[g],[b],1.0
	ret
endp

align 4
proc glColor3fv uses eax, v:dword
	mov eax,[v]
	stdcall glColor4f,[eax],[eax+4],[eax+8],1.0
	ret
endp

align 4
fl_255 dd 255.0

align 4
proc glColor3ub uses eax, r:dword, g:dword, b:dword
	push dword 1.0
	fld dword[fl_255]

	movzx eax,byte[b]
	mov dword[esp-4],eax
	fild dword[esp-4]
	fdiv st0,st1
	fstp dword[esp-4] ;преобразовали int во float
	movzx eax,byte[g]
	mov dword[esp-8],eax
	fild dword[esp-8]
	fdiv st0,st1
	fstp dword[esp-8]
	movzx eax,byte[r]
	mov dword[esp-12],eax
	fild dword[esp-12]
	fdiv st0,st1
	fstp dword[esp-12]

	ffree st0
	fincstp
	sub esp,12
	call glColor4f
	ret
endp

; TexCoord

align 4
proc glTexCoord4f uses eax, s:dword, t:dword, r:dword, q:dword
locals
	p rd 5
endl
	mov dword[p],OP_TexCoord
	mov eax,[s]
	mov dword[p+4],eax
	mov eax,[t]
	mov dword[p+8],eax
	mov eax,[r]
	mov dword[p+12],eax
	mov eax,[q]
	mov dword[p+16],eax

	mov eax,ebp
	sub eax,20 ;=sizeof(dd)*5
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glTexCoord2f, s:dword, t:dword
	stdcall glTexCoord4f,[s],[t],0.0,1.0
	ret
endp

align 4
proc glTexCoord2fv uses eax, v:dword
	mov eax,[v]
	stdcall glTexCoord4f,[eax],[eax+4],0.0,1.0
	ret
endp

align 4
proc glEdgeFlag uses eax, flag:dword
locals
	p rd 2
endl
	mov dword[p],OP_EdgeFlag
	mov eax,[flag]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

; misc

align 4
proc glShadeModel uses eax, mode:dword
locals
	p rd 2
endl

;  assert(mode == GL_FLAT || mode == GL_SMOOTH);

	mov dword[p],OP_ShadeModel
	mov eax,[mode]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glCullFace uses eax, mode:dword
locals
	p rd 2
endl

;  assert(mode == GL_BACK || 
;         mode == GL_FRONT || 
;         mode == GL_FRONT_AND_BACK);

	mov dword[p],OP_CullFace
	mov eax,[mode]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glFrontFace uses eax, mode:dword
locals
	p rd 2
endl

;  assert(mode == GL_CCW || mode == GL_CW);

	mov dword[p],OP_FrontFace
	xor eax,eax
	cmp dword[mode],GL_CCW
	je @f
		inc eax
	@@:
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glPolygonMode uses eax, face:dword, mode:dword
locals
	p rd 3
endl

;  assert(face == GL_BACK || 
;         face == GL_FRONT || 
;         face == GL_FRONT_AND_BACK);
;  assert(mode == GL_POINT || mode == GL_LINE || mode==GL_FILL);

	mov dword[p],OP_PolygonMode
	mov eax,[face]
	mov dword[p+4],eax
	mov eax,[mode]
	mov dword[p+8],eax

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

; glEnable / glDisable

align 4
proc glEnable uses eax, cap:dword
locals
	p rd 3
endl
	mov dword[p],OP_EnableDisable
	mov eax,[cap]
	mov dword[p+4],eax
	mov dword[p+8],1

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glDisable uses eax, cap:dword
locals
	p rd 3
endl
	mov dword[p],OP_EnableDisable
	mov eax,[cap]
	mov dword[p+4],eax
	mov dword[p+8],0

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

; glBegin / glEnd

align 4
proc glBegin uses eax, mode:dword
locals
	p rd 2
endl
	mov dword[p],OP_Begin
	mov eax,[mode]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glEnd uses eax
locals
	p dd ?
endl
	mov dword[p],OP_End

	mov eax,ebp
	sub eax,4 ;=sizeof(dd)*1
	stdcall gl_add_op,eax
	ret
endp

; matrix

align 4
proc glMatrixMode uses eax, mode:dword
locals
	p rd 2
endl
	mov dword[p],OP_MatrixMode
	mov eax,[mode]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glLoadMatrixf uses ecx edi esi, m:dword
locals
	p rd 17
endl
	mov dword[p],OP_LoadMatrix
	mov ecx,16
	mov esi,[m]
	mov edi,ebp
	sub edi,64 ;=sizeof(M4)
	rep movsd

	mov ecx,ebp
	sub ecx,68 ;=sizeof(dd)*17
	stdcall gl_add_op,ecx
	ret
endp

align 4
proc glLoadIdentity uses eax
locals
	p dd ?
endl
	mov dword[p],OP_LoadIdentity

	mov eax,ebp
	sub eax,4 ;=sizeof(dd)*1
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glMultMatrixf uses ecx edi esi, m:dword
locals
	p rd 17
endl
	mov dword[p],OP_MultMatrix
	mov ecx,16
	mov esi,[m]
	mov edi,ebp
	sub edi,64 ;=sizeof(M4)
	rep movsd

	mov ecx,ebp
	sub ecx,68 ;=sizeof(dd)*17
	stdcall gl_add_op,ecx
	ret
endp

align 4
proc glPushMatrix uses eax
locals
	p dd ?
endl
	mov dword[p],OP_PushMatrix

	mov eax,ebp
	sub eax,4 ;=sizeof(dd)*1
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glPopMatrix uses eax
locals
	p dd ?
endl
	mov dword[p],OP_PopMatrix

	mov eax,ebp
	sub eax,4 ;=sizeof(dd)*1
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glRotatef uses eax, angle:dword, x:dword, y:dword, z:dword
locals
	p rd 5
endl
	mov dword[p],OP_Rotate
	mov eax,[angle]
	mov dword[p+4],eax
	mov eax,[x]
	mov dword[p+8],eax
	mov eax,[y]
	mov dword[p+12],eax
	mov eax,[z]
	mov dword[p+16],eax

	mov eax,ebp
	sub eax,20 ;=sizeof(dd)*5
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glTranslatef uses eax, x:dword, y:dword, z:dword
locals
	p rd 4
endl
	mov dword[p],OP_Translate
	mov eax,[x]
	mov dword[p+4],eax
	mov eax,[y]
	mov dword[p+8],eax
	mov eax,[z]
	mov dword[p+12],eax

	mov eax,ebp
	sub eax,16 ;=sizeof(dd)*4
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glScalef uses eax, x:dword, y:dword, z:dword
locals
	p rd 4
endl
	mov dword[p],OP_Scale
	mov eax,[x]
	mov dword[p+4],eax
	mov eax,[y]
	mov dword[p+8],eax
	mov eax,[z]
	mov dword[p+12],eax

	mov eax,ebp
	sub eax,16 ;=sizeof(dd)*4
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glViewport uses eax, x:dword, y:dword, width:dword, heigh:dword
locals
	p rd 5
endl
	mov dword[p],OP_Viewport
	mov eax,[x]
	mov dword[p+4],eax
	mov eax,[y]
	mov dword[p+8],eax
	mov eax,[width]
	mov dword[p+12],eax
	mov eax,[heigh]
	mov dword[p+16],eax

	mov eax,ebp
	sub eax,20 ;=sizeof(dd)*5
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glFrustum uses eax, left:dword,right:dword,bottom:dword,top:dword,\
	near:dword,farv:dword
locals
	p rd 7
endl
	mov dword[p],OP_Frustum
	mov eax,[left]
	fld qword[eax]
	fstp dword[p+4]
	mov eax,[right]
	fld qword[eax]
	fstp dword[p+8]
	mov eax,[bottom]
	fld qword[eax]
	fstp dword[p+12]
	mov eax,[top]
	fld qword[eax]
	fstp dword[p+16]
	mov eax,[near]
	fld qword[eax]
	fstp dword[p+20]
	mov eax,[farv]
	fld qword[eax]
	fstp dword[p+24]

	mov eax,ebp
	sub eax,28 ;=sizeof(dd)*7
	stdcall gl_add_op,eax
	ret
endp

; lightening

align 4
proc glMaterialfv uses eax ecx, mode:dword, type:dword, v:dword
locals
	p rd 7
endl

;  assert(mode == GL_FRONT  || mode == GL_BACK || mode==GL_FRONT_AND_BACK);

	mov dword[p],OP_Material
	mov eax,[mode]
	mov dword[p+4],eax
	mov eax,[type]
	mov dword[p+8],eax

	mov eax,[v]
	mov ecx,[eax+8]
	mov dword[p+12],ecx
	mov ecx,[eax+4]
	mov dword[p+16],ecx
	mov ecx,[eax]
	mov dword[p+20],ecx
	mov ecx,[eax+12]
	mov dword[p+24],ecx

	cmp dword[type],GL_SHININESS
	jne @f
		mov dword[p+16],0.0
		mov dword[p+20],0.0
		mov dword[p+24],0.0
	@@:

	mov eax,ebp
	sub eax,28 ;=sizeof(dd)*7
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glMaterialf uses eax, mode:dword, type:dword, v:dword
locals
	p rd 7
endl
	mov dword[p],OP_Material
	mov eax,[mode]
	mov dword[p+4],eax
	mov eax,[type]
	mov dword[p+8],eax
	mov eax,[v]
	mov dword[p+12],eax
	mov dword[p+16],0.0
	mov dword[p+20],0.0
	mov dword[p+24],0.0

	mov eax,ebp
	sub eax,28 ;=sizeof(dd)*7
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glColorMaterial uses eax, mode:dword, type:dword
locals
	p rd 3
endl
	mov dword[p],OP_ColorMaterial
	mov eax,[mode]
	mov dword[p+4],eax
	mov eax,[type]
	mov dword[p+8],eax

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glLightfv uses eax ecx, light:dword, type:dword, v:dword
locals
	p rd 7
endl
	mov dword[p],OP_Light
	mov eax,[light]
	mov dword[p+4],eax
	mov eax,[type]
	mov dword[p+8],eax

	;TODO: 3 composants ?
	mov eax,[v]
	mov ecx,[eax]
	mov dword[p+12],ecx
	mov ecx,[eax+4]
	mov dword[p+16],ecx
	mov ecx,[eax+8]
	mov dword[p+20],ecx
	mov ecx,[eax+12]
	mov dword[p+24],ecx

	mov eax,ebp
	sub eax,28 ;=sizeof(dd)*7
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glLightf uses eax, light:dword, type:dword, v:dword
locals
	p rd 7
endl
	mov dword[p],OP_Light
	mov eax,[light]
	mov dword[p+4],eax
	mov eax,[type]
	mov dword[p+8],eax
	mov eax,[v]
	mov dword[p+12],eax
	mov dword[p+16],0.0
	mov dword[p+20],0.0
	mov dword[p+24],0.0

	mov eax,ebp
	sub eax,28 ;=sizeof(dd)*7
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glLightModeli uses eax, pname:dword, param:dword
locals
	p rd 6
endl
	mov dword[p],OP_LightModel
	mov eax,[pname]
	mov dword[p+4],eax
	fild dword[param]
	fstp dword[p+8] ;преобразовали int во float

	mov dword[p+12],0.0
	mov dword[p+16],0.0
	mov dword[p+20],0.0

	mov eax,ebp
	sub eax,24 ;=sizeof(dd)*6
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glLightModelfv uses eax ecx, pname:dword, param:dword
locals
	p rd 6
endl
	mov dword[p],OP_LightModel
	mov eax,[pname]
	mov dword[p+4],eax
	mov eax,[param]
	mov ecx,[eax]
	mov dword[p+8],ecx
	mov ecx,[eax+4]
	mov dword[p+12],ecx
	mov ecx,[eax+8]
	mov dword[p+16],ecx
	mov ecx,[eax+12]
	mov dword[p+20],ecx

	mov eax,ebp
	sub eax,24 ;=sizeof(dd)*6
	stdcall gl_add_op,eax
	ret
endp

; clear

align 4
proc glClear uses eax, mask:dword
locals
	p rd 2
endl
	mov dword[p],OP_Clear
	mov eax,[mask]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glClearColor uses eax, r:dword, g:dword, b:dword, a:dword
locals
	p rd 5
endl
	mov dword[p],OP_ClearColor
	mov eax,[b]
	mov dword[p+4],eax
	mov eax,[g]
	mov dword[p+8],eax
	mov eax,[r]
	mov dword[p+12],eax
	mov eax,[a]
	mov dword[p+16],eax

	mov eax,ebp
	sub eax,20 ;=sizeof(dd)*5
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glClearDepth uses eax, depth:dword
locals
	p rd 2
endl
	mov dword[p],OP_ClearDepth
	mov eax,[depth]
	fld qword[eax]
	fstp dword[p+4]

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

; textures

align 4
proc glTexImage2D uses ecx edi esi,\
	target:dword, level:dword, components:dword,\
	width:dword, height:dword, border:dword,\
	format:dword, type:dword, pixels:dword
locals
	p rd 10
endl
	mov dword[p],OP_TexImage2D
	mov ecx,9
	mov esi,ebp
	add esi,8 ;указатель на стек с входными параметрами
	mov edi,ebp
	sub edi,36 ;указатель на стек с локальным массивом
	rep movsd ;копирование в цикле 9-ти входных параметров

	mov ecx,ebp
	sub ecx,40 ;=sizeof(dd)*10
	stdcall gl_add_op,ecx
	ret
endp

align 4
proc glBindTexture uses eax, target:dword, texture:dword
locals
	p rd 3
endl
	mov dword[p],OP_BindTexture
	mov eax,[target]
	mov dword[p+4],eax
	mov eax,[texture]
	mov dword[p+8],eax

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glTexEnvi uses eax, target:dword, pname:dword, param:dword
locals
	p rd 8
endl
	mov dword[p],OP_TexEnv
	mov eax,[target]
	mov dword[p+4],eax
	mov eax,[pname]
	mov dword[p+8],eax
	mov eax,[param]
	mov dword[p+12],eax
	mov dword[p+16],0.0
	mov dword[p+20],0.0
	mov dword[p+24],0.0
	mov dword[p+28],0.0

	mov eax,ebp
	sub eax,32 ;=sizeof(dd)*8
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glTexParameteri uses eax, target:dword, pname:dword, param:dword
locals
	p rd 8
endl
	mov dword[p],OP_TexParameter
	mov eax,[target]
	mov dword[p+4],eax
	mov eax,[pname]
	mov dword[p+8],eax
	mov eax,[param]
	mov dword[p+12],eax
	mov dword[p+16],0.0
	mov dword[p+20],0.0
	mov dword[p+24],0.0
	mov dword[p+28],0.0

	mov eax,ebp
	sub eax,32 ;=sizeof(dd)*8
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glPixelStorei uses eax, pname:dword, param:dword
locals
	p rd 3
endl
	mov dword[p],OP_PixelStore
	mov eax,[pname]
	mov dword[p+4],eax
	mov eax,[param]
	mov dword[p+8],eax

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

; selection

align 4
proc glInitNames uses eax
locals
	p dd ?
endl
	mov dword[p],OP_InitNames

	mov eax,ebp
	sub eax,4 ;=sizeof(dd)*1
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glPushName uses eax, name:dword
locals
	p rd 2
endl
	mov dword[p],OP_PushName
	mov eax,[name]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glPopName uses eax
locals
	p dd ?
endl
	mov dword[p],OP_PopName

	mov eax,ebp
	sub eax,4 ;=sizeof(dd)*1
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glLoadName uses eax, name:dword
locals
	p rd 2
endl
	mov dword[p],OP_LoadName
	mov eax,[name]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glPolygonOffset uses eax, factor:dword, units:dword
locals
	p rd 3
endl
	mov dword[p],OP_PolygonOffset
	mov eax,[factor]
	mov dword[p+4],eax
	mov eax,[units]
	mov dword[p+8],eax

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

; Special Functions

align 4
proc glCallList uses eax, list:dword
locals
	p rd 2
endl
	mov dword[p],OP_CallList
	mov eax,[list]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glFlush ;(void)
	;nothing to do
	ret
endp

align 4
proc glHint uses eax, target:dword, mode:dword
locals
	p rd 3
endl
	mov dword[p],OP_Hint
	mov eax,[target]
	mov dword[p+4],eax
	mov eax,[mode]
	mov dword[p+8],eax

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

; Non standard functions

align 4
proc glDebug uses eax, mode:dword
	stdcall gl_get_context ;после вызова функции в eax указатель на GLContext
	push dword[mode]
	pop dword[eax+GLContext.print_flag]
	ret
endp
