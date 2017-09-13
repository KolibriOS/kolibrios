
put_to_clipboard:
	mov	edi,[clipboard_buf]
	test	edi,edi
	jz	.end
	add	edi,12
	mov	esi,[copy_buf]
	mov	edx,[copy_count]
	mov	ax,0x0a0d	; End of String
	cld
@@:	; convert format from Tinypad to clipboard
	mov	ecx,[esi]
	add	esi,sizeof.EDITOR_LINE_DATA
	rep movsb
	stosw
	dec	edx
	jnz	@b

	sub	edi,2		; delete last EoS
	xor	eax,eax
	stosb
; build the clipboard slot header
	mov	esi,[clipboard_buf]
	sub	edi,esi
 dec edi
	mov	[esi],edi	; clipboard area size
	xor	eax,eax
	mov	[esi+4],eax	; type 'text'
	inc	eax
	mov	[esi+8],eax	; cp866 text encoding
; put slot to the kernel clipboard
	mov	edx,[clipboard_buf]
	mov	ecx,[edx]
	mcall	54,2
	stdcall mem.Free,[clipboard_buf]
	stdcall mem.Free,[copy_buf]
	xor	eax,eax
	mov	[copy_buf],eax
	mov	[copy_size],eax
	mov	[copy_count],eax
	mov	[clipboard_buf],eax
.end:
	ret
;---------------------------------------------------------------
get_from_clipboard:
	mov	[copy_size],0
	pushad
	mcall	54,0
	test	eax,eax
	jz	.exit	; no slots of clipboard
	inc	eax
	jz	.exit	; main list area not found
	sub	eax,2
	mov	ecx,eax
	mcall	54,1
	inc	eax
	jz	.exit	; main list area not found
	sub	eax,2
	jz	.exit	; error
	inc	eax
	mov	[clipboard_buf],eax
; check contents of container
	mov	ebx,[eax+4]
	test	ebx,ebx
	jnz	.freeMemory	; not text
	mov	ebx,[eax+8]
	dec	ebx
	jnz	.freeMemory	; not cp866
	mov	edi,[clipboard_buf]
	mov	al, 10
	mov	ecx,[edi]
	add	edi,12
	sub	ecx,12
	jbe	.freeMemory
	cmp	byte[edi],0
	jz	.freeMemory
@@:
	dec	ecx
	cmp	byte[edi+ecx],0
	jz	@b
	inc	ecx
	push	ecx
	cld
@@:	; count strings
	repnz scasb
	inc	ebx
	test	ecx,ecx
	jnz	@b
	dec	edi
	cmp	byte[edi],10
	jnz	@f
	inc	ebx
@@:
	mov	[copy_count],ebx
	lea	eax,[ebx*4+ebx+2]
	add	eax,[esp]
	stdcall mem.Alloc,eax
	mov	[copy_buf],eax
	mov	esi,eax
	mov	edi,[clipboard_buf]
	add	edi,12
	pop	ecx
	mov	ebx,ecx
	mov	al, 10
.stringSize:	; convert format from clipboard to Tinypad
	repnz scasb
	sub	ebx,ecx
	mov	edx,edi
	sub	edi,ebx
	dec	ebx
	test	ecx,ecx
	jnz	.stringEnd
.lastString:
	cmp	byte[edi+ebx],10
	jz	.stringEnd
	cmp	byte[edi+ebx],0
	jnz	@f
	dec	ebx
	jmp	.lastString
.stringEnd:
	dec	ebx
	cmp	byte[edi+ebx],13
	jz	.copyString
@@:
	inc	ebx
.copyString:
	mov	[esi],ebx
	add	esi,sizeof.EDITOR_LINE_DATA
	xchg	ebx,ecx
	xchg	esi,edi
	rep movsb
	mov	ecx,ebx
	jcxz	.done
	mov	esi,edi
	mov	edi,edx
	jmp	.stringSize
.done:
	cmp	esi,edx
	jz	@f
	inc	ecx
	mov	[edi],ecx
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	byte[edi],' '
	inc	edi
@@:
	sub	edi,[copy_buf]
	mov	[copy_size],edi
.freeMemory:
	stdcall mem.Free,[clipboard_buf]
	mov	[clipboard_buf],0
.exit:
	popad
	ret
;---------------------------------------------------------------
check_clipboard_for_popup:
	pushad
	mov	[popup_valid_text],0
	mcall	54,0
	test	eax,eax
	jz	.exit	; no slots of clipboard
	inc	eax
	jz	.exit	; main list area not found
	sub	eax,2
	mov	ecx,eax
	mcall	54,1
	inc	eax
	jz	.exit	; main list area not found
	sub	eax,2
	jz	.exit	; error
	inc	eax
	mov	[clipboard_buf],eax
; check contents of container
	mov	ebx,[eax+4]
	test	ebx,ebx
	jnz	.freeMemory	; not text
	mov	ebx,[eax+8]
	dec	ebx
	jnz	.freeMemory	; not cp866
	mov	[popup_valid_text],1
.freeMemory:
	stdcall mem.Free,[clipboard_buf]
	mov	[clipboard_buf],0
.exit:
	popad
	ret
