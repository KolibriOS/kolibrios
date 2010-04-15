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

ColColor dd 1
ColWords dd 0
CURSOR_COLOR dd 0x808080
WND_CAPT_COLOR dd 0x80
WND_WORK_COLOR dd 0x0
WND_BORD_COLOR dd 0xd0d0d0
SELECT_COLOR dd 0x208080
CUR_TEXT_COLOR dd 0xff0000
WND_TEXT_COLOR dd 0xffff00

options_file dd 0
options_file_end dd 0
tex_colors dd 0
tex_help_f1 dd 0

FkPos: rd 256
Col dd 0 ;указатель на структуры ключевых слов TexColViv

conv_tabl rb 128 ; таблица для конвертирования scan-кода в ascii-код
keyUpr dd 0

err_sav db 0 ;ошибка сохранения файла
panel_id db 0 ;номер открытой панели

ch_tim	 dd ? ; количество изменений в файле
ls_tim	 dd ? ; время последнего сохранения
co_tim	 dd ? ; время последней цветовой разметки
tim_Undo dd ? ; количество отмененных действий
gpOpt db 0
mode_colored db 1 ; режим выделения слов цветом (0-выкл. 1-вкл.)

key_new db 0 ; символ, который будет добавлятся с клавиатуры
tex	dd 0 ;text memory pointer
tex_1	dd 0 ;text first symbol pointer
tex_end dd 0 ;text end memory pointer

curMod db 1
cur_x dd 0
cur_y dd 0
invis db 0 ;режим показа непечатаемых символов
symbol_new_line db 20 ;символ завершения строки
help_id dd -1  ;идентификатор для справки

hed db 'TextEditor 15.04.10',0 ;подпись окна
sc system_colors

fn_icon db 'te_icon.bmp',0
bmp_icon dd 0
run_file_70 f70 	  


;input:
; reg = index
;output:
; reg = pointer to 'tex' struct
macro ConvertIndexToPointer reg {
  imul reg,sizeof.symbol
  add reg,[tex]
}


;output:
; al = 1 - can save
CanSave:
  push ecx
  mov ecx,[ch_tim]
  sub ecx,[tim_Undo]
  mov al,1
  cmp [ls_tim],ecx
  jne @f
    dec al
  @@:
  pop ecx
  ret


;output:
; al = 1 - selected
proc IsSel, edit:dword
  push ebx edi
  mov edi,dword[edit]
  xor al,al
  cmp ted_drag_m,1
  je @f
    mov al,1
    mov ebx,[sel.x0]
    cmp ebx,[sel.x1]
    jne @f
    mov ebx,[sel.y0]
    cmp ebx,[sel.y1]
    jne @f
    xor al,al
  @@:
  pop edi ebx
  ret
endp

SelNormalize:
  push ecx esi edi
    mov esi,sel
    mov edi,seln
    mov ecx,sizeof.TexSelect
    rep movsb

    jmp @f
    .swp_f:
      mov ecx,[seln.x0]
      m2m [seln.x0],[seln.x1]
      mov [seln.x1],ecx

      mov ecx,[seln.y0]
      cmp ecx,[seln.y1] ;(sel.y0>sel.y1)
      jle .end_f
      m2m [seln.y0],[seln.y1]
      mov [seln.y1],ecx

    jmp .end_f
    @@:

    mov ecx,[seln.y0]
    cmp ecx,[seln.y1] ;(sel.y0>sel.y1)
    jg .swp_f

    cmp ecx,[seln.y1] ;(sel.y0==sel.y1)
    jne .end_f
      mov ecx,[seln.x0]
      cmp ecx,[seln.x1] ;(sel.x0>sel.x1)
      jg .swp_f

    .end_f:
  pop edi esi ecx
  ret

SelStart:
  push ecx
    mov ecx,[cur_x]
    add ecx,[hScr.position]
    mov [sel.x0],ecx
    mov [sel.x1],ecx

    mov ecx,[cur_y]
    add ecx,[wScr.position]
    mov [sel.y0],ecx
    mov [sel.y1],ecx
  pop ecx
  ret

align 4
SelMove:
  push ecx
    mov ecx,[cur_x]
    add ecx,[hScr.position]
    mov [sel.x1],ecx

    mov ecx,[cur_y]
    add ecx,[wScr.position]
    mov [sel.y1],ecx
  pop ecx
  call draw_but_toolbar ;redraw toolbar (need to button Copy)
  ret

;input:
; al = 1 - clear all memory
Clear:
  mov [cur_x],0
  mov [cur_y],0
  mov [ch_tim],0
  mov [ls_tim],0
  mov [co_tim],0
  mov [tim_Undo],0
  mov [help_id],-1

  mov [wScr.position],0
  mov [wScr.max_area],100
  mov [wScr.redraw],1
  mov [hScr.position],0
  mov [hScr.max_area],100

  mov [sel.x0],0
  mov [sel.y0],0
  mov [sel.x1],0
  mov [sel.y1],0

  cmp al,0
  je .exit

  push ecx edx
  mov ecx,sizeof.symbol*maxChars
  mov edx,[tex]
  @@:
    mov byte [edx],0
    inc edx
  loop @b
  mov edx,[tex]
  mov dword [edx+6],1
  pop edx ecx

  .exit:
  ret


;--- out_reg = Col[keyw[ind_reg]].Text[0] ---
macro ColToIndexOffset ind_reg,out_reg {
  mov out_reg,ind_reg
  imul out_reg,sizeof.TexColViv
  add out_reg,[Col]
}



InitColText:
  push eax ebx ecx esi edi

  mov  ebx,dword[fn_col_option]
  copy_path ebx,fn_syntax_dir,file_name_rez,0x0
  copy_path file_name_rez,sys_path,file_name,0x0

  mov eax,70
  mov [run_file_70.func_n], 0
  mov [run_file_70.param1], 0
  mov [run_file_70.param2], 0
  mov ebx,[options_file_end]
  sub ebx,[options_file]
  mov [run_file_70.param3], ebx
  m2m [run_file_70.param4], [options_file]
  mov [run_file_70.rezerv], 0
  mov [run_file_70.name], file_name
  mov ebx,run_file_70
  int 0x40

  mov ecx,0x100
  mov edi,FkPos
  @@:
    mov dword[edi],-1
    add edi,4
  loop @b

  ;init: ColColor, ColWords, ...
  mov dword[ColColor],1
  mov dword[ColWords],0
    mov dword[tex_help_f1],0
  mov [help_id],-1 ;идентификатор слова для справки

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
    push thread
    push msgbox_3
    call [mb_create] ;message: Can-t open color options file!
    jmp .no_colors
  @@:

  mov esi,[options_file]
  mov edi,ColColor
  mov ecx,9*4
  rep movsb

  mov eax,[options_file]
  add eax,32
  mov dword[tex_colors],eax

  mov eax,dword[ColColor] ;init: Col (offset to key words)
  add eax,8
  shl eax,2
  add eax,[options_file]
  mov dword[Col],eax

  mov ecx,dword[ColWords] ;init: FkPos (first key positions)
  xor eax,eax
  @@:
    ColToIndexOffset eax,edi
    xor ebx,ebx
    mov bl,byte[edi]
    shl bx,2
    mov esi,FkPos
    add esi,ebx
    cmp dword[esi],-1
    jne .no_ch_key
      mov dword[esi],eax
    .no_ch_key:
    inc eax
  loop @b

  ;init: tex_help_f1
  mov ecx,dword[ColWords]
  imul ecx,sizeof.TexColViv
  add ecx,dword[Col]
  mov dword[tex_help_f1],ecx

  .no_colors:
  pop edi esi ecx ebx eax
  ret


;input:
; ebx = file size
OpenFile:
  push eax ;destination
  push ecx ;for cycle
  push edx ;source

    xor al,al
    call Clear
    mov eax,ebx
    mov ecx,ebx
    add eax,2
    ConvertIndexToPointer eax
    mov edx,[tex]
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

    mov edx,[tex]
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
    add edx,[tex]
    mov dword [edx+6],1 ; last sumbol 'next=1'

    @@: ;clear memory, need if before was open big file
      add edx,sizeof.symbol
      cmp edx,[tex_end]
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
  ret

;input:
; edx = pointer to symbol struct
;output:
; edx = pointer to 'perv' visible symbol struct
IteratPerv:
  cmp [tim_Undo],0
  je .else
  push ebx
  @@:
    call GetTexPervPos
    cmp edx,[tex]
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
    cmp edx,[tex]
    je .endif
    cmp dword [edx+14],0
    jne .else
    cmp byte[edx],10 ;пропуск символа с кодом 10
    je .else
  .endif:
  ret


;input:
; edx = pointer to symbol struct
;output:
; edx = pointer to 'next' visible symbol struct
IteratNext:
  cmp [tim_Undo],0
  je .else
  push ebx
  @@:
    call GetTexNextPos
    cmp edx,[tex_1]
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
    cmp edx,[tex_1]
    jle .endif
    cmp dword [edx+14],0
    jne .else
    cmp byte[edx],10 ;пропуск символа с кодом 10
    je .else
  .endif:
  ret

;input:
; edx = pointer to symbol struct
; bl = symbol end of select
ItPoNextUc:
  @@:
    cmp bl,byte[edx]
    je @f
    cmp edx,[tex_1]
    jle @f
    call IteratNext
    jmp @b
  @@:
  call IteratNext
  ret

;input:
; edx = pointer to symbol struct
ItPervColorTag:
  @@:
    cmp byte[edx+1],0
    jne @f
    call IteratPerv
    cmp edx,[tex_1]
    jle @f
    jmp @b
  @@:
  ret

;input:
; edx = pointer to symbol struct
ItNextColorTag:
  @@:
    call IteratNext
    cmp byte[edx+1],0
    jne @f
    cmp edx,[tex_1]
    jle @f
    jmp @b
  @@:
  ;call IteratNext
  ret

;input:
; edx = pointer to symbol struct
;output:
; bl = 1 if sumbol not visible
; (tex[i].td+tim_Undo<=ch_tim && tex[i].td) || (tex[i].tc>ch_tim-tim_Undo)
SymbNotVis:
  push eax

  xor bl,bl

  cmp dword [edx+14],0
  je @f
  mov eax,[edx+14] ;eax=tex[i].td
  add eax,[tim_Undo]
  cmp eax,[ch_tim]
  jg @f
    mov bl,1
    pop eax
    ret
  @@:

  mov eax,[ch_tim]
  sub eax,[tim_Undo]
  cmp [edx+10],eax
  jle @f
    or bl,1
  @@:

  pop eax
  ret


;input:
; bh = 1 - move cursor after text add
; bl = 1 - change time when text add
; esi = add text len
; edi = pointer to text string
TextAdd:
  push ebp
  mov ebp,esp
  sub esp,4*2

  push eax ;po_t
  push ecx edx edi

  call GetPos
  call GetTexPervPos
  call GetTexArrIndex ;eax=po_t

  mov dword[ebp-4],0 ;count new spaces
  cmp [gpOpt],2
  je @f
    push eax ebx ;c_sp=cur[cn].x+Scroller->XPos-StrLen(cur[cn].y+Scroller->YPos);
      mov eax,[cur_x]
      add eax,[hScr.position]
      mov dword[ebp-4],eax

      mov eax,[cur_y]
      add eax,[wScr.position]
      call StrLen
      sub dword[ebp-4],ebx
    pop ebx eax
  @@:

  mov dword[ebp-8],0 ;count new lines
  cmp [gpOpt],0
  jne @f
    push eax
      mov eax,[cur_y]
      add eax,[wScr.position]
      inc eax
      mov dword[ebp-8],eax

      call GetNumLines
      sub dword[ebp-8],eax
    pop eax
  @@:

  mov edx,sizeof.symbol
  shl edx,1
  add edx,[tex]
  @@: ;for(i=2;i<texMax;i++)
    cmp dword [edx+10],0 ;if(!tex[i].tc && !tex[i].td)
    jne .u1f
    cmp dword [edx+14],0
    jne .u1f
      cmp bl,0 ;if(n_tim) ch_tim++;
      je .no_tim
	inc [ch_tim]
      .no_tim:
      cmp bh,0
      je .no_cur_mov
      cmp dword[ebp-8],0 ;если есть добавочные строки, то курсор еще не двигаем
      jg .no_cur_mov
      cmp dword[ebp-4],0 ;если нет добавочных пробелов, то курсор тоже не двигаем
      jg .no_cur_mov
	inc [cur_x] ;move cursor
	;call GoToPos
	cmp byte [edi],13
	jne .no_cur_mov
	  mov [cur_x],0
	  inc [cur_y]
      .no_cur_mov:

      xor bl,bl ;n_tim=false;

      mov cl,byte [edi] ;tex[i].c=ta[ns];
      mov byte [edx],cl
      m2m dword [edx+10],dword [ch_tim] ;tex[i].tc=ch_tim;
      mov [edx+2],eax ;tex[i].perv=po_t;

      mov ecx,eax
      imul ecx,sizeof.symbol
      add ecx,[tex] ; *** ecx = tex[po_t] ***
      add ecx,6   ; *** ecx = tex[po_t].next ***
      m2m dword [edx+6],dword [ecx] ;tex[i].next=tex[po_t].next;

      call GetTexArrIndex ;*** eax = i ***
      cmp eax,maxChars
      jge @f
      mov [ecx],eax ;tex[po_t].next=i; // ссылки перенаправляем
      mov ecx,[edx+6] ; *** ecx = tex[i].next ***
      imul ecx,sizeof.symbol
      add ecx,[tex] ; *** ecx = tex[tex[i].next] ***
      mov [ecx+2],eax ;tex[tex[i].next].perv=i;

      cmp dword[ebp-8],0 ;add lines or text
      jle .spc_add
	dec dword[ebp-8]
	mov byte [edx],13
	jmp .u1f
      .spc_add:
      cmp dword[ebp-4],0 ;add spaces or text
      jle .tex_add
	dec dword[ebp-4]
	mov byte [edx],' '
	jmp .u1f
      .tex_add:
      inc edi
      dec esi
    .u1f:
    add edx,sizeof.symbol
    cmp edx,[tex_end]
    jge @f ;out of memory
    cmp esi,0
    jne @b
  @@:

  pop edi edx ecx eax
  mov esp,ebp ; восстанавливаем стек
  pop ebp
  call CmColored
  ret



;input:
;  ecx = position to free insert cell
;  edx = pointer to sumbol, when insert
;  esi = pointer to added symbol
;output:
;  ecx = position to inserted cell
CharAdd:

  .loop_b:
    cmp ecx,[tex_end]
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
  mov eax,[ch_tim]
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
;output:
; edi = count converted symbols
ConvertSelText:
  xor edi,edi
  push eax ebx ecx edx esi
  stdcall IsSel, tedit0
  cmp al,0
  je .end_f
    call SetUndo
    call SelNormalize

    mov esi,[seln.x0]
    mov ecx,[seln.y0]
    call GetPosByParam
    mov eax,edx
    mov esi,[seln.x1]
    mov ecx,[seln.y1]
    call GetPosByParam
    ;call GetTexPervPos
    mov ebx,edx

    cmp eax,ebx
    je .end_f

    inc [ch_tim]
    mov edx,eax ;i=p0;
    mov ecx,2
    ConvertIndexToPointer ecx
    @@:
      mov esi,[edx]
      and esi,0xff
      add esi,[conv_table] ;EvUpper
      cmp byte [esi],0
      je .no_change
	m2m dword [edx+14],dword [ch_tim]
	call CharAdd ;b_pos=CharAdd(tex[i].c^32,i,false,b_pos);
	call GetTexNextPos ;go to added symbol
	inc edi
      .no_change:

      call IteratNext
      cmp edx,[tex]
      je @f 
      cmp edx,ebx
      je @f

      jmp @b
    @@:
    cmp edi,0
    jne @f
      dec [ch_tim]
    @@:
  .end_f:
  pop esi edx ecx ebx eax

  ret

;input:
; bh = (0-backspace, 1-delete)
; bl = 1 - change time
;output:
; bl = 0 - no delete
; bl = 1 - delete
TextDel:
  push edx cx
  xor cl,cl
  cmp bh,1
  je @f
    call CurMoveLeft
    cmp dl,0
    je .no_del
  @@:
  call GetPos
  cmp [gpOpt],1
  je .no_del
    cmp bl,0
    je @f
      inc [ch_tim]
    @@:
    m2m dword [edx+14], dword [ch_tim]
    mov cl,1
  .no_del:
  mov bl,cl
  pop cx edx
  ret



;input:
; eax = pointer to begin select
; ebx = pointer to end select
Revers:
  cmp eax,ebx
  jne @f
    ret
  @@:

  push ecx edx

  mov edx,[tex_1]
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
;cmp edx,[tex]
;je @f
; ---
    mov edx,ecx ;i = tmp;
    ConvertIndexToPointer edx
    jmp @b
  @@:
  pop edx ecx
  call CmColored
  ret


;output:
; dl = 0 not move
; dl = 2 if move up
; dl = 8 if scroll move up
CurMoveUp:
  cmp [cur_y],0
  je @f
    dec [cur_y]
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

;output:
; dl = 0 not move
; dl = 2 if move down
; dl = 8 if scroll move down
CurMoveDown:
  push eax
  mov dl,0
  mov eax,[wScr.cur_area]
  dec eax
  cmp [cur_y],eax
  jge @f
    inc [cur_y]
    mov dl,2
    jmp .ret_f
  @@:
  mov eax,[cur_y]
  add eax,[wScr.position]
  inc eax
  cmp [wScr.max_area],eax
  jle @f
    inc [wScr.position]
    mov dl,8
  @@:
  .ret_f:
  pop eax
  ret


;output:
; dl = 0 not move
; dl = 1 if move up
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

CurMovePageDown:
  push eax ebx
  xor dl,dl
  mov eax,[wScr.max_area]
  sub eax,[wScr.cur_area]
  cmp [wScr.position],eax
  jge @f
    mov ebx,[wScr.cur_area]
    add [wScr.position],ebx
    mov dl,1
    mov [wScr.redraw],1
    cmp [wScr.position],eax
    jle @f
      mov [wScr.position],eax
  @@:
  pop ebx eax
  ret

;output:
; dl = 0 not move
; dl = 1 if move left
; dl = 3 if move left and up
; dl = 8 if scroll move up
CurMoveLeft:
  cmp [cur_x],0
  je @f
    dec [cur_x]
    mov dl,1
    ret
  @@:
  cmp [hScr.position],0
  je @f
    dec [hScr.position]
    mov dl,8
    ret
  @@:
  cmp [cur_y],0
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
  cmp [cur_y],0
  je @f
  dec [cur_y]
  call CurMoveX_LastChar
  cmp dl,8
  je .ret_f
  mov dl,3
  ret
  @@:
  mov dl,0
  .ret_f:
  ret

CurMoveRight:
  push eax
  xor dl,dl
  mov eax,[hScr.cur_area]
  cmp [cur_x],eax
  jge @f
    inc [cur_x]
    mov dl,1
    jmp .ret_f
  @@:
    inc [hScr.position]
    mov dl,8
  .ret_f:
  pop eax
  ret

CurMoveX_LastChar:
;[hScr.position]
;[hScr.cur_area]
;dl-???
  push eax ebx
  mov eax,[cur_y]
  add eax,[wScr.position]
  call StrLen
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
  mov [cur_x],ebx
  pop ebx eax
  ret

;output:
; dl = 0 not move
; dl = 1 move cursor
; dl = 8 move cursor and scroll
CurMoveX_FirstChar:
  xor dl,dl
  cmp [cur_x],0
  je @f
  mov [cur_x],0
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
;output:
; eax = array index
GetTexArrIndex:
  push ecx edx

  mov eax,edx
  sub eax,[tex]
  xor edx,edx
  mov ecx,sizeof.symbol
  div ecx

  pop edx ecx
  ret

;input:
; edx = pointer to symbol struct
;output:
; edx = pointer to 'perv' struct
GetTexPervPos:
  push dword [edx+2]
  pop edx
  imul edx,sizeof.symbol
  add edx,[tex]
  ret

;input:
; edx = pointer to symbol struct
;output:
; edx = pointer to 'next' symbol struct
GetTexNextPos:
  push dword [edx+6]
  pop edx
  imul edx,sizeof.symbol
  add edx,[tex]
  ret


;output:
; edx = symbol under cursor
; gpOpt = 1,2
; edx = tex[1].perv if error
; gpOpt = 0
GetPos:
  push esi ecx
    mov esi,[cur_x]
    add esi,[hScr.position]
    mov ecx,[cur_y]
    add ecx,[wScr.position]
    call GetPosByParam
  pop ecx esi
  ret


;input:
; esi = XPos
; ecx = YPos
;output:
; edx = symbol under cursor
; gpOpt = 1 if found text line
; gpOpt = 2 if found text line and column
; edx = tex[1] if error
; gpOpt = 0 if text no found
GetPosByParam:
  push eax ;Row
  push ebx ;Col

  xor eax,eax
  xor ebx,ebx
  mov [gpOpt],0
  mov edx,[tex]
  @@:
    call IteratNext
    cmp edx,[tex_1]
    jle @f 
    cmp ebx,esi
    jne .u1_0 ;Col <> cur_x
      mov [gpOpt],1
      cmp eax,ecx
      jge @f ; Row >= cur_y
    .u1_0:
    mov [gpOpt],0
    inc ebx
    cmp byte [edx],13
    jne @b
    cmp eax,ecx
    jge @f ; Row >= cur_y
    inc eax
    xor ebx,ebx
    jmp @b
  @@:
  cmp eax,ecx
  jne @f ; Row = cur_y
    inc [gpOpt]
  @@:
  cmp [gpOpt],0
  jne @f
    mov edx,[tex_1]
    ;call GetTexPervPos
  @@:
  pop ebx eax
  ret


;input:
; eax = Row
;output:
; ebx = str len
StrLen:
  push edx ecx
  ;ecx = Row, from cycle

  xor ebx,ebx
  xor ecx,ecx
  mov edx,[tex]
  @@:
    call IteratNext
    cmp edx,[tex_1]
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


;output:
; edx = symbol position
;output:
; eax = number of line
; ebx = symbol position in line
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

    cmp edx,[tex_1]
    jle @f
    cmp byte [edx],13
    jne @b
    inc eax
    jmp @b
  @@:
  dec ebx
  pop edx
  ret

;output:
; eax = num lines
GetNumLines:
  push edx
  mov eax,1
  mov edx,[tex]
  @@:
    call IteratNext
    cmp edx,[tex_1]
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

SetUndo:
  mov [dragk],0 ;заканчиваем выделение от клавиатуры
  cmp [tim_Undo],1
  jl .no_work

  push eax ebx edx
  mov edx,[tex]
  call GetTexNextPos ;long i=tex[0].next;
  mov eax,[tim_Undo]
  sub [ch_tim],eax ;ch_tim-=tim_Undo;
  mov eax,[ch_tim]
  cmp [ls_tim],eax ;if(ls_tim>ch_tim)
  jle @f
    mov dword [ls_tim],0
  @@:
    cmp edx,[tex]
    je @f

    ;if(tex[i].tc>ch_tim){ // если создание символа было отменено
    cmp [edx+10],eax
    jle .no_u1
      mov dword [edx+10],0
      mov dword [edx+14],0

      mov ebx,[edx+2]
      imul ebx,sizeof.symbol
      add ebx,[tex];.next
      m2m dword [ebx+6],dword [edx+6] ;tex[tex[i].perv].next=tex[i].next;

      mov ebx,[edx+6]
      imul ebx,sizeof.symbol
      add ebx,[tex];.perv
      m2m dword [ebx+2],dword [edx+2] ;tex[tex[i].next].perv=tex[i].perv;

    .no_u1:

    ;else if(tex[i].td>ch_tim) tex[i].td=0; // если удаление символа было отменено
    cmp [edx+14],eax
    jle .no_u2
      mov dword [edx+14],0
    .no_u2:

    call GetTexNextPos
    jmp @b
  @@:
  mov dword [tim_Undo],0
  mov eax,[co_tim]
  cmp [ch_tim],eax
  jge @f
    mov [co_tim],0
  @@:
  pop edx ebx eax
  .no_work:
  ret

;input:
; ecx = Col
; edx = Row
GoToPos:
  mov [cur_x],ecx
  sub edx,[wScr.position]

  cmp edx,[wScr.cur_area] ;[cur_y] > [.cur_area]
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
  mov [cur_y],edx
  ret

;input:
; bl = 1 change time when delete text
;output:
; al = 1 if delete
SelTextDel:
  stdcall IsSel, tedit0
  cmp al,0
  jne @f
  ret
  @@:
  ;call SelEnd
  call SelNormalize
  push esi edi ecx edx
    mov esi,[seln.x1]
    mov ecx,[seln.y1]
    call GetPosByParam
    mov edi,edx

    mov esi,[seln.x0]
    mov ecx,[seln.y0]
    call GetPosByParam

    cmp bl,0
    je @f
      inc [ch_tim]

    @@:
      cmp edx,[tex]
      je @f
      cmp edx,edi ;if(i==te)break;
      je @f
      m2m dword[edx+14],dword[ch_tim]
      xor bl,bl   ;n_tim=false;
      call IteratNext
      jmp @b
    @@:
    cmp bl,0
    je @f
      dec [ch_tim]
      xor al,al
    @@:
    cmp bl,0
    jne @f
      mov ecx,[seln.x0]
      mov edx,[seln.y0]
      call GoToPos
      mov [sel.x0],0
      mov [sel.y0],0
      mov [sel.x1],0
      mov [sel.y1],0
    @@:
  pop edx ecx edi esi

  ret


CmColored:
  push eax edx
  mov eax,[ch_tim]
  sub eax,[tim_Undo]
  mov dword[co_tim],eax
  mov edx,[tex]
  @@:
    call IteratNext
    cmp edx,[tex_1]
    jle @f
    mov byte[edx+1],0
    jmp @b
  @@:

  cmp dword[ColWords],1
  jl .no_colors
  mov edx,[tex]
  @@:
    call TextFSColor
    cmp edx,[tex_1]
    jle .no_colors
    jmp @b
  .no_colors:
  pop edx eax
  ret



;input:
; edx = pointer to start symbol
;output:
; edx = pointer to next symbol
TextFSColor:
  ;eax = word_n
  ;ecx = l_pos
  push ebp
  mov ebp,esp
  sub esp,10 ;2*4+2*1
  ;bP  = dword[ebp-4]
  ;eP  = dword[ebp-8]
  ;fnd = byte[ebp-12]
  ;f_color = byte[ebp-13]

  push eax ebx ecx esi edi
  mov dword[ebp-4],1
  mov dword[ebp-4],1
  mov byte[ebp-12],0
  mov byte[ebp-13],1
  @@:
    call IteratNext
    cmp edx,[tex_1]
    jle @f

    xor ebx,ebx
    mov bl,byte[edx]
;mov byte[buf],bl
;mov byte[buf+1],0
    shl bx,2 ;ebx*=4
    add ebx,FkPos
    mov eax,dword[ebx]
    cmp eax,0
    jl @b ;if( (word_n=FkPos[(unsigned char)tex[i].c])>-1 ){

    mov ecx,eax
    ;while(l_pos<ColWords && Col[keyw[l_pos]].Text[0]==Col[keyw[word_n]].Text[0])
    .wh_1b:
      cmp ecx,dword[ColWords]
      jge .wh_1e
      ColToIndexOffset ecx,esi
      mov bh,byte[esi]
      ColToIndexOffset eax,esi
      mov bl,byte[esi]
      cmp bh,bl
      jne .wh_1e
	inc ecx
      jmp .wh_1b
    .wh_1e:

    mov dword[ebp-4],edx ;bP=i;
    mov edi,1

    .wh_2b: ;while(1){
      call IteratNext

      ;while(l_pos>word_n && Col[keyw[l_pos-1]].Text[pos]!=tex[i].c)
      .wh_3b:
	cmp ecx,eax
	jle .wh_3e
	dec ecx
	ColToIndexOffset ecx,ebx
	inc ecx
	;cmp byte[ebx+edi],byte[edx]
	push ax
	  mov al,byte[ebx+edi]
	  mov bl,al
	pop ax
	cmp bl,byte[edx]
	je .wh_3e
	  dec ecx
	jmp .wh_3b
      .wh_3e:

      ColToIndexOffset eax,ebx
      cmp byte[ebx+edi],0
      jne .if_0 ;if(Col[keyw[word_n]].Text[pos]==0){
	mov dword[ebp-8],edx ;eP=i;
	ColToIndexOffset eax,esi
	mov bl,byte[esi+MAX_COLOR_WORD_LEN+6]
	mov byte[ebp-13],bl ;f_color=Col[keyw[word_n]].color;

mov byte[ebp-12],1
;... esi = Col[keyw[word_n]]
	  mov bl,byte[esi+MAX_COLOR_WORD_LEN+4]
	  cmp bl,0 ;if(Col[keyw[word_n]].wwo)
	  je .if_2n
	    push edx
	    mov edx,dword[ebp-4]
     call IteratPerv
;           mov bh,bl

	    btr bx,0 ;1-1
	    jae .if_3e ;if(Col[keyw[word_n]].wwo&1)
	      ;u1= !(isalnum(cont_s)||cont_s=='_')
	      call isalnum
	      jae .if_3e
		mov byte[ebp-12],0
	    .if_3e:

	    btr bx,3 ;4-1
	    jae .if_4e ;if(Col[keyw[word_n]].wwo&8)
	      ;u1= !isalpha(cont_s);
	      call isalpha
	      jae .if_4e
		mov byte[ebp-12],0
	    .if_4e:


	    mov edx,dword[ebp-8]
;     call IteratNext

	    btr bx,1 ;2-1
	    jae .if_5e ;if(Col[keyw[word_n]].wwo&2)
	      ;u1= !(isalnum(cont_s)||cont_s=='_')
	      call isalnum
	      jae .if_5e
		mov byte[ebp-12],0
	    .if_5e:

	    btr bx,4 ;5-1
	    jae .if_6e ;if(Col[keyw[word_n]].wwo&16)
	      ;u1= !isalpha(cont_s);
	      call isalpha
	      jae .if_6e
		mov byte[ebp-12],0
	    .if_6e:

	    btr bx,2 ;3-1
	    jae .if_7e ;if(Col[keyw[word_n]].wwo&4)
       mov bl,byte[esi+MAX_COLOR_WORD_LEN+5]
	      call ItPoNextUc
       cmp edx,[tex_1]
       jle .if_7e
       mov dword[ebp-8],edx
	    .if_7e:

	    pop edx
	  .if_2n:
;                 if(i!=1){ // не конец документа
;                   cont_s=tex[eP].c;
;                   if(Col[keyw[word_n]].wwo&2) u2= !(isalnum(cont_s)||cont_s=='_');  // не букв.-числ. символ
;                   if(u2 && Col[keyw[word_n]].wwo&16) u2= !isalpha(cont_s); // не числ. символ
;                   if(Col[keyw[word_n]].wwo&4) eP=ItPoNextUc(eP,Col[keyw[word_n]].endc);

	cmp eax,ecx
	je .wh_2e ;if(word_n==l_pos) break; // do double - если слово точно последнее
      .if_0:

      cmp edx,[tex_1]
      jle .wh_2e ;if(i==1) break;

      ;while(l_pos>word_n && Col[keyw[word_n]].Text[pos]!=tex[i].c)
      .wh_4b:
	cmp ecx,eax
	jle .wh_4e
	ColToIndexOffset eax,ebx
	;cmp byte[ebx+edi],byte[edx]
	push ax
	  mov al,byte[ebx+edi]
	  mov bl,al
	pop ax
	cmp bl,byte[edx]
	je .wh_4e
	  inc eax
	jmp .wh_4b
      .wh_4e:

      cmp eax,ecx
      je .wh_2e;if(word_n==l_pos) break;
      inc edi ;pos++;
      jmp .wh_2b
    .wh_2e:

    cmp byte[ebp-12],1 ;if(fnd)break;
    je @f
    mov edx,dword[ebp-4];i=bP;
    jmp @b
  @@:

  cmp byte[ebp-12],1
  jne .if_1e ;if(fnd){ // выделение найденого текста
;    if(!mode_sf1 || (mode_sf1 && strlen(Col[keyw[word_n]].f1->c_str())>0)){
    mov eax,dword[ebp-4]
    mov bl,byte[ebp-13]
    mov byte[eax+1],bl ;tex[bP].col=f_color;
    mov eax,dword[ebp-8]
    mov byte[eax+1],0xff ;tex[eP].col=255;
;    return ItPoPerv(eP); // возвращаем позицию конца вхождения
    mov edx,dword[ebp-8]
    call GetTexPervPos
    jmp @f
  .if_1e:
    mov edx,[tex]
  @@:

  pop edi esi ecx ebx eax
  mov esp,ebp
  pop ebp
  ret


;input:
; edx = pointer to char (byte)
;output:
; cf=1 if symbol is...
tab_all_num db 0,0,0,0,0,0,11111111b,11b,11111110b,0xff,0xff,111b,11111110b,0xff,0xff,111b,0,0,0,0,0,0,0,0,0,0;,0,0,0,0,0,0 - tab_alpha_0,0,0,0,0,0
tab_alpha db 0,0,0,0,0,0,0,0,11111110b,0xff,0xff,111b,11111110b,0xff,0xff,111b,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  ;db 0,0,0,0,0,0,11111111b,11b,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
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
  m2m bx,word[ebx]
  btr bx,ax
  pop ebx eax
  ret
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
  m2m bx,word[ebx]
  btr bx,ax
  pop ebx eax
  ret


ShowHelpF1:
  push eax edx

  call GetPos
  push edx
    call ItNextColorTag
    mov eax,edx
  pop edx
  call ItPervColorTag

  cmp eax,[tex]
  jle @f
  cmp edx,[tex_1]
  jle @f
    call FindHelpId
  @@:
  pop edx eax
  ;stdcall draw_main_cursor, tedit0
  stdcall draw_help_f1, tedit0
  ret 

;input:
; edx = position begin 'symbol' struct
; eax = position end 'symbol' struct
FindHelpId:
; ecx = word_n
; ebx = l_pos
  mov [help_id],-1

  push ebx ecx
    xor ebx,ebx
    mov bl,byte[edx]
    shl bx,2 ;ebx*=4
    add ebx,FkPos
    mov ecx,dword[ebx]
    cmp ecx,0
    jl .if_0e ;if( (word_n=FkPos[(unsigned char)tf[0]])>-1 ){
      push esi edi
      mov ebx,ecx ;l_pos=word_n;
      ColToIndexOffset ecx,esi
      push cx
      mov cl,byte[esi]
      @@:
	cmp ebx,dword[ColWords] ;while(l_pos<ColWords
	jge @f
	;ColToIndexOffset ecx,esi
	ColToIndexOffset ebx,edi
	cmp cl,byte[edi] ;&& Col[keyw[l_pos]].Text[0]==Col[keyw[word_n]].Text[0])
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
	    ColToIndexOffset ebx,edi
	    inc ebx
	    cmp byte[edi+esi],dl ;&& Col[keyw[l_pos-1]].Text[pos]!=tf[i])
	    je .wh_0e
	      dec ebx ;l_pos--;
	    jmp .wh_0b
	  .wh_0e:

	  .wh_1b:
	    cmp ebx,ecx ;while(l_pos>word_n
	    jle .wh_1e
	    ColToIndexOffset ecx,edi
	    cmp byte[edi+esi],dl
	    je .wh_1e
	      inc ecx ;word_n++;
	    jmp .wh_1b
	  .wh_1e:
	pop dx

	cmp ecx,ebx ;if(word_n==l_pos) break;
	je @f
	call IteratNext ;pos++;
	cmp edx,eax ;for(...;i<strlen;...)
	je @f ;jge
	inc esi
	jmp @b
      @@:
      pop edi esi

      mov dword[help_id],ecx
      ;return keyw[word_n];

    .if_0e:
  pop ecx ebx
  ret

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
