;---------------------------------------------------------------------
show_tiles_one_iteration:
	mov	[show_tiles_iterations],1
	call	copy_plan_level_to_plan_level_old
	call	show_tiles_1
	call	clear_show_flags_to_plan_level
	call	copy_plan_level_to_plan_level_old
	ret
;---------------------------------------------------------------------
show_tiles:
	mov	[show_tiles_iterations],SPRITE_SIZE_X
;--------------------------------------
.loop:
	call	show_tiles_1
	mcall	5,1
	dec	[show_tiles_iterations]
	jnz	.loop

	call	clear_show_flags_to_plan_level
	call	copy_plan_level_to_plan_level_old
	ret
;---------------------------------------------------------------------
show_tiles_1:
;	DEBUGF	1, "show_tiles_1\n"
	; set start coordinates
	xor	edx,edx
	mov	ebx,LEVEL_MAP_SIZE_Y
	mov	ecx,plan_level
;--------------------------------------
.new_y:
	mov	eax,LEVEL_MAP_SIZE_X
	ror	edx,16
	xor	dx,dx
	rol	edx,16
;--------------------------------------
.new_x:
	push	eax ebx ecx

	mov	ebp,[ecx]
	
	cmp	[draw_all_level],1
	je	.draw_all_level
;;	DEBUGF	1, "Y : %x X: %x cell: %x ebp: %x\n",ebx,eax,ecx,ebp	
	mov	eax,ebp
	rol	eax,8
	test	al,al
	jz	.end_show_buffer

;	cmp	[show_tiles_iterations],1
;	jne	.not_clear
	
;	mov	al,0
;--------------------------------------
;.not_clear:
;	ror	eax,8
;	mov	[ecx],eax
;;	DEBUGF	1, "new cell: %x eax: %x\n",ecx,eax
;--------------------------------------
.draw_all_level:
	and	ebp,0xff
	mov	eax,[base_icons]
	imul	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y ; 8bpp
;	imul	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y*3 ; 24bpp
	call	copy_base_8bpp
	
	mov	eax,[ecx+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4]
	mov	ebp,ecx
	sub	ebp,plan_level
	add	ebp,plan_level_old
	cmp	[ebp+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4],eax
	jne	@f
	
	push	dword [show_tiles_iterations]
	mov	[show_tiles_iterations],1
	call	calculate_new_characters_offsets_and_sizes
	call	analize_and_copy_object
	pop	dword [show_tiles_iterations]
	jmp	.draw_cell
;--------------------------------------
@@:
	
	call	calculate_new_characters_offsets_and_sizes
	call	analize_and_copy_object ; new object moves into the cell.
	cmp	[show_tiles_iterations],1
	je	.draw_cell
;	mov	eax,[ecx+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4]
;	DEBUGF	1, "object id new: %x\n",eax
	sub	ecx,plan_level
	add	ecx,plan_level_old
	
	mov	eax,[ecx]
	shr	eax,16
	test	al,al	; route or static?
	jz	.draw_cell
;	mov	eax,[ecx+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4]
;	DEBUGF	1, "object id old: %x\n",eax
;	cmp	[ecx+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4],eax
;	je	@f	; old and new state of the cell is equal
	
;	DEBUGF	1, "old object moves out of the cell\n"
	call	calculate_old_characters_offsets_and_sizes
	call	analize_and_copy_object	; old object moves out of the cell
;--------------------------------------
.draw_cell:
	xor	ebp,ebp
	mcall	65,buffer_area,<SPRITE_SIZE_X,SPRITE_SIZE_Y>,,32	; 32bpp
;--------------------------------------
.end_show_buffer:
	pop	ecx ebx eax
	
	add	ecx,4
	add	edx,SPRITE_SIZE_X shl 16
	dec	eax
	jnz	.new_x
	
	add	edx,SPRITE_SIZE_Y
	dec	ebx
	jnz	.new_y
;	DEBUGF	1, "show_tiles_1 END\n"
	ret
;---------------------------------------------------------------------
map_level_to_plan_level:
;	mov	esi,map_level_0
;	mov	esi,map_level_game_over
	mov	edi,plan_level
	mov	ecx,LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y
	cld
	rep	movsd
	ret
;---------------------------------------------------------------------
generate_objects_id:
	mov	esi,plan_level
	mov	edi,objects_id
	mov	ecx,LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y
	mov	edx,1
	cld
;--------------------------------------
.loop:
	lodsd
	mov	ebx,eax
	xor	eax,eax
	test	bh,bh
	jz	@f

	mov	eax,edx
	inc	edx	; next object
;--------------------------------------	
@@:
	stosd
	dec	ecx
	jnz	.loop

;	DEBUGF	1, "object id max: %x\n",edx
	ret	
;---------------------------------------------------------------------
copy_plan_level_to_plan_level_old:
	mov	esi,plan_level
	mov	edi,plan_level_old
	mov	ecx,LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*2	; plan_level + objects_id
	cld
	rep	movsd
	ret
;---------------------------------------------------------------------
clear_show_flags_to_plan_level:
	mov	esi,plan_level
	mov	edi,plan_level
	mov	ecx,LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y
	cld
;--------------------------------------
@@:
	lodsd
	ror	eax,24
	mov	al,0
	ror	eax,8
	stosd
	dec	ecx
	jnz	@b

	ret
;---------------------------------------------------------------------
