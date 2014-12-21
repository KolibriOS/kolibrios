;---------------------------------------------------------------------
key:
	mcall	2
	cmp	[extended_key],1
	je	.extended_key

	test	al,al
	jnz	still

	cmp	ah,0xE0
	jne	@f

	mov	[extended_key],1
	jmp	still
;--------------------------------------
@@:
	cmp	ah,72	; arrow up
	je	.arrow_up

	cmp	ah,80	; arrow down
	je	.arrow_down

	cmp	ah,75	; arrow left
	je	.arrow_left

	cmp	ah,77	; arrow right
	je	.arrow_right

	cmp	ah,17	; w - arrow up
	je	.arrow_up

	cmp	ah,31	; s - arrow down
	je	.arrow_down

	cmp	ah,30	; a - arrow left
	je	.arrow_left

	cmp	ah,32	; d - arrow right
	je	.arrow_right

	cmp	ah,57	; space - kick
	je	.kick	

	jmp	still
;---------------------------------------------------------------------
.extended_key:
	mov	[extended_key],0
	cmp	ah,72	; arrow up
	je	.arrow_up

	cmp	ah,80	; arrow down
	je	.arrow_down

	cmp	ah,75	; arrow left
	je	.arrow_left

	cmp	ah,77	; arrow right
	je	.arrow_right

	jmp	still
;---------------------------------------------------------------------
.arrow_up:
	mov	[protagonist_route],ROUTE_UP
	mov	cx,0x0100+ROUTE_UP
	jmp	.save_motion
;---------------------------------------------------------------------
.arrow_down:
	mov	[protagonist_route],ROUTE_DOWN
	mov	cx,0x0100+ROUTE_DOWN
	jmp	.save_motion
;---------------------------------------------------------------------
.arrow_left:
	mov	[protagonist_route],ROUTE_LEFT
	mov	cx,0x0100+ROUTE_LEFT
	jmp	.save_motion
;---------------------------------------------------------------------
.arrow_right:
	mov	[protagonist_route],ROUTE_RIGHT
	mov	cx,0x0100+ROUTE_RIGHT
;---------------------------------------------------------------------
.save_motion:
	call	calculate_offset
	shl	ebx,2
	add	eax,ebx
	add	eax,plan_level
	mov	ebx,[eax]
	ror	ebx,16
	mov	bx,cx
	rol	ebx,16
	mov	[eax],ebx
;	jmp	still
	jmp	calculate_action_for_all.2
;---------------------------------------------------------------------
.kick:
	mov	eax,[protagonist_route]
	cmp	al,ROUTE_UP
	jne	@f
	
	cmp	[protagonist_position.y],0
	je	calculate_action_for_all.1	;still
	
	call	protagonist_kick_action
	mov	eax,[protagonist_position.y]
	dec	eax
	call	calculate_offset.1
	jmp	.calculate_protagonist_kick_brick
;--------------------------------------	
@@:
	cmp	al,ROUTE_DOWN
	jne	@f

	cmp	[protagonist_position.y],LEVEL_MAP_SIZE_Y-1
	je	calculate_action_for_all.1	;still

	call	protagonist_kick_action
	mov	eax,[protagonist_position.y]
	inc	eax
	call	calculate_offset.1
	jmp	.calculate_protagonist_kick_brick
;--------------------------------------	
@@:
	cmp	al,ROUTE_LEFT
	jne	@f

	cmp	[protagonist_position.x],0
	je	calculate_action_for_all.1	;still

	call	protagonist_kick_action
	call	calculate_offset
	dec	ebx
	jmp	.calculate_protagonist_kick_brick
;--------------------------------------	
@@:
	cmp	al,ROUTE_RIGHT
	jne	still
	
	cmp	[protagonist_position.x],LEVEL_MAP_SIZE_X-1
	je	calculate_action_for_all.1	;still	
	
	call	protagonist_kick_action
	call	calculate_offset
	inc	ebx
;--------------------------------------
.calculate_protagonist_kick_brick:
	shl	ebx,2
	add	eax,ebx
	add	eax,plan_level
	mov	ebx,[eax]
	
	cmp	bh,OBJECT_FINAL_MONSTER	;OBJECT_PROTAGONIST
	jbe	.kill

	cmp	bh,OBJECT_WHITE_BRICK
	je	.white_brick

	cmp	bh,OBJECT_RED_BRICK ; red brick?
	jne	still

	and	ebx,0x00ff00ff
	add	ebx,0x01000000 + RED_BRICK_CRASH_1 shl 8	;  reb brick crash 1
	mov	[eax],ebx
	push	eax ebx
;        DEBUGF  1, "80 - reb brick crash 10\n"	
	call	show_tiles_one_iteration
	mcall	5,5
	pop	ebx eax

	and	ebx,0x00ff00ff
	add	ebx,0x01000000 + RED_BRICK_CRASH_2 shl 8	; reb brick crash 2
	mov	[eax],ebx
	push	eax ebx
;        DEBUGF  1, "81 - reb brick crash 20\n"
	call	show_tiles_one_iteration
	mcall	5,5
	pop	ebx eax

	and	ebx,0x00ff00ff	
	add	ebx,0x01000000
	mov	[eax],ebx
;        DEBUGF  1, "81 - reb brick crash 30\n"
;	call	show_tiles_one_iteration
	jmp	calculate_action_for_all.2	;still
;--------------------------------------	
.kill:
	and	ebx,0x000000ff
	add	ebx,0x01000000
	mov	[eax],ebx
;	call	show_tiles_one_iteration
	jmp	calculate_action_for_all.2
;--------------------------------------	
.white_brick:
	ror	ebx,16
	xor	bl,bl
	mov	bh,1
	add	ebx,[protagonist_route]
	rol	ebx,16
	mov	[eax],ebx
	jmp	calculate_action_for_all.2
;---------------------------------------------------------------------
calculate_action_for_all:
	call	show_tiles_one_iteration
;--------------------------------------
.1:
	call	actions_for_all_cell
;--------------------------------------
.2:
	call	show_tiles_one_iteration
	jmp	still
;---------------------------------------------------------------------
calculate_offset:
	mov	eax,[protagonist_position.y]
;--------------------------------------
.1:
	imul	eax,LEVEL_MAP_SIZE_X*4
	mov	ebx,[protagonist_position.x]
	ret
;---------------------------------------------------------------------
protagonist_kick_action:
	call	snd_kick_action
	add	[protagonist_route],4
	call	show_protagonist_1
	sub	[protagonist_route],4
	call	show_protagonist_1
	ret
;---------------------------------------------------------------------
calculate_protagonist_vs_brick:
	shl	ebx,2
	add	eax,ebx
	add	eax,plan_level
	mov	ebx,[eax]
;	cmp	bh,4 ; brick?
	test	bh,bh ; empty?
	jnz	@f
	
	ret
;--------------------------------------
@@:
	add	esp,4
	jmp	still
;---------------------------------------------------------------------
show_protagonist_1:
	mov	ecx,0x01000000 + OBJECT_PROTAGONIST shl 8
	call	calculate_protagonist_position
;	call	show_tiles_one_iteration
;	call	actions_for_all_cell
	call	show_tiles_one_iteration	;show_tiles
	mcall	5,10
	ret
;---------------------------------------------------------------------
calculate_protagonist_position:
	call	calculate_offset
	shl	ebx,2
	add	eax,ebx
	add	eax,plan_level
	mov	ebx,[eax]
	and	ebx,0x00ff00ff
	add	ebx,ecx
	mov	[eax],ebx
	ret
;---------------------------------------------------------------------
