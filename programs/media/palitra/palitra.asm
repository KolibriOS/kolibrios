;#___________________________________________________________________________________________________
;****************************************************************************************************|
; Program Palitra (c) Sergei Steshin (Akyltist)                                                      |
;----------------------------------------------------------------------------------------------------|
;; Charset:DOS-866 Font:Courier New Size:9pt                                                         |
;.....................................................................................................
;; version:      0.8.0                                                                               |
;; last update:  21 Jun 2026                                                                         |
;; e-mail:       dr.steshin@gmail.com                                                                |
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

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../dll.inc'

include 'draw_sliders.inc'
include 'draw_utils.inc'
include 'draw_palitra.inc'
include 'fill_background.inc'

MODE_PALITRA = 0
MODE_PIPET   = 1

WIN_W  = 380
WIN_H  = 301
WIN_X  = 250
WIN_Y  = 190

Left_Border=8
SliderPanel_W = 110
DRAWY  = 12
PIPETY = DRAWY+ICONS+DRAWY+5+2

CELLW       = 11; 11            ; not used yet, but has to be :)

ICONX  = WIN_W - 40
ICONS  = 18             ; icon size  

SLIDER_Y = 61

PALITRA_X = Left_Border+SliderPanel_W+12
PALITRA_W = CELLW*(8)+8+1
PALITRA_XW = PALITRA_X shl 16 + PALITRA_W
PALITRA_YW = DRAWY shl 16 + PALITRA_W

; list of buttons
BTN_PALITRA  = 7

BTN_GRADIENT = 20
BTN_NOISY    = 21
BTN_CHECKERS = 22
BTN_SILK     = 23

BTN_NEXT     = 30
BTN_PIPET    = 31

BTN_COL_SWAP = 40

START:
    mcall SF_SYSTEM, SSF_WINDOW_BEHAVIOR, SSSF_SET_WB, -1, 1 ;always on top
    mcall SF_SYS_MISC,SSF_HEAP_INIT ; инициализация кучи
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
    mcall   SF_MOUSE_GET,SSF_BUTTON
    cmp     [renmode],MODE_PIPET
    jne     left
    push    eax
    call    draw_pipet_preview
    pop     eax
    cmp     al,0
    je      still
    mov     [color],edx
    mov     [renmode],MODE_PALITRA        ; MODE_PIPET => MODE_PALITRA
    jmp     red
  left:
    cmp     al,1b
    jne     right
    mov     [mouse_f],1
    jmp     still
  right:
    cmp     al,10b
    jne     still
    mov     [mouse_f],2
    jmp     still
;end_mouse

button:
    mcall   SF_GET_BUTTON         ; 17 - получить идентификатор нажатой кнопки
    cmp     ah, 1                         ; если нажата кнопка с номером 1,
    jz      bexit                         ; выходим
  ;обработка кнопки Next
    cmp     ah, BTN_NEXT                  ; если нажата кнопка NEXT
    jne     next_bg                       ; выходим
    inc     [pnext]                       ; увеличиваем при нажатии номер палитры
    mov     [renmode],MODE_PALITRA        ; включаем цветовые схемы
    mov     eax,[pnext]                   ; заносим значение в еах
    cmp     al,6                          ; сравниваем с заявленным количеством палитр
    jne     next_redraw                   ; если не больше максимума то на вызов отрисовки
    xor     eax,eax                       ; иначе зануляем палитру на default
    mov     [pnext],eax                   ; и запоминаем что сбросили палитру на default
  next_redraw:
    call    draw_palitra
    jmp     still                         ; Уходим на ожидание другого события
  next_bg:
    cmp     ah, BTN_GRADIENT
    jne     next_bg2
    call    fill_background_gradient
    jmp     still
  next_bg2:
    cmp     ah, BTN_NOISY
    jne     next_bg3
    call    fill_background_noisy
    jmp     still
  next_bg3:
    cmp     ah, BTN_CHECKERS
    jne     next_bg4
    call    fill_background_checkers
    jmp     still
  next_bg4:
    cmp     ah, BTN_SILK 
    jne     activate_pipet
    call    fill_background_silk
    jmp     red
  activate_pipet:
    cmp     ah, BTN_PIPET
    jne     next_end
    mov     [renmode],MODE_PIPET          ; включаем отрисовку круговой палитры
    ;call    draw_palitra                  ; РИСУЕМ ПАЛИТРУ
    jmp     red
  next_end:
    cmp     ah,BTN_COL_SWAP
    jne     color_swap_end
    push    [color2]
    push    [color]
    pop     [color2]
    pop     [color]
    call    draw_result
    jmp     still                         ; И уходим на ожидание другого события
  color_swap_end:
    cmp     ah, BTN_PALITRA               ; Проверяем нажата кнопка с ID=7
    jne     color_button                  ; Если не нажата, то идём дальше
    call    mouse_get                     ; Иначе включаем обработчик мыши, чтобы считать значение цвета с палитры
    jmp     still                         ; И уходим на ожидание другого события
  color_button:                           ; РАСЧЁТ координат для ползунков RGBA
    push    eax                           ; запоминаем еах
    call    mouse_local                   ; получаем локальные координаты
    mov     ecx,[mouse_y]                 ; заносим в есх значение курсора по У
    sub     ecx, SLIDER_Y                 ; находим разность (т.е. куда смещается ползунок)

    ; Защита от выхода за границы (0 <= ecx)
    xor     eax, eax
    cmp     ecx, eax
    jge     .lo_ok
    mov     ecx, eax
  .lo_ok:

    ; 2. Превращаем пиксели в значение цвета (0-255)
    ; Если ползунок 128 пикселей, а диапазон 256 (0-255), то нужно умножить на 2
    shl     ecx, 1

    ; Переворачиваем число
    neg     ecx
    add     ecx, 255

    ; Защита от отрицательного значения (исправляет прыжок в 255)
    cmp     ecx, 0
    jge     @f
    xor     ecx, ecx                      ; Если ушло в минус, принудительно делаем чистый 0
@@:
    pop     eax   
  red_button:                             ; Красный Трекбар                                              :
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
    mcall   SF_REDRAW,SSF_BEGIN_DRAW
    mcall   SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
    mcall   SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
    mov     ecx,eax                      ; Запоминаем высоту скина

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

    mcall   SF_DEFINE_BUTTON, <PALITRA_X,PALITRA_W*2+3>, <DRAWY,PALITRA_W*2+3>, BTN_PALITRA+BT_HIDE+BT_NOFRAME

    inc     edx
    mcall   , <14,22>, <47,150>           ; Рисуем невидимую кнопку под слайдером red
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
    mcall   , <14,22>, <16,20>, BTN_COL_SWAP+BT_HIDE

    call    draw_bottom_panel
    call    draw_right_panel

    mcall SF_REDRAW,SSF_END_DRAW
    ret



;#_______________________________________________________
;*******************************************************|
; БЛОК ВСПОМОГАТЕЛЬНЫХ ПРОЦЕДУР И ФУНКЦИЙ ПРИЛОЖЕНИЯ    |
;-------------------------------------------------------/


    ;------------------------------------------------------------------------------------------------+
    draw_right_panel:
    ;.................................................................................................
    ; button_next_colorsheme
    mcall   SF_DEFINE_BUTTON, <ICONX,ICONS+3>, <DRAWY+1,ICONS+3>, BTN_NEXT+BT_HIDE
    mcall , , <PIPETY,  ICONS+3>, BTN_PIPET+BT_HIDE

    mov     ebx,[icons18bg]
    add     ebx,ICONS*ICONS*4*53
    mcall   SF_PUT_IMAGE_EXT, ebx, <ICONS,ICONS>, <ICONX+2,DRAWY+3>, 32, 0, 0

    ; pipet
    mov     ebx,[icons18bg]
    
    add     ebx,ICONS*ICONS*4*39
    mov     edx,(ICONX+2)*65536+PIPETY+2
    mcall   SF_PUT_IMAGE_EXT

    stdcall DrawDeepRectangle, ICONX-1, DRAWY,   ICONS+5, ICONS+5, [sc.work_graph], [sc.work_graph]
    stdcall DrawDeepRectangle, ICONX,   DRAWY+1, ICONS+3, ICONS+3, [sc.work_light], [sc.work_dark]

    stdcall DrawDeepRectangle, ICONX-1, PIPETY-1, ICONS+5, ICONS+5, [sc.work_graph], [sc.work_graph]
    stdcall DrawDeepRectangle, ICONX,   PIPETY,   ICONS+3, ICONS+3, [sc.work_light], [sc.work_dark]
    ;stdcall DrawDeepRectangle, ICONX+1, DRAWY+175, ICONS+1, ICONS+1, 0xFFFfff, 0xFFFfff

    ret
    ;.................................................................................................


    ;------------------------------------------------------------------------------------------------+
    draw_bottom_panel:                    ; Отрисовка нижней панели
    ;.................................................................................................
    mcall   SF_DEFINE_BUTTON, <PALITRA_X,95>, <WIN_H-67,24>, BTN_GRADIENT, [sc.work_button]

    add     ebx, (PALITRA_W+4) shl 16
    mov     edx, BTN_NOISY
    int     0x40

    mcall   , <PALITRA_X,95>, <WIN_H-37,24>, BTN_CHECKERS

    add     ebx, (PALITRA_W+4) shl 16
    mov     edx, BTN_SILK 
    int     0x40

    ; Write string
    mov     ecx,[sc.work_text]
    add     ecx, 0x90000000
    mcall   SF_DRAW_TEXT, <7, WIN_H-62>, ,bground

    mov     ecx, [sc.work_button_text]
    add     ecx, 0x90000000
    mcall   SF_DRAW_TEXT, <PALITRA_X+16,  WIN_H-62>, , lbl_grad
    mcall   SF_DRAW_TEXT, <PALITRA_X+129, WIN_H-62>, , lbl_noisy
    mcall   SF_DRAW_TEXT, <PALITRA_X+16,  WIN_H-32>, , lbl_check
    mcall   SF_DRAW_TEXT, <PALITRA_X+129, WIN_H-32>, , lbl_silk

    mcall SF_DRAW_LINE, <8, WIN_W-18>, <WIN_H-80, WIN_H-80>, [sc.work_graph]
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
    jmp    re_mouse_loop                  ; КОСТЫЛЬ: Если не ноль то попробуем взять соседний пиксель
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
    jnz     convert_loop                  ; то делим еще
    cld                                   ; ОБЯЗАТЕЛЬНО сбрасываем флаг направления DF (запись вперёд)
    write_loop:                           ; иначе
    pop     eax                           ; достаем из стека в еах
    stosb                                 ; записываем в буфер по адресу ES:(E)DI
    dec     esi                           ; уменьшаем счетчик
    test    esi,esi                       ; если есть что доставать из стека
    jnz     write_loop                    ; то достаём
    mov     byte [edi],0                  ; иначе дописываем нулевой байт
    pop     edi
    ; код ниже не имеет ничего общего к функции, просто возвращает еще длину полученной строки
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
    ; возвращает r,g,b составляющие цвета
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
    title       db 'Palitra v0.8',0      ; хранит имя программы
    hidden      db 'Hidden',0
;    hex         db '#',0                 ; для вывода решётки как текста
    cname       db 'RGBAx'                ; хранит разряды цветов (red,green,blue) x-метка конца
    larrow      db 0x1A,0
    buff        db '000',0
    bground     db 'Background',0
    lbl_grad    db 'Gradient',0
    lbl_noisy   db 'Noisy',0
    lbl_check   db 'Checkers',0
    lbl_silk    db 'Silky',0
    runmode     dd 1                      ; режим запуска (1-normal, 2-hidden, 3-colordialog)
    color2      dd 00FFFFFFh              ; хранит значение второго выбранного цвета

    inifileeskin db '/sys/settings/system.ini',0
    amain       db 'style',0
    aprogram    db 'bg_program',0
    aparam      db 'bg_param',0

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
    renmode     rd 1                      ; режим отрисовки (0-цветовая схема,1-пипетка)
    params      rb 20                     ; приём параметров
    params_c    rb 9                      ; приёмник для цвета
    bgimg_buf   rd 1                      ; buffer for a generated image
    icons18bg   dd ?                      ; pointer to a shared memory of icons18.png with filled bg

        rd 1024
stacktop:
        sys_path rb 1024
I_MEM: