; --------------------------------------------------------
; KJ|ABuIIIA - клавиатурный тренажёр для операционной системы Колибри.
;---------------------------------------------------------------------
; version:	0.95
; last update:  19/08/2011
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      1) Checking for "rolled up" window
;               2) Code optimization
;               3) Clearing source
;---------------------------------------------------------------------
; version:	0.90
; last update:  24/07/2010
; changed by:   Андрей Михайлович (Dron2004)
;
; Последние изменения в исходном коде: 24.07.2010 21:15 GMT+6
;---------------------------------------------------------------------
	use32
	org 0x0
	;Заголовок
	db 'MENUET01'
	dd 0x01
	dd START
	dd IM_END
	dd I_END
	dd stacktop
	dd params
	dd 0x0
;---------------------------------------------------------------------
include 'lang.inc'
include '../../../macros.inc'
;---------------------------------------------------------------------
;Код программы
START:
;Инициализируем кучу
	mcall	68,11
;	call	get_screen_params
	mov	eax,params
	cmp	[eax],byte 0x0
	je	start_with_internal_text  ; Параметры не заданы

; Параметр задан! Пытаемся этим воспользоваться
; Необходимо определить размер файла... Вперёд!!!
;	mov	eax,5
;	mov	[arg1],eax
;	xor	eax,eax
;	mov	[arg2],eax
;	mov	[arg3],eax
;	mov	[arg4],eax
;	mov	eax,filedatastruct
;	mov	[arg5],eax
;	mov	eax,[0x0000001C]
;	mov	[arg7],eax
;	mcall	70,filestruct
;	test	eax,eax
;	jne	start_with_internal_text	;Ошибка
;	mov	eax, dword [size]
;	mov	[filesize], eax  ;теперь размер файла у нас в filesize
;;Выделяем блок памяти под файл
;	mov	ecx, [filesize]
;	inc	ecx  ;Выделим дополнительно один байт для того, чтобы добавить 0xFF
;		 ;защита от "битых" KLA-файлов
;	mcall	68,12
;	mov	[datastart], eax	;В переменной ДатаСтарт теперь находится указатель
;	add	eax, [filesize]
;	mov	bh, 0xFF
;	mov	[eax], bh
;;Собственно, считываем файл в память
;	xor	eax,eax
;	mov	[arg1],eax   ;Номер подфункции
;	mov	[arg2],eax   ;Смещение в файле
;	mov	[arg3],eax   ;Старший dword смещения
;	mov	eax,[filesize]
;	mov	[arg4],eax   ;Размер файла
;	mov	eax,[datastart]
;	mov	[arg5],eax   ;Указатель на данные
;	mov	eax,par
;	mov	[arg7],eax   ;Указатель на ASCIIZ-строку
;	mcall	70,filestruct
  
 mov    ecx, eax
 mov    eax, 68
 mov    ebx, 27  
 int    64
 mov	   [datastart], eax  
 mov    [filesize], edx  
 
	test	eax,eax
	jnz	initialize_variables
start_with_internal_text:
	mov	edx, string
	mov	[datastart], edx
;	mov	edx, string         ; Используем встроенный текст
initialize_variables:
	mov	edx,[datastart]
	mov	[currentsymb],edx	; Начальная инициализация переменных
	mov	[startline],edx
	mov	[lastsymb],edx
	mov	[lessonstart],edx
	xor	eax,eax
	inc	eax
	mov	[currentline], eax
	call	count_lines
;---------------------------------------------------------------------
redraw:
	call	draw_window
;---------------------------------------------------------------------
;Основной цикл
event_wait:
	mcall	10
	cmp	eax, 1  ;Перерисовка окна нужна
	je	redraw
	cmp	eax, 2  ;Клавиша нажата
	je	key
	cmp	eax, 3  ;По кнопке жмахнули
	je	button
	jmp	event_wait
;---------------------------------------------------------------------
key:
	mcall	2	;Теперь код нажатой клавиши в ah
	push	eax
;Запускаем счётчик времени для последующего
; определения скорости набора
	mov	eax, [currentsymb]
	cmp	eax, [lessonstart]
	jne	not_first_symbol
	mov	eax,[mistakes]
	test	eax,eax
	jne	not_first_symbol
	mcall	26,9
	mov	[typestarttime], eax
not_first_symbol:
	pop	eax
;Проверяем, не закончился ли текст
	mov	esi, [currentsymb]
	mov	al, byte [esi]
	cmp	al, 0xFF
	je	text_end
	cmp	al, 0xFE
	je	text_end
	push	ebx
	mov	ebx, [currentsymb]
	mov	al, byte [ebx]
	pop	ebx
	test	al,al
	jne	not_new_line
	cmp	ah, 0x0D
	je	correct_key_nl
	cmp	ah, 0x20
	je	correct_key_nl
	jmp	not_this_key
;---------------------------------------------------------------------
correct_key_nl:
	mov	eax, [currentsymb]
	inc	eax
	mov	[startline], eax
	mov	eax, [currentsymb]
	inc	eax
	mov	[currentsymb], eax
	mov	[lastsymb], eax
	mov	eax, [currentline]
	inc	eax
	mov	[currentline], eax
	mov	eax, [symbtyped]	 ;Увеличим число введённых символов на 1
	inc	eax
	mov	[symbtyped], eax
	call	count_speed
;	call	draw_speed
	jmp	redraw
;---------------------------------------------------------------------
not_new_line:
	cmp	ah, al
	jne	not_this_key
correct_key:
	mov	eax, [currentsymb]
	inc	eax
	mov	[currentsymb], eax
	mov	[lastsymb], eax
	mov	eax, [symbtyped]	 ;Увеличим число введённых символов на 1
	inc	eax
	mov	[symbtyped], eax
	call	count_speed
;	call	draw_speed
	call	redraw_2_symbols
	jmp	event_wait
;---------------------------------------------------------------------
not_this_key:
	mov	esi, [mistakes]
	inc	esi
	mov	[mistakes], esi
	call	redraw_mistakes
	jmp	event_wait
;---------------------------------------------------------------------
text_end:
; /// Препятствует миганию окна по окончании урока
; при нажатии клавиш
	mov	esi, [startline]
	cmp	esi, lessoncomplete
	je	text_end_already_shown
	; ///////////////////////////
	call	speed_to_string
	mov	esi, lessoncomplete
	mov	[startline], esi
	mov	esi, lessoncompleteend
	dec	esi
	mov	[currentsymb], esi
	call	draw_window
text_end_already_shown:
	jmp	event_wait
;---------------------------------------------------------------------
button:
	mcall	17	;Идентификатор нажатой кнопки возвращён в ah
	cmp	ah, 1    ;это кнопка закрытия
	jne	no_close
	mcall	-1
no_close:
;Проверяем остальные кнопки
	cmp	ah, 0x02
	jne	no_button_2
;Нажата кнопка 2
;---------------------------------------------------------------------
; ОЧЕНЬ СТРАШНЫЙ КОД
;---------------------------------------------------------------------
	xor	eax, eax
	mov	[mistakes], 0x0
	mov	esi, [lastsymb]
	mov	al, [esi]
;Нужно проверить, что за символ был последним. Если 0xFF -
;Текст кончился, нужно начать всё сначала
	cmp	al, 0xFF
	je	start_1st_lesson
	cmp	al, 0xFE ; Мы прошли прошлый урок?
	jne	not_completed_previous
init_level_after_fe:
;Ежели да, то проверим, не попал ли случайно следующим символом 0xFF
	inc	esi
	mov	al, [esi]
	cmp	al, 0xFF
;Ежели нет - это и есть следующий урок
	jne	set_lesson_start_from_esi
start_1st_lesson:
	mov	esi, [datastart]
set_lesson_start_from_esi:
	mov	[startline], esi
	mov	[currentsymb], esi
	mov	[lastsymb], esi
	mov	[lessonstart], esi
	xor	eax,eax
	jmp	no_button_3.2
;	inc	eax
;	mov	[currentline], eax
;	call	count_lines
;	call	reset_speed_counters
;	jmp	redraw
;---------------------------------------------------------------------
not_completed_previous:
	inc	esi
	mov	al, [esi]
	cmp	al, 0xFF
	je	start_1st_lesson
	cmp	al, 0xFE
	je	init_level_after_fe
	jmp	not_completed_previous
;---------------------------------------------------------------------
; КОНЕЦ СТРАШНОГО КОДА
;---------------------------------------------------------------------
no_button_2:
	cmp	ah,0x03
	jne	no_button_3
;Перезапуск уровня
	mov	edx,[lessonstart]
	jmp	no_button_3.1
;	mov	[currentsymb],edx
;	mov	[startline],edx
;	mov	[lastsymb],edx
;	xor	eax,eax
;	mov	[mistakes],eax
;	inc	eax
;	mov	[currentline],eax
;	call	reset_speed_counters
;	call	count_lines
;	jmp	redraw
;---------------------------------------------------------------------
no_button_3:
	cmp	ah,0x04
	jne	event_wait
;Перезапуск курса упражнений
	mov	edx,[datastart]
	mov	[lessonstart],edx
.1:
	mov	[currentsymb],edx
	mov	[startline],edx
	mov	[lastsymb],edx
	xor	eax,eax
	mov	[mistakes],eax
.2:
	inc	eax
	mov	[currentline],eax
	call	reset_speed_counters
	call	count_lines
	jmp	redraw
;---------------------------------------------------------------------
;Получим текущие параметры окна
get_window_param:
	mcall	9,procinfo,-1
	ret
;---------------------------------------------------------------------
;"Сердце" программы - функция рисования окна (всё остальное - туловище Ж-)) )
draw_window:
	mcall	12,1
	mov	ax,[areawidth]   ;Это первый вызов draw_window?
	test	ax,ax
	jne	.dw_not_first_call
	call	get_screen_params	  ;Получаем параметры экрана - ширину и высоту
.dw_not_first_call:
;в ebx - X и ширина
	mov	bx,[windowx]
	shl	ebx,16
	mov	bx,780
;	mov	ebx,10*65536+780
;в ecx - Y и высота
	mov	cx, [windowy]
	shl	ecx,16
	mov	cx,580
;	mov	ecx,10*65536+580
	xor	esi,esi
;Определить и вывести окно
	mcall	0,,,0x33CCCCCC,,text
	call	get_window_param
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	.end
;Нарисуем кнопку СЛЕДУЮЩИЙ УРОК и текст на ней
	mcall	8,<295,145>,<8,18>,2,0x0099CC99
;Нарисуем кнопку ПОВТОРИТЬ УПРАЖНЕНИЕ
	mcall	,<450,145>,,3,0x00CC9999
;Нарисуем кнопку НАЧАТЬ КУРС ЗАНОВО
	mcall	,<605,145>,,4,0x00DD7777
;Выводим текст на кнопках
	mcall	4,<310,14>,0x80000000,buttontext
	mcall	,<465,14>,,retrybuttontext	
	mcall	,<626,14>,,restartbuttontext
; Нарисуем логотип
	xor	ebp,ebp
	mcall	65,logo,<32,12>,<12,12>,1,green_text
;Получим текущие параметры окна
	call	get_window_param
	add	ebx,0x2A
	mov	eax,[ebx]
	mov	[windowwidth], eax
	mov	ebx,procinfo
	add	ebx,0x2E
	mov	eax,[ebx]
	mov	[windowheight], eax
;draw_error_count:
; Нарисуем надпись "ОШИБОК"
	mcall	4,<60,15>,0x80AA0000,mistakestext
; Выведем число ошибок
	mcall	47,0x80040000,[mistakes],<105,15>,0x00AA0000
;draw_line_number:
; Нарисуем надпись "Строка       из"
	mcall	4,<140,15>,0x8000AA00,lineistext
;Выведем номер текущей строки
	mcall	47,80040000,[currentline],<195,15>,0x0000AA00
;Выведем текущую строку
	mcall	,,[linecount],<252,15>,
;Вывести рабочий текст
	call	draw_text
.end:
	mcall	12,2
	ret
;---------------------------------------------------------------------
draw_text:
;Подготовка к выводу текста
	mov	edx,40		;Начальное положение выводимого текста по вертикали
	mov	esi,[startline]
	dec	esi 		;Так надо (см. *1 ниже)!
	mov	ebx,esi		;Теперь в edx - начало выводимой строки
.start:
;Наращиваем указатель на текущий символ на единицу
	inc	ebx 	  ;(*1)
.draw_text_without_inc:
;Посмотрим,не выходит ли текст за границу окна
	mov	esi,[windowheight] ;Теперь в esi размер окна
	mov	eax,edx
	and	eax,0x0000FFFF
	add	eax,64
	cmp	eax,esi
	ja	end_draw_text     ;Мы выходим за пределы окна. Перестаём рисовать.
;установим положение по горизонтали (+18 к текущей позиции)
	mov	esi,18*65536
	add	edx,esi
; посмотрим, не вышел ли текст за границу по горизонтали
; если вышел - не рисуем этот кусок
	mov	esi,[windowwidth] ;Теперь в esi размер окна
	shl	esi,16
	mov	eax,edx
	and	eax,0xFFFF0000
	add	eax,40*65536
	cmp	eax,esi
	jna	.horizontal_check_ok     ;Если eax>est, то мы выходим за пределы окна.
.skip_line_end_step:
	mov	ah,byte [ebx]
	cmp	ah,0x00
	je	.end_line_to_next_line
	cmp	ah,0xFE
	je	end_draw_text
	cmp	ah,0xFF
	je	end_draw_text
	inc	ebx
	jmp	.skip_line_end_step
;-------------------------------------------
.end_line_to_next_line:
	and	edx,0x0000FFFF
	jmp	.draw_text_without_inc
;-------------------------------------------
.horizontal_check_ok:
;Проверим, не закончился ли урок
	mov	esi, [startline]
	cmp	esi, lessoncomplete
	jne	.in_process
;Если закончился, текст выводить нужно ЗЕЛЕНЫМ ЦВЕТОМ
	mov	edi, green_text
	jmp	.color_set_sucessful
;--------------------------------------------
.in_process:
	cmp	ebx, [currentsymb]	  ; Рисуется текущий набираемый символ?
	je	.red			  ; Так точно!
	ja	.black		  ; Нет, Рисуется то, что мы ещё не набрали
	mov	edi, gray_text	 ; Нет, рисуется то, что мы уже набрали
	jmp	.color_set_sucessful
;--------------------------------------------
.red:
	mov	edi, red_text
	jmp	.color_set_sucessful
;---------------------------------------------
.black:
	mov	edi, black_text	 ;Шрифт и цвет
.color_set_sucessful:
	xor	esi,esi
	movzx	si, byte [ebx]
	cmp	si, 0x0000
	jne	.continue_drawing
;	call	increase_y;
; Увеличим вертикальную координату вывода букв,
; если кончилась строка (встретился байт 0x00)
	and	edx,0x0000FFFF
	add	edx, 33
	jmp	.continue_text_proc
;----------------------------------------------
.continue_drawing:
	cmp	si, 0x00FF
	je	end_draw_text
	cmp	si, 0x00FE
	je	end_draw_text
;Рисуем букву с помощью БОЛЬШОГО шрифта
	push	ebx
	push	edx
	movzx	eax, byte [ebx]
	shl	eax,6
	add	eax, big_font
	mov	ebx, eax
	pop	edx
	mov	ecx, 16*65536+32
; В edx лежит координата
	mov	esi, 1
; В edi лежит указатель на палитру (цвет шрифта)
	xor	ebp,ebp
	mcall	65
	pop	ebx
;На этом отрисовка буквы завершена
.continue_text_proc:
;	inc	edi
	jmp	.start
end_draw_text:
	ret
;---------------------------------------------------------------------
;ПРОЦЕДУРА ПЕРЕРИСОВКИ ДВУХ СИМВОЛОВ (ДАБЫ МИНИМИЗИРОВАТЬ МИГАНИЕ)
redraw_2_symbols:
;Проверим, не выйдем ли мы при рисованаии за границу допустимой
;области внутри окна. Если выйдем - скроллим строку по горизонтали
	mov	esi, [windowwidth] ;Теперь в esi размер окна
	mov	eax, [currentsymb]
	sub	eax, [startline]
	mov	ecx, 18
	mul	ecx
	add	eax, 20
	add	eax, 20
	add	eax, 40
	cmp	eax, esi
	jna	r2s_horizontal_check_ok	;Если eax>est, то мы выходим за пределы окна.
;Итак, мы выходим за границы окна... Это плохо...
;Строку придётся скроллить...
	mov	eax, [currentsymb]
	dec	eax
	mov	[startline], eax
	call	draw_window
	jmp	return_from_redraw_2_symbols
;---------------------------------------------------------------------
r2s_horizontal_check_ok:
;Рисуем ПРЕДЫДУЩУЮ СЕРУЮ букву с помощью БОЛЬШОГО шрифта
	mov	ebx, [currentsymb]
	dec	ebx
	movzx	eax, byte [ebx]
	shl	eax,6
	add	eax, big_font
	mov	ebx, eax
	mov	ecx, 16*65536+32
	mov	eax, [currentsymb]
	sub	eax, [startline]
	dec	eax
	imul	eax,18
	add	eax, 18
	shl	eax,16
	add	eax, 40
	mov	edx, eax
	xor	ebp,ebp
	mcall	65,,,,1,gray_text
;Рисуем ТЕКУЩУЮ БУКВУ
	mov	ebx, [currentsymb]
	movzx	eax, byte [ebx]
	shl	eax,6
	add	eax, big_font
	mov	ebx, eax
	mov	ecx, 16*65536+32
	mov	eax, [currentsymb]
	sub	eax, [startline]
	imul	eax,18
	add	eax, 18
	shl	eax,16
	add	eax, 40
	mov	edx, eax
	mcall	65,,,,1,red_text
return_from_redraw_2_symbols:
	ret
;---------------------------------------------------------------------
redraw_mistakes:
;Закрасим прямоугольник
	mcall	13,<59,75>,<14,10>,0x00CCCCCC
; Нарисуем надпись "ОШИБОК"
	mcall	4,<60,15>,0x80AA0000,mistakestext
; Выведем число ошибок
	mcall	47,0x80040000,[mistakes],<105,15>,0x00AA0000
	ret
;---------------------------------------------------------------------
count_lines:
	xor	ecx, ecx		; В ecx - счётчик строк
	inc	ecx 		; У нас 1 строка
; Начинаем разбор...
	mov	eax, [startline]
	dec	eax
cl_next_step:
	inc	eax
	mov	bh, [eax]
	cmp	bh, 0x00
	jne	cl_not_new_line
	inc	ecx
	jmp	cl_next_step
;---------------------------------------------------------------------
cl_not_new_line:
	cmp	bh, 0xFE
	je	cl_end
	cmp	bh, 0xFF
	je	cl_end
	jmp	cl_next_step
;---------------------------------------------------------------------
cl_end:
	mov	[linecount],ecx
	ret
;---------------------------------------------------------------------
reset_speed_counters:
	xor	eax,eax
	mov	[symbtyped],eax
	mov	[typestarttime],eax
	mov	[currenttime],eax
	mov	[typingspeed],eax
	ret
;---------------------------------------------------------------------
count_speed:
	mcall	26,9
	mov	[currenttime], eax
	mov	ebx, [typestarttime]
	sub	eax, ebx   ;Теперь в eax - число прошедших долей секунд
	mov	ecx, [symbtyped]
	cmp	ecx, 0x00
	jne	cs_all_ok
	inc	ecx
cs_all_ok:
	xor	edx, edx;
	div	ecx  ;Теперь в eax - средняя пауза между символами
	mov	ecx, eax
	cmp	ecx, 0x00
	jne	cs_all_ok_2
	inc	ecx
cs_all_ok_2:
	xor	edx, edx
	mov	eax, 6000
	div	ecx
	mov	[typingspeed], eax ;Вот и всё. В [typingspeed] - средняя скорость набора
	ret
;---------------------------------------------------------------------
speed_to_string:
; Преобразует число в строку и записывает по адрему переменной speedbytes задом наперёд
	xor	edx, edx
	mov	eax, [typingspeed]
	mov	ecx, 10
	div	ecx
	add	dl, 30h
	mov	[speedbytes + 3], dl
	xor	edx, edx
	div	ecx
	add	dl, 30h
	mov	[speedbytes + 2], dl
	xor	edx, edx
	div	ecx
	add	dl, 30h
	mov	[speedbytes + 1], dl
	xor	edx, edx
	div	ecx
	cmp	dl, 0x00
	je	sts_move_space
	add	dl, 30h
	mov	[speedbytes], dl
	jmp	sts_end
;---------------------------------------------------------------------
sts_move_space:
	mov	[speedbytes], 0x20
sts_end:
	ret
;---------------------------------------------------------------------
get_screen_params:
	mcall	14
	mov	[areaheight], ax
	push	ax
	shr	eax, 16
	mov	[areawidth],ax
;В ax по прежнему находится ширина окна. Воспользуемся этим
	sub	ax, 780   ;Вычтем начальный размер окна
	shr	ax, 1
	mov	[windowx], ax
; Переходим к высоте
	xor	ax, ax
	pop	ax		; Вытолкнем из стека значение высоты
	sub	ax, 580
	shr	ax, 1
	mov	[windowy], ax
	ret
;---------------------------------------------------------------------
;draw_speed:
;Закрасим прямоугольник
;	mcall	13,<59,340>,<29,10>,0x00CCCCCC
; Нарисуем надпись "Скорость набора (знаков в минуту):"
;	mcall	4,<60,30>,0x80008800,speedtext
; Выведем число
;	mcall	47,0x80040000,[typingspeed],<305,30>,0x00008800
;	ret
;---------------------------------------------------------------------
include 'data.inc'
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
par:
params:
	rb 1024
;---------------------------------------------------------------------
procinfo:
	rb 1024
;---------------------------------------------------------------------
	rb 1024
stacktop:
;---------------------------------------------------------------------
I_END: