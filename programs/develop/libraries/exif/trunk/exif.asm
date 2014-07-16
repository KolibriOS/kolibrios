format MS COFF
public EXPORTS
section '.flat' code readable align 16

include '../../../../macros.inc'
include '../../../../proc32.inc'



;---------
offs_m_or_i    equ 8 ;смещение параметра 'MM' или 'II' (Motorola, Intel)
offs_tag_count equ 16 ;смещение количества тегов
offs_tag_0     equ 18 ;смещение 0-го тега
tag_size       equ 12 ;размер структуры тега
;форматы данных
tag_format_ui1b  equ  1 ;unsigned integer 1 byte
tag_format_text  equ  2 ;ascii string
tag_format_ui2b  equ  3 ;unsigned integer 2 byte
tag_format_ui4b  equ  4 ;unsigned integer 4 byte
tag_format_urb   equ  5 ;unsigned integer 4/4 byte
tag_format_si1b  equ  6 ;signed integer 1 byte
tag_format_undef equ  7 ;undefined
tag_format_si2b  equ  8 ;signed integer 2 byte
tag_format_si4b  equ  9 ;signed integer 4 byte
tag_format_srb   equ 10 ;signed integer 4/4 byte
tag_format_f4b	 equ 11 ;float 4 byte
tag_format_f8b	 equ 12 ;float 8 byte

align 4
txt_dp db ': ',0
txt_div db '/',0

;
align 4
exif_tag_numbers:

db 0x01,0x0e,'Image description',0
db 0x01,0x0f,'Manufacturer of digicam',0
db 0x01,0x10,'Model',0
db 0x01,0x12,'Orientation',0
db 0x01,0x1a,'X resolution',0
db 0x01,0x1b,'Y resolution',0
db 0x01,0x28,'Resolution unit',0
db 0x01,0x31,'Software',0
db 0x01,0x32,'Date time',0
db 0x01,0x3e,'White point',0
db 0x01,0x3f,'Primary chromaticities',0
db 0x02,0x11,'YCbCrCoefficients',0
db 0x02,0x13,'YCbCrPositioning',0
db 0x02,0x14,'Reference black white',0
db 0x82,0x98,'Copyright',0
db 0x87,0x69,'Exif offset',0

db 0x88,0x25,'GPS Info',0

db 0xa4,0x01,'Custom rendered',0
db 0xa4,0x02,'Exposure mode',0
db 0xa4,0x03,'White balance',0
db 0xa4,0x04,'Digital zoom ratio',0
db 0xa4,0x05,'Focal length in 35mm format',0
db 0xa4,0x06,'Scene capture type',0
db 0xa4,0x07,'Gain control',0
db 0xa4,0x08,'Contrast',0
db 0xa4,0x09,'Saturation',0
db 0xa4,0x0a,'Sharpness',0
db 0xa4,0x0b,'Device setting description',0
db 0xa4,0x0c,'Subject distance range',0
db 0xa4,0x20,'Image unique ID',0
db 0xa4,0x30,'Owner name',0
db 0xa4,0x31,'Serial number',0
db 0xa4,0x32,'Lens info',0
db 0xa4,0x33,'Lens make',0
db 0xa4,0x34,'Lens model',0
db 0xa4,0x35,'Lens serial number',0
db 0xa4,0x80,'GDAL metadata',0
db 0xa4,0x81,'GDAL no data',0
db 0xa5,0x00,'Gamma',0
db 0xaf,0xc0,'Expand software',0
db 0xaf,0xc1,'Expand lens',0
db 0xaf,0xc2,'Expand film',0
db 0xaf,0xc3,'Expand filterLens',0
db 0xaf,0xc4,'Expand scanner',0
db 0xaf,0xc5,'Expand flash lamp',0

db 0xea,0x1c,'Padding',0
dw 0


;input:
; bof - указатель на начало файла
; app1 - указатель для заполнения exif.app1
;output:
; app1 - указатель на начало exif.app1 (или 0 если не найдено или формат файла не поддерживается)
align 4
proc exif_get_app1 uses eax ebx edi, bof:dword, app1:dword
	mov eax,[bof]
	mov edi,[app1]

	;файл в формате jpg?
	cmp word[eax],0xd8ff
	jne .no_exif
	add eax,2

	;файл содержит exif.app0?
	cmp word[eax],0xe0ff
	jne @f
		add eax,2
		movzx ebx,word[eax]
		ror bx,8 ;всегда ли так надо?
		add eax,ebx
	@@:

	;файл содержит exif.app1?
	cmp word[eax],0xe1ff
	jne .no_exif

	add eax,2
	mov [edi],eax

	jmp @f
	.no_exif:
		mov dword[edi],0
	@@:
	ret
endp

;input:
; app1 - указатель на начало exif.app1
; num - порядковый номер тега (начинается с 1)
; txt - указатель на текст, куда будет записано значение
; t_max - максимальный размер текста
align 4
proc exif_get_app1_tag, app1:dword, num:dword, txt:dword, t_max:dword
pushad
	mov eax,[app1]
	mov edi,[txt]
	mov ecx,[num]

	xor edx,edx
	cmp eax,edx
	je .end_f ;если не найден указатель на начало exif.app1
	cmp ecx,edx
	jle .end_f ;если порядковый номер тега <= 0

	mov byte[edi],0
	cmp word[eax+offs_m_or_i],'II'
	je @f
		inc edx ;if 'MM' edx=1
	@@:

	;проверяем число тегов
	movzx ebx,word[eax+offs_tag_count]
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp ecx,ebx
	jg .end_f ;если номер тега больше чем их есть в файле

	;переходим на заданный тег
	dec ecx
	imul ecx,tag_size
	add eax,offs_tag_0
	add eax,ecx

	;читаем назначение тега
	push exif_tag_numbers
	pop esi
	.next_tag:
	mov bx,word[esi]
	cmp bx,0
	je .tag_unknown ;тег не опознан
	bt edx,0
	jc @f
		ror bx,8
	@@:
	cmp word[eax],bx
	je .found
	inc esi
	@@:
		inc esi
		cmp byte[esi],0
		jne @b
	inc esi
	jmp .next_tag
	.found:

	;копируем строку
	add esi,2
	stdcall str_n_cat,edi,esi,[t_max]

	jmp @f
	.tag_unknown:
		mov dword[edi],'???'
		mov byte[edi+3],0
	@@:

	;читаем информацию в теге
	mov bx,tag_format_text
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_02
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size ;проверяем длинну строки
		cmp ebx,4
		jg @f
			;если строка помещается в 4 символа
			mov esi,eax
			add esi,8
			stdcall str_n_cat,edi,esi,[t_max]
			jmp .end_f
		;если строка не помещается в 4 символа
		@@:
		mov esi,dword[eax+8]
		bt edx,0
		jnc @f
			ror si,8
			ror esi,16
			ror si,8
		@@:
		add esi,offs_m_or_i
		add esi,[app1]
		stdcall str_n_cat,edi,esi,[t_max]
		jmp .end_f
	.tag_02:

	mov bx,tag_format_ui2b
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_03
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size
		cmp ebx,1
		jg .over4b_03
			;если одно 2 байтовое число
			movzx ebx,word[eax+8]
			bt edx,0
			jnc @f
				ror bx,8
			@@:
			stdcall str_len,edi
			add edi,eax
			mov eax,ebx
			call convert_int_to_str ;[t_max]
		.over4b_03:
			;...
		jmp .end_f
	.tag_03:

	mov bx,tag_format_ui4b
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_04
		stdcall str_n_cat,edi,txt_dp,[t_max]
		call get_tag_data_size
		cmp ebx,1
		jg .over4b_04
			;если одно 4 байтовое число
			mov ebx,dword[eax+8]
			bt edx,0
			jnc @f
				ror bx,8
				ror ebx,16
				ror bx,8
			@@:
			stdcall str_len,edi
			add edi,eax
			mov eax,ebx
			call convert_int_to_str ;[t_max]
		.over4b_04:
			;...
		jmp .end_f
	.tag_04:

	mov bx,tag_format_urb
	bt edx,0
	jnc @f
		ror bx,8
	@@:
	cmp word[eax+2],bx
	jne .tag_05
		stdcall str_n_cat,edi,txt_dp,[t_max]
		;call get_tag_data_size
		;cmp ebx,1
		;jg .over4b_05
			mov ebx,dword[eax+8]
			bt edx,0
			jnc @f
				ror bx,8
				ror ebx,16
				ror bx,8
			@@:
			stdcall str_len,edi
			add edi,eax
			add ebx,offs_m_or_i
			add ebx,[app1]
			mov eax,[ebx]
			bt edx,0
			jnc @f
				ror ax,8
				ror eax,16
				ror ax,8
			@@:
			call convert_int_to_str ;ставим 1-е число
			stdcall str_n_cat,edi,txt_div,[t_max] ;ставим знак деления
			stdcall str_len,edi
			add edi,eax
			mov eax,[ebx+4]
			bt edx,0
			jnc @f
				ror ax,8
				ror eax,16
				ror ax,8
			@@:
			call convert_int_to_str ;ставим 2-е число
		;.over4b_05:
			;...
		;jmp .end_f
	.tag_05:

	.end_f:
popad
	ret
endp

;input:
; eax - tag pointer
; edx - 1 if 'MM', 0 if 'II'
;output:
; ebx - data size
align 4
get_tag_data_size:
	mov ebx,dword[eax+4]
	bt edx,0
	jnc @f
		ror bx,8
		ror ebx,16
		ror bx,8
	@@:
	ret

align 4
proc str_n_cat uses eax ecx edi esi, str1:dword, str2:dword, n:dword
	mov esi,dword[str2]
	mov ecx,dword[n]
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	mov byte[edi],0
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

;input:
; eax = value
; edi = string buffer
;output:
align 4
convert_int_to_str:
	pushad
		mov dword[edi+1],0
		mov dword[edi+5],0
		call .str
	popad
	ret

align 4
.str:
	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
    ;преревод числа в ASCII строку взодные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
	cmp eax,ecx  ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx  ;очистить edx
		div ecx      ;разделить - остаток в edx
		push edx     ;положить в стек
		;dec edi             ;смещение необходимое для записи с конца строки
		call .str ;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
	or al,0x30  ;данная команда короче  чем две выше
	stosb	    ;записать элемент из регистра al в ячеку памяти es:edi
	ret	      ;вернуться чень интересный ход т.к. пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться



align 16
EXPORTS:
	dd sz_exif_get_app1, exif_get_app1
	dd sz_exif_get_app1_tag, exif_get_app1_tag
	dd 0,0
	sz_exif_get_app1 db 'exif_get_app1',0
	sz_exif_get_app1_tag db 'exif_get_app1_tag',0
