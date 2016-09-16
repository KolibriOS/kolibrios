
align 4
proc glopViewport uses eax ebx ecx edx, context:dword, p:dword
locals
	xsize dd ? ;int
	ysize dd ? ;int
	xmin dd ? ;int
	ymin dd ? ;int
	xsize_req dd ? ;int
	ysize_req dd ? ;int
endl
	mov edx,[context]
	mov ebx,[p]

	mov ecx,[ebx+4]
	mov [xmin],ecx
	mov ecx,[ebx+8]
	mov [ymin],ecx
	mov ecx,[ebx+12]
	mov [xsize],ecx
	mov ecx,[ebx+16]
	mov [ysize],ecx

	; we may need to resize the zbuffer

	cmp dword[edx+GLContext.viewport+offs_vpor_ysize],ecx
	jne @f
	mov ecx,[xmin]
	cmp dword[edx+GLContext.viewport+offs_vpor_xmin],ecx
	jne @f
	mov ecx,[ymin]
	cmp dword[edx+GLContext.viewport+offs_vpor_ymin],ecx
	jne @f
	mov ecx,[xsize]
	cmp dword[edx+GLContext.viewport+offs_vpor_xsize],ecx
	jne @f
		jmp .end_f
	@@:

	mov ecx,[xmin]
	add ecx,[xsize]
	mov [xsize_req],ecx ;xsize_req = xmin + xsize
	mov ecx,[ymin]
	add ecx,[ysize]
	mov [ysize_req],ecx ;ysize_req = ymin + ysize

	cmp dword[edx+GLContext.gl_resize_viewport],0
	je @f
		mov eax,ebp
		sub eax,4
		push eax
		sub eax,4
		push eax
		stdcall dword[edx+GLContext.gl_resize_viewport], edx ;gl_resize_viewport(context,&xsize_req,&ysize_req)
		cmp eax,0
		je @f
			stdcall dbg_print,sz_glViewport,err_4
	@@:

	mov ecx,[xsize_req]
	sub ecx,[xmin]
	mov [xsize],ecx
	mov ecx,[ysize_req]
	sub ecx,[ymin]
	mov [ysize],ecx
	cmp ecx,0
	jg @f
	cmp dword[xsize],0
	jg @f
		stdcall dbg_print,sz_glViewport,err_5
	@@:
	mov ecx,[xmin]
	mov dword[edx+GLContext.viewport+offs_vpor_xmin],ecx
	mov ecx,[ymin]
	mov dword[edx+GLContext.viewport+offs_vpor_ymin],ecx
	mov ecx,[xsize]
	mov dword[edx+GLContext.viewport+offs_vpor_xsize],ecx
	mov ecx,[ysize]
	mov dword[edx+GLContext.viewport+offs_vpor_ysize],ecx

	mov dword[edx+GLContext.viewport+offs_vpor_updated],1
	.end_f:
	ret
endp

align 4
proc glopEnableDisable uses eax ebx ecx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+8]
	mov ebx,[ebx+4]

	cmp ebx,GL_CULL_FACE
	jne @f
		mov [eax+GLContext.cull_face_enabled],ecx
		jmp .end_f
	@@:
	cmp ebx,GL_LIGHTING
	jne @f
		mov [eax+GLContext.lighting_enabled],ecx
		jmp .end_f
	@@:
	cmp ebx,GL_COLOR_MATERIAL
	jne @f
		mov [eax+GLContext.color_material_enabled],ecx
		jmp .end_f
	@@:
	cmp ebx,GL_TEXTURE_2D
	jne @f
		mov [eax+GLContext.texture_2d_enabled],ecx
		jmp .end_f
	@@:
	cmp ebx,GL_NORMALIZE
	jne @f
		mov [eax+GLContext.normalize_enabled],ecx
		jmp .end_f
	@@:
	cmp ebx,GL_DEPTH_TEST
	jne @f
		mov [eax+GLContext.depth_test],ecx
		jmp .end_f
	@@:
	cmp ebx,GL_POLYGON_OFFSET_FILL
	jne .polygon_offset_fill
		cmp ecx,0
		je @f
			or dword[eax+GLContext.offset_states],TGL_OFFSET_FILL
			jmp .end_f
		@@:
			and dword[eax+GLContext.offset_states],not TGL_OFFSET_FILL
		jmp .end_f
	.polygon_offset_fill:
	cmp ebx,GL_POLYGON_OFFSET_POINT
	jne .polygon_offset_point
		cmp ecx,0
		je @f
			or dword[eax+GLContext.offset_states],TGL_OFFSET_POINT
			jmp .end_f
		@@:
			and dword[eax+GLContext.offset_states],not TGL_OFFSET_POINT
		jmp .end_f
	.polygon_offset_point:
	cmp ebx,GL_POLYGON_OFFSET_LINE
	jne .polygon_offset_line
		cmp ecx,0
		je @f
			or dword[eax+GLContext.offset_states],TGL_OFFSET_LINE
			jmp .end_f
		@@:
			and dword[eax+GLContext.offset_states],not TGL_OFFSET_LINE
		jmp .end_f
	.polygon_offset_line: ;default:
	cmp ebx,GL_LIGHT0
	jl .els_0
	cmp ebx,GL_LIGHT0+MAX_LIGHTS
	jge .els_0 ;if (GL_LIGHT0 <= ebx < GL_LIGHT0+MAX_LIGHTS)
		sub ebx,GL_LIGHT0
		stdcall gl_enable_disable_light, eax,ebx,ecx
		jmp .end_f
	.els_0:
;fprintf(stderr,"glEnableDisable: 0x%X not supported.\n",code);
	.end_f:
	ret
endp

align 4
proc glopShadeModel uses eax ebx, context:dword,p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4]
	mov [eax+GLContext.current_shade_model],ebx
	ret
endp

align 4
proc glopCullFace uses eax ebx, context:dword,p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4]
	mov [eax+GLContext.current_cull_face],ebx
	ret
endp

align 4
proc glopFrontFace uses eax ebx, context:dword,p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4]
	mov [eax+GLContext.current_front_face],ebx
	ret
endp

align 4
proc glopPolygonMode uses eax ebx, context:dword,p:dword
	mov eax,[context]
	mov ebx,[p]

	cmp dword[ebx+4],GL_BACK
	jne @f
		mov ebx,[ebx+8]
		mov [eax+GLContext.polygon_mode_back],ebx
		jmp .end_f
	@@:
	cmp dword[ebx+4],GL_FRONT
	jne @f
		mov ebx,[ebx+8]
		mov [eax+GLContext.polygon_mode_front],ebx
		jmp .end_f
	@@:
	cmp dword[ebx+4],GL_FRONT_AND_BACK
	jne @f
		mov ebx,[ebx+8]
		mov [eax+GLContext.polygon_mode_front],ebx
		mov [eax+GLContext.polygon_mode_back],ebx
		jmp .end_f
	@@:
;    assert(0);
	.end_f:
	ret
endp

align 4
proc glopHint, context:dword,p:dword
if 0
;  int target=p[1].i;
;  int mode=p[2].i;

	; do nothing
end if
	ret
endp

align 4
proc glopPolygonOffset uses eax ebx ecx, context:dword,p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+4]
	mov [eax+GLContext.offset_factor],ecx
	mov ecx,[ebx+8]
	mov [eax+GLContext.offset_units],ecx
	ret
endp