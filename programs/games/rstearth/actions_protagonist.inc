;---------------------------------------------------------------------
processing_protagonist_action:
;;        DEBUGF  1, "processing_protagonist_action\n"
	cmp	dl,ROUTE_UP
	je	protagonist_route_up

	cmp	dl,ROUTE_DOWN
	je	protagonist_route_down

	cmp	dl,ROUTE_LEFT
	je	protagonist_route_left

	cmp	dl,ROUTE_RIGHT
	je	protagonist_route_right

	ret
;---------------------------------------------------------------------
protagonist_route_up:
;        DEBUGF  1, "protagonist route_up\n"
	cmp	ebx,1	; border of map?
	je	action_stop

;        DEBUGF  1, "protagonist up @@:\n"
	mov	eax,[ecx-LEVEL_MAP_SIZE_X*4]
	test	ah,ah
	jnz	action_stop
	
;	cmp	al,BASE_WATER
;	jz	action_stop
	
;        DEBUGF  1, "protagonist good route up\n"
	dec	[protagonist_position.y]
	mov	si,0x0100+ROUTE_UP
	mov	edi,ecx
	sub	edi,LEVEL_MAP_SIZE_X*4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
protagonist_route_down:
;        DEBUGF  1, "protagonist route_down\n"
	cmp	ebx,LEVEL_MAP_SIZE_Y	; border of map?
	je	action_stop

;        DEBUGF  1, "protagonist down @@:\n"
	mov	eax,[ecx+LEVEL_MAP_SIZE_X*4]
	test	ah,ah
	jnz	action_stop

;	cmp	al,BASE_WATER
;	jz	action_stop
	
;        DEBUGF  1, "protagonist good route down\n"
	inc	[protagonist_position.y]
	mov	si,0x0100+ROUTE_DOWN
	mov	edi,ecx
	add	edi,LEVEL_MAP_SIZE_X*4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
protagonist_route_left:
;        DEBUGF  1, "protagonist route_left\n"
	cmp	eax,1	; border of map?
	je	action_stop

;        DEBUGF  1, "protagonist left @@:\n"
	mov	eax,[ecx-4]
	test	ah,ah
	jnz	action_stop

;	cmp	al,BASE_WATER
;	jz	action_stop
	
;        DEBUGF  1, "protagonist good route left\n"
	dec	[protagonist_position.x]
	mov	si,0x0100+ROUTE_LEFT
	mov	edi,ecx
	sub	edi,4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
protagonist_route_right:
;        DEBUGF  1, "protagonist route_right\n"
	cmp	eax,LEVEL_MAP_SIZE_X	; border of map?
	je	action_stop

;        DEBUGF  1, "protagonist righ @@:\n"
	mov	eax,[ecx+4]
	test	ah,ah
	jnz	action_stop

;	cmp	al,BASE_WATER
;	jz	action_stop
	
;        DEBUGF  1, "protagonist good route righ\n"
	inc	[protagonist_position.x]
	mov	si,0x0100+ROUTE_RIGHT
	mov	edi,ecx
	add	edi,4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
