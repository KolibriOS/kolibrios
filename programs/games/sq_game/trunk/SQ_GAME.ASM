;   SQ_game for MenuetOS
;   Author:    Alexei Ershov aka ealex
;   E-mail:    e-al[at]yandex[dot]ru
;   Fidonet:   2:469/335.38
; slightly optimized by diamond
; english translation by diamond (two strings in end of source)


	BtnSize     equ 32	; размер кнопки
	BtnSpace    equ 10	; промежуток между кнопками
	NofBtn	    equ 4   ; размер поля (3-15)
	Color1	    equ 0xcc0000
	Color2	    equ 0x00cc00

	FieldSize   = (BtnSize+BtnSpace)*NofBtn + BtnSpace*3
	center	    = FieldSize / 2


  use32 	     ; включить 32-битный режим ассемблера
  org	 0x0	     ; адресация с нуля

  db	 'MENUET01'  ; 8-байтный идентификатор MenuetOS
  dd	 0x01	     ; версия заголовка (всегда 1)
  dd	 START	     ; адрес первой команды
  dd	 I_END	     ; размер программы
  dd	 0x1000      ; количество памяти
  dd	 0x1000      ; адрес вершины стэка
  dd	 0x0	     ; адрес буфера для параметров (не используется)
  dd	 0x0	     ; зарезервировано

include 'lang.inc'
include 'MACROS.INC' ; макросы облегчают жизнь ассемблерщиков!


START:
    call Game_Init
red:
    call Draw_Window

still:
    mcall 10		; функция 10 - ждать события
	dec	eax
	jz	red
	dec	eax
	jnz	button

 key:		       ; нажата клавиша на клавиатуре
	mov	al, 2
	int	0x40
	jmp	still
;---------------------------------------------------------------------

button:
	mov	al, 17	; 17 - получить идентификатор нажатой кнопки
	int	0x40
    cmp   ah, 1 	; если нажата кнопка с номером 1, выходим
    je	  .exit

    cmp   ah, 2 	; если нажата кнопка 2, начинаем новую игру
    je	  START

      ; Нажа кнопка на поле
	inc	dword [moves]	  ; увеличиваем число ходов

	shr	eax, 8		  ; в eax идентификатор нажатой кнопки

	mov	dh, NofBtn
	div	dh	      ; al = ax(номер кнопки) / NofBtn
			      ; ah = остаток  (см. рисунок)
	mov	ebx, eax      ; сохраним частное и остаток в ebx

      ; al\ah 0   1    2    3
      ;     +-+  +-+  +-+  +-+
      ;  1  |4|  |5|  |6|  |7|
      ;     +-+  +-+  +-+  +-+

      ;     +-+  +-+  +-+  +-+
      ;  2  |8|  |9|  |10  |11
      ;     +-+  +-+  +-+  +-+

      ;     +-+  +-+  +-+  +-+
      ;  3  |12  |13  |14  |15
      ;     +-+  +-+  +-+  +-+

      ;     +-+  +-+  +-+  +-+
      ;  4  |16  |17  |18  |19
      ;     +-+  +-+  +-+  +-+

; Меняем цвет столбика
	shr	eax, 8	      ; eax = ah
	mov	edi, field
	add	edi, eax    ; edi указвыет на первую кнопку и искомом стлбце
	mov	ecx, NofBtn
@@:
	not	byte[edi]    ; меняем цвет во всем столбике
	dec	ecx
	add	edi, NofBtn
	test	ecx, ecx
	jnz	@r

; Меняем цвет строки
	mov	eax, ebx      ; восстанавливаем eax
	dec	eax
	mov	dl, NofBtn
	mul	dl	      ; ax = al * NofBtn
	mov	edi, field
	add	edi, eax     ; edi указвыет на первую кнопку в искомой строке
	mov	ecx, NofBtn
@@:
	not	byte [edi]    ; меняем цвет во всей строке
	dec	ecx
	inc	edi
	test	ecx, ecx
	jnz	@r
; Цвет нажатой кнопки менялся 2 раза (когда обрабатывали строку и столбец),
; т.е. остался первоначалным
; Меняем цвет нажатой кнопки
	shr	ebx, 8	      ; ebx = остаток (ah на рисунке)
	not	byte [field+eax+ebx]
	jmp	red



  .exit:
    mcall -1		; иначе конец программы


;----------------------------------------------------------------------------
;               Draw_Window
;----------------------------------------------------------------------------
Draw_Window:

    mcall 12, 1
    mcall 0, 200*65536+FieldSize,\
	     200*65536+FieldSize+20+20+20,\
	     0x03AABBCC, 0x805080D0, 0x005080D0
    mcall 4, 8*65536+8, 0x10DDEEFF, header, header_e - header

    mcall 8, BtnSpace*2*65536 + 83,\
	     (FieldSize+BtnSpace+20)*65536+ 15, 2
    mcall 4, (BtnSpace*2+3)*65536+(BtnSize+BtnSpace)*NofBtn+BtnSpace*4+20+4,\
	     0x10DDEEFF, strNew, strNew_e - strNew
    mcall 4, (center - 25) * 65536 + 30, 0, strMovs, strMovs_e - strMovs
    mcall 47, 4*65536+1, moves, (center + 2 ) * 65536 + 30, 0



	mov	ecx, (20+20+BtnSpace-BtnSize)*65536+BtnSize
	mov	edx, NofBtn	      ; идентификатор первой кнопки на поле
				      ; см рисунок

buttons:			      ; рисуем кнопочки
	mov	eax, edx
	mov	dh, NofBtn
	div	dh	     ; al = ax / dh, ah - остаток
	mov	dh, 0
	test	ah, ah	     ; если ah = 0, начинаем новый ряд
	jnz	@f
	add	ecx, (BtnSize+BtnSpace)*65536
	mov	ebx, BtnSpace*2*65536+BtnSize
@@:
	mov	esi, Color1		       ; байт по адресу
	cmp	byte [field-NofBtn+edx], 0	; field-NofBtn+номер_кнопки
	jz	@f				; говорит о ее цвете
	mov	esi, Color2
@@:
	mcall 8,
	add	ebx, (BtnSize+BtnSpace)*65536
	inc	edx			   ; переходим к следующей кнопке
	cmp	edx, NofBtn*(NofBtn+1)
	jb	buttons

    mcall 12, 2 		   ; функция 12: сообщить ОС об отрисовке окна

ret

;----------------------------------------------------------------------------
;               Game_Init
;----------------------------------------------------------------------------
Game_Init:
	and	[moves], 0	       ; обнулим количество ходов

	call	random		       ; в eax случайное число
	mov	ecx, NofBtn * NofBtn   ; в ecx кол-во кнопок на поле
	mov	bh, 0		       ; для каждой кнопки будем брать
				       ; 1 бит из eax, bh - сколько
button_init:			       ; бит уже использовали
	shr	eax, 1		 ; младший бит попадает в флаг CF
				 ; в зависимости от его значения
				 ; в bl записывается -1 или 0
	sbb	bl, bl
	mov	[field + ecx - 1], bl

	inc	bh
	cmp	bh, 32		 ; если использованы все 32 бита из eax
	jb	@f		 ; нао получить новое случайное число
	call	random
	mov	bh, 0
@@:
	loop	button_init

ret

;----------------------------------------------------------------------------
;     random - взята из тетриса
;----------------------------------------------------------------------------
random: 	mov eax,[generator]
		add eax,-43ab45b5h
		ror eax,1
		xor eax,32c4324fh
		ror eax,1
		mov [generator],eax
		; --- IVAN ---
		mov eax,26
		mov ebx,9
		int 0x40
		xor eax,0xdeadbeef
		add eax,[generator]
		; --- IVAN ---
		ret

;============================================================================

field		db NofBtn * NofBtn dup(0)
moves		dd 0
generator	dd 0

if lang eq ru
header		db 'Game'
header_e:
strMovs 	db 'ХОД'	; 'ХОД'
strMovs_e:
strNew		db 'Новая игра' ; 'Новая игра'
strNew_e:
else
header		db 'Game'
header_e:
strMovs 	db 'MOVE'
strMovs_e:
strNew		db ' New game'
strNew_e:
end if



I_END:				   ; метка конца программы
