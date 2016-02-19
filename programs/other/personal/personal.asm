;=============================================================================;
;============================[       HEADER       ]===========================;
;=============================================================================;
  use32
  org	 0x0

  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 0x100000
  dd	 0x100000
  dd	 0x0
  dd	 0x0

  include '../../macros.inc'
;=============================================================================;
;============================[       EVENTS       ]===========================;
;=============================================================================;
START:
    mcall   40,0xC0000027                 ; устанавливаем маску событий
    call    button_init                   ; устанавливаем параметры кнопок

    mov     eax,48                        ; Функция 48 - стили отображения окон
    mov     ebx,3                         ; Подфункция 3 - получить стандартные цвета окон.
    mov     ecx,sc                        ; Указатель на буфер размером edx байт, под структуру
    mov     edx,200                       ; Размер таблицы цветов (должен быть 40 байт)
    int     0x40                          ; Прерывание

    mov     eax,48                        ; Функция 48 - стили отображения окон
    mov     ebx,2                         ; Подфункция 3 - получить стандартные цвета окон.
    mov     ecx,sc                        ; Указатель на буфер размером edx байт, под структуру
    mov     edx,200                       ; Размер таблицы цветов (должен быть 40 байт)
    int     0x40                          ; Прерывание

    mov     eax,48                        ; Функция 48 - стили отображения окон
    mov     ebx,3                         ; Подфункция 3 - получить стандартные цвета окон.
    mov     ecx,sc                        ; Указатель на буфер размером edx байт, под структуру
    mov     edx,200                       ; Размер таблицы цветов (должен быть 40 байт)
    int     0x40                          ; Прерывание

red:
    call    draw_window
still:
    mov     eax,10                        ; функция 10 - ждать события
    int     0x40
    cmp     eax,1                         ; перерисовать окно ?
    je      red                           ; если да - на метку red
    cmp     eax,2                         ; нажата клавиша ?
    je      key                           ; если да - на key
    cmp     eax,3                         ; нажата кнопка ?
    je      button                        ; если да - на button
    cmp     eax,6
    je      mouse
    jmp     still                         ; если другое событие - в начало цикла
;end_still

key:                                      ; нажата клавиша на клавиатуре
    mov     eax,2                         ; функция 2 - считать код символа (в ah) (тут в принципе не нужна)
    int     0x40
    jmp     still                         ; вернуться к началу цикла
;end_key

button:
    mov     eax,17                        ; 17 - получить идентификатор нажатой кнопки
    int     0x40
    cmp     ah, 1                         ; если нажата кнопка с номером 1,
    jz      bexit                         ; выходим
    jmp     still
  bexit:
    mov     eax,-1                        ; иначе конец программы
    int     0x40
;end_button

mouse:
    ;       ; нажата ли лкм
    call    draw_edit
    call    mouse_local      ; получаем координаты мыши относительно окна
    call    button_calc      ; отслеживаем наведение курсора
    call    process_slot     ; получаем позицию в оконном стеке
    call    button_draw      ; отрисовываем кнопки
    jmp     still            ; уходим на ожидание события


;============================[        CODE        ]===========================;

    ret
;end_draw_result


;#___________________________________________________________________________________________________
;****************************************************************************************************|
; ГЛАВНЫЙ МОДУЛЬ ОТРИСОВКИ ОКНА И ЭЛЕМЕНТОВ ПРИЛОЖЕНИЯ                                               |
;----------------------------------------------------------------------------------------------------/
draw_window:
    mov     eax,12                        ; функция 12: означает, что будет рисоваться окно
    mov     ebx,1                         ; 1,начало рисования
    int     0x40                          ; Прерывание



    mov     eax,48                        ; Функция 48 - стили отображения окон.
    mov     ebx,4                         ; Подфункция 4 - возвращает eax = высота скина.
    int     0x40                          ; Прерывание
    mov     ecx,eax                       ; Запоминаем высоту скина

    xor     eax,eax                       ; Очищаем eax (mov eax,0) (Функция 0)
    mov     ebx,200 shl 16+240            ; [координата по оси x]*65536 + [размер по оси x]
    add     ecx,200 shl 16+280            ; Высота скина + [координата по y]*65536 + [размер по y]
    mov     edx,[sc.win_body]             ; Видимо стиль окна по дефолту
    or      edx,0x34000000                ; Или окно со скином фиксированных размеров
    mov     edi,title                     ; Заголовок окна
    int     0x40                          ; Прерывание

    call    process_slot                  ; получаем позицию в оконном стеке
    call    button_draw
    call    draw_edit
    call    list_draw

    mov     eax,12                        ; функция 12: означает, что будет рисоваться окно
    mov     ebx,2                         ; 1,начало рисования
    int     0x40                          ; Прерывание

    ret



;===================================[ DATA ]==================================;
include   'inc/mouse.inc'                 ; мышь
include   'inc/process.inc'               ; инфо о процессе
include   'inc/dtp.inc'                   ; структура новой таблицы
include   'inc/button.inc'                ; самописные кнопки
include   'inc/edit.inc'                  ; эмуляция неактивного едита
include   'inc/list.inc'                  ; отрисовка списка

;; window -------------------------------------------------
    sc          new_colors                ; новая таблица цветов
    title       db 'Color Table',0        ; заголовок
    mouse_x     dd 0                      ; хранит глобальную х координату мыши
    mouse_y     dd 0                      ; хранит глобальную у координату мыши
    mouse_l     dd 0                      ; левая кнопка 1 - нажата 0 - нет
    win_slot    dd 0                      ; 0- окно не на верху, 1- на верху
    buffer      rb  80                    ; под 9 функцию

;; button -------------------------------------------------
    bnext       new_button
    bback       new_button
    arrowa db '<',0
    arrowb db '>',0
;; edit ---------------------------------------------------
    edit_cnt    dd 1                      ; counter
    edit_win    db 'WINDOW',0             ; 1
    edit_btn    db 'BUTTON',0             ; 2
    edit_gui    db 'ELEMENT',0            ; 3
    edit_cld    db 'SUPPORT',0            ; 4

;; list win -----------------------------------------------
    text_frame    db 'Frame (Activate):',0
    text_inframe  db 'Frame (Inactivate):',0
    text_fcframe  db 'Frame (Focus):',0
    text_face     db 'Face (Activate):',0
    text_inface   db 'Face (Inactivate):',0
    text_fcface   db 'Face (Focus):',0
    text_border   db 'Border (Activate):',0
    text_inborder db 'Border (Inactivate):',0
    text_wtext    db 'Text:',0
    text_graytext db 'Graytext:',0
    text_title    db 'Title:',0
    text_body     db 'Body:',0
    text_reserved db 'Reserved:',0
    text_text     db 'Text (Activate):',0
    text_intext   db 'Text (Inactivate):',0
    text_fctext   db 'Text (Focus):',0
    text_3dlight  db '3D Light:',0
    text_3ddark   db '3D Dark:',0
    text_3dface   db '3D Face:',0
    text_shadow   db 'Shadow:',0
    text_select   db 'Select:',0
    text_p_face   db 'Panel Body:',0
    text_p_frame  db 'Panel Frame:',0
    text_p_text   db 'Panel Text:',0
    text_m_face   db 'Menu Body:',0
    text_m_frame  db 'Menu Frame:',0
    text_m_text   db 'Menu Text:',0
    text_h_face   db 'Hint Body:',0
    text_h_frame  db 'Hint Frame:',0
    text_h_text   db 'Hint Text:',0
    text_hex      db '#',0
;----------------------------------------------------------
I_END:
