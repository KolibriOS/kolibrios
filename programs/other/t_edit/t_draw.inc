mouse_wnd_main:
  push eax ebx ecx
  mcall 37,2 ;get mouse buttons

  cmp al,1
  jne @f
    mcall 37,1 ;get mouse coords
    mov ebx,[wndMain.top]
    add ebx,[recMain.top]
    cmp ax,bx
    jl @f ;y_mouse<y_wnd

    sub ebx,[recMain.top]
    add ebx,[wndMain.height]
    cmp bx,ax
    jl @f ;y_mouse>y_wnd

    mov ebx,[wndMain.left]
    add ebx,[recMain.left]
    mov ecx,eax
    shr ecx,16
    cmp cx,bx
    jl @f ;x_mouse<x_wnd

    sub ebx,[recMain.left]
    add ebx,[wndMain.width]
    cmp bx,cx
    jl @f ;x_mouse>x_wnd

    call draw_cursor_sumb
    call wnd_main_click
    ;call draw_main_cursor ;???
    jmp .no_in_wnd
  @@:
    call wnd_main_mouse_scroll
    cmp [dragm],0
    je .no_in_wnd
      mov [dragm],0
      call draw_but_toolbar
      call draw_main_win
  .no_in_wnd:
  pop ecx ebx eax
  ret

; input:
;  eax->(x,y)
wnd_main_click:
  push ecx edx

  push eax
    shr eax,16
    and eax,0xffff
    sub eax,[wndMain.left]
    sub eax,[recMain.left]

    xor edx,edx
    mov ecx,[recMain.width]
    div cx
    ;inc eax
    cmp eax,[hScr.cur_area]
    jle @f
      mov eax,[hScr.cur_area]
    @@:
    ;dec eax
    mov [cur_x],eax
  pop eax

  push eax
    and eax,0xffff
    sub eax,[wndMain.top]
    sub eax,[recMain.top]

    xor edx,edx
    mov ecx,[recMain.height]
    div cx
    inc eax
    cmp eax,[wScr.cur_area]
    jle @f
      mov eax,[wScr.cur_area]
    @@:
    dec eax
    mov [cur_y],eax
  pop eax

  cmp [dragm],0
  je @f
    call SelMove
    jmp .sel_move
  @@:
    mov [dragm],1
    call SelStart
  .sel_move:
  pop edx ecx
  ret

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
    call draw_main_win
  .no_scroll:
  pop ebx eax
  ret

draw_but_toolbar:

  push ebx ecx edx
  mov ecx,0x40000000
;  mov edx,5*65536+25
  mov edx,85*65536+2
  call draw_but_icon

  inc cx
;  mov edx,30*65536+25
  mov edx,110*65536+2
  call draw_but_icon

call CanSave
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

call IsSel
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

mov ebx,[tim_Undo]
cmp [ch_tim],ebx
jg @f
and ecx,0xffff
@@:
  mov cx,13
  mov edx,345*65536+25
  call draw_but_icon
or ecx,0x40000000

cmp [tim_Undo],1
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

  pop edx ecx ebx
  ret


;input:
; ebx = x*2^16+y coords to left up point clear line
; esi = 0 clear all rows
; edi - number text line
clear_line_before_draw:
  push eax ebx ecx edx
    sub ebx,0x10001 ;отнимаем отступы дл€ выравнивани€ буквы по центру
    cmp esi,0
    jne @f
      add bx,word[recMain.height]
      inc edi ; ???
      ror ebx,16
      mov bx,word[wndMain.left]
      add bx,word[recMain.left]
      ror ebx,16
    @@:

    mov ax,word[wndMain.height]
    add ax,word[wndMain.top]
    cmp ax,bx
    jl .no_clear
    sub ax,bx

    mov cx,bx
    shl ecx,16

    mov bx,word[wndMain.width]
    sub bx,word[recMain.left]
    mov cx,word[recMain.height]
    mov edx,[WND_WORK_COLOR]

    cmp esi,0
    je .pusto
    cmp ax,cx
    jge @f
      .pusto:
      mov cx,ax
    @@:

    call IsSel
    cmp al,0
    je @f
    cmp [seln.y0],edi
    jg @f
    cmp [seln.y1],edi
    jl @f
      mov edx,[SELECT_COLOR] ;draw selected line
    @@:

    mov eax,13 ;rect
    int 0x40

    call IsSel
    cmp al,0
    je .no_clear

    mov al,13 ;rect
    mov cx,word[recMain.height]
    cmp [seln.y0],edi
    jne @f
      push bx
      mov edx,[seln.x0] ; верхн€€ полоса (затирает слева)
      cmp edx,[hScr.position]
      jle .in_wnd
	sub edx,[hScr.position]
	imul edx,[recMain.width]
	mov bx,dx
	jmp .no_wnd
      .in_wnd:
	mov bx,0
      .no_wnd:
      mov edx,[WND_WORK_COLOR]
      int 0x40
      pop bx
    @@:
    cmp [seln.y1],edi
    jne @f
      mov edx,[seln.x1] ; нижн€€ полоса (затирает справа)
      cmp edx,[hScr.position]
      jle .in_wnd2
	sub edx,[hScr.position]
	imul edx,[recMain.width]
	sub bx,dx
	shl edx,16
	add ebx,edx
      .in_wnd2:

      mov edx,[WND_WORK_COLOR]
      int 0x40
    @@:

  .no_clear:
  pop edx ecx ebx eax
  ret

;include 't_debug.inc'
draw_main_win:
  push eax ebx ecx edx edi esi
;---debug---
;mov ecx,0
;@@:
;call DebugSymb
;inc ecx
;cmp ecx,10
;jge @f
;jmp @b
;@@:
;ret
;---debug---

  mov eax,4 ;draw text
  mov esi,1
  mov ecx,[tex_colors]
  push dword[ecx]
  pop ecx

  mov ebx,[wndMain.left]
  add ebx,[recMain.left]
  inc ebx
  shl ebx,16
  add ebx,[wndMain.top]
  add ebx,[recMain.top]
  inc ebx

  call SelNormalize ;need before draw select
  mov edi,[wScr.position]

  call clear_line_before_draw
  call GetFirstVisiblePos
  cmp edx,0
  je .no_draw_text
  @@:
    call IteratNext
    cmp edx,[tex_1]
    jle .no_draw_text

;--- debug
;cmp dword[edx+6],maxChars
;jge .no_draw_text
;--- debug

    ; *** цветова€ разметка
    cmp byte[mode_colored],0
    je .no_col_change
    cmp byte[edx+1],0
    je .no_col_change
      call GetSymbColor
    .no_col_change:

    cmp byte [edx],13
    jne .no_13
      cmp [invis],1
      jne .no_invis
	push edx
	mov edx,symbol_new_line
	int 0x40
	pop edx
      .no_invis:
      add bx,word [recMain.height]
      ;optimized output \/
      mov ax,word [wndMain.height]
      add ax,word [wndMain.top]
      cmp bx,ax
      jg .no_draw_text
      mov ax,4
      ;optimized output /\
      ror ebx,16
      mov bx,word [wndMain.left]
      add bx,word [recMain.left]
      inc ebx
      ror ebx,16
      inc edi ;increment line number
      call clear_line_before_draw
      call OptDrawLineLeft
      jmp @b
    .no_13:

    int 0x40
    ror ebx,16
    add bx,word [recMain.width]
    mov si,word [wndMain.left]
    add si,word [wndMain.width]
    cmp bx,si
    jl .no_opt
      call OptDrawLineRight
    .no_opt:
    mov si,1
    ror ebx,16
    jmp @b
  .no_draw_text:
  xor esi,esi
  call clear_line_before_draw


  mov eax,13
  ;top panel with caption
  mov ebx,[wndMain.left]
;  add ebx,[recMain.left]
  shl ebx,16
  add ebx,[wndMain.width]
;  sub ebx,[recMain.left]
  mov edx,[WND_WORK_COLOR]
  mov ecx,[wndMain.top] ;draw caption
  shl ecx,16
  add ecx,[recMain.top]
  mov edx,[WND_CAPT_COLOR]
  int 0x40
  ;left panel with numbers
;  mov ebx,[wndMain.left]
;  shl ebx,16
;  add ebx,[recMain.left]
  ;mov cx,word[wndMain.height]
;  mov cx,word[recMain.top]
;  int 0x40

  call draw_line_numbers
  call draw_main_cursor

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
	push	 dword wScr
	call	 [scrollbar_ver_draw]
	push	 dword hScr
	call	 [scrollbar_hor_draw]
; reset all_redraw flag 
	xor	eax,eax
	mov	[wScr.all_redraw],eax
	mov	[hScr.all_redraw],eax
;---------------------------------------------

  mov eax,13
  ;left-bottom square
  mov bx,word[wndMain.left]
  shl ebx,16
  mov bx,word[recMain.left]
  mov ecx,[wndMain.top]
  add ecx,[wndMain.height]
  shl ecx,16
  mov cx,word[hScr.size_y]
  inc cx
  mov edx,[sc.work]
  int 0x40

  ;right-bottom square
  mov ebx,[wndMain.left]
  add ebx,[wndMain.width]
  shl ebx,16
  mov bx,word[wScr.size_x]
  inc bx
  int 0x40

  pop esi edi edx ecx ebx eax
  call draw_panel_find
  call draw_panel_syntax
  ret

;txtBUp db 24
;txtBDn db 25
;txtBRi db 26
;txtBLe db 27
draw_panel_find:
  cmp byte[panel_id],TE_PANEL_FIND ;if not panel
  jne @f
  push eax ebx ecx edx

  mov eax,13 ;рисование пр€моугольника
  mov ebx,TE_PANEL_WIDTH
  mov cx,word[wndMain.top]
  shl ecx,16
  mov cx,20
  mov edx,[sc.work]
  int 0x40

  mov eax,4 ;рисование текста
  mov ebx,30*65536+5
  add bx,word[wndMain.top]
  mov ecx,[sc.work_text]
  or ecx,0x80000000
  mov edx,txtFindCapt
  int 0x40

  push dword edit2
  call [edit_box_draw]

  mov eax,13 ;рисование пр€моугольника
  mov ebx,TE_PANEL_WIDTH
  mov cx,word[wndMain.top]
  add cx,20+15 ; 15 - height text box
  shl ecx,16
  mov cx,word[wndMain.height]
  add cx,word[hScr.size_y]
  sub cx,20+15-1 ; 15 - height text box
  mov edx,[sc.work]
  int 0x40

  mov eax,8 ;кнопка
  mov ebx,5*65536+85
  mov cx,word[wndMain.top]
  add cx,20+15+5
  shl ecx,16
  mov cx,20
  mov edx,201 ;button id
  mov esi,[sc.work_button]
  int 0x40

  mov eax,4 ;рисование текста
  mov ebx,15*65536+(20+15+10)
  add bx,word[wndMain.top]
  mov ecx,[sc.work_text]
  or ecx,0x80000000
  mov edx,txtFindNext
  int 0x40

  pop edx ecx ebx eax
  ret
  @@:
  push eax edx
    mov eax,8
    mov edx,201
    or edx,0x80000000
    int 0x40
  pop edx eax
  ret

draw_panel_syntax:
  cmp byte[panel_id],TE_PANEL_SYNTAX ;if not panel
  jne @f
  push eax ebx ecx edx

  mov eax,13 ;рисование пр€моугольника
  mov ebx,TE_PANEL_WIDTH
  mov cx,word[wndMain.top]
  shl ecx,16
  mov cx,20
  mov edx,[sc.work]
  int 0x40

  push dword tree1
  call dword[tl_draw]

  mov	[ws_dir_lbox.all_redraw],1
  push dword ws_dir_lbox
  call dword[scrollbar_ver_draw]

  ror ecx,16
  add ecx,dword[tree1.box_height]
  add ecx,20
  ror ecx,16
  mov cx,word[wndMain.height]
  add cx,word[hScr.size_y]
  sub cx,20
  sub ecx,dword[tree1.box_height]
  inc cx
  int 0x40

  mov eax,8 ;кнопка
  mov ebx,5*65536+65
  mov cx,word[wndMain.top]
  add cx,25
  add ecx,dword[tree1.box_height]
  shl ecx,16
  mov cx,20
  mov edx,200 ;button id
  mov esi,[sc.work_button]
  int 0x40

  mov eax,4 ;рисование текста
  mov ebx,30*65536+5
  add bx,word[wndMain.top]
  mov ecx,[sc.work_text]
  or ecx,0x80000000
  mov edx,txtFormatCapt
  int 0x40

  mov ebx,10*65536+30
  add ebx,dword[tree1.box_height]
  add bx,word[wndMain.top]
  mov edx,txtFormatApply
  int 0x40

  pop edx ecx ebx eax
  ret
  @@:
  push eax edx
    mov eax,8
    mov edx,200
    or edx,0x80000000
    int 0x40
  pop edx eax
  ret

draw_cur_line:
  push eax ebx ecx edx esi edi

    mov edi,[cur_y]

    mov ebx,[wndMain.left]
    add ebx,[recMain.left]
    inc ebx
    shl ebx,16
    add ebx,edi
    imul bx,word[recMain.height]
    add ebx,[wndMain.top]
    add ebx,[recMain.top]
    inc ebx

    add edi,[wScr.position]
    mov esi,1
    call SelNormalize ;need before draw select
    call clear_line_before_draw

    mov esi,[hScr.position]
    mov ecx,edi
    call GetPosByParam

  cmp [gpOpt],2
  jne .no_draw_text
;  mov eax,4 ;draw text
  call GetSymbColor
  mov esi,1 ;draw 1 symbol
  @@:
    ;call IteratNext
    cmp edx,[tex_1]
    jle .no_draw_text

    ; *** цветова€ разметка
    cmp byte[mode_colored],0
    je .no_col_change
    cmp byte[edx+1],0
    je .no_col_change
      call GetSymbColor
    .no_col_change:

    mov eax,4 ;draw text
    cmp byte [edx],13
    jne .no_13
      cmp [invis],1
      jne .no_draw_text
 push edx
 mov edx,symbol_new_line
 int 0x40
 pop edx
      jmp .no_draw_text
    .no_13:

    int 0x40
    ror ebx,16
    add bx,word[recMain.width]
;    cmp bx,word[wndMain.width]
    mov ax,word[wndMain.width]
    add ax,word[wndMain.left] ;ax = отступ по оси x
    cmp bx,ax
    jge .no_draw_text ;Opt
    ror ebx,16
    call IteratNext
    jmp @b
  .no_draw_text:

  pop edi esi edx ecx ebx eax
  call draw_main_cursor
  ret

MIN_M_WND_H equ 100
MIN_W_SCRL_ARE equ 3
MIN_H_SCRL_ARE equ 3
EvSize:
  push eax ecx edx edi
  m2m [wndMain.width],[procinfo.client_box.width]
  mov ax,word[wndMain.left]
  sub word[wndMain.width],ax
  mov eax,[wScr] ;+0 size_x
  sub word[wndMain.width],ax

  m2m [wndMain.height],[procinfo.client_box.height]
  cmp [wndMain.height],MIN_M_WND_H
  jg @f
    mov [wndMain.height],MIN_M_WND_H
  @@:

  mov ax,word[hScr.size_y]
  sub word[wndMain.height],ax
    mov eax,[wndMain.height] ;.height = .top+.height
    mov word[hScr.start_y],ax
  mov eax,[wndMain.top]
  sub word[wndMain.height],ax

  m2m word[wScr.start_x],word[wndMain.width]
  mov ax,word[wndMain.left]
  add [wScr.start_x],ax
  m2m word[wScr.size_y],word[wndMain.height]

  m2m word[hScr.start_x],word[recMain.left]
  add [hScr.start_x],ax ;ax=[wndMain.left]
  mov eax,[wndMain.width]
  sub ax,word[recMain.left]
  mov word[hScr.size_x],ax

  mov eax,[wndMain.height] ;calculate lines in page
  sub eax,[recMain.top]
  xor edx,edx
  mov ecx,[recMain.height]
  div ecx
  cmp eax,MIN_W_SCRL_ARE
  jg @f
    mov eax,MIN_W_SCRL_ARE
  @@:
  mov [wScr.cur_area],eax

  mov eax,[wndMain.width] ;calculate cols in page
  sub eax,[recMain.left]
  xor edx,edx
  mov ecx,[recMain.width]
  div ecx
  cmp eax,MIN_H_SCRL_ARE
  jg @f
    mov eax,MIN_H_SCRL_ARE
  @@:
  dec eax ; ???
  mov [hScr.cur_area],eax

  mov edi,dword tree1
  m2m tl_box_top,dword[wndMain.top]
  add tl_box_top,20

  m2m dword[edit2.top],dword[wndMain.top]
  add dword[edit2.top],20

  pop edi edx ecx eax
  call OnInitialUpdate
  ret

OnInitialUpdate:
  mov [wScr.redraw],1
  mov [hScr.redraw],1
  ret

;output:
; edx = pointer to symbol
; edx = 0 if text not in screen
GetFirstVisiblePos:
  push ecx
  mov edx,[tex]
  xor ecx,ecx
  @@:
    cmp ecx,[wScr.position]
    je @f
    call IteratNext
    cmp edx,[tex_1]
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
;output:
; edx = pointer to first left symbol
OptDrawLineLeft:
  push ecx
  mov ecx,[hScr.position]
  cmp ecx,0
  je .ret_f
  push eax
  mov eax,edx

  cmp edx,[tex]
  jne @f
    call IteratNext
    jmp .beg_cycle
  @@:

  cmp ecx,0
  je @f

  cmp byte[edx],13
  jne @f
    call IteratNext
    .beg_cycle:
  @@:
    cmp edx,[tex_1]
    jle @f
    cmp byte[edx],13
    je @f
    cmp ecx,0
    je @f
    mov eax,edx
    call IteratNext
    dec ecx
    jmp @b
  @@:
    mov edx,eax
  pop eax
  .ret_f:
  pop ecx
  call GetSymbColor
  ret

;input:
; edx = pointer to symbol
;output:
; edx = pointer to 13 symbol
OptDrawLineRight:
  push eax
  mov eax,edx
  @@:
    cmp edx,[tex_1]
    jle @f
    cmp byte[edx],13
    je @f
    mov eax,edx
    call IteratNext
    jmp @b
  @@:
  mov edx,eax ;perv sumbol
  pop eax
  call GetSymbColor
  ret

txtRow db 'СваЃ™†',0
txtCol db 'З≠†™',0
txtOtm db 'Овђ•≠л',0

draw_main_cursor:
  push eax ebx ecx edx edi esi

  mov eax,13 ;draw cursor
  mov ecx,[wndMain.top] ;calc rect -> y0,y1
  add ecx,[recMain.top]
  mov edx,[cur_y]
  imul edx,[recMain.height]
  add ecx,edx

  cmp [curMod],1
  jne @f
    mov edx,[recMain.height]
    inc edx ; 1->1, 3->2, 5->3, ...
    shr edx,1
    add ecx,edx
  @@:
  shl ecx,16
  add ecx,[recMain.height]
  cmp [curMod],1
  jne @f
    shr cx,1
  @@:

  mov ebx,[wndMain.left] ;calc rect -> x0,x1
  add ebx,[recMain.left]
  mov edx,[cur_x]
  imul edx,[recMain.width]
  add ebx,edx
  shl ebx,16
  add ebx,[recMain.width]

  mov edx,[CURSOR_COLOR]
  int 0x40

  call GetPos
  cmp [gpOpt],2
  jne @f
    mov eax,4 ;draw text
    mov esi,1
    ror ecx,16
    mov bx,cx
    add ebx,0x10001
    cmp [curMod],1
    jne .no_up_tetx
      mov cx,word[recMain.height]
      inc cx ; 1->1, 3->2, 5->3, ...
      shr cx,1
      sub bx,cx
    .no_up_tetx:
    mov ecx,[CUR_TEXT_COLOR]
    call ConvertInvisSymb
    int 0x40
  @@:



  mov eax,4
  mov ebx,[wndMain.left]
  add ebx,[recMain.left]
  shl ebx,16
  add ebx,[wndMain.top]
  add ebx,3
  mov ecx,[WND_BORD_COLOR]
  or  ecx,0x80000000
  mov edx,txtRow
  int 0x40

  add ebx,0x500000
  mov edx,txtCol
  int 0x40

  cmp [tim_Undo],0
  je @f
    add ebx,0x500000
    mov edx,txtOtm
    int 0x40
    sub ebx,0x500000
  @@:

  call draw_bufer
  call draw_help_f1

  mov eax,47 ;draw cursor coords
  mov esi,[WND_BORD_COLOR]
  or  esi,0x40000000
  mov edi,[WND_WORK_COLOR]

  mov edx,ebx
  ror edx,16
  sub edx,35
  ror edx,16
  ;add edx,3
  mov ebx,0x40000 ;Row=...
  mov ecx,[cur_y]
  inc ecx
  add ecx,[wScr.position]
  int 0x40

  ;mov ebx,0x40000 ;Col=...
  mov ecx,[cur_x]
  inc ecx
  add ecx,[hScr.position]
  add edx,0x500000
  int 0x40

  cmp [tim_Undo],0
  je @f
    mov ecx,[tim_Undo]
    add edx,0x500000
    int 0x40
  @@:

  pop esi edi edx ecx ebx eax
  ret


draw_bufer:
  push eax ebx ecx edx esi edi
  cmp byte[buf],0
  je @f
    mov ebx,[wndMain.left]
    add ebx,[recMain.left]
  add bx,250
    shl ebx,16
    add ebx,[wndMain.top]
    add ebx,3
    mov ecx,[WND_BORD_COLOR]
    or	ecx,0x40000000
    mov edi,[WND_WORK_COLOR]
    
    mov edx,txtBuf
    mov esi,buf
    call strlen
    cmp eax,15
    jle .crop_buf
      mov eax,15
    .crop_buf:
    mov esi,7
    add esi,eax

    mov eax,4
    int 0x40
  @@:
  pop edi esi edx ecx ebx eax
  ret

draw_help_f1:
  push eax ebx ecx edx edi
  mov eax,13 ;clear place before draw help
  mov ebx,[wndMain.left]
  add ebx,[recMain.left]
  shl ebx,16
  add ebx,[wndMain.width]
  sub ebx,[recMain.left]
  mov ecx,[wndMain.top]
  add ecx,13
  shl ecx,16
  add ecx,[recMain.height]
  mov edx,[WND_CAPT_COLOR]
  int 0x40

  cmp [help_id],-1
  je @f;.no_help
    mov eax,[help_id]
    ColToIndexOffset eax,edx

    ;SetCoordinates
    mov ebx,[wndMain.left]
    add ebx,[recMain.left]
    shl ebx,16
    add ebx,[wndMain.top]
    add ebx,13 ;=3+10

    ;SetTextColor
    xor eax,eax
    mov al,byte[edx+MAX_COLOR_WORD_LEN+6]
    shl ax,2
    mov ecx,[tex_colors]
    add ecx,eax
    push dword[ecx]
    pop ecx
    or	ecx,0xc0000000 ;SetTextStyles
    mov edi,[WND_WORK_COLOR]

    mov eax,4
    int 0x40

    ;*** draw help string ***
    mov ecx,[WND_BORD_COLOR]
    or ecx,0x80000000
    mov edi,dword[edx+MAX_COLOR_WORD_LEN]
    cmp edi,0
    je @f
      add edi,dword[tex_help_f1]
      mov edx,edi
      add ebx,0x500000
      int 0x40
    @@:
  ;.no_help:
  pop edi edx ecx ebx eax
  ret

draw_line_numbers:
  push eax ebx ecx edx esi ;edi
  ;line numbers
  mov eax,47
  mov esi,[WND_BORD_COLOR]
  ;or esi,0x40000000
  ;mov edi,[WND_WORK_COLOR]
  mov ebx,0x40000 ;format

  mov ecx,[wScr.position]
  inc ecx
  mov dx,3
  add dx,word[wndMain.left]
  shl edx,16
  mov dx,word[wndMain.top]
  add dx,word[recMain.top]
  @@:

push eax ebx ecx edx
  mov eax,13
  ;left panel with numbers
  mov ebx,[wndMain.left]
  shl ebx,16
  add ebx,[recMain.left]
  mov cx,dx
;  mov ecx,[wndMain.top] ;draw caption
;  add ecx,[recMain.top]
  shl ecx,16
  mov cx,word [recMain.height]
  mov edx,[WND_CAPT_COLOR]
  int 0x40
pop edx ecx ebx eax

    int 0x40
    inc ecx
    add dx,word[recMain.height]
    sub dx,word[wndMain.top]
    cmp dx,word[wndMain.height]
    jge @f
    add dx,word[wndMain.top]
    jmp @b
  @@:

  pop esi edx ecx ebx eax
  ret


draw_cursor_sumb: ;this function need to optimize output
  push eax ecx edx
  mov eax,13 ;rect
  mov ebx,[wndMain.left]
  add ebx,[recMain.left]
  mov edx,[cur_x]
  imul edx,[recMain.width]
  add ebx,edx
  shl ebx,16
  add ebx,[recMain.width]

  mov ecx,[wndMain.top] ;calc rect -> y0,y1
  add ecx,[recMain.top]
  mov edx,[cur_y]
  imul edx,[recMain.height]
  add ecx,edx
  shl ecx,16
  add ecx,[recMain.height]

  mov edx,[WND_WORK_COLOR]
  push ecx
    call SelNormalize

    mov ecx,[wScr.position]
    sub [seln.y0],ecx
    sub [seln.y1],ecx

    mov ecx,[cur_y]
    cmp ecx,[seln.y0]
    jl .no_cur_sel
    cmp ecx,[seln.y1]
    jg .no_cur_sel
    mov edx,[SELECT_COLOR]
    
    cmp ecx,[seln.y0]
    jne @f
      mov ecx,[cur_x]
      add ecx,[hScr.position]
      cmp ecx,[seln.x0]
      jge @f
	mov edx,[WND_WORK_COLOR]
    @@:

    mov ecx,[cur_y]
    cmp ecx,[seln.y1]
    jne .no_cur_sel
      mov ecx,[cur_x]
      add ecx,[hScr.position]
      cmp ecx,[seln.x1]
      jl .no_cur_sel
      mov edx,[WND_WORK_COLOR]

    .no_cur_sel:
  pop ecx
  int 0x40

  call GetPos
  cmp [gpOpt],2
  jne @f
    push esi
    mov eax,4 ;draw text
    mov esi,1
    ror ecx,16
    mov bx,cx
    add ebx,0x10001
    ;mov ecx,[WND_TEXT_COLOR]
    call GetSymbColor
    call ConvertInvisSymb
    int 0x40
    pop esi
  @@:

  pop edx ecx eax
  ret



;input:
; edx = pointer to text
;output:
; ecx = color
; if mode_colored=0 then ecx=WND_TEXT_COLOR
GetSymbColor:
  mov ecx,[WND_TEXT_COLOR]

  push eax edx
  cmp byte[mode_colored],0
  je .exit
    jmp .on_first
    @@:
      call IteratPerv
      cmp edx,[tex_1]
      jle .exit
    .on_first:
      xor eax,eax
      mov al,byte[edx+1]
      cmp al,0
    je @b

    ;cmp al,0xff
    ;je .exit
    cmp eax,[ColColor]
    jge .exit

    shl ax,2 ;*4
    mov ecx,[tex_colors]
    add ecx,eax
    push dword[ecx]
    pop ecx
  .exit:
  pop edx eax
  ret

;input:
; edx = pointer to text
ConvertInvisSymb:
  cmp [invis],1
  jne @f
  cmp byte [edx],13
  jne @f
    mov edx,symbol_new_line
  @@:
  ret

;input:
;  ecx = 0x4000____
;   cx = icon index
;  edx = x*2^16+y
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

