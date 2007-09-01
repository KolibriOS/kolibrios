recode:

  .866.1251:
	mov	edi,table.866.1251
	jmp	.main
  .1251.866:
	mov	edi,table.1251.866
	jmp	.main
  .866.koi:
	mov	edi,table.866.koi
	jmp	.main
  .koi.866:
	mov	edi,table.koi.866
	jmp	.main
  .1251.koi:
	mov	edi,table.1251.koi
	jmp	.main
  .koi.1251:
	mov	edi,table.koi.1251

  .main:
	mov	ecx,[cur_editor.Lines.Count]
	mov	esi,[cur_editor.Lines]
	jecxz	.exit
	xor	eax,eax
  .lp0: dec	ecx
	js	.exit
	movzx	edx,word[esi]
	add	esi,4
    @@: dec	edx
	js	.lp0
	lodsb
	add	al,-$80
	js	@b
	mov	al,[edi+eax]
	mov	[esi-1],al
	jmp	@b
  .exit:
	ret

