;
; в этом файле собраны функции нужные для создания и
; работы окна с поиском в ключевых словах
;

prop_wnd_width equ 350 ;ширина окна с поиском
SIZE_ONE_FLOAT equ 14

IMAGE_TOOLBAR_ICON_SIZE equ 20*20*3

wnd_k_words_run dd 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна одновременно

if lang eq ru
txt_caption db 'Поиск в ключевых словах',0
else
txt_caption db 'Search in keywords',0
end if

;даное окно (процесс) вызываеться функцией but_wnd_coords
align 4
prop_start:
pushad
	mcall SF_SET_EVENTS_MASK,0xC0000027 ;маска ожидаемых событий

	;настройка списка объектов
	mov dword[tree3.info_max_count],2
	mov edi,tedit0
	mov edx,ted_key_words_count
	add dword[tree3.info_max_count],edx ;ставим число узлов, равное колличеству ключевых слов в файле подсветки (+2 на начало и конец списка)
	stdcall [tl_data_init], tree3
	mov eax,dword[icon_tl_sys]
	mov dword[tree3.data_img],eax
	mov eax,dword[tree1.data_img_sys]
	mov dword[tree3.data_img_sys],eax

	edit_boxes_set_sys_color edit3,editboxes_end,sc ;установка системных цветов
popad

align 4
prop_red_win:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mov bx,word[procinfo.box.left]
	add bx,5 ;word[buf_0.l]
	shl ebx,16
	mov bx,prop_wnd_width
	mov cx,word[procinfo.box.top]
	add cx,42 ;word[buf_0.t]
	shl ecx,16
	mov cx,250
	mcall SF_CREATE_WINDOW,,,0x33ffffd0,,txt_caption

	mcall SF_DEFINE_BUTTON,(5 shl 16)+19,(5 shl 16)+19,3,[sc.work_button]
	add ebx,(25 shl 16)
	mcall ,,,4

	mov ebx,[bmp_icon]
	add ebx,(ID_BUT_FIND-ID_BUT_0)*IMAGE_TOOLBAR_ICON_SIZE
	mcall SF_PUT_IMAGE,,(20 shl 16)+20,(5 shl 16)+5 ;найти

	sub ebx,2*IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(30 shl 16)+5 ;копировать
	int 0x40

	mcall SF_THREAD_INFO,procinfo,-1
	mov eax,[procinfo.box.height]
	cmp eax,90
	jge @f
		mov eax,90 ;min size
	@@:
	sub eax,58
	cmp [tree3.box_height],eax
	je @f
		mov [tree3.box_height],eax
		mov word[w_scr_t3.y_size],ax
		;need call tb_scrol_resize
	@@:

	mov dword[w_scr_t3.all_redraw],1
	stdcall [scrollbar_ver_draw], w_scr_t3
	stdcall [tl_draw], tree3
	stdcall [edit_box_draw], edit3
	mcall SF_REDRAW,SSF_END_DRAW
popad

;не отрывать эту функцию от предыдущей
align 4
prop_still:
	mcall SF_WAIT_EVENT

	cmp al,1 ;изм. положение окна
	jz prop_red_win
	cmp al,2
	jne @f
		call prop_key
	@@:
	cmp al,3
	jne @f
		call prop_button
	@@:
	cmp al,6
	jne @f 
		call prop_mouse
	@@:
	jmp prop_still
.exit:
	mov dword[tree3.data_img],0 ;чистим указатели на иконки что-бы их не удалил деструктор
	mov dword[tree3.data_img_sys],0 ;т. к. они еще используються в других элементах
	stdcall [tl_data_clear], tree3
	mov dword[wnd_k_words_run],0 ;обнуляем ID окна
	mcall SF_TERMINATE_PROCESS

align 4
prop_key:
push eax
	mcall SF_GET_KEY
	stdcall [edit_box_key], edit3
	stdcall [tl_key], tree3
pop eax
	ret

align 4
prop_mouse:
	;push ecx edi
	stdcall [tl_mouse], tree3
	stdcall [edit_box_mouse], edit3
	;pop edi ecx
	ret

align 4
prop_button:
	pushad
	mcall SF_GET_BUTTON

	cmp ah,1
	je prop_still.exit

	cmp ah,3
	jne @f
		;найти слова
		mov edi,tedit0
		cmp ted_syntax_file,0
		je @f
		stdcall [tl_info_clear], tree3
		and dword[tree3.style], not tl_cursor_pos_limited

		mov ebx,ted_key_words_data
		mov ecx,ted_key_words_count
		cld
		.cycle_0:
		stdcall str_instr, ebx,[edit3.text]
		test al,al
		jnz .end_add
			;ключевое слово
			stdcall [tl_node_add], tree3, (1 shl 16), ebx
			stdcall [tl_cur_next], tree3
			mov edx,[ebx+MAX_COLOR_WORD_LEN]
			or edx,edx
			jz .end_add
				;справка по ключевому слову
				add edx,ted_help_text_f1
				stdcall [tl_node_add], tree3, 1, edx
				stdcall [tl_cur_next], tree3
		.end_add:
		add ebx,MAX_COLOR_WORD_LEN+8 ;sizeof.TexColViv
		loop .cycle_0

		stdcall [tl_cur_beg], tree3
		or dword[tree3.style], tl_cursor_pos_limited ;ограничиваем движение курсора в пределах списка
		stdcall [tl_draw], tree3
		jmp .no_select_0
	@@:
	cmp ah,4
	jne @f
		;копировать в буфер
		stdcall [tl_node_get_data],tree3
		or eax,eax
		jz .no_select_0
			mov esi,eax
			mov edi,[tedit0.buffer]
			mov dword[edi],MAX_COLOR_WORD_LEN
			mov dword[edi+4],0 ;text data
			mov dword[edi+8],1 ;code 866
			add edi,12 ;system buffer header size
			mov byte[edi+MAX_COLOR_WORD_LEN],0
			mov ecx,MAX_COLOR_WORD_LEN
			cld
			rep movsb
			mcall SF_CLIPBOARD,SSF_WRITE_CB,12+MAX_COLOR_WORD_LEN,[tedit0.buffer]
		.no_select_0:
	@@:

	popad
	ret

;дерево с объектами в пользовательском файле
align 4
tree3 tree_list MAX_COLOR_WORD_LEN,3,tl_key_no_edit,\
	16,16, 0x8080ff,0x0000ff,0xffffff, 5,30,300,160, 16, 0,0, el_focus, w_scr_t3,0

align 4
edit3 edit_box 80, 70, 8, 0xffffff, 0xff, 0x80ff, 0, 0x8000, MAX_COLOR_WORD_LEN+2, string1, mouse_dd, ed_focus
editboxes_end:

string1 rb MAX_COLOR_WORD_LEN

align 4
w_scr_t3 scrollbar 16,0, 3,0, 15, 100, 0,0, 0xeeeeee, 0xbbddff, 0, 1