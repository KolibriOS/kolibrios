;#**************************
;#*        LIGHTS       (c)* main.inc
;#************************** 15/10/2007
L0     equ 0x0000F8    ; light off
L1     equ 0xFFFF00    ; light on
LAG    equ 2
deltaX equ 35
deltaY equ 35
posX   equ 50
posY   equ 50

macro main_loop
{

; функция отрисовки окна
draw_window:
        mov     eax,12                ; функция 12: сообщить системе о
                                      ; состоянии перерисовки окна
        mov     ebx,1                 ; подфункция 1, начало перерисовки
        int     0x40
                                      ; =====  ОКНО !!1
        mov     eax,0                 ; функция 0: определить и вывести окно
        mov     ebx,100*65536+270     ; [x начальный] *65536 + [x размер]
        mov     ecx,100*65536+280     ; [y начальный] *65536 + [y размер]
        mov     edx,0x02ccddff        ; цвет рабочей области RRGGBB
                                      ; 0x02000000 = тип окна 2
        mov     esi,0x808899AA        ; цвет области заголовка RRGGBB
                                      ; 0x80000000 = цвет перетекает
        mov     edi,0xFFAAcc          ; цвет рамки RRGGBB
        int     0x40
                                      ; НАДПИСЬ в заголовке Lights v0.001
        mov     eax,4                 ; функция 4: написать текст в окне
        mov     ebx,8*65536+8         ; [x начальный] *65536 + [y начальный]
        mov     ecx,0x00224422        ; цвет текста RRGGBB
                                      ; старший байт (0x00) - размер шрифта
                                      ; (маленький)
        mov     edx,text              ; указатель на начало текста
        mov     esi,texts-text        ; длина текста в байтах
        int     0x40

        xor   eax,eax                 ; НАДПИСЬ в окне!!!
        mov   ecx,eax
  lb:                                 ;loop ecx
        mov   edi,ecx                 ;
        shl   edi,3                   ;
        add   edi,tBut                ;
        add   al,byte[edi+1]          ;
                                      ;
        inc   ecx                     ;
        cmp   ecx,25                  ;
        jne   lb                      ;ecx<25

        cmp   eax, 0xE7    ;25
        jne   goon
        ;int3
        ;call Bin2Hex
        mov     eax,4                 ; функция 4: написать текст в окне
        mov     ebx,80*65536+250       ; [x начальный] *65536 + [y начальный]
        mov     ecx,0x10FF22FF        ; цвет текста RRGGBB
        ;                              ; старший байт (0x10) - размер шрифта
        ;                             ; (большой)
        ;mov     edx,txthex
        ;mov     esi,17
        mov     edx,texts             ; указатель на начало текста
        mov     esi,textend-texts     ; длина текста в байтах
        int     0x40

   goon:
;-------------------------------------------------------------------------------
;                              ----КНОПКИ----
;        button()
;-------------------------------------------------------------------------------

        mov     eax,8                 ; функция 8: определить и вывести кнопку
        mov     ebx,(200-19)*65536+12 ; [x начальный] *65536 + [x размер]
        mov     ecx,5*65536+12        ; [y начальный] *65536 + [y размер]
        mov     edx,100               ; идентификатор кнопки  =1
        mov     esi,0xFF6611          ; цвет кнопки RRGGBB
        int     0x40                  ; ===КНОПКА ЗАКРЫТИЯ ОКНА===


   LoopBegin  labl,0                  ;ecx - loop index
                                      ;add  ecx,5
        mov   edi,ecx
        shl   edi,3      ;edi*8 -index of table
        add   edi,tBut

        xor     ebx,ebx
        mov      bl,byte[edi+2]         ;X
        shl     ebx,16                   ;*65536=256*256
        add     ebx,deltaX-LAG               ;dX
        xor     ecx,ecx
        mov      cl,byte[edi+3]         ;Y
        shl     ecx,16                   ;*65536=256*256
        add     ecx,deltaY-LAG               ;dY
        xor     edx,edx
        mov      dl,byte[edi]           ;N

        xor     esi,esi
        cmp      byte[edi+1], 0         ;Color = 0/1
        je      lit_off
        mov     esi,L1                  ;on
        jmp     lend
    lit_off:
        mov     esi,L0                  ;off
    lend:
        mov     eax,8                    ;8-BUTTON
        int     0x40
   LoopEnd    labl,25

;=============================================================
endbut:
        mov     eax,12                ; функция 12: сообщить системе о
                                      ; состоянии перерисовки окна
        mov     ebx,2                 ; подфункция 2, перерисовка окончена
        int     0x40
        ;ret                           ; конец функции  - это не функция!

event_wait:
        ;mov     eax,10       ; функция 10:=0A ожидание события
        mov     eax,23
        mov     ebx,100
        int     0x40

; тип события возвращён в eax, далее проверяем, какое событие произошло
;-------------------------------------------------------------------------------
        cmp     eax,1        ; запрос на перерисовку?
        je      redraw
        cmp     eax,2        ; нажата клавиша клавиатуры?
        je      key
        cmp     eax,3        ; нажата кнопка в окне программы?
        je      button

        jmp     event_wait   ; возвращаемся к началу цикла ожидания событий

; после того, как событие идентифицировано, его надо обработать
;-------------------------------------------------------------------------------
redraw:
        call    draw_window  ; вызываем функцию draw_window и
        jmp     event_wait   ; возвращаемся назад к циклу ожидания

key:
        mov     eax,2        ; считываем код нажатой клавиши. Возвращен в ah.
        int     0x40         ; Клавиша должна быть прочитана для очистки
        cmp     ah,27
        je      clos0        ; exit on ESC
                             ;
        jmp     event_wait   ; возврат к event_wait

button:
        mov     eax,17       ; считываем идентификатор нажатой кнопки
        int     0x40         ; возвращен ah.

;-------------------------------------------------------------------------------
; смотрим, какая кнопка была нажата и соответствующим образом реагируем.
;-------------------------------------------------------------------------------
        cmp     ah,100         ; кнопка с id=100("закрыть")?
        jne     noclose
  clos0:
        mov     eax,-1         ; функция -1: завершить программу
        int     0x40           ; кнопка 5-29
  noclose:

        xor     ecx,ecx        ;
        mov     cl,ah          ;
        sub     ecx,5          ; ecx - index for tBut

        mov     edi,ecx
        call    x_lit            ;+0 - центр
        mov     [tButIdx],edi    ;save  index

        mov     edi,[tButIdx]
        cmp     byte[edi+6],0
        je      next000
        mov     edi,ecx
        inc     edi
        call    x_lit       ;+1    справа

  next000:
        mov     edi,[tButIdx]
        cmp     byte[edi+5],0
        je      next001
        mov     edi,ecx
        dec     edi
        call    x_lit       ;-1    слева

  next001:
        mov     edi,[tButIdx]
        cmp     byte[edi+4],0
        je      next002
        mov     edi,ecx
        sub     edi,5
        call    x_lit       ;-5     сверху

  next002:
        mov     edi,[tButIdx]
        cmp     byte[edi+7],0
        je      next003
        mov     edi,ecx
        add     edi,5
        call    x_lit       ;+5     снизу
  next003:
        call    draw_window
        jmp     event_wait
;================================--
  x_lit:                      ; edi - index
        shl    edi,3          ; edi*8 -index of table tBut
        add    edi,tBut       ;
        not    byte[edi+1]    ; invert second byte = L0/L1
        ret                   ;
;================================--
        call    draw_window  ;наша кнопка;)
; здесь проверяем остальные кнопки
        jmp     event_wait   ;возвращаемся к циклу ожидания
}


