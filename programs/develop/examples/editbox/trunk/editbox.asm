;Распространяется по лицензии GPL  SEE YOU File FAQ.txt and HISTORY. Good Like! 
;Оптимизированный компонент EditBox (Исходный вариант от Maxxxx32)
;Оптимизация команд.
;<Lrz>  - Теплов Алексей  www.lrz.land.ru
;заголовок приложения
use32           ; транслятор, использующий 32 разрядных команды
    org 0x0             ; базовый адрес кода, всегда 0x0
    db 'MENUET01'       ; идентификатор исполняемого файла (8 байт)
    dd 0x1              ; версия формата заголовка исполняемого файла
    dd start            ; адрес, на который система передаёт управление
                        ; после загрузки приложения в память
    dd i_end            ; размер приложения
    dd i_end            ; Объем используемой памяти, для стека отведем 0х100 байт и выровним на грницу 4 байта
    dd i_end            ; расположим позицию стека в области памяти, сразу за телом программы. Вершина стека в диапазоне памяти, указанном выше
    dd 0x0,0x0          ; указатель на строку с параметрами.
        include 'macros.inc'
        include 'editbox.inc'
align 4
        use_edit_box procinfo,22,5
;Область кода
start:                          ;Точка входа в программу
        mcall   40,0x27         ;установить маску для ожидаемых событий
 ;система будет реагировать только на сообщение о перерисовке,нажата кнопка, определённая ранее, событие от мыши (что-то случилось - нажатие на кнопку мыши или перемещение; сбрасывается при прочтении) и нажатие клавиши
red_win:
    call draw_window            ;первоначально необходимо нарисовать окно
align 4
still:                          ;основной обработчик 
        mcall   10              ;Ожидать события
        cmp al,0x1    ;если изменилось положение окна
        jz red_win
        cmp al,0x2    ;если нажата клавиша то перейти
        jz key
        cmp al,0x3    ;если нажата кнопка то перейти
        jz button
        mouse_edit_boxes editboxes,editboxes_end
        jmp still    ;если ничего из перечисленного то снова в цикл
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
button:
        mcall   17      ;получить идентификатор нажатой клавиши
        test ah,ah              ;если в ah 0, то перейти на обработчик событий still
        jz  still
        mcall   -1
key:
        mcall   2       ;загрузим значение 2 в регистор eax и получим код нажатой клавиши
        key_edit_boxes editboxes,editboxes_end    
    jmp still

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
align 4
draw_window:            ;рисование окна приложения
        mcall   12,1
        mcall   0,(50*65536+390),(30*65536+200),0xb3AABBCC,0x805080DD,hed
        draw_edit_boxes editboxes,editboxes_end,use_f9,procinfo  ;рисование edit box'ов
        mcall   12,2
    ret
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные 
editboxes:
edit1 edit_box 168,5,10,0xffffff,0x6a9480,0,0,0,99,ed_buffer.2,ed_figure_only
edit2 edit_box 250,5,30,0xffffff,0x6a9480,0,0xAABBCC,0,308,hed,ed_focus,53,53
edit3 edit_box 35,5,50,0xffffff,0x6a9480,0,0,0,9,ed_buffer.3,ed_figure_only
edit4 edit_box 16,5,70,0xffffff,0x6a9480,0,0,0,1,ed_buffer.4,ed_figure_only
editboxes_end:
data_of_code dd 0
mouse_flag dd 0x0
hed db   'EDITBOX optimization and retype <Lrz> date 09.05.2007',0
rb  256
ed_buffer:
;.1: rb 514;256
.2: rb 101
.3: rb 11
.4: rb 3
;два запасных байта необходимы для того что бы не пепереписать следующией байты, в конце буфера 0
buffer_end:
align 16
procinfo:
rb 1024 ;1 кб для получения общей структуры 
rb 1024 ;Stack
i_end:  
