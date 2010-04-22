align 4
proc mouse_wnd_main, edit:dword
  push eax ebx ecx edi
  mcall 37,2 ;get mouse buttons

  mov edi,dword[edit]

  cmp al,1
  jne @f
    mcall 37,1 ;get mouse coords
    mov ebx,ted_wnd_t
    add ebx,ted_rec_t
    cmp ax,bx
    jl @f ;y_mouse<y_wnd

    sub ebx,ted_rec_t
    add ebx,ted_wnd_h
    cmp bx,ax
    jl @f ;y_mouse>y_wnd

    mov ebx,ted_wnd_l
    add ebx,ted_rec_l
    mov ecx,eax
    shr ecx,16
    cmp cx,bx
    jl @f ;x_mouse<x_wnd

    sub ebx,ted_rec_l
    add ebx,ted_wnd_w
    cmp bx,cx
    jl @f ;x_mouse>x_wnd

    stdcall draw_cursor_sumb, tedit0
    call wnd_main_click
    jmp .no_in_wnd
  @@:
    call wnd_main_mouse_scroll
    cmp ted_drag_m,0
    je .no_in_wnd
      mov ted_drag_m,0
      call draw_but_toolbar
      stdcall draw_main_win, tedit0
  .no_in_wnd:
  pop edi ecx ebx eax
  ret
endp

;input:
; eax -> (x,y)
; edi -> указатель на структуру tedit
;description:
; функция вызывется при нажатии кнопкой мыши и попадении курсором в окно редактора
align 4
wnd_main_click:
  push ecx edx

  push eax
    shr eax,16
    and eax,0xffff
    sub eax,ted_wnd_l
    sub eax,ted_rec_l

    xor edx,edx
    mov ecx,ted_rec_w
    div cx
    ;inc eax
    cmp eax,[hScr.cur_area]
    jle @f
      mov eax,[hScr.cur_area]
    @@:
    ;dec eax
    mov ted_cur_x,eax
  pop eax

  push eax
    and eax,0xffff
    sub eax,ted_wnd_t
    sub eax,ted_rec_t

    xor edx,edx
    mov ecx,ted_rec_h
    div cx
    inc eax
    cmp eax,[wScr.cur_area]
    jle @f
      mov eax,[wScr.cur_area]
    @@:
    dec eax
    mov ted_cur_y,eax
  pop eax

  cmp ted_drag_m,0
  je @f
    stdcall SelMove,edi
    jmp .sel_move
  @@:
    mov ted_drag_m,1
    stdcall SelStart,edi
  .sel_move:
  pop edx ecx
  ret

align 4
wnd_main_mouse_scroll:
  push eax ebx
  mcall 37,7
  cmp ax,0
  je .no_scroll
    mov ebx,dword[wScr.position] ;copy old scroll position
    and eax,0xffff
    btr ax,15
    jae @f
      or eax,0xffff8000
    @@:
    add dword[wScr.position],eax

    mov eax,[wScr.position]
    cmp eax,[wScr.max_area]
    jb @f
      mov dword[wScr.position],ebx ;if scroll position out of range
      jmp .no_scroll
    @@:
    stdcall draw_main_win, tedit0
  .no_scroll:
  pop ebx eax
  ret

align 4
draw_but_toolbar: ;внешняя функция для рисования панели инструментов
  push ebx ecx edx edi
  mov edi,tedit0

  mov ecx,0x40000000
;  mov edx,5*65536+25
  mov edx,85*65536+2
  call draw_but_icon

  inc cx
;  mov edx,30*65536+25
  mov edx,110*65536+2
  call draw_but_icon

stdcall ted_can_save,edi
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

stdcall IsSel,edi
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

  inc cx ; Reverse
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

  pop edi edx ecx ebx
  ret

;input:
;  clear_o - если =1 очистить одну строку, =0 очистить все строки окна до низу
align 4
proc clear_line_before_draw, edit:dword, coords:dword, clear_o:dword, numb_lin:dword
  pushad
    mov edi,dword[edit]
    mov ebx,dword[coords]   ;ebx = x*2^16+y coords to left up point clear line
    mov esi,dword[numb_lin] ;esi - number text line

    sub ebx,0x10001 ;отнимаем отступы для выравнивания буквы по центру
    cmp dword[clear_o],0
    jne @f
      add ebx,ted_rec_h
      inc esi
      ror ebx,16
	  xor bx,bx
      add ebx,ted_wnd_l
      add ebx,ted_rec_l ;bx = ted_wnd_l+ted_rec_l
      ror ebx,16
    @@:

    mov eax,ted_wnd_h
    add eax,ted_wnd_t
    cmp ax,bx
    jl .no_clear
    sub ax,bx

    mov cx,bx
    shl ecx,16

    xor bx,bx
    add ebx,ted_wnd_w
    sub ebx,ted_rec_l
	xor cx,cx
    add ecx,ted_rec_h
    mov edx,ted_color_wnd_work

    cmp dword[clear_o],0
    je .pusto
    cmp ax,cx
    jge @f
      .pusto:
      mov cx,ax
    @@:

    stdcall IsSel,edi
    cmp al,0
    je @f
    cmp ted_seln_y0,esi
    jg @f
    cmp ted_seln_y1,esi
    jl @f
      mov edx,ted_color_select ;draw selected line
    @@:

    mov eax,13 ;rect
    int 0x40

    stdcall IsSel,edi
    cmp al,0
    je .no_clear

    mov al,13 ;rect
	xor cx,cx
    add ecx,ted_rec_h
    cmp ted_seln_y0,esi
    jne @f
      push bx
      mov edx,ted_seln_x0 ; верхняя полоса (затирает слева)
      cmp edx,[hScr.position]
      jle .in_wnd
	sub edx,[hScr.position]
	imul edx,ted_rec_w
	mov bx,dx
	jmp .no_wnd
      .in_wnd:
	mov bx,0
      .no_wnd:
      mov edx,ted_color_wnd_work
      int 0x40
      pop bx
    @@:
    cmp ted_seln_y1,esi
    jne @f
      mov edx,ted_seln_x1 ; нижняя полоса (затирает справа)
      cmp edx,[hScr.position]
      jle .in_wnd2
	sub edx,[hScr.position]
	imul edx,ted_rec_w
	sub bx,dx
	shl edx,16
	add ebx,edx
      .in_wnd2:

      mov edx,ted_color_wnd_work
      int 0x40
    @@:

  .no_clear:
  popad
  ret
endp

align 4
proc draw_main_win, edit:dword
  locals
    line_num dd ?
  endl
  pushad
  mov edi,dword[edit]

  mov eax,4 ;draw text
  mov ecx,ted_text_colors
  mov ecx,dword[ecx]

  mov ebx,ted_wnd_l
  add ebx,ted_rec_l
  shl ebx,16
  add ebx,ted_wnd_t
  add ebx,ted_rec_t
  add ebx,0x10001 ;добавляем отступы для выравнивания буквы по центру

  stdcall SelNormalize,edi ;need before draw select
  mov esi,[wScr.position]
  mov dword[line_num],esi

  stdcall clear_line_before_draw, edi,ebx,1,esi
  call GetFirstVisiblePos
  cmp edx,0
  je .no_draw_text
  mov esi,1 ;длинна выводимого текста по 1-му символу
  @@:
    call IteratNext
    cmp edx,ted_tex_1
    jle .no_draw_text

    ; *** цветовая разметка
    cmp ted_mode_color,0
    je .no_col_change
    cmp byte[edx+1],0
    je .no_col_change
      call GetSymbColor
    .no_col_change:

    cmp byte [edx],13
    jne .no_13
      cmp ted_mode_invis,1
      jne .no_invis
	push edx
	mov edx,edi
	add edx,ted_offs_symbol_new_line
	int 0x40
	pop edx
      .no_invis:
      add ebx,ted_rec_h
      ;optimized output \/
      mov eax,ted_wnd_h
      add eax,ted_wnd_t
      cmp bx,ax
      jg .no_draw_text
      mov eax,4
      ;optimized output /\        
	  and ebx,0xffff
      ror ebx,16
      add ebx,ted_wnd_l
      add ebx,ted_rec_l
      inc ebx
      ror ebx,16
      inc dword[line_num] ;increment line number
      stdcall clear_line_before_draw,edi,ebx,1,dword[line_num]
      call OptDrawLineLeft
      jmp @b
    .no_13:

    int 0x40
    ror ebx,16
    add ebx,ted_rec_w
    mov esi,ted_wnd_l
    add esi,ted_wnd_w
    cmp bx,si
    jl .no_opt
      call OptDrawLineRight
    .no_opt:
	mov si,1
    ror ebx,16
    jmp @b
  .no_draw_text:

  stdcall clear_line_before_draw,edi,ebx,0,dword[line_num]
  stdcall draw_line_numbers,edi
  stdcall draw_main_cursor,edi

;---------------------------------------------
; set all_redraw flag for draw all ScrollBar
; In some cases it is necessity to draw only the area
; of moving of a "runner", for acceleration of output - 
; in this case the flag needs to be reset to 0 (zero).
	xor	eax,eax
	inc	eax
	mov	[wScr.all_redraw],eax
	mov	[hScr.all_redraw],eax	

; draw for Vertical ScrollBar
	stdcall [scrollbar_ver_draw], dword wScr
	stdcall [scrollbar_hor_draw], dword hScr
; reset all_redraw flag 
	xor	eax,eax
	mov	[wScr.all_redraw],eax
	mov	[hScr.all_redraw],eax
;---------------------------------------------

  mov eax,13
  ;left-bottom square
  mov ebx,ted_wnd_l
  shl ebx,16
  add ebx,ted_rec_l
  mov ecx,ted_wnd_t
  add ecx,ted_wnd_h
  shl ecx,16
  mov cx,word[hScr.size_y]
  inc cx
  mov edx,[sc.work]
  int 0x40

  ;right-bottom square
  mov ebx,ted_wnd_l
  add ebx,ted_wnd_w
  shl ebx,16
  mov bx,word[wScr.size_x]
  inc bx
  int 0x40

  stdcall draw_panel_find, edi
  stdcall draw_panel_syntax, edi
  popad
  ret
endp

;txtBUp db 24
;txtBDn db 25
;txtBRi db 26
;txtBLe db 27
align 4
proc draw_panel_find, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_panel_id,TE_PANEL_FIND ;if not panel
  jne @f
  push eax ebx ecx edx

  mov eax,13 ;рисование прямоугольника
  mov ebx,TE_PANEL_WIDTH
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
  mov ebx,TE_PANEL_WIDTH
  mov ecx,ted_wnd_t
  add cx,20+15 ; 15 - height text box
  shl ecx,16
  add ecx,ted_wnd_h
  add cx,word[hScr.size_y]
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
  pop edi
  ret
endp


align 4
proc draw_panel_syntax, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_panel_id,TE_PANEL_SYNTAX ;if not panel
  jne @f
  push eax ebx ecx edx

  mov eax,13 ;рисование прямоугольника
  mov ebx,TE_PANEL_WIDTH
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
  add cx,word[hScr.size_y]
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

  pop edx ecx ebx eax
  jmp .end_f
  @@:
    push eax edx
      mov eax,8
      mov edx,200
      or edx,0x80000000
      int 0x40 ;если нет панели то удаляем кнопку
    pop edx eax
  .end_f:
  pop edi
  ret
endp


align 4
proc draw_cur_line, edit:dword
  pushad
    mov edi,dword[edit]

    mov ebx,ted_wnd_l
    add ebx,ted_rec_l
    shl ebx,16
    mov eax,ted_cur_y
    imul eax,ted_rec_h
	mov bx,ax
    add ebx,ted_wnd_t
    add ebx,ted_rec_t ;ebx - координаты для прямоугольника очистки линии
	add ebx,0x10001   ;добавляем отступы для выравнивания буквы по центру

    stdcall SelNormalize,edi ;need before draw select
	mov ecx,ted_cur_y
    add ecx,[wScr.position]
    stdcall clear_line_before_draw, tedit0, ebx,1,ecx

    mov esi,[hScr.position]
    call GetPosByParam

  cmp ted_gp_opt,2
  jne .no_draw_text
;  mov eax,4 ;draw text
  call GetSymbColor
  mov esi,1 ;draw 1 symbol
  @@:
    ;call IteratNext
    cmp edx,ted_tex_1
    jle .no_draw_text

    ; *** цветовая разметка
    cmp ted_mode_color,0
    je .no_col_change
    cmp byte[edx+1],0
    je .no_col_change
      call GetSymbColor
    .no_col_change:

    mov eax,4 ;draw text
    cmp byte [edx],13
    jne .no_13
      cmp ted_mode_invis,1
      jne .no_draw_text
 push edx
 mov edx,edi
 add edx,ted_offs_symbol_new_line
 int 0x40
 pop edx
      jmp .no_draw_text
    .no_13:

    int 0x40
    ror ebx,16
    add ebx,ted_rec_w
    mov eax,ted_wnd_w
    add eax,ted_wnd_l ;ax = отступ по оси x
    cmp bx,ax
    jge .no_draw_text ;Opt
    ror ebx,16
    call IteratNext
    jmp @b
  .no_draw_text:

  stdcall draw_main_cursor,edi
  popad
  ret
endp

MIN_M_WND_H equ 100
MIN_W_SCRL_ARE equ 3
MIN_H_SCRL_ARE equ 3
align 4
proc EvSize, edit:dword
  push eax ecx edx edi
  mov edi,dword[edit]

  m2m ted_wnd_w,[procinfo.client_box.width] ;ставим ширину окна редактора равной ширине всего окна
  mov eax,ted_wnd_l
  sub ted_wnd_w,eax ;отнимаем отступ слева
  mov eax,[wScr]    ;+0 size_x
  and eax,0xffff
  sub ted_wnd_w,eax ;отнимаем ширину верт. скроллинга

  m2m ted_wnd_h,[procinfo.client_box.height] ;ставим высоту окна редактора равной высоте всего окна
  cmp ted_wnd_h,MIN_M_WND_H
  jg @f
    mov ted_wnd_h,MIN_M_WND_H
  @@:

  mov ax,word[hScr.size_y]
  and eax,0xffff
  sub ted_wnd_h,eax	      ;отнимаем высоту гориз. скроллинга
    mov eax,ted_wnd_h	      ;eax = высота окна - высота гориз. скроллинга
    mov word[hScr.start_y],ax ;передвигаем гориз. скроллинг
  mov eax,ted_wnd_t
  sub ted_wnd_h,eax	      ;отнимаем отступ сверху

  mov eax,ted_wnd_w
  mov ecx,ted_wnd_l
  add eax,ecx
  mov word[wScr.start_x],ax   ;передвигаем верт. скроллинг
  mov eax,ted_wnd_h
  mov word[wScr.size_y],ax    ;задаем высоту верт. скроллинга

  add ecx,ted_rec_l	;ecx=ted_wnd_l+ted_rec_l
  mov [hScr.start_x],cx ;сдвигаем гориз. скроллинг по размеру левого отступа для цифр
  mov eax,ted_wnd_w
  sub eax,ted_rec_l
  mov word[hScr.size_x],ax ;задаем ширину гориз. скроллинга

  mov eax,ted_wnd_h ;calculate lines in page
  sub eax,ted_rec_t
  xor edx,edx
  mov ecx,ted_rec_h
  div ecx
  cmp eax,MIN_W_SCRL_ARE
  jg @f
    mov eax,MIN_W_SCRL_ARE
  @@:
  mov [wScr.cur_area],eax

  mov eax,ted_wnd_w ;calculate cols in page
  sub eax,ted_rec_l
  xor edx,edx
  mov ecx,ted_rec_w
  div ecx
  cmp eax,MIN_H_SCRL_ARE
  jg @f
    mov eax,MIN_H_SCRL_ARE
  @@:
  dec eax ; ???
  mov [hScr.cur_area],eax

  mov eax,ted_wnd_t
  mov edi,dword tree1
  mov tl_box_top,eax ;=ted_wnd_t
  add tl_box_top,20

  mov dword[edit2.top],eax ;=ted_wnd_t
  add dword[edit2.top],20

  pop edi edx ecx eax
  call OnInitialUpdate
  ret
endp

align 4
OnInitialUpdate:
  mov [wScr.redraw],1
  mov [hScr.redraw],1
  ret

;input:
; edi = pointer to tedit struct
;output:
; ecx = новый цвет символа
; edx = pointer to symbol
; edx = 0 if text not in screen
align 4
GetFirstVisiblePos:
  push ecx
  mov edx,ted_tex
  xor ecx,ecx
  @@:
    cmp ecx,[wScr.position]
    je @f
    call IteratNext
    cmp edx,ted_tex_1
    jle @f
    cmp byte [edx],13
    jne @b
    inc ecx
    jmp @b
  @@:

  cmp ecx,[wScr.position]
  je @f
    xor edx,edx
  @@:
  cmp ecx,[wScr.max_area]
  jle @f
    mov [wScr.max_area],ecx
  @@:
  pop ecx
  call OptDrawLineLeft
  ret



;input:
; edx = pointer to symbol
; edi = pointer to tedit struct
;output:
; ecx = новый цвет символа
; edx = указатель на первый левый символ
align 4
proc OptDrawLineLeft
  push ebx

  mov ebx,[hScr.position]
  cmp ebx,0
  je .ret_f
  push eax
  mov eax,edx

  cmp edx,ted_tex
  jne @f
    call IteratNext
    jmp .beg_cycle
  @@:

  cmp ebx,0
  je @f

  cmp byte[edx],13
  jne @f
    call IteratNext
    .beg_cycle:
  @@:
    cmp edx,ted_tex_1
    jle @f
    cmp byte[edx],13
    je @f
    cmp ebx,0
    je @f
;--------------------------------------
xor eax,eax ;eax будет меняться
mov al,byte[edx+1]
cmp al,0
je .no_color
cmp eax,ted_colors_text_count
jge .no_color
  xor ecx,ecx
  mov cl,byte[edx+1]
  shl cx,2
  add ecx,ted_text_colors
  mov ecx,dword[ecx]
.no_color:
;--------------------------------------
    mov eax,edx
    call IteratNext
    dec ebx
    jmp @b
  @@:
    mov edx,eax
  pop eax
  .ret_f:
  call GetSymbColor
  pop ebx
  ret
endp

;input:
; edx = pointer to symbol
; edi = pointer to tedit struct
;output:
; edx = pointer to 13 symbol
align 4
proc OptDrawLineRight
  push eax
  mov eax,edx
  @@:
    cmp edx,ted_tex_1
    jle @f
    cmp byte[edx],13
    je @f
    mov eax,edx
    call IteratNext
    jmp @b
  @@:
  mov edx,eax ;perv sumbol
  call GetSymbColor

  pop eax
  ret
endp

align 4
proc draw_main_cursor, edit:dword
  pushad
  mov edi,dword[edit]

  mov eax,13 ;draw cursor
  mov ecx,ted_wnd_t ;calc rect -> y0,y1
  add ecx,ted_rec_t
  mov edx,ted_cur_y
  imul edx,ted_rec_h
  add ecx,edx

  cmp ted_cur_ins,1 ;проверка режима работы курсора (обычный или вставка)
  jne @f
    mov edx,ted_rec_h
    inc edx   ;1->1, 3->2, 5->3, ...
    shr edx,1 ;edx = высота строки деленная на 2 (когда курсор не полный)
    add ecx,edx
  @@:
  shl ecx,16
  add ecx,ted_rec_h
  cmp ted_cur_ins,1
  jne @f
    shr cx,1 ;делим высоту курсора на 2
  @@:

  mov ebx,ted_wnd_l ;calc rect -> x0,x1
  add ebx,ted_rec_l
  mov edx,ted_cur_x
  imul edx,ted_rec_w
  add ebx,edx
  shl ebx,16
  add ebx,ted_rec_w

  mov edx,ted_color_cursor
  int 0x40 ;вывод курсора

  call GetPos
  cmp ted_gp_opt,2
  jne @f
    mov eax,4 ;draw text
    mov esi,1
    ror ecx,16
    mov bx,cx
    add ebx,0x10001
    cmp ted_cur_ins,1
    jne .no_up_tetx
      mov ecx,ted_rec_h
      inc cx ; 1->1, 3->2, 5->3, ...
      shr cx,1
      sub bx,cx
    .no_up_tetx:
    mov ecx,ted_color_cur_text
    call ConvertInvisSymb
    int 0x40
  @@:

  mov eax,4
  mov ebx,ted_wnd_l
  add ebx,ted_rec_l
  shl ebx,16
  add ebx,ted_wnd_t
  add ebx,3
  mov ecx,ted_color_wnd_bord
  or  ecx,0x80000000
  mov edx,txtRow
  int 0x40 ;вывод подписи 'Строка'

  add ebx,0x500000
  mov edx,txtCol
  int 0x40 ;вывод подписи 'Знак'

  cmp ted_tim_undo,0
  je @f
    add ebx,0x500000
    mov edx,txtOtm
    int 0x40
    sub ebx,0x500000
  @@:

  stdcall draw_bufer,edi
  stdcall draw_help_f1

  mov eax,47 ;draw cursor coords
  mov esi,ted_color_wnd_bord
  or  esi,0x40000000

  mov edx,ebx
  ror edx,16
  sub edx,35
  ror edx,16
  ;add edx,3
  mov ebx,0x40000 ;Row=...
  mov ecx,ted_cur_y
  inc ecx
  add ecx,[wScr.position]

push edi
  mov edi,ted_color_wnd_work
  int 0x40 ;вывод числа текущей строки
pop edi

  ;mov ebx,0x40000 ;Col=...
  mov ecx,ted_cur_x
  inc ecx
  add ecx,[hScr.position]
  add edx,0x500000
push edi
  mov edi,ted_color_wnd_work
  int 0x40 ;вывод числа знаков
pop edi

  cmp ted_tim_undo,0
  je @f
    mov ecx,ted_tim_undo
    add edx,0x500000
    int 0x40
  @@:

  popad
  ret
endp

align 4
proc draw_bufer, edit:dword
  pushad
  mov edi,dword[edit]

  cmp byte[buf],0
  je @f
    mov ebx,ted_wnd_l
    add ebx,ted_rec_l
  add bx,250
    shl ebx,16
    add ebx,ted_wnd_t
    add ebx,3
    mov ecx,ted_color_wnd_bord
    or	ecx,0x40000000
    mov edi,ted_color_wnd_work
    
    mov edx,txtBuf
    mov esi,buf
    call strlen
if lang eq ru
    cmp eax,15
    jle .crop_buf
      mov eax,15
    .crop_buf:
    mov esi,7 ;7=len(txtBuf)
else
    cmp eax,14
    jle .crop_buf
      mov eax,14
    .crop_buf:
    mov esi,8 ;8=len(txtBuf)
end if
    add esi,eax

    mov eax,4
    int 0x40
  @@:
  popad
  ret
endp

;input:
; edi = pointer to tedit struct
align 4
proc draw_help_f1
  pushad

  mov eax,13 ;clear place before draw help
  mov ebx,ted_wnd_l
  add ebx,ted_rec_l
  shl ebx,16
  add ebx,ted_wnd_w
  sub ebx,ted_rec_l
  mov ecx,ted_wnd_t
  add ecx,13
  shl ecx,16
  add ecx,ted_rec_h
  mov edx,ted_color_wnd_capt
  int 0x40

  cmp ted_help_id,-1
  je @f
    mov eax,ted_help_id
    ColToIndexOffset eax,edx

    ;SetCoordinates
    mov ebx,ted_wnd_l
    add ebx,ted_rec_l
    shl ebx,16
    add ebx,ted_wnd_t
    add ebx,13 ;=3+10

    ;SetTextColor
    xor eax,eax
    mov al,byte[edx+MAX_COLOR_WORD_LEN+6]
    shl ax,2
    mov ecx,ted_text_colors
    add ecx,eax
    mov ecx,dword[ecx]
    or	ecx,0xc0000000 ;SetTextStyles
	mov esi,edi
      mov edi,ted_color_wnd_work
      mov eax,4
      int 0x40
    mov edi,esi

    ;*** draw help string ***
    mov ecx,ted_color_wnd_bord
    or ecx,0x80000000
    mov edx,dword[edx+MAX_COLOR_WORD_LEN]
    cmp edx,0
    je @f
      add edx,ted_help_text_f1
      add ebx,0x500000
      int 0x40
  @@:
  popad
  ret
endp

align 4
proc draw_line_numbers, edit:dword
  pushad
  mov edi,dword[edit]

  ;top panel with caption
  mov ebx,ted_wnd_l
;  add ebx,ted_rec_l
  shl ebx,16
  add ebx,ted_wnd_w
;  sub ebx,ted_rec_l
  mov edx,ted_color_wnd_work
  mov ecx,ted_wnd_t
  shl ecx,16
  add ecx,ted_rec_t
  mov eax,13
  mov edx,ted_color_wnd_capt
  int 0x40

  ;line numbers
  mov ebx,0x40000 ;format
  mov ecx,[wScr.position]
  inc ecx
  mov edx,3
  add edx,ted_wnd_l
  rol edx,16
  add edx,ted_wnd_t
  add edx,ted_rec_t
  @@:

push ebx ecx edx
  ;left panel with numbers
  mov ebx,ted_wnd_l
  shl ebx,16
  add ebx,ted_rec_l
  mov ecx,ted_rec_h
  rol ecx,16
  mov cx,dx
  rol ecx,16
  mov eax,13
  mov edx,ted_color_wnd_capt
  int 0x40 ;рисуем прямоугольник под номером строки
pop edx ecx ebx

    mov eax,47
    mov esi,ted_color_wnd_bord
    int 0x40 ;рисуем номер строки
    inc ecx
    add edx,ted_rec_h
    sub edx,ted_wnd_t
	mov esi,edx
	and esi,0xffff
    cmp esi,ted_wnd_h
    jge @f
    add edx,ted_wnd_t
    jmp @b
  @@:

  popad
  ret
endp
  
;this function need to optimize output
align 4
proc draw_cursor_sumb, edit:dword
  push eax ecx edx edi
  mov edi,dword[edit]

  mov eax,13 ;rect
  mov ebx,ted_wnd_l
  add ebx,ted_rec_l
  mov edx,ted_cur_x
  imul edx,ted_rec_w
  add ebx,edx
  shl ebx,16
  add ebx,ted_rec_w

  mov ecx,ted_wnd_t ;calc rect -> y0,y1
  add ecx,ted_rec_t
  mov edx,ted_cur_y
  imul edx,ted_rec_h
  add ecx,edx
  shl ecx,16
  add ecx,ted_rec_h

  mov edx,ted_color_wnd_work
  push ecx
    stdcall SelNormalize,edi

    mov ecx,[wScr.position]
    sub ted_seln_y0,ecx
    sub ted_seln_y1,ecx

    mov ecx,ted_cur_y
    cmp ecx,ted_seln_y0
    jl .no_cur_sel
    cmp ecx,ted_seln_y1
    jg .no_cur_sel
    mov edx,ted_color_select
    
    cmp ecx,ted_seln_y0
    jne @f
      mov ecx,ted_cur_x
      add ecx,[hScr.position]
      cmp ecx,ted_seln_x0
      jge @f
	mov edx,ted_color_wnd_work
    @@:

    mov ecx,ted_cur_y
    cmp ecx,ted_seln_y1
    jne .no_cur_sel
      mov ecx,ted_cur_x
      add ecx,[hScr.position]
      cmp ecx,ted_seln_x1
      jl .no_cur_sel
      mov edx,ted_color_wnd_work

    .no_cur_sel:
  pop ecx
  int 0x40

  call GetPos
  cmp ted_gp_opt,2
  jne @f
    push esi
    mov eax,4 ;draw text
    mov esi,1
    ror ecx,16
    mov bx,cx
    add ebx,0x10001
    call GetSymbColor
    call ConvertInvisSymb
    int 0x40
    pop esi
  @@:

  pop edi edx ecx eax
  ret
endp


;input:
; edx -> pointer to text
; edi -> указатель на структуру tedit
;output:
; ecx = color
; if ted_mode_color=0 then ecx=ted_color_wnd_text
align 4
GetSymbColor:
  mov ecx,ted_color_wnd_text ;задаем цвет текста по умолчанию

  push eax edx
  cmp ted_mode_color,0
  je .exit
    jmp .on_first
    @@:
      call IteratPerv
      cmp edx,ted_tex_1
      jle .exit
    .on_first:
      xor eax,eax
      mov al,byte[edx+1]
      cmp al,0 ;если al=0 то цвет не меняется
    je @b

    cmp eax,ted_colors_text_count
    jge .exit

    shl ax,2 ;умножаем индекс цвета на 4 байта
    mov ecx,ted_text_colors ;прибавляем смещение 1-го цвета
    add ecx,eax
    mov ecx,dword[ecx] ;устанавливаем текущий цвет текста по смещению
  .exit:
  pop edx eax
  ret

;input:
; edx = pointer to text
; edi = pointer to tedit struct
align 4
ConvertInvisSymb:
  cmp ted_mode_invis,1
  jne @f
  cmp byte [edx],13
  jne @f
    mov edx,edi
	add edx,ted_offs_symbol_new_line
  @@:
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
  mov esi,[sc.work_button]
  int 0x40
  pop esi edx ecx

  mov ebx,ecx
  ror ebx,16
  cmp bx,0x4000
  jne @f

  mov eax,7 ;bmp
  mov ebx,[bmp_icon]
  push ecx
  and ecx,0xffff
  imul ecx,1200
  add ebx,ecx
  mov ecx,20
  shl ecx,16
  add ecx,20
  int 0x40
  pop ecx

  @@:
  pop ebx eax
  ret

