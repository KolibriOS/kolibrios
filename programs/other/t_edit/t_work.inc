struct f70
  func_n dd ?
  param1 dd ?
  param2 dd ?
  param3 dd ?
  param4 dd ?
  rezerv db ?
  name dd ?
ends

struct TexColViv
  Text rb MAX_COLOR_WORD_LEN; // слово
  f1 dd 0
  wwo db ? ; whole words only
  endc db ? ; символ конца выделения (wwo&4)
  color db ? ; номер цвета
ends

struct symbol
  c db ?    ;  +0 символ
  col db ?  ;  +1 цвет
  perv dd ? ;  +2
  next dd ? ;  +6 указатели
  tc dd ?   ; +10 врем. создания
  td dd ?   ; +14 врем. удаления
ends


hed db 'TextEditor 22.04.10',0 ;подпись окна
sc system_colors

fn_icon db 'te_icon.bmp',0
bmp_icon dd 0
run_file_70 f70 	  


;input:
; edi = pointer to tedit struct
; reg = index
;output:
; reg = pointer to 'tex' struct
macro ConvertIndexToPointer reg {
  imul reg,sizeof.symbol
  add reg,ted_tex
}

align 4
proc ted_init, edit:dword
  push eax ecx edi
  mov edi,dword[edit]

  mov ecx,sizeof.symbol*maxChars
  call mem_Alloc ;выделяем память
  mov ted_tex,eax
  mov ted_tex_1,eax
  add ted_tex_1,sizeof.symbol
  mov ted_tex_end,eax
  add ted_tex_end,sizeof.symbol*maxChars

  stdcall ted_clear, edi,1

;-------------------------------------------------
  mov ecx,maxColWords*sizeof.TexColViv+40
  ;add ecx,40
  call mem_Alloc
  mov ted_syntax_file,eax
  mov ted_syntax_file_end,eax
  add ted_syntax_file_end,ecx

  pop edi ecx eax
  ret
endp

align 4
proc ted_delete, edit:dword
  push ecx edi
  mov edi,dword[edit]

  mov ecx,ted_tex
  call mem_Free
  mov ecx,ted_syntax_file
  call mem_Free

  pop edi ecx
  ret
endp


;input:
; eax = key kodes
; ebx = key control
align 4
proc ted_key, edit:dword, table:dword
  pushad
  mov edi,dword[edit]

  cmp ah,KEY_F1 ;[F1]
  jne @f
    stdcall ted_show_help_f1,edi
    jmp .end_key_fun
  @@:
  cmp ah,KEY_F3 ;[F3]
  jne @f
    stdcall but_FindText,edi
    jmp .end_key_fun
  @@:

  test ebx,KM_CTRL ;Ctrl+...
  jz .key_Ctrl

    cmp ah,24 ;Ctrl+O
    jne @f
      call but_OpenFile
    @@:
    cmp ah,33 ;Ctrl+F
    jne @f
    cmp ted_panel_id,TE_PANEL_FIND
    je @f
      stdcall but_find,edi
    @@:
    cmp ah,44 ;Ctrl+Z
	jne @f
      stdcall ted_but_undo,edi
	@@:
    cmp ah,46 ;Ctrl+C
    jne @f
      stdcall ted_but_copy,edi
    @@:
    cmp ah,47 ;Ctrl+V
    jne @f
      stdcall ted_but_paste,edi
    @@:
    cmp ah,49 ;Ctrl+N
    jne @f
      call but_NewFile
    @@:
    cmp ah,199 ;Ctrl+Home
    jne @f
      call but_CtrlHome
    @@:
    ;jmp .end_key_fun
  .key_Ctrl:

  test ebx,KM_SHIFT ;Shift+...
  jz .key_Shift
    cmp ah,72 ;Shift+Up
    jne @f
      stdcall ted_sel_key_up,edi
    @@:
    cmp ah,75 ;Shift+Left
    jne @f
      stdcall ted_sel_key_left,edi
    @@:
    cmp ah,77 ;Shift+Right
    jne @f
      stdcall ted_sel_key_right,edi
    @@:
    cmp ah,80 ;Shift+Down
    jne @f
      stdcall ted_sel_key_down,edi
    @@:
    ;mov ted_drag_k,1 ;начинаем выделение от клавиатуры
    jmp .key_MoveCur
  .key_Shift:
;-------------------------------------------------
  cmp ah,72 ;178 ;Up
  jne @f
    stdcall draw_cursor_sumb,edi
    call CurMoveUp
    cmp dl,8
    jne .no_red_0
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp @f
    .no_red_0:
    stdcall draw_main_cursor,edi
    mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,80 ;177 ;Down
  jne @f
    stdcall draw_cursor_sumb,edi
    call CurMoveDown
    cmp dl,8
    jne .no_red_1
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp @f
    .no_red_1:
    stdcall draw_main_cursor,edi
    mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,75 ;176 ;Left
  jne @f
    stdcall draw_cursor_sumb,edi
    call CurMoveLeft
    cmp dl,8
    jne .no_red_2
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp @f
    .no_red_2:
    stdcall draw_main_cursor,edi
    mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,77 ;179 ;Right
  jne @f
    stdcall draw_cursor_sumb,edi
    call CurMoveRight
    cmp dl,8
    jne .no_red_3
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp @f
    .no_red_3:
    stdcall draw_main_cursor,edi
    mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,71 ;180 ;Home
  jne @f
    stdcall draw_cursor_sumb,edi
    call CurMoveX_FirstChar
    cmp dl,8
    jne .no_red_4
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp @f
    .no_red_4:
    stdcall draw_main_cursor,edi
    mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,79 ;181 ;End
  jne @f
    stdcall draw_cursor_sumb,edi
    call CurMoveX_LastChar
    cmp dl,8
    jne .no_red_5
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp @f
    .no_red_5:
    stdcall draw_main_cursor,edi
    mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  @@:
  cmp ah,73 ;184 ;PageUp
  jne @f
    call CurMovePageUp
    cmp dl,0
    je @f
    call OnInitialUpdate
    stdcall draw_main_win,edi
  @@:
  cmp ah,81 ;183 ;PageDown
  jne @f
    call CurMovePageDown
    cmp dl,0
    je @f
    call OnInitialUpdate
    stdcall draw_main_win,edi
    mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  @@:
;-------------------------------------------------
;    cmp ebx,0
;    jne .end_key_fun
  .key_MoveCur:

  cmp ah,69 ;[Pause Break]
  je .end_key_fun
  cmp ah,120 ;[Fn]
  je .end_key_fun
  cmp ah,0x80 ;if key up
  ja .end_key_fun

  cmp dword[table],0
  je @f
    stdcall KeyConvertToASCII, dword[table]
  @@:

  ;mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры

  xor edx,edx
  mov dl,ah
  add edx,EvChar ;add char to text
  cmp byte [edx],1
  jne @f
    mov ted_key_new,ah
    stdcall ted_set_undo,edi
    mov edx,ted_opt_ed_change_time+ted_opt_ed_move_cursor
    stdcall SelTextDel,edx
    cmp al,1
    jne .del
      mov edx,ted_opt_ed_move_cursor
    .del:
    cmp ted_cur_ins,1
    je .no_ins_mod
      stdcall TextDel,edi,ted_opt_ed_change_time
	  mov edx,ted_opt_ed_move_cursor
    .no_ins_mod:
	mov ecx,edi
	add ecx,ted_offs_key_new
    stdcall ted_text_add,edi,ecx,1,edx ;добавляем символ введенный с клавиатуры
    call draw_but_toolbar
    cmp ted_key_new,13
    jne .dr_m_win
      stdcall draw_main_win,edi
      jmp .dr_cur_l
    .dr_m_win:
      stdcall draw_cur_line,edi
    .dr_cur_l:
  @@:

  cmp ah,8 ;[<-]
  jne @f
    stdcall ted_set_undo,edi
    stdcall SelTextDel,ted_opt_ed_change_time
    cmp al,1
    je .del_one_b
      stdcall TextDel,edi,ted_opt_ed_change_time+ted_opt_ed_move_cursor
    .del_one_b:
    call draw_but_toolbar
    stdcall draw_main_win,edi
    jmp .end_key_fun
  @@:

  cmp ah,182 ;Delete
  jne @f
    stdcall ted_set_undo,edi
    stdcall SelTextDel,ted_opt_ed_change_time
    cmp al,1
    je .del_one_d
      stdcall TextDel,edi,ted_opt_ed_change_time
    .del_one_d:
    call draw_but_toolbar
    stdcall draw_main_win,edi
    jmp .end_key_fun
  @@:

  cmp ah,185 ;Ins
  jne @f
    stdcall draw_cursor_sumb,edi
    xor ted_cur_ins,1
    stdcall draw_main_cursor,edi
  @@:

  .end_key_fun:
  popad
  ret
endp

;output:
; al = 1 - can save
align 4
proc ted_can_save, edit:dword
  push ecx edi
  mov edi,dword[edit]

  mov ecx,ted_tim_ch
  sub ecx,ted_tim_undo
  mov al,1
  cmp ted_tim_ls,ecx
  jne @f
    dec al
  @@:
  pop edi ecx
  ret
endp

;output:
; al = 1 - selected
align 4
proc IsSel, edit:dword
  push ebx edi
  mov edi,dword[edit]
  xor al,al
  cmp ted_drag_m,1
  je @f
    mov al,1
    mov ebx,ted_sel_x0
    cmp ebx,ted_sel_x1
    jne @f
    mov ebx,ted_sel_y0
    cmp ebx,ted_sel_y1
    jne @f
    xor al,al
  @@:
  pop edi ebx
  ret
endp

align 4
proc SelNormalize, edit:dword
  push ecx esi edi
    mov edi,dword[edit]
    mov esi,edi
    add esi,ted_offs_sel
    add edi,ted_offs_seln
    mov ecx,sizeof.TexSelect
    rep movsb

	mov edi,dword[edit]

    jmp @f
    .swp_f:
      mov ecx,ted_seln_x0
      m2m ted_seln_x0,ted_seln_x1
      mov ted_seln_x1,ecx

      mov ecx,ted_seln_y0
      cmp ecx,ted_seln_y1 ;(sel_y0>sel_y1)
      jle .end_f
      m2m ted_seln_y0,ted_seln_y1
      mov ted_seln_y1,ecx

    jmp .end_f
    @@:

    mov ecx,ted_seln_y0
    cmp ecx,ted_seln_y1 ;(sel_y0>sel_y1)
    jg .swp_f

    cmp ecx,ted_seln_y1 ;(sel_y0==sel_y1)
    jne .end_f
      mov ecx,ted_seln_x0
      cmp ecx,ted_seln_x1 ;(sel_x0>sel_x1)
      jg .swp_f

    .end_f:
  pop edi esi ecx
  ret
endp

align 4
proc SelStart, edit:dword
  push ecx edi
    mov edi,dword[edit]

    mov ecx,ted_cur_x
    add ecx,[hScr.position]
    mov ted_sel_x0,ecx
    mov ted_sel_x1,ecx

    mov ecx,ted_cur_y
    add ecx,[wScr.position]
    mov ted_sel_y0,ecx
    mov ted_sel_y1,ecx
  pop edi ecx
  ret
endp

align 4
proc SelMove, edit:dword
  push ecx edi
    mov edi,dword[edit]

    mov ecx,ted_cur_x
    add ecx,[hScr.position]
    mov ted_sel_x1,ecx

    mov ecx,ted_cur_y
    add ecx,[wScr.position]
    mov ted_sel_y1,ecx
  pop edi ecx
  call draw_but_toolbar ;redraw toolbar (need to button Copy)
  ret
endp

;input:
; cl_al_mem = 1 - clear all memory
align 4
proc ted_clear, edit:dword, cl_al_mem:dword
  push edi
  mov edi,dword[edit]

  mov ted_cur_x,0
  mov ted_cur_y,0
  mov ted_tim_ch,0
  mov ted_tim_ls,0
  mov ted_tim_co,0
  mov ted_tim_undo,0
  mov ted_help_id,-1

  mov [wScr.position],0
  mov [wScr.max_area],100
  mov [wScr.redraw],1
  mov [hScr.position],0
  mov [hScr.max_area],100

  mov ted_sel_x0,0
  mov ted_sel_y0,0
  mov ted_sel_x1,0
  mov ted_sel_y1,0

  cmp dword[cl_al_mem],0
  je .exit

  push ecx edx
  mov ecx,sizeof.symbol*maxChars
  mov edx,ted_tex
  @@:
    mov byte [edx],0
    inc edx
  loop @b
  mov edx,ted_tex
  mov dword [edx+6],1
  pop edx ecx

  .exit:
  pop edi
  ret
endp

;--- out_reg = Col[ind_reg].Text[0] ---
macro ColToIndexOffset ind_reg,out_reg {
  mov out_reg,ind_reg
  imul out_reg,sizeof.TexColViv
  add out_reg,ted_key_words_data
}


align 4
proc InitColText, edit:dword
  pushad
  mov edi,dword[edit]

  mov ebx,dword[fn_col_option]
  copy_path ebx,fn_syntax_dir,file_name_rez,0x0
  copy_path file_name_rez,sys_path,file_name,0x0

  mov eax,70
  mov [run_file_70.func_n], 0
  mov [run_file_70.param1], 0
  mov [run_file_70.param2], 0
  mov ebx,ted_syntax_file_end
  sub ebx,ted_syntax_file
  mov [run_file_70.param3], ebx
  m2m [run_file_70.param4], ted_syntax_file
  mov [run_file_70.rezerv], 0
  mov [run_file_70.name], file_name
  mov ebx,run_file_70
  int 0x40

  mov ecx,0x100
  mov edx,FkPos
  @@:
    mov dword[edx],-1
    add edx,4
  loop @b

  ;init: ted_colors_text_count, ted_key_words_count, ...
  mov ted_colors_text_count,1
  mov ted_key_words_count,0
    mov ted_help_text_f1,0
  mov ted_help_id,-1 ;идентификатор слова для справки

  cmp eax,6
  je @f
  cmp eax,0
  je @f
    cmp ax,10
    jl .zifra_0_9
      mov al,'?'
      sub ax,48
    .zifra_0_9:
    add ax,48

    mov byte[msgbox_3.err],al
    stdcall [mb_create],msgbox_3,thread ;message: Can-t open color options file!
    jmp .no_colors
  @@:

  mov eax,edi ;сохраняем значение edi
  mov esi,ted_syntax_file
  add edi,ted_offs_count_colors
  mov ecx,9*4
  rep movsb
  mov edi,eax ;востанавливаем значение edi

  mov eax,ted_syntax_file
  add eax,32
  mov ted_text_colors,eax

  mov eax,ted_colors_text_count ;init: count_colors_text (offset to key words)
  add eax,8
  shl eax,2
  add eax,ted_syntax_file
  mov ted_key_words_data,eax

  mov ecx,ted_key_words_count ;init: FkPos (first key positions)
  xor eax,eax
  @@:
    ColToIndexOffset eax,edx
    xor ebx,ebx
    mov bl,byte[edx]
    shl bx,2
    mov esi,FkPos
    add esi,ebx
    cmp dword[esi],-1
    jne .no_ch_key
      mov dword[esi],eax
    .no_ch_key:
    inc eax
  loop @b

  ;init: ted_help_text_f1
  mov ecx,ted_key_words_count
  imul ecx,sizeof.TexColViv
  add ecx,ted_key_words_data
  mov ted_help_text_f1,ecx

  .no_colors:
  popad
  ret
endp

;input:
; ebx = file size
; edi = pointer to tedit struct
align 4
proc OpenFile
  push eax ;destination
  push ecx ;for cycle
  push edx ;source

    stdcall ted_clear,edi,0 ;чистим не всю память, потому что ниже будем ее заполнять новыми даными
    mov eax,ebx
    mov ecx,ebx
    add eax,2
    ConvertIndexToPointer eax
    mov edx,ted_tex
    add edx,ebx
  push ebx
    @@:
      mov ebx,[edx]
      mov byte [eax],bl
      mov dword [eax+2],ecx
      inc dword [eax+2]
      mov dword [eax+6],ecx
      add dword [eax+6],3
      ;mov byte[eax+1],0 ;col=0
      mov dword [eax+10],-1 ;tc=-1
      mov dword [eax+14],0 ;td=0

      cmp ecx,0
      je @f
      dec ecx
      dec edx
      sub eax,sizeof.symbol
      jmp @b
    @@:
  pop ebx
    add eax,2
    mov dword [eax],0 ; first sumbol 'perv=0'

    mov edx,ted_tex
    ; begining sumbol 'perv=0' 'next=2'
    mov dword [edx+2],0
    mov dword [edx+6],2

    add edx,sizeof.symbol
    mov dword [edx+6],0 ; last sumbol 'next=0'
    mov dword [edx+2],ebx ; last sumbol 'perv=last'
    inc dword [edx+2]

    mov edx,ebx
    inc edx ;2 = rezerv sumbols
    imul edx,sizeof.symbol
    add edx,ted_tex
    mov dword [edx+6],1 ; last sumbol 'next=1'

    @@: ;clear memory, need if before was open big file
      add edx,sizeof.symbol
      cmp edx,ted_tex_end
      jge @f
	mov dword[edx+10],0
	mov dword[edx+14],0
      jmp @b
    @@:

    call GetNumLines
    cmp eax,100
    jge @f
      mov eax,100
    @@:
    mov [wScr.max_area],eax
;    mov [wScr.redraw],1
  pop edx ecx eax

  call CmColored
  call draw_but_toolbar
  stdcall draw_main_win,edi
  ret
endp

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; edx = pointer to 'perv' visible symbol struct
align 4
IteratPerv:
  cmp ted_tim_undo,0
  je .else
  push ebx
  @@:
    call GetTexPervPos
    cmp edx,ted_tex
    je @f
    call SymbNotVis
    cmp bl,1
    je @b
    cmp byte[edx],10 ;пропуск символа с кодом 10
    je @b
  @@:
  pop ebx
  ret
  .else:
    call GetTexPervPos
    cmp edx,ted_tex
    je .endif
    cmp dword [edx+14],0
    jne .else
    cmp byte[edx],10 ;пропуск символа с кодом 10
    je .else
  .endif:
  ret


;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; edx = pointer to 'next' visible symbol struct
align 4
IteratNext:
  cmp ted_tim_undo,0
  je .else
  push ebx
  @@:
    call GetTexNextPos
    cmp edx,ted_tex_1
    jle @f
    call SymbNotVis
    cmp bl,1
    je @b
    cmp byte[edx],10 ;пропуск символа с кодом 10
    je @b
  @@:
  pop ebx
  ret
  .else:
    call GetTexNextPos
    cmp edx,ted_tex_1
    jle .endif
    cmp dword [edx+14],0
    jne .else
    cmp byte[edx],10 ;пропуск символа с кодом 10
    je .else
  .endif:
  ret

;input:
; bl = symbol end of select
; edx = pointer to symbol struct
; edi = pointer to tedit struct
align 4
ItPoNextUc:
  @@:
    cmp bl,byte[edx]
    je @f
    cmp edx,ted_tex_1
    jle @f
    call IteratNext
    jmp @b
  @@:
  call IteratNext
  ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
align 4
ItPervColorTag:
  @@:
    cmp byte[edx+1],0
    jne @f
    call IteratPerv
    cmp edx,ted_tex_1
    jle @f
    jmp @b
  @@:
  ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
align 4
ItNextColorTag:
  @@:
    call IteratNext
    cmp byte[edx+1],0
    jne @f
    cmp edx,ted_tex_1
    jle @f
    jmp @b
  @@:
  ;call IteratNext
  ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; bl = 1 if sumbol not visible
; (tex[i].td+ted_tim_undo<=ted_tim_ch && tex[i].td) || (tex[i].tc>ted_tim_ch-ted_tim_undo)
align 4
SymbNotVis:
  push eax

  xor bl,bl

  cmp dword [edx+14],0
  je @f
  mov eax,[edx+14] ;eax=tex[i].td
  add eax,ted_tim_undo
  cmp eax,ted_tim_ch
  jg @f
    mov bl,1
    pop eax
    ret
  @@:

  mov eax,ted_tim_ch
  sub eax,ted_tim_undo
  cmp [edx+10],eax
  jle @f
    or bl,1
  @@:

  pop eax
  ret


;input:
; text:dword - pointer to text string
; add_opt:dword - options
align 4
proc ted_text_add, edit:dword, text:dword, t_len:dword, add_opt:dword
  locals
    new_spc dd ? ;count new spaces
	new_lin dd ? ;count new lines
  endl
;использование регистров внутри функции:
;eax - позиция для вставки текста
;ebx - для временных нужд, длинна вставляемого текста
;ecx - для временных нужд
;edx - указатель на структуру символа
  pushad
  mov edi,dword[edit]
  mov esi,dword[text]

  call GetPos
  call GetTexPervPos
  call GetTexArrIndex ;eax=po_t

  mov dword[new_spc],0
  cmp ted_gp_opt,2
  je @f
    push eax ;c_sp=cur[cn].x+Scroller->XPos-StrLen(cur[cn].y+Scroller->YPos);
      mov eax,ted_cur_x
      add eax,[hScr.position]
      mov dword[new_spc],eax

      mov eax,ted_cur_y
      add eax,[wScr.position]
      call ted_strlen ;ebx = line len
      sub dword[new_spc],ebx
    pop eax
  @@:

  mov ebx,dword[t_len]

  mov dword[new_lin],0
  cmp ted_gp_opt,0
  jne @f
    push eax
      mov eax,ted_cur_y
      add eax,[wScr.position]
      inc eax
      mov dword[new_lin],eax

      call GetNumLines
      sub dword[new_lin],eax
    pop eax
  @@:

  mov edx,sizeof.symbol
  shl edx,1
  add edx,ted_tex
  @@: ;for(i=2;i<texMax;i++)
    cmp dword [edx+10],0 ;if(!tex[i].tc && !tex[i].td)
    jne .u1f
    cmp dword [edx+14],0
    jne .u1f
      test dword[add_opt],ted_opt_ed_change_time ;if(n_tim) ted_tim_ch++;
      jz .no_tim
	inc ted_tim_ch
      .no_tim:
      test dword[add_opt],ted_opt_ed_move_cursor
      jz .no_cur_mov
      cmp dword[new_lin],0 ;если есть добавочные строки, то курсор еще не двигаем
      jg .no_cur_mov
      cmp dword[new_spc],0 ;если нет добавочных пробелов, то курсор тоже не двигаем
      jg .no_cur_mov
	inc ted_cur_x ;move cursor
	;call GoToPos
	cmp byte [esi],13
	jne .no_cur_mov
	  mov ted_cur_x,0
	  inc ted_cur_y
      .no_cur_mov:

      mov ecx,ted_opt_ed_change_time
	  not ecx
	  and dword[add_opt],ecx ;n_tim=false;

      mov cl,byte [esi] ;tex[i].c=ta[ns];
      mov byte [edx],cl
      m2m dword [edx+10],ted_tim_ch ;tex[i].tc=ted_tim_ch;
      mov [edx+2],eax ;tex[i].perv=po_t;

      mov ecx,eax
      imul ecx,sizeof.symbol
      add ecx,ted_tex ; *** ecx = tex[po_t] ***
      add ecx,6   ; *** ecx = tex[po_t].next ***
      m2m dword [edx+6],dword [ecx] ;tex[i].next=tex[po_t].next;

      call GetTexArrIndex ;*** eax = i ***
      cmp eax,maxChars
      jge @f
      mov [ecx],eax ;tex[po_t].next=i; // ссылки перенаправляем
      mov ecx,[edx+6] ; *** ecx = tex[i].next ***
      imul ecx,sizeof.symbol
      add ecx,ted_tex ; *** ecx = tex[tex[i].next] ***
      mov [ecx+2],eax ;tex[tex[i].next].perv=i;

      cmp dword[new_lin],0 ;add lines or text
      jle .spc_add
	dec dword[new_lin]
	mov byte [edx],13
	jmp .u1f
      .spc_add:
      cmp dword[new_spc],0 ;add spaces or text
      jle .tex_add
	dec dword[new_spc]
	mov byte [edx],' '
	jmp .u1f
      .tex_add:
      inc esi
      dec ebx
    .u1f:
    add edx,sizeof.symbol
    cmp edx,ted_tex_end
    jge @f ;out of memory
    cmp ebx,0
    jne @b
  @@:

  call CmColored
  popad
  ret
endp


;input:
;  ecx = position to free insert cell
;  edx = pointer to sumbol, when insert
;  esi = pointer to added symbol
;  edi = pointer to tedit struct
;output:
;  ecx = position to inserted cell
align 4
CharAdd:

  .loop_b:
    cmp ecx,ted_tex_end
    jge .end_f
    cmp dword[ecx+10],0
    jne @f
      cmp dword[ecx+14],0
      je .loop_e
    @@:
    add ecx,sizeof.symbol
    jmp .loop_b
  .loop_e:

  push eax ebx
  mov eax,ted_tim_ch
  mov dword[ecx+10],eax
  mov al,byte[esi]
  mov byte[ecx],al

  call GetTexArrIndex ; *** eax=pos ***
  mov [ecx+2],eax ;tex[i].perv=pos;
  m2m dword[ecx+6],dword[edx+6] ;tex[i].next=tex[pos].next;

  push edx
    mov edx,ecx
    call GetTexArrIndex ; *** eax=i ***
  pop edx

  mov [edx+6],eax ;tex[pos].next=i; // ссылки перенаправляем
  mov ebx,[ecx+6]
  ConvertIndexToPointer ebx
  mov [ebx+2],eax ;tex[tex[i].next].perv=i; // ...
  pop ebx eax

  .end_f:
  call CmColored
  ret


;input:
; conv_table = pointert to convert table
; edi = pointer to tedit struct
;output:
; esi = count converted symbols
align 4
proc ConvertSelText
  locals
    conv_cou dd ?
  endl
  mov dword[conv_cou],0
  pushad

  stdcall IsSel,edi
  cmp al,0
  je .end_f
    stdcall ted_set_undo,edi
    stdcall SelNormalize,edi

    mov esi,ted_seln_x0
    mov ecx,ted_seln_y0
    call GetPosByParam
    mov eax,edx
    mov esi,ted_seln_x1
    mov ecx,ted_seln_y1
    call GetPosByParam
    ;call GetTexPervPos
    mov ebx,edx

    cmp eax,ebx
    je .end_f

    inc ted_tim_ch
    mov edx,eax ;i=p0;
    mov ecx,2
    ConvertIndexToPointer ecx
    @@:
      mov esi,[edx]
      and esi,0xff
      add esi,[conv_table] ;EvUpper
      cmp byte [esi],0
      je .no_change
	m2m dword [edx+14],ted_tim_ch
	call CharAdd ;b_pos=CharAdd(tex[i].c^32,i,false,b_pos);
	call GetTexNextPos ;go to added symbol
	inc dword[conv_cou]
      .no_change:

      call IteratNext
      cmp edx,ted_tex
      je @f 
      cmp edx,ebx
      je @f

      jmp @b
    @@:
    cmp dword[conv_cou],0
    jne @f
      dec ted_tim_ch
    @@:
  .end_f:
  popad
  mov esi,dword[conv_cou]
  ret
endp

;output:
; bl = 0 - no delete
; bl = 1 - delete
align 4
proc TextDel, edit:dword, del_opt:dword
  push cx edx edi
  mov edi,dword[edit]
  mov ebx,dword[del_opt]

  xor cl,cl
  test ebx,ted_opt_ed_move_cursor
  jz @f
    call CurMoveLeft
    cmp dl,0
    je .no_del
  @@:
  call GetPos
  cmp ted_gp_opt,1
  je .no_del
    test ebx,ted_opt_ed_change_time
    jz @f
      inc ted_tim_ch
    @@:
    m2m dword [edx+14], ted_tim_ch
    mov cl,1
  .no_del:
  mov bl,cl
  pop edi edx cx
  ret
endp

;input:
; edi = pointer to tedit struct
;output:
; al = 1 if delete
;description:
; Функция удаляет выделенный текст
align 4
proc SelTextDel, del_opt:dword
	push ebx ecx edx esi

	stdcall IsSel,edi
	cmp al,0
	je .end_f

		stdcall SelNormalize,edi

		mov esi,ted_seln_x1
		mov ecx,ted_seln_y1
		call GetPosByParam
		mov ebx,edx

		mov esi,ted_seln_x0
		mov ecx,ted_seln_y0
		call GetPosByParam

		test dword[del_opt],ted_opt_ed_change_time
		jz @f
			inc ted_tim_ch
		@@:
		cmp edx,ted_tex
		je @f
		cmp edx,ebx ;if(i==te)break;
		je @f
			m2m dword[edx+14],ted_tim_ch
			mov esi,ted_opt_ed_change_time
			not esi
			and dword[del_opt],esi ;n_tim=false;
			call IteratNext
			jmp @b
		@@:
		test dword[del_opt],ted_opt_ed_change_time
		jz @f
			dec ted_tim_ch
			xor al,al
		@@:
		test dword[del_opt],ted_opt_ed_change_time
		jnz @f
			mov ecx,ted_seln_x0
			mov edx,ted_seln_y0
			call GoToPos
			mov ted_sel_x0,0
			mov ted_sel_y0,0
			mov ted_sel_x1,0
			mov ted_sel_y1,0
		@@:
	.end_f:
	pop esi edx ecx ebx
	ret
endp


;input:
; eax = pointer to begin select
; ebx = pointer to end select
; edi = pointer to tedit struct
align 4
Revers:
  cmp eax,ebx
  jne @f
    ret
  @@:

  push ecx edx

  mov edx,ted_tex_1
  cmp edx,ebx ;if(p1==1)p1=tex[1].perv;
  jne @f
    call GetTexPervPos
    mov ebx,edx
  @@:

  push esi
    mov edx,[eax+2] ; *** edx = tex[p0].perv ***
    ConvertIndexToPointer edx
    add edx,6
    mov ecx,[edx] ;tmp = tex[tex[p0].perv].next;

    mov esi,[ebx+6] ; *** esi = tex[p1].next ***
    ConvertIndexToPointer esi
    add esi,2
    m2m dword[edx],dword[esi] ;tex[tex[p0].perv].next = tex[tex[p1].next].perv;

    mov [esi],ecx ;tex[tex[p1].next].perv = tmp;
  pop esi

  mov ecx,[eax+2] ;tmp = tex[p0].perv;
  m2m dword[eax+2],dword[ebx+6] ;tex[p0].perv = tex[p1].next;
  mov [ebx+6],ecx ;tex[p1].next = tmp;

  mov edx,eax ;i=p0;
  @@:
    mov ecx,[edx+6] ;tmp = tex[i].next;
    m2m dword[edx+6],dword[edx+2] ;tex[i].next = tex[i].perv;
    mov [edx+2],ecx ;tex[i].perv = tmp;
    cmp edx,ebx ;if(i==p1)break;
    je @f
; ---
;cmp edx,ted_tex
;je @f
; ---
    mov edx,ecx ;i = tmp;
    ConvertIndexToPointer edx
    jmp @b
  @@:
  pop edx ecx
  call CmColored
  ret


;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 2 if move up
; dl = 8 if scroll move up
align 4
CurMoveUp:
  cmp ted_cur_y,0
  je @f
    dec ted_cur_y
    mov dl,2
    ret
  @@:
  cmp [wScr.position],0
  je @f
    dec [wScr.position]
    mov dl,8
    ret
  @@:
  mov dl,0
  ret

;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 2 if move down
; dl = 8 if scroll move down
align 4
CurMoveDown:
  push eax ebx
  mov dl,0
  mov ebx,ted_scr_w
  mov eax,dword[ebx+sb_offs_cur_area]
  dec eax
  cmp ted_cur_y,eax
  jge @f
    inc ted_cur_y
    mov dl,2
    jmp .ret_f
  @@:
  mov eax,ted_cur_y
  add eax,dword[ebx+sb_offs_position]
  inc eax
  cmp dword[ebx+sb_offs_max_area],eax
  jle @f
    inc dword[ebx+sb_offs_position]
    mov dl,8
  @@:
  .ret_f:
  pop ebx eax
  ret


;output:
; dl = 0 not move
; dl = 1 if move up
align 4
CurMovePageUp:
  push eax
  mov eax,[wScr.cur_area]
  xor dl,dl
  cmp eax,[wScr.position]
  jg @f
    sub [wScr.position],eax
    mov dl,1
  @@:
  cmp [wScr.position],0
  je @f
  cmp dl,1
  je @f
    mov [wScr.position],0
    mov dl,1
  @@:
  pop eax
  ret

;input:
; edi = pointer to tedit struct
align 4
CurMovePageDown:
	push eax ebx ecx
	mov ecx,ted_scr_w

	xor dl,dl
	mov eax,dword[ecx+sb_offs_max_area]
	sub eax,dword[ecx+sb_offs_cur_area]
	cmp dword[ecx+sb_offs_position],eax
	jge @f
		mov ebx,dword[ecx+sb_offs_cur_area]
		add dword[ecx+sb_offs_position],ebx
		mov dl,1
		mov dword[ecx+sb_offs_redraw],1
		cmp dword[ecx+sb_offs_position],eax
		jle @f
			mov dword[ecx+sb_offs_position],eax
	@@:
	pop ecx ebx eax
	ret

;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 1 if move left
; dl = 3 if move left and up
; dl = 8 if scroll move up
align 4
CurMoveLeft:
  cmp ted_cur_x,0
  je @f
    dec ted_cur_x
    mov dl,1
    ret
  @@:
  cmp [hScr.position],0
  je @f
    dec [hScr.position]
    mov dl,8
    ret
  @@:
  cmp ted_cur_y,0
  jne @f
    mov dl,0
    cmp [wScr.position],0
    je .ret_f
      dec [wScr.position]
      call OnInitialUpdate
      call CurMoveX_LastChar
      mov dl,8
    jmp .ret_f
  @@:
  cmp ted_cur_y,0
  je @f
  dec ted_cur_y
  call CurMoveX_LastChar
  cmp dl,8
  je .ret_f
  mov dl,3
  ret
  @@:
  mov dl,0
  .ret_f:
  ret

;input:
; edi = pointer to tedit struct
align 4
CurMoveRight:
  push eax
  xor dl,dl
  mov eax,[hScr.cur_area]
  cmp ted_cur_x,eax
  jge @f
    inc ted_cur_x
    mov dl,1
    jmp .ret_f
  @@:
    inc [hScr.position]
    mov dl,8
  .ret_f:
  pop eax
  ret

;input:
; edi = pointer to tedit struct
align 4
CurMoveX_LastChar:
;[hScr.position]
;[hScr.cur_area]
;dl-???
  push eax ebx
  mov eax,ted_cur_y
  add eax,[wScr.position]
  call ted_strlen
  xor dl,dl

  cmp ebx,[hScr.position]
  jge @f
    mov dl,8
    mov [hScr.position],ebx
  @@:
  sub ebx,[hScr.position]

  cmp ebx,[hScr.cur_area]
  jle @f ; b---[---]---e
    add [hScr.position],ebx
    mov ebx,[hScr.cur_area]
    sub [hScr.position],ebx
    mov dl,8
  @@:
  mov ted_cur_x,ebx
  pop ebx eax
  ret

;input:
; edi = pointer to tedit struct
;output:
; dl = 0 not move
; dl = 1 move cursor
; dl = 8 move cursor and scroll
align 4
CurMoveX_FirstChar:
  xor dl,dl
  cmp ted_cur_x,0
  je @f
  mov ted_cur_x,0
  mov dl,1
  @@:
  cmp [hScr.position],0
  je @f
    mov [hScr.position],0
    mov dl,8
  @@:
  ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; eax = array index
align 4
GetTexArrIndex:
  push ecx edx

  mov eax,edx
  sub eax,ted_tex
  xor edx,edx
  mov ecx,sizeof.symbol
  div ecx

  pop edx ecx
  ret

;input:
; edx = pointer to symbol struct
; edi = pointer to tedit struct
;output:
; edx = pointer to 'perv' struct
align 4
GetTexPervPos:
  push dword [edx+2]
  pop edx
  imul edx,sizeof.symbol
  add edx,ted_tex
  ret

;input:
; edx = pointer to symbol struct
;output:
; edx = pointer to 'next' symbol struct
align 4
GetTexNextPos:
  push dword [edx+6]
  pop edx
  imul edx,sizeof.symbol
  add edx,ted_tex
  ret

;input:
; edi = pointer to tedit struct
;output:
; edx = symbol under cursor
; ted_gp_opt = 1,2
; edx = tex[1].perv if error
; ted_gp_opt = 0
align 4
GetPos:
  push esi ecx
    mov esi,ted_cur_x
    add esi,[hScr.position]
    mov ecx,ted_cur_y
    add ecx,[wScr.position]
    call GetPosByParam
  pop ecx esi
  ret


;input:
; esi = XPos
; ecx = YPos
; edi = pointer to tedit struct
;output:
; edx = symbol under cursor
; ted_gp_opt = 1 if found text line
; ted_gp_opt = 2 if found text line and column
; edx = tex[1] if error
; ted_gp_opt = 0 if text no found
align 4
GetPosByParam:
  push eax ;Row
  push ebx ;Col

  xor eax,eax
  xor ebx,ebx
  mov ted_gp_opt,0
  mov edx,ted_tex
  @@:
    call IteratNext
    cmp edx,ted_tex_1
    jle @f 
    cmp ebx,esi
    jne .u1_0 ;Col <> ted_cur_x
      mov ted_gp_opt,1
      cmp eax,ecx
      jge @f ; Row >= ted_cur_y
    .u1_0:
    mov ted_gp_opt,0
    inc ebx
    cmp byte [edx],13
    jne @b
    cmp eax,ecx
    jge @f ; Row >= ted_cur_y
    inc eax
    xor ebx,ebx
    jmp @b
  @@:
  cmp eax,ecx
  jne @f ; Row = ted_cur_y
    inc ted_gp_opt
  @@:
  cmp ted_gp_opt,0
  jne @f
    mov edx,ted_tex_1
    ;call GetTexPervPos
  @@:
  pop ebx eax
  ret


;input:
; eax = Row
; edi = pointer to tedit struct
;output:
; ebx = str len
align 4
ted_strlen:
  push edx ecx
  ;ecx = Row, from cycle

  xor ebx,ebx
  xor ecx,ecx
  mov edx,ted_tex
  @@:
    call IteratNext
    cmp edx,ted_tex_1
    jle @f 
    inc ebx
    cmp byte [edx],13
    jne @b
    dec ebx ;lenght minus 1 sumbol to paragraph
    cmp eax,ecx
    je @f
    xor ebx,ebx
    inc ecx
    jmp @b
  @@:

  cmp eax,ecx
  je @f
    xor ebx,ebx
  @@:

  pop ecx edx
  ret


;input:
; edx = symbol position
; edi = pointer to tedit struct
;output:
; eax = number of line
; ebx = symbol position in line
align 4
GetTexCoords:
  push edx
  xor eax,eax
  xor ebx,ebx
  @@:
    call IteratPerv

    cmp eax,0
    jne .no_col_mov
    inc ebx
    .no_col_mov:

    cmp edx,ted_tex_1
    jle @f
    cmp byte [edx],13
    jne @b
    inc eax
    jmp @b
  @@:
  dec ebx
  pop edx
  ret

;input:
; edi = pointer to tedit struct
;output:
; eax = num lines
align 4
GetNumLines:
  push edx
  mov eax,1
  mov edx,ted_tex
  @@:
    call IteratNext
    cmp edx,ted_tex_1
    jle @f
    cmp byte [edx],13
    jne @b
    inc eax
    jmp @b
  @@:
;...
;dec eax
  pop edx
  ret


align 4
proc ted_set_undo, edit:dword
  push edi
  mov edi,dword[edit]

  mov ted_drag_k,0 ;заканчиваем выделение от клавиатуры
  cmp ted_tim_undo,1
  jl .no_work

  push eax ebx edx
  mov edx,ted_tex
  call GetTexNextPos ;long i=tex[0].next;
  mov eax,ted_tim_undo
  sub ted_tim_ch,eax ;ted_tim_ch-=ted_tim_undo;
  mov eax,ted_tim_ch
  cmp ted_tim_ls,eax ;if(ted_tim_ls>ted_tim_ch)
  jle @f
    mov ted_tim_ls,0
  @@:
    cmp edx,ted_tex
    je @f

    ;if(tex[i].tc>ted_tim_ch){ // если создание символа было отменено
    cmp [edx+10],eax
    jle .no_u1
      mov dword [edx+10],0
      mov dword [edx+14],0

      mov ebx,[edx+2]
      imul ebx,sizeof.symbol
      add ebx,ted_tex;.next
      m2m dword [ebx+6],dword [edx+6] ;tex[tex[i].perv].next=tex[i].next;

      mov ebx,[edx+6]
      imul ebx,sizeof.symbol
      add ebx,ted_tex;.perv
      m2m dword [ebx+2],dword [edx+2] ;tex[tex[i].next].perv=tex[i].perv;

    .no_u1:

    ;else if(tex[i].td>ted_tim_ch) tex[i].td=0; // если удаление символа было отменено
    cmp [edx+14],eax
    jle .no_u2
      mov dword [edx+14],0
    .no_u2:

    call GetTexNextPos
    jmp @b
  @@:
  mov ted_tim_undo,0
  mov eax,ted_tim_co
  cmp ted_tim_ch,eax
  jge @f
    mov ted_tim_co,0
  @@:
  pop edx ebx eax
  .no_work:
  pop edi
  ret
endp

;input:
; ecx = Col
; edx = Row
; edi = pointer to tedit struct
align 4
GoToPos:
  mov ted_cur_x,ecx
  sub edx,[wScr.position]

  cmp edx,[wScr.cur_area] ;ted_cur_y > [.cur_area]
  jl @f
    push ebx
    mov ebx,edx
    sub ebx,[wScr.cur_area]
    inc ebx
    add [wScr.position],ebx
    sub edx,ebx
    pop ebx
    ; ??? redrav
  @@:
  mov ted_cur_y,edx
  ret

;input:
; edi = pointer to tedit struct
align 4
CmColored:
  push eax edx
  mov eax,ted_tim_ch
  sub eax,ted_tim_undo
  mov ted_tim_co,eax
  mov edx,ted_tex
  @@:
    call IteratNext
    cmp edx,ted_tex_1
    jle @f
    mov byte[edx+1],0
    jmp @b
  @@:

  cmp ted_key_words_count,1
  jl .no_colors
  mov edx,ted_tex
  @@:
    call TextFSColor
    cmp edx,ted_tex_1
    jle .no_colors
    jmp @b
  .no_colors:
  pop edx eax
  ret


;input:
; edx = pointer to start symbol
; edi = pointer to tedit struct
;output:
; edx = pointer to next symbol
;description:
; Функция для поиска и выделения подсвеченых слов
align 4
proc TextFSColor
locals
  begPos dd ? ;начальная позиция
  endPos dd ? ;конечная позиция
  find db ? ;найдено / не найдено
  f_color db ? ;индекс цвета найденого слова
endl
  push eax ebx ecx esi
;eax = word_n текущий номер (позиция) проверяемого слова в списке
;ebx = для разных целей
;ecx = l_pos последний номер (позиция) подходящего слова в списке
;esi = для разных целей, номер проверяемого символа в слове
  mov dword[begPos],1
  mov dword[endPos],1
  mov byte[find],0
  mov byte[f_color],1
  @@:
    call IteratNext
    cmp edx,ted_tex_1
    jle @f

    xor eax,eax
    mov al,byte[edx]
    shl ax,2 ;eax*=4
    add eax,FkPos
    mov eax,dword[eax]
    cmp eax,0
    jl @b ;if( (word_n=FkPos[(unsigned char)tex[i].c])>-1 ){

    mov ecx,eax
    ;while(l_pos<ted_key_words_count && Col[l_pos].Text[0]==Col[word_n].Text[0])
    .wh_1b:
      cmp ecx,ted_key_words_count
      jge .wh_1e
      ColToIndexOffset ecx,esi
      mov bl,byte[esi]
      ColToIndexOffset eax,esi
      cmp bl,byte[esi]
      jne .wh_1e
	inc ecx
      jmp .wh_1b
    .wh_1e:

    mov dword[begPos],edx ;bP=i;
    mov esi,1

    .wh_2b: ;while(1){
      call IteratNext

      ;while(l_pos>word_n && Col[l_pos-1].Text[pos]!=tex[i].c)
      .wh_3b:
	cmp ecx,eax
	jle .wh_3e
	dec ecx
	ColToIndexOffset ecx,ebx
	inc ecx
	;cmp byte[ebx+esi],byte[edx]
	mov bl,byte[ebx+esi]
	cmp bl,byte[edx]
	je .wh_3e
	  dec ecx
	jmp .wh_3b
      .wh_3e:

      ColToIndexOffset eax,ebx
      cmp byte[ebx+esi],0
      jne .if_0 ;if(Col[word_n].Text[pos]==0){
	mov dword[endPos],edx ;eP=i;
	ColToIndexOffset eax,ebx
	mov bl,byte[ebx+MAX_COLOR_WORD_LEN+6]
	mov byte[f_color],bl ;f_color=Col[word_n].color;

mov byte[find],1
	  ColToIndexOffset eax,ebx ;... ebx = Col[word_n]
	  mov bl,byte[ebx+MAX_COLOR_WORD_LEN+4]
	  cmp bl,0 ;if(Col[word_n].wwo)
	  je .if_2n
	    push edx
	    mov edx,dword[begPos]
     call IteratPerv

	    btr bx,0 ;1-1
	    jae .if_3e ;if(Col[word_n].wwo&1)
	      ;u1= !(isalnum(cont_s)||cont_s=='_')
	      call isalnum
	      jae .if_3e
		mov byte[find],0
	    .if_3e:

	    btr bx,3 ;4-1
	    jae .if_4e ;if(Col[word_n].wwo&8)
	      ;u1= !isalpha(cont_s);
	      call isalpha
	      jae .if_4e
		mov byte[find],0
	    .if_4e:

	    mov edx,dword[endPos]
;     call IteratNext

	    btr bx,1 ;2-1
	    jae .if_5e ;if(Col[word_n].wwo&2)
	      ;u1= !(isalnum(cont_s)||cont_s=='_')
	      call isalnum
	      jae .if_5e
		mov byte[find],0
	    .if_5e:

	    btr bx,4 ;5-1
	    jae .if_6e ;if(Col[word_n].wwo&16)
	      ;u1= !isalpha(cont_s);
	      call isalpha
	      jae .if_6e
		mov byte[find],0
	    .if_6e:

	    btr bx,2 ;3-1
	    jae .if_7e ;if(Col[word_n].wwo&4)
	    ColToIndexOffset eax,ebx
	    mov bl,byte[ebx+MAX_COLOR_WORD_LEN+5]
	    call ItPoNextUc
       cmp edx,ted_tex_1
       jle .if_7e
       mov dword[endPos],edx
	    .if_7e:

	    pop edx
	  .if_2n:
;                 if(i!=1){ // не конец документа
;                   cont_s=tex[eP].c;
;                   if(Col[word_n].wwo&2) u2= !(isalnum(cont_s)||cont_s=='_');  // не букв.-числ. символ
;                   if(u2 && Col[word_n].wwo&16) u2= !isalpha(cont_s); // не числ. символ
;                   if(Col[word_n].wwo&4) eP=ItPoNextUc(eP,Col[word_n].endc);

	cmp eax,ecx
	je .wh_2e ;if(word_n==l_pos) break; // do double - если слово точно последнее
      .if_0:

      cmp edx,ted_tex_1
      jle .wh_2e ;if(i==1) break;

      ;while(l_pos>word_n && Col[word_n].Text[pos]!=tex[i].c)
      .wh_4b:
	cmp ecx,eax
	jle .wh_4e
	ColToIndexOffset eax,ebx
	;cmp byte[ebx+esi],byte[edx]
	mov bl,byte[ebx+esi]
	cmp bl,byte[edx]
	je .wh_4e
	  inc eax
	jmp .wh_4b
      .wh_4e:

      cmp eax,ecx
      je .wh_2e;if(word_n==l_pos) break;
      inc esi ;pos++;
      jmp .wh_2b
    .wh_2e:

    cmp byte[find],1 ;if(fnd)break;
    je @f
    mov edx,dword[begPos];i=bP;
    jmp @b
  @@:

  cmp byte[find],1
  jne .if_1e ;if(fnd){ // выделение найденого текста
;    if(!mode_sf1 || (mode_sf1 && strlen(Col[word_n].f1->c_str())>0)){
    mov eax,dword[begPos]
    mov bl,byte[f_color]
    mov byte[eax+1],bl ;tex[bP].col=f_color;
    mov eax,dword[endPos]
    mov byte[eax+1],0xff ;tex[eP].col=255;
;    return ItPoPerv(eP); // возвращаем позицию конца вхождения
    mov edx,dword[endPos]
    call GetTexPervPos
    jmp @f
  .if_1e:
    mov edx,ted_tex
  @@:

  pop esi ecx ebx eax
  ret
endp

;input:
; edx = pointer to char (byte)
;output:
; cf=1 if symbol is...
align 4
tab_all_num db 0,0,0,0,0,0,11111111b,11b,11111110b,0xff,0xff,111b,11111110b,0xff,0xff,111b,0,0,0,0,0,0,0,0;,0,0,0,0,0,0,0,0 - tab_alpha_0,0,0,0,0,0,0,0
tab_alpha db 0,0,0,0,0,0,0,0,11111110b,0xff,0xff,111b,11111110b,0xff,0xff,111b,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

align 4
isalnum:
  push eax ebx
  mov al,byte[edx] ;al=offset
  shr al,3
  and eax,11111b
  mov ebx,tab_all_num
  add ebx,eax
  mov ah,byte[ebx]
  mov al,byte[edx] ;al=bit
  and ax,111b
  mov bx,word[ebx]
  btr bx,ax
  pop ebx eax
  ret
align 4
isalpha:
  push eax ebx
  mov al,byte[edx] ;al=offset
  shr al,3
  and eax,11111b
  mov ebx,tab_alpha
  add ebx,eax
  mov ah,byte[ebx]
  mov al,byte[edx] ;al=bit
  and ax,111b
  mov bx,word[ebx]
  btr bx,ax
  pop ebx eax
  ret

align 4
proc ted_show_help_f1, edit:dword
  push eax edx edi
  mov edi,dword[edit]

  call GetPos
  push edx
    call ItNextColorTag
    mov eax,edx
  pop edx
  call ItPervColorTag

  cmp eax,ted_tex
  jle @f
  cmp edx,ted_tex_1
  jle @f
    stdcall FindHelpId,eax
  @@:
  ;stdcall draw_main_cursor,edi
  stdcall draw_help_f1
  pop edi edx eax
  ret 
endp

;input:
; edx = position begin 'symbol' struct
; edi = pointer to tedit struct
; end_pos = position end 'symbol' struct
align 4
proc FindHelpId, end_pos:dword
; ecx = word_n
; ebx = l_pos
  mov ted_help_id,-1

  push ebx ecx
    xor ebx,ebx
    mov bl,byte[edx]
    shl bx,2 ;ebx*=4
    add ebx,FkPos
    mov ecx,dword[ebx]
    cmp ecx,0
    jl .if_0e ;if( (word_n=FkPos[(unsigned char)tf[0]])>-1 ){
      push esi eax
      mov ebx,ecx ;l_pos=word_n;
      ColToIndexOffset ecx,esi
      push cx
      mov cl,byte[esi]
      @@:
	cmp ebx,ted_key_words_count ;while(l_pos<ted_key_words_count)
	jge @f
	;ColToIndexOffset ecx,esi
	ColToIndexOffset ebx,eax
	cmp cl,byte[eax] ;&& Col[l_pos].Text[0]==Col[word_n].Text[0])
	jne @f
	  inc ebx ;l_pos++;
	  jmp @b
      @@:
      pop cx
      call IteratNext ;pos=1;
      mov esi,1
      @@:
	push dx
	push word[edx]
	pop dx 
	  .wh_0b:
	    cmp ebx,ecx ;while(l_pos>word_n
	    jle .wh_0e
	    dec ebx
	    ColToIndexOffset ebx,eax
	    inc ebx
	    cmp byte[eax+esi],dl ;&& Col[l_pos-1].Text[pos]!=tf[i])
	    je .wh_0e
	      dec ebx ;l_pos--;
	    jmp .wh_0b
	  .wh_0e:

	  .wh_1b:
	    cmp ebx,ecx ;while(l_pos>word_n
	    jle .wh_1e
	    ColToIndexOffset ecx,eax
	    cmp byte[eax+esi],dl
	    je .wh_1e
	      inc ecx ;word_n++;
	    jmp .wh_1b
	  .wh_1e:
	pop dx

	cmp ecx,ebx ;if(word_n==l_pos) break;
	je @f
	call IteratNext ;pos++;
	cmp edx,dword[end_pos] ;for(...;i<strlen;...)
	je @f ;jge
	inc esi
	jmp @b
      @@:
      pop eax esi

      mov ted_help_id,ecx
      ;return word_n;

    .if_0e:
  pop ecx ebx
  ret
endp

;-----------------------------------------------------------------------------
;функция для выделения памяти
;input:
; ecx = size data
;otput:
; eax = pointer to memory
align 4
mem_Alloc:
  push ebx
  mov eax,68
  mov ebx,12
  int 0x40
  pop ebx
  ret
;-----------------------------------------------------------------------------
;функция для освобождения памяти
;input:
; ecx = pointer to memory
align 4
mem_Free:
  push eax ebx
  cmp ecx,0
  jz @f
    mov eax,68
    mov ebx,13
    int 0x40
  @@:
  pop ebx eax
  ret
