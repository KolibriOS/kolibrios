
include		'hwm_wb.inc'
include		'hwm_it87.inc'

uglobal
	hwm_enable:db	0		; 0 - no, 1 - Winbond, 2 - ITE
	temps	db	0,0, 0,0, 0,0	; градус, дес€тые, градус, ...
	rpms	dd	0, 0, 0		; fan1, fan2, fan3
	Vcore 	dd	0.0
	Vin0	dd	0.0
	Vin1	dd	0.0
	AVcc	dd	0.0
	Vin2	dd	0.0
	V12	dd	0.0
	V5	dd	0.0
endg

hwm_unk		db	9, 'Not found'
hwm_chip_name	dd	hwm_unk

hwm_init:
; ѕроверка наличи€ и инициализаци€
	pusha

	mov	eax, 46		; резервируем 0x295 и 0x296 порты
	xor	ebx, ebx
	mov	ecx, 0x295
	mov	edx, 0x296
	int	0x40
	test	eax, eax
	jz	hwm_ports_ok
hwm_no:
	mov	eax, 46		; освобождаем 0x295 и 0x296 порты
	xor	ebx, ebx
	inc	ebx
	mov	ecx, 0x295
	mov	edx, 0x296
	int	0x40
	popa
	ret
hwm_ports_ok:

	call	wb_init
	jc	not_wb
	mov	byte [hwm_enable], 1
	call	wb_get_name
	jmp	hwm_ok
	
not_wb:	call	it87_init
	jc	hwm_no
	mov	byte [hwm_enable], 2
	call	it87_get_name
	
hwm_ok:	mov	[hwm_chip_name], edx
	popa
	ret
;----------------------------------------------------
hwm_get_params:
; ¬ызывать только если найден
	pusha
	mov	al, [hwm_enable]
	dec	al
	jnz	@f
	call	wb_get_temp
	call	wb_get_fan_speed
	mov	edi, wb_coeff
	call	wb_get_volt
	fld	dword[V12]
	fld	dword[wb_n12v_const]
	faddp	st1, st0
	fstp	dword[V12]
	popa
	ret
@@:	call	it87_get_temp
	call	it87_get_fan_speed
	mov	edi, ite_coeff
	call	wb_get_volt
	popa
	ret
;----------------------------------------------------
