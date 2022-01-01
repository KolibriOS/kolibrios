;#___________________________________________________________________________________________________
;****************************************************************************************************|
; Program Palitra (c) Sergei Steshin (Akyltist)                                                      |
;----------------------------------------------------------------------------------------------------|
;; Charset:DOS-866 Font:Courier New Size:9pt                                                         |
;.....................................................................................................
;; compiler:     FASM 1.69.31                                                                        |
;; version:      0.3.0                                                                               |
;; last update:  08/11/2012                                                                          |
;; e-mail:       dr.steshin@gmail.com                                                                |
;.....................................................................................................
;; History:                                                                                          |
;; 0.1.0 - Первая версия программы.                                                                  |
;; 0.2.0 - Исправлено попадание в сетку, берется ближайший по диагонали пиксель.                     |
;;       - Добавлены ползунки, для регулирования rgb составляющих цвета и вывод этих составляющих.   |
;;       - Убран вывод цвета в бинарном виде (пока за не надобностью и не актуальностью).            |
;;       - Мелкая косметика.                                                                         |
;; 0.3.0 - Добавлено переключение видов цветовых схем (кнопка NEXT)                                  |
;;       - Улучшены ползунки, производится обработка нажатия рядом с ползунком.                      |
;;       - Число сеток в цветовой схеме уменьшено с 6 до 4 (кратность 256, иначе дублирование цвета).|
;;       - Мелкая косметика.                                                                         |
;; 0.4.0 - Добавлено переключение между двумя цветами                                                |
;;       - Добавлен ползунок регулирования прозрачности (без визуализации).                          |
;;       - Добавлено выравнивание значений rgba по центру, в зависимости от длинны.                  |
;;       - Косметические правки.                                                                     |
;;       - Небольшая оптимизация.                                                                    |
;; 0.5.0 - Добавлена кнопка смены фона рабочего стола (от Leency).                                   |
;;       - Добавлено изменение фона рабочего стола градиентной заливкой.                             |
;;       - Косметические правки.                                                                     |
;;       - Небольшая деоптимизация.                                                                  |
;; 0.6.0 - Добавлена возможность запуска с параметрами                                               |
;;       - Добавлен режим H (hidden) производит замену фона рабочего стола градиентной заливкой.     |
;;       - Большая деоптимизация.                                                                    |
;; 0.7.0 - Добавлена пипетка - выбор на среднюю кнопку мыши                                          |
;; 0.7.5 - Нажатие правой клавишей мыши на ячейку с цветом устанавливает не основой, а дополн. цвет  |
;;       - Уменьшено мерцание при работе пипетки                                                     |
;; 0.7.6 - Добавлен режим B, который производит замену фона рабочего стола шумной заливкой (e-andrew)|
;.....................................................................................................
;; All rights reserved.                                                                              |
;;                                                                                                   |
;; Redistribution and use in source and binary forms, with or without modification, are permitted    |
;; provided that the following conditions are met:                                                   |
;;       * Redistributions of source code must retain the above copyright notice, this list of       |
;;         conditions and the following disclaimer.                                                  |
;;       * Redistributions in binary form must reproduce the above copyright notice, this list of    |
;;         conditions and the following disclaimer in the documentation and/or other materials       |
;;         provided with the distribution.                                                           |
;;       * Neither the name of the <organization> nor the names of its contributors may be used to   |
;;         endorse or promote products derived from this software without specific prior written     |
;;         permission.                                                                               |
;;                                                                                                   |
;; THIS SOFTWARE IS PROVIDED BY Sergei Steshin ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,      |
;; INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A        |
;; PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY DIRECT, |
;; INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    |
;; TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS       |
;; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT          |
;; LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS  |
;; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                      |
;....................................................................................................|


;#___________________________________________________________________________________________________
;****************************************************************************************************|
; ЗАГОЛОВОК ИСПОЛНЯЕМОГО ФАЙЛА ПРИЛОЖЕНИЯ ДЛЯ КОЛИБРИ ОС                                             |
;----------------------------------------------------------------------------------------------------/
  use32
  org    0
  db     'MENUET01'
  dd     1,START,I_END,I_MEM,stacktop,params,sys_path

  include '../../../macros.inc'
  include '../../../proc32.inc'
  include '../../../KOSfuncs.inc'
  include '../../../dll.inc'

  include 'draw_sliders.inc'
  include 'draw_utils.inc'
  include 'draw_palitra.inc'



  WIN_W  = 374            ; ширина окна
  WIN_H  = 251            ; высота окна
  WIN_X  = 250            ; координата х окна
  WIN_Y  = 190            ; координата у окна

  Left_Border=4
  SliderPanel_W = 110
  DRAWY  = 9

  CELLW  = 11; 11            ; not used yet, but has to be :)

  ICONX  = WIN_W - 39
  ICONS  = 18             ; icon size  
  SLIDEW = 25

  palitra_x = Left_Border+SliderPanel_W+12
  palitra_w = CELLW*(8)+8+1
  palitra_xw = palitra_x shl 16 + palitra_w
  palitra_yw = DRAWY shl 16 + palitra_w

START:
    mcall   SF_SYS_MISC,SSF_HEAP_INIT ; инициализация кучи
    mcall SF_SYS_MISC, SSF_MEM_OPEN, i18_name
    mov [icons18], eax
    mcall SF_SYS_MISC, SSF_MEM_OPEN, i18bg_name
    mov [icons18bg], eax

    stdcall dll.Load, @IMPORT
    or      eax,eax
    jnz     bexit
    mcall   SF_SET_EVENTS_MASK,EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE ; устанавливаем маску событий
    include 'params_init.inc'             ; обработка параметров командной строки

;#___________________________________________________________________________________________________
;****************************************************************************************************|
; ОСНОВНОЙ ЦИКЛ ПРОГРАММЫ - ОБРАБОТКА СОБЫТИЙ                                                        |
;----------------------------------------------------------------------------------------------------/
red:
    call draw_main                        ; вызываем перерисовку окна приложения
still:
    mcall   SF_WAIT_EVENT         ; функция 10 - ждать события
    cmp     eax,EV_REDRAW                 ; перерисовать окно ?
    je      red                           ; если да - на метку red
    cmp     eax,EV_KEY                    ; нажата клавиша ?
    je      key                           ; если да - на key
    cmp     eax,EV_BUTTON                 ; нажата кнопка ?
    je      button                        ; если да - на button
    cmp     eax,EV_MOUSE                  ; событие от мыши вне окна
    je      mouse                         ; если да - на button
    jmp     still                         ; если другое событие - в начало цикла
;end_still

key:                                      ; нажата клавиша на клавиатуре
    mcall   SF_GET_KEY    ; функция 2 - считать код символа (в ah) (тут в принципе не нужна)
    jmp     still                         ; вернуться к началу цикла
;end_key

mouse:
    cmp     [renmode],2
    jne     left
    call    cyrcle_draw
    jmp     center
  left:
    mcall   SF_MOUSE_GET,SSF_BUTTON
    cmp     al,1b
    jne     right
    mov     [mouse_f],1
    jmp     still
  right:
    cmp     al,10b
    jne     still
    mov     [mouse_f],2
    jmp     still
  center:
    mcall   SF_MOUSE_GET,SSF_BUTTON
    cmp     al,100b
    jne     still
    mov     [mouse_f],3
    mov     [color],edx
    call    draw_result
    jmp     still                         ; вернуться к началу цикла
;end_mouse

button:
    mcall   SF_GET_BUTTON         ; 17 - получить идентификатор нажатой кнопки
    cmp     ah, 1                         ; если нажата кнопка с номером 1,
    jz      bexit                         ; выходим
  ;обработка кнопки Next
    cmp     ah, 12                        ; если нажата кнопка NEXT
    jne     next_bg                       ; выходим
    inc     [pnext]                       ; увеличиваем при нажатии номер палитры
    mov     [renmode],0                   ; включаем цветовые схемы
    mov     eax,[pnext]                   ; заносим значение в еах
    cmp     al,6                          ; сравниваем с заявленным количеством палитр
    jne     next_redraw                   ; если не больше максимума то на вызов отрисовки
    xor     eax,eax                       ; иначе зануляем палитру на default
    mov     [pnext],eax                   ; и запоминаем что сбросили палитру на default
  next_redraw:
    call    draw_palitra                  ; РИСУЕМ ПАЛИТРУ
    jmp     still                         ; Уходим на ожидание другого события
  next_bg:
    cmp     ah, 14                        ; Кнопка BACKGROUND
    jne     next_bg2                      ; если не нажата то выходим
    call    set_background                ; иначе устанавливаем фон
    jmp     still                         ; и на ожидание события
  next_bg2:
    cmp     ah, 16                        ; Кнопка BACKGROUND
    jne     circle_bg                     ; если не нажата то выходим
    call    set_background2               ; иначе устанавливаем фон
    jmp     still                         ; и на ожидание события
  circle_bg:
    cmp     ah, 15                        ; Кнопка Круговая палитра
    jne     next_end                      ; если не нажата то выходим
    mov     [renmode],2                   ; включаем отрисовку круговой палитры
    call    draw_palitra                  ; РИСУЕМ ПАЛИТРУ
    jmp     still                         ; и на ожидание события
  next_end:
    cmp     ah,13                         ; COLOR SWAP
    jne     color_swap_end
    push    [color2]
    push    [color]
    pop     [color2]
    pop     [color]
    call    draw_result
    jmp     still                         ; И уходим на ожидание другого события
  color_swap_end:
    cmp     ah, 7                         ; Проверяем нажата кнопка с ID=7
    jne     color_button                  ; Если не нажата, то идём дальше
    call    mouse_get                     ; Иначе включаем обработчик мыши, чтобы считать значение цвета с палитры
    jmp     still                         ; И уходим на ожидание другого события
  color_button:                           ; РАСЧЁТ координат для ползунков RGBA
    push    eax                           ; запоминаем еах
    call    mouse_local                   ; получаем локальные координаты
    mov     ebx, 188;137                       ; нижняя граница ползунка по У
    mov     ecx,[mouse_y]                 ; занисим в есх значение курсора по У
    sub     ebx,ecx                       ; находим разность (т.е. куда смещается ползунок)
    mov     ecx, 2;3                         ; заносим в есх цифру 3 (256/3=85, где 85-высота ползунков)
    imul    ecx,ebx                       ; находим истинный параметр цвета с учётом масштаба ползунка---+
    pop     eax                           ; восстанавливаем еах                                          :
  ;red_button:                            ; Красный Трекбар                                              :
    cmp     ah, 8                         ; ID=8                                                         :
    jne     green_button                  ; если нет, то проверяем зелёный трекбар                       :
    mov     [cred],cl                     ; иначе присваиваем значение, красному цвету спектра    <------+
    call    set_spectr                    ; устанавливаем спектр
    jmp     still                         ; Уходим на ожидание другого события
  green_button:
    cmp     ah, 9
    jne     blue_button
    mov     [cgreen],cl
    call    set_spectr
    jmp     still                         ; Уходим на ожидание другого события
  blue_button:
    cmp     ah, 10
    jne     alpha_button
    mov     [cblue],cl
    call    set_spectr
    jmp     still                         ; Уходим на ожидание другого события
  alpha_button:
    cmp     ah, 11
    jne     still
    mov     [calpha],cl
    call    set_spectr
    jmp     still                         ; Уходим на ожидание другого события
  bexit:
    mcall SF_TERMINATE_PROCESS ; иначе конец программы
;end_button

;#___________________________________________________________________________________________________
;****************************************************************************************************|
; ГЛАВНЫЙ МОДУЛЬ ОТРИСОВКИ ОКНА И ЭЛЕМЕНТОВ ПРИЛОЖЕНИЯ                                               |
;----------------------------------------------------------------------------------------------------/
draw_main:
    ; функция 12: означает, что будет рисоваться окно
    mcall   SF_REDRAW,SSF_BEGIN_DRAW

    ; Функция 48 - стили отображения окон
    mcall   SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors

    ; Функция 48 - стили отображения окон
    mcall   SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
    mov     ecx,eax                       ; Запоминаем высоту скина

    mov     edi,[runmode]
    cmp     edi,2
    jne     no_hiddenmode
    mov     edi,hidden
    jmp     set_title
  no_hiddenmode:
    cmp     edi,3
    jne     no_dialogmode
    mov     edi,hidden
    jmp     set_title
  no_dialogmode:
    mov     edi,title                     ; Заголовок окна
  set_title:

    xor     eax,eax                       ; Очищаем eax (mov eax,0) (Функция 0)
    mov     ebx,WIN_X shl 16+WIN_W        ; [координата по оси x]*65536 + [размер по оси x]
    add     ecx,WIN_Y shl 16+WIN_H        ; Высота скина + [координата по y]*65536 + [размер по y] (168 для версии 0.2)
    mov     edx,[sc.work]                 ; Видимо стиль окна по дефолту
    or      edx,0x34000000                ; Или окно со скином фиксированных размеров

    int     0x40                          ; Прерывание


    call    draw_palitra                  ; РИСУЕМ ПАЛИТРУ
    call    draw_result                   ; РИСУЕМ РЕЗУЛЬТАТ

    mcall   SF_DEFINE_BUTTON, <palitra_x,palitra_w*2+1  > , <DRAWY,palitra_w*2+2>, 7+BT_HIDE ; palitra

    inc     edx
    mcall   , <10,22>, <56,128>           ; Рисуем невидимую кнопку под слайдером red
    add     ebx,25*65536                  ; Добавляем
    inc     edx                           ; ID = 9
    int     0x40                          ; Рисуем невидимую кнопку под слайдером green
    add     ebx,25*65536                  ; Добавляем
    inc     edx                           ; ID = 10
    int     0x40                          ; Рисуем невидимую кнопку под слайдером blue
    add     ebx,25*65536                  ; Добавляем
    inc     edx                           ; ID = 11
    int     0x40                          ; Рисуем невидимую кнопку под слайдером alpha

    ; Функция 8 - определить/удалить кнопку (СМЕНА ЦВЕТА)
    mcall   , <13,19>, <20,18>, 0x0D+BT_HIDE

    call    draw_bottom_panel
    call    draw_left_panel

    ; функция 12: означает, что будет рисоваться окно
    mcall SF_REDRAW,SSF_END_DRAW
    ret



;#___________________________________________________________________________________________________
;****************************************************************************************************|
; БЛОК ВСПОМОГАТЕЛЬНЫХ ПРОЦЕДУР И ФУНКЦИЙ ПРИЛОЖЕНИЯ                                                 |
;----------------------------------------------------------------------------------------------------/


    ;------------------------------------------------------------------------------------------------+
    draw_left_panel:                      ; Отрисовка боковой панели  SL97: На самом деле правой.
    ;.................................................................................................
    ; button_next_colorsheme
    mcall   SF_DEFINE_BUTTON, <ICONX,ICONS+3>, <DRAWY,ICONS+3>, 12+BT_HIDE

    ; palitra button                    ; ID = 14
    mcall , ,(DRAWY+150) shl 16 + ICONS+3, 14+BT_HIDE

    ; pipet button                        ; ID = 15
    mcall , , (DRAWY+150) shl 16 + ICONS+3, 15+BT_HIDE

    mov     ebx,[icons18bg]
    add     ebx,ICONS*ICONS*4*53
    mcall   SF_PUT_IMAGE_EXT, ebx, <ICONS,ICONS>, <ICONX+2,DRAWY+2>, 32, 0, 0

    add     ebx,ICONS*ICONS*4*(39-53)
    mov     edx,(ICONX+2)*65536+WIN_H-90
    mcall

    mov     ebx,[icons18]
    add     ebx,ICONS*ICONS*4*(53-1)
    sub     edx,40
    mcall

    stdcall DrawDeepRectangle, ICONX-1, DRAWY-1,   ICONS+5, ICONS+5, [sc.work_graph], [sc.work_graph]
    stdcall DrawDeepRectangle, ICONX,   DRAWY,     ICONS+3, ICONS+3, [sc.work_light], [sc.work_dark]

    stdcall DrawDeepRectangle, ICONX-1, DRAWY+109, ICONS+5, ICONS+5, [sc.work_graph], [sc.work_graph]
    stdcall DrawDeepRectangle, ICONX,   DRAWY+110, ICONS+3, ICONS+3, [sc.work_dark], [sc.work_light]
    stdcall DrawDeepRectangle, ICONX+1, DRAWY+111, ICONS+1, ICONS+1, 0xFFFfff, 0xFFFfff

    stdcall DrawDeepRectangle, ICONX-1, DRAWY+149, ICONS+5, ICONS+5, [sc.work_graph], [sc.work_graph]
    stdcall DrawDeepRectangle, ICONX,   DRAWY+150, ICONS+3, ICONS+3, [sc.work_light], [sc.work_dark]

    ;stdcall DrawRectangle3D, ICONX, DRAWY, 22, 22, [sc.work_light], [sc.work_dark]   ;Leency: draw rectangle around the button, buggy now

    ;mov     eax,13                        ; draw rect
    ;mov     ebx,266 shl 16+16             ; [x] + [size]
    ;mov     ecx,9 shl 16+16               ; [y] + [size]
    ;mov     edx,0x666666                  ; RGB
    ;push    esi                           ; backup esi
    ;mov     esi,8                         ; counter=8
    ;draw_lpanel:                          ; loop label
    ;  int     0x40                        ; call draw black rect
    ;  add     ecx,19 shl 16               ; move rect
    ;  dec     esi                         ; decrement counter
    ;  cmp     esi,0                       ; if counter!=zero
    ;  jne     draw_lpanel                 ; then goto label
    ;  mov     esi,8                       ; else counter=8
    ;  mov     ebx,267 shl 16+14           ; [x] + [size]
    ;  mov     ecx,10 shl 16+14            ; [y] + [size]
    ;  mov     edx,0xF3F3F3                ; RGB
    ;draw_lpanel2:                         ; 2 loop label
    ;  int     0x40                        ; call draw white rect
    ;  add     ecx,19 shl 16               ; move rect
    ;  dec     esi                         ; decrement counter
    ;  cmp     esi,0                       ; if counter!=0
    ;  jne     draw_lpanel2                ; then goto label2
    ;pop     esi                           ; restore esi
    ; draw_left_arrow for button_next_colorsheme
    ;mov     eax,4                         ; Write string
    ;mov     ebx,272 shl 16+13             ; [x] + [y]
    ;mov     ecx,0x0                       ; RGB
    ;mov     edx,larrow                    ; string pointer
    ;mov     esi,1                         ; count symbol
    ;int     0x40                          ; call
    ;mov     eax,38                        ; draw line
    ;mov     ebx,270 shl 16+272            ; [start x] + [end x]
    ;mov     ecx,16 shl 16+16              ; [start y] + [end y]
    ;mov     edx,0x0                       ; RGB
    ;int     0x40                          ; call
    ret                                   ; return
    ;.................................................................................................


    ;------------------------------------------------------------------------------------------------+
    draw_bottom_panel:                    ; Отрисовка нижней панели
    ;.................................................................................................
    mcall   SF_DEFINE_BUTTON, <129,90>, <WIN_H-27,16>, 16, [sc.work_button]

    add     ebx, 100 shl 16
    add     edx, 2
    int     0x40

    ; Write string
    mov     ecx,[sc.work_text]            ; RGB
    add     ecx, 0x90000000
    mcall   SF_DRAW_TEXT, <35, WIN_H-26>, ,bground

    mov     ecx, [sc.work_button_text]
    add     ecx, 0x90000000
    add     ebx, 107 shl 16
    mov     edx, bground1
    mcall

    add     ebx, 113 shl 16
    mov     edx, bground2
    mcall

    mcall SF_DRAW_LINE, <4, WIN_W-14>, <WIN_H-35, WIN_H-35>, [sc.work_graph]
    ret
    ;.................................................................................................

mouse_global:
    ;.................................................................................................
    ; Получаем координаты мыши
    ;.................................................................................................
    mcall   SF_MOUSE_GET,SSF_SCREEN_POSITION
    ; eax = x*65536 + y, (x,y)=координаты курсора мыши
    mov     ecx,eax                       ;
    shr     ecx,16                        ; ecx = x+1
    movzx   edx,ax                        ; edx = y+1
    dec     ecx                           ; ecx = x
    dec     edx                           ; edx = y
    mov     [mouse_x],ecx                 ; mouse_x = x
    mov     [mouse_y],edx                 ; mouse_y = y
    ret                                   ; Возвращаем управление
;end_mouse_global

mouse_local:
    ;.................................................................................................
    ; Получаем координаты мыши относительно окна
    ;.................................................................................................
    mcall   SF_MOUSE_GET,SSF_WINDOW_POSITION
        ; eax = x*65536 + y, (x,y)=координаты курсора мыши
    mov     ecx,eax                       ;
    shr     ecx,16                        ; ecx = x+1
    movzx   edx,ax                        ; edx = y+1
    dec     ecx                           ; ecx = x
    dec     edx                           ; edx = y
    mov     [mouse_x],ecx                 ; mouse_x = x
    mov     [mouse_y],edx                 ; mouse_y = y
    ret                                   ; Возвращаем управление
;end_mouse_local

set_background:
    ;.................................................................................................
    ; Устанавливает фон рабочего стола
    ;.................................................................................................
    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 4 - установить режим отрисовки фона.
    ; Режим отрисовки - замостить (1), растянуть (2)
    mcall   SF_BACKGROUND_SET,SSF_MODE_BG,2

    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 1 - установить размер фонового изображения.
    mcall   SF_BACKGROUND_SET,SSF_SIZE_BG,2,2

    mov     eax,[color]
    mov     [cm+0],al
    mov     [cm+9],al
    shr     eax,8
    mov     [cm+1],al
    mov     [cm+10],al
    shr     eax,8
    mov     [cm+2],al
    mov     [cm+11],al

    mov     eax,[color2]
    mov     [cm+3],al
    mov     [cm+6],al
    shr     eax,8
    mov     [cm+4],al
    mov     [cm+7],al
    shr     eax,8
    mov     [cm+5],al
    mov     [cm+8],al

    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 5 - поместить блок пикселей на фон.
    ; - Указатель на данные в формате BBGGRRBBGGRR
    ; - Cмещение в данных фонового изображения
    ; - Размер данных в байтах = 3 * число пикселей
    mcall   SF_BACKGROUND_SET,SSF_IMAGE_BG,cm,0,3*4

    ; Функция 15 - работа с фоновой графикой
    ; Подфункция 3 - перерисовать фон.
    mcall   SF_BACKGROUND_SET,SSF_REDRAW_BG

    stdcall save_eskin_ini, 'H '

    ret
;end_set_background

desktop_get:
    ;.................................................................................................
    ; Определяем ширину экрана
    ;.................................................................................................
    ; Определяем ширину экрана (eax = [xsize]*65536 + [ysize])
    mcall   SF_GET_SCREEN_SIZE ; xsize = размер по горизонтали - 1
    mov     ebx,eax                       ;
    shr     ebx,16                        ; ebx = xsize-1
    movzx   edx,ax                        ; edx = ysize-1 (лишний код)
    inc     ebx                           ; ebx = xsize
    inc     edx                           ; edx = ysize (лишний код)
    mov     [desctop_w],ebx
    mov     [desctop_h],edx
    ret
;end_desktop_get

mouse_get:
    mov     esi,2                         ; КОСТЫЛЬ: флаг для избежания зацикливания
    call    mouse_global
    call    desktop_get
    re_mouse_loop:                        ; КОСТЫЛЬ: метка для возврата если попали в сетку
      mov     ebx,[desctop_w]
      imul    ebx,[mouse_y]               ; ebx = y*xsize
      add     ebx,[mouse_x]               ; ebx = y*xsize+x

      ;.................................................................................................
      ; Берем цвет с палитры в переменную
      ;.................................................................................................
       ;mov     ebx,ecx                    ;; ebx = y*xsize+x (лишний код)
      mcall   SF_GET_PIXEL    ; Получаем цвет в eax
      cmp     eax,[sc.work]               ; Сравниваем с фоном приложения
      je      mouse_err                   ; Если это он - то ничего не делаем
      cmp     eax,0x222222                ; Сравниваем с цветом сетки
      je      mouse_err                   ; Если это он - то ничего не делаем
      jmp     mouse_set                   ; КОСТЫЛЬ: прыгаем чтобы не брать цвет сетки
    mouse_err:                            ; КОСТЫЛЬ: если попали в сетку или фон
      inc     [mouse_y]                   ; КОСТЫЛЬ: смещаем по диагонали сначала по х
      inc     [mouse_x]                   ; КОСТЫЛЬ: смещаем по диагонали потом по у
      dec     esi                         ; КОСТЫЛЬ: Уменьшаем флаг
      cmp     esi,0                       ; КОСТЫЛЬ: Сравниваем с нулем
    jz        mouse_exit                  ; КОСТЫЛЬ: Если ноль то сделали всё что могли
    jmp    re_mouse_loop                  ; КОСТЫЛЬ: Если не ноль то попробуем взять соселний пиксель
    mouse_set:                            ; Иначе запоминаем новый цвет
      cmp     [mouse_f],1
      jne     was_right
      mov     [color],eax
      call    draw_result
      jmp     mouse_exit
    was_right:
      cmp     [mouse_f],2
      jne     mouse_exit
      mov     [color2],eax
      call    draw_result                   ; Выводим результат
    mouse_exit:
    ret                                   ; Возвращаем управление
;end_mouse_get----------------------------------------------------------------------------------------






draw_value:
    ;.................................................................................................
    ; Вывод числа из строки в указанной области
    ;.................................................................................................
    push    ebx                           ; сохраняем присланные координаты
    mov     ebx,10                        ; устанавливаем основание системы счисления
    mov     edi,buff              ; указатель на строку буфера
        call    int2ascii         ; конвертируем число и ложим как строку в буфер + esi длина
    mov     eax,SF_DRAW_TEXT  ; функция 4: написать текст в окне
    pop     ebx                           ; достаем из стека присланные координаты
    cmp     esi,2                         ; ЦЕНТРИРОВАНИЕ ТЕКСТА
    jne     draw_value_1
    add     ebx,4 shl 16
    jmp     draw_value_e
  draw_value_1:
    cmp     esi,1
    jne     draw_value_e
    add     ebx,7 shl 16
  draw_value_e:
    mov     ecx,0x0;0x10000000                 ; цвет текста RRGGBB
    add     ecx,[sc.work_text]
    mov     edx,buff                      ; указатель на начало текста
    int     0x40
    ret                                   ; Возвращаем управление
;end_draw_value

  _read_params:

      mov eax,dword[params+2]
      mov dword[params_c+0],eax

      mov eax,dword[params+6]
      mov dword[params_c+4],eax

      mov   esi,params_c
      mov   ecx,16
      call  ascii2int
      mov   [color],eax

      mov eax,dword[params+11]
      mov dword[params_c+0],eax

      mov eax,dword[params+15]
      mov dword[params_c+4],eax

      mov   esi,params_c
      mov   ecx,16
      call  ascii2int
      mov   [color2],eax

      ret

hex_digit:
    ;.................................................................................................
    ; Преобразование в ASCII (вне зависимости от системы счисления)
    ;.................................................................................................
    cmp    dl,10                          ; в dl ожидается число от 0 до 15
    jb     .less                          ; если dl<10 то переходим
    add    dl,'A'-10                      ; 10->A 11->B 12->C ...
    ret                                   ; Возвращаем управление
    .less:
    or     dl,'0'                         ; Если система счисления 10-я и менее
    ret                                   ; Возвращаем управление
;end_hex_digit

int2ascii:
    ;.................................................................................................
    ; Преобразование числа в строку
    ;.................................................................................................
    ; eax - 32-х значное число
    ; ebx - основание системы счисления
    ; edi - указатель на строку буфера
    ; Возвращает заполненный буфер и esi - длина строки
    push    edi
    xor     esi,esi                       ; зануляем счетчик символов
    convert_loop:
    xor     edx,edx                       ; зануляем регистр под остаток
    div     ebx                           ; eax/ebx - остаток в edx
    call    hex_digit                     ; преобразуем символ
    push    edx                           ; ложим в стек
    inc     esi                           ; увеличиваем счетчик
    test    eax,eax                       ; если еще можно делить
    jnz     convert_loop                  ; то делием еще
    cld                                   ; ОБЯЗАТЕЛЬНО сбрасываем флаг направления DF (запись вперёд)
    write_loop:                           ; иначе
    pop     eax                           ; достаем из стека в еах
    stosb                                 ; записываем в буфер по адресу ES:(E)DI
    dec     esi                           ; уменьшаем счетчик
    test    esi,esi                       ; если есть что доставать из стека
    jnz     write_loop                    ; то достаём
    mov     byte [edi],0                  ; иначе дописыываем нулевой байт
    pop     edi
    ; код ниже не имеет ничего общего к функции, просто возвращает еще длинну полученной строки
    call    str_len
    mov     esi,eax
    ret                                   ; и возвращаем управление
;end_int2ascii

char2byte:
    sub al,'0'
    cmp al,10
    jb  done
    add al,'0'
    and al,0x5f
    sub al,'A'-10
    and al,0x0f
    done:
    ret
;end_char2byte

ascii2int:
    ;.................................................................................................
    ; Преобразование строки в число
    ;.................................................................................................
    ; esi - указатель на нультерминированную строку
    ; ecx - основание системы счисления
    ; Возвращает eax - число
    push esi
    xor eax,eax
    xor ebx,ebx
    cmp byte [esi],'-'
    jnz .next
    inc esi
    .next:
    lodsb
    or al,al
    jz .done
    call char2byte
    imul ebx,ecx
    add ebx,eax
    jmp .next
    .done:
    xchg ebx,eax
    pop esi
    cmp byte [esi],'-'
    jz .negate
    ret
    .negate:
    neg eax
    ret
;end_ascii2int

get_spectr:
    ;.................................................................................................
    ; возвращает r,g,b состовляющие цвета
    ;.................................................................................................
    mov     ecx,[color]
    mov     [cblue],cl
    shr     ecx,8
    mov     [cgreen],cl
    shr     ecx,8
    mov     [cred],cl
    shr     ecx,8
    mov     [calpha],cl
    ret                                   ; и возвращаем управление
;end_get_spectr

set_spectr:
    ;.................................................................................................
    ; устанавливает из r,g,b цвет
    ;.................................................................................................
    movzx   eax,[calpha]
    shl     eax,8
    mov     al,[cred]
    shl     eax,8
    mov     al,[cgreen]
    shl     eax,8
    mov     al,[cblue]
    mov     [color],eax
    call    draw_result                   ; Выводим результат
    ret                                   ; и возвращаем управление
;end_get_spectr

str_len:
    ;.................................................................................................
    ; определяет длину строки (вход->EDI ZS offset ; выход->EAX ZS length)
    ;.................................................................................................
        push ecx esi edi

        cld
        xor   al, al
        mov ecx, 0FFFFFFFFh
        mov esi, edi
        repne scasb
        sub edi, esi
        mov eax, edi
        dec eax

        pop edi esi ecx

        ret
;end_str_len


 ;-------------------------------

 proc random uses ebx ecx edx, max_value
    mov     ebx, 0
    mov     eax, ebx
    or      eax, eax
    jnz     @f
    rdtsc
    xor     eax, edx
    mov     ebx, eax

 @@:
    xor     edx, edx
    mov     ecx, 127773
    div     ecx
    mov     ecx, eax
    mov     eax, 16807
    mul     edx
    mov     edx, ecx
    mov     ecx, eax
    mov     eax, 2836
    mul     edx
    sub     ecx, eax
    xor     edx, edx
    mov     eax, ecx
    mov     ebx, ecx
    mov     ecx, 100000
    div     ecx
    mov     eax, edx

    xor     edx, edx
    mov     ebx, [max_value]
    div     ebx
    mov     eax, edx

    ret
 endp

set_background2:
    mcall   SF_SYS_MISC, SSF_HEAP_INIT
    mcall   SF_SYS_MISC, SSF_MEM_ALLOC, 256 * 256 * 3
    mov     [bgimg_buf], eax

    mov     edx, eax
    mov     ecx, 256 * 256
  @@:
    stdcall random, 15 + 1
    sub     al, 15 / 2

    mov     bh, byte [color + 0]
    add     bh, al
    mov     [edx + 0], bh
    mov     bh, byte [color + 1]
    add     bh, al
    mov     [edx + 1], bh
    mov     bh, byte [color + 2]
    add     bh, al
    mov     [edx + 2], bh
    add     edx, 3
    loop    @b

    mcall   SF_BACKGROUND_SET, SSF_SIZE_BG, 256, 256
    mcall   SF_BACKGROUND_SET, SSF_MODE_BG, 1
    mcall   SF_BACKGROUND_SET, SSF_IMAGE_BG, [bgimg_buf], 0, 256 * 256 * 3
    mcall   SF_BACKGROUND_SET, SSF_REDRAW_BG

    mcall   SF_SYS_MISC, SSF_MEM_FREE, [bgimg_buf]
    stdcall save_eskin_ini, 'B '
ret

align 4
proc save_eskin_ini, opt_HB:dword
        ;save to file eskin.ini
        xor     al,al
        mov     ecx,1024
        mov     edi,sys_path+2
        repne   scasb
        sub     edi,sys_path+3
        invoke  ini_set_str, inifileeskin, amain, aprogram, sys_path+2, edi
        ;add param 'H '
        mov     eax,[opt_HB]
        mov     word[params],ax
        mov     eax,[color]
        or      eax,0xf ;для избежания вечного цикла если eax=0
        mov     edi,params+2
        @@:
        rol     eax,8
        or      al,al
        jnz     @f
        mov     word[edi],'00' ;нули перед числом
        add     edi,2
        jmp     @b
        @@:
        and     al,0xf0
        jnz     @f
        mov     byte[edi],'0'
        inc     edi
        @@:
        mov     eax,[color]
        mov     ebx,16
        call    int2ascii
        mov     byte[params+10],' '
        ;add color2
        mov     eax,[color2]
        or      eax,0xf ;для избежания вечного цикла если eax=0
        mov     edi,params+11
        @@:
        rol     eax,8
        or      al,al
        jnz     @f
        mov     word[edi],'00' ;нули перед числом
        add     edi,2
        jmp     @b
        @@:
        and     al,0xf0
        jnz     @f
        mov     byte[edi],'0'
        inc     edi
        @@:
        mov     eax,[color2]
        mov     ebx,16
        call    int2ascii

        invoke  ini_set_str, inifileeskin, amain, aparam, params, 19
        ret
endp

;#___________________________________________________________________________________________________
;****************************************************************************************************|
; БЛОК ПЕРЕМЕННЫХ И КОНСТАНТ                                                                         |
;----------------------------------------------------------------------------------------------------/
circle:
    title       db 'Palitra v0.77',0      ; хранит имя программы
    hidden      db 'Hidden',0
;    hex         db '#',0                 ; для вывода решётки как текста
    cname       db 'RGBAx'                ; хранит разряды цветов (red,green,blue) x-метка конца
    larrow      db 0x1A,0
    buff        db '000',0
    bground     db 'Background',0         ; имя кнопки - 14
    bground1    db 'Gradient',0           ; имя кнопки - 15
    bground2    db 'Noisy',0              ; имя кнопки - 16
    runmode     dd 1                      ; режим запуска (1-normal, 2-hidden, 3-colordialog)
    color2      dd 00FFFFFFh              ; хранит значение второго выбранного цвета

    inifileeskin db '/sys/settings/system.ini',0
    amain       db 'style',0
    aprogram    db 'bg_program',0
    aparam      db 'bg_param',0

    i18_name    db 'ICONS18',0
    i18bg_name  db 'ICONS18W',0


align 16
@IMPORT:

library \
    libini , 'libini.obj'

import  libini, \
        ini_set_str, 'ini_set_str'

I_END:
    cm          rb 12
    color       rd 1                      ; хранит значение выбранного цвета
    mouse_x     rd 1                      ; хранит глобальную х координату мыши
    mouse_y     rd 1                      ; хранит глобальную у координату мыши
    mouse_f     rd 1                      ; хранит данные о том какая кнопка мыши была нажата
    desctop_w   rd 1                      ; хранит ширину экрана
    desctop_h   rd 1                      ; хранит высоту экрана
    sc          system_colors             ; хранит структуру системных цветов скина
    cred        rb 1                      ; храним красный спектр
    cgreen      rb 1                      ; храним зеленый спектр
    cblue       rb 1                      ; храним синий спектр
    calpha      rb 1                      ; храним прозрачность
    pnext       rd 1                      ; счетчик переключения палитры
    renmode     rd 1                      ; режим отрисовки (1-цветовая схема,2-пипетка,3-круговая)
    params      rb 20                     ; приём параметров
    params_c    rb 9                      ; приёмник для цвета
    bgimg_buf   rd 1                      ; buffer for a generated image
    icons18     dd ?                      ; pointer to a shared memory of icons18.png
    icons18bg   dd ?                      ; pointer to a shared memory of icons18.png with filled bg

        rd 1024
stacktop:
        sys_path rb 1024
I_MEM: