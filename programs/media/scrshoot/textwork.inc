macro use_text_work
{
;edi - 㪠??⥫? ?? ??ப?, ??࠭?祭??? ?㫥?
;??᫮ ??????頥??? ? eax
zstr_to_int:
	xor	eax,eax
	mov	ecx,10
.next:
	movsx	ebx,byte [edi]
	inc	edi
	test	ebx,ebx
	jz	.end_str
	cmp	bl,'9'
	jg	.no_figure
	cmp	bl,'0'
	jl	.no_figure
	sub	ebx,'0'
	mul	ecx
	add	eax,ebx
.no_figure:
	jmp	.next
.end_str:
ret

;edi - 㪠??⥫? ?? ??ப?
;eax - ??᫮
;ebx - ??????᢮ ??????
int_to_str:
	mov	ecx,10
.next:
	test	ebx,ebx
	jz	.end
	xor	edx,edx
	div	ecx
	add	dl,'0'
	dec	ebx
	mov	[edi+ebx],dl
	jmp	.next
.end:
ret
}