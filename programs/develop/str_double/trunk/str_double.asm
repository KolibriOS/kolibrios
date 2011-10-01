; Программа для преобразования чисел из строки
;   в формат float, double, а также из 10 или 16 ричного
;   во float.
; Сделана на основе hex2dec2bin и примеров из файла list2_05.inc
;   (автор Кулаков Владимир Геннадьевич 24.05.2002),
;   которые ChE переделал с 16 на 32 бита на ассемблер fasm.
; Программа позволяет решать такие задачи:
; 1) число в строковом виде перевести в 4 байта (float) в машинный вид
; 2) число в строковом виде перевести в 8 байт (double) в машинный вид
; 3) число в машинном виде (float) перевести в строковый вид (5 знаков после запятой)

use32
    org 0x0
    db	'MENUET01'
    dd	0x01,start,i_end,e_end,e_end,0,sys_path

include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'lang.inc'

@use_library

align 4
start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась наша библиотека
	mov	ebp,lib_0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mcall 40,0x27
	mcall 48, 3, sys_colors, 40
	edit_boxes_set_sys_color edit1,editboxes_end,sys_colors
	option_boxes_set_sys_color sys_colors,Option_boxes1

align 4
red:
    call draw_window

align 4
still:
    mcall 10		; функция 10 - ждать события

    cmp  eax,1		; перерисовать окно ?
    je	 red		; если да - на метку red
    cmp  eax,2		; нажата клавиша ?
    je	 key		; если да - на key
    cmp  eax,3		; нажата кнопка ?
    je	 button 	; если да - на button
    cmp  eax,6
    je	 mouse

    jmp  still		; если другое событие - в начало цикла

;---------------------------------------------------------------------
key: ; нажата клавиша на клавиатуре
	mcall 2
	;cmp ah,13
	stdcall [edit_box_key], dword edit1
	jmp  still ; вернуться к началу цикла

; Количество знаков числа после запятой (1-17)
NumberSymbolsAD DW 5
; Константы (10 в степени N)
MConst DQ 1.0E1,1.0E2,1.0E3,1.0E4,1.0E5
       DQ 1.0E6,1.0E7,1.0E8,1.0E9,1.0E10
       DQ 1.0E11,1.0E12,1.0E13,1.0E14,1.0E15
       DQ 1.0E16,1.0E17,1.0E18,1.0E19,1.0E20
       DQ 1.0E21,1.0E22,1.0E23,1.0E24,1.0E25
       DQ 1.0E26,1.0E27,1.0E28,1.0E29,1.0E30
       DQ 1.0E31,1.0E32,1.0E33,1.0E34,1.0E35
       DQ 1.0E36,1.0E37,1.0E38,1.0E39,1.0E40
       DQ 1.0E41,1.0E42,1.0E43,1.0E44,1.0E45
       DQ 1.0E46,1.0E47,1.0E48,1.0E49,1.0E50
       DQ 1.0E51,1.0E52,1.0E53,1.0E54,1.0E55
       DQ 1.0E56,1.0E57,1.0E58,1.0E59,1.0E60
       DQ 1.0E61,1.0E62,1.0E63,1.0E64,1.0E65
       DQ 1.0E66,1.0E67,1.0E68,1.0E69,1.0E70
       DQ 1.0E71,1.0E72,1.0E73,1.0E74,1.0E75
       DQ 1.0E76,1.0E77,1.0E78,1.0E79,1.0E80
       DQ 1.0E81,1.0E82,1.0E83,1.0E84,1.0E85
       DQ 1.0E86,1.0E87,1.0E88,1.0E89,1.0E90
       DQ 1.0E91,1.0E92,1.0E93,1.0E94,1.0E95
       DQ 1.0E96,1.0E97,1.0E98,1.0E99,1.0E100
       DQ 1.0E101,1.0E102,1.0E103,1.0E104,1.0E105
       DQ 1.0E106,1.0E107,1.0E108,1.0E109,1.0E110
       DQ 1.0E111,1.0E112,1.0E113,1.0E114,1.0E115
       DQ 1.0E116,1.0E117,1.0E118,1.0E119,1.0E120
       DQ 1.0E121,1.0E122,1.0E123,1.0E124,1.0E125
       DQ 1.0E126,1.0E127,1.0E128
; Число с плавающей запятой двойной точности
Data_Double   DQ ?
; Число в BCD-формате 
Data_BCD      DT ?
; Вспомогательный флаг
Data_Flag     DB ?
; Знак результата (если не 0 - отрицательное число)
Data_Sign     DB ?
; Строка для хранения числа в коде ASCII
Data_String   DB 32 DUP (?)

string1 db 32 dup (0)
string1_end:



;*******************************************************
;*  ПРЕОБРАЗОВАНИЕ ЧИСЛА С ПЛАВАЮЩЕЙ ЗАПЯТОЙ В СТРОКУ  *
;* Число имеет формат с удвоенной точностью, результат *
;* выдается в десятичном коде, в "бытовом" формате с   *
;* фиксированным количеством знаков после запятой.     *
;* Входные параметры:                                  *
;* Data_Double - преобразуемое число;                  *
;* NumberSymbolsAD - количество знаков после           *
;*                   запятой (0-17).                   *
;* Выходные параметры:                                 *
;* Data_String - строка-результат.                     *
;*******************************************************
DoubleFloat_to_String:
	pushad
	; Результат записывать в строку Data_String
	mov	EDI, Data_String

	; Сдвигаем число влево на NumberSymbolsAD
	; десятичных разрядов
	fninit		       ;сброс сопроцессора
	fld	[Data_Double]  ;загрузить число
	xor ebx,ebx
	mov	BX,[NumberSymbolsAD]
	cmp	BX, 0
	je	.NoShifts     ;нет цифр после запятой
	jl	.Error	      ;ошибка
	dec	BX
	shl	BX, 3		;умножаем на 8
	add	EBX, MConst
	fmul	qword [EBX] ;умножить на константу
.NoShifts:
	; Извлечь число в коде BCD
	fbstp	[Data_BCD]
; Проверить результат на переполнение
	mov	AX,word [Data_BCD + 8]
	cmp	AX,0FFFFh  ;"десятичное" переполнение?
	je	.Overflow
; Выделить знак числа и записать его в ASCII-коде
	mov	AL, byte [Data_BCD + 9]
	and	AL,AL
	jz	.NoSign
	mov	AL,'-'
	stosb
.NoSign:
; Распаковать число в код ASCII
	mov	ebx,8	 ;смещение последней пары цифр
	mov	ecx,9	 ;счетчик пар цифр
	; Определить позицию десятичной точки в числе
	mov	DX,18
	sub	DX,[NumberSymbolsAD]
	js	.Error	;ошибка, если отрицательная
	jz	.Error	;или нулевая позиция
.NextPair:
	; Загрузить очередную пару разрядов
	mov	AL, byte [ebx + Data_BCD]
	mov	AH,AL
	; Выделить, перевести в ASCII и
	; сохранить старшую тетраду
	shr	AL,4
	add	AL,'0'
	stosb
	dec	DX
	jnz	.N0
	mov	AL,'.'
	stosb
.N0:   ; Выделить, перевести в ASCII и
	; сохранить младшую тетраду
	mov	AL,AH
	and	AL,0Fh
	add	AL,'0'
	stosb
	dec	DX
	jnz	.N1
	mov	AL,'.'
	stosb
.N1:
	dec  BX
	loop .NextPair
	mov  AL,0
	stosb

; Убрать незначащие нули слева
	mov	EDI, Data_String
	mov	ESI, Data_String
	; Пропустить знак числа, если он есть
	cmp	byte [ESI],'-'
	jne	.N2
	inc	ESI
	inc	EDI
.N2:   ; Загрузить в счетчик цикла количество разрядов
	; числа плюс 1 (байт десятичной точки)
	mov	ecx,18+1+1
	; Пропустить незначащие нули
.N3:
	cmp byte [ESI],'0'
	jne .N4
	cmp byte [ESI+1],'.'
	je .N4
	inc ESI
	loop .N3
	; Ошибка - нет значащих цифр
	jmp	.Error
; Скопировать значащую часть числа в начало строки
.N4:	rep movsb
	jmp    .End

; Ошибка
.Error:
	mov	AL,'E'
	stosb
	mov	AL,'R'
	stosb
	mov	AL,'R'
	stosb
	xor	AL,AL
	stosb
	jmp	.End
; Переполнение разрядной сетки
.Overflow:
	mov	AL,'#'
	stosb
	xor	AL,AL
	stosb
; Конец процедуры
.End:
	popad
	ret

;****************************************************
;* ПРЕОБРАЗОВАТЬ СТРОКУ В ЧИСЛО С ПЛАВАЮЩЕЙ ЗАПЯТОЙ *
;*      (число имеет обычный, "бытовой" формат)     *
;* Входные параметры:                               *
;* Data_String - число в коде ASCII.                *
;* Выходные параметры:                              *
;* Data_Double - число в двоичном коде.             *
;****************************************************
String_to_DoubleFloat:
	pushad
	cld
	; Очищаем Data_BCD 
	mov dword [Data_BCD],0
	mov dword [Data_BCD+4],0
	mov  word [Data_BCD+8],0
	; Очищаем байт знака
	mov	[Data_Sign],0
	; Заносим в SI указатель на строку
	mov	ESI, Data_String
	; Пропускаем пробелы перед числом
	mov	ecx,64 ;защита от зацикливания
.ShiftIgnore:
	lodsb
	cmp	AL,' '
	jne	.ShiftIgnoreEnd
	loop	.ShiftIgnore
	jmp	.Error
.ShiftIgnoreEnd:
	; Проверяем знак числа
	cmp	AL,'-'
	jne	.Positive
	mov	[Data_Sign],80h
	lodsb
.Positive:
	mov	[Data_Flag],0 ;признак наличия точки
	mov	DX,0	      ;позиция точки
	mov	ecx,18	      ;макс. число разрядов
.ASCIItoBCDConversion:
	cmp	AL,'.'	      ;точка?
	jne	.NotDot
	cmp	[Data_Flag],0 ;точка не встречалась?
	jne	.Error
	mov	[Data_Flag],1
	lodsb
	cmp	AL,0	      ;конец строки?
	jne	.NotDot
	jmp	.ASCIItoBCDConversionEnd
.NotDot:
	; Увеличить на 1 значение позиции точки,
	; если она еще не встречалась
	cmp	[Data_Flag],0
	jnz	.Figures
	inc	DX
.Figures:
	; Символы числа должны быть цифрами
	cmp	AL,'0'
	jb	.Error
	cmp	AL,'9'
	ja	.Error
	; Пишем очередную цифру в младшую тетраду BCD
	and	AL,0Fh
	or	byte [Data_BCD],AL
	; Проверка на конец строки
	cmp	byte [ESI],0
	je	.ASCIItoBCDConversionEnd
	; Сдвигаем BCD на 4 разряда влево
	; (сдвигаем старшие 2 байта)
	mov	AX,word [Data_BCD+6]
	shld	word [Data_BCD+8],AX,4
	; (сдвигаем средние 4 байта)
	mov	EAX, dword [Data_BCD]
	shld	dword [Data_BCD+4],EAX,4
	; (сдвигаем младшие 4 байта)
	shl	dword [Data_BCD],4
	; Загружаем следующий символ в AL
	lodsb
	loop	.ASCIItoBCDConversion
	; Если 19-й символ не 0 и не точка,
	; то ошибка переполнения
	cmp	AL,'.'
	jne	.NotDot2
	inc	ecx
	lodsb
.NotDot2:
	cmp	AL,0
	jne	.Error ;переполнение разрядной сетки

; ПРЕОБРАЗОВАТЬ ЧИСЛО ИЗ КОДА BCD В ВЕЩЕСТВЕННОЕ ЧИСЛО
.ASCIItoBCDConversionEnd:
	; Вписать знак в старший байт
	mov	AL,[Data_Sign]
	mov	byte [Data_BCD+9],AL
	; Сбросить регистры сопроцессора
	fninit
	; Загрузить в сопроцессор число в BCD-формате
	fbld	[Data_BCD]
	; Вычислить номер делителя
	mov	EBX,18+1
	sub	BX,CX
	sub	BX,DX
	cmp	EBX,0
	je	.NoDiv
	dec	EBX
	shl	EBX,3		;умножаем на 8
	add	EBX, MConst
	fdiv	qword [EBX] ;разделить на константу
.NoDiv:; Выгрузить число в двоичном формате
	fstp	[Data_Double]
	jmp	.End

.Error:; При любой ошибке обнулить результат
	fldz	;занести ноль с стек сопроцессора
	fstp	[Data_Double]
.End:
	popad
	ret



;input:
; buf - указатель на строку, число должно быть в 10 или 16 ричном виде
;output:
; eax - число
align 4
proc conv_str_to_int, buf:dword
	xor eax,eax
	push ebx ecx esi
	xor ebx,ebx
	mov esi,[buf]
	;определение отрицательных чисел
	xor ecx,ecx
	inc ecx
	cmp byte[esi],'-'
	jne @f
		dec ecx
		inc esi
	@@:

	cmp word[esi],'0x'
	je .load_digit_16

	.load_digit_10: ;считывание 10-тичных цифр
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'9'
		jg @f
			sub bl,'0'
			imul eax,10
			add eax,ebx
			inc esi
			jmp .load_digit_10
	jmp @f

	.load_digit_16: ;считывание 16-ричных цифр
		add esi,2
	.cycle_16:
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'f'
		jg @f
		cmp bl,'9'
		jle .us1
			cmp bl,'A'
			jl @f ;отсеиваем символы >'9' и <'A'
		.us1: ;составное условие
		cmp bl,'F'
		jle .us2
			cmp bl,'a'
			jl @f ;отсеиваем символы >'F' и <'a'
			sub bl,32 ;переводим символы в верхний регистр, для упрощения их последущей обработки
		.us2: ;составное условие
			sub bl,'0'
			cmp bl,9
			jle .cor1
				sub bl,7 ;convert 'A' to '10'
			.cor1:
			shl eax,4
			add eax,ebx
			inc esi
			jmp .cycle_16
	@@:
	cmp ecx,0 ;если число отрицательное
	jne @f
		sub ecx,eax
		mov eax,ecx
	@@:
	pop esi ecx ebx
	ret
endp



;---------------------------------------------------------------------
align 4
button:
	mcall 17		; 17 - получить идентификатор нажатой кнопки
	cmp   ah, 1	; если НЕ нажата кнопка с номером 1,
	jne   @f
		mcall -1
	@@:
	cmp ah, 5
	jne @f
		cmp dword[option_group1],opt3
		jne .opt_3_end
			stdcall conv_str_to_int,dword[edit1.text]
			mov dword[Data_Double],eax
			finit
			fld dword[Data_Double]
			fstp qword[Data_Double]

			; Data_Double - преобразуемое число
			; NumberSymbolsAD - количество знаков после запятой (0-17)
			call DoubleFloat_to_String
			mov dword[Data_Double],eax ;восстанавливаем значение в формате float
			jmp .opt_all_end
		.opt_3_end:

		mov esi,string1
		mov edi,Data_String
		cld
		mov ecx,32
		rep movsb

		call String_to_DoubleFloat
		cmp dword[option_group1],opt1
		jne .opt_all_end ;если выбран float, то преобразуем из ранее полученного double
			finit
			fld  qword[Data_Double] ;читаем из double
			fstp dword[Data_Double] ;а возвращаем во float
		.opt_all_end:
		jmp red
	@@:
	jmp still

mouse:
	stdcall [edit_box_mouse], edit1
	stdcall [option_box_mouse], Option_boxes1
	jmp still

;------------------------------------------------
align 4
draw_window:
	mcall 48, 3, sys_colors, sizeof.system_colors

	mcall 12, 1
	mov edx, 0x14000000
	or  edx, [sys_colors.work]
	mcall 0, 200*65536+300, 200*65536+175, ,,title

	mcall 8, (300-53)*65536+38,145*65536+ 15, 5,[sys_colors.work_button] ; кнопка Ok

	mov ecx, 0x80000000
	or  ecx, [sys_colors.work_text]
	mcall 4, 15*65536 +30,, binstr,
	mcall  , 15*65536 +58,, decstr,
		mcall  , (240-56*3)*65536 +58,, Data_String,
	mcall  , 15*65536 +72,, hexstr,
	mcall  , 15*65536+150,, numstr,

	mov ecx, 0x80000000
	or  ecx, [sys_colors.work_button_text]
	mcall  , (300-42)*65536+149,	, Okstr,3

	cmp dword[option_group1],opt1
	je @f ;если выбран float, то старшие 4 байта (из double) не печатаем
	cmp dword[option_group1],opt3
	je @f ;если выбран float, то старшие 4 байта (из double) не печатаем
		mov ecx, dword[Data_Double+4]
		mcall  47, 8*65536+256,,185*65536+72,[sys_colors.work_text]    ; 16-ная

		mov ecx, dword[Data_Double+4]
		mcall	 ,8*65536+512,,240*65536+30,	  ; 2-ная
		ror ecx, 8
		mcall	 ,,,(240-56)*65536+30,
		ror ecx, 8
		mcall	 ,,,(240-56*2)*65536+30,
		ror ecx, 8
		mcall	 ,,,(240-56*3)*65536+30,
		ror ecx, 8
	@@:

	mov ecx,dword[Data_Double]
	mcall  47, 8*65536+256,,240*65536+72,[sys_colors.work_text]	 ; 16-ная

	mov ecx,dword[Data_Double]
	mcall	 , 8*65536+512,,240*65536+44,	  ; 2-ная
	ror ecx, 8
	mcall	 ,,,(240-56)*65536+44,
	ror ecx, 8
	mcall	 ,,,(240-56*2)*65536+44,
	ror ecx, 8
	mcall	 ,,,(240-56*3)*65536+44,
	ror ecx, 8

	mcall 38, 15*65536+300-15, 137*65536+137, [sys_colors.work_graph]
	stdcall [edit_box_draw], edit1
	stdcall [option_box_draw], Option_boxes1
	mcall 12, 2		   ; функция 12: сообщить ОС об отрисовке окна

ret


;-------------------------------------------------
title db 'string to double 07.09.11',0
hexstr db 'hex:',0
decstr db 'dec:',0
binstr db 'bin:',0

if lang eq ru
	numstr db 'Число:',0
	Okstr db 'Ввод',0
	head_f_i:
	head_f_l db 'Системная ошибка',0
else
	numstr db 'Number:',0
	Okstr db 'Ok',0
	head_f_i:
	head_f_l db 'System error',0
end if

mouse_dd dd 0
edit1 edit_box 182, 59, 146, 0xffffff, 0xff, 0x80ff, 0, 0x8000, (string1_end-string1), string1, mouse_dd, 0
editboxes_end:

;option_boxes
opt1 option_box option_group1, 15,  90, 8, 12, 0xffffff, 0x80ff, 0, op_text.1, 17
opt2 option_box option_group1, 15, 106, 8, 12, 0xffffff, 0x80ff, 0, op_text.2, 18
opt3 option_box option_group1, 15, 122, 8, 12, 0xffffff, 0x80ff, 0, op_text.3, 21

op_text: ;текст для радио кнопок
  .1 db 'str(dec) -> float'
  .2 db 'str(dec) -> double'
  .3 db 'float(dec,hex) -> str'
;указатели для option_box
option_group1 dd opt1
Option_boxes1 dd opt1, opt2, opt3, 0

system_dir_0 db '/sys/lib/'
lib_name_0 db 'box_lib.obj',0
err_msg_found_lib_0 db 'Не найдена библиотека ',39,'box_lib.obj',39,0
err_msg_import_0 db 'Ошибка при импорте библиотеки ',39,'box_lib',39,0

l_libs_start:
	lib_0 l_libs lib_name_0, sys_path, library_path, system_dir_0,\
		err_msg_found_lib_0,head_f_l,import_box_lib,err_msg_import_0,head_f_i
l_libs_end:

align 4
import_box_lib:
	;dd sz_init1
	edit_box_draw dd sz_edit_box_draw
	edit_box_key dd sz_edit_box_key
	edit_box_mouse dd sz_edit_box_mouse
	;edit_box_set_text dd sz_edit_box_set_text
	option_box_draw dd aOption_box_draw
	option_box_mouse dd aOption_box_mouse
	;version_op dd aVersion_op
dd 0,0
	;sz_init1 db 'lib_init',0
	sz_edit_box_draw db 'edit_box',0
	sz_edit_box_key db 'edit_box_key',0
	sz_edit_box_mouse db 'edit_box_mouse',0
	;sz_edit_box_set_text db 'edit_box_set_text',0
	aOption_box_draw db 'option_box_draw',0
	aOption_box_mouse db 'option_box_mouse',0
	;aVersion_op db 'version_op',0

i_end:
	sys_colors system_colors
	rb 0x400 ;stack
	sys_path rb 4096
	library_path rb 4096
e_end: ; метка конца программы
