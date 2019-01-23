;
; Строковые функции
;

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

;description:
; проверяет содержится ли строка str1 в строке str0
; проверка делается только начиная с первых символов, указанных в str0 и str1
; пример 1: если str0='aaabbbccc', str1='bbb' совпадения не будет
; пример 2: если str0='aaabbbccc', str1='aaa' совпадение будет
;output:
; al = 0 если строка str1 содержится в str0
; al != 0 если строка str1 не содержится в str0
align 4
proc str_instr uses edi esi, str0:dword, str1:dword
	;xor eax,eax
	mov edi,[str0]
	mov esi,[str1]
	cld
	@@:
		lodsb
		or al,al
		jz .e1
		scasb ;сравниваем символы
	jz @b ;если совпали, то переходим к сравнению следующих
	;сюда попадаем если строки не совпали
	sub al,[edi-1]
	.e1: ;сюда попадаем если строка str1 (esi) закончилась
	ret
endp

;input:
; eax - число
; edi - буфер для строки
; len - длинна буфера
;output:
align 4
proc convert_int_to_str uses eax ecx edx edi esi, len:dword
	mov esi,[len]
	add esi,edi
	dec esi
	call .str
	ret
endp

align 4
.str:
	mov ecx,10
	cmp eax,ecx
	jb @f
		xor edx,edx
		div ecx
		push edx
		;dec edi  ;смещение необходимое для записи с конца строки
		call .str
		pop eax
	@@:
	cmp edi,esi
	jge @f
		or al,0x30
		stosb
		mov byte[edi],0 ;в конец строки ставим 0, что-бы не вылазил мусор
	@@:
	ret