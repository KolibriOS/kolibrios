;---------------------------------------------------------------------
processing_npc_action:
;        DEBUGF  1, "processing_npc_action\n"
	cmp	dl,ROUTE_UP
	je	npc_route_up

	cmp	dl,ROUTE_DOWN
	je	npc_route_down

	cmp	dl,ROUTE_LEFT
	je	npc_route_left

	cmp	dl,ROUTE_RIGHT
	je	npc_route_right

	ret
;---------------------------------------------------------------------
npc_route_up:
;        DEBUGF  1, "npc route_up\n"
	cmp	ebx,1	; border of map?
	jne	@f
;--------------------------------------
.change_route:
;        DEBUGF  1, "npc .change_route_up\n"
	mov	dx,0x0100+ROUTE_RIGHT
	call	get_random_binary
	test	al,al
	jz	.save_new_route

	mov	dx,0x0100+ROUTE_DOWN
;--------------------------------------	
.save_new_route:
	rol	edx,16
	mov	[ecx],edx
	ret
;--------------------------------------
@@:
;        DEBUGF  1, "npc up @@:\n"
	call	npc_search_target_up_left
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,TARGET_RANGE
	sub	edi,[target_action_cell_x]
	
	mov	ebx,TARGET_RANGE
	sub	ebx,[target_action_cell_y]
;	DEBUGF	1, "%x npc_route_up target_up_left X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx
	cmp	edi,ebx
	jb	.continue
	
	mov	si,0x0100+ROUTE_LEFT
	jmp	save_route_to_current_position
;--------------------------------------
@@:
	call	npc_search_target_up_right
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,[target_action_cell_x]
	dec	edi
	
	mov	ebx,TARGET_RANGE
	sub	ebx,[target_action_cell_y]
;        DEBUGF  1, "%x npc_route_up target_up_right X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx
	cmp	edi,ebx
	jb	.continue
	
	mov	si,0x0100+ROUTE_RIGHT
	jmp	save_route_to_current_position
;--------------------------------------
@@:
.continue:
	mov	eax,[ecx-LEVEL_MAP_SIZE_X*4]
	
	cmp	ah,OBJECT_PROTAGONIST
	je	.continue_1
	
	test	ah,ah
	jnz	.change_route
;--------------------------------------
@@:
	cmp	al,BASE_WATER
	jz	.change_route
	
	cmp	al,BASE_LAVA
	jz	.change_route
;--------------------------------------
.continue_1:
;        DEBUGF  1, "npc good route up\n"
	mov	si,0x0100+ROUTE_UP
	mov	edi,ecx
	sub	edi,LEVEL_MAP_SIZE_X*4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
npc_route_down:
;        DEBUGF  1, "npc route_down\n"
	cmp	ebx,LEVEL_MAP_SIZE_Y	; border of map?
	jne	@f
;--------------------------------------
.change_route:
;        DEBUGF  1, "npc .change_route_down\n"
	mov	dx,0x0100+ROUTE_LEFT
	call	get_random_binary
	test	al,al
	jz	.save_new_route

	mov	dx,0x0100+ROUTE_UP
;--------------------------------------	
.save_new_route:
	rol	edx,16
	mov	[ecx],edx
	ret
;--------------------------------------
@@:
;        DEBUGF  1, "npc down @@:\n"
	call	npc_search_target_down_left
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,TARGET_RANGE
	sub	edi,[target_action_cell_x]
	
	mov	ebx,[target_action_cell_y]
	dec	ebx
;        DEBUGF  1, "%x npc_route_down target_down_left X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx
	cmp	edi,ebx
	jb	.continue
	
	mov	si,0x0100+ROUTE_LEFT
	jmp	save_route_to_current_position
;--------------------------------------
@@:
	call	npc_search_target_down_right
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,[target_action_cell_x]
	dec	edi
	
	mov	ebx,[target_action_cell_y]
;        DEBUGF  1, "%x npc_route_down target_down_right X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx	
	cmp	edi,ebx
	jb	.continue
	
	mov	si,0x0100+ROUTE_RIGHT
	jmp	save_route_to_current_position
;--------------------------------------
@@:
.continue:
	mov	eax,[ecx+LEVEL_MAP_SIZE_X*4]
	
	cmp	ah,OBJECT_PROTAGONIST
	je	.continue_1

	test	ah,ah
	jnz	.change_route
;--------------------------------------
@@:
	cmp	al,BASE_WATER
	jz	.change_route
	
	cmp	al,BASE_LAVA
	jz	.change_route
;--------------------------------------
.continue_1:	
;        DEBUGF  1, "npc good route down\n"
	mov	si,0x0100+ROUTE_DOWN
	mov	edi,ecx
	add	edi,LEVEL_MAP_SIZE_X*4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
npc_route_left:
;        DEBUGF  1, "npc route_left\n"
	cmp	eax,1	; border of map?
	jne	@f
;--------------------------------------
.change_route:
;        DEBUGF  1, "npc .change_route_left\n"
	mov	dx,0x0100+ROUTE_UP
	call	get_random_binary
	test	al,al
	jz	.save_new_route

	mov	dx,0x0100+ROUTE_RIGHT
;--------------------------------------	
.save_new_route:
	rol	edx,16
	mov	[ecx],edx
	ret
;--------------------------------------
@@:
;        DEBUGF  1, "npc left @@:\n"
	call	npc_search_target_up_left
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,TARGET_RANGE
	sub	edi,[target_action_cell_x]
	
	mov	ebx,TARGET_RANGE
	sub	ebx,[target_action_cell_y]
;        DEBUGF  1, "%x npc_route_left target_up_left X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx	
	cmp	edi,ebx
	jae	.continue
	
	mov	si,0x0100+ROUTE_UP
	jmp	save_route_to_current_position
;--------------------------------------
@@:
	call	npc_search_target_down_left
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,TARGET_RANGE
	sub	edi,[target_action_cell_x]
	
	mov	ebx,[target_action_cell_y]
	dec	ebx
;        DEBUGF  1, "%x npc_route_left target_down_left X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx
	cmp	edi,ebx
	jae	.continue
	
	mov	si,0x0100+ROUTE_DOWN
	jmp	save_route_to_current_position
;--------------------------------------
@@:
.continue:
	mov	eax,[ecx-4]
	
	cmp	ah,OBJECT_PROTAGONIST
	je	.continue_1

	test	ah,ah
	jnz	.change_route
;--------------------------------------
@@:
	cmp	al,BASE_WATER
	jz	.change_route
	
	cmp	al,BASE_LAVA
	jz	.change_route
;--------------------------------------
.continue_1:
;        DEBUGF  1, "npc good route left\n"
	mov	si,0x0100+ROUTE_LEFT
	mov	edi,ecx
	sub	edi,4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
npc_route_right:
;        DEBUGF  1, "npc route_right\n"
	cmp	eax,LEVEL_MAP_SIZE_X	; border of map?
	jne	@f
;--------------------------------------
.change_route:
;        DEBUGF  1, "npc .change_route_right\n"
	mov	dx,0x0100+ROUTE_DOWN
	call	get_random_binary
	test	al,al
	jz	.save_new_route

	mov	dx,0x0100+ROUTE_LEFT
;--------------------------------------	
.save_new_route:
	rol	edx,16
	mov	[ecx],edx
	ret
;--------------------------------------
@@:
;        DEBUGF  1, "npc righ @@:\n"
	call	npc_search_target_up_right
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,[target_action_cell_x]
	dec	edi
	
	mov	ebx,TARGET_RANGE
	sub	ebx,[target_action_cell_y]
;        DEBUGF  1, "%x npc_route_right target_up_right X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx
	cmp	edi,ebx
	jae	.continue
	
	mov	si,0x0100+ROUTE_UP
	jmp	save_route_to_current_position
;--------------------------------------
@@:
	call	npc_search_target_down_right
	cmp	[target_detected],1
	jne	@f
	
	mov	edi,[target_action_cell_x]
	dec	edi
	
	mov	ebx,[target_action_cell_y]
	dec	ebx
;        DEBUGF  1, "%x npc_route_right target_down_right X: %x Y: %x\n",[current_cell_NPC]:1,edi,ebx	
	cmp	edi,ebx
	jae	.continue
	
	mov	si,0x0100+ROUTE_DOWN
	jmp	save_route_to_current_position
;--------------------------------------
@@:
.continue:
	mov	eax,[ecx+4]

	cmp	ah,OBJECT_PROTAGONIST
	je	.continue_1

	test	ah,ah
	jnz	.change_route
;--------------------------------------
@@:
	cmp	al,BASE_WATER
	jz	.change_route
	
	cmp	al,BASE_LAVA
	jz	.change_route
;--------------------------------------
.continue_1:
;        DEBUGF  1, "npc good route righ\n"
	mov	si,0x0100+ROUTE_RIGHT
	mov	edi,ecx
	add	edi,4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
npc_search_target_up_left:
;        DEBUGF  1, "%x npc_search_target_up_left X: %x Y: %x\n",[current_cell_NPC]:1,[action_cell_x],[action_cell_y]
	push	ecx
	sub	ecx,TARGET_RANGE * 4	; offset x
	sub	ecx,TARGET_RANGE * LEVEL_MAP_SIZE_X * 4 ; offset y
	jmp	npc_search_target
;---------------------------------------------------------------------	
npc_search_target_up_right:
;        DEBUGF  1, "%x npc_search_target_up_right X: %x Y: %x\n",[current_cell_NPC]:1,[action_cell_x],[action_cell_y]
	push	ecx
	sub	ecx,TARGET_RANGE * LEVEL_MAP_SIZE_X * 4	; offset y
	sub	ecx,4	; offset x
	jmp	npc_search_target
;---------------------------------------------------------------------
npc_search_target_down_left:
;        DEBUGF  1, "%x npc_search_target_down_left X: %x Y: %x\n",[current_cell_NPC]:1,[action_cell_x],[action_cell_y]
	push	ecx
	sub	ecx,LEVEL_MAP_SIZE_X * 4	; offset y
	sub	ecx,TARGET_RANGE * 4	; offset x
	jmp	npc_search_target
;---------------------------------------------------------------------	
npc_search_target_down_right:
;        DEBUGF  1, "%x npc_search_target_down_right X: %x Y: %x\n",[current_cell_NPC]:1,[action_cell_x],[action_cell_y]
	push	ecx
	sub	ecx,LEVEL_MAP_SIZE_X * 4	; offset y
	sub	ecx,4	; offset x
;---------------------------------------------------------------------
npc_search_target:
	push	eax
	mov	[target_detected],0
	mov	ebx,TARGET_RANGE
;--------------------------------------
.y:
	mov	eax,TARGET_RANGE
;        DEBUGF  1, "current cell next y\n"
;--------------------------------------
.x:	
	push 	eax ebx ecx
	mov	[target_action_cell_x],eax
	mov	[target_action_cell_y],ebx
	shl	eax,2
	add	ecx,eax
	imul	ebx,LEVEL_MAP_SIZE_X*4
	add	ecx,ebx
	
	cmp	ecx,plan_level
	jb	@f
	
	cmp	ecx,plan_level+LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4-4
	ja	@f

	mov	eax,[ecx]
;        DEBUGF  1, "current cell %x\n",eax
	cmp	ah,OBJECT_PROTAGONIST
	jne	@f
	
	pop	ecx ebx eax
	mov	[target_detected],1
;        DEBUGF  1, "%x target_detected X: %x Y: %x\n",[current_cell_NPC]:1,[target_action_cell_x],[target_action_cell_y]
	jmp	.exit	
;--------------------------------------	
@@:
	pop	ecx ebx eax

	dec	eax
	jnz	.x
	
	dec	ebx
	jnz	.y
.exit:
	pop	eax ecx
	ret
;---------------------------------------------------------------------
