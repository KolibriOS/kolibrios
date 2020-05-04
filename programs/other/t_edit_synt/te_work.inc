
struct TexColViv
	Text  rb MAX_COLOR_WORD_LEN ; слово дл€ подсветки
	f1    dd 0 ; справка по слову
	flags db ? ; f1+4 флаги используемые при выделении
	endc  db ? ; f1+5 символ конца выделени€ (используетс€ при flags&4)
	escc  db ? ; f1+6 экранирующий символ (используетс€ при flags&4)
	color db ? ; f1+7 номер цвета
ends

count_colors_text dd 1
ColWords dd 0
color_cursor dd ?
color_wnd_capt dd ?
color_wnd_work dd ?
color_wnd_bord dd ?
color_select dd ?
color_cur_text dd ?
color_wnd_text dd ?

options_file dd 0
open_file_size dd 0
unpac_mem dd 0
tex_colors dd 0
tex_help_f1 dd 0

Col dd 0 ;указатель на структуры ключевых слов TexColViv

keyUpr dd 0
err_opn db 0 ;ошибка открыти€ файла
err_sav db 0 ;ошибка сохранени€ файла

sc system_colors

run_file_70 FileInfoBlock

align 4
InitColText:
	pushad

	mov ebx,[fn_col_option]
	copy_path ebx,fn_syntax_dir,file_name_rez,0
	copy_path file_name_rez,sys_path,file_name,0

	;[fn_col_option] = '*.syn'
	;fn_syntax_dir = 'info/'
	;file_name_rez = 'info/*.syn'

	mov [run_file_70.Function], SSF_GET_INFO
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], 0
	mov dword[run_file_70.Buffer], open_b
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], file_name
	mcall SF_FILE,run_file_70

	mov ecx,dword[open_b+32] ;+32 qword: размер файла в байтах
	stdcall mem.ReAlloc,[options_file],ecx
	mov [options_file],eax
	mov [open_file_size],ecx
	
	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], ecx
	m2m dword[run_file_70.Buffer], dword[options_file]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], file_name
	mcall SF_FILE,run_file_70 ;загружаем файл изображения
	test eax,eax
	jnz .no_colors
	cmp ebx,0xffffffff
	je .no_colors

	mcall SF_SET_CAPTION,1,file_name

	;init: count_colors_text, ColWords, ...
	mov dword[count_colors_text],1
	mov dword[ColWords],0
	mov dword[tex_help_f1],0

	mov edi,[options_file]
	cmp dword[edi],'KPCK'
	jne .end_unpack
		;notify_window_run ...
		mov ecx,[edi+4]
		cmp [open_file_size],ecx
		jge @f
			;если для распакованого файла не хватает выделенной памяти
			stdcall mem.ReAlloc,edi,ecx
			mov [options_file],eax
			mov [open_file_size],ecx
		@@:
		;выделение памяти для распаковки файла
		stdcall mem.ReAlloc,[unpac_mem],ecx
		mov [unpac_mem],eax

		stdcall unpack,[options_file],eax
		mov edi,[options_file]
		mov esi,[unpac_mem]
		mov ecx,[edi+4]
		cld
		rep movsb
	.end_unpack:
	mov esi,[options_file]
	mov edi,count_colors_text
	mov ecx,9
	rep movsd

	mov eax,[options_file]
	add eax,32
	mov [tex_colors],eax

	mov eax,[count_colors_text] ;init: Col (offset to key words)
	add eax,8
	shl eax,2
	add eax,[options_file]
	mov [Col],eax

	;init: tex_help_f1
	mov ecx,[ColWords]
	imul ecx,sizeof.TexColViv
	add ecx,[Col]
	mov [tex_help_f1],ecx

	;формирование текста в окне текстового редактора
	stdcall [ted_clear], tedit0,1

	stdcall print_capt,capt_header

	stdcall print_capt,col_1
	stdcall print_color,[color_cursor]
	stdcall print_capt,col_2
	stdcall print_color,[color_wnd_capt]
	stdcall print_capt,col_3
	stdcall print_color,[color_wnd_work]
	stdcall print_capt,col_4
	stdcall print_color,[color_wnd_bord]
	stdcall print_capt,col_5
	stdcall print_color,[color_select]
	stdcall print_capt,col_6
	stdcall print_color,[color_cur_text]

	stdcall print_capt,col_7
	mov ecx,[count_colors_text] ;количество цветов текста
	mov eax,[tex_colors]
	@@:
		stdcall print_capt,capt_type_dd
		stdcall print_color,[eax]
		add eax,4
	loop @b

	stdcall print_capt,capt_lbl_wo
	mov ecx,[ColWords]
	mov eax,[Col]
	xor ebx,ebx
	@@: ;цикл по подсвечиваемым словам
		inc ebx
		stdcall print_word,eax,ebx
		add eax,sizeof.TexColViv
	loop @b

	stdcall print_capt,capt_lbl_f1
	mov ecx,[ColWords]
	mov eax,[Col]
	xor ebx,ebx
	@@: ;цикл по справкам к подсв. словам
		inc ebx
		stdcall print_word_f1,eax,ebx
		add eax,sizeof.TexColViv
	loop @b
	
	; подготовка для PathShow
	stdcall [PathShow_prepare], PathShow_data_1
	call draw_window

	.no_colors:
	popad
	ret

align 4
proc print_color, color:dword
	pushad
	mov eax,dword[color]

	mov ebx,eax ;мен€ем местами red и blue
	shr ebx,16
	xchg al,bl
	and eax,0xffff
	and ebx,0xff
	shl ebx,16
	or eax,ebx

	ror al,4
	ror eax,8
	ror al,4
	ror eax,8
	ror al,4
	rol eax,16
	mov edi,text_buffer
	mov word[edi],'0x'
	add edi,2
	mov ecx,6
	@@:
		mov ebx,eax
		and ebx,0xf
		cmp bx,9
		jle .cifra
			add bx,39
		.cifra:
		add bx,48
		mov byte[edi],bl
		inc edi
		shr eax,4
	loop @b
	mov byte[edi],13
	mov ebx,ted_opt_ed_change_time+ted_opt_ed_move_cursor
	stdcall [ted_text_add], tedit0,text_buffer,9,ebx
	popad
	ret
endp

align 4
proc print_capt, capt:dword
	pushad
	mov esi,[capt]
	stdcall str_len,esi
	stdcall [ted_text_add], tedit0,esi,eax,ted_opt_ed_change_time+ted_opt_ed_move_cursor
	popad
	ret
endp

align 4
proc print_word, text:dword, index:dword
	pushad
	mov edi,text_buffer
	mov byte[edi],0
	stdcall str_cat,edi,capt_beg_word
	stdcall str_len,edi
	add edi,eax

	mov ecx,[text]	
	stdcall code_string,ecx,edi ;кодируем строку с ключевым словом
	stdcall str_cat,edi,capt_end_word

	cmp dword[ecx+TexColViv.f1],0
	je @f
		stdcall str_cat_value,edi,[index]
	@@:

	stdcall str_len,edi
	add edi,eax
	mov word[edi],','
	inc edi
	movzx eax,byte[ecx+TexColViv.flags]
	stdcall str_cat_value,edi,eax
	stdcall str_len,edi
	add edi,eax
	mov word[edi],','
	inc edi

	movzx eax,word[ecx+TexColViv.endc] ;(escc shl 8)+endc
	or eax,eax ;проверяем есть ли завершающий и экранинующий символ
	jz @f
		stdcall str_cat_value,edi,eax
		stdcall str_len,edi
		add edi,eax
	@@:
	mov word[edi],','
	inc edi

	movzx eax,byte[ecx+TexColViv.color]
	stdcall str_cat_value,edi,eax
	stdcall str_len,edi
	add edi,eax
	mov word[edi],13

	mov esi,text_buffer
	stdcall str_len,esi
	stdcall [ted_text_add], tedit0,esi,eax,ted_opt_ed_change_time+ted_opt_ed_move_cursor

	popad
	ret
endp

align 4
proc print_word_f1, text:dword, index:dword
	pushad
	mov edi,[text]
	mov esi,[edi+TexColViv.f1]
	cmp esi,0
	je @f
		mov ebx,text_buffer
		mov byte[ebx],0
		stdcall str_cat,ebx,capt_point
		stdcall str_cat_value,ebx,[index]
		stdcall str_cat,ebx,capt_beg_db
		stdcall str_len,ebx
		add ebx,eax
		
		add esi,[tex_help_f1]
		stdcall code_string,esi,ebx ;кодируем строку
		stdcall str_cat,ebx,capt_end_word_f1

		mov ebx,text_buffer
		stdcall str_len,ebx
		stdcall [ted_text_add], tedit0,ebx,eax,ted_opt_ed_move_cursor
	@@:
	popad
	ret
endp

;description:
; кодировка строки, замена непечатаемых символов на числовые коды
align 4
proc code_string, text:dword, out_text:dword
	pushad
	mov esi,[text]
	stdcall str_len,esi
	mov ecx,eax
	mov edi,[out_text]
	or ecx,ecx
	jz .end_copy ;если строка со справкой пустая

	.copy:
		mov al,byte[esi]
		inc esi
		cmp al,9
		je @f
		cmp al,13
		je @f
		cmp al,39
		je @f
			mov byte[edi],al
			inc edi
			loop .copy
			jmp .end_copy
		@@:
			mov byte[edi],39
			mov byte[edi+1],','
			add edi,2
			and eax,0xff
			mov dword[edi],0
			stdcall convert_int_to_str,4
			stdcall str_len,edi
			add edi,eax
			mov byte[edi],','
			mov byte[edi+1],39
			add edi,2
	loop .copy
	.end_copy:
	mov byte[edi],0
	popad
	ret
endp

align 4
proc mem_clear, mem:dword, len:dword
	push eax ecx edi
	cld
	xor al,al
	mov edi, dword[mem]
	mov ecx, dword[len]
	repne stosb
	pop edi ecx eax
	ret
endp

align 4
proc str_cat, str1:dword, str2:dword
	push eax ecx edi esi
	mov esi,dword[str2]
	stdcall str_len,esi
	mov ecx,eax
	inc ecx
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	pop esi edi ecx eax
	ret
endp

align 4
proc str_cat_value, str1:dword, value:dword
	pushad
	mov edi,[str1]
	stdcall str_len,edi
	add edi,eax
	mov eax,[value]
	stdcall convert_int_to_str,12
	popad
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
