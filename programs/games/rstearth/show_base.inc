;---------------------------------------------------------------------
;copy_base_24bpp:
;	DEBUGF	1, "copy_base_24bpp\n"
;	push	ecx
;	mov	esi,[eax+28]	; RAW area
;	add	esi,eax
;	add	esi,ebp
;	mov	edi,buffer_area
;	mov	ecx,SPRITE_SIZE_X*SPRITE_SIZE_Y
;	cld
;--------------------------------------
;@@:
;	lodsd
;	and	eax,0xffffff
;	dec	esi
;	stosd
;	dec	ecx
;	jnz	@b
;
;	pop	ecx
;	DEBUGF	1, "copy_base_24bpp end\n"
;	ret
;---------------------------------------------------------------------
copy_base_8bpp:
;	DEBUGF	1, "copy_base_8bpp\n"
	push	ecx
	mov	esi,[eax+28]	; RAW area
	add	esi,eax
	add	esi,ebp

	mov	ebp,[eax+20]	; RAW palette
	add	ebp,eax
	mov	edi,buffer_area
	mov	ecx,SPRITE_SIZE_X*SPRITE_SIZE_Y
	cld
;--------------------------------------
@@:
	lodsb
	movzx	eax,al
	shl	eax,2
	mov	eax,[eax+ebp]
	stosd
	dec	ecx
	jnz	@b

	pop	ecx
;	DEBUGF	1, "copy_base_8bpp end\n"
	ret
;---------------------------------------------------------------------
