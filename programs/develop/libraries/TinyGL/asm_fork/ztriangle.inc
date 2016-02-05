;
; We draw a triangle with various interpolations
;
; Порядок преобразований цветов вершин:
; ZBufferPoint.r -> r1 (+drdl_min or +drdl_max) -> or1 (+drdx) -> [pixel buffer]
; ZBufferPoint.g -> g1 (+dgdl_min or +dgdl_max) -> og1 (+dgdx) -> [pixel buffer]
; ZBufferPoint.b -> b1 (+dbdl_min or +dbdl_max) -> ob1 (+dbdx) -> [pixel buffer]
;
; В некоторых случаях значения цвета (во 2-м байте переменных: or1,og1,ob1)
; может становиться < 0 или > 255, тогда появляються пиксели не правильного
; цвета. Скорее всего это связано с ошибками округления дробных чисел,
; при вычислении коэфициентов для плавного перехода цвета.
;
; Для лечения этой проблемы в версии на C++ специально ограничиваються
; минимальные и максимальные значения цвета точек (например от 3 до 252).
; Потому цвета граней могут немного отличаться от указанных в программе.
;
; В даной версии алгоритм немного другой. В наиболее вероятных местах появления
; пикселей не правильного цвета (обычно начало и конец линии) иправляеться
; цвет испорченных пикселей. Цвет получаеться наиболее близким к указанному
; пользователем.

	pr1 dd ? ;ZBufferPoint*
	pr2 dd ? ;ZBufferPoint*
	l1 dd ? ;ZBufferPoint*
	l2 dd ? ;ZBufferPoint*
	fdx1 dd ? ;float
	fdx2 dd ? ;float
	fdy1 dd ? ;float
	fdy2 dd ? ;float
	fz dd ? ;float - переменная отвечающая за геометрию фигуры
	d1 dd ? ;float
	d2 dd ? ;float
	pz1 dd ? ;unsigned short*
	pp1 dd ? ;PIXEL*
	part dd ?
	update_left dd ?
	update_right dd ?

	nb_lines dd ? ;число горизонтальных линий в половине треугольника
	dx1 dd ?
	dy1 dd ?
	;dx2 dd ?
	dy2 dd ?

	error dd ? ;int
	derror dd ? ;int
	x1 dd ? ;int
	dxdy_min dd ? ;int
	dxdy_max dd ? ;int
	; warning: x2 is multiplied by 2^16
	x2 dd ? ;int
	dx2dy2 dd ? ;int

	z dd ? ;uint
	n dd ? ;int - длинна горизонтальной линии в пикселях

if INTERP_Z eq 1
	z1 dd ? ;int
	dzdx dd ? ;int
	dzdy dd ? ;int
	dzdl_min dd ? ;int
	dzdl_max dd ? ;int
end if
if INTERP_RGB eq 1
	r1 dd ? ;int
	drdx dd ?
	drdy dd ?
	drdl_min dd ?
	drdl_max dd ?
	g1 dd ?
	dgdx dd ?
	dgdy dd ?
	dgdl_min dd ?
	dgdl_max dd ?
	b1 dd ?
	dbdx dd ?
	dbdy dd ?
	dbdl_min dd ?
	dbdl_max dd ?
	or1 dd ? ;uint
	og1 dd ? ;uint
	ob1 dd ? ;uint
end if
if INTERP_ST eq 1
	s1 dd ? ;int
	dsdy dd ? ;int
	dsdl_min dd ? ;int
	dsdl_max dd ? ;int
	t1 dd ? ;int
	dtdy dd ? ;int
	dtdl_min dd ? ;int
	dtdl_max dd ? ;int
end if
if INTERP_STZ eq 1
	sz1 dd ? ;float
	dszdx dd ? ;float
	dszdy dd ? ;float
	dszdl_min dd ? ;float
	dszdl_max dd ? ;float
	tz1 dd ? ;float
	dtzdx dd ? ;float
	dtzdy dd ? ;float
	dtzdl_min dd ? ;float
	dtzdl_max dd ? ;float
	s_z dd ? ;float
	t_z dd ? ;float
end if
if (INTERP_ST eq 1) | (DRAW_LINE_M eq 1)
	s dd ? ;uint
	t dd ? ;uint
	dsdx dd ? ;int
	dtdx dd ? ;int
end if
endl
pushad

	; we sort the vertex with increasing y
	mov ebx,[p0]
	mov ecx,[p1]
	mov edx,[p2]
	mov eax,[edx+offs_zbup_y]
	cmp [ecx+offs_zbup_y],eax ;(2-1)
	jle @f
		xchg edx,ecx
	@@:
	mov eax,[ecx+offs_zbup_y]
	cmp [ebx+offs_zbup_y],eax ;(1-0)
	jle @f
		xchg ecx,ebx
	@@:
	mov eax,[edx+offs_zbup_y]
	cmp [ecx+offs_zbup_y],eax ;(2-1)
	jle @f
		xchg edx,ecx
	@@:
	mov [p0],ebx
	mov [p1],ecx
	mov [p2],edx

	; we compute dXdx and dXdy for all interpolated values
	mov eax,[ecx+offs_zbup_x]
	sub eax,[ebx+offs_zbup_x]
	mov [fdx1],eax ;p1.x - p0.x
	mov eax,[ecx+offs_zbup_y]
	sub eax,[ebx+offs_zbup_y]
	mov [fdy1],eax ;p1.y - p0.y

	mov eax,[edx+offs_zbup_x]
	sub eax,[ebx+offs_zbup_x]
	mov [fdx2],eax ;p2.x - p0.x
	mov eax,[edx+offs_zbup_y]
	sub eax,[ebx+offs_zbup_y]
	mov [fdy2],eax ;p2.y - p0.y

	fild dword[fdx1]
	fimul dword[fdy2]
	fild dword[fdx2]
	fimul dword[fdy1]
	fsubp ;st0 = st1-st0
	fst dword[fz] ;fz = fdx1 * fdy2 - fdx2 * fdy1
	fldz
	fcompp ;if (fz == 0) return
	fstsw ax
	sahf
	je .end_f

	fld1
	fdiv dword[fz] ;fz = 1.0 / fz
	fst dword[fz]  ;st0 = fz

	fild dword[fdx1]
	fmul st0,st1
	fstp dword[fdx1] ;fdx1 *= fz
	fild dword[fdy1]
	fmul st0,st1
	fstp dword[fdy1] ;fdy1 *= fz
	fild dword[fdx2]
	fmul st0,st1
	fstp dword[fdx2] ;fdx2 *= fz
	fild dword[fdy2]
	fmulp
	fstp dword[fdy2] ;fdy2 *= fz

if INTERP_Z eq 1
	mov eax,[ecx+offs_zbup_z]
	sub eax,[ebx+offs_zbup_z]
	mov [d1],eax
	mov eax,[edx+offs_zbup_z]
	sub eax,[ebx+offs_zbup_z]
	mov [d2],eax
	fild dword[d1] ;d1 = p1.z - p0.z
	fild dword[d2] ;d2 = p2.z - p0.z

	;dzdx = (int) (fdy2*d1 - fdy1*d2)
	;dzdy = (int) (fdx1*d2 - fdx2*d1)
	calc_d1d2 fi, dzdx, dzdy
end if

if INTERP_RGB eq 1
	mov eax,[ecx+offs_zbup_r]
	sub eax,[ebx+offs_zbup_r]
	mov [d1],eax
	mov eax,[edx+offs_zbup_r]
	sub eax,[ebx+offs_zbup_r]
	mov [d2],eax
	fild dword[d1] ;d1 = p1.r - p0.r
	fild dword[d2] ;d2 = p2.r - p0.r

	;drdx = (int) (fdy2*d1 - fdy1*d2)
	;drdy = (int) (fdx1*d2 - fdx2*d1)
	calc_d1d2 fi, drdx, drdy

	mov eax,[ecx+offs_zbup_g]
	sub eax,[ebx+offs_zbup_g]
	mov [d1],eax
	mov eax,[edx+offs_zbup_g]
	sub eax,[ebx+offs_zbup_g]
	mov [d2],eax
	fild dword[d1] ;d1 = p1.g - p0.g
	fild dword[d2] ;d2 = p2.g - p0.g

	;dgdx = (int) (fdy2*d1 - fdy1*d2)
	;dgdy = (int) (fdx1*d2 - fdx2*d1)
	calc_d1d2 fi, dgdx, dgdy

	mov eax,[ecx+offs_zbup_b]
	sub eax,[ebx+offs_zbup_b]
	mov [d1],eax
	mov eax,[edx+offs_zbup_b]
	sub eax,[ebx+offs_zbup_b]
	mov [d2],eax
	fild dword[d1] ;d1 = p1.b - p0.b
	fild dword[d2] ;d2 = p2.b - p0.b

	;dbdx = (int) (fdy2*d1 - fdy1*d2)
	;dbdy = (int) (fdx1*d2 - fdx2*d1)
	calc_d1d2 fi, dbdx, dbdy
end if

if INTERP_ST eq 1
	mov eax,[ecx+offs_zbup_s]
	sub eax,[ebx+offs_zbup_s]
	mov [d1],eax
	mov eax,[edx+offs_zbup_s]
	sub eax,[ebx+offs_zbup_s]
	mov [d2],eax
	fild dword[d1] ;d1 = p1.s - p0.s
	fild dword[d2] ;d2 = p2.s - p0.s

	;dsdx = (int) (fdy2*d1 - fdy1*d2)
	;dsdy = (int) (fdx1*d2 - fdx2*d1)
	calc_d1d2 fi, dsdx, dsdy

	mov eax,[ecx+offs_zbup_t]
	sub eax,[ebx+offs_zbup_t]
	mov [d1],eax
	mov eax,[edx+offs_zbup_t]
	sub eax,[ebx+offs_zbup_t]
	mov [d2],eax
	fild dword[d1] ;d1 = p1.t - p0.t
	fild dword[d2] ;d2 = p2.t - p0.t

	;dtdx = (int) (fdy2*d1 - fdy1*d2)
	;dtdy = (int) (fdx1*d2 - fdx2*d1)
	calc_d1d2 fi, dtdx, dtdy
end if

if INTERP_STZ eq 1
	fild dword[ebx+offs_zbup_z]
	fild dword[ebx+offs_zbup_s]
	fmul st0,st1
	fstp dword[ebx+offs_zbup_sz] ;p0.sz = (float) p0.s * p0.z
	fild dword[ebx+offs_zbup_t]
	fmulp
	fstp dword[ebx+offs_zbup_tz] ;p0.tz = (float) p0.t * p0.z

	fild dword[ecx+offs_zbup_z]
	fild dword[ecx+offs_zbup_s]
	fmul st0,st1
	fstp dword[ecx+offs_zbup_sz] ;p1.sz = (float) p1.s * p1.z
	fild dword[ecx+offs_zbup_t]
	fmulp
	fstp dword[ecx+offs_zbup_tz] ;p1.tz = (float) p1.t * p1.z

	fild dword[edx+offs_zbup_z]
	fild dword[edx+offs_zbup_s]
	fmul st0,st1
	fstp dword[edx+offs_zbup_sz] ;p2.sz = (float) p2.s * p2.z
	fild dword[edx+offs_zbup_t]
	fmulp
	fstp dword[edx+offs_zbup_tz] ;p2.tz = (float) p2.t * p2.z

	fld dword[ecx+offs_zbup_sz]
	fsub dword[ebx+offs_zbup_sz] ;d1 = p1.sz - p0.sz
	fld dword[edx+offs_zbup_sz]
	fsub dword[ebx+offs_zbup_sz] ;d2 = p2.sz - p0.sz

	;dszdx = (fdy2*d1 - fdy1*d2)
	;dszdy = (fdx1*d2 - fdx2*d1)
	calc_d1d2 f, dszdx, dszdy

	fld dword[ecx+offs_zbup_tz]
	fsub dword[ebx+offs_zbup_tz] ;d1 = p1.tz - p0.tz
	fld dword[edx+offs_zbup_tz]
	fsub dword[ebx+offs_zbup_tz] ;d2 = p2.tz - p0.tz

	;dtzdx = (fdy2*d1 - fdy1*d2)
	;dtzdy = (fdx1*d2 - fdx2*d1)
	calc_d1d2 f, dtzdx, dtzdy
end if

	; screen coordinates
	mov eax,[zb]
	mov edx,[eax+offs_zbuf_linesize]
	imul edx,[ebx+offs_zbup_y]
	add edx,[eax+offs_zbuf_pbuf]
	mov [pp1],edx ;pp1 = zb.pbuf + zb.linesize * p0.y
	mov edx,[eax+offs_zbuf_xsize]
	imul edx,[ebx+offs_zbup_y]
	shl edx,1
	add edx,[eax+offs_zbuf_zbuf]
	mov [pz1],edx ;pz1 = zb.zbuf + zb.xsize * p0.y

	DRAW_INIT

	mov dword[part],0
	.cycle_0:
		mov ebx,[p0]
		mov ecx,[p1]
		mov edx,[p2]
		cmp dword[part],0 ;if (part == 0)
		jne .els_0
			mov dword[update_left],1
			mov dword[update_right],1
			mov [l1],ebx
			mov [pr1],ebx
			fldz
			fld dword[fz]
			fcompp ;if (fz > 0)
			fstsw ax
			sahf
			jbe .els_1
				mov [l2],edx
				mov [pr2],ecx
				jmp .end_1
align 4
			.els_1:
				mov [l2],ecx
				mov [pr2],edx
			.end_1:
			mov eax,[ecx+offs_zbup_y]
			sub eax,[ebx+offs_zbup_y]
			mov [nb_lines],eax ;nb_lines = p1.y - p0.y
			jmp .end_0
align 4
		.els_0:
			; second part
			fldz
			fld dword[fz]
			fcompp ;if (fz > 0)
			fstsw ax
			sahf
			jbe .els_2
				mov dword[update_left],0
				mov dword[update_right],1
				mov [pr1],ecx
				mov [pr2],edx
				jmp .end_2
align 4
			.els_2:
				mov dword[update_left],1
				mov dword[update_right],0
				mov [l1],ecx
				mov [l2],edx
			.end_2:
			mov eax,[edx+offs_zbup_y]
			sub eax,[ecx+offs_zbup_y]
			inc eax
			mov [nb_lines],eax ;nb_lines = p2.y - p1.y + 1
		.end_0:

	; compute the values for the left edge
	cmp dword[update_left],0 ;if (update_left)
	je .end_upd_l
		mov ebx,[l1]
		mov ecx,[l2]
		mov edx,[ecx+offs_zbup_y]
		sub edx,[ebx+offs_zbup_y]
		mov [dy1],edx ;dy1 = l2.y - l1.y
		mov eax,[ecx+offs_zbup_x]
		sub eax,[ebx+offs_zbup_x]
		mov [dx1],eax ;dx1 = l2.x - l1.x
		cmp edx,0 ;if (dy1 > 0) 
		jle .els_3
			xor edx,edx
			cmp eax,0
			jl .otr_dx1
			shl eax,16
			div dword[dy1] ;eax = (dx1 << 16) / dy1
			jmp .end_3
align 4
			.otr_dx1:
			neg eax
			inc eax
			shl eax,16
			div dword[dy1] ;eax = (-dx1 << 16) / dy1
			neg eax
			inc eax
			jmp .end_3
align 4
		.els_3:
			xor eax,eax
		.end_3:
		mov edx,[ebx+offs_zbup_x]
		mov [x1],edx ;x1 = l1.x
		mov dword[error],0 ;error = 0
		mov dword[derror],eax
		and dword[derror],0xffff ;derror = eax & 0x0000ffff
		sar eax,16
		mov [dxdy_min],eax ;dxdy_min = eax >> 16
		inc eax
		mov [dxdy_max],eax

if INTERP_Z eq 1
	mov eax,[l1]
	mov eax,[eax+offs_zbup_z]
	mov [z1],eax ;z1 = l1.z
	mov eax,[dzdx]
	imul eax,[dxdy_min]
	add eax,[dzdy]
	mov [dzdl_min],eax ;dzdl_min = (dzdy +dzdx*dxdy_min)
	add eax,[dzdx]
	mov [dzdl_max],eax ;dzdl_max = dzdl_min +dzdx
end if
if INTERP_RGB eq 1
	mov ebx,[l1]
	mov eax,[ebx+offs_zbup_r]
	mov [r1],eax ;r1 = l1.r
	mov eax,[drdx]
	imul eax,[dxdy_min]
	add eax,[drdy]
	mov [drdl_min],eax ;drdl_min = (drdy +drdx*dxdy_min)
	add eax,[drdx]
	mov [drdl_max],eax ;drdl_max = drdl_min +drdx

	mov eax,[ebx+offs_zbup_g]
	mov [g1],eax ;g1 = l1.g
	mov eax,[dgdx]
	imul eax,[dxdy_min]
	add eax,[dgdy]
	mov [dgdl_min],eax ;dgdl_min = (dgdy +dgdx*dxdy_min)
	add eax,[dgdx]
	mov [dgdl_max],eax ;dgdl_max = dgdl_min +dgdx

	mov eax,[ebx+offs_zbup_b]
	mov [b1],eax ;b1 = l1.b
	mov eax,[dbdx]
	imul eax,[dxdy_min]
	add eax,[dbdy]
	mov [dbdl_min],eax ;dbdl_min = (dbdy +dbdx*dxdy_min)
	add eax,[dbdx]
	mov [dbdl_max],eax ;dbdl_max = dbdl_min +dbdx
end if
if INTERP_ST eq 1
	mov ebx,[l1]
	mov eax,[ebx+offs_zbup_s]
	mov [s1],eax ;s1 = l1.s
	mov eax,[dsdx]
	imul eax,[dxdy_min]
	add eax,[dsdy]
	mov [dsdl_min],eax ;dsdl_min = (dsdy +dsdx*dxdy_min)
	add eax,[dsdx]
	mov [dsdl_max],eax ;dsdl_max = dsdl_min +dsdx

	mov eax,[ebx+offs_zbup_t]
	mov [t1],eax ;t1 = l1.t
	mov eax,[dtdx]
	imul eax,[dxdy_min]
	add eax,[dtdy]
	mov [dtdl_min],eax ;dtdl_min = (dtdy +dtdx*dxdy_min)
	add eax,[dtdx]
	mov [dtdl_max],eax ;dtdl_max = dtdl_min +dtdx
end if
if INTERP_STZ eq 1
	mov ebx,[l1]
	mov eax,[ebx+offs_zbup_sz]
	mov [sz1],eax ;sz1 = l1.sz - преобразований нет, потому без сопроцессора
	fild dword[dxdy_min]
	fmul dword[dszdx]
	fadd dword[dszdy]
	fst dword[dszdl_min] ;dszdl_min = (dszdy +dszdx*dxdy_min)
	fadd dword[dszdx]
	fstp dword[dszdl_max] ;dszdl_max = dszdl_min +dszdx

	mov eax,[ebx+offs_zbup_tz]
	mov [tz1],eax ;tz1 = l1.tz - преобразований нет, потому без сопроцессора
	fild dword[dxdy_min]
	fmul dword[dtzdx]
	fadd dword[dtzdy]
	fst dword[dtzdl_min] ;dtzdl_min = (dtzdy +dtzdx*dxdy_min)
	fadd dword[dtzdx]
	fstp dword[dtzdl_max] ;dtzdl_max = dtzdl_min +dtzdx
end if
	.end_upd_l:

	; compute values for the right edge

	cmp dword[update_right],0 ;if(update_right)
	je .end_upd_r
		mov ebx,[pr1]
		mov ecx,[pr2]
		mov edx,[ebx+offs_zbup_x]
		mov eax,[ecx+offs_zbup_x]
		sub eax,edx
		;mov [dx2],eax ;dx2 = pr2.x - pr1.x
		shl edx,16
		mov [x2],edx ; x2 = pr1.x << 16
		mov edx,[ecx+offs_zbup_y]
		sub edx,[ebx+offs_zbup_y]
		mov [dy2],edx ;dy2 = pr2.y - pr1.y
		cmp edx,0 ;if (dy2 > 0) 
		jle .els_4
			xor edx,edx
			cmp eax,0
			jl .otr_dx2
			shl eax,16
			div dword[dy2] ;eax = (dx2 << 16) / dy2
			jmp .end_4
align 4
			.otr_dx2:
			neg eax
			inc eax ;dx2 *= -1
			shl eax,16
			div dword[dy2] ;eax = (-dx2 << 16) / dy2
			neg eax
			inc eax
			jmp .end_4
align 4
		.els_4:
			xor eax,eax
		.end_4:
		mov [dx2dy2],eax
	.end_upd_r:

	; we draw all the scan line of the part
if DEBUG ;[nb_lines]
push ecx edi
	mov eax,[nb_lines]
	mov ecx,80
	lea edi,[buf_param]
	stdcall convert_int_to_str,ecx

	stdcall str_n_cat,edi,txt_nl,2
	stdcall dbg_print,f_fill_tr_nl,buf_param
pop edi ecx
end if

	.beg_w_lin:
	cmp dword[nb_lines],0 ;while (nb_lines>0)
	jle .end_w_lin
		dec dword[nb_lines]
if DRAW_LINE_M eq 1
		DRAW_LINE
else
		; generic draw line
		mov eax,[x2]
		sar eax,16
		mov edi,[x1]
		sub eax,edi
		mov [n],eax ;n = (x2 >> 16) - x1
		imul edi,PSZB
		add edi,[pp1] ;pp = pp1 + x1 * PSZB

if INTERP_Z eq 1
		mov esi,[x1]
		shl esi,1
		add esi,[pz1]
		mov eax,[z1]
		mov [z],eax
end if
if INTERP_RGB eq 1
		mov eax,[r1]
		bt eax,31 ; коректирование испорченных пикселей (в начале линии)
		jnc @f
			xor eax,eax
		@@:
		bt eax,16
		jnc @f
			mov eax,0xff00
		@@:
		mov [or1],eax
		mov eax,[g1]
		bt eax,31
		jnc @f
			xor eax,eax
		@@:
		bt eax,16
		jnc @f
			mov eax,0xff00
		@@:
		mov [og1],eax
		mov eax,[b1]
		bt eax,31
		jnc @f
			xor eax,eax
		@@:
		bt eax,16
		jnc @f
			mov eax,0xff00
		@@:
		mov [ob1],eax
end if
if INTERP_ST eq 1
		mov eax,[s1]
		mov [s],eax
		mov eax,[t1]
		mov [t],eax
end if
if INTERP_STZ eq 1
		mov eax,[sz1]
		mov [s_z],eax
		mov eax,[tz1]
		mov [t_z],eax
end if

align 4
		.cycle_1: ;while (n>=3)
if INTERP_RGB eq 1
		cmp dword[n],5
		jl .cycle_2
else
		cmp dword[n],3
		jl .cycle_2
end if
			PUT_PIXEL 0
			PUT_PIXEL 1
			PUT_PIXEL 2
			PUT_PIXEL 3
if INTERP_Z eq 1
			add esi,8 ;=4*sizeof(uint)
end if
			add edi,4*PSZB
			sub dword[n],4
		jmp .cycle_1
align 4
		.cycle_2: ;while (n>=0)
		cmp dword[n],0
		jl .cycle_2_end
if INTERP_RGB eq 1
		; коректирование испорченных пикселей (в конце линии)
		bt dword[or1],31
		jnc @f
			mov dword[or1],0
			jmp .end_r
align 4
		@@:
		bt dword[or1],16
		jnc .end_r
			mov dword[or1],0xff00
		.end_r:
		bt dword[og1],31
		jnc @f
			mov dword[og1],0
			jmp .end_g
align 4
		@@:
		bt dword[og1],16
		jnc .end_g
			mov dword[og1],0xff00
		.end_g:
		bt dword[ob1],31
		jnc @f
			mov dword[ob1],0
			jmp .end_b
align 4
		@@:
		bt dword[ob1],16
		jnc .end_b
			mov dword[ob1],0xff00
		.end_b:
end if
			PUT_PIXEL 0
if INTERP_Z eq 1
			add esi,2 ;=sizeof(uint)
end if
			add edi,PSZB
			dec dword[n]
		jmp .cycle_2
align 4
		.cycle_2_end:
end if ;проверка от макроса DRAW_LINE

	; left edge
	mov eax,[derror]
	add [error],eax
	cmp dword[error],0 ;if (error > 0)
	jle .els_er
		sub dword[error],0x10000
		mov eax,[dxdy_max]
		add [x1],eax
if INTERP_Z eq 1
		mov eax,[dzdl_max]
		add [z1],eax
end if      
if INTERP_RGB eq 1
		mov eax,[drdl_max]
		add [r1],eax
		mov eax,[dgdl_max]
		add [g1],eax
		mov eax,[dbdl_max]
		add [b1],eax
end if
if INTERP_ST eq 1
		mov eax,[dsdl_max]
		add [s1],eax
		mov eax,[dtdl_max]
		add [t1],eax
end if
if INTERP_STZ eq 1
		fld dword[dszdl_max]
		fadd dword[sz1]
		fstp dword[sz1]
		fld dword[dtzdl_max]
		fadd dword[tz1]
		fstp dword[tz1]
end if
		jmp .end_er
align 4
	.els_er:
		mov eax,[dxdy_min]
		add [x1],eax
if INTERP_Z eq 1
		mov eax,[dzdl_min]
		add [z1],eax
end if
if INTERP_RGB eq 1
		mov eax,[drdl_min]
		add [r1],eax
		mov eax,[dgdl_min]
		add [g1],eax
		mov eax,[dbdl_min]
		add [b1],eax
end if
if INTERP_ST eq 1
		mov eax,[dsdl_min]
		add [s1],eax
		mov eax,[dtdl_min]
		add [t1],eax
end if
if INTERP_STZ eq 1
		fld dword[dszdl_min]
		fadd dword[sz1]
		fstp dword[sz1]
		fld dword[dtzdl_min]
		fadd dword[tz1]
		fstp dword[tz1]
end if
	.end_er:
			; right edge
			mov eax,[dx2dy2]
			add [x2],eax

			; screen coordinates
			mov ebx,[zb]
			mov eax,[ebx+offs_zbuf_linesize]
			add [pp1],eax
			mov eax,[ebx+offs_zbuf_xsize]
			shl eax,1
			add [pz1],eax
			jmp .beg_w_lin
align 4
		.end_w_lin:
		inc dword[part]
		cmp dword[part],2
		jl .cycle_0
	.end_f:
popad
	ret
endp

restore INTERP_Z
restore INTERP_RGB
restore INTERP_ST
restore INTERP_STZ
restore DRAW_LINE_M

purge DRAW_INIT
purge DRAW_LINE
purge PUT_PIXEL
