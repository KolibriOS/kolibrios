;
; Ghost Monitor - ??????? (??????) ???????????? ? ??????????? ???????
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

; struc DiagramData size,color
; {
      ; .color dd color
      ; times size db 0 ;;change to value > 100
; }

;------------------------------------------------------
d_show:
; esi - pointer to struct
	; ???? ???? ????, ?? ?? ??????
	cmp	dword[esi], tcol
	jne	@f
	ret
@@:	pusha

	mov	edx, [esi]
	add	esi, 4
	mov	edi, esi
	add	edi, 285

	mov	ebx, 12 * 65536 + 13
	xor	eax, eax
	lodsb
	mov	cx, 240
	sub	ecx, eax

@@:	shl	ecx, 16
	mov	cx, 240
	lodsb
	sub	ecx, eax	
	mov	eax, 38
	int	0x40
	add	ebx, 0x10001
	cmp	esi, edi
	jle	@b

	popa
	ret
;------------------------------------------------------
d_add:
; esi - pointer to struct
;  al - value
	pusha
	cmp	byte [diag_add_v], 0	; ????????? ?????? ?? ???????
	je	d_add_end
	cld
	mov	ecx, 285
	add	esi, 4
	mov	edi, esi
	inc	esi
	rep movsb
	cmp	al, 100
	jbe	@f
	mov	al, 100
@@:	mov	[edi], al
d_add_end:
	popa
	ret
