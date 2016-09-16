
align 4
proc glopClearColor uses ecx esi edi, context:dword, p:dword
	mov esi,[p]
	add esi,4
	mov edi,[context]
	add edi,GLContext.clear_color
	mov ecx,4
	rep movsd
	ret
endp

align 4
proc glopClearDepth uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	mov dword[eax+GLContext.clear_depth],ebx
	ret
endp

align 4
fl_65535 dd 65535.0

align 4
proc glopClear uses eax ebx, context:dword, p:dword
	mov eax,[context]
	fld dword[eax+GLContext.clear_color+8] ;context.clear_color.v[2]
	fmul dword[fl_65535]
	fistp dword[esp-4]
	fld dword[eax+GLContext.clear_color+4] ;context.clear_color.v[1]
	fmul dword[fl_65535]
	fistp dword[esp-8]
	fld dword[eax+GLContext.clear_color] ;context.clear_color.v[0]
	fmul dword[fl_65535]
	fistp dword[esp-12]
	sub esp,12

	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	and ebx,GL_COLOR_BUFFER_BIT
	push ebx
	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	and ebx,GL_DEPTH_BUFFER_BIT

	; TODO : correct value of Z
	stdcall ZB_clear,[eax+GLContext.zb],ebx,0 ;,...,r,g,b
	ret
endp


