;---------------------------------------------------------------------
death_of_protagonist_start:
	mov	esi,map_level_game_over
	call	map_level_to_plan_level
.red:
	call	draw_window
;---------------------------------------------------------------------
.still:
	mcall	10

	cmp	eax,1
	je	.red

	cmp	eax,2
	je	.key

	cmp	eax,3
	je	.button
	
	jmp	.still
;---------------------------------------------------------------------
.key:
	mcall	2
	cmp	[extended_key],1
	je	.extended_key

	test	al,al
	jnz	.still

	cmp	ah,0xE0
	jne	.menu_still	; start_level_0

	mov	[extended_key],1
	jmp	.still
;---------------------------------------------------------------------
.extended_key:
	mov	[extended_key],0
	jmp	.menu_still	; start_level_0
;---------------------------------------------------------------------
.menu_still:
	mcall	5,200
	jmp	menu_still
;---------------------------------------------------------------------
.button:
	mcall	17

	cmp	ah,1
	jne	.still
	
	jmp	button.exit
;--------------------------------------