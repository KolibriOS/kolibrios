;---------------------------------------------------------------------
actions_for_all_cell:
;;        DEBUGF  1, "actions_for_all\n"
	; set start coordinates
	mov	ebx,LEVEL_MAP_SIZE_Y
	mov	ecx,plan_level+(LEVEL_MAP_SIZE_Y*LEVEL_MAP_SIZE_X*4)-4
;--------------------------------------
.new_y:
	mov	eax,LEVEL_MAP_SIZE_X
;--------------------------------------
.new_x:
	push	eax ebx ecx
	mov	[action_cell_x],eax
	mov	[action_cell_y],ebx
	mov	edx,[ecx]
	mov	[current_cell_NPC],dh
	test	dh,dh	; empty
	jz	.end_processing_sell
;	mov	ebp,ecx
;	sub	ebp,plan_level
;        DEBUGF  1, "Y : %x X: %x cell: %x ebp: %x\n",ebx,eax,edx,ebp
	ror	edx,16
	test	dh,dh
	jnz	.end_processing_sell

	test	dl,dl
	jz	.end_processing_sell

;	call	processing_NPC_route
;        DEBUGF  1, "new cell: %x\n",edx

	cmp	[current_cell_NPC],OBJECT_FINAL_MONSTER
	ja	@f
	
	call	processing_npc_action
	jmp	.end_processing_sell
;--------------------------------------	
@@:
	cmp	[current_cell_NPC],OBJECT_PROTAGONIST
	jne	@f
	
	call	processing_protagonist_action
	jmp	.end_processing_sell
;--------------------------------------	
@@:
	cmp	[current_cell_NPC],OBJECT_WHITE_BRICK
	jne	.end_processing_sell
	
	call	processing_white_brick
;--------------------------------------	
.end_processing_sell:
	pop	ecx ebx eax
	
	sub	ecx,4
	dec	eax
	jnz	.new_x
	
	dec	ebx
	jnz	.new_y
;;        DEBUGF  1, "actions_for_all END\n"
	ret
;---------------------------------------------------------------------
action_stop:
;        DEBUGF  1, "action_stop\n"
	mov	dx,0x0100
	rol	edx,16
	mov	[ecx],edx
	ret
;---------------------------------------------------------------------
object_death:
	mov	dx,0x01
	shl	edx,24
	mov	[ecx],edx
	ret
;---------------------------------------------------------------------
save_route_to_current_position:
	ror	eax,16
	mov	ax,si
	rol	eax,16
	rol	edx,16
	mov	ax,dx
	mov	[ecx],eax ; new route for unit
	ret
;---------------------------------------------------------------------
save_new_and_clear_old_position:
;        DEBUGF  1, "save_new_and_clear_old_position\n"
	ror	eax,16
	mov	ax,si
	rol	eax,16
	rol	edx,16
	mov	ah,dh
	mov	[edi],eax ; new cell for unit
	and	edx,0xff
	add	edx,0x01000000	; clear old cell and show
	mov	[ecx],edx
; copy object id	
	mov	eax,[ecx+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4]
	mov	[edi+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4],eax
	xor	eax,eax
	mov	[ecx+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4],eax
	ret
;---------------------------------------------------------------------
harvest_of_death:
	xor	eax,eax
	mov	[death_of_protagonist],al
	mov	[npc_alive],al
	mov	esi,plan_level
	mov	ecx,LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y
	cld
;--------------------------------------
.loop:
	lodsd
	
	test	ah,ah
	jz	.counter

	cmp	al,BASE_LAVA
	je	.base_lava
	
	cmp	al,BASE_WATER
	je	.base_water
	
	cmp	ah,OBJECT_FINAL_MONSTER
	ja	.counter
	
	mov	[npc_alive],1
;--------------------------------------
.counter:
	dec	ecx
	jnz	.loop
	
	ret
;--------------------------------------
.base_water:
	cmp	ah,OBJECT_PROTAGONIST
	jne	.kill_object
;--------------------------------------
.kill_protagonist:
	mov	[death_of_protagonist],1
	jmp	.kill_object
;--------------------------------------
.base_lava:
	cmp	ah,OBJECT_PROTAGONIST
	je	.kill_protagonist

	cmp	ah,OBJECT_SKELETON
	ja	.counter
;--------------------------------------
.kill_object:
;        DEBUGF  1, "kill_object\n"
	and	eax,0x000000ff
	add	eax,0x01000000
	mov	[esi-4],eax
	xor	eax,eax
	mov	[esi-4-plan_level+objects_id],eax
	jmp	.counter
;---------------------------------------------------------------------
