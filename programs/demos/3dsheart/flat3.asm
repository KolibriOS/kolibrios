draw_triangle:
    ;----------in - eax - x1 shl 16 + y1
    ;------------- -ebx - x2 shl 16 + y2
    ;---------------ecx - x3 shl 16 + y3
    ;---------------edx - color 0x00rrggbb
    ;---------------edi - pointer to screen buffer
    @ch3:
	cmp	ax,bx
	jg	@ch1
    @ch4:			; sort parameters
	cmp	bx,cx
	jg	@ch2
	jle	@chEnd
    @ch1:
	xchg	eax,ebx
	jmp	@ch4
    @ch2:
	xchg	ebx,ecx
	jmp	@ch3
    @chEnd:
	mov	dword[@y1],eax	; ....and store to user friendly  variables
	mov	dword[@y2],ebx
	mov	dword[@y3],ecx
	mov	[@col],edx
	mov	edx,eax 	; eax,ebx,ecx are ORd together into edx which means that
	or	edx,ebx 	; if any *one* of them is negative a sign flag is raised
	or	edx,ecx
	test	edx,80008000h	; Check both X&Y at once
	jne	@end_triangle

	cmp	[@x1],SIZE_X	; {
	jg	@end_triangle
	cmp	[@x2],SIZE_X	 ; This can be optimized with effort
	jg	@end_triangle
	cmp	[@x3],SIZE_X
	jg	@end_triangle	; }

	shr	eax,16
	shr	ebx,16
	shr	ecx,16

	neg	ax		; calculate delta 12
	add	ax,bx
	cwde
	shl	eax,ROUND
	cdq
	mov	bx,[@y2]
	mov	cx,[@y1]
	sub	bx,cx
	;cmp     ebx,0
	jne	@noZero1
	mov	[@dx12],0
	jmp	@yesZero1
    @noZero1:
	idiv	ebx
	mov	[@dx12],eax
    @yesZero1:

	mov	ax,[@x3]	; calculate delta 13
	sub	ax,[@x1]
	cwde
	shl	eax,ROUND
	cdq
	mov	bx,[@y3]
	mov	cx,[@y1]
	sub	bx,cx
	;cmp     ebx,0
	jne	@noZero2
	mov	[@dx13],0
	jmp	@yesZero2
    @noZero2:
	idiv	ebx
	mov	[@dx13],eax
    @yesZero2:

	mov	ax,[@x3]     ; calculate delta 23 [dx23]
	sub	ax,[@x2]
	cwde
	shl	eax,ROUND
	cdq
	mov	bx,[@y3]
	mov	cx,[@y2]
	sub	bx,cx
	;cmp     ebx,0
	jne	@noZero3
	mov	[@dx23],0
	jmp	@yesZero3
    @noZero3:
	idiv	ebx
	mov	[@dx23],eax
    @yesZero3:

	movzx	eax,word[@x1]	; eax - xk1
	shl	eax,ROUND
	mov	ebx,eax 	; ebx - xk2
	movzx	esi,word[@y1]	; esi - y
    @next_line1:
	mov	ecx,eax 	; ecx - x11
	sar	ecx,ROUND
	mov	edx,ebx 	; edx - x12
	sar	edx,ROUND
	cmp	ecx,edx
	jle	@nochg
	xchg	ecx,edx
    @nochg:
	pusha
	mov	ebx,ecx
	sub	edx,ecx
	mov	ecx,edx
	mov	edx,esi
	mov	eax,[@col]
	call	@horizontal_line
	popa
	add	eax,[@dx13]
	add	ebx,[@dx12]
	inc	esi
	cmp	si,[@y2]
	jl	@next_line1

	movzx	esi,word[@y2]
	movzx	ebx,word[@x2]
	shl	ebx,ROUND
    @next_line2:
	mov	ecx,eax
	sar	ecx,ROUND
	mov	edx,ebx
	sar	edx,ROUND
	cmp	ecx,edx
	jle	@nochg1
	xchg	ecx,edx
    @nochg1:
	pusha
	mov	ebx,ecx
	sub	edx,ecx
	mov	ecx,edx
	mov	edx,esi
	mov	eax,[@col]
	call	@horizontal_line
	popa
	add	eax,[@dx13]
	add	ebx,[@dx23]
	inc	esi
	cmp	si,[@y3]
	jl	@next_line2
    @end_triangle:
ret

@horizontal_line:
    ;---------in
    ;---------eax - color of line,  0x00RRGGBB
    ;---------ebx - x1 - x position of line begin
    ;---------ecx - lenght of line
    ;---------edx - y position of line
    ;---------edi - pointer to buffer
	jcxz	@end_hor_l
;        or      edx,edx
;        jl      @end_hor_l
	cmp	edx,SIZE_Y
	jg	@end_hor_l
	push	eax
	mov	eax,SIZE_X*3
	mul	edx
	add	edi,eax 	; calculate line begin adress
	;add     edi,ebx
	;shl     ebx,1
	lea	edi,[edi+ebx*2]
	add	edi,ebx
	pop	eax
	cld
	;mov dword[edi-3],0000FF00h
    @ddraw:			; Drawing horizontally:
	;push    eax
	stosd			; 4 bytes at a time
	dec edi 		; point to the 4th
	;shr     eax,16
	;stosb
	;pop     eax
	;pusha                    ; If you want to draw pixel-by-pixel
	;    mov  eax,7           ; put image
	;    mov  ebx,screen
	;    mov  ecx,SIZE_X shl 16 + SIZE_Y
	;    mov  edx,5 shl 16 + 20
	;    int  0x40
	;popa
	loop	@ddraw
	mov	byte[edi],0	; The last 4th will be reset
    @end_hor_l:
ret
