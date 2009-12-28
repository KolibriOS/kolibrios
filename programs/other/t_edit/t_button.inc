but_NewFile:
  push eax ebx
  call CanSave
  cmp al,1
  jne @f
    push thread
    push msgbox_2
    call [mb_create] ;message: save changes in file?
    ;mov eax,5
    ;mov ebx,50
    ;int 0x40
    push msgbox_2_funct
    call [mb_setfunctions]
    jmp .ret_f
  @@:
  call On_NewFile
  .ret_f:
  pop ebx eax
  ret

On_NewFile:
  push eax
  mov al,1
  call Clear
  call draw_but_toolbar
  call draw_main_win
  pop eax
  ret

On_SaveAndNewFile:
  call but_SaveFile
  cmp byte[err_sav],0
  jne @f
    call On_NewFile
  @@:
  ret

but_OpenFile:
  push eax
  call CanSave
  cmp al,1
  jne @f
    push thread
    push msgbox_5
    call [mb_create] ;message: save changes in file?
    push msgbox_5_funct
    call [mb_setfunctions]
    jmp .ret_f
  @@:
  call On_OpenFile
  .ret_f:
  pop eax
  ret

On_OpenFile:
  push eax ebx
  mov eax,70
  mov [run_file_70.func_n], 0
  mov [run_file_70.param1], 0
  mov [run_file_70.param2], 0
  mov [run_file_70.param3], maxChars-2 ;sizeof.symbol
  m2m [run_file_70.param4], [tex]
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
    push thread
    push msgbox_4
    call [mb_create] ;message: Can-t open text file!
    jmp .ret_f
  @@:
  mov [err_opn],1
  cmp ebx,-1
  je .ret_f
    ;if open file
    call OpenFile
    call draw_but_toolbar
    call draw_main_win
    cmp ebx,maxChars-2
    jl .ret_f
    push thread
    push msgbox_1
    call [mb_create]
  .ret_f:
  pop ebx eax
  ret

but_SaveFile:
  push eax ebx ecx edx
  call CanSave
  cmp al,0
  je .no_save

  mov	ecx,maxChars
  call mem_Alloc
  push eax

  mov edx,[tex]
  xor ecx,ecx
  @@:
    call IteratNext
    cmp edx,[tex_1]
    jle @f ;edx = [tex] or edx = [tex]+sizeof.symbol
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

    mov [err_sav],al

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
      push thread
      push msgbox_6
      call [mb_create] ;message: Can-t save text file!
      jmp @f
    .no_msg:
    m2m [ls_tim],[ch_tim]
  @@:

  pop ecx
  call mem_Free
  .no_save:
  pop edx ecx ebx eax
  ret

but_FindText:
  push eax ebx ecx edx edi
    call GetPos
    mov eax,[edit2.text]
    mov bl,byte[eax]
    @@:
      call GetFindRezult
      cmp bh,1
      je @f ; find

      call IteratNext
      cmp edx,[tex_1]
      jle @f
      jmp @b
    @@:
    cmp bh,0
    je @f
      call GetTexCoords
      inc ebx ;move cursor right
      mov [sel.x1],ebx
      mov [sel.y1],eax
      mov edx,eax
      mov ecx,ebx
      call GoToPos
      mov edx,edi
      call GetTexCoords
      mov [sel.x0],ebx
      mov [sel.y0],eax
      call draw_main_win
      jmp .end_find
    @@:
      push thread
      push msgbox_7
      call [mb_create] ;message: Can not find text
    .end_find:
  pop edi edx ecx ebx eax
  ret

;input:
; eax - text need find
; bl - first symbol to find
; edx - first symbol pointer
;output:
; bh - rezult
; edx - last text position (if find sucess)
; edi - first symbol pointer
GetFindRezult:
  push eax
    mov bh,1
    mov edi,edx ;copy edx
    @@:
      cmp byte[edx],bl
      jne .no_text

      inc eax ;*** get next symbol (in find text) ***
      mov bl,byte[eax]
      cmp bl,0
      je @f ;end of find text

      call IteratNext ;*** get next symbol (in editor text) ***
      cmp edx,[tex_1]
      jle @f ;end of editor text

      jmp @b
      .no_text:
	xor bh,bh
	mov edx,edi ;restore edx
    @@:
  pop eax
  mov bl,byte[eax] ;restore bl
  ret

but_OpenSyntax:
  cmp byte[panel_id],TE_PANEL_SYNTAX
  jne @f
    push dword tree1
    call dword [tl_node_get_data]
    pop  dword [fn_col_option]
    call InitColText
    call CmColored
    call draw_main_win
  @@:
  ret

but_select_word:
  push eax ebx ecx edx

  call GetPos
  push edx
    call ItPervColorTag
    cmp edx,[tex_1]
    jle @f
      call GetTexCoords
      mov [sel.x0],ebx
      mov [sel.y0],eax
    @@:
  pop edx
    call ItNextColorTag
    cmp edx,[tex_1]
    jle @f
      call GetTexCoords
      mov [sel.x1],ebx
      mov [sel.y1],eax
    @@:
  pop edx ecx ebx eax
  call draw_but_toolbar
  call draw_main_win
  jmp still

but_cut:
  call but_Copy
  call SetUndo
  push bx
    mov bl,1
    call SelTextDel
  pop bx
  cmp al,1
  jne @f
    call draw_but_toolbar
    call draw_main_win
  @@:
  jmp still

;output:
; al = 1 if copy text
but_Copy:
  push ax
  call IsSel
  cmp al,0
  jne @f
    pop ax
    ret ;if not selected text
  @@:
  call SelNormalize

  push ebx ecx edx esi
  mov esi,[seln.x1]
  mov ecx,[seln.y1]
  call GetPosByParam
  mov ebx,edx
  mov esi,[seln.x0]
  mov ecx,[seln.y0]
  call GetPosByParam
  mov esi,ebx

  xor ecx,ecx
  mov ebx,buf
  ;mov edx,[tex]
  @@:
    cmp edx,[tex_1] ;end of file
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
  je @f
    call draw_but_toolbar
    call draw_bufer
  @@:
  pop esi edx ecx ebx ax
  ret

but_paste:
  push eax bx esi edi
    mov esi,buf
    call strlen
    cmp eax,1
    jl @f
      call SetUndo
      mov esi,eax
      mov bx,0x0101
      call SelTextDel
      mov edi,buf
      call TextAdd
      call draw_but_toolbar
      call draw_main_win
    @@:
  pop edi esi bx eax
  jmp still

but_find:
  cmp byte[panel_id],TE_PANEL_NULL
  je @f
    mov byte[panel_id],TE_PANEL_NULL
    mov word[wndMain.left],0
    jmp .e_if
  @@:
    mov byte[panel_id],TE_PANEL_FIND
    mov word[wndMain.left],TE_PANEL_WIDTH
  .e_if:
  call EvSize
  call draw_main_win
  jmp still

but_replace:
  jmp still

but_find_key_w:
  jmp still

but_sumb_upper:
  push edi
  mov [conv_table],EvUpper
  call ConvertSelText
  cmp edi,0
  je @f
    call draw_main_win
  @@:
  pop edi
  jmp still

but_sumb_lover:
  push edi
  mov [conv_table],EvLover
  call ConvertSelText
  cmp edi,0
  je @f
    call draw_main_win
  @@:
  pop edi
  jmp still

but_reverse:
  push eax ebx
  call IsSel
  cmp al,0
  je @f
    call SelNormalize
    push esi ecx edx
      mov esi,[seln.x0]
      mov ecx,[seln.y0]
      call GetPosByParam
      mov eax,edx
      mov esi,[seln.x1]
      cmp esi,0
      je .beg_str
	dec esi
      .beg_str:
      mov ecx,[seln.y1]
      call GetPosByParam
      ;call GetTexPervPos
      mov ebx,edx
    pop edx ecx esi
    ;cmp eax,...
    ;je @f
    call Revers
  @@:
  pop ebx eax
  call draw_main_win
  jmp still

but_undo:
  mov eax,[tim_Undo]
  cmp [ch_tim],eax
  jbe still
  inc [tim_Undo]
  ;call CmColored
  jmp red_win

but_redo:
  cmp [tim_Undo],1
  jb still
  dec [tim_Undo]
  ;call CmColored
  jmp red_win


but_sumb_invis:
  xor [invis],1
  jmp red_win

but_k_words_show:
  xor byte[mode_colored],1
  cmp byte[mode_colored],0
  je red_win
  call CmColored

;  push eax ebx ecx esi edi
;  mov eax,dword[cur_x]
;  ColToIndexOffset eax,edx
;  mov eax,4
;  mov ebx,50*65536+75
;  mov ecx,0x40ffffff
;  mov edi,0x404040
;  mov esi,10
;  int 0x40
;
;  mov eax,47
;  mov ecx,250
;  mov esi,0xffff80
;  mov edx,50*65536+80
;  mov esi,FkPos;[keyW]
;  @@:
;    mov ebx,0x30001
;    cmp dx,300
;    jl .no_br
;      mov dx,80
;      add edx,0x200000
;    .no_br:
;    add dx,10
;    push ecx esi
;      mov ecx,esi
;      cmp dword[ecx],-1
;      jne .no_minus
;       mov ebx,0x10000
;       mov ecx,1
;      .no_minus:
;      mov esi,0xffff00
;      int 0x40
;    pop esi ecx
;    add esi,4
;  loop @b
;  pop edi esi ecx ebx eax
  jmp red_win

but_CtrlHome:
  mov [cur_x],0
  mov [cur_y],0
  mov [wScr.position],0
  mov [hScr.position],0

  call draw_but_toolbar
  call draw_main_win
  ret

but_synt_show:
  cmp byte[panel_id],TE_PANEL_NULL
  je @f
    mov byte[panel_id],TE_PANEL_NULL
    mov word[wndMain.left],0
    jmp .e_if
  @@:
    mov byte[panel_id],TE_PANEL_SYNTAX
    mov word[wndMain.left],TE_PANEL_WIDTH
  .e_if:
  call EvSize
  call draw_main_win
  jmp still

sel_KeyUp:
  cmp [dragk],1
  je @f
    call SelStart
    mov [dragk],1
  @@:
  push dx
    call CurMoveUp
    cmp dl,8
    jne @f
      call OnInitialUpdate
    @@:
  pop dx
  call SelMove
  call draw_main_win
  ret

sel_KeyDown:
  cmp [dragk],1
  je @f
    call SelStart
    mov [dragk],1
  @@:
  push dx
    call CurMoveDown
    cmp dl,8
    jne @f
      call OnInitialUpdate
    @@:
  pop dx
  call SelMove
  call draw_main_win
  ret

sel_KeyLeft:
  cmp [dragk],1
  je @f
    call SelStart
  @@:
  push dx
    call CurMoveLeft
    call SelMove
    cmp [dragk],1
    je @f
      mov [dragk],1
      mov dl,8
    @@:
    cmp dl,8
    jne @f
      call OnInitialUpdate
      call draw_main_win
      jmp .end_f
    @@:
      call draw_cur_line
    .end_f:
  pop dx
  ret

sel_KeyRight:
  cmp [dragk],1
  je @f
    call SelStart
  @@:
  push dx
    call CurMoveRight
    call SelMove
    cmp [dragk],1
    je @f
      mov [dragk],1
      mov dl,8
    @@:
    cmp dl,8
    jne @f
      call OnInitialUpdate
      call draw_main_win
      jmp .end_f
    @@:
      call draw_cur_line
    .end_f:
  pop dx
  ret