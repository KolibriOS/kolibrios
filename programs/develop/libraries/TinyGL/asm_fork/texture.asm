;
; Texture Manager
;

align 4
proc find_texture uses ebx ecx, context:dword, h:dword
	mov ebx,[context]
	mov ebx,[ebx+GLContext.shared_state+4] ;ebx = &texture_hash_table
	mov eax,[h]
	and eax,0xff
	shl eax,2
	add eax,ebx ;eax = &context.shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE]

	; [eax] - указатель на текстуру, получаемую через хеш таблицу
	mov ecx,[h] ; ecx - указатель на искомую текстуру
	@@:
		cmp dword[eax],0
		je .no_found
		mov ebx,[eax]
		cmp dword[ebx+offs_text_handle],ecx
		je .found
		mov eax,[ebx+offs_text_next]
		jmp @b
	.no_found:
	xor eax,eax ;ret NULL
	.found:
	ret
endp

align 4
proc free_texture uses eax ebx ecx edx, context:dword, h:dword
	mov edx,[context]

	stdcall find_texture,edx,[h] ;t=find_texture(context,h)
	cmp dword[eax+offs_text_prev],0 ;if (t.prev==NULL)
	jne .else
		mov edx,[edx+GLContext.shared_state+4] ;edx = &context.shared_state.texture_hash_table[0]
		mov ebx,[eax+offs_text_handle]
		and ebx,0xff
		shl ebx,2
		add edx,ebx ;edx = &context.shared_state.texture_hash_table[t.handle % TEXTURE_HASH_TABLE_SIZE]
		mov ebx,[eax+offs_text_next]
		mov [edx],ebx ;*ht=t.next
		jmp @f
	.else:
		mov ebx,[eax+offs_text_prev]
		mov ecx,[eax+offs_text_next]
		mov [ebx+offs_text_next],ecx ;t.prev.next=t.next
	@@:
	cmp dword[eax+offs_text_next],0 ;if (t.next!=NULL)
	je @f
		mov ebx,[eax+offs_text_next]
		mov ecx,[eax+offs_text_prev]
		mov [ebx+offs_text_prev],ecx ;t.next.prev=t.prev
	@@:

	xor ebx,ebx
	mov ecx,[eax+offs_text_images] ;im=&t.images[0]
	.cycle_0: ;for(i=0;i<MAX_TEXTURE_LEVELS;i++)
	cmp ebx,MAX_TEXTURE_LEVELS
	jge .cycle_0_end
		cmp dword[ecx+offs_imag_pixmap],0 ;if (im.pixmap != NULL)
		je @f
			stdcall gl_free,[ecx+offs_imag_pixmap]
		@@:
		add ecx,sizeof.GLImage
		inc ebx
		jmp .cycle_0
	.cycle_0_end:

	stdcall gl_free,eax
	ret
endp

;output:
; eax - указатель на память
align 4
proc alloc_texture uses ebx ecx, context:dword, h:dword

	stdcall gl_zalloc,sizeof.GLTexture

	mov ebx,[context]
	mov ebx,[ebx+GLContext.shared_state+4] ;ebx = &texture_hash_table
	mov ecx,[h]
	and ecx,0xff
	shl ecx,2
	add ecx,ebx ;ecx = &context.shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE]

	mov ebx,[ecx]
	mov [eax+offs_text_next],ebx
	mov dword[eax+offs_text_prev],0 ;NULL
	cmp dword[eax+offs_text_next],0 ;NULL
	je @f
		mov [eax+offs_text_prev],eax
	@@:
	mov [ecx],eax

	mov ebx,[h]
	mov [eax+offs_text_handle],ebx

	ret
endp

align 4
proc glInitTextures uses eax edx, context:dword
	; textures
	mov edx,[context]
	mov dword[edx+GLContext.texture_2d_enabled],0
	stdcall find_texture,edx,0
	mov dword[edx+GLContext.current_texture],eax
	ret
endp

align 4
proc glGenTextures uses eax ebx ecx edx esi, n:dword, textures:dword
;edx - GLTexture *t
	call gl_get_context
	add eax,GLContext.shared_state+4 ;offset texture_hash_table = 4

	xor ebx,ebx ;max=0
	xor ecx,ecx ;i=0
	.cycle_0: ;for(i=0;i<TEXTURE_HASH_TABLE_SIZE;i++)
	cmp ecx,TEXTURE_HASH_TABLE_SIZE
	jge .cycle_0_end
		mov esi,ecx
		shl esi,2
		add esi,[eax]
		mov edx,dword[esi] ;t=context.shared_state.texture_hash_table[i]
		.cycle_1: ;while (t!=NULL)
		or edx,edx
		jz .cycle_1_end
			cmp [edx+offs_text_handle],ebx ;if (t.handle>max)
			jle @f
				mov ebx,[edx+offs_text_handle] ;max=t.handle
			@@:
			mov edx,[edx+offs_text_next] ;t=t.next
			jmp .cycle_1
		.cycle_1_end:
		inc ecx
		jmp .cycle_0
	.cycle_0_end:

	xor ecx,ecx ;i=0
	mov esi,[textures]
	.cycle_2: ;for(i=0;i<n;i++)
	cmp ecx,[n]
	jge .cycle_2_end
		inc ebx
		mov [esi],ebx ;textures[i]=max+i+1
		add esi,4
		inc ecx
		jmp .cycle_2
	.cycle_2_end:
	ret
endp

align 4
proc glDeleteTextures uses eax ebx ecx edx, n:dword, textures:dword
	call gl_get_context
	mov edx,eax
	mov ecx,[textures]

	xor ebx,ebx
	.cycle_0: ;for(i=0;i<n;i++)
	cmp ebx,[n]
	jge .cycle_0_end
		stdcall find_texture,edx,[ecx] ;t=find_texture(context,textures[i])
		or eax,eax ;if (t!=0)
		jz @f
			cmp eax,[edx+GLContext.current_texture] ;if (t==context.current_texture)
			jne .end_1
				stdcall glBindTexture,GL_TEXTURE_2D,0
			.end_1:
			stdcall free_texture, edx,[ecx]
		@@:
		add ecx,4
		inc ebx
		jmp .cycle_0
	.cycle_0_end:
	ret
endp

align 4
proc glopBindTexture uses eax ebx edx, context:dword, p:dword
	mov ebx,[p]
	mov edx,[context]

	cmp dword[ebx+4],GL_TEXTURE_2D
	je @f
	;jne .error
	;cmp dword[ebx+8],0
	;jge @f
	.error:
		stdcall dbg_print,sz_glBindTexture,err_7
	@@:

	mov ebx,[ebx+8] ;ebx = p[2]
	stdcall find_texture, edx,ebx
	or eax,eax ;if(t==NULL)
	jnz @f
		stdcall alloc_texture, edx,ebx
	@@:
	mov [edx+GLContext.current_texture],eax
	ret
endp

align 4
proc glopTexImage2D, context:dword, p:dword
locals
	pixels1 dd ?
	do_free dd ?
	aligned_width dd ?
	aligned_height dd ?
endl
pushad
	mov edi,[p]
	mov eax,[edi+4] ;target=p[1].i
	mov ebx,[edi+8] ;level=p[2].i
	mov ecx,[edi+12] ;components=p[3].i;
	mov edx,[edi+16] ;width=p[4].i;
	mov esi,[edi+20] ;height=p[5].i;

	cmp eax,GL_TEXTURE_2D ;if (param != GL_TEXTURE_2D)
	jne .error
	or ebx,ebx ;if (level != 0)
	jnz .error
	cmp ecx,3 ;if (components != 3)
	jne .error
	cmp dword[edi+24],0 ;if (border != 0)
	jne .error
	cmp dword[edi+28],GL_RGB ;if (format != GL_RGB)
	jne .error
	cmp dword[edi+32],GL_UNSIGNED_BYTE ;if (type != GL_UNSIGNED_BYTE)
	jne .error

	jmp @f
	.error:
		stdcall dbg_print,sz_glTexImage2D,err_8 ;"glTexImage2D: combinaison of parameters not handled"
	@@:

	stdcall gl_getPervPowerOfTwo,edx
	mov [aligned_width],eax
	stdcall gl_getPervPowerOfTwo,esi
	mov [aligned_height],eax

	mov dword[do_free],0
	cmp edx,[aligned_width]
	jne .else
	cmp esi,[aligned_height]
	jne .else
		mov eax,[edi+36]
		mov [pixels1],eax ;pixels1=pixels
		jmp @f
align 4
	.else: ;if (width != aligned_width || height != aligned_height)
		imul eax,[aligned_width]
		imul eax,3
		stdcall gl_malloc, eax
		mov [pixels1],eax ;pixels1 = gl_malloc(aligned_width * aligned_height * 3)
		; no interpolation is done here to respect the original image aliasing !
		stdcall gl_resizeImage, eax,[aligned_width],[aligned_height],[edi+36],edx,esi
		mov dword[do_free],1
		mov edx,[aligned_width]
		mov esi,[aligned_height]
	@@:

	mov ecx,[context]
	mov ecx,[ecx+GLContext.current_texture]
	add ecx,offs_text_images
	imul ebx,sizeof.GLTexture
	add ecx,ebx ;ecx = &context.current_texture.images[level]
	mov [ecx+offs_imag_xsize],edx ;im.xsize=width
	mov [ecx+offs_imag_ysize],esi ;im.ysize=height
	mov ebx,edx
	dec ebx
	shl ebx,ZB_POINT_TEXEL_SIZE
	mov [ecx+offs_imag_s_bound],ebx ;im.s_bound = (unsigned int)(width-1)
	shr ebx,ZB_POINT_TEXEL_SIZE

	mov dword[ecx+offs_imag_xsize_log2],ZB_POINT_TEXEL_SIZE
	or ebx,ebx
	jz .set_l2
	@@:
		dec dword[ecx+offs_imag_xsize_log2]
		shr ebx,1
		or ebx,ebx
		jnz @b
	.set_l2:
	;im.xsize_log2 = ZB_POINT_TEXEL_SIZE-log_2(width)
	dec esi
	shl esi,ZB_POINT_TEXEL_SIZE
	mov [ecx+offs_imag_t_bound],esi ;im.t_bound = (unsigned int)(height-1)
	shr esi,ZB_POINT_TEXEL_SIZE
	inc esi
	cmp dword[ecx+offs_imag_pixmap],0 ;if (im.pixmap!=NULL) 
	je @f
		stdcall gl_free, [ecx+offs_imag_pixmap]
	@@:
if TGL_FEATURE_RENDER_BITS eq 24
	imul edx,esi
	imul edx,3
	stdcall gl_malloc,edx
	mov [ecx+offs_imag_pixmap],eax ;im.pixmap = gl_malloc(width*height*3)
	or eax,eax ;if(im.pixmap)
	jz @f
		mov edi,eax
		mov esi,[pixels1]
		mov ecx,edx
		rep movsb ;memcpy(im.pixmap,pixels1,width*height*3)
	@@:
end if
if TGL_FEATURE_RENDER_BITS eq 32
	mov ebx,edx
	imul edx,esi
	shl edx,2
	stdcall gl_malloc,edx
	mov [ecx+offs_imag_pixmap],eax ;im.pixmap = gl_malloc(width*height*4)
	or eax,eax ;if(im.pixmap)
	jz @f
;gl_convertRGB_to_8A8R8G8B(eax,[pixels1],ebx,esi)
	@@:
end if
	cmp dword[do_free],0 ;if (do_free)
	je @f
		stdcall gl_free, [pixels1]
	@@:
popad
	ret
endp

; TODO: not all tests are done
align 4
proc glopTexEnv uses eax ebx ecx, context:dword, p:dword
	mov ecx,[p]
	mov eax,[ecx+4] ;target=p[1].i
	mov ebx,[ecx+8] ;pname=p[2].i
	mov ecx,[ecx+12] ;param=p[3].i

	cmp eax,GL_TEXTURE_ENV ;if (target != GL_TEXTURE_ENV)
	jne .error
	cmp ebx,GL_TEXTURE_ENV_MODE ;if (pname != GL_TEXTURE_ENV_MODE)
	jne .error
	cmp ecx,GL_DECAL ;if (param != GL_DECAL)
	jne .error

	jmp @f
	.error:
		stdcall dbg_print,sz_glTexParameteri,err_6
	@@:
	ret
endp

; TODO: not all tests are done
align 4
proc glopTexParameter uses eax ebx ecx, context:dword, p:dword
	mov ecx,[p]
	mov eax,[ecx+4] ;target=p[1].i
	mov ebx,[ecx+8] ;pname=p[2].i
	mov ecx,[ecx+12] ;param=p[3].i

	cmp eax,GL_TEXTURE_2D ;if (target != GL_TEXTURE_2D)
	jne .error
	cmp ebx,GL_TEXTURE_WRAP_S
	je @f
	cmp ebx,GL_TEXTURE_WRAP_T
	je @f
		jmp .error
	@@:
	cmp ecx,GL_REPEAT ;if (param != GL_REPEAT)
	jne .error

	jmp @f
	.error:
		stdcall dbg_print,sz_glTexParameteri,err_6
	@@:
	ret
endp

align 4
proc glopPixelStore uses eax ebx, context:dword, p:dword
	mov ebx,[p]
	mov eax,[ebx+4] ;pname=p[1].i
	mov ebx,[ebx+8] ;param=p[2].i

	cmp eax,GL_UNPACK_ALIGNMENT ;if (pname != GL_UNPACK_ALIGNMENT)
	jne .error
	cmp ebx,1 ;if (param != 1)
	jne .error

	jmp @f
	.error:
		stdcall dbg_print,sz_glPixelStorei,err_6
	@@:
	ret
endp
