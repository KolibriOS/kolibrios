;* Call: ***************************************************************
lzma_set_dict_size:
	mov	ecx,[esp+4]
	xor	eax,eax
	inc	eax
	shl	eax,cl
	mov	[0x29568],eax
	lea	eax,[ecx+ecx]
	mov	[0x29570],eax
	ret	4
;*****************************************************************************
