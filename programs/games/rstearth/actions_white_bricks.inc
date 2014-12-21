;---------------------------------------------------------------------
processing_white_brick:
;;        DEBUGF  1, "processing_white_brick\n"
	cmp	dl,ROUTE_UP
	je	white_brick_route_up

	cmp	dl,ROUTE_DOWN
	je	white_brick_route_down

	cmp	dl,ROUTE_LEFT
	je	white_brick_route_left

	cmp	dl,ROUTE_RIGHT
	je	white_brick_route_right

	ret
;---------------------------------------------------------------------
white_brick_route_up:
;        DEBUGF  1, "white_brick route_up\n"
	cmp	ebx,1	; border of map?
	je	action_stop

;        DEBUGF  1, "white_brick up @@:\n"
	mov	eax,[ecx-LEVEL_MAP_SIZE_X*4]
	cmp	ah,OBJECT_WHITE_BRICK	; new coordinates have white block?
	je	action_stop

;        DEBUGF  1, "white_brick good route up\n"
	mov	si,0x0100+ROUTE_UP
	mov	edi,ecx
	sub	edi,LEVEL_MAP_SIZE_X*4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
white_brick_route_down:
;        DEBUGF  1, "white_brick route_down\n"
	cmp	ebx,LEVEL_MAP_SIZE_Y	; border of map?
	je	action_stop

;        DEBUGF  1, "white_brick down @@:\n"
	mov	eax,[ecx+LEVEL_MAP_SIZE_X*4]
	cmp	ah,OBJECT_WHITE_BRICK	; new coordinates have white block?
	je	action_stop

;        DEBUGF  1, "white_brick good route down\n"
	mov	si,0x0100+ROUTE_DOWN
	mov	edi,ecx
	add	edi,LEVEL_MAP_SIZE_X*4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
white_brick_route_left:
;        DEBUGF  1, "white_brick route_left\n"
	cmp	eax,1	; border of map?
	je	action_stop

;        DEBUGF  1, "white_brick left @@:\n"
	mov	eax,[ecx-4]
	cmp	ah,OBJECT_WHITE_BRICK	; new coordinates have white block?
	je	action_stop

;        DEBUGF  1, "white_brick good route left\n"
	mov	si,0x0100+ROUTE_LEFT
	mov	edi,ecx
	sub	edi,4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
white_brick_route_right:
;        DEBUGF  1, "white_brick route_right\n"
	cmp	eax,LEVEL_MAP_SIZE_X	; border of map?
	je	action_stop

;        DEBUGF  1, "white_brick righ @@:\n"
	mov	eax,[ecx+4]
	cmp	ah,OBJECT_WHITE_BRICK	; new coordinates have white block?
	je	action_stop

;        DEBUGF  1, "white_brick good route righ\n"
	mov	si,0x0100+ROUTE_RIGHT
	mov	edi,ecx
	add	edi,4
	jmp	save_new_and_clear_old_position
;---------------------------------------------------------------------
