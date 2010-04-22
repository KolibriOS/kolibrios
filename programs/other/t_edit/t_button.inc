align 4
but_NewFile:
  push eax ebx
  stdcall ted_can_save, tedit0
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
  stdcall ted_clear, tedit0,1
  call draw_but_toolbar
  stdcall draw_main_win, tedit0
  ret

align 4
On_SaveAndNewFile:
  push edi
  mov edi, tedit0

  stdcall but_SaveFile,edi
  cmp ted_err_save,0
  jne @f
    call On_NewFile
  @@:
  pop edi
  ret

align 4
but_no_msg_OpenFile: ;функция открытия файла через без окона сообщения
  stdcall ted_on_open_file, tedit0
  ret


;функция открытия файла с возможным окном сообщения (о возможности отменить открытие)
align 4
but_OpenFile:
  push eax
  stdcall ted_can_save, tedit0
  cmp al,1
  jne @f
    stdcall [mb_create],msgbox_5,thread ;message: save changes in file?
    stdcall [mb_setfunctions],msgbox_5_funct
    jmp .ret_f
  @@:
  stdcall ted_on_open_file, tedit0
  .ret_f:
  pop eax
  ret

align 4
proc ted_on_open_file, edit:dword ;функция открытия файла
  push eax ebx edi
  mov edi,dword[edit]

  mov eax,70
  mov [run_file_70.func_n], 0
  mov [run_file_70.param1], 0
  mov [run_file_70.param2], 0
  mov [run_file_70.param3], maxChars-2 ;sizeof.symbol
  m2m [run_file_70.param4], ted_tex
  mov [run_file_70.rezerv], 0
  push dword[edit1.text]
  pop dword[run_file_70.name]
  mov ebx,run_file_70
  int 0x40

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
    call OpenFile
    cmp ebx,maxChars-2
    jl .ret_f
    stdcall [mb_create],msgbox_1,thread
  .ret_f:
  pop edi ebx eax
  ret
endp


align 4
proc but_SaveFile, edit:dword
  pushad

  stdcall ted_can_save,edi
  cmp al,0
  je .no_save

  mov edi,dword[edit]

  mov ecx,maxChars
  call mem_Alloc
  push eax

  mov edx,ted_tex
  xor ecx,ecx
  @@:
    call IteratNext
    cmp edx,ted_tex_1
    jle @f ;edx = ted_tex or edx = ted_tex+sizeof.symbol
    mov bl,[edx]
    mov byte[eax],bl
    inc eax
    inc ecx
    jmp @b
  @@:

  cmp ecx,0
  je @f
    pop eax
    mov [run_file_70.param4],eax
    push eax
    mov eax,70
    mov [run_file_70.func_n], 2
    mov [run_file_70.param1], 0
    mov [run_file_70.param2], 0
    mov [run_file_70.param3], ecx
    mov [run_file_70.rezerv], 0
    push [edit1.text]
    pop [run_file_70.name]
    mov ebx,run_file_70
    int 0x40

    mov ted_err_save,ax

    cmp eax,0
    je .no_msg
;    cmp eax,6
;    je @f
      cmp ax,10
      jl .zifra_0_9
	mov al,'?'
	sub ax,48
      .zifra_0_9:
      add ax,48

      mov byte[msgbox_6.err],al
      stdcall [mb_create],msgbox_6,thread ;message: Can-t save text file!
      jmp @f
    .no_msg:
    m2m ted_tim_ls,ted_tim_ch
  @@:

  pop ecx
  call mem_Free
  .no_save:
  popad
  ret
endp


align 4
proc but_FindText, edit:dword
  pushad
    mov edi,dword[edit]

    call GetPos
    mov eax,[edit2.text]
    mov bl,byte[eax]
    @@:
      call GetFindRezult
      cmp bh,1
      je @f ; find

      call IteratNext
      cmp edx,ted_tex_1
      jle @f
      jmp @b
    @@:
    cmp bh,0
    je @f
      call GetTexCoords
      inc ebx ;move cursor right
      mov ted_sel_x1,ebx
      mov ted_sel_y1,eax
      mov edx,eax
      mov ecx,ebx
      call GoToPos
      mov edx,esi ;esi было установлео в GetFindRezult
      call GetTexCoords
      mov ted_sel_x0,ebx
      mov ted_sel_y0,eax
      stdcall draw_main_win,edi
      jmp .end_find
    @@:
      stdcall [mb_create],msgbox_7,thread ;message: Can not find text
    .end_find:
  popad
  ret
endp

;input:
; eax - text need find
; bl - first symbol to find
; edx - first symbol pointer
; edi - pointer to tedit struct
;output:
; bh - rezult
; edx - last text position (if find sucess)
; esi - first symbol pointer
;description:
; Функция проверяет совпадает ли текст в буфере eax
; с текстом редактора по указателю edx.
; Стандартные функции (напр. strcmp) тут не подойдут, потому что
; в памяти редактора текст содержится не в виде ascii строк.
align 4
GetFindRezult:
  push eax
    mov bh,1
    mov esi,edx ;copy edx
    @@:
      cmp byte[edx],bl
      jne .no_text

      inc eax ;*** get next symbol (in find text) ***
      mov bl,byte[eax]
      cmp bl,0
      je @f ;end of find text

      call IteratNext ;*** get next symbol (in editor text) ***
      cmp edx,ted_tex_1
      jle @f ;end of editor text

      jmp @b
      .no_text:
	xor bh,bh
	mov edx,esi ;restore edx
    @@:
  pop eax
  mov bl,byte[eax] ;restore bl
  ret

align 4
proc ted_but_open_syntax, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_panel_id,TE_PANEL_SYNTAX
  jne @f
    stdcall dword[tl_node_get_data], tree1
    pop dword[fn_col_option]
    stdcall InitColText,edi
    call CmColored
    stdcall draw_main_win,edi
  @@:
  pop edi
  ret
endp

align 4
proc ted_but_select_word, edit:dword
  pushad
  mov edi,dword[edit]

  call GetPos
  push edx
    call ItPervColorTag
    cmp edx,ted_tex_1
    jle @f
      call GetTexCoords
      mov ted_sel_x0,ebx
      mov ted_sel_y0,eax
    @@:
  pop edx
    call ItNextColorTag
    cmp edx,ted_tex_1
    jle @f
      call GetTexCoords
      mov ted_sel_x1,ebx
      mov ted_sel_y1,eax
    @@:

  call draw_but_toolbar
  stdcall draw_main_win,edi
  popad
  ret
endp

align 4
proc ted_but_cut, edit:dword
  push edi
  mov edi,dword[edit]

  stdcall ted_but_copy,edi
  stdcall ted_set_undo,edi
  stdcall SelTextDel,ted_opt_ed_change_time

  cmp al,1
  jne @f
    call draw_but_toolbar
    stdcall draw_main_win,edi
  @@:
  pop edi
  ret
endp

;output:
; al = 1 if copy text
align 4
proc ted_but_copy, edit:dword
  pushad
  mov edi,dword[edit]

  stdcall IsSel,edi
  cmp al,0
  je .end_f ;if not selected text
  stdcall SelNormalize,edi

  mov esi,ted_seln_x1
  mov ecx,ted_seln_y1
  call GetPosByParam
  mov ebx,edx
  mov esi,ted_seln_x0
  mov ecx,ted_seln_y0
  call GetPosByParam
  mov esi,ebx

  xor ecx,ecx
  mov ebx,buf
  ;mov edx,ted_tex
  @@:
    cmp edx,ted_tex_1 ;end of file
    jle @f
    cmp edx,esi ;end of select
    je @f
    inc ecx
    cmp ecx,BUF_SIZE ;owerflow bufer
    je @f

    mov al,byte[edx]
    mov byte[ebx],al
    inc ebx

    call IteratNext
    jmp @b
  @@:
  add ecx,buf
  mov byte[ebx],0

  cmp ecx,0
  je .end_f
    call draw_but_toolbar
    stdcall draw_bufer,edi
  .end_f:
  popad
  ret
endp


align 4
proc ted_but_paste, edit:dword
  push eax ebx esi edi
  mov edi,dword[edit]

    mov esi,buf
    call strlen
    cmp eax,1
    jl @f
      stdcall ted_set_undo,edi
      mov esi,eax
      mov ebx,ted_opt_ed_change_time+ted_opt_ed_move_cursor
      stdcall SelTextDel,ebx
	  cmp al,1
	  jne .del
	    mov ebx,ted_opt_ed_move_cursor
	  .del:
      stdcall ted_text_add,edi,buf,esi,ebx
      call draw_but_toolbar
      stdcall draw_main_win,edi
    @@:
  pop edi esi ebx eax
  ret
endp

align 4
proc but_find, edit:dword ;вызов/скрытие панели поиска
  push edi
  mov edi,dword[edit]

  cmp ted_panel_id,TE_PANEL_NULL
  je @f
    mov ted_panel_id,TE_PANEL_NULL
    mov ted_wnd_l,0
    jmp .e_if
  @@:
    mov ted_panel_id,TE_PANEL_FIND
    mov ted_wnd_l,TE_PANEL_WIDTH
  .e_if:
  stdcall EvSize,edi
  stdcall draw_main_win,edi
  pop edi
  ret
endp

align 4
but_replace:
  ret

align 4
but_find_key_w:
  ret

align 4
proc but_sumb_upper, edit:dword
  push edi esi
  mov edi,dword[edit]

  mov [conv_table],EvUpper
  call ConvertSelText
  cmp esi,0
  je @f
    stdcall draw_main_win,edi
  @@:
  pop esi edi
  ret
endp

align 4
proc but_sumb_lover, edit:dword
  push edi esi
  mov edi,dword[edit]

  mov [conv_table],EvLover
  call ConvertSelText
  cmp esi,0
  je @f
    stdcall draw_main_win,edi
  @@:
  pop esi edi
  ret
endp

align 4
proc but_reverse, edit:dword
  push eax ebx edi
  mov edi,dword[edit]

  stdcall IsSel,edi
  cmp al,0
  je @f
    stdcall SelNormalize,edi
    push esi ecx edx
      mov esi,ted_seln_x0
      mov ecx,ted_seln_y0
      call GetPosByParam
      mov eax,edx
      mov esi,ted_seln_x1
      cmp esi,0
      je .beg_str
	dec esi
      .beg_str:
      mov ecx,ted_seln_y1
      call GetPosByParam
      ;call GetTexPervPos
      mov ebx,edx
    pop edx ecx esi
    ;cmp eax,...
    ;je @f
    call Revers
  @@:
  stdcall draw_main_win,edi
  pop edi ebx eax
  ret
endp

align 4
proc ted_but_undo, edit:dword
  push eax edi
  mov edi,dword[edit]

  mov eax,ted_tim_undo
  cmp ted_tim_ch,eax
  jbe @f
    inc ted_tim_undo
    ;call CmColored
	stdcall draw_main_win,edi
	call draw_but_toolbar
  @@:
  pop edi eax
  ret
endp

align 4
proc ted_but_redo, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_tim_undo,1
  jb @f
    dec ted_tim_undo
    ;call CmColored
	stdcall draw_main_win,edi
	call draw_but_toolbar
  @@:
  pop edi
  ret
endp

align 4
proc but_sumb_invis, edit:dword
  push edi
  mov edi,dword[edit]

  xor ted_mode_invis,1
  call draw_but_toolbar
  stdcall draw_main_win,edi
  pop edi
  ret
endp

align 4
proc but_k_words_show, edit:dword
  push edi
  mov edi,dword[edit]

  xor ted_mode_color,1
  cmp ted_mode_color,0
  je @f
    call CmColored
  @@:
  call draw_but_toolbar
  stdcall draw_main_win,edi
  pop edi
  ret
endp


;input:
; edi = pointer to tedit struct
align 4
but_CtrlHome:
	mov ted_cur_x,0
	mov ted_cur_y,0
	push eax
		mov eax,ted_scr_w
		mov dword[eax+sb_offs_position],0
		mov eax,ted_scr_h
		mov dword[eax+sb_offs_position],0
	pop eax
	call draw_but_toolbar
	stdcall draw_main_win,edi
	ret

align 4
proc but_synt_show, edit:dword
  push edi
  mov edi,[edit]

  cmp ted_panel_id,TE_PANEL_NULL
  je @f
    mov ted_panel_id,TE_PANEL_NULL
    mov ted_wnd_l,0
    jmp .e_if
  @@:
    mov ted_panel_id,TE_PANEL_SYNTAX
    mov ted_wnd_l,TE_PANEL_WIDTH
  .e_if:
  stdcall EvSize,edi
  stdcall draw_main_win,edi
  pop edi
  ret
endp

align 4
proc ted_sel_key_up, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_drag_k,1
  je @f
    stdcall SelStart,edi
    mov ted_drag_k,1
  @@:
  push dx
    call CurMoveUp
    cmp dl,8
    jne @f
      call OnInitialUpdate
    @@:
  pop dx
  stdcall SelMove,edi
  stdcall draw_main_win,edi
  pop edi
  ret
endp

align 4
proc ted_sel_key_down, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_drag_k,1
  je @f
    stdcall SelStart,edi
    mov ted_drag_k,1
  @@:
  push dx
    call CurMoveDown
    cmp dl,8
    jne @f
      call OnInitialUpdate
    @@:
  pop dx
  stdcall SelMove,edi
  stdcall draw_main_win,edi
  pop edi
  ret
endp

align 4
proc ted_sel_key_left, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_drag_k,1
  je @f
    stdcall SelStart,edi
  @@:
  push dx
    call CurMoveLeft
    stdcall SelMove,edi
    cmp ted_drag_k,1
    je @f
      mov ted_drag_k,1
      mov dl,8
    @@:
    cmp dl,8
    jne @f
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp .end_f
    @@:
      stdcall draw_cur_line,edi
    .end_f:
  pop dx
  pop edi
  ret
endp

align 4
proc ted_sel_key_right, edit:dword
  push edi
  mov edi,dword[edit]

  cmp ted_drag_k,1
  je @f
    stdcall SelStart,edi
  @@:
  push dx
    call CurMoveRight
    stdcall SelMove,edi
    cmp ted_drag_k,1
    je @f
      mov ted_drag_k,1
      mov dl,8
    @@:
    cmp dl,8
    jne @f
      call OnInitialUpdate
      stdcall draw_main_win,edi
      jmp .end_f
    @@:
      stdcall draw_cur_line,edi
    .end_f:
  pop dx
  pop edi
  ret
endp
