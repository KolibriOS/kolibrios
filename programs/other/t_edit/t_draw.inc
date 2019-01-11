

align 16
draw_window:
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mov edx,[sc.work]
	or  edx,0x73000000
	mcall SF_CREATE_WINDOW,dword[wnd_s_pos],dword[wnd_s_pos+4],,,hed

	mcall SF_THREAD_INFO,procinfo,-1
	mov edi,tedit0 ;значение edi нужно для EvSize и ted_wnd_t
	call EvSize

	movzx ebx,word[procinfo.client_box.width]
	inc bx
	mcall SF_DRAW_RECT,,ted_wnd_t ;верхний прямоугольник, для очистки верхней панели
	call draw_but_toolbar
	stdcall [kmainmenu_draw], [main_menu]
	stdcall [ted_draw], tedit0

	mcall SF_REDRAW,SSF_END_DRAW
	ret

align 4
draw_but_toolbar: ;функция для рисования панели инструментов
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
		call draw_but_icon ;Выбор файла подсветки
	@@:

	mov cx,ID_BUT_SYNTAX_MODE
	bt word[panel_but+18],0
	jnc @f
		add edx,25 shl 16
		call draw_but_icon ;Подсветка вкл|выкл
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

;input:
; edi = pointer to tedit struct
align 4
proc draw_panel_find
	cmp ted_panel_id,TED_PANEL_FIND ;if not panel
	jne @f
	push eax ebx ecx edx
		mov ecx,ted_wnd_t
		shl ecx,16
		mov cx,20
		mcall SF_DRAW_RECT,TED_PANEL_WIDTH,,[sc.work]

		mov ebx,(30 shl 16)+25
		add ebx,ted_wnd_t
		mov ecx,[sc.work_text]
		or ecx,0x80000000
		mcall SF_DRAW_TEXT,,,txtFindCapt

		stdcall [edit_box_draw], edit2

		mov ecx,ted_wnd_t
		add cx,20+15 ; 15 - height text box
		shl ecx,16
		add ecx,ted_wnd_h
		mov edx,ted_scr_h
		add cx,word[edx+sb_offs_size_y]
		sub cx,20+15-1 ; 15 - height text box
		mcall SF_DRAW_RECT,TED_PANEL_WIDTH,,[sc.work]

		mov ecx,ted_wnd_t
		add cx,20+15+5
		shl ecx,16
		mov cx,20
		mcall SF_DEFINE_BUTTON,(5 shl 16)+85,,201,[sc.work_button] ;201 - button id

		mov ebx,(15 shl 16)+(20+15+10)
		add ebx,ted_wnd_t
		mov ecx,[sc.work_text]
		or ecx,0x80000000
		mcall SF_DRAW_TEXT,,,txtFindNext
	pop edx ecx ebx eax
	jmp .end_f
	@@:
	push eax edx
		mov edx,201
		or edx,0x80000000
		mcall SF_DEFINE_BUTTON
	pop edx eax
	.end_f:
	ret
endp

;input:
; edi = pointer to tedit struct
align 4
proc draw_panel_syntax
	cmp ted_panel_id,TED_PANEL_SYNTAX ;if not panel
	jne @f
	pushad

	mov ecx,ted_wnd_t
	shl ecx,16
	mov cx,20
	mcall SF_DRAW_RECT,TED_PANEL_WIDTH,,[sc.work] ;рисование верхнего фонового прямоугольника

	stdcall dword[tl_draw], tree1
	mov [ws_dir_lbox.all_redraw],1 ;для полной перерисовки дочернего скроллинга
	stdcall dword[scrollbar_ver_draw], dword ws_dir_lbox

	ror ecx,16
	add ecx,dword[tree1.box_height]
	add ecx,20
	and ecx,0xffff
	ror ecx,16
	add ecx,ted_wnd_h
	mov esi,ted_scr_h
	add cx,word[esi+sb_offs_size_y]
	sub cx,20
	sub ecx,dword[tree1.box_height]
	inc cx
	int 0x40 ;рисование нижнего фонового прямоугольника

	mov ecx,ted_wnd_t
	add ecx,25
	add ecx,dword[tree1.box_height]
	shl ecx,16
	mov cx,20
	mcall SF_DEFINE_BUTTON,(5 shl 16)+65,,200,[sc.work_button] ;200 - button id

	mov ebx,(30 shl 16)+5
	add ebx,ted_wnd_t
	mov ecx,[sc.work_text]
	or ecx,0x80000000
	mcall SF_DRAW_TEXT,,,txtFormatCapt

	mov ebx,(10 shl 16)+31
	add ebx,dword[tree1.box_height]
	add ebx,ted_wnd_t
	mcall ,,,txtFormatApply

	popad
	jmp .end_f
	@@:
		push eax edx
		mov edx,200
		or edx,0x80000000
		mcall SF_DEFINE_BUTTON ;если нет панели то удаляем кнопку
		pop edx eax
	.end_f:
	ret
endp

MIN_M_WND_H equ 100 ;минимальная высота главного окна
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
	mov edi,dword tree1
	mov tl_box_top,eax ;=ted_wnd_t
	add tl_box_top,20

	mov [edit2.top],eax ;=ted_wnd_t
	add dword[edit2.top],20

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
			add ebx,TOOLBAR_ICONS_SIZE ;перемещаемся на серые кнопки
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
; функция для генерирования серых иконок
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

