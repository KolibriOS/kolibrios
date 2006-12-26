; ABIT uGuru
;
;	Chip		Temp	Volt	Fan   ISA   SMBus    LPC
;    W83L950D(?)	 3	 11	 5     -      -       +


; uGuru ports
UGURU_CMD_PORT		equ	0x0e0
UGURU_DATA_PORT 	equ	0x0e4
; temp sensors
UGURU_CPUTEMP		equ	0x2100
UGURU_SYSTEMP		equ	0x2101
UGURU_PWMTEMP		equ	0x210f
; fans. These are placed at bank 2
SENS_CPUFAN	    	equ	0x2600
SENS_NBFAN	   	equ	0x2601
SENS_SYSFAN	   	equ	0x2602
SENS_AUXFAN1    	equ	0x2603
SENS_AUXFAN2    	equ	0x2604
; Voltage sensors
SENS_VCORE		equ	0x2103	; 3.49V max
SENS_DDRVDD		equ	0x2104	; 3.49V max
SENS_DDRVTT		equ	0x210A	; 3.49V max
SENS_NBVDD		equ	0x2108	; 3.49V max
SENS_SBVDD		equ	0x210E	; 3.49V max
SENS_HTV		equ	0x2102	; 3.49V max
SENS_AGP		equ	0x2109	; 3.49V max
SENS_5V			equ	0x2106	; 6.25V max
SENS_3V3		equ	0x2105	; 4.36V max
SENS_5VSB		equ	0x210B	; 6.25V max
SENS_3VDUAL		equ	0x210D	; 4.36V max
; Voltage ADC multipliers
MLTP_3V49		equ	0.0136862745098039
MLTP_4V36		equ	0.0170980392156863
MLTP_6V25		equ	0.0245098039215686


v_addr:	dw	SENS_VCORE, SENS_DDRVDD, SENS_DDRVTT, SENS_NBVDD, SENS_SBVDD
	dw	SENS_HTV, SENS_AGP, SENS_5V, SENS_3V3, SENS_5VSB, SENS_3VDUAL
	
v_mult:	dd	MLTP_3V49, MLTP_3V49, MLTP_3V49, MLTP_3V49, MLTP_3V49, MLTP_3V49
	dd	MLTP_3V49, MLTP_6V25, MLTP_4V36, MLTP_6V25, MLTP_4V36


msg_uguru:	db	10, 'ABIT uGuru'

;-----------------------------------
uGuru_init:
; Проверка наличия и инициализация
; OUT - CF = 1 - error
	cmp	byte[acc_type], 2	; Special IO interface
	jne	.uGuru_no
	pusha
	mov	ecx, UGURU_CMD_PORT
	mov	edx, UGURU_DATA_PORT
	call	ReservePorts
	jc	.no_lpc
	in	al, UGURU_CMD_PORT
	test	al, al
	jz	@f
	cmp	al, 0xac
	jne	.bad_sig
@@:	in	al, UGURU_DATA_PORT
	test	al, al
	jz	@f
	cmp	al, 8
	jne	.bad_sig
@@:	mov	[hwm_chip_name], msg_uguru
	popa
	clc
	ret
.bad_sig:
	; popa				;
	; clc				; <- uncomment this three line, if you certain that you
	; ret				;         have ABIT uGuru but he is not defined
.no_lpc:
	mov	ecx, UGURU_CMD_PORT
	mov	edx, UGURU_DATA_PORT
	call	FreePorts
	popa
.uGuru_no:
	stc
	ret
;-----------------------------------
uGuru_getparam:
	; Temperature
	mov	ax, UGURU_CPUTEMP
	call	uGuru_ReadSensor
	mov	[hwm_temps + 0], al
	mov	ax, UGURU_SYSTEMP
	call	uGuru_ReadSensor
	mov	[hwm_temps + 1], al
	mov	ax, UGURU_PWMTEMP
	call	uGuru_ReadSensor
	mov	[hwm_temps + 2], al
	; Fan speed
	mov	ax, SENS_CPUFAN
	call	uGuru_ReadSensor
	mov	dl, 60
	mul	dl
	mov	word[hwm_rpms + 0 ], ax
	mov	ax, SENS_NBFAN
	call	uGuru_ReadSensor
	mov	dl, 60
	mul	dl
	mov	word[hwm_rpms + 4 ], ax
	mov	ax, SENS_SYSFAN
	call	uGuru_ReadSensor
	mov	dl, 60
	mul	dl
	mov	word[hwm_rpms + 8 ], ax
	mov	ax, SENS_AUXFAN1
	call	uGuru_ReadSensor
	mov	dl, 60
	mul	dl
	mov	word[hwm_rpms + 12], ax
	mov	ax, SENS_AUXFAN2
	call	uGuru_ReadSensor
	mov	dl, 60
	mul	dl
	mov	word[hwm_rpms + 16], ax
	; voltages
	cld				; Paranoia
	xor	ecx, ecx
	mov	esi, v_addr
@@:	push	ecx
	lodsw
	call	uGuru_ReadSensor
	movzx	eax, al
	fld	dword[v_mult + ecx * 4]
	push	eax
	fild	dword [esp]
	fmulp	st1, st0
	fstp	dword [hwm_voltages + ecx * 4]
	pop	eax
	pop	ecx
	inc	ecx
	cmp	ecx, 11			; <-- количество датчиков (ADC Vin)
	jne	@b

	ret
;-----------------------------------
uGuru_ReadSensor:
; Read a sensor
; In  : ax - sensorID
; Out : al - result
	mov	ebx, eax		; Ask to read from uGuru
	shr	eax, 8
	out	UGURU_DATA_PORT, al
	mov	ecx, 1000		; I guess this is to se if uGuru is ready to take a command
.is_ready:
	in	al, UGURU_DATA_PORT
	dec	ecx
	jnz	@f
	xor	al, al
	ret
@@:	cmp	al, 8
	jne	.is_ready
	mov	al, bl			; Whitch sensor to read?
	out	UGURU_CMD_PORT, al
	mov	ecx, 1000		; Wait until uGuru is ready to be read
.is_ready2:
	in	al, UGURU_DATA_PORT
	dec	ecx
	jnz	@f
	xor	al, al
	ret
@@:	cmp	al, 1
	jne	.is_ready2
	in	al, UGURU_CMD_PORT	; Read the sensor
	push	eax
	call	uGuru_Ready		; Put the chip in ready state
	pop	eax			; Return the result of the sensor
	ret
;-----------------------------------
uGuru_Ready:
; Put uGuru in ready state. uGuru will hold 0x08 at Data port and 0xAC at Command port after this.
; Out : CF = 1 - error
	; Wait until uGuru is in ready-state
	; The loop shouldn't be needed to execut more then one time
	mov	edx, 1000
.nxt:	in	al, UGURU_DATA_PORT
	cmp	al, 8
	je	.ok
	xor	al, al			; after 0x00 is written to Data port
	out	UGURU_DATA_PORT, al
	mov	ecx, 1000		; Wait until 0x09 is read at Data port
.is_ready:
	in	al, UGURU_DATA_PORT
	dec	ecx
	jnz	@f
	stc
	ret
@@:	cmp	al, 9
	jne	.is_ready
	mov	ecx, 1000		; Wait until 0xAC is read at Cmd port
.is_ready2:
	in	al, UGURU_CMD_PORT
	dec	ecx
	jnz	@f
	stc
	ret
@@:	cmp	al, 0xAC
	jne	.is_ready2
	dec	edx			; Prevent a lockup
	jnz	.nxt
	stc
	ret
.ok:	clc
	ret

