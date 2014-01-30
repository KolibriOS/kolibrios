;-----------------------------------------------------------------------------
put_to_clipboard:
; we have allocated memory?
	mov	edi,[clipboard_buf]
	test	edi,edi
	jz	.end
; convert from Tinypad format to the kernel clipboard format
	add	edi,3*4
	mov	esi,[copy_buf]
	mov	ecx,[copy_count]
	cld
;--------------------------------------
@@:
	push	ecx
	lodsd
	mov	ecx,eax
	lodsw
	rep	movsb
	mov	ax,0x0a0d ; EOS (end of string)
	stosw
;	mov	ax,0x0d
;	stosb
	pop	ecx
	dec	ecx
	jnz	@b

	sub	edi,2 ; delete last EOS (0x0a0d)
	xor	eax,eax
	stosb
; building the clipboard slot header
	mov	esi,[clipboard_buf]
	sub	edi,esi
	mov	[esi],edi ; clipboard area size
	xor	eax,eax
	mov	[esi+4],eax ;	; type 'text'
	inc	eax
	mov	[esi+8],eax	; cp866 text encoding
; put slot to the kernel clipboard
	mov	edx,[clipboard_buf]
	mov	ecx,[edx]
	mcall	54,2
; remove unnecessary memory area
	xor	eax,eax
        stdcall mem.ReAlloc,[clipboard_buf],eax
        mov     [clipboard_buf],eax
	xor	eax,eax
	mov	[copy_size],eax
        mov     [copy_count],eax
        stdcall mem.ReAlloc,[copy_buf],eax
        mov     [copy_buf],eax
;--------------------------------------
.end:
	ret
;-----------------------------------------------------------------------------
get_from_clipboard:
	pushad
	mcall	54,0
; no slots of clipboard ?
	test	eax,eax
	jz	.exit
; main list area not found ?	
	inc	eax
	test	eax,eax
	jz	.exit

	sub	eax,2
	mov	ecx,eax
	mcall	54,1
; main list area not found ?
	inc	eax
	test	eax,eax
	jz	.exit
; error ?
	sub	eax,2
	test	eax,eax
	jz	.exit
	
	inc	eax
	mov	[clipboard_buf],eax
; check contents of container
	mov	ebx,[eax+4]
; check for text
	test	ebx,ebx
	jnz	.no_valid_text
	
	mov	ebx,[eax+8]
; check for cp866
	cmp	bl,1
	jz	.yes_valid_text
	
.no_valid_text:
	xor	eax,eax
	mov	[copy_size],eax
	jmp	.remove_area
;--------------------------------------	
.yes_valid_text:
	call	know_number_line_breaks
        mov     [copy_count],ebx
	
	mov	eax,[clipboard_buf]
	sub	esi,eax
	mov	[eax],esi

	shl	ebx,1
	lea	ebx,[ebx*3]

	mov	eax,[clipboard_buf]
	mov	eax,[eax]
	sub	eax,4*3
	add	eax,ebx
        mov     [copy_size],eax	

        stdcall mem.ReAlloc,[copy_buf],eax
        mov     [copy_buf],eax

	call	convert_clipboard_buf_to_copy_buf
; remove unnecessary memory area
.remove_area:
	xor	eax,eax
        stdcall mem.ReAlloc,[clipboard_buf],eax
        mov     [clipboard_buf],eax
;--------------------------------------
.exit:
	popad
	ret
;-----------------------------------------------------------------------------
check_clipboard_for_popup:
	pushad
	mov	[popup_valid_text],0
	mcall	54,0
; no slots of clipboard ?
	test	eax,eax
	jz	.exit
; main list area not found ?	
	inc	eax
	test	eax,eax
	jz	.exit

	sub	eax,2
	mov	ecx,eax
	mcall	54,1
; main list area not found ?
	inc	eax
	test	eax,eax
	jz	.exit
; error ?
	sub	eax,2
	test	eax,eax
	jz	.exit
	
	inc	eax
	mov	[clipboard_buf],eax
; check contents of container
	mov	ebx,[eax+4]
; check for text
	test	ebx,ebx
	jnz	.remove_area
	
	mov	ebx,[eax+8]
; check for cp866
	cmp	bl,1
	jnz	.remove_area

.yes_valid_text:
	mov	[popup_valid_text],1
; remove unnecessary memory area
.remove_area:
	xor	eax,eax
        stdcall mem.ReAlloc,[clipboard_buf],eax
        mov     [clipboard_buf],eax
;--------------------------------------
.exit:
	popad
	ret
;-----------------------------------------------------------------------------
convert_clipboard_buf_to_copy_buf:
	mov	edi,[copy_buf]
	mov	ebx,edi
	add	edi,6
	mov	eax,[clipboard_buf]
	mov	esi,eax
	add	esi,4*3
	mov	ecx,[eax]
	sub	ecx,4*3-1
	xor	edx,edx
	cld
;--------------------------------------
.loop:
	lodsb

	test	al,al
	jz	.end_of_data
	
	cmp	al,0x0d
	je	.check_0x0a
	
	cmp	al,0x0a
	je	.inc_counter

	dec	ecx
	jz	.end_of_data
	
	stosb
	jmp	.loop
;--------------------------------------
.check_0x0a:
	dec	ecx
	jz	.end_of_data
	
	cmp	[esi],byte 0x0a
	jne	@f
	
	lodsb
;--------------------------------------
.inc_counter:
	dec	ecx
	jz	.end_of_data
;--------------------------------------
@@:
	mov	eax,edi
	sub	eax,ebx
	sub	eax,6
	mov	[ebx],eax ; size of current string
	mov	ebx,edi
	add	edi,6
	inc	edx
	jmp	.loop
;--------------------------------------	
.end_of_data:
	mov	eax,edi
	sub	eax,ebx
	sub	eax,6
	mov	[ebx],eax ; size of current string
	sub	edi,[copy_buf]
        mov     [copy_size],edi	
	ret
;-----------------------------------------------------------------------------
know_number_line_breaks:
; to know the number of line breaks
	mov	eax,[clipboard_buf]
	mov	esi,eax
	add	esi,4*3
	mov	ecx,[eax]
	sub	ecx,4*3
	xor	ebx,ebx
	cld
;--------------------------------------
@@:
	lodsb
	
	test	al,al
	jz	.end_of_data_1
	
	cmp	al,0x0d
	je	.check_0x0a
	
	cmp	al,0x0a
	je	.inc_counter
	
	dec	ecx
	jnz	@b
	
	jmp	.end_of_data
;--------------------------------------
.check_0x0a:
	inc	ebx
	dec	ecx
	jz	.end_of_data
	
	cmp	[esi],byte 0x0a
	jne	@b
	
	lodsb
	dec	ecx
	jnz	@b
	
	jmp	.end_of_data
;--------------------------------------	
.inc_counter:
	inc	ebx
	dec	ecx
	jnz	@b
;--------------------------------------	
.end_of_data_1:
	cmp	[esi-2],byte 0x0d
	je	.end_of_data

	cmp	[esi-2],byte 0x0a
	je	.end_of_data

	inc	ebx
	inc	esi
;--------------------------------------	
.end_of_data:
	inc	ebx
	ret
;-----------------------------------------------------------------------------