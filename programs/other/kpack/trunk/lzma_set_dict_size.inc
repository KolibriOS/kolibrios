;* Call: ***************************************************************
; lzma_set_dict_size(unsigned logdictsize)
lzma_set_dict_size:
	mov	ecx,[esp+4]
	xor	eax,eax
	inc	eax
	shl	eax,cl
	mov	[_dictionarySize],eax
	lea	eax,[ecx+ecx]
	mov	[_distTableSize],eax
	ret	4
;*****************************************************************************
