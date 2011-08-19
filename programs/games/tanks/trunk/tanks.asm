;*************************************************************
;*GAME TANKS CRETED BY ANDREY IGNATYEV AKA ANDREW_PROGRAMMER *
;*********************/8/2005*********************************
; version:	1.15
; last update:  19/08/2011
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      1) Checking for "rolled up" window
;               2) Code optimization
;               3) Clearing source
;---------------------------------------------------------------------
;Lipatov Kirill aka Leency /07/2011
;dunkaist /07/2011
;v1.1
;----------------------------------------------------------
	use32
	org	0x0
	db	'MENUET01'
	dd	0x1
	dd	START
	dd	I_END
	dd	0x4000+640*400*3+50*20*20*3+512+640+1+24*20*20*3+1
	dd	0x4000
	dd	0x0
	dd	0x0
;----------------------------------------------------------
include 'lang.inc'
include '../../../macros.inc'
;----------------------------------------------------------	
START:
	mcall	40,111b
;----------------------------------------------------------
;---------load all sprites from arrays to memory-----------
;----------------------------------------------------------
	and	[number_sprite],0
	xor	eax,eax
nextfile:
	mov	ebx,[spisok+4*eax]
	mov	ecx,50
	mov	esi,ebx
	add	esi,2
	mov	edi,0x4000+(640*400*3)+(50*20*20*3)+10
	rep	movsb	;copy palitra to memory
	mov	esi,ebx
	add	esi,52
	xor	ecx,ecx
	mov	cl,byte [ebx]
	mov	edi,0x4000+(640*400*3)+(50*20*20*3)+512
	push	eax
	call	unpakin
	pop	eax
	mov	ecx,20*20
	mov	esi,0x4000+(640*400*3)+(50*20*20*3)+512
	mov	edi,[number_sprite]
	imul	edi,3*20*20
	add	edi,0x4000+(640*400*3)
unp:
	xor	ebx,ebx
	mov	bl,byte[esi]
	lea	ebx,[ebx+ebx*2]
	add	ebx,0x4000+(640*400*3)+(50*20*20*3)+10
	mov	edx,[ebx]
	mov	[edi],edx
	add	esi,1
	add	edi,3
	dec	ecx
	jnz	unp
	inc	[number_sprite]
	inc	eax
	cmp	[number_sprite],26	;total number of pictures equal 26
	jne	nextfile
;----------------------------------------------------------
;------rotate sprites of tanks to pi/2---------------------
;----------------------------------------------------------
	mov	[sp_pos],0x4000+(640*400*3)+18*20*20*3
	and	[num_blocks],0
next_block_rotate:
	mov	[number_sprite],0
next_sprite_rotate:
	mov	[x],19
	and	[y],0
rotate_90:
	mov	ebx,[number_sprite]
	mov	esi,[y]
	imul	esi,60
	imul	ebx,1200
	add	esi,ebx	;esi=number_sprite*1200+y*60
	add	esi,dword [sp_pos]
	mov	edi,0x4000+(640*400*3)+(50*20*20*3)
	mov	ecx,15
	rep	movsd
	mov	edi,[number_sprite]
	imul	edi,1200
	add	edi,dword [sp_pos]
	add	edi,8*20*20*3
	mov	ebx,[x]
	lea	ebx,[ebx+2*ebx]
	add	edi,ebx
	mov	esi,0x4000+(640*400*3)+(50*20*20*3)
	mov	ecx,20*3
vertical:
	xor	eax,eax
	mov	al,byte[esi+2]
	mov	[edi+2],al
	xor	eax,eax
	mov	ax,word[esi]
	mov	[edi],ax
	add	edi,60
	add	esi,3
	sub	ecx,3
	jnz	vertical
	dec	[x]
	inc	[y]
	cmp	[x],-1
	jne	rotate_90
	inc	[number_sprite]
	cmp	[number_sprite],9
	jne	next_sprite_rotate
	add	[sp_pos],8*20*20*3
	inc	[num_blocks]
	cmp	[num_blocks],3
	jne	next_block_rotate
;---------------------------------------------------------
restart_level:
	call	drawwin
;Start game(demo scen)
	mov	[y],18
@1:
	mov	[x],30
@2:
	mov	esi,[x]
	mov	edi,[y]
	imul	esi,20*3
	imul	edi,20*(640*3)
	add	edi,esi
	add	edi,0x4000
	mov	esi,0x4000+(640*400*3)+1200
	mov	[counter],20
	mov	eax,esi
	mov	ebx,edi
@3:
	mov	esi,eax
	mov	edi,ebx
	mov	ecx,15
	rep	movsd
	add	eax,20*3
	add	ebx,640*3
	dec	[counter]
	jnz	@3
	dec	[x]
	jnz	@2
	dec	[y]
	jnz	@1
;draw script ******TANKS*******
	mov	eax,54
@11:
	mov	esi,5
	mov	edi,80
	xor	ebx,ebx
	xor	edx,edx
	mov	bl,byte[TANKS_script+2*eax]	;dx
	mov	dl,byte[TANKS_script+2*eax+1]	;dy
	imul	ebx,20
	imul	edx,20
	add	esi,ebx
	add	edi,edx
	lea	esi,[esi+2*esi]
	lea	edi,[edi+2*edi]
	imul	edi,640
	add	edi,esi
	add	edi,0x4000
	mov	esi,0x4000+(640*400*3)
	mov	ebx,esi
	mov	edx,edi
	mov	[counter],20
@22:
	mov	esi,ebx
	mov	edi,edx
	mov	ecx,15
	rep	movsd
	add	ebx,20*3
	add	edx,640*3
	dec	[counter]
	jnz	@22
	dec	eax
	cmp	eax,-1
	jne	@11
	mcall	7,0x4000,<640,400>,<0,20>
	call	menu
;----------------------------
new_level:
	call	paint_area	;drawwin
	mov	[SpriteInfo+72+8],dword 100	;Xo
	mov	[SpriteInfo+72+36],dword 100	;Yo
	mov	edx,[number_level]
	call	load_level
;--------------------------
	and	[strike_action],0
	and	[bazas],0
	and	[y],0
	mov	eax,8
next_y:
	and	[x],0
next_x:
	mov	esi,[y]
	shl	esi,5
	add	esi,[x]
	add	esi,0x4000+(640*400*3)+(50*20*20*3)+512
	xor	ebx,ebx
	mov	bl,byte[esi]
	cmp	bl,7
	jne	no_baza
	mov	ecx,[x]
	mov	edx,[y]
	inc	ecx
	inc	[bazas]
	add	eax,4
	imul	ecx,20
	imul	edx,20
	mov	[SpriteInfo+9*eax+0],dword 3
	mov	[SpriteInfo+9*eax+4],dword 1
	mov	[SpriteInfo+9*eax+8],dword ecx
	mov	[SpriteInfo+9*eax+12],dword edx
	mov	[SpriteInfo+9*eax+16],dword 5
	mov	[SpriteInfo+9*eax+28],dword -5
	mov	edx,eax
	shr	edx,2
	mov	[LifesTanks+edx],byte 3	;three lifes
	mov	[PulyTanks+edx],byte 1	;can draw animation of "puly"
no_baza:
	inc	[x]
	cmp	[x],32
	jne	next_x
	inc	[y]
	cmp	[y],20
	jne	next_y
	mov	[LifesPlayer],3*4
	and	[score],0
;----------------------------------------------------------
;--------------------main loop-----------------------------
;----------------------------------------------------------
maincycle:
;save fon
	mov	eax,23*4
SaveFonForSprites:
	mov	ebx,[SpriteInfo+9*eax+4]
	test	ebx,ebx	;if ebx<>0 then can save
	jz	NoSaveFon
	mov	esi,[SpriteInfo+9*eax+8]	;cordinat x
	mov	edi,[SpriteInfo+9*eax+12]	;cordinat y
	mov	edx,eax
	shr	edx,2
	call	SaveFon
NoSaveFon:
	sub	eax,4
	cmp	eax,-4
	jne	SaveFonForSprites
;put sprites
	mov	eax,23*4
PutSprites:
	mov	ebx,[SpriteInfo+9*eax+4]
	test	ebx,ebx
	jz	NoPutSprite
	and	ebx,10000b
	test	ebx,ebx	;move to transparent shablon?
	jnz	NoPutSprite
	mov	esi,[SpriteInfo+9*eax+8]	;x
	mov	edi,[SpriteInfo+9*eax+12]	;y
	mov	edx,[SpriteInfo+9*eax+0]	;number sprite in sprites table
	shl	edx,2
	add	edx,[SpriteInfo+9*eax+24]	;edx=4*NumSprites+phas
	call	PutSprite
NoPutSprite:
	sub	eax,4
	cmp	eax,-4
	jne	PutSprites
;put image
	call	clock
	mov	ebx,[time]
	sub	ebx,[time_frame_old]
	cmp	ebx,5
	jl	NoPutScreen
	mov	ebx,[time]
	mov	[time_frame_old],ebx
	mcall	7,0x4000,<640,400>,<0,20>
NoPutScreen:
;put fon
	mov	eax,23*4
PutFonForSprites:
	mov	ebx,[SpriteInfo+9*eax+4]
	test	ebx,ebx
	jz	NoPutFon
	mov	esi,[SpriteInfo+9*eax+8]
	mov	edi,[SpriteInfo+9*eax+12]
	mov	edx,eax
	shr	edx,2
	call	PutFon
NoPutFon:
	sub	eax,4
	cmp	eax,-4
	jne	PutFonForSprites
;change phas(if need)
	mov	eax,23*4
ChangePhasSprites:
	push	eax
	call	clock
	pop	eax
	mov	ebx,[SpriteInfo+9*eax+16]	;time of animation
	mov	ecx,[SpriteInfo+9*eax+20]	;time of last frame
	mov	edx,[time]
	sub	edx,ecx
	cmp	edx,ebx
	jl	no_change_phas
	mov	ebx,[time]
	mov	[SpriteInfo+9*eax+20],ebx	;save new time
;------------------------------
	mov	esi,[SpriteInfo+9*eax+8]
	mov	edi,[SpriteInfo+9*eax+12]
	mov	ecx,[SpriteInfo+9*eax+28]
	mov	edx,[SpriteInfo+9*eax+32]
	add	esi,ecx	;x=x+dx
	add	edi,edx	;y=y+dy
	mov	[SpriteInfo+9*eax+8],dword esi
	mov	[SpriteInfo+9*eax+12],dword edi
;-------------------------------
	mov	edx,[SpriteInfo+9*eax+24]	;phas
	add	edx,1
	cmp	edx,4
	jne	no_limit_phas
	xor	edx,edx
	mov	[SpriteInfo+9*eax+24],dword edx
	mov	[SpriteInfo+9*eax+4],dword edx
	jmp	no_change_phas
no_limit_phas:
	mov	[SpriteInfo+9*eax+24],dword edx
no_change_phas:
	sub	eax,4
	cmp	eax,-4
	jne	ChangePhasSprites
;-------------------------------------------------
;----------------keys-----------------------------
;-------------------------------------------------
	call	pause_cicle
	mcall	23,3
	cmp	eax,1
	je	.redraw
	cmp	eax,2
	je	.key
	cmp	eax,3
	je	.button
	jmp	action
.button:
	mcall	-1
.redraw:
	call	drawwin
	jmp	action
.key:
keypressed:
	cmp	eax,2
	jne	action
	and	[_dx],0
	and	[_dy],0
	mcall	2
	shr	eax,8
;---------
	cmp	eax,32
	jne	key2
	cmp	[strike_action],0
	jne	action
	mov	[SpriteInfo+36+4],dword 1	;can draw sprite "puly"
	mov	[strike_action],1		;sprite	is active
	mov	eax,[SpriteInfo+72+8]
	mov	ebx,[SpriteInfo+72+12]
	mov	ecx,[SpriteInfo+72+28]
	mov	edx,[SpriteInfo+72+32]
	add	eax,ecx
	add	ebx,edx
	mov	[SpriteInfo+36+8],dword eax
	mov	[SpriteInfo+36+12],dword ebx
	jmp	action
key2:
	cmp	eax,176
	jne	key3
	mov	[SpriteInfo+8*9+0],dword 8
	mov	[SpriteInfo+8*9+4],dword 1
	mov	[SpriteInfo+8*9+28],dword -5
	mov	[SpriteInfo+8*9+32],dword 0
	mov	[_dx],0
	mov	[_dy],10
	cmp	[strike_action],0
	jne	action
	mov	[SpriteInfo+36+28],dword -5
	mov	[SpriteInfo+36+32],dword 0
	jmp	action
key3:
	cmp	eax,179
	jne	key4
	mov	[SpriteInfo+8*9+0],dword 4
	mov	[SpriteInfo+8*9+4],dword 1
	mov	[SpriteInfo+8*9+28],dword 5
	mov	[SpriteInfo+8*9+32],dword 0
	mov	[_dx],15
	mov	[_dy],10
	cmp	[strike_action],0
	jne	action
	mov	[SpriteInfo+36+28],dword 5
	mov	[SpriteInfo+36+32],dword 0
	jmp	action
key4:
	cmp	eax,178
	jne	key5
	mov	[SpriteInfo+8*9+0],dword 2
	mov	[SpriteInfo+8*9+4],dword 1
	mov	[SpriteInfo+8*9+28],dword 0
	mov	[SpriteInfo+8*9+32],dword -5
	mov	[_dx],10
	mov	[_dy],1
	cmp	[strike_action],0
	jne	action
	mov	[SpriteInfo+36+28],dword 0
	mov	[SpriteInfo+36+32],dword -5
	jmp	action
key5:
	cmp	eax,177
	jne	key6
	mov	[SpriteInfo+8*9+0],dword 6
	mov	[SpriteInfo+8*9+4],dword 1
	mov	[SpriteInfo+8*9+28],dword 0
	mov	[SpriteInfo+8*9+32],dword 5
	mov	[_dy],15
	mov	[_dx],10
	cmp	[strike_action],0
	jne	action
	mov	[SpriteInfo+36+28],dword 0
	mov	[SpriteInfo+36+32],dword 5
	jmp	action
key6:
	cmp	eax,27
	jne	action
	mcall	-1
action:
	mov	[SpriteInfo+72+4],dword 1
	mov	[SpriteInfo+72+16],dword 5	;usal speed of tank
	mov	esi,[SpriteInfo+8*9+8]		;cordinat x of sprite
	mov	edi,[SpriteInfo+8*9+12]		;cordinat y of sprite
	mov	ecx,[SpriteInfo+72+28]
	mov	edx,[SpriteInfo+72+32]
	add	ecx,[_dx]
	add	edx,[_dy]
	add	esi,ecx	;x=x+_dx+dx
	add	edi,edx	;y=y+_dy+dy
	call	map_x_y
	cmp	ecx,6
	jne	no_woter
	mov	[SpriteInfo+72+16],dword 7	;in water tank slow move
no_woter:
	cmp	ecx,4
	jne	no_palma
	mov	[SpriteInfo+72+4],dword 10000b	;if bit 4 than transparent sprite
no_palma:
	cmp	ecx,8
	jne	no_derevo
	mov	[SpriteInfo+72+4],dword 10000b
no_derevo:
	test	edx,edx
	jnz	ani
	mov	[SpriteInfo+8*9+28],dword 0
	mov	[SpriteInfo+8*9+32],dword 0
;-------------------------------------------------
ani:
	mov	eax,[strike_action]
	test	eax,eax
	jz	no_anim_strike
	mov	esi,[SpriteInfo+36+8]
	mov	edi,[SpriteInfo+36+12]
	mov	ecx,[SpriteInfo+36+28]
	mov	edx,[SpriteInfo+36+32]
	add	esi,ecx
	add	edi,edx
	call	map_x_y
	cmp	cl,0
	jne	no_stena
	mov	[ebx],byte 2	;trava
	jmp	bum
no_stena:
	cmp	cl,1
	jne	no_brony
	jmp	bum
no_brony:
	cmp	cl,7
	jne	anim_action_puly
	sub	[bazas],1
	mov	[ebx],byte 2	;trava
bum:
	mov	esi,[SpriteInfo+36+8]
	mov	edi,[SpriteInfo+36+12]
	mov	[SpriteInfo+4],dword 1
	mov	[SpriteInfo+8],dword esi
	mov	[SpriteInfo+12],dword edi
	mov	[SpriteInfo+36+4],dword 0
	mov	[end_bum],1
	jmp	no_anim_strike
anim_action_puly:
	mov	[SpriteInfo+36+4],dword 1
no_anim_strike:
;-------------------------------------------------
;проверяем попала ли пуля в противников
	mov	esi,[SpriteInfo+4*9+8]
	mov	edi,[SpriteInfo+4*9+12]
	mov	[x],esi	;координата x пули
	mov	[y],edi	;координата y пули
	mov	eax,3*4
bum_tank:
	mov	ebx,eax
	shr	ebx,2
	xor	ecx,ecx
	mov	cl,byte [LifesTanks+ebx]
	cmp	ecx,0
	jz	no_strike
	mov	esi,[SpriteInfo+9*eax+8]	;x
	mov	edi,[SpriteInfo+9*eax+12]	;y
	sub	esi,[x]
	sub	edi,[y]
	cmp	esi,5
	ja	no_strike
	cmp	edi,5
	ja	no_strike
	add	[score],5
	cmp	[score],20
	jl	no_bonus
	add	[LifesPlayer],4
	and	[score],0
no_bonus:
	dec	ecx;,1
	mov	[LifesTanks+ebx],cl	;life=life-1
	mov	esi,[x]
	mov	edi,[y]
	mov	[SpriteInfo+4],dword 1
	mov	[SpriteInfo+8],dword esi
	mov	[SpriteInfo+12],dword edi
	mov	[SpriteInfo+36+4],dword 0
	mov	[end_bum],1
no_strike:
	add	eax,4
	cmp	eax,10*4
	jne	bum_tank
;-------------------------------------------------
	xor	eax,eax
	mov	al,[end_bum]
	test	eax,eax
	jz	no_end_strike
	mov	eax,[SpriteInfo+24]
	cmp	eax,3
	jne	no_end_strike
	and	[strike_action],0
	and	[end_bum],0
	mov	edx,[number_level]
	mov	ecx,15
	call	load_level
no_end_strike:
;----------------------------
;проверяем,попали ли противники	в игрока
	mov	eax,3*4
	mov	esi,[SpriteInfo+72+8]
	mov	edi,[SpriteInfo+72+12]
	mov	[x],esi
	mov	[y],edi
strike_to_player:
	mov	ebx,eax
	add	ebx,28
	mov	esi,[SpriteInfo+9*ebx+8]
	mov	edi,[SpriteInfo+9*ebx+12]
	sub	esi,[x]
	sub	edi,[y]
	cmp	esi,5
	ja	no_strike_to_player
	cmp	edi,5
	ja	no_strike_to_player
	mov	esi,[x]
	mov	edi,[y]
	mov	[SpriteInfo+9*ebx+4],dword 0
	add	ebx,28
	mov	[SpriteInfo+9*ebx+4],dword 1
	mov	[SpriteInfo+9*ebx+8],dword esi
	mov	[SpriteInfo+9*ebx+12],dword edi
	sub	[LifesPlayer],1
	cmp	[LifesPlayer],0
	jne	no_game_over
	call	end_game
	jmp	restart_level
no_game_over:
no_strike_to_player:
	add	eax,4
	cmp	eax,10*4
	jne	strike_to_player
;----------------------------
	xor	eax,eax
	mov	al,[bazas]
	test	eax,eax
	jnz	no_end_level
	call	you_won
	inc	[number_level]
	cmp	[number_level],25
	jne	no_end_game
	mov	eax,-1
	mcall
no_end_game:
	jmp	new_level
no_end_level:
;-------------------------------------------------
;------------ logic of tanks----------------------
;-------------------------------------------------
	mov	eax,12
next_bad_tank:
	mov	ebx,eax
	shr	ebx,2
	xor	ecx,ecx
	mov	cl,byte[LifesTanks+ebx]
	test	ecx,ecx
	jz	no_action_bad_tank
	mov	ebx,[SpriteInfo+9*eax+4]
	test	ebx,ebx
	jnz	no_action_bad_tank
;---------------------
	mov	esi,[SpriteInfo+9*eax+8]
	mov	edi,[SpriteInfo+9*eax+12]
	mov	[x],esi
	mov	[y],edi
	mov	ecx,[SpriteInfo+9*eax+28]
	mov	edx,[SpriteInfo+9*eax+32]
;смотрим что впереди танка-противника
	cmp	ecx,0
	ja	more_null_x
	lea	ecx,[ecx+2*ecx]
	jmp	test_y
more_null_x:
	shl	ecx,2
	add	ecx,15
test_y:
	cmp	edx,0
	ja	more_null_y
	lea	edx,[edx+2*edx]
	jmp	add_numbers
more_null_y:
	shl	edx,2
	add	edx,15
add_numbers:
;--------------------
	add	esi,ecx
	add	edi,edx
	call	map_x_y
	cmp	ecx,4
	jne	no_palma_p
	mov	[SpriteInfo+9*eax+4],dword 10000b
no_palma_p:
	cmp	ecx,8
	jne	no_derevo_p
	mov	[SpriteInfo+9*eax+4],dword 10000b
	no_derevo_p:
;если	на пути	танка препятствие,то надо изменить
;направление движения
	test	edx,edx
	jnz	lab1
;проверяем - не попал ли танк в тупик(3	напрвления заняты)
	and	[tupik],0
	mov	esi,[x]
	mov	edi,[y]
	add	esi,(20+15)
	add	edi,10
	call	map_x_y
	test	edx,edx
	jnz	direct1
	inc	[tupik]
direct1:
	mov	esi,[x]
	mov	edi,[y]
	add	esi,10
	add	edi,(20+15)
	call	map_x_y
	test	edx,edx
	jnz	direct2
	inc	[tupik]
direct2:
	mov	esi,[x]
	mov	edi,[y]
	add	esi,-15
	add	edi,10
	call	map_x_y
	test	edx,edx
	jnz	direct3
	inc	[tupik]
direct3:
	mov	esi,[x]
	mov	edi,[y]
	add	esi,10
	add	edi,-15
	call	map_x_y
	test	edx,edx
	jnz	direct4
	inc	[tupik]
direct4:
	cmp	[tupik],3
	je	no_move0
;------------------
;strategy1
	mov	esi,[x]
	mov	edi,[y]
	add	esi,(20+15)
	add	edi,10
	call	map_x_y
	test	edx,edx
	jnz	no_strategy1
	mov	esi,[x]
	mov	edi,[y]
	add	esi,10
	add	edi,-15
	call	map_x_y
	jnz	no_strategy1
	jmp	no_move3	;going to left
no_strategy1:
;------------------
	mov	esi,[x]
	mov	edi,[y]
	add	esi,-15
	add	edi,10
	call	map_x_y
	test	edx,edx
	jnz	no_strategy2
	mov	esi,[x]
	mov	edi,[y]
	add	esi,10
	add	edi,-15
	call	map_x_y
	test	edx,edx
	jnz	no_strategy2
	jmp	no_move2
no_strategy2:
;------------------
	mov	esi,[x]
	mov	edi,[y]
	add	edi,(20+15)
	add	esi,10
	call	map_x_y
	test	edx,edx
	jnz	no_strategy3
	mov	esi,[x]
	mov	edi,[y]
	add	esi,-15
	add	edi,10
	call	map_x_y
	test	edx,edx
	jnz	no_strategy3
	jmp	no_move1
no_strategy3:
;-------------------------------------------------
no_move0:
	mov	esi,[x]	;	x
	mov	edi,[y]	;	y
	add	edi,-15	;	(y-20) up
	add	esi,10
	call	map_x_y
	test	edx,edx
	jz	no_move1
;вверху	свободно - можно двигаться up
	mov	[SpriteInfo+9*eax+0],dword 3
	mov	[SpriteInfo+9*eax+28],dword 0
	mov	[SpriteInfo+9*eax+32],dword -5
	jmp	lab1
no_move1:
	mov	esi,[x]
	mov	edi,[y]
	add	esi,(20+15)	;x+20
	add	edi,10
	call	map_x_y
	test	edx,edx
	jz	no_move2
;right
	mov	[SpriteInfo+9*eax+0],dword 5
	mov	[SpriteInfo+9*eax+28],dword 5
	mov	[SpriteInfo+9*eax+32],dword 0
	jmp	lab1
no_move2:
	mov	esi,[x]
	mov	edi,[y]
	add	esi,10	;x-20
	add	edi,(20+15)
	call	map_x_y
	test	edx,edx
	jz	no_move3
;down
	mov	[SpriteInfo+9*eax+0],dword 7
	mov	[SpriteInfo+9*eax+28],dword 0
	mov	[SpriteInfo+9*eax+32],dword 5
	jmp	lab1
no_move3:
	mov	esi,[x]
	mov	edi,[y]
	add	edi,10	;y+20
	add	esi,-15
	call	map_x_y
	test	edx,edx
	jz	no_move4
;внизу свободно-можно двигаться left
	mov	[SpriteInfo+9*eax+0],dword 9
	mov	[SpriteInfo+9*eax+28],dword -5
	mov	[SpriteInfo+9*eax+32],dword 0
	jmp	lab1
no_move4:
	mov	[SpriteInfo+9*eax+28],dword 0
	mov	[SpriteInfo+9*eax+32],dword 0
lab1:
	mov	edx,dword[SpriteInfo+9*eax+4]
	test	edx,edx
	jnz	no_action_bad_tank
	mov	[SpriteInfo+9*eax+4],dword 1
no_action_bad_tank:
	add	eax,4
	cmp	eax,10*4
	jne	next_bad_tank
;-------------------------------------------------
	mov	eax,12
next_puly:
	mov	edx,eax
	shr	edx,2
	xor	ecx,ecx
	mov	cl,byte[PulyTanks+edx]
	test	ecx,ecx	;can change course of puly?
	jz	no_change_course
	xor	ecx,ecx
	mov	cl,byte[LifesTanks+edx]
	test	ecx,ecx	;is tank not destroed?
	jz	no_draw_s
	mov	ecx,[SpriteInfo+9*eax+24]	;tank's phas of animation
	test	ecx,ecx		;phas=0?
	jnz	no_change_course
	mov	esi,[SpriteInfo+9*eax+8]
	mov	edi,[SpriteInfo+9*eax+12]
	mov	ecx,[SpriteInfo+9*eax+28]
	mov	edx,[SpriteInfo+9*eax+32]
	mov	ebx,eax
	add	ebx,28
	mov	[SpriteInfo+9*ebx+0],dword 1
	mov	[SpriteInfo+9*ebx+4],dword 1
	mov	[SpriteInfo+9*ebx+28],dword ecx
	mov	[SpriteInfo+9*ebx+32],dword edx
	mov	[SpriteInfo+9*ebx+16],dword 3
	mov	[SpriteInfo+9*ebx+8],dword esi
	mov	[SpriteInfo+9*ebx+12],dword edi
	mov	ebx,eax
	shr	ebx,2
	mov	[PulyTanks+ebx],byte 0
	jmp	no_draw_s
;-------------------
no_change_course:
	mov	ebx,eax
	add	ebx,28
	mov	ecx,[SpriteInfo+9*ebx+24]
	test	ecx,ecx
	jnz	no_draw_s
	mov	esi,[SpriteInfo+9*ebx+8]
	mov	edi,[SpriteInfo+9*ebx+12]
	mov	ecx,[SpriteInfo+9*ebx+28]
	mov	edx,[SpriteInfo+9*ebx+32]
	call	map_x_y
	cmp	ecx,1
	je	bumm
	cmp	ecx,0
	je	strike_to_wall
	jmp	no_bumm
strike_to_wall:
	mov	[ebx],byte 2
bumm:
;-------------------
	mov	ebx,eax
	add	ebx,28
	mov	[SpriteInfo+9*ebx+28],dword 0
	mov	[SpriteInfo+9*ebx+32],dword 0
	mov	esi,[SpriteInfo+9*ebx+8]
	mov	edi,[SpriteInfo+9*ebx+12]
	add	ebx,28
	mov	[SpriteInfo+9*ebx+4],dword 1
	mov	[SpriteInfo+9*ebx+8],dword esi
	mov	[SpriteInfo+9*ebx+12],dword edi
	mov	[SpriteInfo+9*ebx+16],dword 3
	mov	[SpriteInfo+9*ebx+28],dword 0
	mov	[SpriteInfo+9*ebx+32],dword 0
	mov	ebx,eax
	shr	ebx,2
	mov	[PulyTanks+ebx],byte 1
	mov	edx,[number_level]
	mov	ecx,15
	push	eax
	call	load_level
	pop	eax
	jnp	no_draw_s
;-------------------
no_bumm:
	mov	ebx,eax
	add	ebx,28
	mov	[SpriteInfo+9*ebx+4],dword 1	;can draw sprite of puly
no_draw_s:
	add	eax,4
	cmp	eax,10*4
	jne	next_puly
;-------------------------------------------------
;-------------end of tanks logic------------------
;-------------------------------------------------
	xor	edx,edx
	mcall	13,<190,192>,<5,7>
	mcall	4,<190,5>,0x1ded00,Level,5	
	mcall	,<260,5>,0x3558ff,Lifes,
	mcall	,<330,5>,0xf93500,Score,
	mov	ecx,[number_level]
;	inc	ecx	//leency
	mcall	47,3*65536,,<225,5>,0x1ded00
	xor	ecx,ecx
	mov	cl,[LifesPlayer]
	shr	ecx,2
	mcall	,,,<295,5>,0x3558ff
	xor	ecx,ecx
	mov	cl,byte[score]
	mcall	,,<365,5>,0xf93500
	jmp	maincycle
;----------------------------------------------------------
;-----------------end of main cycle------------------------
;----------------------------------------------------------
pause_cicle:
	pusha
.start:
	mcall	9,procinfo,-1
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	@f
	popa
	ret
@@:
	mcall	10
	dec	eax
	jz	.redraw
	dec	eax
	jz	.key
	dec	eax
	jnz	.start	
.button:
	mcall	-1
.key:
	mcall	2
	jmp	.start
.redraw:
	call	drawwin
	jmp	.start
;---------------------------------------------------------
;draw sprite in video memory
PutSprite:
	push	eax
	mov	ebx,esi
	mov	eax,edi
	lea	ebx,[ebx+2*ebx]
	lea	eax,[eax+2*eax]
	imul	eax,640
	add	eax,0x4000
	add	eax,ebx
	mov	[counter],20
	mov	esi,edx
	imul	esi,1200
	add	esi,0x4000+(640*400*3)+10*1200
	mov	ebx,esi
	mov	edi,eax
draw:
	mov	esi,ebx
	mov	edi,eax
	mov	ecx,20
;--------------------
rep_movsb:
	xor	edx,edx
	mov	edx,[esi]
	and	edx,0xffffff
	test	edx,edx	;0 is transparent color
	jz	transparent_color
	mov	[edi],dx
	shr	edx,16
	mov	[edi+2],dl
transparent_color:
	add	esi,3
	add	edi,3
	sub	ecx,1
	jnz	rep_movsb
;--------------------
	add	eax,640*3
	add	ebx,20*3
	dec	[counter]
	jnz	draw
	pop	eax
	ret
;----------------------------------------------------------
SaveFon:
	push	eax
	lea	esi,[esi+2*esi]
	lea	edi,[edi+2*edi]
	imul	edi,640
	add	esi,edi
	add	esi,0x4000
	mov	edi,edx
	imul	edi,1200
	add	edi,0x4000+(640*400*3)+(50*20*20*3)+512+641
	mov	[counter],20
	mov	eax,esi
	mov	ebx,edi
save_to:
	mov	esi,eax
	mov	edi,ebx
	mov	ecx,15
	rep	movsd
	add	eax,640*3
	add	ebx,20*3
	dec	[counter]
	jnz	save_to
	pop	eax
	ret
;----------------------------------------------------------
PutFon:
	push	eax
	lea	esi,[esi+2*esi]
	lea	edi,[edi+2*edi]
	imul	edi,640
	add	edi,esi
	add	edi,0x4000
	mov	esi,edx
	imul	esi,1200
	add	esi,0x4000+(640*400*3)+(50*20*20*3)+512+641
	mov	[counter],20
	mov	eax,esi
	mov	ebx,edi
put_to:
	mov	esi,eax
	mov	edi,ebx
	mov	ecx,15
	rep	movsd
	add	eax,20*3
	add	ebx,640*3
	dec	[counter]
	jnz	put_to
	pop	eax
	ret
;----------------------------------------------------------
;get time in 1/100 sec
clock:
	mcall	26,9
	mov	[time],eax
	ret
;----------------------------------------------------------
;-----------------load level to memory---------------------
;----------------------------------------------------------
load_level:
	cmp	ecx,15
	je	no_load_level
	mov	eax,edx
	mov	ebx,[levels+4*eax]
	mov	esi,ebx
	add	esi,2
	xor	ecx,ecx
	xor	eax,eax
	mov	cl,byte[ebx]
	mov	al,byte[ebx+1]
	add	ecx,eax
	mov	edi,0x4000+(640*400*3)+(50*20*20*3)+512
	call	unpakin
no_load_level:
	and	[y],0
c_y:
	and	[x],0
c_x:
	mov	eax,[x]
	mov	ebx,[y]
	shl	ebx,5
	add	eax,ebx
	add	eax,0x4000+(640*400*3)+(50*20*20*3)+512
	mov	ecx,eax
	xor	eax,eax
	mov	al,byte [ecx]
	imul	eax,1200
	add	eax,0x4000+(640*400*3)
	mov	ebx,[x]
	imul	ebx,20*3
	mov	ecx,[y]
	imul	ecx,20*3*640
	add	ebx,ecx
	add	ebx,0x4000
	mov	esi,eax
	mov	edi,ebx
;----------------------------
	mov	edx,20
next_line:
	mov	esi,eax
	mov	edi,ebx
	mov	ecx,15
	rep	movsd
	add	eax,20*3
	add	ebx,(640*3)
	dec	edx
	jnz	next_line
;----------------------------
	inc	[x]
	cmp	[x],32
	jne	c_x
	inc	[y]
	cmp	[y],20
	jne	c_y
	ret
;----------------------------------------------------------
map_x_y:
	push	eax
	mov	ecx,20
	mov	eax,esi
	cdq
	idiv	ecx
	mov	esi,eax
	mov	eax,edi
	cdq
	idiv	ecx
	mov	edi,eax
;------------------
	mov	ebx,edi
	shl	ebx,5
	add	ebx,esi
	add	ebx,0x4000+(640*400*3)+(50*20*20*3)+512
	xor	ecx,ecx
	mov	cl,byte[ebx]
	cmp	cl,0
	je	false_draw
	cmp	cl,1
	je	false_draw
	cmp	cl,3
	je	false_draw
	cmp	cl,7
	je	false_draw
	mov	edx,1
	jmp	lab2
false_draw:
	xor	edx,edx
lab2:
	pop	eax
	ret
;----------------------------------------------------------
you_won:
	mcall	13,<1,640>,<20,400>,0xc6e9
	mcall	4,<255,190>,0xffffff,won1,29
	mcall	,<255,200>,,won2,
	mcall	,<255,210>,,won3,
	mcall	5,400
	ret
;----------------------------------------------------------
end_game:
	xor	edx,edx
	mcall	13,<0,640>,<20,400>
	mcall	4,<280,200>,0xffffff,game_over,9
	mcall	5,350
	ret
;----------------------------------------------------------
drawwin:
	mcall	12,1
;рисуем окно задавая все необходимые цвета
	mcall	0,100*65536+649,50*65536+446,(0x74000000+0xffffff),,name
	mcall	9,procinfo,-1
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	@f
	call	paint_area
@@:
	mcall	12,2
	ret
;----------------------------------------------------------
paint_area:
	xor	edx,edx
	mcall	13,0*65536+640,0*65536+20
	ret
;----------------------------------------------------------	
menu:
cycle_menu:
	mcall	13,<238,141>,<229,20>,0xed16
	mcall	4,<255,235>,0xff0200,start_menu,11
	mcall	47,3*65536,[number_level],<345,235>,0xff0200
	mcall	4,<465,5>,0x888888,description,25
still:
	mcall	10
	dec	eax
	jz	.redraw
	dec	eax
	jz	.key
;	jmp	.button
;.button:
	mcall	-1
.redraw:
	pop	eax
	jmp	restart_level
.key:
	mcall	2
	shr	eax,8
	cmp	eax,32
	je	start_game
	cmp	eax,13
	je	start_game
	cmp	eax,176
	jne	no_left
	dec	[number_level]
	and	[number_level],15
	jmp	cycle_menu
no_left:
	cmp	eax,179
	jne	no_right
	inc	[number_level]
	and	[number_level],15
	jmp	cycle_menu
no_right:
	cmp	eax,27
	jne	cycle_menu
	mov	eax,-1
	mcall
	start_game:
	ret
;----------------------------------------------------------
;--------------unpak pix engin-----------------------------
;----------------------------------------------------------
	unpakin:
NextLitlColor:
	xor	edx,edx
	mov	dl,byte[esi]
	xor	eax,eax
	xor	ebx,ebx
	mov	al,dl
	mov	bl,al
	shr	al,4
	and	al,0xf
	mov	[LitlCounter],al
	and	bl,0xf
	mov	[LitlColor],bl
	xor	eax,eax
;----------------------
	mov	al,[LitlColor]
beg:
	mov	[edi],al
	add	edi,1
	add	[LitlCounter],-1
	cmp	[LitlCounter],-1
	jne	beg
	inc	esi
	dec	ecx
	jnz	NextLitlColor
	ret
;----------------------------------------------------------
include	'data.inc'
;----------------------------------------------------------
procinfo:
	rb 1024
;----------------------------------------------------------
I_END:
