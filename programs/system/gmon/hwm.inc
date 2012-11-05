;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;
; HWM abstract interface, provide:
;  hwm_init, hwm_get_params, hwm_enable, hwm_temps, hwm_rpms, hwm_voltages, hwm_chip_name
;


if SMBus eq yes
 include	'SMBus.inc'
end if

include		'hwm_wb.inc'
include		'hwm_it87.inc'
include		'hwm_uguru.inc'
;include	'hwm_hudson.inc'

uglobal
	hwm_enable:
		dd	0		; 0 - no, 1 - Winbond, 2 - ITE, ... see HWMProbe list
	hwm_temps:
		db	0,0, 0,0, 0,0	; градус, десятые, градус, ...
	hwm_rpms:
		dd	0, 0, 0, 0, 0	; fan1, fan2, fan3, ...
	hwm_voltages:
	Vcore 	dd	0.0
	Vin0	dd	0.0
	Vin1	dd	0.0
	AVcc	dd	0.0
	Vin2	dd	0.0
	V12	dd	0.0
	V5	dd	0.0
	vol8	dd	0.0
	vol9	dd	0.0
	vol10	dd	0.0
	vol11	dd	0.0
	acc_type db	0		; 0 - ISA Bus, 1 - SMBus, 2 - other
endg

IO_Read		dd	isaRead		; In : al - addr, out : al - value
IO_Write	dd	isaWrite	; In : al - addr, bl - value

hwm_unk		db	'Not found', 0
hwm_chip_name	dd	hwm_unk

; NOTE !!!
; HWMProbe - список, процедур проверки чипов, заканчивается
; нулевым двойным словом
; HWMGetParam - список, процедур получения параметров
; оба эти списка должны соответствовать друг другу по
; по порядку процедур
HWMProbe:	
        dd	wb_init
		dd	it87_init
		dd	uGuru_init
;        dd  hudson_init
		dd	0		; <- конец списка
		
HWMGetParam:	
        dd	wb_getparam
		dd	it87_getparam
		dd	uGuru_getparam
;        dd  hudson_getparam
		
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hwm_init:
; Проверка наличия и инициализация
	pusha
	; Ищем датчики на шине ISA
	call	isaInit
	jc	@f
	call	chips_probe
	cmp	byte[hwm_enable], 0
	jne	.exit
@@:	call	isaClose

	; Ищем датчики на шине SMBus
	inc	byte [acc_type]
if SMBus eq yes
	call	smbInit
	jc	@f
	call	chips_probe
	cmp	byte[hwm_enable], 0
	jne	.exit
@@:	call	smbClose
end if
	; Ищем датчики на собственной шине (LPC etc)
	inc	byte [acc_type]
	call	chips_probe

.exit:	popa
	ret
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=	
chips_probe:
; Находим нужный чип
; Out : hwm_enable = !0 if found, else hwm_enable not changed
	cld			; Paranoia
	mov	esi, HWMProbe
@@:	lodsd
	test	eax, eax
	jz	@f		; Конец списка
	call	eax
	jc	@b
	sub	esi, HWMProbe
	shr	esi, 2
	mov	dword[hwm_enable], esi
@@:	ret
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hwm_get_params:
	pusha
	mov	eax, [hwm_enable]
	test	eax, eax
	jz	@f
	dec	eax
	sal	eax, 2
	call	dword[eax + HWMGetParam]
@@:	popa
	ret

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; ISA interface
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
isaRead:
	push	edx
	mov	dx, 0x295
	out	dx, al
	inc	edx
	in	al, dx
	pop	edx
	ret
	
isaWrite:
	push	edx
	mov	dx, 0x295
	out	dx, al
	inc	edx
	mov	al, bl
	out	dx, al
	pop	edx
	ret

isaInit:
	; Out : CF = 0 if Ok
	; резервируем 0x295 и 0x296 порты
	mov	ecx, 0x295
	mov	edx, 0x296
	call	ReservePorts
	; jc	@f
	; mov	[IO_Read], isaRead
	; mov	[IO_Write], isaWrite
	; @@:	
	ret

isaClose:
	; освобождаем 0x295 и 0x296 порты
	mov	ecx, 0x295
	mov	edx, 0x296
	call	FreePorts
	ret
