; <--- description --->
; compiler:     FASM 1.67.23
; name:         Mouse Emulation For KolibriOS
;-----------------------------------------------------------------------------
; version:	1.1
; last update:  26/05/2012
; written by:   Lipatov Kirill aka Leency
; changes:      shows notify with instructions, while opening program
;-----------------------------------------------------------------------------
; version:	1.0
; last update:  04/09/2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      total optimization of code,
;               different events for Up and Down of key Num 5
;               advansed acceleration mode
;-----------------------------------------------------------------------------
; version:      0.8
; last update:  03/10/2007
; written by:   Zlobny_Gluk
; e-mail:       umbrosia@land.ru
;-----------------------------------------------------------------------------
; <--- include all KolibriOS stuff --->
include "lang.inc"
include '../../../macros.inc'

; <--- start of KolibriOS application --->
MEOS_APP_START

; <--- start of code --->
CODE
;-----------------------------------------------------------------------------
start:
    mov eax,70
    mov ebx,notifyapp
    mcall
	
start1:
	xor	ebx,ebx
	mcall	40
@@:
	mcall	23,10	

	mcall	66,3
	test	eax,0x80
	jz	@r

	mov	ebx,4
	call	hotkeys_common

	mcall	40,10b
still:
	mcall	10
	
	cmp	eax,2		; if event == 2
	jne	still
;-----------------------------------------------------------------------------
key:				   ; key event handler
	call	mouse_acceleration

	mcall	2		   ; get key code
	
	push	eax
	mcall	66,3
	test	eax,0x80
	pop	eax
	jnz	@f
	mov	ebx,5
	call	hotkeys_common	
	jmp	start1
@@:
	cmp	al,2
	jne	still

	xor	edx,edx

	cmp	ah,80	; Down Num 2
	je	keytwo
	cmp	ah,81	; Down Num 3
	je	keythree
	cmp	ah,75	; Down Num 4
	je	keyfour
	cmp	ah,77	; Down Num 6
	je	keysix
	cmp	ah,71	; Down Num 7
	je	keyseven
	cmp	ah,72	; Down Num 8
	je	keyeight
	cmp	ah,73	; Down Num 9
	je	keynine
	cmp	ah,76	; Down Num 5
	je	keyfive
	cmp	ah,204	; Up Num 5
	je	keyfive_1
	cmp	ah,79	; Down Num 1
	jne	still
;-----------------------------------------------------------------------------
keyone:
	call	down
keyfour:
	call	left
	jmp	mouseread
;-----------------------------------------------------------------------------
keythree:
	call	right
keytwo:
	call	down
	jmp	mouseread
;-----------------------------------------------------------------------------
keyseven:
	call	left
keyeight:
	call	up
	jmp	mouseread
;-----------------------------------------------------------------------------
keynine:
	call	up
keysix:
	call	right
	jmp	mouseread
;-----------------------------------------------------------------------------
keyfive:
	inc	edx
keyfive_1:
	mcall	18,19,5
	jmp	still
;-----------------------------------------------------------------------------
left:
	mov	eax,esi
	shl	eax,16
	sub	edx,eax
	ret
;-----------------------------------------------------------------------------
right:
	mov	eax,esi
	shl	eax,16
	add	edx,eax
	ret
;-----------------------------------------------------------------------------
down:
	add	edx,esi
	ret
;-----------------------------------------------------------------------------
up:
	sub	edx,esi
	ret
;-----------------------------------------------------------------------------
mouseread:
	xor	ebx,ebx
	mcall	37
	add	edx,eax

sravn:
	xor	ebx,ebx
	xor	edi,edi

real:
	mov	ebx,edx
	mov	edi,ebx
	shr	ebx,16 ; get x1
;	shl	edi,16 ; get y1
;	shr	edi,16
	and	edi,0xffff

nullli:
	add	ebx,16
	cmp	ebx,65535
	jg	xmin
	sub	ebx,15

	add	edi,16
	cmp	edi,65535
	jg	ymin
	sub	edi,15

razr:
	mcall	14

	mov	ecx,eax
	shr	eax,16 ; get x2
;	shl	ecx,16 ; get y2
;	shr	ecx,16
	and	ecx,0xffff

rightdownli:
	cmp	eax,ebx
	jl	xmax
	cmp	ecx,edi
	jl	ymax

mousewrite:
	mcall	18,19,4
	mcall	26,9
	mov	[mouse_timer_ticks],eax
	jmp	still
;-----------------------------------------------------------------------------
mouse_acceleration:
	xor	esi,esi
	inc	esi
	mcall	18,19,2
	mov	ecx,eax
	mcall	26,9
	sub	eax,[mouse_timer_ticks]
	cmp	eax,ecx  ; mouse_delay
	ja	@f
	xor	ecx,ecx
	mcall	18,19	; checkspeed
	mov	esi,eax
	shl	esi,2
@@:
	ret
;-----------------------------------------------------------------------------
xmax:
	dec	eax
	dec	ebx
	dec	edi
	shl	eax,16
	add	edi,eax
	mov	edx,edi
	jmp	sravn
;-----------------------------------------------------------------------------
xmin:
	mov	edx,edi
	jmp	sravn
;-----------------------------------------------------------------------------
ymax:
	dec	ecx
	dec	ebx

	shl	ebx,16
	mov	edi,ebx
	add	edi,ecx
	mov	edx,edi
	jmp	sravn
;-----------------------------------------------------------------------------
ymin:
	shl	ebx,16
	mov	edx,ebx
	shr	ebx,16
	jmp	sravn
;-----------------------------------------------------------------------------
hotkeys_common:
	xor	ecx,ecx
	xor	edx,edx
	mov	cl,79	; Down Num 1
	mcall	66

	mov	cl,80	; Down Num 2
	mcall	66

	mov	cl,81	; Down Num 3
	mcall	66

	mov	cl,75	; Down Num 4
	mcall	66

	mov	cl,76	; Down Num 5
	mcall	66

	mov	cl,204 ; Up Num 5
	mcall	66

	mov	cl,77	; Down Num 6
	mcall	66

	mov	cl,71	; Down Num 7
	mcall	66

	mov	cl,72	; Down Num 8
	mcall	66

	mov	cl,73	; Down Num 9
	mcall	66
	ret
;-----------------------------------------------------------------------------
; <--- initialised data --->
DATA
;-----------------------------------------------------------------------------
; <--- uninitialised data --->
UDATA
mouse_timer_ticks	dd 0
;-----------------------------------------------------------------------------
if lang eq ru
ud_user_message db 'NumLock вкл/выкл эмулятор мыши. Управление Numpad',0 ;удалить строчку из хот_кейз
else
ud_user_message db 'NumLock - on/off mouse emul. Numpad - move cursor',0
end if

notifyapp:
        dd      7
        dd      0
        dd      ud_user_message
        dd      0
        dd      0
        db      '@notify',0
;-----------------------------------------------------------------------------



MEOS_APP_END
; <--- end of KolibriOS application --->
; ZG