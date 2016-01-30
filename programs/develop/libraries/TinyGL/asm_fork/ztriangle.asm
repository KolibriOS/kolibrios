
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
		mov al,[colorR]
		mov ah,[colorG]
		mov word[edi+3*_a],ax
		mov al,[colorB]
		mov byte[edi+3*_a +2],al
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
		mov ebx,[or1]
		mov eax,[og1]
		mov al,bh
		mov word[edi+3*_a],ax
		mov eax,[ob1]
		mov byte[edi+3*_a +2],ah
end if
;if TGL_FEATURE_RENDER_BITS eq 32
;      pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);
;end if
	.end_0:
	mov eax,[dzdx]
	add [z],eax
	mov eax,[dgdx]
	add [og1],eax
	mov eax,[drdx]
	add [or1],eax
	mov eax,[dbdx]
	add [ob1],eax
}

align 4
proc ZB_fillTriangleSmooth, zb:dword, p0:dword, p1:dword, p2:dword
locals
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

macro DRAW_LINE
{
if TGL_FEATURE_RENDER_BITS eq 24
	mov eax,[x2]
	sar eax,16
	mov edi,[x1]
	sub eax,edi
	mov [n],eax ;n = (x2 >> 16) - x1
fld1
	fild dword[z1]
	fst dword[f_z] ;fz = (float)z1
	;fld1
	fdivp
	fstp dword[zinv] ;zinv = 1.0 / fz
	imul edi,PSZB
	add edi,[pp1] ;pp = (pp1 + x1 * PSZB)
	mov eax,[x1]
	shl eax,1
	add eax,[pz1]
	mov [pz],eax ;pz = pz1 + x1
	mov eax,[z1]
	mov [z],eax ;z = z1
	mov eax,[sz1]
	mov [s_z],eax ;sz = sz1
	mov eax,[tz1]
	mov [t_z],eax ;tz = tz1
align 4
	.cycle_2: ;while (n>=(NB_INTERP-1))
	cmp dword[n],NB_INTERP-1
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
		fld dword[f_z]
		fadd dword[fndzdx]
		fst dword[f_z] ;fz += fndzdx
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
		sub dword[n],NB_INTERP ;n -= NB_INTERP
		fld dword[ndszdx]
		fadd dword[s_z]
		fstp dword[s_z] ;sz += ndszdx
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
	.cycle_3: ;while (n>=0)
	cmp dword[n],0
	jl .cycle_3_end
		PUT_PIXEL 0
		add dword[pz],2 ;pz += 1
		add edi,PSZB ;pp += PSZB
		dec dword[n]
		jmp .cycle_3
	.cycle_3_end:
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
	zinv dd ? ;float
	f_z dd ? ;float - переменная отвечающая за геометрию текстуры
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
		fistp dword[esp-4] ;s = (int) (s_z / (float) z)
		fld dword[t_z]
		fdiv st0,st1
		fistp dword[esp-8] ;t = (int) (t_z / (float) z)
		mov eax,dword[esp-8]
		and eax,0x3FC00000
		or eax,dword[esp-4]
		shr eax,12 ;14
		add eax,[texture]
		mov eax,[eax]
		stosd ;pp[_a] = texture[((t & 0x3FC00000) | s) >> 14]
		sub edi,4
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
