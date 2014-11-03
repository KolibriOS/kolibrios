
align 4
proc glopClearColor uses ecx esi edi, context:dword, p:dword
	mov esi,[p]
	add esi,4
	mov edi,[context]
	add edi,offs_cont_clear_color
	mov ecx,4
	rep movsd
endp

align 4
proc glopClearDepth uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	mov dword[eax+offs_cont_clear_depth],ebx
	ret
endp

align 4
proc glopClear uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[eax+offs_cont_clear_color+8] ;context.clear_color.v[2]
	shl ebx,16
	push ebx
	mov ebx,[eax+offs_cont_clear_color+4] ;context.clear_color.v[1]
	shl ebx,16
	push ebx
	mov ebx,[eax+offs_cont_clear_color] ;context.clear_color.v[0]
	shl ebx,16
	push ebx

	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	and ebx,GL_COLOR_BUFFER_BIT
	push ebx
	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	and ebx,GL_DEPTH_BUFFER_BIT

	; TODO : correct value of Z
	stdcall ZB_clear,[eax+offs_cont_zb],ebx,0 ;,...,r,g,b
	ret
endp


