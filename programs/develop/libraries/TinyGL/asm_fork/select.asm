
align 4
proc glRenderMode uses ebx ecx, mode:dword
	call gl_get_context
	xor ebx,ebx

	cmp dword[eax+GLContext.render_mode],GL_RENDER
	je .e_sw_1
	cmp dword[eax+GLContext.render_mode],GL_SELECT
	jne .def_1
		cmp dword[eax+GLContext.select_overflow],0
		je @f
			sub ebx,[eax+GLContext.select_hits]
			jmp .else_e
		@@:
			mov ebx,[eax+GLContext.select_hits]
		.else_e:
			mov dword[eax+GLContext.select_overflow],0
			mov ecx,[eax+GLContext.select_buffer]
			mov dword[eax+GLContext.select_ptr],ecx
			mov dword[eax+GLContext.name_stack_size],0
		jmp .e_sw_1
	.def_1:
;    assert(0);
	.e_sw_1:
	cmp dword[mode],GL_RENDER
	jne @f
		mov dword[eax+GLContext.render_mode],GL_RENDER
		jmp .e_sw_2
	@@:
	cmp dword[mode],GL_SELECT
	jne .def_2
		mov dword[eax+GLContext.render_mode],GL_SELECT
;    assert( c->select_buffer != NULL);
		mov ecx,[eax+GLContext.select_buffer]
		mov dword[eax+GLContext.select_ptr],ecx
		mov dword[eax+GLContext.select_hits],0
		mov dword[eax+GLContext.select_overflow],0
		mov dword[eax+GLContext.select_hit],0 ;NULL
		jmp .e_sw_2
	.def_2:
;    assert(0);
	.e_sw_2:
	mov eax,ebx
	ret
endp

align 4
proc glSelectBuffer uses eax ebx, size:dword, buf:dword
	call gl_get_context

;  assert(c->render_mode != GL_SELECT);
  
	mov ebx,[buf]
	mov dword[eax+GLContext.select_buffer],ebx
	mov ebx,[size]
	mov dword[eax+GLContext.select_size],ebx
	ret
endp

align 4
proc glopInitNames uses eax, context:dword, p:dword
	mov eax,[context]
	cmp dword[eax+GLContext.render_mode],GL_SELECT
	jne @f
		mov dword[eax+GLContext.name_stack_size],0
		mov dword[eax+GLContext.select_hit],0 ;=NULL
	@@:
	ret
endp

align 4
proc glopPushName uses eax ebx, context:dword, p:dword
	mov eax,[context]
	cmp dword[eax+GLContext.render_mode],GL_SELECT
	jne @f
;    assert(c->name_stack_size<MAX_NAME_STACK_DEPTH);
		mov dword[eax+GLContext.select_hit],0 ;=NULL
		inc dword[eax+GLContext.name_stack_size]
		mov ebx,dword[eax+GLContext.name_stack_size]
		shl ebx,2
		add ebx,eax
		mov eax,[p]
		mov eax,[eax+4]
		mov dword[ebx+GLContext.name_stack],eax ;context.name_stack[context.name_stack_size++]=p[1]
	@@:
	ret
endp

align 4
proc glopPopName uses eax, context:dword, p:dword
	mov eax,[context]
	cmp dword[eax+GLContext.render_mode],GL_SELECT
	jne @f
;    assert(c->name_stack_size>0);
		dec dword[eax+GLContext.name_stack_size]
		mov dword[eax+GLContext.select_hit],0 ;=NULL
	@@:
	ret
endp

align 4
proc glopLoadName uses eax ebx, context:dword, p:dword
	mov eax,[context]
	cmp dword[eax+GLContext.render_mode],GL_SELECT
	jne @f
;   assert(c->name_stack_size>0);
		mov dword[eax+GLContext.select_hit],0 ;=NULL
		mov ebx,dword[eax+GLContext.name_stack_size]
		dec ebx
		shl ebx,2
		add ebx,eax
		mov eax,[p]
		mov eax,[eax+4]
		mov dword[ebx+GLContext.name_stack],eax ;context.name_stack[context.name_stack_size-1]=p[1]
	@@:
	ret
endp

align 4
proc gl_add_select uses eax ebx ecx edx, context:dword, zmin:dword, zmax:dword
;  unsigned int *ptr;
;  int i;
	mov eax,[context]

	cmp dword[eax+GLContext.select_overflow],0
	jne .end_f ;if (!context.select_overflow)
	cmp dword[eax+GLContext.select_hit],0 ;if (context.select_hit==NULL)
	jne .els_0
		mov ecx,[eax+GLContext.name_stack_size]
		mov ebx,[eax+GLContext.select_ptr]
		sub ebx,[eax+GLContext.select_buffer]
		sub ebx,3
		sub ebx,ecx
		cmp ebx,[eax+GLContext.select_size]
		jle .els_1
			mov dword[eax+GLContext.select_overflow],1
		jmp .end_f
		.els_1:
		mov ebx,[eax+GLContext.select_ptr]
		mov [eax+GLContext.select_hit],ebx
		mov edx,[eax+GLContext.name_stack_size]
		mov [ebx],edx
		add ebx,4
		mov edx,[zmin]
		mov [ebx],edx
		add ebx,4
		mov edx,[zmax]
		mov [ebx],edx
		add ebx,4
;	for(i=0;i<ecx;i++) *ptr++=c->name_stack[i];
		mov [eax+GLContext.select_ptr],ebx
		inc dword[eax+GLContext.select_hits]
		jmp .end_f
	.els_0:
		mov ebx,[zmin]
		cmp dword[eax+GLContext.select_hit+4],ebx
		jle @f
			mov dword[eax+GLContext.select_hit+4],ebx
		@@:
		mov ebx,[zmax]
		cmp dword[eax+GLContext.select_hit+8],ebx
		jge .end_f
			mov dword[eax+GLContext.select_hit+8],ebx
	.end_f:
	ret
endp

