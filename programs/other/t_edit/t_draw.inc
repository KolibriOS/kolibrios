align 4
draw_but_toolbar: ;функция для рисования панели инструментов
  pushad
  mov edi,tedit0

  mov ecx,0x40000000
;  mov edx,5*65536+25
  mov edx,85*65536+2
  call draw_but_icon

  inc cx
;  mov edx,30*65536+25
  mov edx,110*65536+2
  call draw_but_icon

stdcall [ted_can_save],edi
cmp al,1
je @f
and ecx,0xffff
@@:
  inc cx
;  mov edx,55*65536+25
  mov edx,135*65536+2
  call draw_but_icon
or ecx,0x40000000

  inc cx
  mov edx,85*65536+25
  call draw_but_icon

call [ted_is_select]
cmp al,0
jne @f
and ecx,0xffff
@@:
  inc cx ; Cut
  mov edx,110*65536+25
  call draw_but_icon

  inc cx ; Copy
  mov edx,135*65536+25
  call draw_but_icon

  mov cx,10 ; Upper
  mov edx,265*65536+25
  call draw_but_icon

  inc cx ; Lower
  mov edx,290*65536+25
  call draw_but_icon

  inc cx ; reverse
  mov edx,315*65536+25
  call draw_but_icon
or ecx,0x40000000

cmp byte[buf],0
jne @f
and ecx,0xffff
@@:
  mov cx,6 ; Paste
  mov edx,160*65536+25
  call draw_but_icon
or ecx,0x40000000

  inc cx
  mov edx,185*65536+25
  call draw_but_icon

  inc cx
  mov edx,210*65536+25
  call draw_but_icon

  inc cx
  mov edx,235*65536+25
  call draw_but_icon

mov ebx,ted_tim_undo
cmp ted_tim_ch,ebx
jg @f
and ecx,0xffff
@@:
  mov cx,13
  mov edx,345*65536+25
  call draw_but_icon
or ecx,0x40000000

cmp ted_tim_undo,1
jge @f
and ecx,0xffff
@@:
  inc cx
  mov edx,370*65536+25
  call draw_but_icon
or ecx,0x40000000

  inc cx
  mov edx,400*65536+25
  call draw_but_icon

  inc cx
  mov edx,425*65536+25
  call draw_but_icon

  mov cx,17
  mov edx,450*65536+25
  call draw_but_icon

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
;push edi
;mov edi,dword[edit]
  cmp ted_panel_id,TED_PANEL_FIND ;if not panel
  jne @f
  push eax ebx ecx edx

  mov eax,13 ;рисование прямоугольника
  mov ebx,TED_PANEL_WIDTH
  mov ecx,ted_wnd_t
  shl ecx,16
  mov cx,20
  mov edx,[sc.work]
  int 0x40

  mov eax,4 ;рисование текста
  mov ebx,30*65536+5
  add ebx,ted_wnd_t
  mov ecx,[sc.work_text]
  or ecx,0x80000000
  mov edx,txtFindCapt
  int 0x40

  stdcall [edit_box_draw], dword edit2

  mov eax,13 ;рисование прямоугольника
  mov ebx,TED_PANEL_WIDTH
  mov ecx,ted_wnd_t
  add cx,20+15 ; 15 - height text box
  shl ecx,16
  add ecx,ted_wnd_h
  mov edx,ted_scr_h
  add cx,word[edx+sb_offs_size_y]
  sub cx,20+15-1 ; 15 - height text box
  mov edx,[sc.work]
  int 0x40

  mov eax,8 ;кнопка
  mov ebx,5*65536+85
  mov ecx,ted_wnd_t
  add cx,20+15+5
  shl ecx,16
  mov cx,20
  mov edx,201 ;button id
  mov esi,[sc.work_button]
  int 0x40

  mov eax,4 ;рисование текста
  mov ebx,15*65536+(20+15+10)
  add ebx,ted_wnd_t
  mov ecx,[sc.work_text]
  or ecx,0x80000000
  mov edx,txtFindNext
  int 0x40

  pop edx ecx ebx eax
  jmp .end_f
  @@:
  push eax edx
    mov eax,8
    mov edx,201
    or edx,0x80000000
    int 0x40
  pop edx eax
  .end_f:
;pop edi
  ret
endp

;input:
; edi = pointer to tedit struct
align 4
proc draw_panel_syntax
;push edi
;mov edi,dword[edit]
  cmp ted_panel_id,TED_PANEL_SYNTAX ;if not panel
  jne @f
  pushad

  mov eax,13 ;рисование прямоугольника
  mov ebx,TED_PANEL_WIDTH
  mov ecx,ted_wnd_t
  shl ecx,16
  mov cx,20
  mov edx,[sc.work]
  int 0x40 ;рисование верхнего фонового прямоугольника

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

  mov eax,8 ;кнопка
  mov ebx,5*65536+65
  mov ecx,ted_wnd_t
  add ecx,25
  add ecx,dword[tree1.box_height]
  shl ecx,16
  mov cx,20
  mov edx,200 ;button id
  mov esi,[sc.work_button]
  int 0x40

  mov eax,4 ;рисование текста
  mov ebx,30*65536+5
  add ebx,ted_wnd_t
  mov ecx,[sc.work_text]
  or ecx,0x80000000
  mov edx,txtFormatCapt
  int 0x40

  mov ebx,10*65536+30
  add ebx,dword[tree1.box_height]
  add ebx,ted_wnd_t
  mov edx,txtFormatApply
  int 0x40

  popad
  jmp .end_f
  @@:
    push eax edx
      mov eax,8
      mov edx,200
      or edx,0x80000000
      int 0x40 ;если нет панели то удаляем кнопку
    pop edx eax
  .end_f:
;pop edi
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
  mov eax,dword[esi+sb_offs_size_x]
  and eax,0xffff
  sub ted_wnd_w,eax ;отнимаем ширину верт. скроллинга

  m2m ted_wnd_h,[procinfo.client_box.height] ;ставим высоту окна редактора равной высоте всего окна
  cmp ted_wnd_h,MIN_M_WND_H
  jg @f
    mov ted_wnd_h,MIN_M_WND_H
  @@:

  mov ax,word[ebx+sb_offs_size_y]
  and eax,0xffff
  sub ted_wnd_h,eax	      ;отнимаем высоту гориз. скроллинга
  mov eax,ted_wnd_t
  sub ted_wnd_h,eax	      ;отнимаем отступ сверху

  stdcall [ted_init_scroll_bars], tedit0,2

  mov eax,ted_wnd_t
  mov edi,dword tree1
  mov tl_box_top,eax ;=ted_wnd_t
  add tl_box_top,20

  mov dword[edit2.top],eax ;=ted_wnd_t
  add dword[edit2.top],20

  popad
  ret

;input:
;  ecx = 0x4000____
;   cx = icon index
;  edx = x*2^16+y
align 4
draw_but_icon:
	push eax ebx

	mov eax,8 ;кнопка
	push ecx edx esi
		mov ebx,edx
		mov edx,ecx
		add edx,3
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
			mov eax,7 ;bmp
		@@:
	pop esi edx ecx

	cmp eax,7
	jne @f ;кнопка не влезла в окно
		mov ebx,[bmp_icon]
		bt ecx,30 ;if (ecx or 0x40000000)
		jc .gray
			add ebx,TOOLBAR_ICONS_SIZE ;перемещаемся на серые кнопки
		.gray:
		push ecx
		and ecx,0xffff
		imul ecx,1200
		add ebx,ecx
		mov ecx,20
		shl ecx,16
		add ecx,20
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
proc img_to_gray uses eax ebx ecx edi esi, buf_rgb:dword, buf_g24:dword, pixels:dword
	mov esi,[buf_rgb]
	mov edi,[buf_g24]
	mov ecx,[pixels]
	@@:
		movzx eax,byte[esi]
		movzx ebx,byte[esi+1]
		add eax,ebx
		movzx ebx,byte[esi+2]
		add eax,ebx
		shr eax,2
		mov ah,al
		mov word[edi],ax
		mov byte[edi+2],al
		add esi,3
		add edi,3
		loop @b
	ret
endp

