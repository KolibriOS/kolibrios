; simple gl like driver for TinyGL and KolibriOS - porting iadn


struct TinyGLContext
	gl_context dd ?
	xsize dd ? ;+4
	ysize dd ? ;+8
	d_x dd ? ;+12
	d_y dd ? ;+16
	x dd ? ;+20
	y dd ? ;+24
ends

;KOSGLContext kosglCreateContext(KOSGLContext shareList, int flags)
;{
;  TinyGLContext *ctx;

;  if (shareList != NULL) {
;    gl_fatal_error("No sharing available in TinyGL");    
;  }

;    ctx=gl_malloc(sizeof(TinyGLContext));
;  if (!ctx)
;      return NULL;
;  ctx->gl_context=NULL;
;  return (KOSGLContext) ctx;
;}

;void kosglDestroyContext( KOSGLContext ctx1 )
;{
;  TinyGLContext *ctx = (TinyGLContext *) ctx1;
;  if (ctx->gl_context != NULL) {
;    glClose();
;  }
;  gl_free(ctx);
;}

; resize the glx viewport : we try to use the xsize and ysize
; given. We return the effective size which is guaranted to be smaller

align 4
proc gl_resize_viewport uses ebx ecx edx edi esi, context:dword, xsize_ptr:dword, ysize_ptr:dword
	mov ecx,[xsize_ptr] ; ecx = &xsize
	mov edi,[ecx]       ; edi =  xsize
	mov esi,[ysize_ptr] ; esi = &ysize
	mov esi,[esi]       ; esi =  ysize

	xor eax,eax
	or edi,edi
	jnz @f
	or esi,esi
	jnz @f
		mov eax,-1
		jmp .end_f
	@@:

	mov [ecx],edi
	dec dword[ecx]
	mov ecx,[ysize_ptr]
	mov [ecx],esi
	dec dword[ecx]

	mov ebx,[context]
	mov edx,[ebx+GLContext.opaque] ; edx = (TinyGLContext *)context.opaque
	mov [edx+TinyGLContext.xsize],edi
	mov [edx+TinyGLContext.d_x],edi
	mov [edx+TinyGLContext.ysize],esi
	mov [edx+TinyGLContext.d_y],esi

	; resize the Z buffer
	stdcall ZB_resize, dword[ebx+GLContext.zb],0,edi,esi
	.end_f:
	ret
endp

; we assume here that drawable is a window
align 4
proc kosglMakeCurrent uses ebx ecx, win_x0:dword, win_y0:dword, win_x:dword, win_y:dword, ctx1:dword
	mov ebx,[ctx1]
	cmp dword[ebx],0 ;if (ctx.gl_context == NULL)
	jne .end_f
		; create the TinyGL context
		mov ecx,[win_x0]
		mov [ebx+TinyGLContext.x],ecx
		mov ecx,[win_y0]
		mov [ebx+TinyGLContext.y],ecx
		mov ecx,[win_x]
		mov [ebx+TinyGLContext.d_x],ecx
		mov ecx,[win_y]
		mov [ebx+TinyGLContext.d_y],ecx

		; currently, we only support 16 bit rendering
		xor eax,eax
		stdcall ZB_open, dword[win_x], dword[win_y], dword ZB_MODE_RGB24, eax,eax,eax,eax ;NULL,NULL,NULL

		or eax,eax
		jnz @f
			stdcall dbg_print,sz_kosglMakeCurrent,err_0
			xor eax,eax
			jmp .err_f
		@@:

		; initialisation of the TinyGL interpreter
		stdcall glInit, eax

		call gl_get_context
		mov [ebx],eax ;ctx.gl_context = eax

		mov [eax+GLContext.opaque],ebx ;ctx.gl_context.opaque = ctx
		mov dword[eax+GLContext.gl_resize_viewport],gl_resize_viewport

		; set the viewport : we force a call to gl_resize_viewport
		dec dword[eax+GLContext.viewport+GLViewport.xsize]
		dec dword[eax+GLContext.viewport+GLViewport.ysize]

		stdcall glViewport, 0, 0, [win_x], [win_y]
	.end_f:
	xor eax,eax
	inc eax
	.err_f:
	ret
endp

align 4
proc kosglSwapBuffers uses eax ebx ecx edx esi
	; retrieve the current TinyGLContext
	call gl_get_context
	mov ebx,[eax+GLContext.zb]
	mov ebx,[ebx+ZBuffer.pbuf]
	mov esi,[eax+GLContext.opaque] ;esi = &context.opaque
	mov eax,SF_PUT_IMAGE
	mov ecx,[esi+TinyGLContext.d_x]
	shl ecx,16
	mov cx,word[esi+TinyGLContext.d_y]
	mov edx,[esi+TinyGLContext.x]
	shl edx,16
	mov dx,word[esi+TinyGLContext.y]
	int 0x40
	ret
endp
