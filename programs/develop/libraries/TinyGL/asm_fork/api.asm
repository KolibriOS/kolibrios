; glVertex 

align 4
glVertex4f: ;x, y, z, w
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Vertex
	stdcall gl_add_op,eax
	pop eax
	ret 20 ;=sizeof(dd)*5

align 4
proc glVertex4d, x:qword, y:qword, z:qword, w:qword
	add esp,-16
	fld qword[w]
	fstp dword[esp+12]
	fld qword[z]
	fstp dword[esp+8]
	fld qword[y]
	fstp dword[esp+4]
	fld qword[x]
	fstp dword[esp]
	call glVertex4f
	ret
endp

align 4
proc glVertex2f, x:dword, y:dword
	stdcall glVertex4f,[x],[y],0.0,1.0
	ret
endp

align 4
proc glVertex2d, x:qword, y:qword
	push 1.0
	push 0.0
	add esp,-8
	fld qword[y]
	fstp dword[esp+4]
	fld qword[x]
	fstp dword[esp]
	call glVertex4f
	ret
endp

align 4
proc glVertex2fv uses eax, v:dword
	mov eax,[v]
	stdcall glVertex4f,[eax],[eax+4],0.0,1.0
	ret
endp

align 4
proc glVertex2dv uses eax, v:dword
	mov eax,[v]
	push 1.0
	push 0.0
	add esp,-8
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glVertex4f
	ret
endp

align 4
proc glVertex3f, x:dword, y:dword, z:dword
	stdcall glVertex4f,[x],[y],[z],1.0
	ret
endp

align 4
proc glVertex3d, x:qword, y:qword, z:qword
	push 1.0
	add esp,-12
	fld qword[z]
	fstp dword[esp+8]
	fld qword[y]
	fstp dword[esp+4]
	fld qword[x]
	fstp dword[esp]
	call glVertex4f
	ret
endp

align 4
proc glVertex3fv uses eax, v:dword
	mov eax,[v]
	stdcall glVertex4f,[eax],[eax+4],[eax+8],1.0
	ret
endp

align 4
proc glVertex3dv uses eax, v:dword
	mov eax,[v]
	push 1.0
	add esp,-12
	fld qword[eax+16]
	fstp dword[esp+8]
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glVertex4f
	ret
endp

align 4
proc glVertex4fv uses eax, v:dword
	mov eax,[v]
	stdcall glVertex4f,[eax],[eax+4],[eax+8],[eax+12]
	ret
endp

align 4
proc glVertex4dv uses eax, v:dword
	mov eax,[v]
	add esp,-16
	fld qword[eax+24]
	fstp dword[esp+12]
	fld qword[eax+16]
	fstp dword[esp+8]
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glVertex4f
	ret
endp

; glNormal

align 4
glNormal3f: ;x, y, z
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Normal
	stdcall gl_add_op,eax
	pop eax
	ret 16 ;=sizeof(dd)*4

align 4
proc glNormal3d, x:qword, y:qword, z:qword
	add esp,-12
	fld qword[z]
	fstp dword[esp+8]
	fld qword[y]
	fstp dword[esp+4]
	fld qword[x]
	fstp dword[esp]
	call glNormal3f
	ret
endp

align 4
proc glNormal3fv uses eax, v:dword
	mov eax,[v]
	stdcall glNormal3f,[eax],[eax+4],[eax+8]
	ret
endp

align 4
proc glNormal3dv uses eax, v:dword
	mov eax,[v]
	add esp,-12
	fld qword[eax+16]
	fstp dword[esp+8]
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glNormal3f
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
	lea eax,[ebp-12] ;ebp-12 = &p[5]
	push eax
	add eax,4 ;ebp-8 = &p[6]
	push eax
	add eax,4 ;ebp-4 = &p[7]
	push eax
	stdcall RGBFtoRGBI,[r],[g],[b] ;call: r,g,b,&p[7],&p[6],&p[5]

	lea eax,[ebp-32] ;=sizeof(dd)*8
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glColor4d, r:qword, g:qword, b:qword, a:qword
	add esp,-16
	fld qword[a]
	fstp dword[esp+12]
	fld qword[b]
	fstp dword[esp+8]
	fld qword[g]
	fstp dword[esp+4]
	fld qword[r]
	fstp dword[esp]
	call glColor4f
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
proc glColor3d, r:qword, g:qword, b:qword
	push 1.0
	add esp,-12
	fld qword[b]
	fstp dword[esp+8]
	fld qword[g]
	fstp dword[esp+4]
	fld qword[r]
	fstp dword[esp]
	call glColor4f
	ret
endp

align 4
proc glColor3fv uses eax, v:dword
	mov eax,[v]
	stdcall glColor4f,[eax],[eax+4],[eax+8],1.0
	ret
endp

align 4
proc glColor3dv uses eax, v:dword
	mov eax,[v]
	push 1.0
	add esp,-12
	fld qword[eax+16]
	fstp dword[esp+8]
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glColor4f
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

align 4
proc glColor4dv uses eax, v:dword
	mov eax,[v]
	add esp,-16
	fld qword[eax+24]
	fstp dword[esp+12]
	fld qword[eax+16]
	fstp dword[esp+8]
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glColor4f
	ret
endp

; TexCoord

align 4
glTexCoord4f: ;s, t, r, q
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_TexCoord
	stdcall gl_add_op,eax
	pop eax
	ret 20 ;=sizeof(dd)*5

align 4
proc glTexCoord4d, s:qword, t:qword, r:qword, q:qword
	add esp,-16
	fld qword[q]
	fstp dword[esp+12]
	fld qword[r]
	fstp dword[esp+8]
	fld qword[t]
	fstp dword[esp+4]
	fld qword[s]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
proc glTexCoord1f, s:dword
	stdcall glTexCoord4f,[s],0.0,0.0,1.0
	ret
endp

align 4
proc glTexCoord1d, s:qword
	push 1.0
	push 0.0
	push 0.0
	add esp,-4
	fld qword[s]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
proc glTexCoord1fv uses eax, v:dword
	mov eax,[v]
	stdcall glTexCoord4f,[eax],0.0,0.0,1.0
	ret
endp

align 4
proc glTexCoord1dv uses eax, v:dword
	mov eax,[v]
	push 1.0
	push 0.0
	push 0.0
	add esp,-4
	fld qword[eax]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
proc glTexCoord2f, s:dword, t:dword
	stdcall glTexCoord4f,[s],[t],0.0,1.0
	ret
endp

align 4
proc glTexCoord2d, s:qword, t:qword
	push 1.0
	push 0.0
	add esp,-8
	fld qword[t]
	fstp dword[esp+4]
	fld qword[s]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
proc glTexCoord2fv uses eax, v:dword
	mov eax,[v]
	stdcall glTexCoord4f,[eax],[eax+4],0.0,1.0
	ret
endp

align 4
proc glTexCoord2dv uses eax, v:dword
	mov eax,[v]
	push 1.0
	push 0.0
	add esp,-8
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
proc glTexCoord3f, s:dword, t:dword, r:dword
	stdcall glTexCoord4f,[s],[t],[r],1.0
	ret
endp

align 4
proc glTexCoord3d, s:qword, t:qword, r:qword
	push 1.0
	add esp,-12
	fld qword[r]
	fstp dword[esp+8]
	fld qword[t]
	fstp dword[esp+4]
	fld qword[s]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
proc glTexCoord3fv uses eax, v:dword
	mov eax,[v]
	stdcall glTexCoord4f,[eax],[eax+4],[eax+8],1.0
	ret
endp

align 4
proc glTexCoord3dv uses eax, v:dword
	mov eax,[v]
	push 1.0
	add esp,-12
	fld qword[eax+16]
	fstp dword[esp+8]
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
proc glTexCoord4fv uses eax, v:dword
	mov eax,[v]
	stdcall glTexCoord4f,[eax],[eax+4],[eax+8],[eax+12]
	ret
endp

align 4
proc glTexCoord4dv uses eax, v:dword
	mov eax,[v]
	add esp,-16
	fld qword[eax+24]
	fstp dword[esp+12]
	fld qword[eax+16]
	fstp dword[esp+8]
	fld qword[eax+8]
	fstp dword[esp+4]
	fld qword[eax]
	fstp dword[esp]
	call glTexCoord4f
	ret
endp

align 4
glEdgeFlag: ;flag
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_EdgeFlag
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

; misc

align 4
glShadeModel: ;mode
;  assert(mode == GL_FLAT || mode == GL_SMOOTH);
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_ShadeModel
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

align 4
glCullFace: ;mode
;  assert(mode == GL_BACK || 
;         mode == GL_FRONT || 
;         mode == GL_FRONT_AND_BACK);
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_CullFace
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

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

	lea eax,[ebp-8] ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
glPolygonMode: ;face, mode
;  assert(face == GL_BACK || 
;         face == GL_FRONT || 
;         face == GL_FRONT_AND_BACK);
;  assert(mode == GL_POINT || mode == GL_LINE || mode==GL_FILL);
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_PolygonMode
	stdcall gl_add_op,eax
	pop eax
	ret 12 ;=sizeof(dd)*3

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

	lea eax,[ebp-12] ;=sizeof(dd)*3
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

	lea eax,[ebp-12] ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

; glBegin / glEnd

align 4
glBegin: ;mode
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Begin
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

align 4
glEnd:
	stdcall gl_add_op,op_End
	ret

; matrix

align 4
glMatrixMode: ;mode
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_MatrixMode
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

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

	lea ecx,[ebp-68] ;=sizeof(dd)*17
	stdcall gl_add_op,ecx
	ret
endp

align 4
glLoadIdentity:
	stdcall gl_add_op,op_LoadIdentity
	ret

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

	lea ecx,[ebp-68] ;=sizeof(dd)*17
	stdcall gl_add_op,ecx
	ret
endp

align 4
glPushMatrix:
	stdcall gl_add_op,op_PushMatrix
	ret

align 4
glPopMatrix:
	stdcall gl_add_op,op_PopMatrix
	ret

align 4
glRotatef: ;angle, x, y, z
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Rotate
	stdcall gl_add_op,eax
	pop eax
	ret 20 ;=sizeof(dd)*5

align 4
glTranslatef: ;x, y, z
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Translate
	stdcall gl_add_op,eax
	pop eax
	ret 16 ;=sizeof(dd)*4

align 4
glScalef: ;x, y, z
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Scale
	stdcall gl_add_op,eax
	pop eax
	ret 16 ;=sizeof(dd)*4

align 4
glViewport: ;x, y, width, heigh
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Viewport
	stdcall gl_add_op,eax
	pop eax
	ret 20 ;=sizeof(dd)*5

align 4
proc glFrustum uses eax, left:qword, right:qword, bottom:qword, top:qword,\
	near:qword, farv:qword
locals
	p rd 7
endl
	mov dword[p],OP_Frustum
	fld qword[left]
	fstp dword[p+4]
	fld qword[right]
	fstp dword[p+8]
	fld qword[bottom]
	fstp dword[p+12]
	fld qword[top]
	fstp dword[p+16]
	fld qword[near]
	fstp dword[p+20]
	fld qword[farv]
	fstp dword[p+24]

	lea eax,[ebp-28] ;=sizeof(dd)*7
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glOrtho uses eax, left:qword, right:qword, bottom:qword, top:qword,\
	near:qword, farv:qword
locals
	p rd 7
endl
	mov dword[p],OP_Ortho
	fld qword[left]
	fstp dword[p+4]
	fld qword[right]
	fstp dword[p+8]
	fld qword[bottom]
	fstp dword[p+12]
	fld qword[top]
	fstp dword[p+16]
	fld qword[near]
	fstp dword[p+20]
	fld qword[farv]
	fstp dword[p+24]

	lea eax,[ebp-28] ;=sizeof(dd)*7
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

	lea eax,[ebp-28] ;=sizeof(dd)*7
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

	lea eax,[ebp-28] ;=sizeof(dd)*7
	stdcall gl_add_op,eax
	ret
endp

align 4
glColorMaterial: ;mode, type
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_ColorMaterial
	stdcall gl_add_op,eax
	pop eax
	ret 12 ;=sizeof(dd)*3

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

	lea eax,[ebp-28] ;=sizeof(dd)*7
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

	lea eax,[ebp-28] ;=sizeof(dd)*7
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

	lea eax,[ebp-24] ;=sizeof(dd)*6
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

	lea eax,[ebp-24] ;=sizeof(dd)*6
	stdcall gl_add_op,eax
	ret
endp

; clear

align 4
glClear: ;mask
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Clear
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

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

	lea eax,[ebp-20] ;=sizeof(dd)*5
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

	lea eax,[ebp-8] ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

; textures

align 4
glTexImage2D: ;target, level, components, width, height, border, format, type, pixels
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_TexImage2D
	stdcall gl_add_op,eax
	pop eax
	ret 40 ;=sizeof(dd)*10

align 4
glBindTexture: ;target, texture
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_BindTexture
	stdcall gl_add_op,eax
	pop eax
	ret 12 ;=sizeof(dd)*3

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

	lea eax,[ebp-32] ;=sizeof(dd)*8
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

	lea eax,[ebp-32] ;=sizeof(dd)*8
	stdcall gl_add_op,eax
	ret
endp

align 4
glPixelStorei: ;pname, param
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_PixelStore
	stdcall gl_add_op,eax
	pop eax
	ret 12 ;=sizeof(dd)*3

; selection

align 4
glInitNames:
	stdcall gl_add_op,op_InitNames
	ret

align 4
glPushName: ;name
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_PushName
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

align 4
glPopName:
	stdcall gl_add_op,op_PopName
	ret

align 4
glLoadName: ;name
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_LoadName
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

align 4
glPolygonOffset: ;factor, units
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_PolygonOffset
	stdcall gl_add_op,eax
	pop eax
	ret 12 ;=sizeof(dd)*3

; Special Functions

align 4
glCallList: ;list
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_CallList
	stdcall gl_add_op,eax
	pop eax
	ret 8 ;=sizeof(dd)*2

align 4
proc glFlush ;(void)
	;nothing to do
	ret
endp

align 4
glHint: ;target, mode
	push dword[esp] ;копируем адрес возврата
	push eax
	lea eax,[esp+8]
	mov dword[eax],OP_Hint
	stdcall gl_add_op,eax
	pop eax
	ret 12 ;=sizeof(dd)*3

; Non standard functions

align 4
proc glDebug uses eax, mode:dword
	call gl_get_context ;после вызова функции в eax указатель на GLContext
	push dword[mode]
	pop dword[eax+GLContext.print_flag]
	ret
endp
