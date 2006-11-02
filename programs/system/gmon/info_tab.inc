;
;
; Вкладка информации о системе
;
; Дописать ещё чтоить (маловато будет)
;
;

	; Пишем параметры
	mov	edx, lab_info_tab
	mov	ebx, 17 * 65536 + 265
	call	show_text

	; Пишем производителя процессора
	mov	edx, Vendor
	mov	esi, 12
	mov	ebx, 160 * 65536 + 265
	int	0x40
	; Поддержка MMX
	mov	ebx, 160 * 65536 + 275
	mov	esi, 3
	mov	edx, msg_yes
	cmp	byte[CPU_mmx], 1
	je	mmx_en
	mov	edx, msg_no
mmx_en:	int	0x40
	; Чип мониторинга
	int	0x40
	mov	edx, [hwm_chip_name]
	mov	al, byte[edx]
	movzx	esi, al
	inc	edx
	mov	ebx, 160 * 65536 + 285
	mov	al, 4
	int	0x40
	; Семейство и модель процессора
	mov	eax, 47
	mov	ebx, 0x10100
	xor	ecx, ecx
	mov	cl, [CPU_fam]
	mov	edx, 250 * 65536 + 265
	xor	esi, esi
	int	0x40
	mov	cl, [CPU_mod]
	add	edx, 10 * 65536
	int	0x40
	
	; коэффициент умножения
	call	multiplier
	test	ecx, ecx
	jz	@f
	movzx	ecx, cl
	push	ecx
	fild	dword[esp]
	push	10
	fidiv	dword[esp]
	fstp	dword[esp]
	mov	esi, esp
	call	ftoa
	mov	ebx, 160 * 65536 + 295
	mov	edx, Data_String
	mov	eax, 4
	mov	esi, eax
	xor	ecx, ecx
	int	0x40
	pop	eax
	pop	eax
@@:
	; Свободная память
if (OS_version >= 0x0530)
	mov	eax, 18
	mov	ebx, 16
	int	0x40
	mov	ecx, eax
	call	digit_len
	mov	eax, 47
	mov	edx, 160 * 65536 + 305
	xor	esi, esi
	int	0x40
end if
	
	; Расставляем двоеточия
	mov	eax, 4
	mov	ebx, 152 * 65536 + 265
	mov	edx, dub_p
	mov	esi, 1
	mov	ecx, 5		; <- количество строк
nxt_dp:	push	ecx
	mov	ecx, 0x10000000
	int	0x40
	add	ebx, 10
	pop	ecx
	loop	nxt_dp
;---------------------------------------------------------------------	
	jmp	end_drow_tab
dub_p	db	':'

lab_info_tab mls \
	'CPU Vendor',\
	'MMX tehnology',\
	'Monitoring chip',\
	'Multiplier',\
	'Free memory (Kb)'
