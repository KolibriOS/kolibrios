

align 16
draw_window:
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mov edx,[sc.work]
	or  edx,0x73000000
	mcall SF_CREATE_WINDOW,dword[wnd_s_pos],dword[wnd_s_pos+4],,,hed

	mcall SF_THREAD_INFO,procinfo,-1
	mov edi,tedit0 ;значение edi нужно дл€ EvSize и ted_wnd_t
	call EvSize

	movzx ebx,word[procinfo.client_box.width]
	inc bx
	mcall SF_DRAW_RECT,,ted_wnd_t ;верхний пр€моугольник, дл€ очистки верхней панели
	call draw_but_toolbar
	stdcall [kmainmenu_draw], [main_menu]
	stdcall [ted_draw], tedit0

	mcall SF_REDRAW,SSF_END_DRAW
	ret

align 4
draw_but_toolbar: ;функци€ дл€ рисовани€ панели инструментов
	pushad
	mov edi,tedit0

	mov ecx,0x40000000+ID_BUT_NEW
	mov edx,(5 shl 16)+25
	bt word[panel_but],0
	jnc @f
		call draw_but_icon ;New
	@@:

	mov cx,ID_BUT_OPEN
	bt word[panel_but+1],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Open
	@@:

	stdcall [ted_can_save],edi
	cmp al,1
	je @f
		and ecx,0xffff
	@@:
	mov cx,ID_BUT_SAVE
	bt word[panel_but+2],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Save
	@@:
	or ecx,0x40000000

	mov cx,ID_BUT_SAVE_AS
	bt word[panel_but+3],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Save as
	@@:

	mov cx,ID_BUT_SELECT
	bt word[panel_but+4],0
	jnc @f
		add edx,30 shl 16
		call draw_but_icon ;Select word
	@@:

	call [ted_is_select]
	or al,al
	jnz @f
		and ecx,0xffff
	@@:
	mov cx,ID_BUT_CUT
	bt word[panel_but+5],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Cut
	@@:

	mov cx,ID_BUT_COPY
	bt word[panel_but+6],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Copy
	@@:
	or ecx,0x40000000

	cmp dword[buf],0
	jne @f
		and ecx,0xffff
	@@:
	mov cx,ID_BUT_PASTE
	bt word[panel_but+7],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Paste
	@@:
	or ecx,0x40000000

	mov cx,ID_BUT_FIND
	bt word[panel_but+8],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Found
	@@:

	mov cx,ID_BUT_REPLACE
	bt word[panel_but+9],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Replace
	@@:

	mov cx,ID_BUT_KEY_WORDS
	bt word[panel_but+10],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Key words
	@@:

	call [ted_is_select]
	or al,al
	jnz @f
		and ecx,0xffff
	@@:
	mov cx,ID_BUT_UPPER
	bt word[panel_but+11],0
	jnc @f
		add edx,30 shl 16
		call draw_but_icon ;Upper
	@@:

	mov cx,ID_BUT_LOWER
	bt word[panel_but+12],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Lower
	@@:

	mov cx,ID_BUT_REVERSE
	bt word[panel_but+13],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Reverse
	@@:
	or ecx,0x40000000

	mov ebx,ted_tim_undo
	cmp ted_tim_ch,ebx
	jg @f
		and ecx,0xffff
	@@:
	mov cx,ID_BUT_UNDO
	bt word[panel_but+14],0
	jnc @f
		add edx,30 shl 16
		call draw_but_icon ;Undo
	@@:
	or ecx,0x40000000

	cmp ted_tim_undo,1
	jge @f
		and ecx,0xffff
	@@:
	mov cx,ID_BUT_REDO
	bt word[panel_but+15],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Redo
	@@:
	or ecx,0x40000000

	mov cx,ID_BUT_INVISIBLE
	bt word[panel_but+16],0
	jnc @f
		add edx,30 shl 16
		call draw_but_icon ;Invisible on|off
	@@:

	mov cx,ID_BUT_SYNTAX_LIST
	bt word[panel_but+17],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;¬ыбор файла подсветки
	@@:

	mov cx,ID_BUT_SYNTAX_MODE
	bt word[panel_but+18],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;ѕодсветка вкл|выкл
	@@:

	mov cx,ID_BUT_CONVERT_1251_866
	bt word[panel_but+19],0
	jnc @f
		add edx,30 shl 16
		call draw_but_icon ;cp 1251 -> 866
	@@:

	mov cx,ID_BUT_CONVERT_866_1251
	bt word[panel_but+20],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;cp 866 -> 1251
	@@:

	popad
	ret

;txtBUp db 24
;txtBDn db 25
;txtBRi db 26
;txtBLe db 27

align 4
proc draw_panels uses eax edx edi, edit:dword
	mov edi,[edit]
	cmp ted_panel_id,TED_PANEL_FIND
	jne @f
		mcall SF_DEFINE_BUTTON,,,0x80000000+200
		mcall ,,,0x80000000+202
		mcall ,,,0x80000000+203
		mcall ,,,0x80000000+204
		call p_find
		jmp .end_f
	@@:
	cmp ted_panel_id,TED_PANEL_REPLACE
	jne @f
		mcall SF_DEFINE_BUTTON,,,0x80000000+200
		mcall ,,,0x80000000+201
		mcall ,,,0x80000000+204
		call p_replace
		jmp .end_f
	@@:
	cmp ted_panel_id,TED_PANEL_GOTO
	jne @f
		mcall SF_DEFINE_BUTTON,,,0x80000000+200
		mcall ,,,0x80000000+201
		mcall ,,,0x80000000+202
		mcall ,,,0x80000000+203
		call p_goto
		jmp .end_f
	@@:
	cmp ted_panel_id,TED_PANEL_SYNTAX
	jne @f
		mcall SF_DEFINE_BUTTON,,,0x80000000+201
		mcall ,,,0x80000000+202
		mcall ,,,0x80000000+203
		mcall ,,,0x80000000+204
		call p_syntax
		;jmp .end_f
	@@:
	.end_f:
	ret
endp

align 4
p_find:
	push eax ebx ecx edx
		mov ecx,ted_wnd_t
		shl ecx,16
		mov cx,20
		mcall SF_DRAW_RECT,TED_PANEL_WIDTH,,[sc.work]

		mov ecx,ted_wnd_t
		add cx,20+15 ; 15 - height text box
		shl ecx,16
		add ecx,ted_wnd_h
		mov edx,ted_scr_h
		add cx,word[edx+sb_offs_size_y]
		sub cx,20+15-1 ; 15 - height text box
		mcall ,,,[sc.work]

		stdcall [edit_box_draw], edit_find

		mov ecx,ted_wnd_t
		shl ecx,16
		add ecx,(111 shl 16)+20
		mcall SF_DEFINE_BUTTON,(5 shl 16)+85,,201,[sc.work_button] ;201 - button id

		mov ebx,(23 shl 16)+7
		add ebx,ted_wnd_t
		mov ecx,[sc.work_text]
		or ecx,0x80000000
		mcall SF_DRAW_TEXT,,,txtFindCapt

		mov ecx,[sc.work_button_text]
		or ecx,0x80000000
		mov ebx,(15 shl 16)+118
		add ebx,ted_wnd_t
		mcall ,,,txtFindNext

		stdcall [option_box_draw], opt_grlist1
		stdcall [check_box_draw], ch1
	pop edx ecx ebx eax
	ret

align 4
p_replace:
	push eax ebx ecx edx
		mov ecx,ted_wnd_t
		shl ecx,16
		mov cx,20
		mcall SF_DRAW_RECT,TED_PANEL_WIDTH,,[sc.work]

		mov ecx,ted_wnd_t
		add cx,20+15 ; 15 - height text box
		shl ecx,16
		add ecx,ted_wnd_h
		mov edx,ted_scr_h
		add cx,word[edx+sb_offs_size_y]
		sub cx,20+15-1 ; 15 - height text box
		mcall ,,,[sc.work]

		stdcall [edit_box_draw], edit_replace
		stdcall [edit_box_draw], edit_find

		mov ecx,ted_wnd_t
		shl ecx,16
		add ecx,(133 shl 16)+20
		mcall SF_DEFINE_BUTTON,(5 shl 16)+95,,202,[sc.work_button] ;replace next

		add ecx,(26 shl 16)
		mcall ,,,203 ;replace all

		mov ebx,ted_wnd_t
		add ebx,(23 shl 16)+7
		mov ecx,[sc.work_text]
		or ecx,0x80000000
		mcall SF_DRAW_TEXT,,,txtFindCapt
		add ebx,93
		mcall SF_DRAW_TEXT,,,txtRerlaceCapt

		mov ecx,[sc.work_button_text]
		or ecx,0x80000000
		mov ebx,ted_wnd_t
		add ebx,(15 shl 16)+140
		mcall ,,,txtRerlace
		add ebx,26
		mcall ,,,txtRerlaceAll

		stdcall [option_box_draw], opt_grlist1
	pop edx ecx ebx eax
	ret

align 4
p_goto:
	push eax ebx ecx edx
		mov ecx,ted_wnd_t
		shl ecx,16
		mov cx,20
		mcall SF_DRAW_RECT,TED_PANEL_WIDTH,,[sc.work]

		mov ecx,ted_wnd_t
		add cx,20+15 ; 15 - height text box
		shl ecx,16
		add ecx,ted_wnd_h
		mov edx,ted_scr_h
		add cx,word[edx+sb_offs_size_y]
		sub cx,20+15-1 ; 15 - height text box
		mcall ,,,[sc.work]

		stdcall [edit_box_draw], edit_goto

		mov ecx,ted_wnd_t
		shl ecx,16
		add ecx,(40 shl 16)+20
		mcall SF_DEFINE_BUTTON,(5 shl 16)+85,,204,[sc.work_button]

		mov ebx,(23 shl 16)+7
		add ebx,ted_wnd_t
		mov ecx,[sc.work_text]
		or ecx,0x80000000
		mcall SF_DRAW_TEXT,,,txtGotoCapt

		mov ecx,[sc.work_button_text]
		or ecx,0x80000000
		mov ebx,(15 shl 16)+47
		add ebx,ted_wnd_t
		mcall ,,,txtGoto
	pop edx ecx ebx eax
	ret

;input:
; edi = pointer to tedit struct
align 4
p_syntax:
	pushad

	mov ecx,ted_wnd_t
	shl ecx,16
	mov cx,20
	mcall SF_DRAW_RECT,TED_PANEL_WIDTH,,[sc.work] ;рисование верхнего фонового пр€моугольника

	stdcall [tl_draw], tree1
	mov [ws_dir_lbox.all_redraw],1 ;дл€ полной перерисовки дочернего скроллинга
	stdcall [scrollbar_ver_draw], ws_dir_lbox

	ror ecx,16
	add ecx,[tree1.box_height]
	add ecx,20
	and ecx,0xffff
	ror ecx,16
	add ecx,ted_wnd_h
	mov esi,ted_scr_h
	add cx,word[esi+sb_offs_size_y]
	sub cx,20
	sub ecx,[tree1.box_height]
	inc cx
	int 0x40 ;рисование нижнего фонового пр€моугольника

	mov ecx,ted_wnd_t
	add ecx,25
	add ecx,[tree1.box_height]
	shl ecx,16
	mov cx,20
	mcall SF_DEFINE_BUTTON,(5 shl 16)+65,,200,[sc.work_button] ;200 - button id

	mov ebx,(30 shl 16)+5
	add ebx,ted_wnd_t
	mov ecx,[sc.work_text]
	or ecx,0x80000000
	mcall SF_DRAW_TEXT,,,txtFormatCapt

	mov ebx,(10 shl 16)+31
	add ebx,[tree1.box_height]
	add ebx,ted_wnd_t
	mov ecx,[sc.work_button_text]
	or ecx,0x80000000
	mcall ,,,txtFormatApply

	popad
	ret

MIN_M_WND_H equ 100 ;минимальна€ высота главного окна
;input:
; edi = pointer to tedit struct
align 4
EvSize:
	pushad
	mov ebx,ted_scr_h
	mov esi,ted_scr_w

	m2m ted_wnd_w,[procinfo.client_box.width] ;ставим ширину окна редактора равной ширине всего окна
	mov eax,ted_wnd_l
	sub ted_wnd_w,eax ;отнимаем отступ слева
	movzx eax,word[esi+sb_offs_size_x]
	sub ted_wnd_w,eax ;отнимаем ширину верт. скроллинга

	m2m ted_wnd_h,[procinfo.client_box.height] ;ставим высоту окна редактора равной высоте всего окна
	cmp ted_wnd_h,MIN_M_WND_H
	jg @f
		mov ted_wnd_h,MIN_M_WND_H
	@@:

	movzx eax,word[ebx+sb_offs_size_y]
	sub ted_wnd_h,eax	      ;отнимаем высоту гориз. скроллинга
	mov eax,ted_wnd_t
	sub ted_wnd_h,eax	      ;отнимаем отступ сверху

	stdcall [ted_init_scroll_bars], tedit0,2

	mov eax,ted_wnd_t
	mov edi,tree1
	mov tl_box_top,eax ;=ted_wnd_t
	add tl_box_top,20

	mov [edit_find.top],eax ;=ted_wnd_t
	add dword[edit_find.top],20
	mov [edit_replace.top],eax
	add dword[edit_replace.top],113
	mov [edit_goto.top],eax
	add dword[edit_goto.top],20
	popad
	ret

;input:
;  ecx = 0x4000____
;   cx = ID button
;  edx = x*2^16+y
align 4
draw_but_icon:
	push eax ebx

	mov eax,SF_DEFINE_BUTTON
	push ecx edx esi
		mov ebx,edx
		mov edx,ecx
		mov cx,bx
		shl ecx,16
		mov cx,19 ;=20-1
		mov bx,19 ;=20-1
		mov esi,ebx
		shr esi,16
		add esi,20
		cmp esi,[procinfo.client_box.width]
		jge @f ;кнопка не влезла в окно
			mov esi,[sc.work_button]
			int 0x40 ;ставим кнопку
			mov eax,SF_PUT_IMAGE
		@@:
	pop esi edx ecx

	cmp eax,SF_PUT_IMAGE
	jne @f ;кнопка не влезла в окно
		mov ebx,[bmp_icon]
		bt ecx,30 ;if (ecx & 0x40000000)
		jc .gray
			add ebx,TOOLBAR_ICONS_SIZE ;перемещаемс€ на серые кнопки
		.gray:
		push ecx
		sub ecx,ID_BUT_0
		and ecx,0xffff
		imul ecx,1200
		add ebx,ecx
		mov ecx,(20 shl 16)+20
		int 0x40 ;ставим рисунок на кнопку
		pop ecx
	@@:
	pop ebx eax
	ret

;descrition:
; функци€ дл€ генерировани€ серых иконок
;input:
; buf_rgb - буфер с входным 24 битным цветным изображением
; buf_g24 - буфер с выходным 24 битным серым изображением
; pixels - число пикселей в изображении
align 4
proc img_to_gray, buf_rgb:dword, buf_g24:dword, pixels:dword
pushad
	mov esi,[buf_rgb]
	mov edi,[buf_g24]
	mov ecx,[pixels]
	mov ebx,3
	@@:
		movzx eax,byte[esi]
		movzx edx,byte[esi+1]
		add eax,edx
		movzx edx,byte[esi+2]
		add eax,edx
		xor edx,edx
		div ebx ;shr eax,2
		mov ah,al
		mov word[edi],ax
		mov byte[edi+2],al
		add esi,3
		add edi,3
		loop @b
popad
	ret
endp

