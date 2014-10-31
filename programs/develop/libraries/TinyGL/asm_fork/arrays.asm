VERTEX_ARRAY   equ 0x0001
COLOR_ARRAY    equ 0x0002
NORMAL_ARRAY   equ 0x0004
TEXCOORD_ARRAY equ 0x0008

align 4
proc glopArrayElement uses eax ebx ecx edx, context:dword, param:dword
locals
	p rd 5
endl
	mov eax,[context]
	mov ebx,[param]
	mov ebx,[ebx+4] ;ebx = p[1]
  
	bt dword[eax+offs_cont_client_states],1 ;2^1=COLOR_ARRAY
	jnc @f
		mov ecx,[eax+offs_cont_color_array_size]
		add ecx,[eax+offs_cont_color_array_stride]
		imul ecx,ebx
		shl ecx,2
		add ecx,eax
		add ecx,offs_cont_color_array ;ecx = &context.color_array[i]
		mov ebx,ebp
		sub ebx,20 ;=sizeof(dd)*5
		mov edx,[ecx]
		mov [ebx+4],edx
		mov edx,[ecx+4]
		mov [ebx+8],edx
		mov edx,[ecx+8]
		mov [ebx+12],edx
		cmp dword[eax+offs_cont_color_array_size],3
		jg .l0
			mov edx,1.0
			jmp .l1
		.l0:
			mov edx,[ecx+12]
		.l1:
		mov [ebx+16],edx
		stdcall glopColor, eax, ebx
	@@:
	bt dword[eax+offs_cont_client_states],2 ;2^2=NORMAL_ARRAY
	jnc @f
		mov esi,dword[eax+offs_cont_normal_array_stride]
		add esi,3
		imul esi,ebx
		shl esi,2
		add esi,eax
		add esi,offs_cont_normal_array ;esi = &normal_array[ebx * (3 + c->normal_array_stride)]
		mov edi,eax
		add edi,offs_cont_current_normal
		mov ecx,3
		rep movsd
		mov dword[edi],0.0
	@@:
	bt dword[eax+offs_cont_client_states],3 ;2^3=TEXCOORD_ARRAY
	jnc @f
		mov ecx,[eax+offs_cont_texcoord_array_size]
		add ecx,[eax+offs_cont_texcoord_array_stride]
		imul ecx,ebx
		shl ecx,2
		add ecx,eax
		add ecx,offs_cont_texcoord_array ;ecx = &context.texcoord_array[i]
		mov edx,[ecx]
		mov [eax+offs_cont_current_tex_coord],edx
		mov edx,[ecx+4]
		mov [eax+offs_cont_current_tex_coord+4],edx

		cmp dword[eax+offs_cont_texcoord_array_size],2
		jg .l2
			mov edx,0.0
			jmp .l3
		.l2:
			mov edx,[ecx+8]
		.l3:
		mov [eax+offs_cont_current_tex_coord+8],edx

		cmp dword[eax+offs_cont_texcoord_array_size],3
		jg .l4
			mov edx,1.0
			jmp .l5
		.l4:
			mov edx,[ecx+12]
		.l5:

		mov [eax+offs_cont_current_tex_coord+12],edx
	@@:
	bt dword[eax+offs_cont_client_states],0 ;2^0=VERTEX_ARRAY
	jnc @f
		mov ecx,[eax+offs_cont_vertex_array_size]
		add ecx,[eax+offs_cont_vertex_array_stride]
		imul ecx,ebx
		shl ecx,2
		add ecx,eax
		add ecx,offs_cont_vertex_array ;ecx = &context.vertex_array[i]
		mov ebx,ebp
		sub ebx,20 ;=sizeof(dd)*5
		mov edx,[ecx]
		mov [ebx+4],edx
		mov edx,[ecx+4]
		mov [ebx+8],edx

		cmp dword[eax+offs_cont_vertex_array_size],2
		jg .l6
			mov edx,0.0
			jmp .l7
		.l6:
			mov edx,[ecx+8]
		.l7:
		mov [ebx+12],edx

		cmp dword[eax+offs_cont_vertex_array_size],3
		jg .l8
			mov edx,1.0
			jmp .l9
		.l8:
			mov edx,[ecx+12]
		.l9:

		mov [ebx+16],edx
		stdcall glopVertex, eax, ebx
	@@:
	ret
endp

align 4
proc glArrayElement uses eax, i:dword
locals
	p rd 2
endl
	mov dword[p],OP_ArrayElement
	mov eax,[i]
	mov dword[p+4],eax

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glopEnableClientState uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	or dword[eax+offs_cont_client_states],ebx
	ret
endp

align 4
proc glEnableClientState uses eax, array:dword
locals
	p rd 2
endl
	mov dword[p],OP_EnableClientState

	cmp dword[array],GL_VERTEX_ARRAY
	jne @f
		mov dword[p+4],VERTEX_ARRAY
		jmp .end_f
	@@:
	cmp dword[array],GL_NORMAL_ARRAY
	jne @f
		mov dword[p+4],NORMAL_ARRAY
		jmp .end_f
	@@:
	cmp dword[array],GL_COLOR_ARRAY
	jne @f
		mov dword[p+4],COLOR_ARRAY
		jmp .end_f
	@@:
	cmp dword[array],GL_TEXTURE_COORD_ARRAY
	jne @f
		mov dword[p+4],TEXCOORD_ARRAY
		jmp .end_f
	@@:
		;assert(0);
	.end_f:

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glopDisableClientState uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	and dword[eax+offs_cont_client_states],ebx
	ret
endp

align 4
proc glDisableClientState uses eax, array:dword
locals
	p rd 2
endl
	mov dword[p],OP_DisableClientState

	cmp dword[array],GL_VERTEX_ARRAY
	jne @f
		mov dword[p+4], not VERTEX_ARRAY
		jmp .end_f
	@@:
	cmp dword[array],GL_NORMAL_ARRAY
	jne @f
		mov dword[p+4], not NORMAL_ARRAY
		jmp .end_f
	@@:
	cmp dword[array],GL_COLOR_ARRAY
	jne @f
		mov dword[p+4], not COLOR_ARRAY
		jmp .end_f
	@@:
	cmp dword[array],GL_TEXTURE_COORD_ARRAY
	jne @f
		mov dword[p+4], not TEXCOORD_ARRAY
		jmp .end_f
	@@:
		;assert(0);
	.end_f:

	mov eax,ebp
	sub eax,8 ;=sizeof(dd)*2
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glopVertexPointer uses eax ebx ecx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+4] ;ecx = p[1]
	mov dword[eax+offs_cont_vertex_array_size],ecx
	mov ecx,[ebx+8] ;ecx = p[2]
	mov dword[eax+offs_cont_vertex_array_stride],ecx
	mov ecx,[ebx+12] ;ecx = p[3]
	mov dword[eax+offs_cont_vertex_array],ecx
	ret
endp

align 4
proc glVertexPointer uses eax, size:dword, type:dword, stride:dword, pointer:dword
locals
	p rd 4
endl
;  assert(type == GL_FLOAT);

	mov dword[p],OP_TexCoordPointer
	mov eax,[size]
	mov dword[p+4],eax
	mov eax,[stride]
	mov dword[p+8],eax
	mov eax,[pointer]
	mov dword[p+12],eax

	mov eax,ebp
	sub eax,16 ;=sizeof(dd)*4
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glopColorPointer uses eax ebx ecx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+4] ;ecx = p[1]
	mov dword[eax+offs_cont_color_array_size],ecx
	mov ecx,[ebx+8] ;ecx = p[2]
	mov dword[eax+offs_cont_color_array_stride],ecx
	mov ecx,[ebx+12] ;ecx = p[3]
	mov dword[eax+offs_cont_color_array],ecx
	ret
endp

align 4
proc glColorPointer uses eax, size:dword, type:dword, stride:dword, pointer:dword
locals
	p rd 4
endl
;  assert(type == GL_FLOAT);

	mov dword[p],OP_ColorPointer
	mov eax,[size]
	mov dword[p+4],eax
	mov eax,[stride]
	mov dword[p+8],eax
	mov eax,[pointer]
	mov dword[p+12],eax

	mov eax,ebp
	sub eax,16 ;=sizeof(dd)*4
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glopNormalPointer uses eax ebx ecx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+4] ;ecx = p[1]
	mov dword[eax+offs_cont_normal_array_stride],ecx
	mov ecx,[ebx+8] ;ecx = p[2]
	mov dword[eax+offs_cont_normal_array],ecx
	ret
endp

align 4
proc glNormalPointer uses eax, type:dword, stride:dword, pointer:dword
locals
	p rd 3
endl
;  assert(type == GL_FLOAT);

	mov dword[p],OP_NormalPointer
	mov eax,[stride]
	mov dword[p+4],eax
	mov eax,[pointer]
	mov dword[p+8],eax

	mov eax,ebp
	sub eax,12 ;=sizeof(dd)*3
	stdcall gl_add_op,eax
	ret
endp

align 4
proc glopTexCoordPointer uses eax ebx ecx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+4] ;ecx = p[1]
	mov dword[eax+offs_cont_texcoord_array_size],ecx
	mov ecx,[ebx+8] ;ecx = p[2]
	mov dword[eax+offs_cont_texcoord_array_stride],ecx
	mov ecx,[ebx+12] ;ecx = p[3]
	mov dword[eax+offs_cont_texcoord_array],ecx
	ret
endp

align 4
proc glTexCoordPointer uses eax, size:dword, type:dword, stride:dword, pointer:dword
locals
	p rd 4
endl
;  assert(type == GL_FLOAT);

	mov dword[p],OP_TexCoordPointer
	mov eax,[size]
	mov dword[p+4],eax
	mov eax,[stride]
	mov dword[p+8],eax
	mov eax,[pointer]
	mov dword[p+12],eax

	mov eax,ebp
	sub eax,16 ;=sizeof(dd)*4
	stdcall gl_add_op,eax
	ret
endp
