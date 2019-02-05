struct symbol
	c db ?    ; +0 символ
	col db ?  ; +1 цвет
	perv dd ? ; +2
	next dd ? ; +6 указатели
	tc dd ?   ;+10 врем. создания
	td dd ?   ;+14 врем. удаления
ends

ID_BUT_0 equ 3 ;минимальный ID кнопки на панели

ID_BUT_NEW equ 3
ID_BUT_OPEN equ 4
ID_BUT_SAVE equ 5
ID_BUT_SAVE_AS equ 6
ID_BUT_SELECT equ 7
ID_BUT_CUT equ 8
ID_BUT_COPY equ 9
ID_BUT_PASTE equ 10
ID_BUT_FIND equ 11
ID_BUT_REPLACE equ 12
ID_BUT_KEY_WORDS equ 13
ID_BUT_UPPER equ 14
ID_BUT_LOWER equ 15
ID_BUT_REVERSE equ 16
ID_BUT_UNDO equ 17
ID_BUT_REDO equ 18
ID_BUT_INVISIBLE equ 19
ID_BUT_SYNTAX_LIST equ 20
ID_BUT_SYNTAX_MODE equ 21
ID_BUT_CONVERT_1251_866 equ 22
ID_BUT_CONVERT_866_1251 equ 23

macro ini_panel b_key,b_id
{
	stdcall [ini_get_int],file_name,ini_sec_window,b_key,1
	mov byte[panel_but+b_id-ID_BUT_0],al
}

bmp_icon dd 0

align 16
button:
	mcall SF_GET_BUTTON
	cmp ah,ID_BUT_NEW
	jne @f
		call ted_but_new_file
		jmp still
	@@:
	cmp ah,ID_BUT_OPEN
	jne @f
		call ted_but_open_file
		jmp still
	@@:
	cmp ah,ID_BUT_SAVE
	jne @f
		xor eax,eax
		call ted_but_save_file ;сохранение без диалога
		jmp still
	@@:
	cmp ah,ID_BUT_SAVE_AS
	jne @f
		xor eax,eax
		inc eax
		call ted_but_save_file ;сохранение с диалогом
		jmp still
	@@:
	cmp ah,ID_BUT_SELECT
	jne @f
		stdcall [ted_but_select_word], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_CUT
	jne @f
		stdcall [ted_but_cut], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_COPY
	jne @f
		stdcall [ted_but_copy], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_PASTE
	jne @f
		stdcall [ted_but_paste], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_FIND
	jne @f
		call but_panel_find
		jmp still
	@@:
	cmp ah,ID_BUT_REPLACE
	jne @f
		call but_panel_replace
		jmp still
	@@:
	cmp ah,ID_BUT_KEY_WORDS
	jne @f
		call but_find_key_w
		jmp still
	@@:
	cmp ah,ID_BUT_UPPER
	jne @f
		stdcall [ted_but_sumb_upper], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_LOWER
	jne @f
		stdcall [ted_but_sumb_lover], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_REVERSE
	jne @f
		stdcall [ted_but_reverse], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_UNDO
	jne @f
		stdcall [ted_but_undo], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_REDO
	jne @f
		stdcall [ted_but_redo], tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_INVISIBLE
	jne @f
		stdcall but_sumb_invis, tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_SYNTAX_LIST
	jne @f
		stdcall but_use_syntax, tedit0
		jmp still
	@@:
	cmp ah,ID_BUT_SYNTAX_MODE
	jne @f
		call but_panel_choose_syntax
		jmp still
	@@:
	cmp ah,ID_BUT_CONVERT_1251_866
	jne @f
		stdcall [ted_but_convert_by_table],tedit0,tbl_1251_866
		jmp still
	@@:
	cmp ah,ID_BUT_CONVERT_866_1251
	jne @f
		stdcall [ted_but_convert_by_table],tedit0,tbl_866_1251
		jmp still
	@@:

	cmp ah,200
	jne @f
		stdcall ted_but_open_syntax, tedit0
		jmp still
	@@:
	cmp ah,201 ;найти далее
	jne .no_find
		call get_find_options
		stdcall [ted_but_find], tedit0, eax
		jmp still
	.no_find:
	cmp ah,202 ;заменить
	jne .no_change
		call get_find_options
		stdcall [ted_but_replace], tedit0, buf_replace, eax, 1
		or eax,eax
		jz still
		stdcall [ted_draw], tedit0
		jmp still
	.no_change:
	cmp ah,203 ;заменить все
	jne @f
		call but_replace_all
		jmp still
	@@:
	cmp ah,204 ;перейти на строку
	jne @f
		call but_goto_line
		jmp still
	@@:

	cmp ah,1
	jne @f
		call ted_Exit
		jmp still
	@@:

	cmp ah,199
	jne still
		call ted_Exit
	jmp still

align 4
ted_Exit:
	cmp dword[wnd_k_words_run],0
	je @f
		push ebx ecx
		mcall SF_SYSTEM, SSF_TERMINATE_THREAD_ID,[wnd_k_words_run]
		pop ecx ebx
		mov dword[wnd_k_words_run],0
		;test eax,eax
	@@:
	cmp dword[exit_code],1
	je @f
	stdcall [ted_can_save], tedit0
	cmp al,1
	jne @f
		stdcall [mb_create],msgbox_2,thread ;message: save buf in file?
		stdcall [mb_setfunctions],msgbox_2E_funct
		ret
	@@:
	stdcall mem.Free,[bmp_icon]
	cmp dword[unpac_mem],0
	je @f
		stdcall mem.Free,[unpac_mem]
	@@:
	stdcall [ted_delete], tedit0
	stdcall [tl_data_clear], tree1
	mcall SF_TERMINATE_PROCESS ;выход из программы
	;ret

align 4
get_find_options:
	xor eax,eax
	cmp [opt_gr1],opt1
	je @f
		inc eax
	cmp [opt_gr1],opt2
	je @f
		inc eax
	@@:
	bt dword[ch1.flags],1
	jc @f
		bts eax,30
	@@:
	ret

align 4
proc but_replace_all uses ebx ecx edx
	call get_find_options
	mov ebx,eax
	stdcall [ted_but_replace], tedit0, buf_replace, ebx, 1
	xor edx,edx
	or eax,eax
	jz @f
	inc edx
	mov ecx,0x1000
	.cycle0:
		stdcall [ted_but_replace], tedit0, buf_replace, ebx, 0
		or eax,eax
		jz @f
		inc edx
		loop .cycle0
	@@:
	or edx,edx
	jz @f
		stdcall [ted_draw], tedit0
	@@:
	mov eax,edx
	mov edi,msgbox_1.z
	stdcall convert_int_to_str, 8
	stdcall str_cat, edi,txt_Info
	notify_window_run msgbox_1
	ret
endp

align 4
proc but_goto_line uses eax
	stdcall conv_str_to_int, buf_goto
	stdcall [ted_go_to_position], tedit0,eax,1
	ret
endp

;input:
; al = код ошибки
align 4
ted_on_init_synt_err:
	mov byte[msgbox_3.err],'0'
	add byte[msgbox_3.err],al
	stdcall [mb_create],msgbox_3,thread ;message: Can-t open color options file!
	ret

align 4
ted_but_new_file:
	push eax ebx
	stdcall [ted_can_save], tedit0
	cmp al,1
	jne @f
		stdcall [mb_create],msgbox_2,thread ;message: save changes in file?
		stdcall [mb_setfunctions],msgbox_2N_funct
		jmp .ret_f
	@@:
	call On_NewFile
	.ret_f:
		mov byte[openfile_path],0
		mcall SF_SET_CAPTION,1,hed
	pop ebx eax
	ret

align 4
On_NewFile:
	stdcall [ted_clear], tedit0,1
	call draw_but_toolbar
	stdcall [ted_draw], tedit0
	mov dword[openfile_path],0
	ret

align 4
On_SaveAndNewFile:
	push edi
	mov edi, tedit0

	call ted_but_save_file
	cmp ted_err_save,0
	jne @f
		call On_NewFile
	@@:
	pop edi
	ret

align 4
On_SaveAndOpenFile:
	push edi
	mov edi, tedit0

	call ted_but_save_file
	cmp ted_err_save,0
	jne @f
		call ted_but_open_file.no_msg
	@@:
	pop edi
	ret

align 4
On_SaveAndExit:
	push edi
	mov edi, tedit0

	call ted_but_save_file
	cmp ted_err_save,0
	jne @f
		mov dword[exit_code],1
	@@:
	pop edi
	ret

align 4
On_Exit:
	mov dword[exit_code],1
	ret

;description:
; функция открытия файла без окна сообщения
align 4
proc but_no_msg_OpenFile uses eax ebx esi
	stdcall [ted_open_file], tedit0,run_file_70,openfile_path
	call ted_messages_after_open_file

	mov esi,openfile_path
	stdcall str_len,esi
	add esi,eax
	@@: ;цикл для поиска начала имени файла
		dec esi
		cmp byte[esi],'/'
		je @f
		cmp byte[esi],0x5c ;'\'
		je @f
		cmp esi,openfile_path
		jg @b
	@@:
	inc esi
	stdcall str_len,esi
	cmp eax,255
	jle @f
		mov eax,255
	@@:
	inc eax ;для 0 в конце строки
	stdcall mem_cpy, filename_area,esi,eax ;копируем имя файла в диалог сохранения
	ret
endp

;description:
; функция вызываемую при нажатии Ctrl+N,O,F,S,H,G
align 4
proc ted_but_ctrl_all uses eax, opt_key:dword
	mov eax,[opt_key]
	cmp al,'N' ;Ctrl+N
	jne @f
		call ted_but_new_file
		jmp .end0
	@@:
	cmp al,'O' ;Ctrl+O
	jne @f
		call ted_but_open_file
		jmp .end0
	@@:
	cmp al,'S' ;Ctrl+S
	jne @f
		shr eax,8
		call ted_but_save_file
		jmp .end0
	@@:
	cmp al,'F' ;Ctrl+F
	jne @f
		call but_panel_find
		jmp .end0
	@@:
	cmp al,'G' ;Ctrl+G
	jne @f
		call but_panel_goto
		jmp .end0
	@@:
	cmp al,'H' ;Ctrl+H
	jne .end0
		call but_panel_replace
	.end0:
	ret
endp

;description:
; функция открытия файла с возможным окном сообщения (о возможности отменить открытие)
align 4
ted_but_open_file:
	pushad

	stdcall [ted_can_save], tedit0
	cmp al,1
	jne @f
		stdcall [mb_create],msgbox_2,thread ;message: save changes in file?
		stdcall [mb_setfunctions],msgbox_2O_funct
		jmp .ret_f
	.no_msg: ;метка для вызова функции через кнопку в сообщении msgbox_5
		pushad
	@@:

	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	jne @f
		stdcall mem_spac, msgbox_9.fdp,100
		stdcall str_len,file_name
		cmp eax,100
		jle .no_crop
			mov eax,100
		.no_crop:
		stdcall mem_cpy, msgbox_9.fdp,esi,eax
		stdcall [mb_create],msgbox_9,thread ;message: error run 'open file dialog'
		jmp .ret_f
	@@:
	cmp [OpenDialog_data.status],1
	jne .ret_f
		stdcall auto_open_syntax,[OpenDialog_data.openfile_path]
		stdcall [ted_open_file], tedit0,run_file_70,openfile_path
		call ted_messages_after_open_file
	.ret_f:
	popad
	ret

proc auto_open_syntax, of_path:dword
pushad
	;ищем есть ли файл подсветки для открываемого текстового файла
	stdcall str_len,[of_path]
	mov edx,[of_path]
	add edx,eax
	.f_beg:
	cmp byte[edx],'.'
	je .found
		dec edx
		cmp edx,[of_path]
		jg .f_beg
	.found:
	inc edx ;edx - указатель на расширение открытого файла

	mov ebx,synt_auto_open
	.cycle_0:
		add ebx,32
		stdcall strcmp,ebx,edx ;сравниваем расширение открытого файла со значениями открытыми с ini файла
		test eax,eax
		jz .ok
		add ebx,32
		cmp byte[ebx],0
		jne .cycle_0
	jmp .end_0
	.ok: ;открытие файла подсветки в зависимости от найденного расширения
		sub ebx,32
		stdcall open_unpac_synt_file,ebx
	.end_0:
popad
	ret
endp

;input:
; eax если равен: 0 - сохранение без диалога, 1 - сохранение с диалогом
;description:
; функция сохранения файла
align 4
proc ted_but_save_file
	or eax,eax
	jnz .init_dlg
		; сохраняем без вызова диалогового окна
		cmp byte[openfile_path],0
		jne @f
	.init_dlg:
		; вызов диалогового окна для сохранения файла
		copy_path open_dialog_name,communication_area_default_path,file_name,0
		mov [OpenDialog_data.type],1
		stdcall [OpenDialog_Start],OpenDialog_data
		cmp [OpenDialog_data.status],1 ;if status==1 then save
		jne .end_save
	@@:
		stdcall [ted_save_file],tedit0,run_file_70,openfile_path
	.end_save:
	ret
endp

;description:
; копирование памяти
align 4
proc mem_cpy uses ecx esi edi, destination:dword, source:dword, len:dword
	cld
	mov esi,[source]
	mov edi,[destination]
	mov ecx,[len]
	rep movsb
	ret
endp

;description:
; заполнение памяти пробелами
align 4
proc mem_spac uses eax ecx edi, mem:dword, len:dword
	cld
	mov al,' '
	mov edi,[mem]
	mov ecx,[len]
	repne stosb
	ret
endp

;input:
; eax = код ошибки
; ebx = колличество прочитанных байт
align 4
ted_messages_after_open_file:
	push ecx edi
	or eax,eax
	jz @f
		cmp ax,10
		jl .zifra_0_9
			mov al,'?'
			sub ax,48
		.zifra_0_9:
		add ax,48

		mov byte[msgbox_4.err],al
		stdcall [mb_create],msgbox_4,thread ;message: Can-t open text file!
	@@:
	pop edi ecx
	ret

align 4
ted_save_err_msg:
	mov byte[msgbox_6.err],al
	stdcall [mb_create],msgbox_6,thread ;message: Can-t save text file!
	ret

;description:
; функция нажатия на кнопку [Применить] в панели синтаксиса
align 4
proc ted_but_open_syntax uses eax ebx ecx edi, edit:dword
	mov edi,[edit]
	cmp ted_panel_id,TED_PANEL_SYNTAX
	jne @f
		stdcall [tl_node_get_data], tree1
		mov [fn_col_option],eax
		stdcall open_unpac_synt_file,eax
		cmp ebx,-1
		je @f

		call [ted_text_colored]
		stdcall [ted_draw],edi
	@@:
	ret
endp

;открытие и распаковка файла подсветки синтаксиса
;input:
; f_name - указатель на имя открываемого файла синтаксиса (без пути и папки info)
;output:
; ebx - число прочитанных байт из файла
; ecx - разрушается
align 4
proc open_unpac_synt_file uses eax edi esi, f_name:dword
	mov ebx,[f_name]
	cmp byte[ebx],0
	je @f
	mov edi,last_open_synt_file
	stdcall strcmp,ebx,edi ;сравниваем имя подключенного файла с подключенным ранее файлом
	test eax,eax
	jz @f

	;копируем имя подключенного файла (для избежания повторных подключений)
	mov esi,ebx
	mov ecx,32/4
	cld
	rep movsd

	copy_path ebx,fn_syntax_dir,syntax_path,0
	copy_path syntax_path,sys_path,file_name,0
	mov edi, tedit0
	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov ecx, ted_syntax_file_size
	mov dword[run_file_70.Count], ecx
	m2m dword[run_file_70.Buffer], ted_syntax_file
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], file_name
	mcall SF_FILE, run_file_70
	cmp ebx,-1
	jne .end_0
		call ted_on_init_synt_err
		jmp @f
	.end_0:
		mov eax,ted_syntax_file
		cmp dword[eax],'KPCK'
		jne .end_unpack
		cmp dword[eax+4],ecx
		jg @f ;если для распакованого файла не хватает выделенной памяти
		cmp dword[unpac_mem],0
		jne .end_1
			;выделение памяти для распаковки файла
			stdcall mem.Alloc,ecx
			mov [unpac_mem],eax
		.end_1:
		stdcall unpack,ted_syntax_file,[unpac_mem]
		mov edi,ted_syntax_file
		mov esi,[unpac_mem]
		mov ecx,[edi+4]
		cld
		rep movsb
		.end_unpack:
		stdcall [ted_init_syntax_file], tedit0
	@@:
	ret
endp

;description:
; вызов/скрытие панели поиска
align 4
proc but_panel_find
	stdcall on_panel_change, tedit0,TED_PANEL_FIND
	;or word[edit_find.flags],ed_focus
	ret
endp

;description:
; вызов/скрытие панели перехода
align 4
proc but_panel_goto
	stdcall on_panel_change, tedit0,TED_PANEL_GOTO
	;or word[edit_goto.flags],ed_focus
	ret
endp

;description:
; вызов/скрытие панели замены
align 4
proc but_panel_replace
	stdcall on_panel_change, tedit0,TED_PANEL_REPLACE
	;or word[edit_find.flags],ed_focus
	ret
endp

;description:
; вызов/скрытие панели с файлами синтаксиса
align 4
proc but_panel_choose_syntax
	stdcall on_panel_change, tedit0,TED_PANEL_SYNTAX
	ret
endp

align 4
proc on_panel_change uses eax edi, edit:dword, new_id:dword
	mov edi,[edit]
	mov eax,[new_id]

	;if(panel_id==new_id) panel_id=0, draw all
	cmp ted_panel_id,al
	jne @f
		mov ted_panel_id,TED_PANEL_NULL
		mov ted_wnd_l,0
		jmp .e_if
	@@:
	;elseif(panel_id==0) panel_id=new_id, draw all
	cmp ted_panel_id,TED_PANEL_NULL
	jne @f
		mov ted_panel_id,al
		mov ted_wnd_l,TED_PANEL_WIDTH
	.e_if:
	call EvSize
	stdcall [ted_draw],edi
	jmp .end0
	;else panel_id=new_id, draw panel
	@@:
		mov ted_panel_id,al
		stdcall draw_panels,edi
	.end0:
	ret
endp

align 4
tbl_1251_866:
rb 128
db   0,  0,  0,  0,  0,  0,  0,  0,  0,0,0,0,0,0,0,0 ;128
db   0,  0,  0,  0,  0,  0,  0,  0,  0,0,0,0,0,0,0,0 ;144
db   0,  0,  0,  0,  0,  0,  0,  0,240,0,0,0,0,0,0,0 ;160
db   0,  0,  0,  0,  0,  0,  0,  0,241,0,0,0,0,0,0,0 ;176
db 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 ;192
db 144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 ;208
db 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 ;224
db 224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 ;240

align 4
tbl_866_1251:
rb 128
db 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 ;128
db 208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 ;144
db 224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 ;160
db   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ;176
db   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ;192
db   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ;208
db 240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 ;224
db 168,184,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ;240

align 4
ted_on_find_err:
	notify_window_run msgbox_7 ;message: Can not find text
	ret

;description:
; функция поиска информации в списке ключевых слов
align 4
but_find_key_w:
	cmp dword[wnd_k_words_run],0
	jne @f
		pushad
		mcall SF_CREATE_THREAD,1,prop_start,thread_coords
		mov [wnd_k_words_run],eax
		popad
	@@:
	ret

;description:
; выделение/скрытие непечатаемых символов
align 4
proc but_sumb_invis uses edi, edit:dword
	mov edi,[edit]

	xor ted_mode_invis,1
	call draw_but_toolbar
	stdcall [ted_draw],edi
	ret
endp

;description:
; выделение/скрытие ключевых слов
align 4
proc but_use_syntax uses edi, edit:dword
	mov edi,[edit]

	xor ted_mode_color,1
	cmp ted_mode_color,0
	je @f
		call [ted_text_colored]
	@@:
	call draw_but_toolbar
	stdcall [ted_draw],edi
	ret
endp

