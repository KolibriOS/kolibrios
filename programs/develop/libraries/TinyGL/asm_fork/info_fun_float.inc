;
; функции для работы с числами float
;

; Количество знаков числа после запятой (1-17)
NumberSymbolsAD DW 5
; Константы (10 в степени N)
MConst: DQ 1.0E1,1.0E2,1.0E3,1.0E4,1.0E5
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
.end:
; Число с плавающей запятой двойной точности
Data_Double   DQ ?
; Число в BCD-формате 
Data_BCD      DT ?
; Вспомогательный флаг
Data_Flag     DB ?
; Знак результата (если не 0 - отрицательное число)
Data_Sign     DB ?
; Знак результата - 0 для ..e+.. и 1 для ..e-..
Data_Sign_Exp DB ?

align 4
; Строка для хранения числа в коде ASCII
Data_String   DB 32 DUP (?)



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
align 4
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
	lea ebx,[MConst+8*ebx]
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
align 4
.N4:	rep movsb
	jmp    .End
; Ошибка
align 4
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
align 4
.Overflow:
	mov	AL,'#'
	stosb
	xor	AL,AL
	stosb
; Конец процедуры
align 4
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
align 4
String_to_DoubleFloat:
	pushad
	cld
	; Очищаем Data_BCD 
	mov dword [Data_BCD],0
	mov dword [Data_BCD+4],0
	mov  word [Data_BCD+8],0
	; Очищаем байт знака
	mov	[Data_Sign],0
	; Заносим в esi указатель на строку
	mov	esi, Data_String
	; Пропускаем пробелы перед числом
	mov	ecx,64 ;защита от зацикливания
.ShiftIgnore:
	lodsb
	cmp	al,' '
	jne .ShiftIgnoreEnd
	loop .ShiftIgnore
	jmp .Error
align 4
.ShiftIgnoreEnd:
	; Проверяем знак числа
	cmp	al,'-'
	jne	.Positive
	mov	[Data_Sign],80h
	lodsb
.Positive:
	mov	[Data_Flag],0 ;признак наличия точки
	xor	edx,edx	      ;позиция точки
	mov	ecx,18	      ;макс. число разрядов
align 4
.ASCIItoBCDConversion:
	cmp	al,'.'	      ;точка?
	jne	.NotDot
	cmp	[Data_Flag],0 ;точка не встречалась?
	jne	.Error        ;если точка уже была
	mov	[Data_Flag],1
	lodsb
	or al,al	      ;конец строки?
	jnz	.NotDot
	jmp	.ASCIItoBCDConversionEnd
align 4
.NotDot:
	; Увеличить на 1 значение позиции точки,
	; если она еще не встречалась
	cmp	[Data_Flag],0
	jnz	.Figures
	inc	edx
.Figures:
	cmp al,'e'
	je .exp_form
	cmp al,'E'
	jne @f
	.exp_form:
		call string_ExpForm ;если число в формате ..e..
		or al,al
		jnz .Error
		jmp	.ASCIItoBCDConversionEnd
	@@:
	; Символы числа должны быть цифрами
	cmp	al,'0'
	jb	.Error
	cmp	al,'9'
	ja	.Error
	; Пишем очередную цифру в младшую тетраду BCD
	and	al,15 ;символы 0-9 переводим в число
	or	byte [Data_BCD],al
	; Проверка на конец строки
	cmp	byte [esi],0
	je	.ASCIItoBCDConversionEnd
	; Сдвигаем BCD на 4 разряда влево
	; (сдвигаем старшие 2 байта)
	mov	ax,word [Data_BCD+6]
	shld	word [Data_BCD+8],ax,4
	; (сдвигаем средние 4 байта)
	mov	eax,dword [Data_BCD]
	shld	dword [Data_BCD+4],eax,4
	; (сдвигаем младшие 4 байта)
	shl	dword [Data_BCD],4
	; Загружаем следующий символ в AL
	lodsb
	loop .ASCIItoBCDConversion ;если не компил. то поставить dec ecx, jnz ...
	; Если 19-й символ не 0 и не точка,
	; то ошибка переполнения
	cmp	al,'.'
	jne	.NotDot2
	inc	ecx ;пропуск точки в конце очень большого числа
	lodsb
.NotDot2:
	or al,al	;переполнение разрядной сетки?
	jz	.ASCIItoBCDConversionEnd
align 4
.Error: ; При любой ошибке обнулить результат
	fldz	;занести ноль с стек сопроцессора
	fstp	[Data_Double]
	jmp	.End

; ПРЕОБРАЗОВАТЬ ЧИСЛО ИЗ КОДА BCD В ВЕЩЕСТВЕННОЕ ЧИСЛО
.ASCIItoBCDConversionEnd:
	; Вписать знак в старший байт
	mov	al,[Data_Sign]
	mov	byte [Data_BCD+9],al
	; Сбросить регистры сопроцессора
	fninit
	; Загрузить в сопроцессор число в BCD-формате
	fbld	[Data_BCD]
	; Вычислить номер делителя или множителя
	lea ebx,[ecx+edx-18]
	cmp ebx,0
	jle .NoMul ;если число e-..
	dec ebx
	jz .NoDiv ;если число e+0
	dec ebx
	lea ebx,[MConst+8*ebx]
	cmp ebx,MConst.end
	jl @f
		ffree st0
		fincstp
		jmp .Error ;если очень большое число e+**
	@@:
	fmul qword [ebx] ;умножить на константу (для чисел с приставкой e+..)
	jmp .NoDiv
.NoMul:
	neg ebx
	lea ebx,[MConst+8*ebx]
	cmp ebx,MConst.end
	jl @f
		ffree st0
		fincstp
		jmp .Error ;если очень маленькое число e-**
	@@:
	fdiv qword [ebx] ;разделить на константу
.NoDiv: ;Выгрузить число в двоичном формате
	fstp [Data_Double]
.End:
	popad
	ret

;output:
; eax - 1 if error
; edx += size
align 4
proc string_ExpForm uses ebx
	mov [Data_Sign_Exp],0
	xor eax,eax
	lodsb
	cmp al,'+'
	jne @f
		lodsb
	@@:
	cmp al,'-'
	jne @f
		inc [Data_Sign_Exp]
		lodsb
	@@:
	
	xor ebx,ebx
	.cycle0:
		cmp al,0
		je .cycle0end
		cmp al,9
		je .cycle0end
		cmp al,10
		je .cycle0end
		cmp al,13
		je .cycle0end
		cmp al,' '
		je .cycle0end
		cmp	al,'0'
		jb .Error
		cmp	al,'9'
		ja .Error
	
		imul ebx,10
		and	eax,15 ;символы 0-9 переводим в число
		add ebx,eax
		lodsb
		jmp .cycle0
	.cycle0end:
	
	cmp ebx,328 ;308 - макс. размер степени для double + 20 - число разрядов в BCD
	ja .Error
	cmp [Data_Sign_Exp],0
	je @f
		neg ebx
	@@:
	cmp	[Data_Flag],0 ;точка не встречалась?
	jne @f
		dec edx
	@@:
	add edx,ebx

	xor eax,eax
	jmp @f
	.Error:
		xor eax,eax
		inc eax
	@@:
	ret
endp

align 4
proc str_cat uses eax ecx edi esi, str1:dword, str2:dword
	mov esi,dword[str2]
	stdcall str_len,esi
	mov ecx,eax
	inc ecx
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	ret
endp

;output:
; eax = strlen
align 4
proc str_len, str1:dword
	mov eax,[str1]
	@@:
		cmp byte[eax],0
		je @f
		inc eax
		jmp @b
	@@:
	sub eax,[str1]
	ret
endp

align 4
proc String_crop_0 uses eax ebx ecx edi
	mov edi,Data_String
	mov al,'.'
	mov ecx,32
	repne scasb
	mov ebx,edi
	mov edi,Data_String
	xor al,al
	mov ecx,32
	repne scasb
	cmp ebx,edi
	jg .end_f
	dec edi
	.cycle0:
		dec edi
		cmp edi,Data_String
		jle .end_f
		cmp byte[edi],'0'
		jne .cycle0end
		mov byte[edi],0
		jmp .cycle0
	.cycle0end:
	cmp byte[edi],'.'
	jne .end_f
		mov byte[edi],0
	.end_f:
	ret
endp