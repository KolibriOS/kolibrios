main_menu_start:
	mov	[main_menu_pointer],0
.red:
	call	draw_main_menu_window
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

	cmp	ah,72	; arrow up
	je	.arrow_up

	cmp	ah,80	; arrow down
	je	.arrow_down

	cmp	ah,17	; w - arrow up
	je	.arrow_up

	cmp	ah,31	; s - arrow down
	je	.arrow_down

	cmp	ah,57	; space
	je	.enter_to_point
	
	cmp	ah,28	; enter
	je	.enter_to_point

	mov	[extended_key],1
	jmp	.still
;---------------------------------------------------------------------
.extended_key:
	mov	[extended_key],0
	cmp	ah,72	; arrow up
	je	.arrow_up

	cmp	ah,80	; arrow down
	je	.arrow_down
	
	jmp	.still
;---------------------------------------------------------------------
.arrow_up:
	cmp	[main_menu_pointer],0
	je	.still
	
	call	clear_old_pointer
	dec	[main_menu_pointer]
	call	draw_new_pointer
	jmp	.still
;---------------------------------------------------------------------
.arrow_down:
	cmp	[main_menu_pointer],4
	je	.still
	
	call	clear_old_pointer
	inc	[main_menu_pointer]
	call	draw_new_pointer
	jmp	.still
;---------------------------------------------------------------------
.enter_to_point:
	cmp	[main_menu_pointer],0
	je	start_level_0
	
	cmp	[main_menu_pointer],4
	je	button.exit
	
	jmp	.still
;---------------------------------------------------------------------
.button:
	mcall	17

	cmp	ah,1
	jne	.still
	
	jmp	button.exit
;---------------------------------------------------------------------
clear_old_pointer:
	movzx	ecx,byte [main_menu_pointer]
	imul	ecx,SPRITE_SIZE_Y
	add	ecx,SPRITE_SIZE_Y*2 + 32
	shl	ecx,16
	mov	cx,SPRITE_SIZE_Y
	mcall	13,<SPRITE_SIZE_X*2,SPRITE_SIZE_X>,,0
	ret
;---------------------------------------------------------------------
draw_new_pointer:
;	movzx	ecx,byte [main_menu_pointer]
;	imul	ecx,SPRITE_SIZE_Y
;	add	ecx,SPRITE_SIZE_Y*2
;	shl	ecx,16
;	mov	cx,SPRITE_SIZE_Y
;	mcall	13,<SPRITE_SIZE_X*1,SPRITE_SIZE_X>,,0xff0000
	
	mov	eax,[npc_miku_icons]
	mov	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y*12
	call	copy_base_8bpp
	
	movzx	edx,byte [main_menu_pointer]
	imul	edx,SPRITE_SIZE_Y
	add	edx,SPRITE_SIZE_Y*2 + 32
	add	edx,SPRITE_SIZE_X*2 shl 16

	xor	ebp,ebp
	mcall	65,buffer_area,<SPRITE_SIZE_X,SPRITE_SIZE_Y>,,32
	ret
;---------------------------------------------------------------------
draw_main_menu_window:
	mcall	12,1
	mcall	48,4
	mov	ecx,100 shl 16 + 644
	add	cx,ax
	mcall	0,<100,649>,,0x74AABBCC,,title
	
	call	clear_screen

	mov	eax,4
	mov	ebx,SPRITE_SIZE_X*3 shl 16 + SPRITE_SIZE_Y*3
	mov	ecx,0x90ffffff
	mov	esi,main_menu_text_pointers
;--------------------------------------	
@@:
	mov	edx,[esi]
	test	edx,edx
	jz	.end

	mcall
	add	ebx,SPRITE_SIZE_Y
	add	esi,4
	jmp	@b
;--------------------------------------		
.end:
	call	draw_new_pointer

	mcall	12,2
	ret
;---------------------------------------------------------------------
clear_screen:	
	xor	edx,edx
	mcall	13,<0,SPRITE_SIZE_X*LEVEL_MAP_SIZE_X>,<0,SPRITE_SIZE_Y*LEVEL_MAP_SIZE_Y>
	ret
;---------------------------------------------------------------------