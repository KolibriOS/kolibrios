;
; functions for calculating specular color (glare)
;

align 4
proc calc_buf uses ebx ecx, buf:dword, shininess:dword
locals
	val dd ? ;float
	f_inc dd ? ;float
endl
	mov dword[f_inc],SPECULAR_BUFFER_SIZE
	mov ebx,[buf]
	add ebx,GLSpecBuf.buf
	mov dword[ebx],0.0 ;buf.buf[0] = 0.0
	xor ecx,ecx
	fld1
	fidiv dword[f_inc]
	fstp dword[f_inc] ;f_inc = 1.0f/SPECULAR_BUFFER_SIZE
	mov dword[val],0.0
align 4
	.cycle_0: ;for (i = 1; i < SPECULAR_BUFFER_SIZE; i++)
	inc ecx
	cmp ecx,SPECULAR_BUFFER_SIZE
	jge @f
		fld dword[shininess]
		fld dword[val]
		fadd dword[f_inc]
		fst dword[val] ;val += f_inc
		;need to calculate pow(val, shininess)

		fyl2x ;the FPU stack now contains: st0=z=y*log2(x):
		;now we count 2**z:
		fld st0 ;copy z
		frndint
		fsubr st0,st1  ;st1=z, st0=z-trunc(z)
		f2xm1  ;st1=z, st0=2**(z-trunc(z))-1
		fld1
		faddp  ;st1=z, st0=2**(z-trunc(z))
		fscale ;st1=z, st0=(2**trunc(z))*(2**(z-trunc(z)))=2**t
		fxch st1
		fstp st ;the result remains on the top of the stack st0

		add ebx,4
		fstp dword[ebx] ;buf.buf[i] = pow(val, shininess)
	jmp .cycle_0
	@@:
	mov dword[ebx+4],1.0
	ret
endp

align 4
proc specbuf_get_buffer uses ebx ecx edx, context:dword, shininess_i:dword, shininess:dword
locals
	found dd ? ;GLSpecBuf *
	oldest dd ? ;GLSpecBuf *
endl
	mov edx,[context]
	mov eax,[edx+GLContext.specbuf_first]
	mov [found],eax
	mov [oldest],eax
	mov ebx,[shininess_i]
	.cycle_0:
	or eax,eax ;while (found)
	jz @f
	cmp [eax+GLSpecBuf.shininess_i],ebx ;while (found.shininess_i != shininess_i)
	je @f
		mov ecx,[oldest]
		mov ecx,[ecx+GLSpecBuf.last_used]
		cmp [eax+GLSpecBuf.last_used],ecx ;if (found.last_used < oldest.last_used)
		jge .end_0
			mov [oldest],eax ;oldest = found
		.end_0:
		mov eax,[eax+GLSpecBuf.next] ;found = found.next
		jmp .cycle_0
	@@:
	cmp dword[found],0 ;if (found) /* hey, found one! */
	je @f
		mov eax,[found]
		mov ecx,[edx+GLContext.specbuf_used_counter]
		mov [eax+GLSpecBuf.last_used],ecx ;found.last_used = context.specbuf_used_counter
		inc dword[edx+GLContext.specbuf_used_counter]
		jmp .end_f ;return found
	@@:
	cmp dword[oldest],0 ;if (oldest == NULL || context.specbuf_num_buffers < MAX_SPECULAR_BUFFERS)
	je @f
	cmp dword[edx+GLContext.specbuf_num_buffers],MAX_SPECULAR_BUFFERS
	jge .end_1
	@@:
		; create new buffer
	stdcall gl_malloc, sizeof.GLSpecBuf
	or eax,eax
	jnz @f
;gl_fatal_error("could not allocate specular buffer")
	@@:
	inc dword[edx+GLContext.specbuf_num_buffers]
	mov ecx,[edx+GLContext.specbuf_first]
	mov [eax+GLSpecBuf.next],ecx
	mov [edx+GLContext.specbuf_first],eax
	mov ecx,[edx+GLContext.specbuf_used_counter]
	mov [eax+GLSpecBuf.last_used],ecx
	inc dword[edx+GLContext.specbuf_used_counter]
	mov [eax+GLSpecBuf.shininess_i],ebx
	stdcall calc_buf, eax,dword[shininess]
	jmp .end_f
	.end_1:
	; overwrite the lru buffer
	;tgl_trace("overwriting spec buffer :(\n");
	mov eax,[oldest]
	mov [eax+GLSpecBuf.shininess_i],ebx
	mov ecx,[edx+GLContext.specbuf_used_counter]
	mov [eax+GLSpecBuf.last_used],ecx
	inc dword[edx+GLContext.specbuf_used_counter]
	stdcall calc_buf, eax,dword[shininess]
	.end_f:
	ret
endp
