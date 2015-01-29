;
; Файл для работы со строковыми функциями
;

; str_len (str1)
;   Длина строки (возвращается в eax)

; str_cat (str1, str2)
;   Добавление к строке st1 строки str2

; str_n_cat (str1, str2, n_len)
;   Добавление к строке st1 строки str2. Если строка str2 длиннее чем n_len, то добавляются первые n_len символов.

; conv_str_to_int (buf)
;   Преобразование строки buf в число (возвращается в eax)

; convert_int_to_str (len)
;   Преобразование числа (eax) в строку (edi). len - длинна строки edi, которую можно использовать для преобразования.


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
proc str_cat uses eax ecx edi esi, str1:dword, str2:dword
	mov esi,[str2]
	stdcall str_len,esi
	mov ecx,eax
	inc ecx
	mov edi,[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	ret
endp

align 4
proc str_n_cat uses eax ecx edi esi, str1:dword, str2:dword, n_len:dword
	mov esi,[str2]
	mov ecx,[n_len]
	mov edi,[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	mov byte[edi],0
	ret
endp

;input:
; buf - указатель на строку, число должно быть в 10 или 16 ричном виде
;output:
; eax - число
align 4
proc conv_str_to_int uses ebx ecx esi, buf:dword
	xor eax,eax
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
	or ecx,ecx ;если число отрицательное
	jnz @f
		neg eax
		inc eax
	@@:
	ret
endp

;input:
; eax - число
; edi - буфер для строки
; len - длинна буфера
;output:
align 4
proc convert_int_to_str, len:dword
pushad
	mov esi,[len]
	add esi,edi
	dec esi
	bt eax,31
	jae @f
		;если число отрицательное
		neg eax
		;inc eax ;???
		mov byte[edi],'-'
		inc edi
	@@:
	call .str
popad
	ret
endp

align 4
.str:
	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
	;преревод числа в ASCII строку взодные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
	cmp eax,ecx ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx ;очистить edx
		div ecx   ;разделить - остаток в edx
		push edx  ;положить в стек
		;dec edi  ;смещение необходимое для записи с конца строки
		call .str ;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
	cmp edi,esi
	jge @f
		or al,0x30 ;данная команда короче  чем две выше
		stosb      ;записать элемент из регистра al в ячеку памяти es:edi
		mov byte[edi],0 ;в конец строки ставим 0, что-бы не вылазил мусор
	@@:
	ret	   ;пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться
