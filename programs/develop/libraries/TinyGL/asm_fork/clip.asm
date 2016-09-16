; fill triangle profile
; #define PROFILE

CLIP_XMIN equ (1<<0)
CLIP_XMAX equ (1<<1)
CLIP_YMIN equ (1<<2)
CLIP_YMAX equ (1<<3)
CLIP_ZMIN equ (1<<4)
CLIP_ZMAX equ (1<<5)

align 16
proc gl_transform_to_viewport uses eax ebx ecx, context:dword,v:dword
	mov eax,[context]
	mov ebx,[v]

	; coordinates
	fld1
	fdiv dword[ebx+offs_vert_pc+offs_W] ;st0 = 1/v.pc.W

	fld dword[ebx+offs_vert_pc+offs_X] ;st0 = v.pc.X
	fmul st0,st1
	fmul dword[eax+GLContext.viewport+offs_vpor_scale+offs_X]
	fadd dword[eax+GLContext.viewport+offs_vpor_trans+offs_X]
	fistp dword[ebx+offs_vert_zp] ;v.zp.x = st0, st0 = st1

	fld dword[ebx+offs_vert_pc+offs_Y] ;st0 = v.pc.Y
	fmul st0,st1
	fmul dword[eax+GLContext.viewport+offs_vpor_scale+offs_Y]
	fadd dword[eax+GLContext.viewport+offs_vpor_trans+offs_Y]
	fistp dword[ebx+offs_vert_zp+offs_zbup_y] ;v.zp.y = st0, st0 = st1

	fld dword[ebx+offs_vert_pc+offs_Z] ;st0 = v.pc.Z
	fmulp
	fmul dword[eax+GLContext.viewport+offs_vpor_scale+offs_Z]
	fadd dword[eax+GLContext.viewport+offs_vpor_trans+offs_Z]
	fistp dword[ebx+offs_vert_zp+offs_zbup_z] ;v.zp.z = st0, st0 = st1

	; color
	cmp dword[eax+GLContext.lighting_enabled],0 ;if (context.lighting_enabled)
	je @f
		mov ecx,ebx
		add ecx,offs_vert_zp+offs_zbup_b
		push ecx
		add ecx,offs_zbup_g-offs_zbup_b
		push ecx
		add ecx,offs_zbup_r-offs_zbup_g
		push ecx
		stdcall RGBFtoRGBI, dword[ebx+offs_vert_color],dword[ebx+offs_vert_color+4],dword[ebx+offs_vert_color+8]
		jmp .end_if
align 4
	@@:
		; no need to convert to integer if no lighting : take current color
		mov ecx,[eax+GLContext.longcurrent_color]
		mov dword[ebx+offs_vert_zp+offs_zbup_r],ecx
		mov ecx,[eax+GLContext.longcurrent_color+4]
		mov dword[ebx+offs_vert_zp+offs_zbup_g],ecx
		mov ecx,[eax+GLContext.longcurrent_color+8]
		mov dword[ebx+offs_vert_zp+offs_zbup_b],ecx
	.end_if:
  
	; texture
	cmp dword[eax+GLContext.texture_2d_enabled],0
	je @f
		mov eax,[eax+GLContext.current_texture] ;eax = &context.current_texture
		mov eax,[eax] ;eax = context.current_texture
		;[eax+offs_text_images] = im = &context.current_texture.images[0]

		fild dword[eax+offs_text_images+offs_imag_s_bound]
		fmul dword[ebx+offs_vert_tex_coord+offs_X]
		fistp dword[ebx+offs_vert_zp+offs_zbup_s]
		;v.zp.s=(int)(v.tex_coord.X * im.s_bound)

		fild dword[eax+offs_text_images+offs_imag_t_bound]
		fmul dword[ebx+offs_vert_tex_coord+offs_Y]
		fistp dword[ebx+offs_vert_zp+offs_zbup_t]
		;v.zp.t=(int)(v.tex_coord.Y * im.t_bound)
	@@:
	ret
endp

align 16
proc gl_add_select1 uses eax ebx ecx, context:dword, z1:dword,z2:dword,z3:dword
	mov eax,[z1]
	mov ebx,eax
	cmp [z2],eax
	jge @f
		mov eax,[z2]
	@@:
	cmp [z3],eax
	jge @f
		mov eax,[z3]
	@@:
	cmp [z2],ebx
	jle @f
		mov ebx,[z2]
	@@:
	cmp [z3],ebx
	jle @f
		mov ebx,[z3]
	@@:
	mov ecx,0xffffffff
	sub ecx,ebx
	push ecx
	mov ecx,0xffffffff
	sub ecx,eax
	push ecx
	stdcall gl_add_select, [context] ;,0xffffffff-eax,0xffffffff-ebx
	ret
endp

; point

align 16
proc gl_draw_point uses eax ebx, context:dword, p0:dword
	mov ebx,[p0]
	cmp dword[ebx+offs_vert_clip_code],0 ;if (p0.clip_code == 0)
	jne @f
	mov eax,[context]
	cmp dword[eax+GLContext.render_mode],GL_SELECT
	jne .els
		stdcall gl_add_select, eax,dword[ebx+offs_vert_zp+offs_zbup_z],dword[ebx+offs_vert_zp+offs_zbup_z] ;p0.zp.z,p0.zp.z
		jmp @f
align 4
	.els:
		add ebx,offs_vert_zp
		stdcall ZB_plot, dword[eax+GLContext.zb],ebx
	@@:
	ret
endp

; line

;input:
;q - регистр с адресом вершины для интерполяции
;p0 - регистр с адресом 1-й вершины
;p1 - регистр с адресом 2-й вершины
;t - float
macro interpolate q, p0, p1, t
{
	fld dword[t]

	; интерполяция по координатам
	fld dword[p1+offs_vert_pc]
	fsub dword[p0+offs_vert_pc]
	fmul st0,st1
	fadd dword[p0+offs_vert_pc]
	fstp dword[q+offs_vert_pc] ;q.pc.X = p0.pc.X + (p1.pc.X - p0.pc.X) * t

	fld dword[p1+offs_vert_pc+offs_Y]
	fsub dword[p0+offs_vert_pc+offs_Y]
	fmul st0,st1
	fadd dword[p0+offs_vert_pc+offs_Y]
	fstp dword[q+offs_vert_pc+offs_Y]

	fld dword[p1+offs_vert_pc+offs_Z]
	fsub dword[p0+offs_vert_pc+offs_Z]
	fmul st0,st1
	fadd dword[p0+offs_vert_pc+offs_Z]
	fstp dword[q+offs_vert_pc+offs_Z]

	fld dword[p1+offs_vert_pc+offs_W]
	fsub dword[p0+offs_vert_pc+offs_W]
	fmul st0,st1
	fadd dword[p0+offs_vert_pc+offs_W]
	fstp dword[q+offs_vert_pc+offs_W]

	; интерполяция по цвету
	fld dword[p1+offs_vert_color]
	fsub dword[p0+offs_vert_color]
	fmul st0,st1
	fadd dword[p0+offs_vert_color]
	fstp dword[q+offs_vert_color]

	fld dword[p1+offs_vert_color+4]
	fsub dword[p0+offs_vert_color+4]
	fmul st0,st1
	fadd dword[p0+offs_vert_color+4]
	fstp dword[q+offs_vert_color+4]

	fld dword[p1+offs_vert_color+8]
	fsub dword[p0+offs_vert_color+8]
	fmulp
	fadd dword[p0+offs_vert_color+8]
	fstp dword[q+offs_vert_color+8]
}

;
; Line Clipping 
;

; Line Clipping algorithm from 'Computer Graphics', Principles and
; Practice
; tmin,tmax -> &float
align 16
proc ClipLine1 uses ebx, denom:dword,num:dword,tmin:dword,tmax:dword
	fld dword[denom]
	ftst
	fstsw ax
	sahf
	jbe .els_0 ;if (denom>0)
		fld dword[num]
		fxch st1
		fdivp ;t=num/denom
		mov ebx,[tmax]
		fcom dword[ebx]
		fstsw ax
		sahf
		ja .r0_f1 ;if (t>*tmax) return 0
		mov ebx,[tmin]
		fcom dword[ebx]
		fstsw ax
		sahf
		jbe .r1_f1 ;if (t>*tmin) *tmin=t
			fstp dword[ebx] 
		jmp .r1
align 4
	.els_0: ;else if (denom<0)
		jae .els_1
		fld dword[num]
		fxch st1
		fdivp ;t=num/denom
		mov ebx,[tmin]
		fcom dword[ebx]
		fstsw ax
		sahf
		jb .r0_f1 ;if (t<*tmin) return 0
		mov ebx,[tmax]
		fcom dword[ebx]
		fstsw ax
		sahf
		jae .r1_f1
			fstp dword[ebx] ;if (t<*tmin) *tmax=t
		jmp .r1
align 4
	.els_1: ;else if (num>0)
		ffree st0 ;denom
		fincstp
		fld dword[num]
		ftst
		fstsw ax
		sahf
		ja .r0_f1 ;if (num>0) return 0
		jmp .r1_f1
align 4
	.r0_f1: ;return 0 & free st0
		ffree st0
		fincstp
	.r0: ;return 0
		xor eax,eax
		jmp .end_f
align 4
	.r1_f1: ;return 1 & free st0
		ffree st0
		fincstp
	.r1: ;return 1
		xor eax,eax
		inc eax
	.end_f:
	ret
endp

align 16
proc gl_draw_line, context:dword, p1:dword, p2:dword
locals
	d_x dd ?
	d_y dd ?
	d_z dd ?
	d_w dd ?
	x1 dd ?
	y1 dd ?
	z1 dd ?
	w1 dd ?
	q1 GLVertex ?
	q2 GLVertex ?
	tmin dd ? ;ebp-8
	tmax dd ? ;ebp-4
endl
pushad
	mov edx,[context]
	mov edi,[p1]
	mov esi,[p2]

	cmp dword[edi+offs_vert_clip_code],0
	jne .els_i
	cmp dword[esi+offs_vert_clip_code],0
	jne .els_i
		;if ( (p1.clip_code | p2.clip_code) == 0)
		cmp dword[edx+GLContext.render_mode],GL_SELECT ;if (context.render_mode == GL_SELECT)
		jne .els_1
			stdcall gl_add_select1, edx,dword[edi+offs_vert_zp+offs_zbup_z],\
				dword[esi+offs_vert_zp+offs_zbup_z],dword[esi+offs_vert_zp+offs_zbup_z]
			jmp .end_f
align 4
		.els_1:
			add edi,offs_vert_zp
			add esi,offs_vert_zp
			push esi
			push edi
			push dword[edx+GLContext.zb]
			cmp dword[edx+GLContext.depth_test],0
			je .els_2
				;if (context.depth_test)
				call ZB_line_z ;, dword[edx+GLContext.zb],edi,esi
				jmp .end_f
align 4
			.els_2:
				call ZB_line ;, dword[edx+GLContext.zb],edi,esi
				jmp .end_f
align 4
	.els_i:
		;else if ( (p1.clip_code & p2.clip_code) != 0 )
		mov eax,[edi+offs_vert_clip_code]
		and eax,[esi+offs_vert_clip_code]
		or eax,eax
		jnz .end_f
	.els_0:

	fld dword[esi+offs_vert_pc+offs_X]
	fsub dword[edi+offs_vert_pc+offs_X]
	fstp dword[d_x] ;d_x = p2.pc.X - p1.pc.X
	fld dword[esi+offs_vert_pc+offs_Y]
	fsub dword[edi+offs_vert_pc+offs_Y]
	fstp dword[d_y] ;d_y = p2.pc.Y - p1.pc.Y
	fld dword[esi+offs_vert_pc+offs_Z]
	fsub dword[edi+offs_vert_pc+offs_Z]
	fstp dword[d_z] ;d_z = p2.pc.Z - p1.pc.Z
	fld dword[esi+offs_vert_pc+offs_W]
	fsub dword[edi+offs_vert_pc+offs_W]
	fstp dword[d_w] ;d_w = p2.pc.W - p1.pc.W

	mov eax,[edi+offs_vert_pc+offs_X]
	mov [x1],eax ;x1 = p1.pc.X
	mov eax,[edi+offs_vert_pc+offs_Y]
	mov [y1],eax ;y1 = p1.pc.Y
	mov eax,[edi+offs_vert_pc+offs_Z]
	mov [z1],eax ;z1 = p1.pc.Z
	mov eax,[edi+offs_vert_pc+offs_W]
	mov [w1],eax ;w1 = p1.pc.W

	mov dword[tmin],0.0
	mov dword[tmax],1.0

	mov eax,ebp
	sub eax,4
	push eax ;толкаем в стек адрес &tmax
	sub eax,4
	push eax ;толкаем в стек адрес &tmin
	fld dword[x1]
	fadd dword[w1]
	fchs
	fstp dword[esp-4]
	fld dword[d_x]
	fadd dword[d_w]
	fstp dword[esp-8]
	sub esp,8
	call ClipLine1 ;d_x+d_w,-x1-w1,&tmin,&tmax
	bt eax,0
	jnc .end_f

	sub esp,8 ;толкаем в стек адреса переменных &tmin и &tmax
	fld dword[x1]
	fsub dword[w1]
	fstp dword[esp-4]
	fld dword[d_w]
	fsub dword[d_x]
	fstp dword[esp-8]
	sub esp,8
	call ClipLine1 ;-d_x+d_w,x1-w1,&tmin,&tmax
	bt eax,0
	jnc .end_f

	sub esp,8 ;толкаем в стек адреса переменных &tmin и &tmax
	fld dword[y1]
	fadd dword[w1]
	fchs
	fstp dword[esp-4]
	fld dword[d_y]
	fadd dword[d_w]
	fstp dword[esp-8]
	sub esp,8
	call ClipLine1 ;d_y+d_w,-y1-w1,&tmin,&tmax
	bt eax,0
	jnc .end_f

	sub esp,8 ;толкаем в стек адреса переменных &tmin и &tmax
	fld dword[y1]
	fsub dword[w1]
	fstp dword[esp-4]
	fld dword[d_w]
	fsub dword[d_y]
	fstp dword[esp-8]
	sub esp,8
	call ClipLine1 ;-d_y+d_w,y1-w1,&tmin,&tmax
	bt eax,0
	jnc .end_f

	sub esp,8 ;толкаем в стек адреса переменных &tmin и &tmax
	fld dword[z1]
	fadd dword[w1]
	fchs
	fstp dword[esp-4]
	fld dword[d_z]
	fadd dword[d_w]
	fstp dword[esp-8]
	sub esp,8
	call ClipLine1 ;d_z+d_w,-z1-w1,&tmin,&tmax
	bt eax,0
	jnc .end_f

	sub esp,8 ;толкаем в стек адреса переменных &tmin и &tmax
	fld dword[z1]
	fsub dword[w1]
	fstp dword[esp-4]
	fld dword[d_w]
	fsub dword[d_z]
	fstp dword[esp-8]
	sub esp,8
	call ClipLine1 ;-d_z+d_w,z1-w1,&tmin,&tmax
	bt eax,0
	jnc .end_f

	mov eax,ebp
	sub eax,8+2*sizeof.GLVertex ;eax = &q1
	interpolate eax,edi,esi,tmin
	stdcall gl_transform_to_viewport, edx,eax
	add eax,sizeof.GLVertex ;eax = &q2
	interpolate eax,edi,esi,tmax
	stdcall gl_transform_to_viewport, edx,eax

	sub eax,sizeof.GLVertex ;eax = &q1
	mov ebx,eax
	add ebx,offs_vert_zp+offs_zbup_b
	push ebx
	add ebx,offs_zbup_g-offs_zbup_b
	push ebx
	add ebx,offs_zbup_r-offs_zbup_g
	push ebx
	stdcall RGBFtoRGBI, dword[eax+offs_vert_color],dword[eax+offs_vert_color+4],dword[eax+offs_vert_color+8]

	add eax,sizeof.GLVertex ;eax = &q2
	mov ebx,eax
	add ebx,offs_vert_zp+offs_zbup_b
	push ebx
	add ebx,offs_zbup_g-offs_zbup_b
	push ebx
	add ebx,offs_zbup_r-offs_zbup_g
	push ebx
	stdcall RGBFtoRGBI, dword[eax+offs_vert_color],dword[eax+offs_vert_color+4],dword[eax+offs_vert_color+8]

	add eax,offs_vert_zp ;eax = &q2.zp
	push eax
	sub eax,sizeof.GLVertex ;eax = &q1.zp
	push eax
	push dword[edx+GLContext.zb]
	cmp dword[edx+GLContext.depth_test],0
	je .els_3
		call ZB_line_z ;(context.zb,&q1.zp,&q2.zp)
		jmp .end_f
align 4
	.els_3:
		call ZB_line ;(context.zb,&q1.zp,&q2.zp)
	.end_f:
popad
	ret
endp

; triangle

;
; Clipping
;

; We clip the segment [a,b] against the 6 planes of the normal volume.
; We compute the point 'c' of intersection and the value of the parameter 't'
; of the intersection if x=a+t(b-a). 
;
; sign: 0 -> '-', 1 -> '+'
macro clip_func sign,dir,dir1,dir2
{
locals
	t dd ?
	d_X dd ?
	d_Y dd ?
	d_Z dd ?
	d_W dd ?
endl
	mov edx,[a]
	mov ebx,[b]
	mov ecx,[c]
	fld dword[ebx+offs_X]
	fsub dword[edx+offs_X]
	fstp dword[d_X] ;d_X = (b.X - a.X)
	fld dword[ebx+offs_Y]
	fsub dword[edx+offs_Y]
	fstp dword[d_Y] ;d_Y = (b.Y - a.Y)
	fld dword[ebx+offs_Z]
	fsub dword[edx+offs_Z]
	fstp dword[d_Z] ;d_Z = (b.Z - a.Z)
	fld dword[ebx+offs_W]
	fsub dword[edx+offs_W]
	fst dword[d_W] ;d_W = (b.W - a.W)
if sign eq 0
	fadd dword[d#dir]
else
	fsub dword[d#dir]
end if

	ftst
	fstsw ax
	sahf
	jne @f
		fldz
		fst dword[t] ;t=0
		jmp .e_zero
align 4
	@@: ;else
		fld dword[edx+offs#dir]
if sign eq 0		
		fchs
end if
		fsub dword[edx+offs_W]
		fdiv st0,st1
		fst dword[t] ;t = ( sign a.dir - a.W) / den
	.e_zero:

	fmul dword[d#dir1] ;st0 = t * d.dir1
	fadd dword[edx+offs#dir1]
	fstp dword[ecx+offs#dir1] ;c.dir1 = a.dir1 + t * d.dir1

	ffree st0
	fincstp

	fld dword[t]
	fmul dword[d#dir2] ;st0 = t * d.dir2
	fadd dword[edx+offs#dir2]
	fstp dword[ecx+offs#dir2] ;c.dir2 = a.dir2 + t * d.dir2

	fld dword[t]
	fmul dword[d_W]
	fadd dword[edx+offs_W]
	fst dword[ecx+offs_W] ;c.W = a.W + t * d_W

if sign eq 0		
		fchs
end if
	fstp dword[ecx+offs#dir] ;c.dir = sign c.W
	mov eax,[t]
}

align 16
proc clip_xmin uses ebx ecx edx, c:dword, a:dword, b:dword
	clip_func 0,_X,_Y,_Z
	ret
endp

align 16
proc clip_xmax uses ebx ecx edx, c:dword, a:dword, b:dword
	clip_func 1,_X,_Y,_Z
	ret
endp

align 16
proc clip_ymin uses ebx ecx edx, c:dword, a:dword, b:dword
	clip_func 0,_Y,_X,_Z
	ret
endp

align 16
proc clip_ymax uses ebx ecx edx, c:dword, a:dword, b:dword
	clip_func 1,_Y,_X,_Z
	ret
endp

align 16
proc clip_zmin uses ebx ecx edx, c:dword, a:dword, b:dword
	clip_func 0,_Z,_X,_Y
	ret
endp

align 16
proc clip_zmax uses ebx ecx edx, c:dword, a:dword, b:dword
	clip_func 1,_Z,_X,_Y
	ret
endp

align 4
clip_proc dd clip_xmin,clip_xmax, clip_ymin,clip_ymax, clip_zmin,clip_zmax

;input:
;edi - q
align 16
proc updateTmp uses eax ecx edx, context:dword, p0:dword, p1:dword, t:dword
	mov edx,[context]
	mov eax,[p0]
	cmp dword[edx+GLContext.current_shade_model],GL_SMOOTH ;if (context.current_shade_model == GL_SMOOTH)
	jne .els_0
		mov ecx,[p1]
		fld dword[ecx+offs_vert_color]
		fsub dword[eax+offs_vert_color]
		fmul dword[t]
		fadd dword[eax+offs_vert_color]
		fstp dword[edi+offs_vert_color] ;q.color.v[0]=p0.color.v[0] + (p1.color.v[0]-p0.color.v[0])*t
		fld dword[ecx+offs_vert_color+4]
		fsub dword[eax+offs_vert_color+4]
		fmul dword[t]
		fadd dword[eax+offs_vert_color+4]
		fstp dword[edi+offs_vert_color+4] ;q.color.v[1]=p0.color.v[1] + (p1.color.v[1]-p0.color.v[1])*t
		fld dword[ecx+offs_vert_color+8]
		fsub dword[eax+offs_vert_color+8]
		fmul dword[t]
		fadd dword[eax+offs_vert_color+8]
		fstp dword[edi+offs_vert_color+8] ;q.color.v[2]=p0.color.v[2] + (p1.color.v[2]-p0.color.v[2])*t
		jmp @f
align 4
	.els_0:
		mov ecx,[eax+offs_vert_color]
		mov [edi+offs_vert_color],ecx ;q.color.v[0]=p0.color.v[0]
		mov ecx,[eax+offs_vert_color+4]
		mov [edi+offs_vert_color+4],ecx ;q.color.v[1]=p0.color.v[1]
		mov ecx,[eax+offs_vert_color+8]
		mov [edi+offs_vert_color+8],ecx ;q.color.v[2]=p0.color.v[2]
	@@:

	cmp dword[edx+GLContext.texture_2d_enabled],0 ;if (context.texture_2d_enabled)
	je @f
		mov ecx,[p1]
		fld dword[ecx+offs_vert_tex_coord+offs_X]
		fsub dword[eax+offs_vert_tex_coord+offs_X]
		fmul dword[t]
		fadd dword[eax+offs_vert_tex_coord+offs_X]
		fstp dword[edi+offs_vert_tex_coord+offs_X] ;q.tex_coord.X=p0.tex_coord.X + (p1.tex_coord.X-p0.tex_coord.X)*t
		fld dword[ecx+offs_vert_tex_coord+offs_Y]
		fsub dword[eax+offs_vert_tex_coord+offs_Y]
		fmul dword[t]
		fadd dword[eax+offs_vert_tex_coord+offs_Y]
		fstp dword[edi+offs_vert_tex_coord+offs_Y] ;q.tex_coord.Y=p0.tex_coord.Y + (p1.tex_coord.Y-p0.tex_coord.Y)*t
	@@:

	stdcall gl_clipcode, [edi+offs_vert_pc+offs_X],[edi+offs_vert_pc+offs_Y],\
		[edi+offs_vert_pc+offs_Z],[edi+offs_vert_pc+offs_W]
	mov dword[edi+offs_vert_clip_code],eax
	or eax,eax ;if (q.clip_code==0)
	jnz @f
		stdcall gl_transform_to_viewport,[context],edi
		mov eax,edi
		add eax,offs_vert_zp+offs_zbup_b
		push eax
		add eax,offs_zbup_g-offs_zbup_b
		push eax
		add eax,offs_zbup_r-offs_zbup_g
		push eax
		stdcall RGBFtoRGBI, dword[edi+offs_vert_color],dword[edi+offs_vert_color+4],dword[edi+offs_vert_color+8]
	@@:
	ret
endp

align 16
proc gl_draw_triangle, context:dword, p0:dword, p1:dword, p2:dword
locals
	cc rd 3
	front dd ?
	norm dd ? ;float
endl
pushad
	mov ebx,[p0]
	mov ecx,[p1]
	mov edx,[p2]
	mov edi,[ebx+offs_vert_clip_code]
	mov dword[cc],edi
	mov eax,[ecx+offs_vert_clip_code]
	mov dword[cc+4],eax
	or edi,eax
	mov eax,[edx+offs_vert_clip_code]
	mov dword[cc+8],eax
	or edi,eax ;co = cc[0] | cc[1] | cc[2]

	; we handle the non clipped case here to go faster
	;or edi,___ - было выше
	jnz .els_0
		;if (co==0)
		mov edi,dword[edx+offs_vert_zp+offs_zbup_x]
		sub edi,dword[ebx+offs_vert_zp+offs_zbup_x]
		mov dword[norm],edi ;p2.x-p0.x
		fild dword[norm]
		mov edi,dword[ecx+offs_vert_zp+offs_zbup_y]
		sub edi,dword[ebx+offs_vert_zp+offs_zbup_y]
		mov dword[norm],edi ;p1.y-p0.y
		fimul dword[norm]
		fchs
		mov edi,dword[ecx+offs_vert_zp+offs_zbup_x]
		sub edi,dword[ebx+offs_vert_zp+offs_zbup_x]
		mov dword[norm],edi ;p1.x-p0.x
		fild dword[norm]
		mov edi,dword[edx+offs_vert_zp+offs_zbup_y]
		sub edi,dword[ebx+offs_vert_zp+offs_zbup_y]
		mov dword[norm],edi ;p2.y-p0.y
		fimul dword[norm]
		faddp
		;st0 = (p1.zp.x-p0.zp.x)*(p2.zp.y-p0.zp.y) - (p2.zp.x-p0.zp.x)*(p1.zp.y-p0.zp.y)

		mov dword[front],0
		ftst
		fstsw ax
		ffree st0
		fincstp
		sahf
		je .end_f
		jae @f
			inc dword[front] ;front = norm < 0.0
		@@:
		mov edi,[context]
		mov eax,dword[edi+GLContext.current_front_face]
		xor dword[front],eax ;front ^= context.current_front_face

		; back face culling
		cmp dword[edi+GLContext.cull_face_enabled],0
		je .els_1
			; most used case first
			cmp dword[edi+GLContext.current_cull_face],GL_BACK
			jne @f
				cmp dword[front],0
				je .end_f
					stdcall dword[edi+GLContext.draw_triangle_front], edi,ebx,ecx,edx
				jmp .end_f
align 4
			@@:
			cmp dword[edi+GLContext.current_cull_face],GL_FRONT
			jne .end_f
				cmp dword[front],0
				jne .end_f
					stdcall dword[edi+GLContext.draw_triangle_back], edi,ebx,ecx,edx
			jmp .end_f
align 4
		.els_1:
			; no culling
			cmp dword[front],0
			je @f
				stdcall dword[edi+GLContext.draw_triangle_front], edi,ebx,ecx,edx
				jmp .end_f
align 4
			@@:
				stdcall dword[edi+GLContext.draw_triangle_back], edi,ebx,ecx,edx
		jmp .end_f
align 4
	.els_0:
		;eax = cc[2]
		and eax,[cc]
		and eax,[cc+4] ;eax = c_and = cc[0] & cc[1] & cc[2]
		or eax,eax ;if (c_and==0)
		jnz .end_f
			stdcall gl_draw_triangle_clip, [context],ebx,ecx,edx,0
	.end_f:
popad
	ret
endp

align 16
proc gl_draw_triangle_clip, context:dword, p0:dword, p1:dword, p2:dword, clip_bit:dword
locals
	co dd ?
	cc rd 3
	edge_flag_tmp dd ?
	clip_mask dd ?
	q rd 3 ;GLVertex*
	tmp1 GLVertex ?
	tmp2 GLVertex ?
endl
pushad
	mov ebx,[p0]
	mov ecx,[p1]
	mov edx,[p2]

	mov edi,[ebx+offs_vert_clip_code]
	mov [cc],edi
	mov eax,[ecx+offs_vert_clip_code]
	mov [cc+4],eax
	or edi,eax
	mov eax,[edx+offs_vert_clip_code]
	mov [cc+8],eax
	or edi,eax
	mov [co],edi ;co = cc[0] | cc[1] | cc[2]

	or edi,edi ;if (co == 0)
	jnz .els_0
		stdcall gl_draw_triangle, [context],ebx,ecx,edx
		jmp .end_f
align 4
	.els_0:
		;eax = cc[2]
		and eax,[cc]
		and eax,[cc+4] ;c_and = cc[0] & cc[1] & cc[2]

		; the triangle is completely outside
		or eax,eax ;if (c_and!=0) return
		jnz .end_f

		; find the next direction to clip
		.cycle_0: ;while (clip_bit < 6 && (co & (1 << clip_bit)) == 0)
		cmp dword[clip_bit],6
		jge .cycle_0_end
		xor eax,eax
		inc eax
		mov ecx,[clip_bit]
		shl eax,cl
		and eax,[co]
		or eax,eax
		jnz .cycle_0_end
			inc dword[clip_bit]
			jmp .cycle_0
align 4
		.cycle_0_end:

	; this test can be true only in case of rounding errors
	cmp dword[clip_bit],6
if 0
	jne @f
;      printf("Error:\n");
;      printf("%f %f %f %f\n",p0->pc.X,p0->pc.Y,p0->pc.Z,p0->pc.W);
;      printf("%f %f %f %f\n",p1->pc.X,p1->pc.Y,p1->pc.Z,p1->pc.W);
;      printf("%f %f %f %f\n",p2->pc.X,p2->pc.Y,p2->pc.Z,p2->pc.W);
		jmp .end_f
	@@:
end if
if 1
	je .end_f
end if

	xor eax,eax
	inc eax
	mov ecx,[clip_bit]
	shl eax,cl
	mov [clip_mask],eax ;1 << clip_bit
	mov edi,[cc]
	xor edi,[cc+4]
	xor edi,[cc+8]
	and eax,edi ;eax = co1 = (cc[0] ^ cc[1] ^ cc[2]) & clip_mask

	mov ecx,[p1] ;востанавливаем после shl ___,cl

	or eax,eax ;if (co1)
	jz .els_1
		; one point outside
		mov eax,[cc]
		and eax,[clip_mask]
		or eax,eax ;if (cc[0] & clip_mask)
		jz .els_2
			;q[0]=p0 q[1]=p1 q[2]=p2
			mov [q],ebx
			mov [q+4],ecx
			mov [q+8],edx
			jmp .els_2_end
align 4
		.els_2:
		mov eax,[cc+4]
		and eax,[clip_mask]
		or eax,eax ;else if (cc[1] & clip_mask)
		jz .els_3
			;q[0]=p1 q[1]=p2 q[2]=p0
			mov [q],ecx
			mov [q+4],edx
			mov [q+8],ebx
			jmp .els_2_end
align 4
		.els_3:
			;q[0]=p2 q[1]=p0 q[2]=p1
			mov [q],edx
			mov [q+4],ebx
			mov [q+8],ecx
		.els_2_end:

		mov ebx,[q]
		add ebx,offs_vert_pc
		mov ecx,[q+4]
		add ecx,offs_vert_pc
		mov edx,[q+8]
		add edx,offs_vert_pc

		lea eax,[clip_proc]
		mov edi,[clip_bit]
		shl edi,2
		add eax,edi
		mov edi,ebp
		sub edi,(2*sizeof.GLVertex)-offs_vert_pc
		stdcall dword[eax],edi,ebx,ecx ;clip_proc[clip_bit](&tmp1.pc,&q[0].pc,&q[1].pc)
		sub edi,offs_vert_pc

		sub ebx,offs_vert_pc
		sub ecx,offs_vert_pc
		stdcall updateTmp,[context],ebx,ecx,eax ;(c,&tmp1,q[0],q[1],tt)
		add ebx,offs_vert_pc

		lea eax,[clip_proc]
		mov edi,[clip_bit]
		shl edi,2
		add eax,edi
		mov edi,ebp
		sub edi,sizeof.GLVertex-offs_vert_pc
		stdcall dword[eax],edi,ebx,edx ;clip_proc[clip_bit](&tmp2.pc,&q[0].pc,&q[2].pc)
		sub edi,offs_vert_pc
		sub ebx,offs_vert_pc
		sub edx,offs_vert_pc
		stdcall updateTmp,[context],ebx,edx,eax ;(c,&tmp2,q[0],q[2],tt)

		mov eax,[ebx+offs_vert_edge_flag]
		mov [tmp1.edge_flag],eax ;q[0].edge_flag
		mov eax,[edx+offs_vert_edge_flag]
		mov [edge_flag_tmp],eax	;q[2].edge_flag
		mov dword[edx+offs_vert_edge_flag],0 ;q[2].edge_flag=0
		mov eax,[clip_bit]
		inc eax
		push eax ;для вызова нижней функции
		mov edi,ebp
		sub edi,2*sizeof.GLVertex
		stdcall gl_draw_triangle_clip,[context],edi,ecx,edx,eax ;gl_draw_triangle_clip(c,&tmp1,q[1],q[2],clip_bit+1)

		mov dword[tmp2.edge_flag],0
		mov dword[tmp1.edge_flag],0
		mov eax,[edge_flag_tmp]
		mov [edx+offs_vert_edge_flag],eax ;q[2].edge_flag=edge_flag_tmp
		push edx
		push edi
		add edi,sizeof.GLVertex ;edi = &tmp2
		stdcall gl_draw_triangle_clip,[context],edi ;gl_draw_triangle_clip(c,&tmp2,&tmp1,q[2],clip_bit+1)
		jmp .end_f
align 4
	.els_1:
		; two points outside
		mov eax,[cc]
		and eax,[clip_mask]
		cmp eax,0 ;if (cc[0] & clip_mask)==0
		jne .els_4
			;q[0]=p0 q[1]=p1 q[2]=p2
			mov [q],ebx
			mov [q+4],ecx
			mov [q+8],edx
			jmp .els_4_end
align 4
		.els_4:
		mov eax,[cc+4]
		and eax,[clip_mask]
		cmp eax,0 ;else if (cc[1] & clip_mask)==0
		jne .els_5
			;q[0]=p1 q[1]=p2 q[2]=p0
			mov [q],ecx
			mov [q+4],edx
			mov [q+8],ebx
			jmp .els_4_end
align 4
		.els_5:
			;q[0]=p2 q[1]=p0 q[2]=p1
			mov [q],edx
			mov [q+4],ebx
			mov [q+8],ecx
		.els_4_end:

		mov ebx,[q]
		add ebx,offs_vert_pc
		mov ecx,[q+4]
		add ecx,offs_vert_pc
		mov edx,[q+8]
		add edx,offs_vert_pc

		lea eax,[clip_proc]
		mov edi,[clip_bit]
		shl edi,2
		add eax,edi
		mov edi,ebp
		sub edi,(2*sizeof.GLVertex)-offs_vert_pc
		stdcall dword[eax],edi,ebx,ecx ;clip_proc[clip_bit](&tmp1.pc,&q[0].pc,&q[1].pc)
		sub edi,offs_vert_pc
		stdcall updateTmp,[context],[q],[q+4],eax

		lea eax,[clip_proc]
		mov edi,[clip_bit]
		shl edi,2
		add eax,edi
		mov edi,ebp
		sub edi,sizeof.GLVertex-offs_vert_pc
		stdcall dword[eax],edi,ebx,edx ;clip_proc[clip_bit](&tmp2.pc,&q[0].pc,&q[2].pc)
		sub edi,offs_vert_pc
		stdcall updateTmp,[context],[q],[q+8],eax

		mov dword[tmp1.edge_flag],1
		mov eax,[edx+offs_vert_edge_flag-offs_vert_pc]
		mov dword[tmp2.edge_flag],eax ;tmp2.edge_flag = q[2].edge_flag
		mov eax,[clip_bit]
		inc eax
		push eax
		push edi
		sub edi,sizeof.GLVertex
		stdcall gl_draw_triangle_clip,[context],[q],edi ;gl_draw_triangle_clip(c,q[0],&tmp1,&tmp2,clip_bit+1)
	.end_f:
popad
	ret
endp

align 16
proc gl_draw_triangle_select uses eax, context:dword, p0:dword,p1:dword,p2:dword
	mov eax,[p2]
	push dword[eax+offs_vert_zp+offs_Z]
	mov eax,[p1]
	push dword[eax+offs_vert_zp+offs_Z]
	mov eax,[p0]
	push dword[eax+offs_vert_zp+offs_Z]
	stdcall gl_add_select1, [context] ;,p0.zp.z, p1.zp.z, p2.zp.z
	ret
endp

if PROFILE eq 1
	count_triangles dd ?
	count_triangles_textured dd ?
	count_pixels dd ?
end if

align 16
proc gl_draw_triangle_fill, context:dword, p0:dword,p1:dword,p2:dword
pushad
if PROFILE eq 1
;    int norm;
;    assert(p0->zp.x >= 0 && p0->zp.x < c->zb->xsize);
;    assert(p0->zp.y >= 0 && p0->zp.y < c->zb->ysize);
;    assert(p1->zp.x >= 0 && p1->zp.x < c->zb->xsize);
;    assert(p1->zp.y >= 0 && p1->zp.y < c->zb->ysize);
;    assert(p2->zp.x >= 0 && p2->zp.x < c->zb->xsize);
;    assert(p2->zp.y >= 0 && p2->zp.y < c->zb->ysize);
    
;    norm=(p1->zp.x-p0->zp.x)*(p2->zp.y-p0->zp.y)-
;      (p2->zp.x-p0->zp.x)*(p1->zp.y-p0->zp.y);
;    count_pixels+=abs(norm)/2;
	inc dword[count_triangles]
end if

	mov ebx,[p1]
	add ebx,offs_vert_zp
	mov ecx,[p2]
	add ecx,offs_vert_zp
	mov edx,[context]
	cmp dword[edx+GLContext.texture_2d_enabled],0
	je .els_i
		;if (context.texture_2d_enabled)
if PROFILE eq 1
	inc dword[count_triangles_textured]
end if
		mov eax,[edx+GLContext.current_texture]
		mov eax,[eax] ;переход по указателю
		;так как offs_text_images+offs_imag_pixmap = 0 то context.current_texture.images[0].pixmap = [eax]
		stdcall ZB_setTexture, [edx+GLContext.zb], [eax],\
			[eax+offs_imag_s_bound],[eax+offs_imag_t_bound],[eax+offs_imag_xsize_log2]
		mov eax,[p0]
		add eax,offs_vert_zp
		push ecx
		push ebx
		push eax
		push dword[edx+GLContext.zb]
		cmp dword[edx+GLContext.matrix_model_projection_no_w_transform],0
		je @f
			call ZB_fillTriangleMappingPerspective
			jmp .end_f
align 4
		@@:
			call ZB_fillTriangleMapping
		jmp .end_f
align 4
	.els_i:
		mov eax,[p0]
		add eax,offs_vert_zp
		cmp dword[edx+GLContext.current_shade_model],GL_SMOOTH
		jne .els
			;else if (context.current_shade_model == GL_SMOOTH)
			stdcall ZB_fillTriangleSmooth, dword[edx+GLContext.zb],eax,ebx,ecx
			jmp .end_f
align 4
		.els:
			stdcall ZB_fillTriangleFlat, dword[edx+GLContext.zb],eax,ebx,ecx
	.end_f:
popad
	ret
endp

; Render a clipped triangle in line mode

align 16
proc gl_draw_triangle_line uses eax ebx ecx edx, context:dword, p0:dword,p1:dword,p2:dword
	mov edx,[context]
	cmp dword[edx+GLContext.depth_test],0
	je .els
		lea ecx,[ZB_line_z]
		jmp @f
align 4
	.els:
		lea ecx,[ZB_line]
	@@:

	;if (p0.edge_flag) ZB_line_z(context.zb,&p0.zp,&p1.zp)
	mov eax,[p0]
	cmp dword[eax+offs_vert_edge_flag],0
	je @f
		mov ebx,eax
		add ebx,offs_vert_zp
		mov eax,[p1]
		add eax,offs_vert_zp
		stdcall ecx,dword[edx+GLContext.zb],ebx,eax
	@@:
	;if (p1.edge_flag) ZB_line_z(context.zb,&p1.zp,&p2.zp)
	mov eax,[p1]
	cmp dword[eax+offs_vert_edge_flag],0
	je @f
		mov ebx,eax
		add ebx,offs_vert_zp
		mov eax,[p2]
		add eax,offs_vert_zp
		stdcall ecx,dword[edx+GLContext.zb],ebx,eax
	@@:
	;if (p2.edge_flag) ZB_line_z(context.zb,&p2.zp,&p0.zp);
	mov eax,[p2]
	cmp dword[eax+offs_vert_edge_flag],0
	je @f
		mov ebx,eax
		add ebx,offs_vert_zp
		mov eax,[p0]
		add eax,offs_vert_zp
		stdcall ecx,dword[edx+GLContext.zb],ebx,eax
	@@:

	ret
endp

; Render a clipped triangle in point mode
align 16
proc gl_draw_triangle_point uses eax ebx edx, context:dword, p0:dword,p1:dword,p2:dword
	mov edx,[context]
	mov eax,[p0]
	cmp dword[eax+offs_vert_edge_flag],0
	je @f
		mov ebx,eax
		add ebx,offs_vert_zp
		stdcall ZB_plot,dword[edx+GLContext.zb],ebx
	@@:
	mov eax,[p1]
	cmp dword[eax+offs_vert_edge_flag],0
	je @f
		mov ebx,eax
		add ebx,offs_vert_zp
		stdcall ZB_plot,dword[edx+GLContext.zb],ebx
	@@:
	mov eax,[p2]
	cmp dword[eax+offs_vert_edge_flag],0
	je @f
		mov ebx,eax
		add ebx,offs_vert_zp
		stdcall ZB_plot,dword[edx+GLContext.zb],ebx
	@@:
	ret
endp




