format MS COFF
public EXPORTS
section '.flat' code readable align 16

include '../../../macros.inc'
include 'strlen.inc'

MB_FIRST_BUT_ID equ 3 ;идентификатор 1-й кнопки на сообщении
MB_MAX_BUT equ 8 ;максимальное число кнопок в сообщении
MB_BUT_HEIGHT equ 20 ;высота кнопки на сообщении
CHAR_WIDTH equ 6 ;ширина 1-й буквы текста
MB_BUT_OTY_1 equ  5 ;выртикальный(е) отступ(ы) между: заголовком окна и текстом, текстом кнопками
;--- отступы на кнопках, указанные вопросами: <-?->, без учета текста ---
MB_BUT_OTX_1 equ  5 ;[<-?->Caption.....]
MB_BUT_OTX_2 equ 10 ;[<-?->Caption<-?->]
MB_BUT_OTX_3 equ 15 ;[<-?->Caption<-?->]<-?->
MB_TEXT_OFFSET equ 2 ;смещение, по которому начинаеться текст заголовка окна

align 4
start:
  mov eax,48
  mov ebx,3
  mov ecx,sc
  mov edx,sizeof.system_colors
  mcall
  mov eax,40
  mov ebx,0x27
  mcall

  ;-- clear id pressed button ---
  mov ebx,[mb_text]
  mov al,byte[ebx]
  add al,MB_FIRST_BUT_ID
  dec al
  mov byte[mb_key_foc],al

  push [mb_text]
  call MsgBoxReInit

align 4
red_win:
  mov eax,12
  mov ebx,1
  mcall

  xor eax,eax
  mov bx,word[mb_left]
  shl ebx,16
  mov bx,word[mb_width]
  mov cx,word[mb_top]
  shl ecx,16
  mov cx,word[mb_height]
  mov edx,[sc.work]
  or  edx,0x33000000
  mov edi,[mb_text]
  add edi,MB_TEXT_OFFSET ;Caption
  mcall

  mcall 9,mb_procinfo,-1
  mov eax,dword[mb_procinfo.client_box.width]

  sub eax,[but_width]
  shr eax,1
  add eax,MB_BUT_OTX_1
  mov [ot_left],eax

  mov eax,4 ;рисование текста
  mov bx,MB_BUT_OTX_1
  shl ebx,16
  mov bx,MB_BUT_OTY_1
  mov ecx,[sc.work_text]
  mov edx,[txtMsg]

  @@:
    mov esi,edx
    call strlen
    call linlen

    cmp eax,0
    je @f

    mov esi,eax
    push eax
      mov eax,4
      int 0x40
    pop eax

    add edx,eax
    cmp byte[edx],0
    je @f
    inc edx

    add ebx,10 ; move <--y-->
    jmp @b
  @@:

  call MsgBoxDrawAllBut
  mcall 12,2

align 4
still:
  mov eax,10
  mcall

  cmp al,1 ;изм. положение окна
  jz red_win
  cmp al,2
  jz key
  cmp al,3
  jz button

  jmp still

align 4
key:
  mcall 2
  cmp ah,13
  jne @f
    mov ah,[mb_key_foc]
    jmp close
  @@:

  cmp ah,176 ;Left
  jne .no_left
  cmp [mb_key_foc],MB_FIRST_BUT_ID
  jg @f
    mov al,[mb_key_max]
    add [mb_key_foc],al
  @@:
    dec [mb_key_foc]
    call MsgBoxDrawAllBut
  .no_left:

  cmp ah,9 ;Tab
  je @f
  cmp ah,179 ;Right
  je @f
	jmp .no_right
  @@:
  mov al,[mb_key_max]
  add al,MB_FIRST_BUT_ID
  cmp [mb_key_foc],al
  je .no_right
    inc [mb_key_foc]
    cmp [mb_key_foc],al
    jl @f
      mov [mb_key_foc],MB_FIRST_BUT_ID
    @@:
    call MsgBoxDrawAllBut
  .no_right:

  jmp still

align 4
button:
  mcall 17 ;получить код нажатой кнопки
  cmp ah,MB_FIRST_BUT_ID
  jge close
  cmp ah,1
  jne still

;.exit:
  mov ah,MB_FIRST_BUT_ID
  dec ah
close:
  sub ah,MB_FIRST_BUT_ID
  inc ah
  mov ebx,[mb_text]
  mov byte[ebx],ah

  ;*** call msgbox functions ***
  cmp ah,0
  je @f
  cmp dword[mb_funct],0
  je @f
    xor ebx,ebx
    mov bl,ah
    dec bl
    shl bx,2 ;bx=bx*4
    add ebx,dword[mb_funct]
    cmp dword[ebx],0
    je @f
      call dword[ebx]
  @@:
  mcall -1 ;выход из программы

align 4
MsgBoxDrawAllBut:
  mov ebx,[ot_left]
  shl ebx,16
  mov bx,word[ot_top]
  add bx,MB_BUT_OTY_1

  ; Buttons ...
  push edi
  mov edi,txtBut
  mov edx,[edi]

  mov ecx,MB_FIRST_BUT_ID
  @@:
    ;eax = len button caption text
    mov esi,edx
    call strlen

    cmp eax,0
    je @f

    call MsgBoxDrawButton
    inc ecx

    add edi,4
    cmp edi,endBut
    jge @f
    mov edx,[edi]

    push bx
    ror ebx,16
      imul esi,CHAR_WIDTH
      add esi,MB_BUT_OTX_3
      add ebx,esi ; ...
    ror ebx,16
    pop bx

    jmp @b
  @@:
  pop edi
  ret

;input:
; ecx = button id
align 4
MsgBoxDrawButton:
  push ecx

  ;button
  push eax ebx edx
    mov edx,ecx
    imul eax,CHAR_WIDTH
    add eax,MB_BUT_OTX_2

    mov esi,[sc.work_button]
    cmp cl,[mb_key_foc]
    jne @f
      mov esi,[sc.work_button_text]
    @@:

    mov cx,bx
    sub cx,MB_BUT_OTY_1
    shl ecx,16
    mov cx,MB_BUT_HEIGHT
    ror ebx,16
    sub bx,MB_BUT_OTX_1
    ror ebx,16
    mov bx,ax

    mov eax,8
    int 0x40
  pop edx ebx eax

  ;caption
  mov ecx,[sc.work_button_text]
  cmp esi,ecx
  jne @f
    mov ecx,[sc.work_button]
  @@:
  mov esi,eax
  mov eax,4
  int 0x40

  pop ecx
  ret

align 4
MsgBoxReInit:
  push ebp
  mov ebp,esp
    m2m dword[mb_text],dword[ebp+8]
  push eax ebx ecx edi esi

  mov esi,[mb_text]
  add esi,MB_TEXT_OFFSET
  call strlen
  inc esi ;add terminated zero
  add esi,eax ;add len of caption
  mov [txtMsg],esi

  ;--- go to first button text --- buttons 1...MB_MAX_BUT
  mov ebx,txtBut
  mov cx,MB_MAX_BUT
  @@:
    call strlen
    add esi,eax ;add len of caption
    inc esi ;add terminated zero

    mov dword[ebx],esi
    cmp byte[esi],0
    je @f
    add ebx,4
  loop @b
  @@:

  ;--- calc button top ---
  mov eax,1
  mov ebx,[txtMsg]
  @@:
    inc ebx
    cmp byte[ebx],0
    je @f
    cmp byte[ebx],13
    jne @b
    inc eax
    jmp @b
  @@:
  imul eax,10
  add eax,2*MB_BUT_OTY_1
  mov [ot_top],eax

  ;--- calc window height ---
  add eax,MB_BUT_OTY_1
  add eax,MB_BUT_HEIGHT
  add ax,5 ;height of border
  mov [mb_height],ax
  mcall 48,4 ;skin height
  add [mb_height],ax

  ;--- calc window top ---
  mov eax,14
  int 0x40
  mov word[mb_top],ax
  mov ax,word[mb_height]
  sub word[mb_top],ax
  shr word[mb_top],1
  ;---
  shr eax,16
  mov word[mb_left],ax
  mov ax,word[mb_width]
  sub word[mb_left],ax
  shr word[mb_left],1

  ;--- calc button width ---
  xor ebx,ebx
  xor ecx,ecx
  mov esi,[txtBut]
  mov byte[mb_key_max],0
  @@:
    cmp byte[esi],0
    je @f
    inc ecx
    call strlen
    add esi,eax
    inc esi
    inc byte[mb_key_max]
    add ebx,eax
    jmp @b
  @@:

  imul ebx,CHAR_WIDTH
  imul ecx,MB_BUT_OTX_3
  add ebx,ecx
  add ebx,MB_BUT_OTX_2
  sub ebx,MB_BUT_OTX_3
  mov [but_width],ebx

  pop esi edi ecx ebx eax
  pop ebp
  ret 4

;input:
; [esp+8] = pointer to msgbox text
align 4
MsgBoxCreate:
  push ebp
  mov ebp,esp
    m2m dword[mb_text],dword[ebp+8]
    push eax ebx ecx edx

    mov eax,51
    mov ebx,1
    mov ecx,start
    mov edx,dword[ebp+12];thread
    int 0x40

    mov dword[mb_funct],0
    pop edx ecx ebx eax
  pop ebp
  ret 8

;input:
; [esp+8] = pointer to msgbox functions pointers
align 4
MsgBoxSetFunctions:
  push ebp
  mov ebp,esp
    m2m dword[mb_funct],dword[ebp+8]
  pop ebp
  ret 4

;--- data ---
  txtMsg dd 0 ;указатель на начало текста сообщения
  txtBut rd MB_MAX_BUT ;указатель на начало текста подписей кнопок
  endBut:
  ot_left dd 0 ;отступ слева (для рисования кнопок)
  ot_top dd 0 ;отступ сверху (для рисования кнопок)
  but_width dd 0 ;ширина всех кнопок + зазоры между ними
  ;--- размеры на экране окна сообщения ---
  mb_left dw 0
  mb_width dw 300
  mb_top dw 0
  mb_height dw 50
  mb_key_foc db MB_FIRST_BUT_ID ;кнопка в фокусе (с учетом MB_FIRST_BUT_ID)
  mb_key_max db 1 ;число всех кнопок сообщения
;--------------------------------------------------
  sc system_colors
  mb_procinfo process_information ;указатель на структуру process_information родительского окна
  mb_text dd 0 ;msgbox_1 ;указатель данные для сообщения (которы передаються окну)
  mb_funct dd 0 ;pointer to functions
;--------------------------------------------------
align 16
EXPORTS:
  dd sz_mb_create, MsgBoxCreate
  dd sz_mb_reinit, MsgBoxReInit
  dd sz_mb_setfunctions, MsgBoxSetFunctions
  dd 0,0
  sz_mb_create db 'mb_create',0
  sz_mb_reinit db 'mb_reinit',0
  sz_mb_setfunctions db 'mb_setfunctions',0