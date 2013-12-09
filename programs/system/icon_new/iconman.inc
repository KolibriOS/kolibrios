;ret eax=numIcon
proc AddIcon stdcall,x,y,lpIcon,lpName,lpExeFile,lpParams
	push	ebx edi esi

	mov	eax,[x]
	mov	ebx,[y]

	xor	edx,edx
	test	eax,8000h
	jnz	@f
	mov	dx,[wsW]    ;если надо, то преобразовываем в
	shr	edx,1		;отрицательные координаты
	add	dx,[wsX]
	cmp	eax,edx
	jbe	@f
	sub	ax,[wsW]
	inc	eax
      @@:

	test	ebx,8000h
	jnz	@f
	mov	dx,[wsY]
	shr	edx,1
	add	dx,[wsH]
	cmp	ebx,edx
	jbe	@f
	sub	bx,[wsH]
	inc	ebx
      @@:
				;поправка на случай выхода за край экрана

	test	eax,8000h
	jnz	@f
	mov	dx,[wsXe]
	sub	edx,ICON_SIZE
	cmp	eax,edx
	jbe	@f
	mov	eax,edx
     @@:

	test	ebx,8000h
	jnz	@f
	mov	dx,[wsYe]
	sub	edx,ICON_SIZE
	cmp	ebx,edx
	jbe	@f
	mov	ebx,edx
     @@:

	mov	[x],eax
	mov	[y],ebx

	xor	al,al
	or	ecx,-1
	mov	edi,[lpName]
	repne	scasb
	sub	edi,[lpName]
	mov	ebx,edi
	mov	edi,[lpExeFile]
	repne	scasb
	sub	edi,[lpExeFile]
	add	ebx,edi
	mov	edi,[lpParams]
	repne	scasb
	sub	edi,[lpParams]
	add	ebx,edi
	mov	edi,[lpIcon]
	repne	scasb
	sub	edi,[lpIcon]
	add	ebx,edi
	add	ebx,4
	mov	ecx,dword[SizeData]
	add	ecx,ebx
	mov	edx,[IconsOffs]
	mcall	68,20

	mov	edx,dword[SizeData]
	mov	dword[SizeData],ecx
	mov	dword[BegData],eax
	mov	edi,eax
	add	edi,edx
	mov	esi,[lpName]
    @@: lodsb
	stosb
	test	al,al
	jnz	@b

	mov	esi,[lpExeFile]
    @@: lodsb
	stosb
	test	al,al
	jnz	@b

	mov	esi,[lpParams]
    @@: lodsb
	stosb
	test	al,al
	jnz	@b

	mov	esi,[lpIcon]
    @@: lodsb
	stosb
	test	al,al
	jnz	@b

	mov	eax,[x]
	mov	ebx,[y]
	shl	eax,16
	mov	ax,bx
	mov	dword[edi],eax

	stdcall FillIconsOffs

	pop	esi edi ebx
	ret
endp

proc EditIcon stdcall,NumIcon,lpIcon,lpName,lpExeFile,lpParams
	push	edi
	mov	eax,[NumIcon]
	mov	edi,[IconsOffs+eax*4]
	xor	al,al
	or	ecx,-1
	repne	scasb
	repne	scasb
	repne	scasb
	repne	scasb
	push	dword[edi]
	stdcall DelIcon,[NumIcon]
	pop	edx
	xor	eax,eax
	mov	ax,dx
	shr	edx,16
	stdcall AddIcon,edx,eax,DAreaIcon,DAreaName,DAreaPath,DAreaParams
	pop	edi
	ret
endp

proc SetPosition stdcall,NumIcon,x,y
	push	edi
	mov	eax,[NumIcon]
	mov	edi,[IconsOffs+eax*4]
	xor	al,al
	or	ecx,-1
	repne	scasb
	repne	scasb
	repne	scasb
	repne	scasb
	mov	eax,[x]
	shl	eax,16
	mov	ax,word[y]
	mov	dword[edi],eax
	pop	edi
	ret
endp

proc GetNumIcon stdcall,x,y,NumIconI  ;номер иконки, который проверять не надо или -1
local posX:WORD,\
      posY:WORD
	push	ebx edi
	mov	ecx,[MaxNumIcon]
	test	ecx,ecx
	jnz	@f
	or	eax,-1
	pop	edi ebx
	ret
     @@:

	mov	ebx,ecx
	dec	ebx
   .TestIcon:
	cmp	dword[NumIconI],-1
	je	@f

	cmp	ebx,[NumIconI]
	jne	@f
	dec	ebx
	dec	ecx
	test	ecx,ecx
	jz	.NoIcon
     @@:

	push	ecx

	or	ecx,-1
	xor	al,al
	mov	edi,[IconsOffs+ebx*4]
	repne	scasb
	repne	scasb
	repne	scasb
	repne	scasb

	mov	ax,[edi+2]
	test	ax,8000h
	jz	@f
	add	ax,[wsXe]
	jmp	.gni1
      @@:
	add	ax,[wsX]
     .gni1:
	mov	[posX],ax

	mov	ax,[edi]
	test	ax,8000h
	jz	@f
	add	ax,[wsYe]
	jmp	.gni2
      @@:
	add	ax,[wsY]
     .gni2:

	mov	[posY],ax

	mov	eax,[x]
	mov	edx,[y]

	cmp	ax,[posX]
	jb	@f
	add	word[posX],ICON_SIZE
	cmp	ax,[posX]
	ja	@f

	cmp	dx,[posY]
	jb	@f
	add	word[posY],ICON_SIZE
	cmp	dx,[posY]
	ja	@f

	jmp	.OkIcon
     @@:

	dec	ebx
	pop	ecx

	;loop    .TestIcon
	dec	ecx
	jnz	.TestIcon
	jmp	.NoIcon
   .OkIcon:
	mov	eax,ebx
	pop	edi ebx
	ret
   .NoIcon:
	or	eax,-1
	pop	edi ebx
	ret
endp

proc DelIcon stdcall,NumIcon
	push	ebx edi esi

	cmp	[MaxNumIcon],0
	je	.end

	cmp	[MaxNumIcon],1
	je	.OhneIco

	mov	ebx,[NumIcon]

	mov	eax,[MaxNumIcon]
	dec	eax
	mov	[MaxNumIcon],eax
	cmp	ebx,eax
	je	@f

	shl	ebx,2

	mov	ecx,dword[BegData]
	add	ecx,dword[SizeData]
	sub	ecx,[IconsOffs+4+ebx]

	mov	edx,[IconsOffs+4+ebx]
	sub	edx,[IconsOffs+ebx]
	sub	dword[SizeData],edx

	mov	esi,[IconsOffs+4+ebx]
	mov	edi,[IconsOffs+ebx]
	rep	movsb
	jmp	.endDel

     @@:
	mov	ecx,dword[BegData]
	add	ecx,dword[SizeData]
	sub	ecx,[IconsOffs+ebx*4]
	sub	dword[SizeData],ecx

  .endDel:
	stdcall FillIconsOffs
	jmp	.end

.OhneIco:
	mov	edi,[BegData]
	mov	[SizeData],0
	mov	dword[edi],0
	mov	[MaxNumIcon],0
	mov	dword[IconsOffs],0
   .end:
	mcall	15,3
	pop	esi edi ebx
	ret
endp