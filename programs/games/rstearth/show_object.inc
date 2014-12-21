;---------------------------------------------------------------------
calculate_new_characters_offsets_and_sizes:
	mov	eax,[ecx]
	shr	eax,16
	and	eax,0xff
;--------------------------------------
	cmp	eax,ROUTE_UP
	jne	@f
	
	xor	eax,eax
	mov	[characters_offset_y],eax
	mov	[characters_offset_x],eax
	mov	[buffer_offset_x],eax
	
	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[buffer_offset_y],eax
	
	mov	eax,SPRITE_SIZE_Y
	sub	eax,[buffer_offset_y]
	mov	[characters_size_y],eax
	
	mov	eax,SPRITE_SIZE_X
	mov	[characters_size_x],eax
	jmp	.ready
;--------------------------------------
@@:
	cmp	eax,ROUTE_DOWN
	jne	@f
	
	xor	eax,eax
	mov	[characters_offset_x],eax
	mov	[buffer_offset_y],eax
	mov	[buffer_offset_x],eax

	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[characters_offset_y],eax
	
	mov	eax,SPRITE_SIZE_Y
	sub	eax,[characters_offset_y]
	mov	[characters_size_y],eax

	mov	eax,SPRITE_SIZE_X
	mov	[characters_size_x],eax
	jmp	.ready
;--------------------------------------
@@:
	cmp	eax,ROUTE_LEFT
	jne	@f	
	
	xor	eax,eax
	mov	[characters_offset_y],eax
	mov	[characters_offset_x],eax
	mov	[buffer_offset_y],eax

	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[buffer_offset_x],eax
	
	mov	eax,SPRITE_SIZE_X
	sub	eax,[buffer_offset_x]
	mov	[characters_size_x],eax
	
	mov	eax,SPRITE_SIZE_Y
	mov	[characters_size_y],eax
	jmp	.ready	
;--------------------------------------
@@:
	xor	eax,eax
	mov	[characters_offset_y],eax
	mov	[buffer_offset_y],eax
	mov	[buffer_offset_x],eax

	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[characters_offset_x],eax
	
	mov	eax,SPRITE_SIZE_X
	sub	eax,[characters_offset_x]
	mov	[characters_size_x],eax

	mov	eax,SPRITE_SIZE_Y
	mov	[characters_size_y],eax
;--------------------------------------
.ready:
	ret
;---------------------------------------------------------------------
calculate_old_characters_offsets_and_sizes:
	mov	eax,[ecx]
	shr	eax,16
	and	eax,0xff
;--------------------------------------
	cmp	eax,ROUTE_UP
	jne	@f
	
	xor	eax,eax	
	mov	[characters_offset_x],eax
	mov	[buffer_offset_y],eax
	mov	[buffer_offset_x],eax

	mov	eax,SPRITE_SIZE_Y
	sub	eax,[show_tiles_iterations]
	inc	eax
	mov	[characters_offset_y],eax
	
	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[characters_size_y],eax
	
	mov	eax,SPRITE_SIZE_X
	mov	[characters_size_x],eax
	jmp	.ready
;--------------------------------------
@@:
	cmp	eax,ROUTE_DOWN
	jne	@f
	
	xor	eax,eax
	mov	[characters_offset_x],eax
	mov	[characters_offset_y],eax
	mov	[buffer_offset_x],eax

	mov	eax,SPRITE_SIZE_Y
	sub	eax,[show_tiles_iterations]
	inc	eax
	mov	[buffer_offset_y],eax
	
	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[characters_size_y],eax

	mov	eax,SPRITE_SIZE_X
	mov	[characters_size_x],eax
	jmp	.ready
;--------------------------------------
@@:
	cmp	eax,ROUTE_LEFT
	jne	@f	
	
	xor	eax,eax
	mov	[characters_offset_y],eax
	mov	[buffer_offset_y],eax
	mov	[buffer_offset_x],eax

	mov	eax,SPRITE_SIZE_X
	sub	eax,[show_tiles_iterations]
	inc	eax
	mov	[characters_offset_x],eax
	
	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[characters_size_x],eax
	
	mov	eax,SPRITE_SIZE_Y
	mov	[characters_size_y],eax
	jmp	.ready	
;--------------------------------------
@@:
	xor	eax,eax
	mov	[characters_offset_y],eax
	mov	[characters_offset_x],eax
	mov	[buffer_offset_y],eax

	mov	eax,SPRITE_SIZE_X
	sub	eax,[show_tiles_iterations]
	inc	eax
	mov	[buffer_offset_x],eax
	
	mov	eax,[show_tiles_iterations]
	dec	eax
	mov	[characters_size_x],eax

	mov	eax,SPRITE_SIZE_Y
	mov	[characters_size_y],eax
;--------------------------------------
.ready:
	ret
;---------------------------------------------------------------------
analize_and_copy_object:
;	DEBUGF	1, "analize_and_copy_object\n"
	pusha
	mov	ebp,[ecx]
;	DEBUGF	1, "EBP: %x ECX: %x\n",ebp, ecx
	shr	ebp,8
	and	ebp,0xff
	test	ebp,ebp
	jz	.no_characters

	cmp	ebp,OBJECT_DEATH
	jnz	@f

	mov	ebp,[ecx]
	shr	ebp,16
	and	ebp,0xff
	
	mov	eax,[npc_death_icons]
	jmp	.copy_characters_1
;--------------------------------------
@@:
	cmp	ebp,OBJECT_SKELETON
	jnz	@f
	
	mov	ebp,[ecx]
	shr	ebp,16
	and	ebp,0xff

	mov	eax,[npc_skeleton_icons]
	jmp	.copy_characters_1
;--------------------------------------
@@:
	cmp	ebp,OBJECT_IFRIT
	jnz	@f
	
	mov	ebp,[ecx]
	shr	ebp,16
	and	ebp,0xff

	mov	eax,[npc_ifrit_icons]
	jmp	.copy_characters_1
;--------------------------------------
@@:
	cmp	ebp,OBJECT_BARRET
	jnz	@f
	
	mov	ebp,[ecx]
	shr	ebp,16
	and	ebp,0xff

	mov	eax,[npc_barret_icons]
	jmp	.copy_characters_1
;--------------------------------------
@@:
	cmp	ebp,OBJECT_PROTAGONIST
	jnz	@f

	mov	eax,[npc_miku_icons]
	mov	ebp,[protagonist_route]
	cmp	ebp,5
	jb	.copy_characters_1
	
	sub	ebp,4
	jmp	.copy_characters_kick
;--------------------------------------
@@:
	cmp	ebp,OBJECT_RED_BRICK
	jnz	@f
	
	mov	eax,[red_brick_icons]
	mov	ebp,1
	jmp	.copy_characters
;--------------------------------------
@@:
	cmp	ebp,OBJECT_WHITE_BRICK
	jnz	@f
	
	mov	eax,[white_brick_icons]
	mov	ebp,1
	jmp	.copy_characters
;--------------------------------------
@@:
	cmp	ebp,RED_BRICK_CRASH_1	; 0x80 - reb brick crash 1
	jnz	@f
;	DEBUGF	1, "reb brick crash 11\n"	
	mov	eax,[red_brick_icons]
	mov	ebp,2
	jmp	.copy_characters
;--------------------------------------
@@:
	cmp	ebp,RED_BRICK_CRASH_2	; 0x81 - reb brick crash 2
	jnz	.no_characters
;	DEBUGF	1, "reb brick crash 21\n"		
	mov	eax,[red_brick_icons]
	mov	ebp,3
;--------------------------------------
.copy_characters:
	dec	ebp
	imul	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y; 8bpp
	call	copy_characters_with_offset
;--------------------------------------
.no_characters:
;	DEBUGF	1, "analize_and_copy_object END\n"
	popa
	ret
;--------------------------------------
.copy_characters_kick:
	dec	ebp
	imul	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y; 8bpp
	add	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y*4*4 ; 8 bpp * 4 frames * 4 route
	call	copy_characters_with_offset
	jmp	.no_characters
;--------------------------------------
.copy_characters_1:
	dec	ebp
	imul	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y*4 ; 8 bpp * 4 frames
	mov	ebx,[show_tiles_iterations]
	dec	ebx
	shr	ebx,4 ; divider 16
	imul	ebx,SPRITE_SIZE_X*SPRITE_SIZE_Y ; 8bpp
	add	ebp,ebx
	call	copy_characters_with_offset
	jmp	.no_characters
;---------------------------------------------------------------------
copy_characters_with_offset:
;	DEBUGF	1, "copy_characters_with_offset\n"
	pusha	;	ecx

	mov	esi,[eax+28]	; data offset
	add	esi,eax
	add	esi,ebp
	
	mov	ebp,[eax+20]	; palette offset
	add	ebp,eax
; add Y offset to source
	mov	eax,[characters_offset_y]
	imul	eax,SPRITE_SIZE_X	; 8bpp
	add	esi,eax
; add X offset to source
	mov	eax,[characters_offset_x]
	add	esi,eax		; 8bpp

	mov	edi,buffer_area
; add Y offset to destination
	mov	eax,[buffer_offset_y]
	imul	eax,SPRITE_SIZE_X*4	; 32bpp
	add	edi,eax
; add X offset to destination
	mov	eax,[buffer_offset_x]
	shl	eax,2	; 32bpp
	add	edi,eax

	mov	edx,[characters_size_y]
	mov	ecx,[characters_size_x]
	cld
;--------------------------------------
.y:
	push	ecx esi edi
;--------------------------------------
.x:
	lodsb
	test	al,al
	jz	.transparently
	
	and	eax,0xff
	shl	eax,2
	mov	eax,[ebp+eax] ; palette value
	mov	[edi],eax
;--------------------------------------
.transparently:
	add	edi,4
	dec	ecx
	jnz	.x
	
	pop	edi esi ecx
	add	esi,SPRITE_SIZE_X	; 8bpp
	add	edi,SPRITE_SIZE_X*4	; 32bpp
	dec	edx
	jnz	.y
	
	popa
	ret
;---------------------------------------------------------------------
