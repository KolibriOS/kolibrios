
align 4
proc ZB_plot uses eax ebx ecx edx edi, zb:dword, p:dword
	mov eax,[zb]
	mov ebx,[p]
	mov ecx,[ebx+ZBufferPoint.y]
	imul ecx,[eax+ZBuffer.xsize]
	add ecx,[ebx+ZBufferPoint.x]
	shl ecx,1
	add ecx,[eax+ZBuffer.zbuf]
	mov edx,[eax+ZBuffer.linesize]
	imul edx,[ebx+ZBufferPoint.y]
	mov edi,[ebx+ZBufferPoint.x]
	imul edi,PSZB
	add edx,edi
	add edx,[eax+ZBuffer.pbuf]
	mov edi,[ebx+ZBufferPoint.z]
	shr edi,ZB_POINT_Z_FRAC_BITS
	cmp di,word[ecx]
	jl .end_f
if TGL_FEATURE_RENDER_BITS eq 24
	mov eax,[ebx+ZBufferPoint.r]
	mov byte[edx],ah
	mov eax,[ebx+ZBufferPoint.g]
	mov byte[edx+1],ah
	mov eax,[ebx+ZBufferPoint.b]
	mov byte[edx+2],ah
else
;	*pp = RGB_TO_PIXEL(p->r, p->g, p->b);
end if
	mov word[ecx],di
	.end_f:
	ret
endp

INTERP_Z equ 1
align 4
proc ZB_line_flat_z, zb:dword, p1:dword, p2:dword, color:dword
include 'zline.inc'

; line with color interpolation
INTERP_Z equ 1
align 4
proc ZB_line_interp_z, zb:dword, p1:dword, p2:dword
include 'zline_r.inc'

; no Z interpolation

align 4
proc ZB_line_flat, zb:dword, p1:dword, p2:dword, color:dword
include 'zline.inc'

align 4
proc ZB_line_interp, zb:dword, p1:dword, p2:dword
include 'zline_r.inc'

align 4
proc ZB_line_z uses eax ebx ecx, zb:dword, p1:dword, p2:dword
if DEBUG ;ZB_line_z
push edi
	mov ecx,80
	mov eax,[p1]
	mov eax,[eax+ZBufferPoint.x]
	lea edi,[buf_param]
	stdcall convert_int_to_str,ecx
	stdcall str_n_cat,edi,txt_zp_sp,2
	stdcall str_len,edi
	add edi,eax
	sub ecx,eax

	mov eax,[p1]
	mov eax,[eax+ZBufferPoint.y]
	stdcall convert_int_to_str,ecx
	stdcall str_n_cat,edi,txt_zp_sp,2
	stdcall str_len,edi
	add edi,eax
	sub ecx,eax

	mov eax,[p2]
	mov eax,[eax+ZBufferPoint.x]
	stdcall convert_int_to_str,ecx
	stdcall str_n_cat,edi,txt_zp_sp,2
	stdcall str_len,edi
	add edi,eax
	sub ecx,eax

	mov eax,[p2]
	mov eax,[eax+ZBufferPoint.y]
	stdcall convert_int_to_str,ecx

	stdcall str_n_cat,edi,txt_nl,2
	stdcall dbg_print,f_zbz,buf_param
pop edi
end if
	mov ebx,[p1]
	RGB_TO_PIXEL dword[ebx+ZBufferPoint.r],dword[ebx+ZBufferPoint.g],dword[ebx+ZBufferPoint.b]
	mov ecx,eax
	mov ebx,[p2]
	RGB_TO_PIXEL dword[ebx+ZBufferPoint.r],dword[ebx+ZBufferPoint.g],dword[ebx+ZBufferPoint.b]

	; choose if the line should have its color interpolated or not
	cmp ecx,eax
	jne .els
		stdcall ZB_line_flat_z, dword[zb], dword[p1], ebx, ecx
		jmp @f
	.els:
		stdcall ZB_line_interp_z, dword[zb], dword[p1], ebx
	@@:
	ret
endp

align 4
proc ZB_line uses eax ebx ecx, zb:dword, p1:dword, p2:dword
if DEBUG ;ZB_line
push edi
	mov ecx,80
	mov eax,[p1]
	mov eax,[eax+ZBufferPoint.x]
	lea edi,[buf_param]
	stdcall convert_int_to_str,ecx
	stdcall str_n_cat,edi,txt_zp_sp,2
	stdcall str_len,edi
	add edi,eax
	sub ecx,eax

	mov eax,[p1]
	mov eax,[eax+ZBufferPoint.y]
	stdcall convert_int_to_str,ecx
	stdcall str_n_cat,edi,txt_zp_sp,2
	stdcall str_len,edi
	add edi,eax
	sub ecx,eax

	mov eax,[p2]
	mov eax,[eax+ZBufferPoint.x]
	stdcall convert_int_to_str,ecx
	stdcall str_n_cat,edi,txt_zp_sp,2
	stdcall str_len,edi
	add edi,eax
	sub ecx,eax

	mov eax,[p2]
	mov eax,[eax+ZBufferPoint.y]
	stdcall convert_int_to_str,ecx

	stdcall str_n_cat,edi,txt_nl,2
	stdcall dbg_print,f_zb,buf_param
pop edi
end if
	mov ebx,[p1]
	RGB_TO_PIXEL dword[ebx+ZBufferPoint.r],dword[ebx+ZBufferPoint.g],dword[ebx+ZBufferPoint.b]
	mov ecx,eax
	mov ebx,[p2]
	RGB_TO_PIXEL dword[ebx+ZBufferPoint.r],dword[ebx+ZBufferPoint.g],dword[ebx+ZBufferPoint.b]

	; choose if the line should have its color interpolated or not
	cmp ecx,eax
	jne .els
		stdcall ZB_line_flat, dword[zb], dword[p1], ebx, ecx
		jmp @f
	.els:
		stdcall ZB_line_interp, dword[zb], dword[p1], ebx
	@@:
	ret
endp
