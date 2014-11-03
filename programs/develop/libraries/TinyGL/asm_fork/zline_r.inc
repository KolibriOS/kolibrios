;PUTPIXEL -> RGBPIXEL

locals
	n dd ?
	d_x dd ?
	d_y dd ?
	sx dd ? ;ширина буфера в пикселях
	pp_inc_1 dd ?
	pp_inc_2 dd ?
	a dd ? ;register int
	r dd ? ;register unsigned int
	g dd ?
	b dd ?
	rinc dd ? ;register unsigned int
	ginc dd ?
	binc dd ?
if INTERP_Z eq 1
	pz dd ? ;register unsigned short *
	zinc dd ?
	z dd ? ;register int
	zz dd ?
end if
endl
pushad

	mov eax,[p1]
	mov ebx,[p2]
	mov ecx,[ebx+offs_zbup_y]
	cmp [eax+offs_zbup_y], ecx ;if (p1.y > p2.y)
	jg @f
	jl .end_0 ;if (p1.y != p2.y)
	mov ecx,[ebx+offs_zbup_x]
	cmp [eax+offs_zbup_x], ecx ;if (p1.x > p2.x)
	jle .end_0 ;if (p1.x <= p2.x)
	@@: ;if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x))
		mov [p1],ebx
		mov [p2],eax
	.end_0:

	mov eax,[zb]
	mov edx,[eax+offs_zbuf_xsize]
	mov [sx],edx
	mov ecx,[p1]
	mov edi,[eax+offs_zbuf_linesize]
	imul edi,[ecx+offs_zbup_y]
	mov edx,[ecx+offs_zbup_x]
	imul edx,PSZB
	add edi,edx
	add edi,[eax+offs_zbuf_pbuf] ;edi = (zb.pbuf + zb.linesize*p1.y + p1.x*PSZB)
if INTERP_Z eq 1
	mov edx,[ecx+offs_zbup_y]
	imul edx,[sx]
	add edx,[ecx+offs_zbup_x]
	shl edx,1
	add edx,[eax+offs_zbuf_zbuf]
	mov [pz],edx ;pz = zb.zbuf + (p1.y*sx + p1.x)
	mov edx,[ecx+offs_zbup_z]
	mov [z],edx ;z = p1.z
end if

	mov ebx,[p2]
	mov eax,[ebx+offs_zbup_x]
	sub eax,[ecx+offs_zbup_x]
	mov [d_x],eax ;d_x = p2.x - p1.x
	mov eax,[ebx+offs_zbup_y]
	sub eax,[ecx+offs_zbup_y]
	mov [d_y],eax ;d_y = p2.y - p1.y

	mov eax,[ecx+offs_zbup_r]
	shl eax,8
	mov [r],eax ;r = p1.r << 8
	mov eax,[ecx+offs_zbup_g]
	shl eax,8
	mov [g],eax ;g = p1.g << 8
	mov eax,[ecx+offs_zbup_b]
	shl eax,8
	mov [b],eax ;b = p1.b << 8

macro RGBPIXEL
{
if TGL_FEATURE_RENDER_BITS eq 24
	mov eax,[r]
	shr eax,16
	mov byte[edi],al
	mov eax,[g]
	shr eax,16
	mov byte[edi+1],al
	mov eax,[b]
	shr eax,16
	mov byte[edi+2],al
;;;	else
;*pp = RGB_TO_PIXEL(r >> 8,g >> 8,b >> 8)
end if
}

macro PUTPIXEL
{
if INTERP_Z eq 1
local .end_0
	mov eax,[z]
	shr eax, ZB_POINT_Z_FRAC_BITS
	mov [zz],eax
	mov ebx,[pz]
	cmp ax,word[ebx]
	jl .end_0
		RGBPIXEL
		mov eax,dword[zz]
		mov ebx,[pz]
		mov word[ebx],ax
	.end_0:
else
	RGBPIXEL
end if
}

macro DRAWLINE d_x,d_y,inc_1,inc_2
{
local .mz_0
local .mz_1
local .mz_2
local .mr_0
local .mr_1
local .mr_2
local .mg_0
local .mg_1
local .mg_2
local .mb_0
local .mb_1
local .mb_2

	mov eax,d_x
	mov [n],eax

	mov ebx,[p1]
	mov ecx,[p2]
if INTERP_Z eq 1
	mov eax,[ecx+offs_zbup_z]
	cmp eax,[ebx+offs_zbup_z]
	jg .mz_0
	je .mz_1
		;if(p2.z<p1.z)
		sub eax,[ebx+offs_zbup_z]
		neg eax
		inc eax
		xor edx,edx
		div dword[n]
		neg eax
		inc eax
		jmp .mz_2
	.mz_0:
	sub eax,[ebx+offs_zbup_z]
	xor edx,edx
	div dword[n]
	jmp .mz_2
	.mz_1:
		xor eax,eax
	.mz_2:
	mov [zinc],eax ;zinc=(p2.z-p1.z)/n
end if

	;ebx=&p1, ecx=&p2
	mov eax,[ecx+offs_zbup_r]
	cmp eax,[ebx+offs_zbup_r]
	jg .mr_0
	je .mr_1
		;if(p2.r<p1.r)
		sub eax,[ebx+offs_zbup_r]
		neg eax
		inc eax
		shl eax,8
		xor edx,edx
		div dword[n]
		neg eax
		inc eax
		jmp .mr_2
	.mr_0:
	sub eax,[ebx+offs_zbup_r]
	shl eax,8
	xor edx,edx
	div dword[n]
	jmp .mr_2
	.mr_1:
		xor eax,eax
	.mr_2:
	mov [rinc],eax ;rinc=((p2.r-p1.r)<<8)/n

	mov eax,[ecx+offs_zbup_g]
	cmp eax,[ebx+offs_zbup_g]
	jg .mg_0
	je .mg_1
		;if(p2.g<p1.g)
		sub eax,[ebx+offs_zbup_g]
		neg eax
		inc eax
		shl eax,8
		xor edx,edx
		div dword[n]
		neg eax
		inc eax
		jmp .mg_2
	.mg_0:
	sub eax,[ebx+offs_zbup_g]
	shl eax,8
	xor edx,edx
	div dword[n]
	jmp .mg_2
	.mg_1:
		xor eax,eax
	.mg_2:
	mov [ginc],eax ;ginc=((p2.g-p1.g)<<8)/n

	mov eax,[ecx+offs_zbup_b]
	cmp eax,[ebx+offs_zbup_b]
	jg .mb_0
	je .mb_1
		;if(p2.b<p1.b)
		sub eax,[ebx+offs_zbup_b]
		neg eax
		inc eax
		shl eax,8
		xor edx,edx
		div dword[n]
		neg eax
		inc eax
		jmp .mb_2
	.mb_0:
	sub eax,[ebx+offs_zbup_b]
	shl eax,8
	xor edx,edx
	div dword[n]
	jmp .mb_2
	.mb_1:
		xor eax,eax
	.mb_2:
	mov [binc],eax ;binc=((p2.b-p1.b)<<8)/n

	shl dword d_y,1
	mov eax, d_y
	sub eax, d_x
	mov [a],eax ;a=2*d_y-d_x
	shl dword d_x,1
	mov eax, d_y
	sub d_x,eax
	mov eax,inc_1
	imul eax,PSZB
	mov [pp_inc_1],eax ;pp_inc_1 = inc_1*PSZB
	mov eax,inc_2
	imul eax,PSZB
	mov [pp_inc_2],eax ;pp_inc_2 = inc_2*PSZB

local .do_cycle
local .els_0
local .end_0

align 4
.do_cycle:

	PUTPIXEL
if INTERP_Z eq 1
	mov eax,[zinc]
	add [z],eax
end if
	mov eax,[rinc]
	add [r],eax
	mov eax,[ginc]
	add [g],eax
	mov eax,[binc]
	add [b],eax

	cmp dword[a],0
	jle .els_0
	add edi,[pp_inc_1]
if INTERP_Z eq 1
	mov eax,inc_1
	shl eax,1
	add [pz],eax
end if
	mov eax,d_x
	sub [a],eax
	jmp .end_0

.els_0:
	add edi,[pp_inc_2]
if INTERP_Z eq 1
	mov eax,inc_2
	shl eax,1
	add [pz],eax
end if
	mov eax,d_y
	add [a],eax

.end_0:
	dec dword[n]
	cmp dword[n],0
	jge .do_cycle
}

; fin macro

	mov eax,[d_x]
	cmp eax,0
	jne .els_0
	cmp dword[d_y],0
	jne .els_0
		;if (d_x==0 && d_y==0)
		PUTPIXEL
		jmp .end_2
	.els_0:
	cmp eax,0
	jle .els_3
		;if (d_x > 0)
		mov esi,[sx]
		inc esi
		cmp eax,[d_y]
		jl .els_2
			;if (d_x >= d_y)
			DRAWLINE [d_x], [d_y], esi, 1
			jmp .end_2
		.els_2:
			DRAWLINE [d_y], [d_x], esi, [sx]
			jmp .end_2
	.els_3:
		xor eax,eax
		sub eax,[d_x]
		mov [d_x],eax
		mov esi,[sx]
		dec esi
		cmp eax,[d_y]
		jl .els_4
			;if (d_x >= d_y)
			DRAWLINE [d_x], [d_y], esi, -1
			jmp .end_2
		.els_4:
			DRAWLINE [d_y], [d_x], esi, [sx]
	.end_2:

popad
	ret
endp

restore INTERP_Z

; internal defines
purge DRAWLINE
purge PUTPIXEL
purge RGBPIXEL
