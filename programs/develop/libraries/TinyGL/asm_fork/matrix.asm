
if DEBUG
align 4
proc gl_print_matrix uses eax ebx ecx edi, m:dword, rows:dword
	mov ecx,[rows]
	cmp ecx,1
	jl .end_f
	mov ebx,[m]
	mov word[NumberSymbolsAD],3
	finit
	@@:
		lea edi,[buf_param]
		mov byte[edi],0

		fld dword[ebx]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		stdcall str_cat, edi,Data_String

		stdcall str_n_cat,edi,txt_zp_sp,2
		stdcall str_len,edi
		add edi,eax

		fld dword[ebx+4]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		stdcall str_cat, edi,Data_String

		stdcall str_n_cat,edi,txt_zp_sp,2
		stdcall str_len,edi
		add edi,eax

		fld dword[ebx+8]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		stdcall str_cat, edi,Data_String

		stdcall str_n_cat,edi,txt_zp_sp,2
		stdcall str_len,edi
		add edi,eax

		fld dword[ebx+12]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		stdcall str_cat, edi,Data_String

		stdcall str_n_cat,edi,txt_nl,2
		stdcall dbg_print,txt_sp,buf_param
		
		add ebx,16
	dec ecx
	cmp ecx,0
	jg @b
	.end_f:
	ret
endp
end if

macro gl_matrix_update context, reg
{
	local .end_0
	xor reg,reg
	cmp dword[context+GLContext.matrix_mode],1
	jg .end_0
		inc reg
	.end_0:
	mov dword[context+GLContext.matrix_model_projection_updated],reg
}

align 4
proc glopMatrixMode uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]

	cmp dword[ebx+4],GL_MODELVIEW ;cmp p[1],...
	jne @f
		mov dword[eax+GLContext.matrix_mode],0
		jmp .end_f
	@@:
	cmp dword[ebx+4],GL_PROJECTION
	jne @f
		mov dword[eax+GLContext.matrix_mode],1
		jmp .end_f
	@@:
	cmp dword[ebx+4],GL_TEXTURE
	jne .def
		mov dword[eax+GLContext.matrix_mode],2
		jmp .end_f
	.def:
;assert(0);
	.end_f:
	ret
endp

align 4
proc glopLoadMatrix uses eax edi esi, context:dword, p:dword
	mov eax,[context]
	mov edi,[eax+GLContext.matrix_mode]
	lea edi,[eax+4*edi]
	mov edi,dword[edi+GLContext.matrix_stack_ptr]

	mov esi,[p]
	add esi,4
	stdcall gl_M4_Transpose,edi,esi ;транспонируем входную матрицу в матрицу context.matrix_stack_ptr[context.matrix_mode]

	gl_matrix_update eax,edi
	ret
endp

align 4
proc glopLoadIdentity uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[eax+GLContext.matrix_mode]
	lea ebx,[eax+4*ebx]

	stdcall gl_M4_Id,[ebx+GLContext.matrix_stack_ptr]
	gl_matrix_update eax,ebx
	ret
endp

align 4
proc glopMultMatrix uses eax edi esi, context:dword, p:dword
locals
	m M4
endl
	mov esi,[p]
	add esi,4
	lea edi,[ebp-sizeof.M4]
	stdcall gl_M4_Transpose,edi,esi ;транспонируем входную матрицу в локальную матрицу m

	mov eax,[context]
	mov esi,[eax+GLContext.matrix_mode]
	lea esi,[eax+4*esi]
	stdcall gl_M4_MulLeft,dword[esi+GLContext.matrix_stack_ptr],edi

	gl_matrix_update eax,edi
	ret
endp

align 4
proc glopPushMatrix uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[eax+GLContext.matrix_mode]

;  assert( (c->matrix_stack_ptr[ebx] - c->matrix_stack[ebx] + 1 )
;	   < c->matrix_stack_depth_max[ebx] );

	lea ebx,[eax+4*ebx+GLContext.matrix_stack_ptr]
	add dword[ebx],sizeof.M4
	mov ebx,[ebx] ;ebx = ++context.matrix_stack_ptr[context.matrix_mode]

	sub ebx,sizeof.M4
	push ebx
	add ebx,sizeof.M4
	stdcall gl_M4_Move, ebx

	gl_matrix_update eax,ebx
	ret
endp

align 4
proc glopPopMatrix uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[eax+GLContext.matrix_mode]

;  assert( c->matrix_stack_ptr[n] > c->matrix_stack[n] );

	lea ebx,[eax+4*ebx]
	sub dword[ebx+GLContext.matrix_stack_ptr],sizeof.M4

	gl_matrix_update eax,ebx
	ret
endp

align 4
proc glopRotate uses eax ebx ecx edx, context:dword, p:dword
locals
	u0 dd ?
	u1 dd ?
	u2 dd ?
	angle dd ?
	cost dd ?
	sint dd ?
	m M4
endl

	mov eax,[context]
	mov ebx,[p]
	lea ecx,[ebp-sizeof.M4] ;ecx=&m
	finit
	fldpi
	fmul dword[ebx+4]
	fdiv dword[an180f]
	fst dword[angle] ;angle = p[1].f * M_PI / 180.0
	;st0 = angle

	; simple case detection
	xor edx,edx
	fld dword[ebx+16]
	ftst
	fstsw ax
	sahf
	je @f
		inc edx
	@@:
	fstp dword[u2]
	fld dword[ebx+12]
	ftst
	fstsw ax
	sahf
	je @f
		or edx,2
	@@:
	fstp dword[u1]
	fld dword[ebx+8]
	ftst
	fstsw ax
	sahf
	je @f
		or edx,4
	@@:
	fstp dword[u0]

	;st0 = angle
	;ebx = ((u0 != 0)<<2) | ((u1 != 0)<<1) | (u2 != 0)
	or edx,edx
	jz .end_f ;если нет поворотов выход из функции
	cmp edx,4
	jne @f
		fld dword[u0]
		ftst
		fstsw ax
		ffree st0
		fincstp
		sahf
		jae .u0ch
			fchs ;if (u0 < 0) angle *= -1
		.u0ch:
		push dword 0
		fstp dword[esp-4]
		sub esp,4
		stdcall gl_M4_Rotate, ecx
		jmp .end_sw
	@@:
	cmp edx,2
	jne @f
		fld dword[u1]
		ftst
		fstsw ax
		ffree st0
		fincstp
		sahf
		jae .u1ch
			fchs ;if (u1 < 0) angle *= -1
		.u1ch:
		push dword 1
		fstp dword[esp-4]
		sub esp,4
		stdcall gl_M4_Rotate, ecx
		jmp .end_sw
	@@:
	cmp edx,1
	jne @f
		fld dword[u2]
		ftst
		fstsw ax
		ffree st0
		fincstp
		sahf
		jae .u2ch
			fchs ;if (u2 < 0) angle *= -1
		.u2ch:
		push dword 2
		fstp dword[esp-4]
		sub esp,4
		stdcall gl_M4_Rotate, ecx
		jmp .end_sw
	@@: ;default:

		; normalize vector
		fld dword[u0]
		fmul st0,st0
		fld dword[u1]
		fmul st0,st0
		faddp
		fld dword[u2]
		fmul st0,st0
		faddp
;		fst dword[len] ;len = u0*u0+u1*u1+u2*u2
		ftst
		fstsw ax
		sahf
		je .f2 ;if (len == 0.0f) return
		fsqrt
		fld1
		fxch
		fdivp ;len = 1.0f / sqrt(len)
;		fst dword[len]
		fld dword[u0]
		fmul st0,st1
		fstp dword[u0] ;u0 *= len
		fld dword[u1]
		fmul st0,st1
		fstp dword[u1] ;u1 *= len
		fld dword[u2]
		fmul st0,st1
		fstp dword[u2] ;u2 *= len
		;st0 = len, st1 = angle
		ffree st0
		fincstp

		; store cos and sin values
		fcos
		fst dword[cost] ;cost=cos(angle)
		fld dword[angle]
		fsin
		fst dword[sint] ;sint=sin(angle)

		; fill in the values
		mov ebx,0.0
		mov [ecx+3*16   ],ebx ;m[3][0]
		mov [ecx+3*16 +4],ebx ;m[3][1]
		mov [ecx+3*16 +8],ebx ;m[3][2]
		mov [ecx+     12],ebx ;m[0][3]
		mov [ecx+  16+12],ebx ;m[1][3]
		mov [ecx+2*16+12],ebx ;m[2][3]
		mov ebx,1.0
		mov [ecx+3*16+12],ebx ;m[3][3]

		; do the math
		fld dword[u0]
		fmul st0,st0
		fld1
		fsub st0,st1
		fmul st0,st3 ;st0 = cost*(1-u0^2)
		faddp
		fstp dword[ecx] ;m[0][0] = u0*u0+cost*(1-u0*u0)

		fld1
		fsub st0,st2 ;st0 = 1-cost

		fld st0
		fmul dword[u0]
		fmul dword[u1]
		fld dword[u2]
		fmul st0,st3 ;st0 = u2*sint
		fchs
		faddp
		fstp dword[ecx+16] ;m[1][0] = u0*u1*(1-cost)-u2*sint

		fld st0
		fmul dword[u0]
		fmul dword[u2]
		fld dword[u1]
		fmul st0,st3 ;st0 = u1*sint
		faddp
		fstp dword[ecx+32] ;m.m[2][0]=u[2]*u[0]*(1-cost)+u[1]*sint

		fld st0
		fmul dword[u0]
		fmul dword[u1]
		fld dword[u2]
		fmul st0,st3 ;st0 = u2*sint
		faddp
		fstp dword[ecx+4] ;m.m[0][1]=u[0]*u[1]*(1-cost)+u[2]*sint

		fld dword[u1]
		fmul st0,st0
		fld1
		fsub st0,st1
		fmul st0,st4 ;st0 = cost*(1-u1^2)
		faddp
		fstp dword[ecx+20] ;m.m[1][1]=u[1]*u[1]+cost*(1-u[1]*u[1])

		fld st0
		fmul dword[u1]
		fmul dword[u2]
		fld dword[u0]
		fmul st0,st3 ;st0 = u0*sint
		fchs
		faddp
		fstp dword[ecx+36] ;m.m[2][1]=u[1]*u[2]*(1-cost)-u[0]*sint

		fld st0
		fmul dword[u0]
		fmul dword[u2]
		fld dword[u1]
		fmul st0,st3 ;st0 = u1*sint
		fchs
		faddp
		fstp dword[ecx+8] ;m.m[0][2]=u[2]*u[0]*(1-cost)-u[1]*sint

		fld st0
		fmul dword[u1]
		fmul dword[u2]
		fld dword[u0]
		fmul st0,st3 ;st0 = u0*sint
		faddp
		fstp dword[ecx+24] ;m.m[1][2]=u[1]*u[2]*(1-cost)+u[0]*sint

		ffree st0 ;1-cost
		fincstp

		fld dword[u2]
		fmul st0,st0
		fld1
		fsub st0,st1
		fmul st0,st3 ;st0 = cost*(1-u2^2)
		faddp
		fstp dword[ecx+40] ;m[2][2] = u2*u2+cost*(1-u2*u2)

		ffree st0 ;sint
  fincstp
		ffree st0 ;cost
  fincstp
	.end_sw:

	mov eax,[context]
	mov ebx,[eax+GLContext.matrix_mode]
	lea ebx,[eax+4*ebx]
	stdcall gl_M4_MulLeft,dword[ebx+GLContext.matrix_stack_ptr],ecx
	gl_matrix_update eax,ebx
	jmp .end_f
	.f2:
		ffree st0 ;len
		fincstp
		ffree st0 ;angle
		fincstp
	.end_f:
	ret
endp

align 4
proc glopScale uses eax ebx ecx, context:dword, p:dword
	mov ecx,[p]

	mov eax,[context]
	mov ebx,[eax+GLContext.matrix_mode]
	lea ebx,[eax+4*ebx]
	mov ebx,[ebx+GLContext.matrix_stack_ptr] ;ebx = &m[0]

	fld dword[ecx+ 4] ;x
	fld dword[ecx+ 8] ;y
	fld dword[ecx+12] ;z
	mov ecx,4
	@@:
	fld dword[ebx]   ;m[0]
	fmul st0,st3     ;m[0] * x
	fstp dword[ebx]  ;m[0] *= x
	fld dword[ebx+4] ;m[1]
	fmul st0,st2     ;m[1] * y
	fstp dword[ebx+4];m[1] *= y
	fld dword[ebx+8] ;m[2]
	fmul st0,st1     ;m[2] * z
	fstp dword[ebx+8];m[2] *= z
	add ebx,16
	loop @b
	ffree st0
	fincstp
	ffree st0
	fincstp
	ffree st0
	fincstp

	gl_matrix_update eax,ebx
	ret
endp

align 4
proc glopTranslate uses eax ebx ecx, context:dword, p:dword
	mov ecx,[p]

	mov eax,[context]
	mov ebx,[eax+GLContext.matrix_mode]
	lea ebx,[eax+4*ebx]
	mov ebx,[ebx+GLContext.matrix_stack_ptr] ;ebx = &m[0]

	fld dword[ecx+ 4] ;x
	fld dword[ecx+ 8] ;y
	fld dword[ecx+12] ;z
	mov ecx,4
	@@:
		fld dword[ebx]   ;m[0]
		fmul st0,st3     ;m[0] * x
		fld dword[ebx+4] ;m[1]
		fmul st0,st3     ;m[1] * y
		faddp
		fld dword[ebx+8] ;m[2]
		fmul st0,st2     ;m[2] * z
		faddp
		fadd dword[ebx+12] ;m[3]
		fstp dword[ebx+12] ;m[3] = m[0] * x + m[1] * y + m[2] * z + m[3]
		add ebx,16
	loop @b
	ffree st0
	fincstp
	ffree st0
	fincstp
	ffree st0
	fincstp

	gl_matrix_update eax,ebx
	ret
endp

align 4
proc glopFrustum uses eax ebx ecx, context:dword, p:dword
locals
	x dd ?
	y dd ?
	A dd ?
	B dd ?
	C dd ?
	D dd ?
	m M4
endl
	mov eax,[context]
	mov ebx,[p]

	fld dword[ebx+8]
	fsub dword[ebx+4]  ;st0 = (right-left)
	fld dword[ebx+20]  ;st0 = near
	fadd st0,st0
	fdiv st0,st1
	fstp dword[x]      ;x = (2.0*near) / (right-left)
	fld dword[ebx+16]
	fsub dword[ebx+12] ;st0 = (top-bottom)
	fld dword[ebx+20]  ;st0 = near
	fadd st0,st0
	fdiv st0,st1
	fstp dword[y]      ;y = (2.0*near) / (top-bottom)
	fld dword[ebx+8]
	fadd dword[ebx+4]  ;st0 = (right+left)
	fdiv st0,st2       ;st2 = (right-left)
	fstp dword[A]      ;A = (right+left) / (right-left)
	fld dword[ebx+16]
	fadd dword[ebx+12] ;st0 = (top+bottom)
	fdiv st0,st1       ;st1 = (top-bottom)
	fstp dword[B]      ;B = (top+bottom) / (top-bottom)
	fld dword[ebx+24]
	fsub dword[ebx+20] ;st0 = (farp-near)
	fld dword[ebx+24]
	fadd dword[ebx+20]
	fchs ;st0 = -(farp+near)
	fdiv st0,st1
	fstp dword[C]      ;C = -(farp+near) / (farp-near)
	fld dword[ebx+24]
	fmul dword[ebx+20] ;st0 = farp*near
	fadd st0,st0
	fchs               ;st0 = -(2.0*farp*near)
	fdiv st0,st1
	fstp dword[D]      ;D = -(2.0*farp*near) / (farp-near)
	ffree st0
	fincstp
	ffree st0
	fincstp
	ffree st0
	fincstp

	mov ecx,ebp
	sub ecx,sizeof.M4

	mov ebx,[x]
	mov dword[ecx],ebx
	mov dword[ecx+4],0.0
	mov ebx,[A]
	mov dword[ecx+8],ebx
	mov dword[ecx+12],0.0
	mov dword[ecx+16],0.0
	mov ebx,[y]
	mov dword[ecx+20],ebx
	mov ebx,[B]
	mov dword[ecx+24],ebx
	mov dword[ecx+28],0.0
	mov dword[ecx+32],0.0
	mov dword[ecx+36],0.0
	mov ebx,[C]
	mov dword[ecx+40],ebx
	mov ebx,[D]
	mov dword[ecx+44],ebx
	mov dword[ecx+48],0.0
	mov dword[ecx+52],0.0
	mov dword[ecx+56],-1.0
	mov dword[ecx+60],0.0

	mov ebx,[eax+GLContext.matrix_mode]
	shl ebx,2
	add ebx,eax
	stdcall gl_M4_MulLeft,dword[ebx+GLContext.matrix_stack_ptr],ecx

if DEBUG ;glopFrustum
	stdcall gl_print_matrix,ecx,4
	stdcall gl_print_matrix,dword[ebx+GLContext.matrix_stack_ptr],4
end if
	gl_matrix_update eax,ebx
	ret
endp

align 4
proc glopOrtho uses eax ebx ecx, context:dword, p:dword
locals
	x dd ?
	y dd ?
	A dd ?
	B dd ?
	C dd ?
	D dd ?
	m M4
endl
	mov eax,[context]
	mov ebx,[p]

	fld dword[ebx+8]
	fsub dword[ebx+4]  ;st0 = (right-left)
	fld1
	fadd st0,st0       ;st0 = 2.0
	fdiv st0,st1
	fstp dword[x]      ;x = 2.0 / (right-left)
	fld dword[ebx+16]
	fsub dword[ebx+12] ;st0 = (top-bottom)
	fld1
	fadd st0,st0       ;st0 = 2.0
	fdiv st0,st1
	fstp dword[y]      ;y = 2.0 / (top-bottom)
	fld dword[ebx+8]
	fadd dword[ebx+4]
	fchs               ;st0 = -(right+left)
	fdiv st0,st2       ;st2 = (right-left)
	fstp dword[A]      ;A = -(right+left) / (right-left)
	fld dword[ebx+16]
	fadd dword[ebx+12]
	fchs               ;st0 = -(top+bottom)
	fdiv st0,st1       ;st1 = (top-bottom)
	fstp dword[B]      ;B = -(top+bottom) / (top-bottom)
	fld dword[ebx+24]
	fsub dword[ebx+20] ;st0 = (farp-near)
	fld1
	fadd st0,st0
	fchs ;st0 = -2.0
	fdiv st0,st1
	fstp dword[C]      ;C = -2.0 / (farp-near)
	fld dword[ebx+24]
	fadd dword[ebx+20] ;st0 = farp+near
	fchs               ;st0 = -(farp+near)
	fdiv st0,st1
	fstp dword[D]      ;D = -(farp*near) / (farp-near)
	ffree st0
	fincstp
	ffree st0
	fincstp
	ffree st0
	fincstp

	lea ecx,[ebp-sizeof.M4]

	mov ebx,[x]
	mov dword[ecx],ebx
	mov dword[ecx+4],0.0
	mov dword[ecx+8],0.0
	mov ebx,[A]
	mov dword[ecx+12],ebx
	mov dword[ecx+16],0.0
	mov ebx,[y]
	mov dword[ecx+20],ebx
	mov dword[ecx+24],0.0
	mov ebx,[B]
	mov dword[ecx+28],ebx
	mov dword[ecx+32],0.0
	mov dword[ecx+36],0.0
	mov ebx,[C]
	mov dword[ecx+40],ebx
	mov ebx,[D]
	mov dword[ecx+44],ebx
	mov dword[ecx+48],0.0
	mov dword[ecx+52],0.0
	mov dword[ecx+56],0.0
	mov dword[ecx+60],1.0

	mov ebx,[eax+GLContext.matrix_mode]
	lea ebx,[eax+4*ebx]
	stdcall gl_M4_MulLeft,dword[ebx+GLContext.matrix_stack_ptr],ecx

if DEBUG ;glopOrtho
	stdcall gl_print_matrix,ecx,4
	stdcall gl_print_matrix,dword[ebx+GLContext.matrix_stack_ptr],4
end if
	gl_matrix_update eax,ebx
	ret
endp
