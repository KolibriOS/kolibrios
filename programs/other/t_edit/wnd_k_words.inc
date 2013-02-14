;
; в этом файле собраны функции нужные для создания и
; работы окна с поиском в ключевых словах
;

prop_wnd_width equ 350 ;ширина окна с поиском
SIZE_ONE_FLOAT equ 14

IMAGE_TOOLBAR_ICON_SIZE equ 20*20*3

wnd_k_words_run db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна одновременно

if lang eq ru
txt_caption db 'Поиск в ключевых словах',0
else
txt_caption db 'Search in keywords',0
end if

;даное окно (процесс) вызываеться функцией but_wnd_coords
align 4
prop_start:
pushad
	mcall 40,0xC0000027 ;маска ожидаемых событий
	inc byte[wnd_k_words_run]

	;настройка списка объектов
	mov dword[tree3.info_max_count],2
	mov edi,tedit0
	mov edx,ted_key_words_count
	add dword[tree3.info_max_count],edx ;ставим число узлов, равное колличеству ключевых слов в файле подсветки (+2 на начало и конец списка)
	stdcall dword[tl_data_init], tree3
	mov eax,dword[icon_tl_sys]
	mov dword[tree3.data_img],eax
	mov eax,dword[tree1.data_img_sys]
	mov dword[tree3.data_img_sys],eax

	edit_boxes_set_sys_color edit3,editboxes_end,sc ;установка системных цветов
	;stdcall [edit_box_set_text], edit3, txt_q
popad

align 4
prop_red_win:
pushad
	mcall 12,1

	xor eax,eax
	mov edi,txt_caption
	mov bx,word[procinfo.box.left]
	add bx,5 ;word[buf_0.l]
	shl ebx,16
	mov bx,prop_wnd_width
	mov cx,word[procinfo.box.top]
	add cx,5 ;word[buf_0.t]
	shl ecx,16
	mov cx,250
	mov edx,0xffffd0
	or  edx,0x33000000
	int 0x40

	mov eax,8
	mov ebx,(5 shl 16)+19
	mov ecx,(5 shl 16)+19
	mov edx,3
	mov esi,[sc.work_button]
	int 0x40

	add ebx,(25 shl 16)
	mov edx,4
	int 0x40

	mov eax,7
	mov ebx,[bmp_icon]
	add ebx,7*IMAGE_TOOLBAR_ICON_SIZE
	mov ecx,(20 shl 16)+20
	mov edx,(5 shl 16)+5 ;найти
	int 0x40

	sub ebx,2*IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(30 shl 16)+5 ;копировать
	int 0x40

	mov dword[w_scr_t3.all_redraw],1
	stdcall [scrollbar_ver_draw],dword w_scr_t3
	stdcall [tl_draw], tree3
	stdcall [edit_box_draw], edit3
	mcall 12,2
popad

;не отрывать эту функцию от предыдущей
align 4
prop_still:
	mcall 10

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
	stdcall dword[tl_data_clear], tree3
	mov byte[wnd_k_words_run],0 ;обнуляем счетчик окон
	mcall -1 ;закрытие окна поиска

align 4
prop_key:
push eax
	mcall 2
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
	mcall 17 ;получить код нажатой кнопки

	cmp ah,1
	je prop_still.exit

	cmp ah,3
	jne @f
		;найти слова
		mov edi,tedit0
		cmp ted_syntax_file,0
		je @f
		stdcall dword[tl_info_clear], tree3

		mov ebx,ted_key_words_data
		mov ecx,ted_key_words_count
		cld
		.cycle_0:
		stdcall str_instr, ebx,[edit3.text]
		test al,al
		jnz .end_add
			stdcall dword[tl_node_add], ebx, (0 shl 16), tree3
			stdcall dword[tl_cur_next], tree3
		.end_add:
		add ebx,MAX_COLOR_WORD_LEN+8 ;sizeof.TexColViv
		loop .cycle_0

		stdcall dword[tl_cur_beg], tree3
		stdcall dword[tl_draw], tree3
	@@:
	cmp ah,4
	jne @f
		;копировать в буфер
		stdcall [tl_node_get_data],tree3
		pop esi
		cmp esi,0
		je .no_select_0
			mov edi,[tedit0.buffer]
			mov byte[edi+MAX_COLOR_WORD_LEN],0
			mov ecx,MAX_COLOR_WORD_LEN
			cld
			rep movsb
		.no_select_0:
	@@:

	popad
	ret

;дерево с объектами в пользовательском файле
align 4
tree3 tree_list MAX_COLOR_WORD_LEN,3,\
	tl_key_no_edit+tl_list_box_mode,\
	16,16, 0x8080ff,0x0000ff,0xffffff, 5,30,300,160, 16, 0,0, el_focus,\
	w_scr_t3,0

align 4
edit3 edit_box 80, 70, 8, 0xffffff, 0xff, 0x80ff, 0, 0x8000, MAX_COLOR_WORD_LEN+2, string1, mouse_dd, 0
editboxes_end:

string1 rb MAX_COLOR_WORD_LEN

align 4
w_scr_t3:
.size_x     dw 16 ;+0
rb 2+2+2
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100  ;+16
rb 4+4
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
rb 4+2+2
.run_x:
rb 2+2+2+2+4+4+4+4+4+4
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84

