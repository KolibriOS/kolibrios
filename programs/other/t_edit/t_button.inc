struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

struct symbol
  c db ?    ;  +0 символ
  col db ?  ;  +1 цвет
  perv dd ? ;  +2
  next dd ? ;  +6 указатели
  tc dd ?   ; +10 врем. создания
  td dd ?   ; +14 врем. удаления
ends


hed db 'TextEditor 22.08.11',0 ;подпись окна
sc system_colors

fn_icon db 'te_icon.bmp',0
bmp_icon dd 0
run_file_70 FileInfoBlock 	  


;input:
; al = код ошибки
align 4
ted_on_init_synt_err:
	mov byte[msgbox_3.err],al
	stdcall [mb_create],msgbox_3,thread ;message: Can-t open color options file!
	ret


align 4
ted_but_new_file:
  push eax ebx
  stdcall [ted_can_save], tedit0
  cmp al,1
  jne @f
    stdcall [mb_create],msgbox_2,thread ;message: save changes in file?
    ;mov eax,5
    ;mov ebx,50
    ;int 0x40
    stdcall [mb_setfunctions],msgbox_2_funct
    jmp .ret_f
  @@:
  call On_NewFile
  .ret_f:
  pop ebx eax
  ret

align 4
On_NewFile:
  stdcall [ted_clear], tedit0,1
  call draw_but_toolbar
  stdcall [ted_draw], tedit0
  ret

align 4
On_SaveAndNewFile:
  push edi
  mov edi, tedit0

  stdcall [ted_but_save_file],edi,run_file_70,[edit1.text]
  cmp ted_err_save,0
  jne @f
    call On_NewFile
  @@:
  pop edi
  ret

;description:
; функция открытия файла без окна сообщения
align 4
but_no_msg_OpenFile:
	push eax ebx
	stdcall [ted_open_file], tedit0,run_file_70,[edit1.text]
	call ted_messages_after_open_file
	pop ebx eax
	ret

;description:
; функция открытия файла с возможным окном сообщения (о возможности отменить открытие)
align 4
ted_but_open_file:
	pushad

	stdcall [ted_can_save], tedit0
	cmp al,1
	jne @f
		stdcall [mb_create],msgbox_5,thread ;message: save changes in file?
		stdcall [mb_setfunctions],msgbox_5_funct
		jmp .ret_f
	@@:

	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	jne @f
		stdcall mem_spac, msgbox_9.fdp,100
		mov esi,file_name
		call strlen
		cmp eax,100
		jle .no_crop
			mov eax,100
		.no_crop:
		stdcall mem_copy, esi,msgbox_9.fdp,eax
		stdcall [mb_create],msgbox_9,thread ;message: error run 'open file dialog'
		jmp .ret_f
	@@:
	mov esi,[OpenDialog_data.openfile_path]
	call strlen
	mov [edit1.size],eax
	mov [edit1.pos],eax
	stdcall [edit_box_draw], edit1
	stdcall [ted_open_file], tedit0,run_file_70,[edit1.text]
	call ted_messages_after_open_file
	.ret_f:
	popad
	ret

;description:
; копирование памяти
align 4
proc mem_copy, source:dword, destination:dword, len:dword
  push ecx esi edi
    cld
    mov esi, dword[source]
    mov edi, dword[destination]
    mov ecx, dword[len]
    rep movsb
  pop edi esi ecx
  ret
endp

;description:
; заполнение памяти пробелами
align 4
proc mem_spac, mem:dword, len:dword
  push eax ecx edi
    cld
    mov al,' '
    mov edi, dword[mem]
    mov ecx, dword[len]
    repne stosb
  pop edi ecx eax
  ret
endp

;input:
; eax = код ошибки
; ebx = колличество прочитанных байт
; edi = pointer to tedit struct
align 4
ted_messages_after_open_file:
	push ecx
	cmp eax,0
	je @f
	cmp eax,6
	je @f
		cmp ax,10
		jl .zifra_0_9
			mov al,'?'
			sub ax,48
		.zifra_0_9:
		add ax,48

		mov byte[msgbox_4.err],al
		stdcall [mb_create],msgbox_4,thread ;message: Can-t open text file!
		jmp .ret_f
	@@:
	cmp ebx,-1
	je .ret_f
		;if open file
		mov ecx,ted_max_chars
		sub ecx,2 ;ecx = максимальное число байт, для которых была выделена память
		cmp ebx,ecx
		jl .ret_f
			stdcall [mb_create],msgbox_1,thread
	.ret_f:
	pop ecx
	ret

align 4
ted_save_err_msg:
	mov byte[msgbox_6.err],al
	stdcall [mb_create],msgbox_6,thread ;message: Can-t save text file!
	ret

;description:
; функция нажатия на кнопку [Применить] в панели синтаксиса
align 4
proc ted_but_open_syntax, edit:dword
  push ebx edi
  mov edi,dword[edit]

  cmp ted_panel_id,TED_PANEL_SYNTAX
  jne @f
    stdcall dword[tl_node_get_data], tree1
    pop dword[fn_col_option]
	mov ebx,dword[fn_col_option]
	copy_path ebx,fn_syntax_dir,file_name_rez,0x0
	copy_path file_name_rez,sys_path,file_name,0x0
    stdcall [ted_init_syntax_file],edi,run_file_70,file_name
    call [ted_text_colored]
    stdcall [ted_draw],edi
  @@:
  pop edi ebx
  ret
endp

;description:
; вызов/скрытие панели поиска
align 4
proc ted_but_find
  push edi
  mov edi,tedit0

  cmp ted_panel_id,TED_PANEL_NULL
  je @f
    mov ted_panel_id,TED_PANEL_NULL
    mov ted_wnd_l,0
    jmp .e_if
  @@:
    mov ted_panel_id,TED_PANEL_FIND
    mov ted_wnd_l,TED_PANEL_WIDTH
  .e_if:
  stdcall EvSize,edi
  stdcall [ted_draw],edi
  pop edi
  ret
endp

align 4
ted_on_find_err:
	stdcall [mb_create],msgbox_7,thread ;message: Can not find text
	ret

;description:
; кнопка [Заменить] на панели
align 4
but_replace:
	; пока не сделано
	ret

;description:
; функция поиска информации в списке ключевых слов
align 4
but_find_key_w:
	; пока не сделано
	ret

;description:
; выделение/скрытие непечатаемых символов
align 4
proc but_sumb_invis, edit:dword
  push edi
  mov edi,dword[edit]

  xor ted_mode_invis,1
  call draw_but_toolbar
  stdcall [ted_draw],edi
  pop edi
  ret
endp

;description:
; выделение/скрытие ключевых слов
align 4
proc but_k_words_show, edit:dword
  push edi
  mov edi,dword[edit]

  xor ted_mode_color,1
  cmp ted_mode_color,0
  je @f
    call [ted_text_colored]
  @@:
  call draw_but_toolbar
  stdcall [ted_draw],edi
  pop edi
  ret
endp

;description:
; вызов/скрытие панели с файлами синтаксиса
align 4
proc but_synt_show, edit:dword
  push edi
  mov edi,[edit]

  cmp ted_panel_id,TED_PANEL_NULL
  je @f
    mov ted_panel_id,TED_PANEL_NULL
    mov ted_wnd_l,0
    jmp .e_if
  @@:
    mov ted_panel_id,TED_PANEL_SYNTAX
    mov ted_wnd_l,TED_PANEL_WIDTH
  .e_if:
  stdcall EvSize,edi
  stdcall [ted_draw],edi
  pop edi
  ret
endp
