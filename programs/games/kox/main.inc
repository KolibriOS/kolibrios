;#**************************
;#*    X-0-X    =  MAIN()
;#************************** 21/10/2007
cB     equ cBLACK
cW     equ cWHITE

macro main_loop
{

; Получить стандартные цвета окон
;========================================
mov eax, 48
mov ebx, 3
mov ecx, colors
mov edx, 40
int 0x40



; функция отрисовки окна
;========================================
draw_window:
        mov     eax,12                ; функция 12:     сообщить системе
        mov     ebx,1                 ; подфункция 1,  начало перерисовки
        int     0x40
                                      ; =====  ОКНО !!!
        mov     eax,0                 ; функция 0: определить и вывести окно
        mov     ebx,100*65536+250     ; [x начальный] *65536 + [x размер]
        mov     ecx,ebx  ;,100*65536+250     ; [y начальный] *65536 + [y размер]
        mov     edx, 0x34FFFFFF

        mov     esi, 0x808899AA       ; цвет заголовка RRGGBB
        mov     edi, caption          ; заголовок
        int     0x40

;-----------------------------------#-----------------------------------------
; SCENE OF GAME - PLAY YARD - BOARD - DESK - PLAYING FIELD
;---------------
 playing_field                        ;macro in macs.inc
;-------------------------------------------------------------------------------
;                              ----КНОПКИ----
;-------------------------------------------------------------------------------
;buttons:
        mov     eax,8                 ;
        mov     ebx,200*65536+35      ;
        mov     ecx,15*65536+20       ;
        mov     edx,200               ;== NEW GAME ==
        mov     esi, [colors+0x18]
        int     0x40
;--------------------------------------------
        mov     eax,4                 ;  написать текст на кнопке NEW
        mov     ebx,210*65536+20
        mov     ecx,[colors+0x1C]
        mov     edx,textend
        mov     esi,3
        int     0x40
 endbut:
        mov     eax,12
        mov     ebx,2
        int     0x40
;============================= draw_window: END
 event_wait:
        mov     eax,23        ;==СОБЫТИЯ==
        mov     ebx,100       ;ждать секунду
        int     0x40
;-------------------------------------------------------------------------------
; тип события возвращён в eax, далее проверяем, какое событие произошло
;-------------------------------------------------------------------------------
        cmp     eax,1        ; запрос на перерисовку?
        je      redraw
        cmp     eax,2        ; нажата клавиша клавиатуры?
        je      key
        cmp     eax,3        ; нажата кнопка в окне программы?
        je      button
        cmp     eax,6        ; mouse - click or move?
        je      MOUSE
        jmp     event_wait   ; возвращаемся к началу цикла ожидания событий
                             ; если событий  нет
;-------------------------------------------------------------------------------
;  обработка событий
;-------------------------------------------------------------------------------
redraw:
        call    draw_window  ; вызываем функцию draw_window и
        jmp     event_wait   ; возвращаемся назад к циклу ожидания
key:
        mov     eax,2        ; считываем код нажатой клавиши. Возвращен в ah.
        int     0x40         ; Клавиша должна быть прочитана для очистки
        jmp     event_wait   ; возврат к event_wait
button:
        mov     eax,17       ; считываем идентификатор нажатой кнопки
        int     0x40         ; возвращен в ah.
;-------------------------------------------------------------------------------
; смотрим, какая кнопка была нажата и соответствующим образом реагируем.
;-------------------------------------------------------------------------------
        cmp     ah, 1          ; кнопка с id=1 ("закрыть")?
        jne     noclose        ;
  clos0:                       ;
        mov     eax,-1         ; функция -1: завершить программу
        int     0x40           ;
  noclose:                     ; кнопка id=200( "New" )
        mov dword[pox],0
        mov dword[pox+4],0     ;clear memory
        mov dword[pox_],0      ;rest and other stuff
        mov word[poxl],0       ; bits of pole X
        mov word[pool],0       ; bits of pole O
        ;mov byte[fill],0       ; обнуляем счетчик ходов
        ;mov byte[xwin],0       ; flag - x winner
        ;mov byte[owin],0       ; flag - o winner
        jmp     redraw         ; чистое игровое поле ==>>
;-------------------------------------------------------------------------------
; смотрим, где была нажата мышь
;-------------------------------------------------------------------------------
 MOUSE:
        mov eax,37      ; мышью..
        mov ebx,2
        int 0x40
        cmp eax,0       ; ..ничего не нажато - ждать событий
        je  event_wait
                        ;нажата кнопка мыши
        mov eax,37      ; cursor coords
        mov ebx,1
        int 0x40        ;eax=x*65536+y
        mov [px],eax    ;[px][py]  -  координата ткнутого места
	
	cmp [px],word 40
        jb  event_wait	
	
	cmp [px],word 200
        ja  event_wait	
	
	cmp [py],word 40
        jb  event_wait	
	
	cmp [py],word 200
        ja  event_wait	

          xor eax,eax
     g11:                      ;1st cell======
          cmp word[px],92
          ja g12               ;>92
          cmp word[py],92
          ja g12               ;>92

          ;mov dword[idx],0     ;N=(0..8)
          jmp puts

     g12:                      ;2nd cell========
          inc al
          cmp word[py],145
          ja g13               ;>142
          cmp word[px],92
          ja g13               ;>92
          ;mov dword[idx],1     ;N=(0..8)
          jmp puts
     g13:                      ;3=========
          inc al
          cmp word[px],92
          ja g21               ;>92
          ;mov dword[idx],2     ;N=(0..8)
          jmp puts
     g21:                      ;4========
          inc al
          cmp word[px],145
          ja g22
          cmp word[py],92
          ja g22
          ;mov dword[idx],3     ;N=(0..8)
          jmp puts
     g22:                      ;5=======
          inc al
          cmp word[px],145
          ja g23
          cmp word[py],145
          ja g23
          ;mov dword[idx],4     ;N=(0..8)
          jmp puts
     g23:                      ;6=======
          inc al
          cmp word[px],142
          ja g31               ;>142
          ;mov dword[idx],5     ;N=(0..8)
          jmp puts
     g31:                      ;7=======
          inc al
          cmp word[py],92
          ja g32
          ;mov dword[idx],6     ;N=(0..8)
          jmp puts
     g32:                      ;8======
          inc al
          cmp word[py],145
          ja g33
          ;mov dword[idx],7     ;N=(0..8)
          jmp puts
     g33:                      ;9======
          inc al
          ;mov dword[idx],8     ;N=(0..8)

   puts:                            ;уже определили клетку
           push eax
           pop dword[idx]
          mov esi,dword[idx]        ;
          cmp byte[pox+esi],0       ;поле свободно? (0-свободное поле)
          jne event_wait            ;возвращаемся назад к циклу ожидания

          shl esi,2                 ;*4
          mov eax,dword[pole+esi]
          mov dword[px],eax           ;x coord from tabl ->px
          ;mov ax,word[pole+esi+2]
          ;mov word[py],ax           ;y coord from tabl ->py

          call x_put                ;"X"

          mov esi,dword[idx]        ;esi - cell number  =0..8
          mov byte[pox+esi],'x'
          inc byte[fill]            ;turn+1

          xor eax,eax               ;eax=0
          inc eax                   ;bit=1       eax=1
          mov ecx,dword[idx]        ;  esi=n
          shl  ax,cl                ; <<n-bit   bit[n]=1
          or word[poxl],ax          ;set bit in [poxl]

          call x_win                ;проверка выигрыша Х
          cmp byte[xwin],1          ;флаг победы установлен?
          jne del5                  ;нет - перескок на delay

        mov     eax,4                 ; функ 4: написать текст 'WINNER!'
        mov     ebx,18*65536+215      ; [x нач] *65536 + [y нач]
        mov     ecx, [colors+0x20]    ; цвет текста RRGGBB
        mov     edx,texts             ; указатель на начало текста
        mov     esi,textend-texts     ; длина текста в байтах
        int     0x40                  ;
        jmp   event_wait              ; возвращаемся назад к циклу ожидания

;===========================================================
 del5:    mov eax,5                 ;=== delay 0,5 sec ===
          mov ebx,50
          int 0x40

          cmp byte[fill],8    ;все поля заняты - ходить некуда
          jnb event_wait      ;возвращаемся назад к циклу ожидания - конец.

   rnew:                      ;
          call rndnext              ; случайное число в eax
          and eax, 0111b            ; eax = 0..7
          mov esi,eax               ; eax->esi
          call rndnext              ; случайное число в eax
          and eax, 01b              ; eax= 0..1
          add esi,eax               ; esi+=eax
                                    ;esi=0..8 - index
          cmp byte[pox+esi],0       ;случайное поле свободно?
          jne rnew                  ;нет - новое случайное число

          cmp byte[pox+4],0         ;центр свободен?
          jne @F                    ;нет - случайный ход (уже в esi)
          mov esi,4                 ;да  - ходим в центр (4) вместо случ.
     @@:
          mov dword[idx],esi        ;сохраним в idx
                                    ;esi=0..8 - index
          call o_com                ;проверка 24х комбинаций XX0
          call x_com                ;проверка 24х комбинаций 00*

          mov esi,dword[idx]        ;idx-индекс клетки(0..8) куда ход
          shl esi,2                 ; esi*4 - поле состоит из записей по dword
          mov eax,dword[pole+esi]     ;
          mov dword[px],eax           ;x coord from tabl ->px
        ;  mov ax,word[pole+esi+2]   ;
        ;  mov word[py],ax           ;y coord from tabl ->py
          call o_put                ; рисуем нолик 'O'

          mov esi,dword[idx]
          mov byte[pox+esi],'o'
          inc byte[fill]            ;turn++

          xor eax,eax               ;eax=0
          inc eax                   ;bit=1       eax=1
          mov ecx,dword[idx]        ;  ecx=n
          shl  ax,cl                ; <<n-bit   bit[n]=1
          or word[pool],ax          ;set bit in [pool]

          call o_win
          cmp byte[owin],1
          jne event_wait

        mov     eax,4                 ; функ 4: написать текст 'LOOSER!'
        mov     ebx,48*65536+215      ; [x нач] *65536 + [y нач]
        mov     ecx, [colors+0x20]    ; цвет текста RRGGBB
        mov     edx,lootxt            ; указатель на начало текста
        mov     esi,text-lootxt       ; длина текста в байтах
        int     0x40                  ;
        jmp   event_wait              ; возвращаемся назад к циклу ожидания
;===========================================--
;
}


