;#___________________________________________________________________________________________________
;****************************************************************************************************|
; Program Palitra (c) Sergei Steshin (Akyltist)                                                      |
;----------------------------------------------------------------------------------------------------|
;; Charset:DOS-866 Font:Courier New Size:9pt                                                         |
;.....................................................................................................
;; compiler:     FASM 1.69.31                                                                        |
;; version:      0.3.0                                                                               |
;; last update:  07/11/2013                                                                          |
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
;; 0.7.6 - Переработан режим H (hidden): теперь цвета загружаются из wallpaper.dat       (eAndrew)   |
;;       - При установки фона кнопкой, цвет фона сохраняется в wallpaper.dat             (eAndrew)   |
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
  org	 0x0
  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 I_RDATA+4096
  dd	 I_RDATA+4096
  dd	 params
  dd	 0x0

  include '../../../macros.inc'

  WIN_W  equ 295	    ; ширина окна
  WIN_H  equ 195	    ; высота окна
  WIN_X  equ 250	    ; координата х окна
  WIN_Y  equ 190	    ; координата у окна

panel:
  file "panel.raw"

START:
    mcall   68,11			  ; инициализация кучи
    mcall   40,0x27			  ; устанавливаем маску событий
    include 'params_init.inc'		  ; обработка параметров командной строки

;#___________________________________________________________________________________________________
;****************************************************************************************************|
; ОСНОВНОЙ ЦИКЛ ПРОГРАММЫ - ОБРАБОТКА СОБЫТИЙ                                                        |
;----------------------------------------------------------------------------------------------------/
red:
    call draw_main			  ; вызываем перерисовку окна приложения
still:
    mcall   10				  ; функция 10 - ждать события
    cmp     eax,1			  ; перерисовать окно ?
    je	    red 			  ; если да - на метку red
    cmp     eax,2			  ; нажата клавиша ?
    je	    key 			  ; если да - на key
    cmp     eax,3			  ; нажата кнопка ?
    je	    button			  ; если да - на button
    cmp     eax,6			  ; событие от мыши вне окна
    je	    mouse			  ; если да - на button
    jmp     still			  ; если другое событие - в начало цикла
;end_still

key:					  ; нажата клавиша на клавиатуре
    mcall   2				  ; функция 2 - считать код символа (в ah) (тут в принципе не нужна)
    jmp     still			  ; вернуться к началу цикла
;end_key

mouse:
    cmp     [renmode],2
    jne     left
    call    cyrcle_draw
    jmp     center
  left:
    mcall   37,2
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
    mcall   37,2
    cmp     al,100b
    jne     still
    mov     [mouse_f],3
    mov     [color],edx
    call    draw_result
    jmp     still			  ; вернуться к началу цикла
;end_mouse

button:
    mcall   17				  ; 17 - получить идентификатор нажатой кнопки
    cmp     ah, 1			  ; если нажата кнопка с номером 1,
    jz	    bexit			  ; выходим
  ;обработка кнопки Next
    cmp     ah, 12			  ; если нажата кнопка NEXT
    jne     next_bg			  ; выходим
    inc     [pnext]			  ; увеличиваем при нажатии номер палитры
    mov     [renmode],0 		  ; включаем цветовые схемы
    mov     eax,[pnext] 		  ; заносим значение в еах
    cmp     al,6			  ; сравниваем с заявленным количеством палитр
    jne     next_redraw 		  ; если не больше максимума то на вызов отрисовки
    xor     eax,eax			  ; иначе зануляем палитру на default
    mov     [pnext],eax 		  ; и запоминаем что сбросили палитру на default
  next_redraw:
    call    draw_palitra		  ; РИСУЕМ ПАЛИТРУ
    jmp     still			  ; Уходим на ожидание другого события
  next_bg:
    cmp     ah, 14			  ; Кнопка BACKGROUND
    jne     circle_bg			  ; если не нажата то выходим
    call    set_background		  ; иначе устанавливаем фон
    jmp     still			  ; и на ожидание события
  circle_bg:
    cmp     ah, 15			  ; Кнопка Круговая палитра
    jne     next_end			  ; если не нажата то выходим
    mov     [renmode],2 		  ; включаем отрисовку круговой палитры
    call    draw_palitra		  ; РИСУЕМ ПАЛИТРУ
    jmp     still			  ; и на ожидание события
  next_end:
    cmp     ah,13			  ; COLOR SWAP
    jne     color_swap_end
    push    [color2]
    push    [color]
    pop     [color2]
    pop     [color]
    call    draw_result
    jmp     still			  ; И уходим на ожидание другого события
  color_swap_end:
    cmp     ah, 7			  ; Проверяем нажата кнопка с ID=7
    jne     color_button		  ; Если не нажата, то идём дальше
    call    mouse_get			  ; Иначе включаем обработчик мыши, чтобы считать значение цвета с палитры
    jmp     still			  ; И уходим на ожидание другого события
  color_button: 			  ; РАСЧЁТ координат для ползунков RGBA
    push    eax 			  ; запоминаем еах
    call    mouse_local 		  ; получаем локальные координаты
    mov     ebx,137			  ; нижняя граница ползунка по У
    mov     ecx,[mouse_y]		  ; занисим в есх значение курсора по У
    sub     ebx,ecx			  ; находим разность (т.е. куда смещается ползунок)
    mov     ecx,3			  ; заносим в есх цифру 3 (256/3=85, где 85-высота ползунков)
    imul    ecx,ebx			  ; находим истинный параметр цвета с учётом масштаба ползунка---+
    pop     eax 			  ; восстанавливаем еах                                          :
  ;red_button:                            ; Красный Трекбар                                              :
    cmp     ah, 8			  ; ID=8                                                         :
    jne     green_button		  ; если нет, то проверяем зелёный трекбар                       :
    mov     [cred],cl			  ; иначе присваиваем значение, красному цвету спектра    <------+
    call    set_spectr			  ; устанавливаем спектр
    jmp     still			  ; Уходим на ожидание другого события
  green_button:
    cmp     ah, 9
    jne     blue_button
    mov     [cgreen],cl
    call    set_spectr
    jmp     still			  ; Уходим на ожидание другого события
  blue_button:
    cmp     ah, 10
    jne     alpha_button
    mov     [cblue],cl
    call    set_spectr
    jmp     still			  ; Уходим на ожидание другого события
  alpha_button:
    cmp     ah, 11
    jne     still
    mov     [calpha],cl
    call    set_spectr
    jmp     still			  ; Уходим на ожидание другого события
  bexit:
    mcall -1				  ; иначе конец программы
;end_button

;#___________________________________________________________________________________________________
;****************************************************************************************************|
; ГЛАВНЫЙ МОДУЛЬ ОТРИСОВКИ ОКНА И ЭЛЕМЕНТОВ ПРИЛОЖЕНИЯ                                               |
;----------------------------------------------------------------------------------------------------/
draw_main:
    mov     eax,12			  ; функция 12: означает, что будет рисоваться окно
    mov     ebx,1			  ; 1,начало рисования
    int     0x40			  ; Прерывание

    mov     eax,48			  ; Функция 48 - стили отображения окон
    mov     ebx,3			  ; Подфункция 3 - получить стандартные цвета окон.
    mov     ecx,sc			  ; Указатель на буфер размером edx байт, под структуру
    mov     edx,sizeof.system_colors	  ; Размер таблицы цветов (должен быть 40 байт)
    int     0x40			  ; Прерывание

    mov     eax,48			  ; Функция 48 - стили отображения окон.
    mov     ebx,4			  ; Подфункция 4 - возвращает eax = высота скина.
    int     0x40			  ; Прерывание
    mov     ecx,eax			  ; Запоминаем высоту скина

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
    mov     edi,title			  ; Заголовок окна
  set_title:

    xor     eax,eax			  ; Очищаем eax (mov eax,0) (Функция 0)
    mov     ebx,WIN_X shl 16+WIN_W	  ; [координата по оси x]*65536 + [размер по оси x]
    add     ecx,WIN_Y shl 16+WIN_H	  ; Высота скина + [координата по y]*65536 + [размер по y] (168 для версии 0.2)
    mov     edx,[sc.work]		  ; Видимо стиль окна по дефолту
    or	    edx,0x34000000		  ; Или окно со скином фиксированных размеров

    int     0x40			  ; Прерывание


    call    draw_palitra		  ; РИСУЕМ ПАЛИТРУ
    call    draw_result 		  ; РИСУЕМ РЕЗУЛЬТАТ

    mov     eax,8			  ; Функция 8 - определить/удалить кнопку
    mov     ebx,110 shl 16+147		  ; Начальные координаты по х [к-та x]*65536 + [размер]
    mov     ecx,9 shl 16+147		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    mov     edx,0x60000007		  ; Начальное ID кнопок - 0xXYnnnnnn
    int     0x40

    mov     ebx,13 shl 16+12		  ; Начальные координаты по х [к-та x]*65536 + [размер]
    mov     edx,0x60000008		  ; ID = 8
    mov     ecx,51 shl 16+85		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40			  ; Рисуем невидимую кнопку под слайдером red
    add     ebx,23 shl 16		  ; Добавляем
    inc     edx 			  ; ID = 9
    int     0x40			  ; Рисуем невидимую кнопку под слайдером green
    add     ebx,23 shl 16		  ; Добавляем
    inc     edx 			  ; ID = 10
    int     0x40			  ; Рисуем невидимую кнопку под слайдером blue
    add     ebx,23 shl 16		  ; Добавляем
    inc     edx 			  ; ID = 11
    int     0x40			  ; Рисуем невидимую кнопку под слайдером alpha

    mov     eax,8			  ; Функция 8 - определить/удалить кнопку (СМЕНА ЦВЕТА)
    mov     ebx,11 shl 16+20		  ; Начальные координаты по х [к-та x]*65536 + [размер]
    mov     ecx,12 shl 16+20		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    mov     edx,0x6000000D		  ; ID = 13
    int     0x40

    call    draw_bottom_panel
    call    draw_left_panel

    mov     eax,12			  ; функция 12: означает, что будет рисоваться окно
    mov     ebx,2			  ; 2 - конец рисования
    int     0x40			  ; Прерывание
    ret

;#___________________________________________________________________________________________________
;****************************************************************************************************|
; БЛОК ВСПОМОГАТЕЛЬНЫХ ПРОЦЕДУР И ФУНКЦИЙ ПРИЛОЖЕНИЯ                                                 |
;----------------------------------------------------------------------------------------------------/


    ;------------------------------------------------------------------------------------------------+
    draw_left_panel:			  ; Отрисовка боковой панели
    ;.................................................................................................
    mov     eax,8			  ; button_next_colorsheme
    mov     ebx,266 shl 16+16		  ; [x] + [size]
    mov     ecx,9 shl 16+16		  ; [y] + [size]
    mov     edx,0x6000000C		  ; ID = 12
    mov     esi,[sc.work_button]	  ; RGB
    int     0x40			  ; call
    ; circle diagram
    add     ecx,19 shl 16		  ; move rect
    mov     edx,0x6000000F		  ; ID = 15
    int     0x40			  ; call

    mov     eax,7			  ; 7 - номер функции
    mov     ebx,panel			  ; указатель на изображение в формате BBGGRRBBGGRR...
    mov     ecx, 16 shl 16 + 149	  ; [размер по оси x]*65536 + [размер по оси y]
    mov     edx,266 shl 16 +   9	    ; [координата по оси x]*65536 + [координата по оси y]
    int     0x40

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
    ret 				  ; return
    ;.................................................................................................


    ;------------------------------------------------------------------------------------------------+
    draw_bottom_panel:			  ; Отрисовка нижней панели
    ;.................................................................................................
    mov     eax,8			  ; draw button
    mov     ebx,92 shl 16+100		  ; [x] + [size]
    mov     ecx,169 shl 16+16		  ; [y] + [size]
    mov     edx,14			  ; ID = 14
    mov     esi,[sc.work_button]	  ; RGB
    int     0x40			  ; call
    mov     eax,4			  ; Write string
    mov     ebx,113 shl 16+174		  ; [x] + [y]
    mov     ecx,[sc.work_button_text]	  ; RGB
    mov     edx,bground 		  ; string pointer
    mov     esi,10			  ; count symbol
    int     0x40			  ; call
    mov     eax,38			  ; draw line
    mov     ebx,4 shl 16+282		  ; [start x] + [end x]
    mov     ecx,163 shl 16+163		  ; [start y] + [end y]
    mov     edx,0x00666666		  ; RGB
    int     0x40			  ; call
    ret 				  ; return
    ;.................................................................................................

mouse_global:
    ;.................................................................................................
    ; Получаем координаты мыши
    ;.................................................................................................
    mov     eax,37			  ; Функция 37 - работа с мышью
    mov     ebx,0			  ; нам нужны глобальные координаты
    int     0x40			  ; eax = x*65536 + y, (x,y)=координаты курсора мыши
    mov     ecx,eax			  ;
    shr     ecx,16			  ; ecx = x+1
    movzx   edx,ax			  ; edx = y+1
    dec     ecx 			  ; ecx = x
    dec     edx 			  ; edx = y
    mov     [mouse_x],ecx		  ; mouse_x = x
    mov     [mouse_y],edx		  ; mouse_y = y
    ret 				  ; Возвращаем управление
;end_mouse_global

mouse_local:
    ;.................................................................................................
    ; Получаем координаты мыши относительно окна
    ;.................................................................................................
    mov     eax,37			  ; Функция 37 - работа с мышью
    mov     ebx,1			  ; нам нужны глобальные координаты
    int     0x40			  ; eax = x*65536 + y, (x,y)=координаты курсора мыши
    mov     ecx,eax			  ;
    shr     ecx,16			  ; ecx = x+1
    movzx   edx,ax			  ; edx = y+1
    dec     ecx 			  ; ecx = x
    dec     edx 			  ; edx = y
    mov     [mouse_x],ecx		  ; mouse_x = x
    mov     [mouse_y],edx		  ; mouse_y = y
    ret 				  ; Возвращаем управление
;end_mouse_local

set_background:
    ;.................................................................................................
    ; Устанавливает фон рабочего стола
    ;.................................................................................................
    mov     eax,15			  ; Функция 15 - работа с фоновой графикой
    mov     ebx,4			  ; Подфункция 4 - установить режим отрисовки фона.
    mov     ecx,2			  ; Режим отрисовки - замостить (1)
    int     0x40			  ; Вызываем

    mov     eax,15			  ; Функция 15 - работа с фоновой графикой
    mov     ebx,1			  ; Подфункция 1 - установить размер фонового изображения.
    mov     ecx,2			  ; Ширина изображения
    mov     edx,2			  ; Высота изображения
    int     0x40			  ; Вызываем

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

    mov     eax,15			  ; Функция 15 - работа с фоновой графикой
    mov     ebx,5			  ; Подфункция 5 - поместить блок пикселей на фон.
    mov     ecx,cm			; Указатель на данные в формате BBGGRRBBGGRR
    mov     edx,0			  ; Cмещение в данных фонового изображения
    mov     esi,3*4			 ; Размер данных в байтах = 3 * число пикселей
    int     0x40			  ; Вызываем

    mov     eax,15			  ; Функция 15 - работа с фоновой графикой
    mov     ebx,3			  ; Подфункция 3 - перерисовать фон.
    int     0x40			  ; Вызываем

    mov     eax, [color]
    mov     [fi.file+0], al
    shr     eax, 8
    mov     [fi.file+1], al
    shr     eax, 8
    mov     [fi.file+2], al

    mov     eax, [color2]
    mov     [fi.file+3], al
    shr     eax, 8
    mov     [fi.file+4], al
    shr     eax, 8
    mov     [fi.file+5], al

    mov     [fi.p00], dword 2
    mov     [fi.p04], dword 0
    mov     [fi.p08], dword 0
    mov     [fi.p12], dword 6
    mov     [fi.p16], dword fi.file
    mov     [fi.p20], dword 0
    mov     [fi.p21], dword dat_file
    mcall   70, fi

    ret
;end_set_background

desktop_get:
    ;.................................................................................................
    ; Определяем ширину экрана
    ;.................................................................................................
    mov     eax,14			  ; Определяем ширину экрана (eax = [xsize]*65536 + [ysize])
    int     0x40			  ; xsize = размер по горизонтали - 1
    mov     ebx,eax			  ;
    shr     ebx,16			  ; ebx = xsize-1
    movzx   edx,ax			  ; edx = ysize-1 (лишний код)
    inc     ebx 			  ; ebx = xsize
    inc     edx 			  ; edx = ysize (лишний код)
    mov     [desctop_w],ebx
    mov     [desctop_h],edx
    ret
;end_desktop_get

mouse_get:
    mov     esi,2			  ; КОСТЫЛЬ: флаг для избежания зацикливания
    call    mouse_global
    call    desktop_get
    re_mouse_loop:			  ; КОСТЫЛЬ: метка для возврата если попали в сетку
      mov     ebx,[desctop_w]
      imul    ebx,[mouse_y]		  ; ebx = y*xsize
      add     ebx,[mouse_x]		  ; ebx = y*xsize+x

      ;.................................................................................................
      ; Берем цвет с палитры в переменную
      ;.................................................................................................
      mov     eax,35			  ; Функция взятия цвета
      ;mov     ebx,ecx                    ;; ebx = y*xsize+x (лишний код)
      int     0x40			  ; Получаем цвет в eax
      cmp     eax,[sc.work]		  ; Сравниваем с фоном приложения
      je      mouse_err 		  ; Если это он - то ничего не делаем
      cmp     eax,0x222222		  ; Сравниваем с цветом сетки
      je      mouse_err 		  ; Если это он - то ничего не делаем
      jmp     mouse_set 		  ; КОСТЫЛЬ: прыгаем чтобы не брать цвет сетки
    mouse_err:				  ; КОСТЫЛЬ: если попали в сетку или фон
      inc     [mouse_y] 		  ; КОСТЫЛЬ: смещаем по диагонали сначала по х
      inc     [mouse_x] 		  ; КОСТЫЛЬ: смещаем по диагонали потом по у
      dec     esi			  ; КОСТЫЛЬ: Уменьшаем флаг
      cmp     esi,0			  ; КОСТЫЛЬ: Сравниваем с нулем
    jz	      mouse_exit		  ; КОСТЫЛЬ: Если ноль то сделали всё что могли
    jmp    re_mouse_loop		  ; КОСТЫЛЬ: Если не ноль то попробуем взять соселний пиксель
    mouse_set:				  ; Иначе запоминаем новый цвет
      cmp     [mouse_f],1
      jne     was_right
      mov     [color],eax
      call    draw_result
      jmp     mouse_exit
    was_right:
      cmp     [mouse_f],2
      jne     mouse_exit
      mov     [color2],eax
      call    draw_result		    ; Выводим результат
    mouse_exit:
    ret 				  ; Возвращаем управление
;end_mouse_get----------------------------------------------------------------------------------------

draw_palitra:
    mov     eax,13			  ; Функция 13 - нарисовать прямоугольник
    mov     ebx,110 shl 16+148		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,9 shl 16+148		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    mov     edx,[sc.work]		  ; цвет
    int     0x40

    cmp     [renmode],2
    je	    cyrcle_draw
    ;cmp     [renmode],1
    ;je      picker_draw
    cmp     [renmode],0
    je	    sheme_draw
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
    call    mouse_local 		  ; получаем координаты мыши относительно окна
    mov     ecx, [mouse_x]		  ; заносим в регистр
    mov     edx, [mouse_y]		  ; заносим в регистр
    cmp     ecx, WIN_W-10
    jg	    cyrcle_draw_2
    cmp     edx, WIN_H
    jle     end_cyrcle_draw

    cyrcle_draw_2:
	mcall	5,10
    call    desktop_get
    call    mouse_global

    mov     ebx,112*65536+11
    mov     esi,0			  ; counter=8
    circle_loop:
      mov     ecx, 10*65536+11
      mov     edi,0
      circle_loop2:
	mov	eax,13
	call	circle_pixel_read
	mcall
	add	ecx,11 shl 16
	inc	edi
	cmp	edi,13
	jne	circle_loop2

      add     ebx,11 shl 16
      inc     esi
      cmp     esi,13
      jne     circle_loop


    mov     eax,13
    mov     edx,0x0
    mov     ebx,177*65536+13
    mov     ecx, 76*65536+13
    mcall
    mov     ecx, [mouse_x]
    mov     edx, [mouse_y]
    inc     ecx
    inc     edx
    mov     ebx, edx
    imul    ebx, [desctop_w]
    add     ebx, ecx
    mov     eax, 35
    mcall
    mov     edx,eax
    mov     eax,13
    mov     ebx,178*65536+11
    mov     ecx, 77*65536+11
    mcall
    ret
    end_cyrcle_draw:
    mov     eax,13
    mov     edx,0x666666
    mov     ebx,111*65536+145
    mov     ecx,  9*65536+145
    mcall
    ret

    circle_pixel_read:
    push    ecx
    push    ebx
    push    eax
    mov     ecx, [mouse_x]
    mov     edx, [mouse_y]
    inc     ecx
    add     ecx, esi
    cmp     ecx, 6
    jl	    _cpr_exit
    sub     ecx, 6
    inc     edx
    add     edx, edi
    cmp     edx, 6
    jl	    _cpr_exit
    sub     edx, 6

    mov     ebx, edx
    imul    ebx, [desctop_w]
    add     ebx, ecx
    mov     eax, 35
    mcall
    mov     edx,eax
    pop     eax
    pop     ebx
    pop     ecx
    ret

    _cpr_exit:
    mov     edx,0x00000000
    pop     eax
    pop     ebx
    pop     ecx
    ret

    ;picker_draw:
    ;ret

    ;.................................................................................................
    ; Отрисовка фона под кнопки
    ;.................................................................................................
    sheme_draw:
    mov     eax,13			  ; Функция 13 - нарисовать прямоугольник
    mov     edx,0x222222		  ; цвет
    mov     ecx,9 shl 16+73		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    mov     esi,2			  ; Счетчик линий подложек
    re_draw:
    mov     ebx,110 shl 16+73		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     edi,2			  ; Счетчик количества подложек
    for_fon_loop:
      int     0x40			  ; Прерывание
      add     ebx,75 shl 16		  ; Смещаем положение линии по х
      dec     edi			  ; Уменьшаем счетчик кнопок
      cmp     edi,0			  ; Сравниваем с нулем
    jnz     for_fon_loop		  ; Если не ноль то в начало цикла
    dec     esi 			  ; Уменьшаем его
    cmp     esi,0			  ; Сравниваем с нулем
    mov     ecx,84 shl 16+73		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    jnz     re_draw			  ; Если не ноль то в начало цикла

    ;.................................................................................................
    ; Отрисовка кнопок по циклу
    ;.................................................................................................
    mov     eax,13			  ; Функция 13 - нарисовать прямоугольник
    mov     edx,0x0FFFFFFF		  ; цвет
    mov     esi,4			  ; Счетчик количества абзацев (#4,8)
    mov     ebx,99 shl 16+8		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    for_abz:
      ;;push    esi                       ; Сохраняем значение счетчика линий в стек
      cmp     esi,2
      jne     x2_line
      mov     ebx,99 shl 16+8
      x2_line:
      add     ebx,3 shl 16		  ; Смещаем положение линии по x
      mov     edi,8			  ; Счетчик количества кнопок в строке
      for_stolbik:
	push	edi			  ; Сохраняем значение счетчика линий в стек
	mov	edi,8			  ; Счетчик количества кнопок в строке
	mov	ecx,  1 shl 16+8	  ; Начальные координаты по y [к-та y]*65536 + [размер]
	cmp	esi,2
	jg	y2_line 		  ; Если больше 4 то игнорим
	mov	ecx,76 shl 16+8
	y2_line:
	add	ebx,9 shl 16		  ; Смещаем положение линии по x
	for_loop:
	  add	  ecx,9 shl 16		  ; Смещаем положение линии по y
	  int	  0x40			  ; Прерывание
	  call	  sub_red
	  dec	  edi			  ; Уменьшаем счетчик кнопок
	  cmp	  edi,0 		  ; Сравниваем с нулем
	  jnz	  for_loop		  ; Если не ноль то в начало цикла
      call    sub_green
      pop     edi			  ; Иначе восстанавливаем счетчик линий
      dec     edi			  ; Уменьшаем его
      cmp     edi,0			  ; Сравниваем с нулем
      jnz     for_stolbik		  ; Если не ноль то в начало цикла
    call    sub_blue
    ;;pop     esi                         ; Иначе восстанавливаем счетчик линий
    dec     esi 			  ; Уменьшаем его
    cmp     esi,0			  ; Сравниваем с нулем
    jnz     for_abz			  ; Если не ноль то в начало цикла
    ret 				  ; Возвращаем управление
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
  sub_red_1:				  ; random
    cmp     al,1
    jne     sub_red_2
    sub     edx,0x20
    jmp     sub_red_e
  sub_red_2:				  ; gree
    cmp     al,2
    jne     sub_red_3
    sub     edx,0x010101
    jmp     sub_red_e
  sub_red_3:				  ; red
    cmp     al,3
    jne     sub_red_4
    sub     edx,0x0101
    jmp     sub_red_e
  sub_red_4:				  ; green
    cmp     al,4
    jne     sub_red_5
    sub     edx,0x010001
    jmp     sub_red_e
  sub_red_5:				  ; blue
    cmp     al,5
    jne     sub_red_e
    sub     edx,0x010100
    jmp     sub_red_e
  sub_red_e:
    pop     eax
    ret 				  ; Возвращаем управление
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
    ret 				  ; Возвращаем управление
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
    ret 				  ; Возвращаем управление
;end_sub_blue-----------------------------------------------------------------------------------------

draw_result:
    ;.................................................................................................
    ; Большая рамка вывода результата
    ;.................................................................................................
    mov     eax,13			  ; Функция 13 - нарисовать прямоугольник
    mov     edx,0x00666666		  ; цвет-обводки
    mov     ebx,4 shl 16+98		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,9 shl 16+148		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40
    mov     edx,0x00F3F3F3		  ; цвет-фона
    mov     ebx,5 shl 16+96		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,10 shl 16+146		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40

    ;.................................................................................................
    ; Отрисовка результата цвета в hex color2
    ;.................................................................................................
    mov     eax,13			  ; Функция 13 - нарисовать прямоугольник
    ; color 2 - draw fringle
    mov     edx,0x222222		  ; цвет-обводки
    mov     ebx,16 shl 16+15		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,16 shl 16+15		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40			  ; Прерывание (ос рисует прямоугольник)
    ; color 2 - draw color
    mov     edx,[color2]		  ; цвет
    mov     ebx,17 shl 16+13		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,17 shl 16+13		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40			  ; Прерывание (ос рисует прямоугольник)
    ; color 1 - draw fringle
    mov     edx,0x222222		  ; цвет-обводки
    mov     ebx,11 shl 16+15		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,12 shl 16+15		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40			  ; Прерывание (ос рисует прямоугольник)
    ; value - draw fringle
    mov     ebx,34 shl 16+62		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,16 shl 16+15		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40			  ; Прерывание (ос рисует прямоугольник)

    ; color 2 - draw color
    mov     edx,[color] 		  ; цвет
    mov     ebx,12 shl 16+13		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,13 shl 16+13		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40			  ; Прерывание (ос рисует прямоугольник)
    ; value - draw background
    mov     edx,0xFFFFFF		  ; цвет-фона
    mov     ebx,35 shl 16+60		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,17 shl 16+13		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40			  ; Прерывание (ос рисует прямоугольник)

    mov     eax,47			  ; Функция 47 - вывод числа в окно
    mov     ecx,[color] 		  ; число (при bl=0) или указатель (при bl=1)
    mov     esi,0x0			  ; 0xX0RRGGBB
    mov     ebx,256+8 shl 16		  ; параметры преобразования числа в текст (HEX)
    mov     edx,45 shl 16+20		  ; [координата по оси x]*65536 + [координата по оси y]
    int     0x40			  ; Прерывание - выводим результат в окно (HEX)

    mov     eax,4			  ; функция 4: написать текст в окне
    mov     ebx,38*65536+20		  ; [x начальный] *65536 + [y начальный]
    mov     ecx,0x0			  ; цвет текста RRGGBB
    mov     edx,hex			  ; рисуем '#'
    mov     esi,1			  ; длина текста в байтах
    int     0x40

    ;.................................................................................................
    ; Отрисовка r g b значений
    ;.................................................................................................
    call    get_spectr

    movzx   eax,[cred]			  ; красный
    mov     ebx,9*65536+142		  ; [x начальный] *65536 + [y начальный]
    call    draw_value			  ; выводим результат

    movzx   eax,[cgreen]		  ; зеленый
    mov     ebx,32*65536+142		  ; [x начальный] *65536 + [y начальный]
    call    draw_value			  ; выводим результат

    movzx   eax,[cblue] 		  ; синий
    mov     ebx,55*65536+142		  ; [x начальный] *65536 + [y начальный]
    call    draw_value			  ; выводим результат

    movzx   eax,[calpha]		  ; альфа
    mov     ebx,78*65536+142		  ; [x начальный] *65536 + [y начальный]
    call    draw_value			  ; выводим результат

    ;.................................................................................................
    ; Выводим буквы r g b a
    ;.................................................................................................
    mov     eax,4			  ; 4 - вывести строку текста в окно
    mov     ebx,16 shl 16+40		  ; [координата по оси x]*65536 + [координата по оси y]
    mov     ecx,0x0			  ; 0xX0RRGGBB (RR, GG, BB задают цвет текста)
    mov     edx,cname			  ; указатель на начало строки
    mov     esi,1			  ; выводить esi символов
    newline_s:				  ; цикл
      int     0x40			  ; Прерывание
      add     ebx,23 shl 16		  ; Добавляем
      add     edx,1			  ; Добавляем
      cmp     [edx],byte 'x'		  ; Сравнение с байтом х
    jne    newline_s			  ; Если не нуль или не равно

    ;.................................................................................................
    ; Отрисовка слайдеров
    ;.................................................................................................
    mov     eax,13			  ; Функция 13 - нарисовать прямоугольник
    mov     edx,0x222222		  ; цвет-обводки
    mov     ebx,16 shl 16+4		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,51 shl 16+86		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40
    add     ebx,23 shl 16		  ; Начальные координаты по x [к-та y]*65536 + [размер]
    int     0x40
    add     ebx,23 shl 16		  ; Начальные координаты по x [к-та y]*65536 + [размер]
    int     0x40
    add     ebx,23 shl 16		  ; Начальные координаты по x [к-та y]*65536 + [размер]
    int     0x40

    mov     edx,0xFA0919		  ; цвет-обводки
    mov     ebx,17 shl 16+2		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    mov     ecx,52 shl 16+84		  ; Начальные координаты по y [к-та y]*65536 + [размер]
    int     0x40
    mov     edx,0x08CE19		  ; цвет-обводки
    add     ebx,23 shl 16		  ; Начальные координаты по x [к-та y]*65536 + [размер]
    int     0x40
    mov     edx,0x0909FF		  ; цвет-обводки
    add     ebx,23 shl 16		  ; Начальные координаты по x [к-та y]*65536 + [размер]
    int     0x40
    mov     edx,0xE4E4E4		  ; цвет-обводки
    add     ebx,23 shl 16		  ; Начальные координаты по x [к-та y]*65536 + [размер]
    int     0x40

    ;.................................................................................................
    ; Отрисовка ползунков
    ;.................................................................................................
    mov     edx,0x0			  ; цвет-ползунков
    mov     ebx,17 shl 16+11		  ; Начальные координаты по x [к-та x]*65536 + [размер]
    movzx   eax,[cred]			  ; берем значение цвета
    call    draw_slider
    add     ebx,23 shl 16
    movzx   eax,[cgreen]		  ; берем значение цвета
    call    draw_slider
    add     ebx,23 shl 16
    movzx   eax,[cblue] 		  ; берем значение цвета
    call    draw_slider
    add     ebx,23 shl 16
    movzx   eax,[calpha]		  ; берем значение цвета
    call    draw_slider

    ret 				  ; Возвращаем управление
;end_draw_result

draw_slider:
    xor     ecx,ecx
    mov     cl,0x3			  ; будем делить его на 3 так как ползунки длинной 85 пикселей
    div     cl				  ; делим - целое в al остаток в ah
    mov     cl,136			  ; нижняя точка ползунка
    sub     cl,al			  ; cl=cl-al
    shl     ecx,16
    add     ecx,1			  ; задаем начальное положение
    mov     eax,13			  ; Функция 13 - нарисовать прямоугольник
    int     0x40
    push    ebx 			  ; сохраняем для следующего ползунка x+w
    sub     ebx,8			  ; так как ширина линии 11 то отнимаем 8 чтобы кубик стал шириной 3
    add     ebx,8 shl 16		  ; Координаты по x для квадрата на ползунке смещаем на 8 в право
    inc     ecx 			  ; быдлокодим высоту ползунка до 3-х (1+1=2)
    inc     ecx 			  ; (2+1=3) ну вот у нас будет кубик 3х3 на 8 в право от линии
    int     0x40			  ; рисуем
    pop     ebx 			  ; восстанавливаем нормальные координаты
    ret 				  ; Возвращаем управление

;end_slider

draw_value:
    ;.................................................................................................
    ; Вывод числа из строки в указанной области
    ;.................................................................................................
    push    ebx 			  ; сохраняем присланные координаты
    mov     ebx,10			  ; устанавливаем основание системы счисления
    mov     edi,buff			  ; указатель на строку буфера
    call    int2ascii			  ; конвертируем число и ложим как строку в буфер + esi длина
    mov     eax,4			  ; функция 4: написать текст в окне
    pop     ebx 			  ; достаем из стека присланные координаты
    cmp     esi,2			  ; ЦЕНТРИРОВАНИЕ ТЕКСТА
    jne     draw_value_1
    add     ebx,4 shl 16
    jmp     draw_value_e
  draw_value_1:
    cmp     esi,1
    jne     draw_value_e
    add     ebx,7 shl 16
  draw_value_e:
    mov     ecx,0x0			  ; цвет текста RRGGBB
    mov     edx,buff			  ; указатель на начало текста
    int     0x40
    ret 				  ; Возвращаем управление
;end_draw_value

  _read_params:
      mov   [fi.p00], dword 0
      mov   [fi.p04], dword 0
      mov   [fi.p08], dword 0
      mov   [fi.p12], dword 6
      mov   [fi.p16], dword fi.file
      mov   [fi.p20], dword 0
      mov   [fi.p21], dword dat_file
      mcall 70, fi

      xor   eax, eax
      mov   al, [fi.file+2]
      shl   eax, 8
      mov   al, [fi.file+1]
      shl   eax, 8
      mov   al, [fi.file+0]
      mov   [color], eax

      xor   eax, eax
      mov   al, [fi.file+5]
      shl   eax, 8
      mov   al, [fi.file+4]
      shl   eax, 8
      mov   al, [fi.file+3]
      mov   [color2], eax

   ;   mov al,[params+2]
   ;   mov [params_c+0],al
   ;
   ;   mov al,[params+3]
   ;   mov [params_c+1],al
   ;
   ;   mov al,[params+4]
   ;   mov [params_c+2],al
   ;
   ;   mov al,[params+5]
   ;   mov [params_c+3],al
   ;
   ;   mov al,[params+6]
   ;   mov [params_c+4],al
   ;
   ;   mov al,[params+7]
   ;   mov [params_c+5],al
   ;
   ;   mov al,[params+8]
   ;   mov [params_c+6],al
   ;
   ;   mov al,[params+9]
   ;   mov [params_c+7],al
   ;
   ;   mov   esi,params_c
   ;   mov   ecx,16
   ;   call  ascii2int
   ;   mov   [color],eax
   ;
   ;   mov al,[params+11]
   ;   mov [params_c+0],al
   ;
   ;   mov al,[params+12]
   ;   mov [params_c+1],al
   ;
   ;   mov al,[params+13]
   ;   mov [params_c+2],al
   ;
   ;   mov al,[params+14]
   ;   mov [params_c+3],al
   ;
   ;   mov al,[params+15]
   ;   mov [params_c+4],al
   ;
   ;   mov al,[params+16]
   ;   mov [params_c+5],al
   ;
   ;   mov al,[params+17]
   ;   mov [params_c+6],al
   ;
   ;   mov al,[params+18]
   ;   mov [params_c+7],al
   ;
   ;   mov   esi,params_c
   ;   mov   ecx,16
   ;   call  ascii2int
   ;   mov   [color2],eax

      ret

hex_digit:
    ;.................................................................................................
    ; Преобразование в ASCII (вне зависимости от системы счисления)
    ;.................................................................................................
    cmp    dl,10			  ; в dl ожидается число от 0 до 15
    jb	   .less			  ; если dl<10 то переходим
    add    dl,'A'-10			  ; 10->A 11->B 12->C ...
    ret 				  ; Возвращаем управление
    .less:
    or	   dl,'0'			  ; Если система счисления 10-я и менее
    ret 				  ; Возвращаем управление
;end_hex_digit

int2ascii:
    ;.................................................................................................
    ; Преобразование числа в строку
    ;.................................................................................................
    ; eax - 32-х значное число
    ; ebx - основание системы счисления
    ; edi - указатель на строку буфера
    ; Возвращает заполненный буфер и esi - длина строки
    ;pushad
    xor     esi,esi			  ; зануляем счетчик символов
    convert_loop:
    xor     edx,edx			  ; зануляем регистр под остаток
    div     ebx 			  ; eax/ebx - остаток в edx
    call    hex_digit			  ; преобразуем символ
    push    edx 			  ; ложим в стек
    inc     esi 			  ; увеличиваем счетчик
    test    eax,eax			  ; если еще можно делить
    jnz     convert_loop		  ; то делием еще
    cld 				  ; ОБЯЗАТЕЛЬНО сбрасываем флаг направления DF (запись вперёд)
    write_loop: 			  ; иначе
    pop     eax 			  ; достаем из стека в еах
    stosb				  ; записываем в буфер по адресу ES:(E)DI
    dec     esi 			  ; уменьшаем счетчик
    test    esi,esi			  ; если есть что доставать из стека
    jnz     write_loop			  ; то достаём
    mov     byte [edi],0		  ; иначе дописыываем нулевой байт
    ;popad                                 ; восстанавливаем значения регистров
    ; код ниже не имеет ничего общего к функции, просто возвращает еще длинну полученной строки
    mov     edi,buff			  ; указатель на начало текста
    call    str_len
    mov     esi,eax
    ret 				  ; и возвращаем управление
;end_int2ascii

char2byte:
    sub al,'0'
    cmp al,10
    jb	done
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
    ret 				  ; и возвращаем управление
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
    call    draw_result 		  ; Выводим результат
    ret 				  ; и возвращаем управление
;end_get_spectr

str_len:
    ;.................................................................................................
    ; определяет длину строки (вход->EDI ZS offset ; выход->EAX ZS length)
    ;.................................................................................................
	push ecx
	push esi
	push edi

	cld
	xor   al, al
	mov ecx, 0FFFFFFFFh
	mov esi, edi
	repne scasb
	sub edi, esi
	mov eax, edi
	dec eax

	pop edi
	pop esi
	pop ecx

	ret
;end_str_len



;#___________________________________________________________________________________________________
;****************************************************************************************************|
; БЛОК ПЕРЕМЕННЫХ И КОНСТАНТ                                                                         |
;----------------------------------------------------------------------------------------------------/
circle:
    dat_file	db '/sys/settings/wallpaper.dat', 0

    cm		db 12 dup(0)
    color	dd 00000000h		  ; хранит значение выбранного цвета
    color2	dd 00FFFFFFh		  ; хранит значение второго выбранного цвета
    mouse_x	dd 0			  ; хранит глобальную х координату мыши
    mouse_y	dd 0			  ; хранит глобальную у координату мыши
    mouse_f	dd 0			  ; хранит данные о том какая кнопка мыши была нажата
    desctop_w	dd 0			  ; хранит ширину экрана
    desctop_h	dd 0			  ; хранит высоту экрана
    sc		system_colors		  ; хранит структуру системных цветов скина
    title	db 'Palitra v0.75',0	  ; хранит имя программы
    hidden	db 'Hidden',0
    hex 	db '#',0		  ; для вывода решётки как текста
    cname	db 'RGBAx'		  ; хранит разряды цветов (red,green,blue) x-метка конца
    cred	db 0			  ; храним красный спекрт
    cgreen	db 0			  ; храним зеленый спектр
    cblue	db 0			  ; храним синий спектр
    calpha	db 0			  ; храним прозрачность
    larrow	db 0x1A,0
    buff	db '000',0
    bground	db 'BACKGROUND',0	  ; имя кнопки - 14
    pnext	dd 0			  ; счетчик переключения палитры
    renmode	dd 0			  ; режим отрисовки (1-цветовая схема,2-пипетка,3-круговая)
    runmode	dd 1			  ; режим запуска (1-normal, 2-hidden, 3-colordialog)
    params	db 20 dup(0)		  ; приём параметров
 ;   params_c    db 9  dup(0)              ; приёмник для цвета
I_END:


fi:
 .p00:
    rd	    1
 .p04:
    rd	    1
 .p08:
    rd	    1
 .p12:
    rd	    1
 .p16:
    rd	    1
 .p20:
    rb	    1
 .p21:
    rd	    1
 .file:
    rb	    32

I_RDATA: