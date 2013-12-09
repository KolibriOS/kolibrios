proc MovingWnd
local MySlot:DWORD
	mcall	40,0	;SetMaskMessage 000b

	mcall	12,1	;RedrawWinBeg

	mcall	14
	xor	ebx,ebx
	xor	ecx,ecx
	mov	cx,ax
	shr	eax,16
	mov	cx,ax
	dec	ebx
	dec	ecx
	mcall	0,,,01000000h
	mcall	12,2	;RedrawWinEnd


	mov	eax,[SelIcon]
	call	DrawIconToArea

	mcall	37,4,IconArea,10100002h
	push	eax
	test	eax,eax
	jz	@f
	mov	ecx,eax
	mcall	37,5

     @@:
	mcall	9,RBProcInfo,-1    ;+30
	mcall	18,21,dword[RBProcInfo+30]
	mov	[MySlot],eax

  .messages:
	cmp	[MovingActiv],1
	jne	.close
	mcall	18,7
	cmp	eax,[MySlot]
	je	@f
	mcall	18,3,[MySlot]
     @@:

	mcall	5,1	;Yield
	jmp	.messages

.close:
	pop	ecx
	test	ecx,ecx
	jz	@f
	mcall	37,6
     @@:


	mcall	-1

endp



;eax = NumIcon
proc DrawIconToArea  ;предназначен для иконок размером = 32x32
	push	edi esi

	;mov     eax,[NumIcon]
	mov	edi,[IconsOffs+eax*4]
	xor	al,al
	or	ecx,-1
	repne	scasb
	repne	scasb
	repne	scasb
	mov	al,[edi]
	cmp	al,'9'
	ja	.PathToIcon
	cmp	al,'/'
	jne	.GetIconInd
   .PathToIcon:
	mov	al,30h		 ;заглушка!!!!!!!!!!!!!
	mov	byte[edi+1],0
   .GetIconInd:
;int3
	sub	al,30h
	cmp	byte[edi+1],0
	je	@f
	shl	eax,1
	lea	eax,[eax*4+eax]
	xor	edx,edx
	mov	dl,[edi+1]
	sub	dl,30h
	add	eax,edx
     @@:	     ;eax=num icon
	cmp	eax,[icon_count]
	jb	@f
	xor	eax,eax
     @@:
	test	eax,eax
	je	.DIA
	mov	ecx,eax
	xor	eax,eax
      @@:
	add	eax,32*32*4;IMG_SIZE*IMG_SIZE*4
	loop	@b
     .DIA:
	mov	esi,eax
	add	esi,[raw_pointer]
	add	esi,44

	mov	edi,IconArea;+((IMAGE_TOP_Y*ICON_SIZE)+((ICON_SIZE-IMG_SIZE)/2))*4
;int3
	mov	eax,32;IMG_SIZE
	mov	edx,eax
      @@:
	mov	ecx,eax
	rep	movsd
     ;   add     edi,32*4
	dec	edx
	jnz	@b

	pop	esi edi
	ret
endp
