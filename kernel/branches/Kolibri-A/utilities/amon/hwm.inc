;
; Ghost Monitor - ?⨫??? (??????) ?????஢???? ? ??????ਭ?? ???⥬?
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;
; HWM abstract interface, provide:
;  hwm_init, hwm_get_params, hwm_enable, hwm_temps, hwm_rpms, hwm_voltages, hwm_chip_name
;


if SMBus eq yes
 include	'SMBus.inc'
end if

include 	'hwm_hudson.inc'
;include         'hwm_it87.inc'
;include         'hwm_uguru.inc'
;include        'hwm_via686.inc'

uglobal
	hwm_enable:
		dd	1		; 0 - no, 1 - Hudson,  ... see HWMProbe list
	hwm_temps:
		db	0,0, 0,0, 0,0	; ?ࠤ??, ???????, ?ࠤ??, ...
	hwm_rpms:
		dd	0, 0, 0, 0, 0	; fan1, fan2, fan3, ...
	hwm_voltages:
	Vcore	dd	0.0
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

IO_Read 	dd	pm2read 	; In : al - addr, out : al - value
IO_Write	dd	pm2write	; In : al - addr, bl - value

hwm_unk 	db	'Not found', 0
hwm_chip_name	dd	hwm_unk

; NOTE !!!
; HWMProbe - ᯨ᮪, ???楤?? ?஢?ન 稯??, ?????稢?????
; ?㫥??? ??????? ᫮???
; HWMGetParam - ᯨ᮪, ???楤?? ????祭?? ??ࠬ??஢
; ??? ??? ᯨ᪠ ?????? ᮮ⢥??⢮???? ???? ????? ??
; ?? ???浪? ???楤??
HWMProbe:
		dd	hudson_init
;                dd      wb_init
;                dd      it87_init
;                dd      uGuru_init
		dd	0		; <- ????? ᯨ᪠

HWMGetParam:
		dd	hudson_getparam
;                dd      wb_getparam
;                dd      it87_getparam
;                dd      uGuru_getparam

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
hwm_init:
; ?஢?ઠ ??????? ? ???樠???????
	pusha
	; ?饬 ???稪? ?? 設? ISA
;        call    isaInit
;        jc      @f
	call	chips_probe
;        cmp     byte[hwm_enable], 0
;        jne     .exit
;@@:     call    isaClose

	; ?饬 ???稪? ?? 設? SMBus
;        inc     byte [acc_type]
;if SMBus eq yes
;        call    smbInit
;        jc      @f
;        call    chips_probe
;        cmp     byte[hwm_enable], 0
;        jne     .exit
;@@:     call    smbClose
;end if
	; ?饬 ???稪? ?? ᮡ?⢥???? 設? (LPC etc)
;        inc     byte [acc_type]
;        call    chips_probe
	mov  [acc_type], 2	; bus type = 'other'

.exit:	popa
	ret
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
chips_probe:
; ??室?? ?㦭?? 稯
; Out : hwm_enable = !0 if found, else hwm_enable not changed
	cld			; Paranoia
	mov	esi, HWMProbe
@@:	lodsd
	test	eax, eax
	jz	@f		; ????? ᯨ᪠
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
pm2read:
	push	edx
	mov	dx, 0xCD0	;  0x295
	out	dx, al
	inc	edx
	in	al, dx
	pop	edx
	ret

pm2write:
	push	edx
	mov	dx, 0xCD0	;    295
	out	dx, al
	inc	edx
	mov	al, bl
	out	dx, al
	pop	edx
	ret

isaInit:
	ret

isaClose:
	ret
