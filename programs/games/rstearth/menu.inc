;---------------------------------------------------------------------
main_menu_start:
	mov	[main_menu_pointer],0
;--------------------------------------		
.red:
	mov	[menu_text_pointers],main_menu_text_pointers
	mov	[text_step_Y],SPRITE_SIZE_Y
	mov	[use_separate_draw_text],1
	call	draw_menu_window
	call	draw_new_pointer
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
	jne	@f

	mov	[extended_key],1
	jmp	.still
;--------------------------------------
@@:
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
	
	call	snd_kick_action
	call	clear_old_pointer
	dec	[main_menu_pointer]
	call	draw_new_pointer
	jmp	.still
;---------------------------------------------------------------------
.arrow_down:
	cmp	[main_menu_pointer],4
	je	.still
	
	call	snd_kick_action
	call	clear_old_pointer
	inc	[main_menu_pointer]
	call	draw_new_pointer
	jmp	.still
;---------------------------------------------------------------------
.enter_to_point:
	cmp	[main_menu_pointer],0	; game
	je	start_level_0
	
	cmp	[main_menu_pointer],1	; settings
	je	settings_menu_start

	cmp	[main_menu_pointer],2	; credits
	je	menu_credits
	
	cmp	[main_menu_pointer],3	; help
	je	menu_help

	cmp	[main_menu_pointer],4	; exit
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
	add	ecx,SPRITE_SIZE_Y*3 - 16
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

	movzx	edx,byte [main_menu_pointer]
	imul	edx,SPRITE_SIZE_Y
	add	edx,SPRITE_SIZE_Y*3 - 16
	add	edx,SPRITE_SIZE_X*2 shl 16
	
	mov	eax,[npc_miku_icons]
	mov	ebp,SPRITE_SIZE_X*SPRITE_SIZE_Y*12
	call	copy_base_8bpp
	
	xor	ebp,ebp
	mcall	65,buffer_area,<SPRITE_SIZE_X,SPRITE_SIZE_Y>,,32
	ret
;---------------------------------------------------------------------
draw_menu_window:
	mcall	12,1
	mcall	48,4
	mov	ecx,100 shl 16 + 644
	add	cx,ax
	mcall	0,<100,649>,,0x74AABBCC,,title
	
	call	clear_screen

	mov	eax,4
	mov	ebx,SPRITE_SIZE_X*3 shl 16 + SPRITE_SIZE_Y*3
	mov	ecx,0x90ffffff
	mov	esi,[menu_text_pointers]
;--------------------------------------	
@@:
	mov	edx,[esi]
	test	edx,edx
	jz	.end

	cmp	[use_separate_draw_text],1
	je	.use_separate_draw_text

	mcall
	jmp	.continue
;--------------------------------------		
.use_separate_draw_text:
	call	draw_font
;--------------------------------------	
.continue:
	add	ebx,[text_step_Y]
	add	esi,4
	jmp	@b
;--------------------------------------		
.end:

	mcall	12,2
	ret
;---------------------------------------------------------------------
clear_screen:	
	xor	edx,edx
	mcall	13,<0,SPRITE_SIZE_X*LEVEL_MAP_SIZE_X>,<0,SPRITE_SIZE_Y*LEVEL_MAP_SIZE_Y>
	ret
;---------------------------------------------------------------------
menu_credits:
	call	snd_kick_action
	mov	[menu_text_pointers],menu_credits_text_pointers
	jmp	menu_help.1
;---------------------------------------------------------------------
menu_help:
	call	snd_kick_action
	mov	[menu_text_pointers],menu_help_text_pointers
;--------------------------------------	
.1:
	mov	[text_step_Y],20
;--------------------------------------	
.red:
	mov	[use_separate_draw_text],0
	call	draw_menu_window
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
	jne	@f

	mov	[extended_key],1
	jmp	.still
;--------------------------------------
@@:
	cmp	ah,57	; space
	je	main_menu_start.red
	
	cmp	ah,28	; enter
	je	main_menu_start.red

	jmp	.still
;---------------------------------------------------------------------
.extended_key:
	mov	[extended_key],0

	jmp	.still
;---------------------------------------------------------------------
.button:
	mcall	17

	cmp	ah,1
	jne	.still
	
	jmp	button.exit
;---------------------------------------------------------------------
settings_menu_start:
	call	snd_kick_action
	mov	al,[main_menu_pointer]
	mov	[settings_menu_pointer],al
	xor	al,al
	mov	[main_menu_pointer],al
;--------------------------------------		
.red:
	mov	[menu_text_pointers],settings_menu_text_pointers
	mov	[text_step_Y],SPRITE_SIZE_Y
	call	prepare_settings_text
	mov	[use_separate_draw_text],1
	call	draw_menu_window
	call	draw_new_pointer
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
	jne	@f

	mov	[extended_key],1
	jmp	.still
;--------------------------------------
@@:
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
	
	call	snd_kick_action
	call	clear_old_pointer
	dec	[main_menu_pointer]
	call	draw_new_pointer
	jmp	.still
;---------------------------------------------------------------------
.arrow_down:
	cmp	[main_menu_pointer],2
	je	.still
	
	call	snd_kick_action
	call	clear_old_pointer
	inc	[main_menu_pointer]
	call	draw_new_pointer
	jmp	.still
;---------------------------------------------------------------------
.enter_to_point:
	cmp	[main_menu_pointer],0	; sounds
	jne	@f
	
	inc	[sounds_flag]
	and	[sounds_flag],1b
	jmp	.red
;--------------------------------------	
@@:
	cmp	[main_menu_pointer],1	; music
	jne	@f
	
	inc	[music_flag]
	and	[music_flag],1b
	jmp	.red
;--------------------------------------	
@@:
	cmp	[main_menu_pointer],2	; exit
	jne	.still
	
	call	snd_kick_action
	mov	al,[settings_menu_pointer]
	mov	[main_menu_pointer],al

	jmp	main_menu_start.red
;---------------------------------------------------------------------
.button:
	mcall	17

	cmp	ah,1
	jne	.still
	
	jmp	button.exit
;---------------------------------------------------------------------
prepare_settings_text:
	mov	eax,[text_on_label]
	cmp	[sounds_flag],1
	je	@f
	
	mov	eax,[text_off_label]
;--------------------------------------
@@:
	mov	[menu_settings_sound_label+7],eax
;--------------------------------------	
	mov	eax,[text_on_label]
	cmp	[music_flag],1
	je	@f
	
	mov	eax,[text_off_label]
;--------------------------------------
@@:
	mov	[menu_settings_music_label+7],eax	
	ret
;---------------------------------------------------------------------