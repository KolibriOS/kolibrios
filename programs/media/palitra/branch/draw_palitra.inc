draw_palitra:
    ; Функция 13 - нарисовать прямоугольник
    ;mcall   SF_DRAW_RECT, PALITRA_XW, PALITRA_YW, [sc.work]

    cmp     [renmode],MODE_PIPET
    je      cyrcle_draw
    ;cmp     [renmode],1
    ;je      picker_draw
    cmp     [renmode],MODE_PALITRA
    je      sheme_draw
    ret

    ;.................................................................................................
    ; Отрисовка круговой диаграммы
    ;.................................................................................................
    cyrcle_draw:
    ;mov     eax,13
    ;mov     edx,0x666666
    ;mov     ebx,108*65536+152
    ;mov     ecx,125*65536+33
    ;mcall
    ;mov     edx,0xF3F3F3
    ;mov     ebx,109*65536+150
    ;mov     ecx,126*65536+31
    ;mcall

    ;mov     eax,4                         ; 4 - вывести строку текста в окно
    ;mov     ebx,115 shl 16+131            ; [координата по оси x]*65536 + [координата по оси y]
    ;mov     ecx,0x666666                  ; 0xX0RRGGBB (RR, GG, BB задают цвет текста)
    ;mov     edx,hint                      ; указатель на начало строки
    ;mov     esi,24                        ; выводить esi символов
    ;mcall
    ;mov     ebx,125 shl 16+144            ; [координата по оси x]*65536 + [координата по оси y]
    ;mov     edx,hint2                     ; указатель на начало строки
    ;mov     esi,21                        ; выводить esi символов
    ;mcall

    ;mov     edx,[color]
    ;mov     ebx,109*65536+150
    ;mov     ecx, 10*65536+150
    ;mcall
    
PIPET_CELLW        = 13
PIPET_AREA_W       = PIPET_CELLW*PIPET_CELL_COUNT_X
PIPET_AREA_H       = PIPET_CELLW*PIPET_CELL_COUNT_Y
PIPET_CELL_COUNT_X = 15
PIPET_CELL_COUNT_Y = 13
PIPET_SELECTED_X   = PIPET_AREA_W/2+PALITRA_X
PIPET_SELECTED_Y   = PIPET_AREA_H/2+DRAWY+2

    call    mouse_local                   ; получаем координаты мыши относительно окна
    mov     ecx, [mouse_x]                ; заносим в регистр
    mov     edx, [mouse_y]                ; заносим в регистр
    cmp     ecx, WIN_W-10
    jg      cyrcle_draw_2
    cmp     edx, WIN_H
    jle     end_cyrcle_draw

    cyrcle_draw_2:
        mcall   SF_SLEEP,10
    call    desktop_get
    call    mouse_global

    mov     ebx,PALITRA_X*65536+PIPET_CELLW
    mov     esi,0                         ; counter=8
    circle_loop:
      mov     ecx, (DRAWY+1)*65536+PIPET_CELLW
      mov     edi,0
      circle_loop2:
        call    circle_pixel_read
        mcall   SF_DRAW_RECT
        add     ecx,PIPET_CELLW shl 16
        inc     edi
        cmp     edi,PIPET_CELL_COUNT_Y
        jne     circle_loop2

      add     ebx,PIPET_CELLW shl 16
      inc     esi
      cmp     esi,PIPET_CELL_COUNT_X
      jne     circle_loop

    mcall   SF_DRAW_RECT, <PIPET_SELECTED_X-10, PIPET_CELLW+4>, <PIPET_SELECTED_Y-10, PIPET_CELLW+4>, 0xFF0000
    mov     ecx, [mouse_x]
    mov     edx, [mouse_y]
    inc     ecx
    inc     ecx
    inc     edx
    mov     ebx, edx
    imul    ebx, [desctop_w]
    add     ebx, ecx
    mcall   SF_GET_PIXEL
    mov     edx,eax
    mcall   SF_DRAW_RECT, <PIPET_SELECTED_X-8, PIPET_CELLW>, <PIPET_SELECTED_Y-8, PIPET_CELLW>
    ret
    end_cyrcle_draw:
    mcall   SF_DRAW_RECT, <PALITRA_X-1, PIPET_AREA_W+2>, <DRAWY, PIPET_AREA_H+2>, [sc.work_graph]
    ret

    circle_pixel_read:
    push    ecx ebx
    mov     ecx, [mouse_x]
    mov     edx, [mouse_y]
    inc     ecx
    add     ecx, esi
    cmp     ecx, 6
    jl      _cpr_exit
    sub     ecx, 6
    inc     edx
    add     edx, edi
    cmp     edx, 6
    jl      _cpr_exit
    sub     edx, 6

    mov     ebx, edx
    imul    ebx, [desctop_w]
    add     ebx, ecx
    mcall   SF_GET_PIXEL
    mov     edx,eax
    pop     ebx ecx
    ret

    _cpr_exit:
    xor     edx,edx
    pop     ebx ecx
    ret

    ;picker_draw:
    ;ret

    ;.................................................................................................
    ; Отрисовка фона под кнопки
    ;.................................................................................................

    sheme_draw:
    mov     eax,SF_DRAW_RECT  ; Функция 13 - нарисовать прямоугольник
    mov     edx,0x222222                  ; цвет
    mov     ecx, PALITRA_YW               ; Начальные координаты по y [к-та y]*65536 + [размер]
    mov     esi,2                         ; Счетчик линий подложек
    re_draw:
    mov     ebx,PALITRA_XW             ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     edi,2                         ; Счетчик количества подложек
    for_fon_loop:
      int     0x40                        ; Прерывание
      add     ebx,(PALITRA_W+2) shl 16               ; Смещаем положение линии по х
      dec     edi                         ; Уменьшаем счетчик кнопок
      cmp     edi,0                       ; Сравниваем с нулем
    jnz     for_fon_loop                  ; Если не ноль то в начало цикла
    dec     esi                           ; Уменьшаем его
    cmp     esi,0                         ; Сравниваем с нулем
    mov     ecx,(PALITRA_W+DRAWY+3) shl 16+PALITRA_W              ; Начальные координаты по y [к-та y]*65536 + [размер]
    jnz     re_draw                       ; Если не ноль то в начало цикла

    ;.................................................................................................
    ; Отрисовка кнопок по циклу
    ;.................................................................................................
    mov     eax,SF_DRAW_RECT  ; Функция 13 - нарисовать прямоугольник
    mov     edx,0x0FFFFFFF                ; цвет
    mov     esi,4                         ; Счетчик количества абзацев (#4,8)
    mov     ebx,(PALITRA_X-CELLW-3) shl 16+CELLW               ; Начальные координаты по x [к-та x]*65536 + [размер]
    for_abz:
      ;;push    esi                       ; Сохраняем значение счетчика линий в стек
      cmp     esi,2
      jne     x2_line
      mov     ebx,(PALITRA_X-CELLW-3) shl 16+CELLW
      x2_line:
      add     ebx,3 shl 16                ; Смещаем положение линии по x
      mov     edi,8                       ; Счетчик количества кнопок в строке
      for_stolbik:
        push    edi                       ; Сохраняем значение счетчика линий в стек
        mov     edi,8                     ; Счетчик количества кнопок в строке
        mov     ecx,  (1-CELLW/3) shl 16+CELLW          ; Начальные координаты по y [к-та y]*65536 + [размер]
        cmp     esi,2
        jg      y2_line                   ; Если больше 4 то игнорим
        mov     ecx,(PALITRA_W+4-(CELLW/3)) shl 16+CELLW
        y2_line:
        add     ebx,(CELLW+1) shl 16              ; Смещаем положение линии по x
        for_loop:
          add     ecx,(CELLW+1) shl 16            ; Смещаем положение линии по y
          int     0x40                    ; Прерывание
          call    sub_red
          dec     edi                     ; Уменьшаем счетчик кнопок
          cmp     edi,0                   ; Сравниваем с нулем
          jnz     for_loop                ; Если не ноль то в начало цикла
      call    sub_green
      pop     edi                         ; Иначе восстанавливаем счетчик линий
      dec     edi                         ; Уменьшаем его
      cmp     edi,0                       ; Сравниваем с нулем
      jnz     for_stolbik                 ; Если не ноль то в начало цикла
    call    sub_blue
    ;;pop     esi                         ; Иначе восстанавливаем счетчик линий
    dec     esi                           ; Уменьшаем его
    cmp     esi,0                         ; Сравниваем с нулем
    jnz     for_abz                       ; Если не ноль то в начало цикла
    ret                                   ; Возвращаем управление
;end_draw_palitra-------------------------------------------------------------------------------------


sub_red:
    ;.................................................................................................
    ; Расчёт смещений красного цвета в генерации цвета палитры
    ;.................................................................................................
    push    eax
    mov     eax,[pnext]
  ;sub_red_0                              ; palitra
    cmp     al,0
    jne     sub_red_1
    sub     edx,0x40
    jmp     sub_red_e
  sub_red_1:                              ; random
    cmp     al,1
    jne     sub_red_2
    sub     edx,0x20
    jmp     sub_red_e
  sub_red_2:                              ; gree
    cmp     al,2
    jne     sub_red_3
    sub     edx,0x010101
    jmp     sub_red_e
  sub_red_3:                              ; red
    cmp     al,3
    jne     sub_red_4
    sub     edx,0x0101
    jmp     sub_red_e
  sub_red_4:                              ; green
    cmp     al,4
    jne     sub_red_5
    sub     edx,0x010001
    jmp     sub_red_e
  sub_red_5:                              ; blue
    cmp     al,5
    jne     sub_red_e
    sub     edx,0x010100
    jmp     sub_red_e
  sub_red_e:
    pop     eax
    ret                                   ; Возвращаем управление
;end_sub_red------------------------------------------------------------------------------------------

sub_green:
    ;.................................................................................................
    ; Расчёт смещений красного цвета в генерации цвета палитры
    ;.................................................................................................
    push    eax
    mov     eax,[pnext]
  ;sub_green_0
    cmp     al,0
    jne     sub_green_1
    sub     edx,0x4000
    jmp     sub_green_e
  sub_green_1:
    cmp     al,1
    jne     sub_green_e
    sub     edx,0x2000
    ;jmp     sub_green_e
  sub_green_e:
    pop     eax
    ret                                   ; Возвращаем управление
;end_sub_green----------------------------------------------------------------------------------------


sub_blue:
    ;.................................................................................................
    ; Расчёт смещений красного цвета в генерации цвета палитры
    ;.................................................................................................
    push    eax
    mov     eax,[pnext]
  ;sub_blue_0
    cmp     al,0
    jne     sub_blue_1
    sub     edx,0x600000
    jmp     sub_blue_e
  sub_blue_1:
    cmp     al,1
    jne     sub_blue_e
    sub     edx,0x400000
    ;jmp     sub_blue_e
  sub_blue_e:
    pop     eax
    ret                                   ; Возвращаем управление
;end_sub_blue-----------------------------------------------------------------------------------------