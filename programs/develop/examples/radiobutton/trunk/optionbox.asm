;компонент OptionBox (основан на Checkbox)
;Огромная благодарность Maxxxx32, Diamond, Heavyiron и другим программистам, и их программам, без
;которых я не смог бы написать этот компонент. 
;21.02.2007 модернизация и поддержка двух разных вариантов с использованием сис цветов и старой схемой
;19.02.2007 общее улучшение кода, уменьшение размера и использование системных цветов для отображения optionkbox
;16.02.2007 дата создания компонента
;<Lrz>  - Теплов Алексей  www.lrz.land.ru

;заголовок приложения
use32		; транслятор, использующий 32 разрядных команды
    org 0x0		; базовый адрес кода, всегда 0x0
    db 'MENUET01'	; идентификатор исполняемого файла (8 байт)
    dd 0x1		; версия формата заголовка исполняемого файла
    dd start		; адрес, на который система передаёт управление
	                ; после загрузки приложения в память
    dd i_end		; размер приложения
    dd (i_end+0x100) and not 3	; Объем используемой памяти, для стека отведем 0х100 байт и выровним на грницу 4 байта
    dd (i_end+0x100) and not 3	; расположим позицию стека в области памяти, сразу за телом программы. Вершина стека в диапазоне памяти, указанном выше
    dd 0x0,0x0		; указатель на строку с параметрами.
                 ;    если после запуска не равно нулю, приложение было
                 ;    запущено с параметрами из командной строки

		     ;    указатель на строку, в которую записан путь,
                 ;    откуда запущено приложение
;------------------
	include	'..\..\..\..\macros.inc'
	include 'optionbox.inc'	;включить файл opeck.inc

      version_op              ;вариант, при котором используются цвета, которые задает пользователь
;	version_op1             ;цвета берутся из системы
	use_option_box		;используя макросы,внести процедуры для рисования optionbox
align 16
;Область кода
start:				;Точка входа в программу
	mov  eax,48             ;получить системные цвета
	mov  ebx,3
	mov  ecx,sc
	mov  edx,sizeof.system_colors
	mcall

	mov	eax,40		;установить маску для ожидаемых событий
	mov 	ebx,0x25          ;система будет реагировать только на сообщение о перерисовке,нажата кнопка, определённая ранее, событие от мыши (что-то случилось - нажатие на кнопку мыши или перемещение; сбрасывается при прочтении)
        mcall
red_win:
    call draw_window 		;первоначально необходимо нарисовать окно
still:				;основной обработчик 
     mov  eax,10 		      ;Ожидать события
     mcall      		;ожидать событие в течение 2 миллисекунд
  
    cmp al,0x1    ;если изменилось положение окна
    jz red_win
    cmp al,0x3    ;если нажата кнопка то перейти
    jz button
     mouse_option_boxes option_boxes,option_boxes_end  ;проверка чек бокса      
    jmp still    ;если ничего из перечисленного то снова в цикл
button:
;    mov eax,17		;получить идентификатор нажатой клавиши
;    mcall
;    test ah,ah		;если в ah 0, то перейти на обработчик событий still
;    jz  still
    or eax,-1       ;в eax,-1 - 5 ,байтов у нас же только 3  выйти 
    mcall ;далее выполняется выход из программы

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
draw_window:		;рисование окна приложения
    mov eax,12		;в регистр внести значение = 12
    mov ebx,1 		;присвоить 1
    mcall

    xor  eax,eax		;обнулить eax
    mov  ebx,50*65536+180	;[координата по оси x]*65536 + [размер по оси x]
    mov  ecx,30*65536+200	;[координата по оси y]*65536 + [размер по оси y]
    mov  edx,[sc.work] 	         ; color of work area RRGGBB,8->color gl
    or   edx,0xb3000000
    mov  edi,hed
    mcall			;нарисовать окно приложения

draw_option_boxes option_boxes,option_boxes_end ;рисование чекбоксов

    mov eax,12 			;Функция 12 - начать/закончить перерисовку окна.
    mov ebx,2			;Подфункция 2 - закончить перерисовку окна.
    mcall
    ret

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные 
;Формат данных чек бокса:
;10 - координата чек бокса по х 
;30 - координата чек бокса по у
;0 
;0 - цвет рамки чек бокса
;0 - цвет текста надписи
;op_text.1 - указатель на начало строки
;option_group1 - это признак группы, т.е. этот код может обрабатывать много групп из optibox
;op_text.e1-op_text.1 - длина строки
;
align 16
option_boxes:
;op1 option_box1 option_group1,10,15,op_text.1,op_text.e1-op_text.1
;op2 option_box1 option_group1,10,30,op_text.2,op_text.e2-op_text.2
;op3 option_box1 option_group1,10,45,op_text.3,op_text.e3-op_text.3
;op11 option_box1 option_group2,10,80,op_text.1,op_text.e1-op_text.1
;op12 option_box1 option_group2,10,95,op_text.2,op_text.e2-op_text.2
;op13 option_box1 option_group2,10,110,op_text.3,op_text.e3-op_text.3
;
;struc option_box point_gr,left,top,color,border_color,text_color,text,text_length
op1 option_box option_group1,10,15,0xffffff,0,0,op_text.1,op_text.e1-op_text.1
op2 option_box option_group1,10,30,0xFFFFFF,0,0,op_text.2,op_text.e2-op_text.2
op3 option_box option_group1,10,45,0xffffff,0,0,op_text.3,op_text.e3-op_text.3
op11 option_box option_group2,10,80,0xffffff,0,0,op_text.1,op_text.e1-op_text.1
op12 option_box option_group2,10,95,0xffffff,0,0,op_text.2,op_text.e2-op_text.2
op13 option_box option_group2,10,110,0xffffff,0,0,op_text.3,op_text.e3-op_text.3

option_boxes_end:

op_text:		; Сопровождающий текст для чек боксов
.1 db 'Option_Box #1' 
.e1:
.2 db 'Option_Box #2'
.e2:
.3 db 'Option_Box #3'
.e3:

option_group1	dd op1  ;указатели, они отображаются по умолчанию, когда выводится 
option_group2	dd op11 ;приложение

hed db 'Optionbox [21.02.2007]',0	;заголовок приложения
sc     system_colors
i_end:			;конец кода