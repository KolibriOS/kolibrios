;
; Макрос DRAW_LINE имеет параметр code, от которого зависит
; будет ли включен код или объявлены переменные.
; В версии на C++ параметра code нет, потому что там переменные
; можно ставить в любом месте функции, в asm версии такое не проходит.
;

INTERP_Z equ 1

macro DRAW_INIT
{
if TGL_FEATURE_RENDER_BITS eq 24
	mov ecx,[p2]
	mov eax,[ecx+offs_zbup_r]
	mov [colorR],ah ;colorR=p2.r>>8
	mov eax,[ecx+offs_zbup_g]
	mov [colorG],ah ;colorG=p2.g>>8
	mov eax,[ecx+offs_zbup_b]
	mov [colorB],ah ;colorB=p2.b>>8
;else
;  color=RGB_TO_PIXEL(p2->r,p2->g,p2->b);
end if
}

macro PUT_PIXEL _a
{
local .end_0
	mov eax,[z]
	shr eax, ZB_POINT_Z_FRAC_BITS
	mov [zz],eax
	mov ebx,[pz]
	cmp ax,word[ebx+2*_a] ;if (zz >= pz[_a])
	jl .end_0
		;edi = pp
		mov word[ebx+2*_a],ax ;пишем в буфер глубины новое значение
if TGL_FEATURE_RENDER_BITS eq 24
		mov cl,[colorR]
		mov ch,[colorG]
		mov word[edi+3*_a],cx
		mov cl,[colorB]
		mov byte[edi+3*_a +2],cl
;else
;      pp[_a]=color;
end if
	.end_0:
	mov eax,[dzdx]
	add [z],eax
}

align 4
proc ZB_fillTriangleFlat, zb:dword, p0:dword, p1:dword, p2:dword
locals
if TGL_FEATURE_RENDER_BITS eq 24
	colorR db ?
	colorG db ?
	colorB db ? ;unsigned char
else
	color dd ? ;int
end if
include 'ztriangle.inc'

;
; Smooth filled triangle.
; The code below is very tricky :)
;

INTERP_Z equ 1
INTERP_RGB equ 1

macro DRAW_INIT
{
if TGL_FEATURE_RENDER_BITS eq 16
;  _drgbdx=((drdx / (1<<6)) << 22) & 0xFFC00000;
;  _drgbdx|=(dgdx / (1<<5)) & 0x000007FF;
;  _drgbdx|=((dbdx / (1<<7)) << 12) & 0x001FF000;
end if
}

macro PUT_PIXEL _a
{
local .end_0
	mov eax,[z]
	shr eax,ZB_POINT_Z_FRAC_BITS
	mov [zz],eax
	mov ebx,[pz]
	cmp ax,word[ebx+2*_a] ;if (zz >= pz[_a])
	jl .end_0
		;edi = pp
		mov word[ebx+2*_a],ax ;пишем в буфер глубины новое значение
if TGL_FEATURE_RENDER_BITS eq 24
		mov ecx,[og1]
		mov eax,[or1]
		mov cl,ah
		mov word[edi+3*_a],cx
		mov eax,[ob1]
		mov byte[edi+3*_a +2],ah
end if
if TGL_FEATURE_RENDER_BITS eq 16
;      tmp=rgb & 0xF81F07E0;
;      pp[_a]=tmp | (tmp >> 16);
;else
;      pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);
end if
	.end_0:
	mov eax,[dzdx]
	add [z],eax
if TGL_FEATURE_RENDER_BITS eq 16
;    rgb=(rgb+drgbdx) & ( ~ 0x00200800);
end if
if TGL_FEATURE_RENDER_BITS <> 16
	mov eax,[dgdx]
	add [og1],eax
	mov eax,[drdx]
	add [or1],eax
	mov eax,[dbdx]
	add [ob1],eax
end if
}

macro DRAW_LINE code
{
local .cycle_0
local .cycle_1
if TGL_FEATURE_RENDER_BITS eq 16
if code eq 0
	tmp dd ? ;uint
	rgb dd ? ;uint
	drgbdx dd ? ;uint
end if
if code eq 1
	mov eax,[x2]
	sar eax,16
	sub eax,[x1]
	mov [n],eax ;n = (x2 >> 16) - x1
	mov edi,[pp1]
	add edi,[x1] ;pp = pp1 + x1
	mov eax,[pz1]
	add eax,[x1]
	mov [pz],eax ;pz = pz1 + x1
	mov eax,[z1]
	mov [z],eax ;z = z1
	mov eax,[r1]
	shl eax,16
	and eax,0xFFC00000
	mov [rgb],eax ;rgb = (r1 << 16) & 0xFFC00000
	mov eax,[g1]
	shr eax,5
	and eax,0x000007FF
	or [rgb],eax ;rgb |= (g1 >> 5) & 0x000007FF
	mov eax,[b1]
	shl eax,5
	and eax,0x001FF000
	or [rgb],eax ;rgb |= (b1 << 5) & 0x001FF000
	mov eax,[_drgbdx]
	mov [drgbdx],eax ;drgbdx = _drgbdx
align 4
	.cycle_0: ;while (n>=3)
	cmp dword[n],3
	jl .cycle_1
		PUT_PIXEL 0
		PUT_PIXEL 1
		PUT_PIXEL 2
		PUT_PIXEL 3
		add dword[pz],8
		add edi,4*3
		sub [n],4
	jmp .cycle_0
	.cycle_1: ;while (n>=0)
	cmp dword[n],0
	jl .cycle_1_end
		PUT_PIXEL 0
		add dword[pz],2
		add edi,3
		dec dword[n]
		jmp .cycle_1
	.cycle_1_end:
end if
end if
}

align 4
proc ZB_fillTriangleSmooth, zb:dword, p0:dword, p1:dword, p2:dword
locals
if TGL_FEATURE_RENDER_BITS eq 16
	_drgbdx dd ? ;int
end if
include 'ztriangle.inc'

align 4
proc ZB_setTexture uses eax ebx, zb:dword, texture:dword
	mov eax,[zb]
	mov ebx,[texture]
	mov dword[eax+offs_zbuf_current_texture],ebx
	ret
endp

INTERP_Z equ 1
INTERP_ST equ 1

macro DRAW_INIT
{
	mov eax,[zb]
	mov eax,[eax+offs_zbuf_current_texture]
	mov [texture],eax
}

macro PUT_PIXEL _a
{
local .end_0
	mov eax,[z]
	shr eax,ZB_POINT_Z_FRAC_BITS
	mov [zz],eax
	mov ebx,[pz]
	cmp ax,word[ebx+2*_a] ;if (zz >= pz[_a])
	jl .end_0
		;edi = pp
		mov word[ebx+2*_a],ax ;пишем в буфер глубины новое значение
if TGL_FEATURE_RENDER_BITS eq 24
		mov ebx,[t]
		and ebx,0x3fc00000
		or ebx,[s]
		shr ebx,14
		imul ebx,3
		add ebx,[texture] ;ptr = texture + (((t & 0x3fc00000) | s) >> 14) * 3
		mov ax,word[ebx]
		mov word[edi+3*_a],ax ;pp[3 * _a]= ptr[0,1]
		mov al,byte[ebx+2]
		mov byte[edi+3*_a +2],al ;pp[3 * _a + 2]= ptr[2]
else
;       pp[_a]=texture[((t & 0x3FC00000) | s) >> 14];
end if
	.end_0:
	mov eax,[dzdx]
	add [z],eax
	mov eax,[dsdx]
	add [s],eax
	mov eax,[dtdx]
	add [t],eax
}

align 4
proc ZB_fillTriangleMapping, zb:dword, p0:dword, p1:dword, p2:dword
locals
	texture dd ? ;PIXEL*
include 'ztriangle.inc'

;
; Texture mapping with perspective correction.
; We use the gradient method to make less divisions.
; TODO: pipeline the division
;
if 1

INTERP_Z equ 1
INTERP_STZ equ 1

NB_INTERP equ 8

macro DRAW_INIT
{
	mov eax,[zb]
	mov eax,[eax+offs_zbuf_current_texture]
	mov [texture],eax
	mov dword[esp-4],NB_INTERP
	fild dword[esp-4]
	fild dword[dzdx]
	fst dword[fdzdx] ;fdzdx = (float)dzdx
	fmul st0,st1
	fstp dword[fndzdx] ;fndzdx = NB_INTERP * fdzdx
	fld dword[fdzdx]
	fmul st0,st1
	fstp dword[ndszdx] ;ndszdx = NB_INTERP * dszdx
	fmul dword[dtzdx]
	fstp dword[ndtzdx] ;ndtzdx = NB_INTERP * dtzdx
}

macro PUT_PIXEL _a
{
local .end_0
	mov eax,[z]
	shr eax,ZB_POINT_Z_FRAC_BITS
	mov [zz],eax
	mov ebx,[pz]
	cmp ax,word[ebx+2*_a] ;if (zz >= pz[_a])
	jl .end_0
		;edi = pp
		mov word[ebx+2*_a],ax ;пишем в буфер глубины новое значение
if TGL_FEATURE_RENDER_BITS eq 24
		mov ebx,[t]
		and ebx,0x3fc00000
		mov eax,[s]
		and eax,0x003fc000
		or ebx,eax
		shr ebx,14
		imul ebx,3
		add ebx,[texture] ;ptr = texture + (((t & 0x3fc00000) | (s & 0x003FC000)) >> 14) * 3
		mov ax,word[ebx]
		mov word[edi+3*_a],ax ;pp[3 * _a]= ptr[0,1]
		mov al,byte[ebx+2]
		mov byte[edi+3*_a +2],al ;pp[3 * _a + 2]= ptr[2]
else
;       pp[_a]=*(PIXEL *)((char *)texture+
;           (((t & 0x3FC00000) | (s & 0x003FC000)) >> (17 - PSZSH)));
end if
	.end_0:
	mov eax,[dzdx]
	add [z],eax
	mov eax,[dsdx]
	add [s],eax
	mov eax,[dtdx]
	add [t],eax
}

DRAW_LINE_M equ 1

macro DRAW_LINE code
{
if TGL_FEATURE_RENDER_BITS eq 24
if code eq 0
	s dd ? ;uint
	t dd ? ;uint
	n1 dd ? ;int - длинна горизонтальной линии в пикселях
	dsdx dd ? ;int
	dtdx dd ? ;int
	fz dd ? ;float
	zinv dd ? ;float
end if
if code eq 1
	mov eax,[x2]
	sar eax,16
	sub eax,[x1]
	mov [n1],eax ;n1 = (x2 >> 16) - x1
fld1
	fild dword[z1]
	fst dword[fz] ;fz = (float)z1
	;fld1
	fdivp
	fstp dword[zinv] ;zinv = 1.0 / fz
	mov edi,[x1]
	imul edi,PSZB
	add edi,[pp1] ;pp = (pp1 + x1 * PSZB)
	mov eax,[pz1]
	add eax,[x1]
	mov [pz],eax ;pz = pz1 + x1
	mov eax,[z1]
	mov [z],eax ;z = z1
	mov eax,[sz1]
	mov [s_z],eax ;sz = sz1
	mov eax,[tz1]
	mov [t_z],eax ;tz = tz1
align 4
	.cycle_2: ;while (n1>=(NB_INTERP-1))
	cmp dword[n1],NB_INTERP-1
	jl .cycle_2_end
		fld dword[zinv]
		fld st0
		fmul dword[s_z] ;ss = (sz * zinv)
		fist dword[s] ;s = (int) ss
		fmul dword[fdzdx]
		fchs
		fadd dword[dszdx]
		fmul dword[zinv]
		fistp dword[dsdx] ;dsdx = (int)( (dszdx - ss*fdzdx)*zinv )
		fmul dword[t_z] ;tt = (tz * zinv)
		fist dword[t] ;t = (int) tt
		fmul dword[fdzdx]
		fchs
		fadd dword[dtzdx]
		fmul dword[zinv]
		fistp dword[dtdx] ;dtdx = (int)( (dtzdx - tt*fdzdx)*zinv )
fld1
		fld dword[fz]
		fadd dword[fndzdx]
		fst dword[fz] ;fz += fndzdx
		;fld1
		fdivp
		fstp dword[zinv] ;zinv = 1.0 / fz
		PUT_PIXEL 0
		PUT_PIXEL 1
		PUT_PIXEL 2
		PUT_PIXEL 3
		PUT_PIXEL 4
		PUT_PIXEL 5
		PUT_PIXEL 6
		PUT_PIXEL 7
		add dword[pz],2*NB_INTERP ;pz += NB_INTERP
		add edi,NB_INTERP*PSZB ;pp += NB_INTERP * PSZB
		sub dword[n1],NB_INTERP ;n1 -= NB_INTERP
		fld dword[ndszdx]
		fadd dword[s_z]
		fstp dword[s_z] ;s_z += ndszdx
		fld dword[ndtzdx]
		fadd dword[t_z]
		fstp dword[t_z] ;tz += ndtzdx
		jmp .cycle_2
	.cycle_2_end:
	fld dword[zinv]
	fld st0
	fmul dword[s_z] ;ss = (sz * zinv)
	fist dword[s] ;s = (int) ss
	fmul dword[fdzdx]
	fchs
	fadd dword[dszdx]
	fmul dword[zinv]
	fistp dword[dsdx] ;dsdx = (int)( (dszdx - ss*fdzdx)*zinv )
	fmul dword[t_z] ;tt = (tz * zinv)
	fist dword[t] ;t = (int) tt
	fmul dword[fdzdx]
	fchs
	fadd dword[dtzdx]
	fmul dword[zinv]
	fistp dword[dtdx] ;dtdx = (int)( (dtzdx - tt*fdzdx)*zinv )
align 4
	.cycle_3: ;while (n1>=0)
	cmp dword[n1],0
	jl .cycle_3_end
		PUT_PIXEL 0
		add dword[pz],2 ;pz += 1
		add edi,PSZB ;pp += PSZB
		dec dword[n1]
		jmp .cycle_3
	.cycle_3_end:
end if
end if
}

align 4
proc ZB_fillTriangleMappingPerspective, zb:dword, p0:dword, p1:dword, p2:dword
locals
	texture dd ? ;PIXEL *
	fdzdx dd ? ;float
	fndzdx dd ? ;float
	ndszdx dd ? ;float
	ndtzdx dd ? ;float
include 'ztriangle.inc'

end if

if 0

; slow but exact version (only there for reference, incorrect for 24
; bits)

INTERP_Z equ 1
INTERP_STZ equ 1

macro DRAW_INIT
{
	mov eax,[zb]
	mov eax,[eax+offs_zbuf_current_texture]
	mov [texture],eax
}

macro PUT_PIXEL _a
{
local .end_0
;   int s,t;
	mov eax,[z]
	shr eax,ZB_POINT_Z_FRAC_BITS
	mov [zz],eax
	mov ebx,[pz]
	cmp ax,word[ebx+2*_a] ;if (zz >= pz[_a])
	jl .end_0
		;edi = pp
		mov word[ebx+2*_a],ax ;пишем в буфер глубины новое значение
		fild dword[z]
		fld dword[s_z]
		fdiv st0,st1
		;fistp dword[...s...] ;s = (int) (s_z / (float) z)
;       t= (int) (t_z / (float) z);
;       pp[_a]=texture[((t & 0x3FC00000) | s) >> 14];
	.end_0:
	mov eax,[dzdx]
	add [z],eax
	fld dword[dszdx]
	fadd dword[s_z]
	fstp dword[s_z]
	fld dword,[dtzdx]
	fadd dword[t_z]
	fstp dword[t_z]
}

align 4
proc ZB_fillTriangleMappingPerspective, zb:dword, p0:dword, p1:dword, p2:dword
locals
	texture dd ? ;PIXEL*
include 'ztriangle.inc'

end if
