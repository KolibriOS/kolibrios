
INTERP_Z equ 1

macro DRAW_INIT
{
if TGL_FEATURE_RENDER_BITS eq 24
	mov ecx,[p2]
	mov eax,[ecx+offs_zbup_r]
	shr eax,8
	mov [colorR],al ;colorR=p2.r>>8
	mov eax,[ecx+offs_zbup_g]
	shr eax,8
	mov [colorG],al ;colorG=p2.g>>8
	mov eax,[ecx+offs_zbup_b]
	shr eax,8
	mov [colorB],al ;colorB=p2.b>>8
else
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
if TGL_FEATURE_RENDER_BITS eq 24
		mov cl,[colorR]
		mov ch,[colorG]
		mov word[edi+3*_a],cx
		mov cl,[colorB]
		mov byte[edi+3*_a +2],cl
else
;      pp[_a]=color;
end if
		mov word[ebx+2*_a],ax ;пишем в буфер глубины новое значение
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
	mov eax,[z]
	shr eax,ZB_POINT_Z_FRAC_BITS
	mov [zz],eax
if TGL_FEATURE_RENDER_BITS eq 24
;    if (zz >= pz[_a]) {
;      pp[3 * _a]=or1 >> 8;
;      pp[3 * _a + 1]=og1 >> 8;
;      pp[3 * _a + 2]=ob1 >> 8;
;      pz[_a]=zz;
;    }
	mov eax,[dzdx]
	add [z],eax
;    og1+=dgdx;
;    or1+=drdx;
;    ob1+=dbdx;
elseif TGL_FEATURE_RENDER_BITS eq 16
;    if (zz >= pz[_a]) {
;      tmp=rgb & 0xF81F07E0;
;      pp[_a]=tmp | (tmp >> 16);
;      pz[_a]=zz;
;    }
;    z+=dzdx;
;    rgb=(rgb+drgbdx) & ( ~ 0x00200800);
else
;    if (zz >= pz[_a]) {
;      pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);
;      pz[_a]=zz;
;    }
	mov eax,[dzdx]
	add [z],eax
;    og1+=dgdx;
;    or1+=drdx;
;    ob1+=dbdx;
end if
}

DRAW_LINE_M equ 1

macro DRAW_LINE code
{
if TGL_FEATURE_RENDER_BITS eq 16
if code eq 0
;  register unsigned short *pz;
;  register PIXEL *pp;
;  register unsigned int tmp,z,zz,rgb,drgbdx;
;  register int n;
end if
if code eq 1
;  n=(x2 >> 16) - x1;
;  pp=pp1+x1;
;  pz=pz1+x1;
;  z=z1;
;  rgb=(r1 << 16) & 0xFFC00000;
;  rgb|=(g1 >> 5) & 0x000007FF;
;  rgb|=(b1 << 5) & 0x001FF000;
;  drgbdx=_drgbdx;
;  while (n>=3) {
;    PUT_PIXEL(0);
;    PUT_PIXEL(1);
;    PUT_PIXEL(2);
;    PUT_PIXEL(3);
;    pz+=4;
;    pp+=4;
;    n-=4;
;  }
;  while (n>=0) {
;    PUT_PIXEL(0);
;    pz+=1;
;    pp+=1;
;    n-=1;
;  }
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
;  texture=zb->current_texture;
}

macro PUT_PIXEL _a
{
;   zz=z >> ZB_POINT_Z_FRAC_BITS;
if TGL_FEATURE_RENDER_BITS eq 24
;   unsigned char *ptr;
;     if (zz >= pz[_a]) {
;       ptr = texture + (((t & 0x3FC00000) | s) >> 14) * 3;
;       pp[3 * _a]= ptr[0];
;       pp[3 * _a + 1]= ptr[1];
;       pp[3 * _a + 2]= ptr[2];
;       pz[_a]=zz;
;    }
else
;     if (zz >= pz[_a]) {
;       pp[_a]=texture[((t & 0x3FC00000) | s) >> 14];
;       pz[_a]=zz;
;    }
end if
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
;  texture=zb->current_texture;
;  fdzdx=(float)dzdx;
;  fndzdx=NB_INTERP * fdzdx;
;  ndszdx=NB_INTERP * dszdx;
;  ndtzdx=NB_INTERP * dtzdx;
}

macro PUT_PIXEL _a
{
;   zz=z >> ZB_POINT_Z_FRAC_BITS;
if TGL_FEATURE_RENDER_BITS eq 24
;   unsigned char *ptr;
;     if (zz >= pz[_a]) {
;       ptr = texture + (((t & 0x3FC00000) | (s & 0x003FC000)) >> 14) * 3;
;       pp[3 * _a]= ptr[0];
;       pp[3 * _a + 1]= ptr[1];
;       pp[3 * _a + 2]= ptr[2];
;       pz[_a]=zz;
;    }
else
;     if (zz >= pz[_a]) {
;       pp[_a]=*(PIXEL *)((char *)texture+
;               (((t & 0x3FC00000) | (s & 0x003FC000)) >> (17 - PSZSH)));
;       pz[_a]=zz;
;    }
end if
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
	pz dd ? ;uint *
	;edi = pp dd ?
	s dd ? ;uint
	t dd ? ;uint
	z dd ? ;uint
	zz dd ? ;uint
	n dd ? ;int
	dsdx dd ? ;int
	dtdx dd ? ;int
	s_z dd ? ;float
	t_z dd ? ;float
	fz dd ? ;float
	zinv dd ? ;float
end if
if code eq 1
;  n=(x2>>16)-x1;
;  fz=(float)z1;
;  zinv=1.0 / fz;
;  pp=(pp1 + x1 * PSZB);
;  pz=pz1+x1;
;  z=z1;
;  sz=sz1;
;  tz=tz1;
;  while (n>=(NB_INTERP-1)) {
;    {
;      float ss,tt;
;      ss=(sz * zinv);
;      tt=(tz * zinv);
;      s=(int) ss;
;      t=(int) tt;
;      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );
;      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );
;      fz+=fndzdx;
;      zinv=1.0 / fz;
;    }
;    PUT_PIXEL(0);
;    PUT_PIXEL(1);
;    PUT_PIXEL(2);
;    PUT_PIXEL(3);
;    PUT_PIXEL(4);
;    PUT_PIXEL(5);
;    PUT_PIXEL(6);
;    PUT_PIXEL(7);
;    pz+=NB_INTERP;
;    pp=(pp + NB_INTERP * PSZB);
;    n-=NB_INTERP;
;    sz+=ndszdx;
;    tz+=ndtzdx;
;  }
;    {
;      float ss,tt;
;      ss=(sz * zinv);
;      tt=(tz * zinv);
;      s=(int) ss;
;      t=(int) tt;
;      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );
;      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );
;    }
;  while (n>=0) {
;    PUT_PIXEL(0);
;    pz+=1;
;    pp=(PIXEL *)((char *)pp + PSZB);
;    n-=1;
;  }
end if
end if
}

align 4
proc ZB_fillTriangleMappingPerspective, zb:dword, p0:dword, p1:dword, p2:dword
locals
	texture dd ? ;PIXEL *
	fdzdx dd ? ;float
	fndzdx dd ?
	ndszdx dd ?
	ndtzdx dd ?
include 'ztriangle.inc'

end if

if 0

; slow but exact version (only there for reference, incorrect for 24
; bits)

INTERP_Z equ 1
INTERP_STZ equ 1

macro DRAW_INIT
{
;  texture=zb->current_texture;
}

macro PUT_PIXEL _a
{
;   float zinv;
;   int s,t;
;   zz=z >> ZB_POINT_Z_FRAC_BITS;
;     if (zz >= pz[_a]) {
;       zinv= 1.0 / (float) z;
;       s= (int) (sz * zinv);
;       t= (int) (tz * zinv);
;       pp[_a]=texture[((t & 0x3FC00000) | s) >> 14];
;       pz[_a]=zz;
;    }
	mov eax,[dzdx]
	add [z],eax
	mov eax,[dszdx]
	add [sz],eax
	mov eax,[dtzdx]
	add [tz],eax
}

align 4
proc ZB_fillTriangleMappingPerspective, zb:dword, p0:dword, p1:dword, p2:dword
locals
	texture dd ? ;PIXEL*
include 'ztriangle.inc'

end if
