;Libray from Editbox
; SEE YOU File FAQ.txt and HISTORY. Good Like!
;;;;;;;;;;;;;;;;;;

format MS COFF

public EXPORTS

section '.flat' code readable align 16
include 'macros.inc'
include 'editbox.mac'    ;макрос который должен облегчить жизнь :) специально для editbox

edit_box:
ed_width        equ [edi]               ;ширина компонента
ed_left         equ [edi+4]             ;положение по оси х
ed_top          equ [edi+8]             ;положение по оси у
ed_color        equ [edi+12]            ;цвет фона компонента
shift_color     equ [edi+16]            ;=0x6a9480 для примера возьем желеный цвет бокса
ed_focus_border_color   equ [edi+20]    ;цвет рамки компонента
ed_blur_border_color    equ [edi+24]    ;цвет не активного компонента
ed_text_color   equ [edi+28]            ;цвет текста
ed_max          equ [edi+32]                    ;кол-во символов которые можно максимально ввести
ed_text         equ [edi+36]                    ;указатель на буфер
ed_flags        equ [edi+40]            ;флаги
ed_size equ [edi+44]                    ;кол-во символов
ed_pos  equ [edi+48]                    ;позиция курсора
ed_offset       equ [edi+52]            ;смещение
cl_curs_x       equ [edi+56]            ;предыдущее координата курсора по х
cl_curs_y       equ [edi+60]            ;предыдущее координата курсора по у
ed_shift_pos    equ [edi+64]            ;положение курсора
ed_shift_pos_old equ [edi+68]           ;старое положение курсора
.draw:
        pusha
        mov     eax,9
        push    procinfo
        pop     ebx
        or      ecx,-1
        mcall
;--- рисуем рамку ---
        mov     edi,dword [esp+36]
        call    .draw_border            ; Функция стабильна
.draw_bg_cursor_text:
;--- изменяем смещение, если надо ---
        call    .check_offset           ;вычисление позиции курсора стабильна
;--- рисуем внутреннюю область ---
        call    .draw_bg                ;нарисовать прямоугольник рабочей области
;---- рисуем выделение, по shift если есть
        call    .draw_shift
.draw_cursor_text:
;--- рисуем курсор ---
        ;--- может его не надо рисовать ----
        test    word ed_flags,ed_focus
        je     @f
        call    .draw_cursor
@@:
        call    .draw_text
;        ret
;;;;;;;;;;;;;;;;;;;;;;;;;;
;Общий выход из editbox для всех функций и пост обработчиков
;;;;;;;;;;;;;;;;;;;;;;;;;;
.editbox_exit:
        popa
        ret 4

;==========================================================
;=== обработка клавиатуры =================================
;==========================================================
edit_box_key:
pusha
        mov     edi,dword [esp+36]
        test    word ed_flags,ed_focus ; если не в фокусе, выходим
        je      edit_box.editbox_exit
        test    word ed_flags,ed_mouse_on
        jne     edit_box.editbox_exit

;Проверка нажат shift ?
        call    edit_box_key.check_shift
;----------------------------------------------------------
;--- проверяем, что нажато --------------------------------
;----------------------------------------------------------
use_key_process  backspase,delete,left,right,home,end,insert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Заглушка на обработку клавиш вверх и вниз т.е. при обнаружении этих кодов происходит выход из обработчика
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_key_no_process   up,down,esc
;--- нажата другая клавиша ---
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Проверка установлен ли флаг при котором нужно выводить только цифры в нужном боксе если такойнеобходимости нет нужно закоментировать макрос
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_key_figures_only
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;проверка на shift был ли нажат
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
are_key_shift_press
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; проверяем, находится ли курсор в конце + дальнейшая обработка
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
are_key_cur_end
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Обработка клавиш insert,delete.backspase,home,end,left,right
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_work_key

;==========================================================
;=== обработка мыши =======================================
;==========================================================
;save for stdcall ebx,esi,edi,ebp
edit_box_mouse:
        pop     eax     ;opint to back
        pop     edx     ;scr_w
        pop     ecx     ;ebp     ;scr_h
        push    eax

;        pop     eax
;        pop     edx     ;scr_w
;        pop     ecx     ;scr_h
;        push    eax     ;pointer to back
pusha
        mov     ebp,ecx

        mov     edi,dword [esp+36]
;debug
;----------------------------------------------------------
;--- получаем состояние кнопок мыши -----------------------
;----------------------------------------------------------
        mcall   37,2
;----------------------------------------------------------
;--- проверяем состояние ----------------------------------
;----------------------------------------------------------
        test    eax,1
        jnz     edit_box_mouse.mouse_left_button
        and     word ed_flags,ed_mouse_on_off
        xor     ebx,ebx
        mov     dword [mouse_flag],ebx
        jmp     edit_box.editbox_exit
.mouse_left_button:
;----------------------------------------------------------
;--- блокировка от фокусировки в других боксах при попадании на них курсора
;----------------------------------------------------------
        mov     eax,dword [mouse_flag]
        test    eax,eax
        jz      @f
        cmp     eax,edi
        je      @f
        jmp     edit_box_mouse._blur
;----------------------------------------------------------
;--- получаем координаты мыши относительно 0 т.е всей области экрана
;----------------------------------------------------------
@@:     mcall   37,0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функция обработки  мышки получение координат и проверка их + выделения
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_work_mouse
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Общие функции обработки
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_general_func
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функции для работы с key
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_key_func
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функции для работы с mouse
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_mouse_func ;scr_w
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Bit mask from editbox
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ed_figure_only= 1000000000000000b   ;одни символы
ed_always_focus= 100000000000000b
ed_focus=                     10b   ;фокус приложения
ed_shift_on=                1000b   ;если не установлен -значит впервые нажат shift,если был установлен, значит мы уже что - то делали удерживая shift
ed_shift_on_off=1111111111110111b
ed_shift=                    100b   ;включается при нажатии на shift т.е. если нажимаю
ed_shift_off=   1111111111111011b
ed_shift_bac=              10000b   ;бит для очистки выделеного shift т.е. при установке говорит что есть выделение
ed_shift_bac_cl=1111111111101111b   ;очистка при удалении выделения
ed_shift_cl=    1111111111100011b
ed_shift_mcl=   1111111111111011b
ed_left_fl=               100000b
ed_right_fl=    1111111111011111b
ed_offset_fl=            1000000b
ed_offset_cl=   1111111110111111b
ed_insert=              10000000b
ed_insert_cl=   1111111101111111b
ed_mouse_on =          100000000b
ed_mous_adn_b=         100011000b
ed_mouse_on_off=1111111011111111b
ed_height=14 ; высота

;----------------------------------------------------
;CheckBox
;----------------------------------------------------
check_box_draw:
;ch_struc_size=24
ch_left equ [edi]    ;координата начала рисования по х
ch_top equ [edi+2]   ;координата начала рисования по у
ch_text_margin equ [edi+4]      ;=4 расстояние от прямоугольника чек бокса до надписи 
ch_size equ [edi+8]             ;12 размер квадрата чек бокса 
ch_color equ [edi+12]
ch_border_color equ [edi+16] ; or [edi+4] ;цвет рамки checkbox ее можно задать самостоятельно
ch_text_color equ   [edi+20];[edi+4]  ;цвет текста
ch_text_ptr equ [edi+24]    ;указатель на начало текстовой строки 
ch_text_length equ [edi+28]
ch_flags equ [edi+32]       ;флаги 

       pusha   ;сохраним все регистры
       mov     edi,dword [esp+36]
       mov eax,13 
       movzx ebx,word ch_left 
       shl ebx,16 
       add ebx,ch_size 
       mov ecx,ch_top 
       shl ecx,16 
       add ecx,dword ch_size 
       mov edx,dword ch_border_color 
       mcall ;рисуем рамку 

       mov edx,dword ch_color 
       add ebx,1 shl 16 - 2 
       add ecx,1 shl 16 - 2 
       mcall ;закрашиваем внутренности чекбокса 

       test dword ch_flags,2  ;достать значение бита из переменной и поместить в  флаг CF 
       jz   @f                ;в если CF=1, то выполним следующую процедуру иначе перейти на нижнюю @@
       call check_box_draw_ch  ;нарисовать включенный чек бокс
@@:
;----------------------------
;расчет куда будет произведен вывод текста
;----------------------------
        movzx ebx,word ch_left        ;загрузить значение х для чек бокса
        add   ebx,dword ch_size
        add   ebx,dword ch_text_margin;добавить размер стороны и расстояние на котором начнется вывод текста
        shl   ebx,16                ;сдвинем на 16 разрядов в лево (умножим на 65536)
        mov   bx,word ch_top        ;загрузим значение по y
        add   ebx,ch_size
        mov   ecx,dword ch_text_color        ;загрузим цвет надписи + flags
        sub   ebx,7        ;добавим значение длины стороны -9+2
        
        mov   edx,dword ch_text_ptr                ;укажем адрес от куда нужно выводить строку
        mov   esi,dword ch_text_length
        ;внесем в eax значение вывода надписи на канву
        mov   eax,4
        mcall                  ;Вывод 
popa                              ;восстановить значения регистров из стека
ret 4                             ;выйдем из процедуры

check_box_clear_ch:                        ;очистка чек бокса
        mov   edx,dword ch_color   ;цвет внутри чек бокса
        jmp   @f             ;безусловный прыжок на нижнюю метку @@

check_box_draw_ch:            ;нарисовать включенный чек бокс
        mov   edx,dword ch_border_color        ;загрузить цвет
@@:
        movzx ebx,word ch_left  ;загрузить координату по х
        mov   eax,dword ch_size
        push  ax
        shr   eax,2
        add   ebx,eax          ;добавить (сторона прямоугольника/3)
        shl   ebx,16            ;сдвинем на 16 разрядов в лево (умножим на 65536)
        pop   bx
        shr   bx,1              ;загрузить (сторона прямоугольника/2)
        mov   bp,bx             ;сохраним регистр bx в регистре указателя базы

        movzx ecx,word ch_top ;загрузить координату по у
        mov   eax,dword ch_size
        shr   eax,2
        add   ecx,eax         ;добавить (сторона прямоугольника/4)
        shl   ecx,16          ;сдвинем на 16 разрядов в лево (умножим на 65536)
        mov   cx,bp           ;загрузим значения регистра указателя базы в cx
        mov   eax,13          ;в eax - значения функции для вывода полосы т.е. по сути прямоугольника, который отображает включенный компонент чек бокс
        mcall            ;вывод
ret                                ;выйти из процедуры
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Обработчик mouse
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
check_box_mouse:      ;обработка мыши 
pusha
        mov     edi,dword [esp+36]
        mov     eax,37           ;будем что то делать если у нас что - нить нажато
        mov     ebx,2            ;внести в регистр значение 2
        mcall             ;проверка не нажал ли пользователь кнопку мышки
        test    eax,eax   ;проверка если у нас в eax=0, то установим флаг и выйдем
        jnz     @f         ;перейти на нижнюю метку @@
        btr     dword ch_flags,2  ;извлечение значения заданного бита в флаг cf и изменение его значения на нулевое. 
        jmp     check_box_mouse_end
        
@@:
        bts  dword ch_flags,2   ;проверка флага т.е. перенос в cf значение бита и установка бита в состояние включено
        jc   check_box_mouse_end         ;если CF=1 то перейти  в конец т.е. это выход
        mov  esi,dword ch_text_length ;загрузить кол-во символов в текстовой строке
        ;Умножение на 6 Быстрое умножение можно воспользоваться любым мз методов, но на старых Процессорах (386,486,P1)быстрее будет с инструкцией Lea
        ;lea esi,[eax*2+eax]
        ;shl eax,1
        imul esi,6             ; или можно и так умножить на 6
        add  esi,dword ch_text_margin ;добавить 3 - расстояние от чек бокса до надписи

        mov  eax,37             ;получим координаты мышки 
        mov  ebx,1              ;добавить 1
        mcall               ;получить координаты курсора относительно окна 
        
        movzx ebx,word ch_top  ;загрузить в bx значение координаты у
        cmp   ax,bx              ;сравнить с с координатой курсора
        jl   check_box_mouse_end          ;SF <> OF если меньше 
        add   ebx,dword ch_size        ;добавить размер 
        cmp   ax,bx              ;сравнить
        jg   check_box_mouse_end          ;ZF = 0 и SF = OF если больше 
             
        shr   eax,16              ;разделим на 65536 или просто сдвинем биты на 16 значений
        movzx ebx,word ch_left  ;произведем аналогичное сравнение
        cmp   ax,bx            ;сравнить регистры
        jl   check_box_mouse_end        ;если меньше
        add   ebx,dword ch_size      ;добавить длину стороны прямоугольника
        add   ebx,esi          ;Учесть в значении по х еще и длину надписи к чекбоксу
        cmp   ax,bx            ;стравнить регистры
        jg   check_box_mouse_end        ;если больше 

        bts  dword ch_flags,1  ;извлечение значения заданного бита в флаг cf и изменение его значения на 1. 
        jc   @f                ;CF=1 то перейти на нижнюю @@
        
        call check_box_draw_ch        ;отобразить включенный чек бокс
;        mov   dword [esp+24],1 ;дальнейшая проверка чек боксов бесмыслена, по этому в стек, где располагается ecx поместитм 0 
        jmp  check_box_mouse_end       ;выйти 
@@:
        btr   word ch_flags,1  ;извлечение значения заданного бита в флаг cf и изменение его значения на нулевое. 
        call check_box_clear_ch         ;выключить чек бокс т.е. на месте закрашенного прямоугольника отобразить цвет фона.
check_box_mouse_end:
popa                                ;восстановить регистры из стека
ret  4                              ;выйти

;--------------------------------------------------
;radiobutton Group
;--------------------------------------------------
option_box_draw_box:
option_group equ [edi]
op_left equ [edi+4]    ;координата начала рисования по х
op_top equ [edi+6]     ;координата начала рисования по у
op_text_margin equ [edi+8]      ;=4 расстояние от прямоугольника чек бокса до надписи 
op_size equ [edi+12]             ;12 размер квадрата чек бокса 
op_color equ [edi+16]
op_border_color equ [edi+20] ; or [edi+4] ;цвет рамки checkbox ее можно задать самостоятельно
op_text_color equ   [edi+24];[edi+4]  ;цвет текста
op_text_ptr equ [edi+28]    ;указатель на начало текстовой строки 
op_text_length equ [edi+32]
op_flags equ [edi+36]       ;флаги

        pusha   ;сохраним все регистры

        movzx ebx,word op_left 
        shl ebx,16 
        add ebx,dword op_size 
        movzx ecx,word op_top 
        shl ecx,16 
        add ecx,dword op_size 
        mov edx,dword op_border_color 
        mov eax,13 
        mcall ;рисуем рамку 
  
        mov edx,dword op_color 
        add ebx,1 shl 16 - 2 
        add ecx,1 shl 16 - 2 
        mcall ;закрашиваем внутренности чекбокса 

;        mov     eax,dword option_group
;        mov     dword eax,[eax]
;        cmp     eax,edi
;        jne     @f
;        call    option_box_draw_op  ;нарисовать включенный чек бокс


;----------------------------
;расчет куда будет произведен вывод текста
;----------------------------
@@:     movzx ebx,word op_left        ;загрузить значение х для чек бокса
        add   ebx,dword op_size
        add   ebx,dword op_text_margin;добавить размер стороны и расстояние на котором начнется вывод текста
        shl   ebx,16                ;сдвинем на 16 разрядов в лево (умножим на 65536)
        mov   bx,word op_top        ;загрузим значение по y
        add   ebx,op_size
        mov   ecx,dword op_text_color        ;загрузим цвет надписи + flags
        sub   ebx,7        ;добавим значение длины стороны -9+2
        
        mov   edx,dword op_text_ptr                ;укажем адрес от куда нужно выводить строку
        mov   esi,dword op_text_length
        ;внесем в eax значение вывода надписи на канву
        mov   eax,4
        mcall                  ;Вывод 
popa                           ;восстановить значения регистров из стека
ret                            ;выйдем из процедуры

option_box_clear_op:                      ;очистка чек бокса
        mov     edx,dword op_color    ;цвет внутри чек бокса
        jmp     @f              ;безусловный прыжок на нижнюю метку @@


option_box_draw_op:            ;нарисовать включенный чек бокс
        mov   edx,dword op_border_color        ;загрузить цвет
@@:
        movzx ebx,word op_left  ;загрузить координату по х
        mov   eax,dword op_size
        push  ax
        shr   eax,2
        add   ebx,eax          ;добавить (сторона прямоугольника/3)
        shl   ebx,16            ;сдвинем на 16 разрядов в лево (умножим на 65536)
        pop   bx
        shr   bx,1              ;загрузить (сторона прямоугольника/2)
        mov   bp,bx             ;сохраним регистр bx в регистре указателя базы

        movzx ecx,word op_top ;загрузить координату по у
        mov   eax,dword op_size
        shr   eax,2
        add   ecx,eax         ;добавить (сторона прямоугольника/4)
        shl   ecx,16          ;сдвинем на 16 разрядов в лево (умножим на 65536)
        mov   cx,bp           ;загрузим значения регистра указателя базы в cx
        mov   eax,13          ;в eax - значения функции для вывода полосы т.е. по сути прямоугольника, который отображает включенный компонент чек бокс
        mcall            ;вывод
ret                                ;выйти из процедуры
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Обработчик mouse
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
option_box_mouse_op:                 ;обработка мыши 
pusha
        mov     eax,37          ;будем что то делать если у нас что - нить нажато
        mov     ebx,2           ;внести в регистр значение 2
        mcall           ;проверка не нажал ли пользователь кнопку мышки
        test    eax,eax    ;проверка если у нас в eax=0, то установим флаг и выйдем
        jnz     @f         ;перейти на нижнюю метку @@

        jmp     option_box_mouse_end
                           ;если ничего не произошло, то восстановим значения регистров из стека
                           ;выход
@@:
        mov     esi,dword op_text_length ;загрузить кол-во символов в текстовой строке
        ;Умножение на 6 Быстрое умножение можно воспользоваться любым мз методов, но на старых Процессорах (386,486,P1)быстрее будет с инструкцией Lea
        ;lea    esi,[eax*2+eax]
        ;shl    eax,1
        imul    esi,6               ; или можно и так умножить на 6
        xor     ebx,ebx
        add     esi,dword op_text_margin   ;добавить 3 - расстояние от чек бокса до надписи
        
        mov     eax,37          ;получим координаты мышки 
        inc     ebx             ;добавить 1
        mcall                ;получить координаты курсора относительно окна 

        movzx   ebx,word op_top           ;загрузить в bx значение координаты у
        cmp     ax,bx               ;сравнить с с координатой курсора
        jl      option_box_mouse_end          ;SF <> OF если меньше 
        add     ebx,dword op_size          ;добавить размер 
        cmp     ax,bx               ;сравнить
        jg      option_box_mouse_end          ;ZF = 0 и SF = OF если больше 
        
        shr     eax,16              ;разделим на 65536 или просто сдвинем биты на 16 значений
        movzx   ebx,word op_left          ;произведем аналогичное сравнение
        cmp     ax,bx                ;сравнить регистры
        jl      option_box_mouse_end           ;если меньше
        add     ebx,dword op_size          ;добавить длину стороны прямоугольника
        add     ebx,esi              ;Учесть в значении по х еще и длину надписи к чекбоксу
        cmp     ax,bx                ;стравнить регистры
        jg      option_box_mouse_end           ;если больше 
        mov     eax,dword option_group
        mov     [eax],edi

option_box_mouse_end:
popa                              ;восстановить регистры из стека
ret                               ;выйти

option_box_draw:
        pusha

        mov     eax,dword [esp+36]
@@:     mov     edi,dword [eax]
        test    edi,edi
        je      option_check
        call    option_box_draw_box
        add     eax,4
        jmp    @b 

option_check:
        mov     eax,dword [esp+36]
@@:     mov     edi,dword [eax]
        test    edi,edi
        je      @f
        
        mov     ebx,dword [edi]
        mov     ebx,dword [ebx]
        cmp     edi,ebx
        jne     .clear_op
        
        pusha
        call    option_box_draw_op
        popa
        add     eax,4
        jmp     @b

.clear_op:
        pusha
        call    option_box_clear_op
        popa
        add     eax,4
        jmp     @b


@@:     popa
        ret 4 
        ; exit вообще :)


option_box_mouse:
        pusha

        mov     eax,dword [esp+36]
@@:     mov     edi,dword [eax]
        test    edi,edi
        je      option_check
        call    option_box_mouse_op
        add     eax,4
        jmp    @b 


align 16
EXPORTS:

        dd      sz_edit_box,            edit_box
        dd      sz_edit_box_key,        edit_box_key
        dd      sz_edit_box_mouse,      edit_box_mouse
        dd      szVersion_ed,           0x00000001
        dd      sz_check_box_draw,      check_box_draw
        dd      sz_check_box_mouse,     check_box_mouse
        dd      szVersion_ch,           0x00000001
        dd      sz_option_box_draw,     option_box_draw
        dd      sz_option_box_mouse,    option_box_mouse
        dd      szVersion_op,           0x00000001
        dd      0,0

sz_edit_box            db 'edit_box',0
sz_edit_box_key        db 'edit_box_key',0
sz_edit_box_mouse      db 'edit_box_mouse',0
szVersion_ed           db 'version_ed',0
sz_check_box_draw      db 'check_box_draw',0
sz_check_box_mouse     db 'check_box_mouse',0
szVersion_ch           db 'version_ch',0
sz_option_box_draw     db 'option_box_draw',0
sz_option_box_mouse    db 'option_box_mouse',0
szVersion_op           db 'version_op',0

;;;;;;;;;;;
;;Data
;;;;;;;;;;;
align 16
mouse_flag dd 0x0
procinfo process_information