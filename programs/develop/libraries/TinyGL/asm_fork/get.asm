align 4
proc glGetIntegerv uses eax edi, pname:dword, params:dword
	mov eax,[pname]
	mov edi,[params]
	cmp eax,GL_VIEWPORT
	jne @f
		push esi
		call gl_get_context
		lea esi,[eax+GLContext.viewport]
		movsd ;m2m dword[edi],dword[eax+GLContext.viewport.xmin]
		movsd ;copy .ymin
		movsd ;copy .xsize
		movsd ;copy .ysize
		pop esi
		jmp .endf
	@@:
	cmp eax,GL_MAX_MODELVIEW_STACK_DEPTH
	jne @f
		mov dword[edi],MAX_MODELVIEW_STACK_DEPTH
		jmp .endf
	@@:
	cmp eax,GL_MAX_PROJECTION_STACK_DEPTH
	jne @f
		mov dword[edi],MAX_PROJECTION_STACK_DEPTH
		jmp .endf
	@@:
	cmp eax,GL_MAX_LIGHTS
	jne @f
		mov dword[edi],MAX_LIGHTS
		jmp .endf
	@@:
	cmp eax,GL_MAX_TEXTURE_SIZE
	jne @f
		mov dword[edi],4096
		jmp .endf
	@@:
	cmp eax,GL_MAX_TEXTURE_STACK_DEPTH
	jne @f
		mov dword[edi],MAX_TEXTURE_STACK_DEPTH
		jmp .endf
	@@:
	stdcall dbg_print,sz_kosglMakeCurrent,err_glGet
	.endf:
	ret
endp