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
	xor eax,eax

	mov ecx,[xsize_ptr] ; ecx = &xsize
	mov edi,[ecx]       ; edi =  xsize
	mov esi,[ysize_ptr] ; esi = &ysize
	mov esi,[esi]       ; esi =  ysize

	; we ensure that xsize and ysize are multiples of 2 for the zbuffer.
	; TODO: find a better solution
	and edi, not 3
	and esi, not 3

	cmp edi,0
	jne @f
	cmp esi,0
	jne @f
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
	mov [edx+4],edi
	mov [edx+12],edi ;d_x = xsize
	mov [edx+8],esi
	mov [edx+16],esi ;d_y = ysize

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
		mov [ebx+20],ecx ;ctx.x = win_x0
		mov ecx,[win_y0]
		mov [ebx+24],ecx ;ctx.y = win_y0
		mov ecx,[win_x]
		mov [ebx+12],ecx ;ctx.d_x = win_x
		mov ecx,[win_y]
		mov [ebx+16],ecx ;ctx.d_y = win_y

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
		dec dword[eax+GLContext.viewport+offs_vpor_xsize]
		dec dword[eax+GLContext.viewport+offs_vpor_ysize]

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
	mov ebx,[ebx+offs_zbuf_pbuf]
	mov esi,[eax+GLContext.opaque] ;esi = &context.opaque
	mov eax,7
	mov ecx,[esi+12] ;d_x
	shl ecx,16
	mov cx,[esi+16] ;d_y
	mov edx,[esi+20] ;x
	shl edx,16
	mov dx,[esi+24] ;y
	int 0x40
	ret
endp
