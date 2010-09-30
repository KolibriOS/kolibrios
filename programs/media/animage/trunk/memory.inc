;----------------------------------------------------------
;--------change size of memory which use program-----------
;----------------------------------------------------------
;GetMemory:
;	pushad
;	mov	eax,64
;	mov	ebx,1
;	mov	ecx,[Picture_SizeX]
;	mov	edx,[Picture_SizeY]
;	imul	ecx,edx
;	lea	ecx,[ecx+ecx*2]
;	lea	ecx,[ecx+ecx*4]		;(Picture_SizeX*Picture_SizeY*3)*5
;	add	ecx,[ScreenPointer]
;	add	ecx,(1200*1000)*3+50*(20*20*3)+500000+16000+0x4000 ; 4192384
;	add	ecx,[extended_memory]
;	mcall
;	test	eax,eax
;	jz	have_new_memory
;	mov	esi,sound_havent_memory
;	call	sound
;	jmp	still
;have_new_memory:
;	popad
;	ret

GetMemory:
	pushad
	mov	ecx,[Picture_SizeX]
	mov	edx,[Picture_SizeY]
	imul	ecx,edx
	lea	ecx,[ecx+ecx*2]
	lea	ecx,[ecx+ecx*4]		;(Picture_SizeX*Picture_SizeY*3)*5
	add	ecx,(1200*1000)*3+50*(20*20*3)+500000+16000+0x4000 ; 4192384
	mcall	68,20,,[ScreenPointer]
	test	eax,eax
	jnz	have_new_memory
	mov	esi,sound_havent_memory
	call	sound
	pushad
	jmp	still

have_new_memory:
	mov	[ScreenPointer],eax
	call	prepare_data_pointers
	popad
	ret
;---------------------------------------------------------
prepare_data_pointers:
	mov	eax,[Picture_SizeX]
	mov	ebx,[Picture_SizeY]
	imul	eax,ebx
	lea	eax,[eax+eax*2]
	mov	ebx,[ScreenPointer]
	add	ebx,(1200*1000*3)+50*(20*20*3)+500000

	mov	[PointerToPicture],ebx
	mov	[PointerToCopyPicture],ebx
	mov	[PointerToCopyPicture2],ebx
	mov	[PointerToEditBufer],ebx
	mov	[PointerToSpriteBufer],ebx

	add	[PointerToCopyPicture],eax

	add	[PointerToCopyPicture2],eax
	add	[PointerToCopyPicture2],eax

	add	[PointerToEditBufer],eax
	add	[PointerToEditBufer],eax
	add	[PointerToEditBufer],eax

	add	[PointerToSpriteBufer],eax
	add	[PointerToSpriteBufer],eax
	add	[PointerToSpriteBufer],eax
	add	[PointerToSpriteBufer],eax
	ret
;---------------------------------------------------------