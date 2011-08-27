;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; Includes source code by Kulakov Vladimir Gennadievich.       ;;
;; Modified by Mario79 and Rus.                                 ;;
;; 02.12.2009 <Lrz>						;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;driver sceletone

format MS COFF

DEBUG        equ 0

include 'proc32.inc'
include 'imports.inc'

API_VERSION	equ 5  ;debug

struc IOCTL
{  .handle	dd ?
   .io_code	dd ?
   .input	dd ?
   .inp_size	dd ?
   .output	dd ?
   .out_size	dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

public START
public version


DRV_ENTRY    equ 1
DRV_EXIT     equ -1
STRIDE	     equ 4	;size of row in devices table

SRV_GETVERSION	equ 0

section '.flat' code readable align 16

proc START stdcall, state:dword

	   cmp [state], 1
	   jne .exit
.entry:
	;Detect_COM_Mouse:
if DEBUG
	mov    esi, msgInit
	call   Boot_Log
end if
	mov    bx, 0x3f8
	call   MSMouseSearch
	cmp    AL,'M'
	jne    @f
	;mov    [com1_mouse_detected],1
	;mov     [irq_owner+4*4], 1      ; IRQ4 owner is System

	mov	dx, bx
	inc	dx			 ; 0x3f8 + 1
	mov	al, 1
	out	dx, al

	stdcall AttachIntHandler, 4, irq4_handler, dword 0
if DEBUG
	test	eax, eax
	jne	.label1

	mov	esi, msg_error_attach_int_handler
	call	Boot_Log
end if
     .label1:
;	mov	eax, 0
;	mov	ebx, 0x3F8
;	mov	ecx, 0x3FF
	xor	ebx,ebx
	mov	ecx, 0x3F8
	mov	edx, 0x3FF
	call	ReservePortArea

if DEBUG
	cmp	eax, 1
	jne	.go

	mov	esi, msg_error_reserve_ports
	call	Boot_Log

     .go:
	mov	esi,boot_setmouse_type
	call	Boot_Log
end if
     @@:
	mov	bx, 0x2f8
	call	MSMouseSearch
	cmp	AL,'M'
	jne	.resume
	;mov     [com2_mouse_detected],1
	;mov     [irq_owner+3*4], 1      ; IRQ3 owner is System

	stdcall AttachIntHandler, 3, irq3_handler, dword 0

;	mov	eax, 0
;	mov	ebx, 0x2F8
;	mov	ecx, 0x3F8
	xor	ebx,ebx
	mov	ecx, 0x2F8
	mov	edx, 0x3F8

	call	ReservePortArea
if DEBUG
	cmp	eax, 1
	jne	@f

	mov	esi, msg_error_reserve_ports
	call	Boot_Log
      @@:

	mov	esi,boot_setmouse_type + 22
	call	Boot_Log
end if
      .resume:

	   stdcall RegService, my_service, service_proc
if DEBUG
	   test	eax, eax
	   jne	@f

	   mov	esi, msg_exit
	   call Boot_Log
end if
	 @@:
	   ret
.fail:
.exit:
if DEBUG
	   mov	esi, msg_exit
	   call Boot_Log
end if
	   xor eax, eax
	   ret
endp

handle	   equ	IOCTL.handle
io_code    equ	IOCTL.io_code
input	   equ	IOCTL.input
inp_size   equ	IOCTL.inp_size
output	   equ	IOCTL.output
out_size   equ	IOCTL.out_size

align 4
proc service_proc stdcall, ioctl:dword

	   mov ebx, [ioctl]
	   mov eax, [ebx+io_code]
	   cmp eax, SRV_GETVERSION
	   jne @F

	   mov eax, [ebx+output]
	   cmp [ebx+out_size], 4
	   jne .fail
	   mov [eax], dword API_VERSION
	   xor eax, eax
	   ret
@@:
.fail:
	   or eax, -1
	   ret
endp

align 4
MSMouseSearch:
	; ПОИСК МЫШИ ЧЕРЕЗ COM-ПОРТЫ
MouseSearch:
	; Устанавливаем скорость
	; приема/передачи 1200 бод
	; in bx COM Port Base Address
	mov	DX, bx
	add	DX,3
	in	AL,DX
	or	AL,80h	;установить бит DLAB
	out	DX,AL
	mov	DX, bx
	mov	AL,60h	;1200 бод
	out	DX,AL
	inc	DX
	mov	AL,0
	out	DX,AL
	; Установить длину слова 7 бит, 1 стоповый бит,
	; четность не контролировать
	mov	DX, bx
	add	DX,3
	mov	AL,00000010b
	out	DX,AL
	; Запретить все прерывани
	mov	dx, bx
	inc	dx
	mov	AL,0
	out	DX,AL
; Проверить, что устройство подключено и являетс
; мышью типа MSMouse
	; Отключить питание мыши и прерывани
	mov	DX, bx
	add	EDX,4	;регистр управления модемом
	mov	AL,0	;сбросить DTR, RTS и OUT2
	out	DX,AL
	; Ожидать 5 "тиков" (0,2 с)
	mov	ecx, 0xFFFF
	loop	$
	; Включить питание мыши
	mov	al, 1
	out	dx, al
	mov	ecx, 0xFFFF
	loop	$
	; Очистить регистр данных
	mov	dx, bx
	in	AL,DX
	add	edx, 4
	mov	AL, 1011b  ;установить DTR и RTS и OUT2
	out	DX,AL
	mov	ecx, 0x1FFFF
; Цикл опроса порта
WaitData:
	; Ожидать еще 10 "тиков"
	 dec  ecx
;	 cmp  ecx,0
	 jz	NoMouse
	; Проверить наличие идентификационного байта
	mov	DX, bx
	add	DX,5
	in	AL,DX
	test	AL,1   ;Данные готовы?
	jz	WaitData
	; Ввести данные
	mov	DX, bx
	in	AL,DX
NoMouse:
	ret

align 4
irq3_handler:
	mov	dx, 0x2f8
	mov	esi, com2_mouse
	jmp	irq_handler

align 4
irq4_handler:
	mov	dx, 0x3f8
	mov	esi, com1_mouse

irq_handler:

; in: esi -> COM_MOUSE_DATA struc, dx = base port (xF8h)
	add	edx, 5		; xFDh
	in	al, dx
	test	al, 1		; Данные готовы?
	jz	.Error
; Ввести данные
	sub	edx, 5
	in	al, dx
; Сбросить старший незначащий бит
	and	al, 01111111b

; Определить порядковый номер принимаемого байта
	cmp	[esi+COM_MOUSE_DATA.MouseByteNumber], 2
	ja	.Error
	jz	.ThirdByte
	jp	.SecondByte
; Сохранить первый байт данных
.FirstByte:
	test	al, 1000000b	; Первый байт посылки?
	jz	.Error
	mov	[esi+COM_MOUSE_DATA.FirstByte], al
	inc	[esi+COM_MOUSE_DATA.MouseByteNumber]
	jmp	.EndMouseInterrupt
; Сохранить второй байт данных
.SecondByte:
	test	al, 1000000b
	jnz	.Error
	mov	[esi+COM_MOUSE_DATA.SecondByte], al
	inc	[esi+COM_MOUSE_DATA.MouseByteNumber]
	jmp	.EndMouseInterrupt
; Сохранить третий байт данных
.ThirdByte:
	test	al, 1000000b
	jnz	.Error
	mov	[esi+COM_MOUSE_DATA.ThirdByte], al
	mov	[esi+COM_MOUSE_DATA.MouseByteNumber], 0
; (Пакет данных от мыши принят полностью).
; Записать новое значение состояния кнопок мыши
	mov	al, [esi+COM_MOUSE_DATA.FirstByte]
	mov	ah, al
	shr	al, 3
	and	al, 2
	shr	ah, 5
	and	ah, 1
	add	al, ah
	movzx	eax, al
	mov	[BTN_DOWN], eax

; Прибавить перемещение по X к координате X
	mov	al, [esi+COM_MOUSE_DATA.FirstByte]
	shl	al, 6
	or	al, [esi+COM_MOUSE_DATA.SecondByte]

	cbw
	movzx	eax, ax
	mov	[MOUSE_X], eax

; Прибавить перемещение по Y к координате Y
	mov	al, [esi+COM_MOUSE_DATA.FirstByte]
	and	al, 00001100b
	shl	al, 4
	or	al, [esi+COM_MOUSE_DATA.ThirdByte]

	cbw
	movzx	eax, ax
	neg	eax
	mov	[MOUSE_Y], eax

	stdcall SetMouseData, [BTN_DOWN], [MOUSE_X], [MOUSE_Y], 0, 0

	jmp	.EndMouseInterrupt

.Error:
; Произошел сбой в порядке передачи информации от
; мыши, обнулить счетчик байтов пакета данных

	mov	[esi+COM_MOUSE_DATA.MouseByteNumber],0
.EndMouseInterrupt:
        mov al, 1
	ret

;all initialized data place here

align 4

struc COM_MOUSE_DATA {
; Номер принимаемого от мыши байта
	.MouseByteNumber	db	?
; Трехбайтовая структура данных, передаваемая мышью
	.FirstByte		db	?
	.SecondByte		db	?
	.ThirdByte		db	?
	;.timer_ticks_com        dd      ?
}
virtual at 0
 COM_MOUSE_DATA COM_MOUSE_DATA
end virtual

com1_mouse COM_MOUSE_DATA
com2_mouse COM_MOUSE_DATA

MOUSE_X      dd 0
MOUSE_Y      dd 0
BTN_DOWN     dd 0

COMPortBaseAddr dw 3F8h



version      dd (5 shl 16) or (API_VERSION and 0xFFFF)

my_service   db 'COM_Mouse',0  ;max 16 chars include zero

if DEBUG
msgInit 		     db   'Preved bugoga!',13,10,0
boot_setmouse_type	     db   'Detected - COM1 mouse',13,10,0
			     db   'Detected - COM2 mouse',13,10,0
msg_error_reserve_ports      db   'Error reserving ports!',13,10,0
msg_error_attach_int_handler db   'Error attach interrupt handler!',13,10,0
msg_exit		     db   'Exit!',13,10,0
end if

section '.data' data readable writable align 16

;all uninitialized data place here

