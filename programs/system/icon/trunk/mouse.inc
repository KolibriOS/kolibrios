;------------------------------------------------------------------------------
align 4
START_mouse_thread:
	mcall	40,100000b	; only Event 6 - Mouse
;------------------------------------------------------------------------------
align 4
still_mouse:
	mcall	10
align 4
mouse:
	mcall	37,2
	cmp	eax,1	; если не левая, возврат
	jne	still_mouse
; координаты курсора	
	xor	ebx,ebx
	mcall	37

	mov	ebx,eax		; eax = cursor_x
	shr	eax,16		; ebx = cursor_y
	and	ebx,0xffff
	mov	[curx1],eax	; curx1 = cursor_x
	mov	[cury1],ebx	; cury1 = cursor_y
	
; кому принадлежит точка?
	mcall	34,[curx1],[cury1]
	cmp	al,1 ; 1 - ядро
	jne	still_mouse
;--------------------------------------
align 4
@@:		; подождём, пока пользователь не отпустил правую кнопку мыши
	mcall	37,2    ;   нажаты ли кнопки мыши?
	test	eax,1   ; если отпустил
	jz	@f	;   идём в начало главного цикла

	mcall	68,1	; иначе переключимся на следующий поток системы и когда
	jmp	@b	; выполнение вернется этому потоку, проверим мышь опять
;--------------------------------------
align 4	
@@:
; координаты курсора	
	xor	ebx,ebx
	mcall	37

	mov	ebx,eax		; eax = cursor_x
	shr	eax,16		; ebx = cursor_y
	and	ebx,0xffff
	mov	[curx2],eax	; curx1 = cursor_x
	mov	[cury2],ebx	; cury1 = cursor_y
	
; кому принадлежит точка?
	mcall	34,[curx2],[cury2]
	cmp	al,1 ; 1 - ядро
	jne	still_mouse
; check for icon by click down
	call	check_icons_position
	mov	eax,[click_pos]
	test	eax,eax
	jz	still_mouse
	
	push	eax
	mov	eax,[curx2]
	mov	[curx1],eax
	mov	eax,[cury2]
	mov	[cury1],eax	
; check for icon by click up
	call	check_icons_position	
	pop	eax
	
	mov	ebx,[click_pos]
	
	test	ebx,ebx
	jz	still_mouse
	
	cmp	eax,ebx
	jne	still_mouse
;--------------------------------------
align 4
start_appl:
	mov	esi,[adress_of_icon_data_2]	;[ebp+8]
	mov	ebx,1
	mov	edi,finfo.path
	call	fill_paths
	inc	ebx
	mov	edi,param_str
	mov	dword[finfo_start+8],edi
	call	fill_paths
	cmp	byte[edi],0
	jne	.no0
	and	dword[finfo_start+8],0
;--------------------------------------
align 4
.no0:
	mov	ebx,finfo_start
	mcall	70
	jmp	still_mouse
;------------------------------------------------------------------------------
align 4
fill_paths:
	push	esi edi
	movzx	ecx,byte[str_lens+ebx]
	add	esi,[positions+ebx*4]
	push	esi
	add	esi,ecx
;--------------------------------------
align 4
.l1:
	dec	esi
	cmp	byte[esi],' '
	jnz	.found
	loop	.l1
	pop	esi
	jmp	.noms
;--------------------------------------
align 4
.found:
	lea	ecx,[esi+1]
	pop	esi
	sub	ecx,esi
	rep	movsb
;--------------------------------------
align 4
.noms:
	and	byte[edi],0
	pop	edi esi
	ret
;------------------------------------------------------------------------------	
align 4
check_icons_position:
	mov	ecx,[icons]
	mov	ebx,icon_data
;--------------------------------------
align 4
.loop:
	push	ebx ecx
	mov	[adress_of_icon_data_2],ebx
	mov	eax,[ebx]
	call	calc_icon_pos

	mov	[current_X_mouse_thread],ebx
	mov	[current_Y_mouse_thread],eax
	mov	eax,[curx1]
	mov	ebx,[current_X_mouse_thread]
	cmp	eax,ebx
	jb	.next
	
	add	ebx,52
	cmp	eax,ebx
	ja	.next
	
	mov	eax,[cury1]
	mov	ebx,[current_Y_mouse_thread]
	cmp	eax,ebx
	jb	.next
	
	add	ebx,52
	cmp	eax,ebx
	ja	.next

	pop	ecx ebx
	mov	eax,[adress_of_icon_data_2]
	mov	[click_pos],eax
	ret
;--------------------------------------
align 4
.next:
	pop	ecx ebx
	add	ebx,REC_SIZE
	dec	ecx
	jnz	.loop
; all icons survived!
	xor	eax,eax
	mov	[click_pos],eax
	ret
;------------------------------------------------------------------------------
