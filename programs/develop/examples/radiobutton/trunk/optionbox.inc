;компонент OptionBox (Основан на Checkbox)
;Огромная благодарность Maxxxx32, Diamond, Heavyiron, и другим программистам, и их программам, без
;которых я не смог бы написать этот компонент. 
;21.02.2007 модернизация и поддержка двух разных вариантов с использованием сис цветов и старой схемой
;19.02.2007 общее улучшение кода, уменьшение размера и использование системных цветов для отображения optionkbox
;16.02.2007 дата создания компонента
;<Lrz>  - Теплов Алексей  www.lrz.land.ru

macro version_op           ;для совместимости со старыми версиями optionbox все цвета задаются ручками
{
op_struc_size=26
option_group equ [edi]
op_left equ [edi+4]    ;координата начала рисования по х
op_top equ [edi+6]   ;координата начала рисования по у
op_color equ [edi+8]
op_border_color equ [edi+12] ; or [edi+4] ;цвет рамки checkbox ее можно задать самостоятельно
op_text_color equ   [edi+16];[edi+4]  ;цвет текста
op_text_ptr equ [edi+20]    ;указатель на начало текстовой строки 
op_text_length equ [edi+24] ;длина надписи (2^64 такой длины может быть текст)
}

macro   version_op1        ;продвинутая версия, использует цвета скина. 
{
op_struc_size=14
option_group equ [edi]
op_left equ [edi+4]    ;координата начала рисования по х
op_top equ [edi+6]   ;координата начала рисования по у
op_color equ [sc.work] ;
op_border_color equ [sc.work_graph] ; or [edi+4] ;цвет рамки checkbox ее можно задать самостоятельно
op_text_color equ   [sc.work_text];[edi+4]  ;цвет текста
op_text_ptr equ [edi+8]    ;указатель на начало текстовой строки 
op_text_length equ [edi+12] ;длина надписи (2^64 такой длины может быть текст)
}


macro use_option_box
{
option_box:
.draw:
pusha   ;сохраним все регистры
op_text_margin=4     ;расстояние от прямоугольника чек бокса до надписи 
op_size=12           ;размер квадрата чек бокса 
       mov eax,13 
       mov ebx,op_left 
       shl ebx,16 
       add ebx,op_size 
       mov ecx,op_top 
       shl ecx,16 
       add ecx,op_size 
       mov edx,op_border_color 
       int 0x40 ;рисуем рамку 

       mov edx,op_color 
       add ebx,1 shl 16 - 2 
       add ecx,1 shl 16 - 2 
       int 0x40 ;закрашиваем внутренности чекбокса 

        mov	eax,option_group
	mov	dword eax,[eax]
        cmp	eax,edi
	jne	@f
	call	.draw_op  ;нарисовать включенный чек бокс
;----------------------------
;расчет куда будет произведен вывод текста
;----------------------------
@@:	movzx	ebx,word op_left	;загрузить значение х для чек бокса
	add	ebx,(op_size+op_text_margin) ;добавить размер стороны и расстояние на котором начнется вывод текста
	shl	ebx,16		;сдвинем на 16 разрядов в лево (умножим на 65536)
	mov	bx,op_top	;загрузим значение по y
	add	ebx,(op_size-9+2)	;добавим значение длины стороны -9+2
	mov	ecx,op_text_color	;загрузим цвет надписи
	
	mov	edx,op_text_ptr		;укажем адрес от куда нужно выводить строку
	movzx 	esi,word op_text_length ;Загрузим длину надписи в esi
	;внесем в eax значение вывода надписи на канву
	mov	eax,4
	int	0x40          	;Вывод 
popa				;восстановить значения регистров из стека
ret				;выйдем из процедуры

.clear_op:			;очистка чек бокса
	mov	edx,op_color	;цвет внутри чек бокса
	jmp	@f		;безусловный прыжок на нижнюю метку @@

.draw_op:            ;нарисовать включенный чек бокс

	mov   edx,op_border_color	;загрузить цвет
@@:
        movzx ebx,word op_left  ;загрузить координату по х
        add   ebx,(op_size/4)   ;добавить (сторона прямоугольника/3)
        shl   ebx,16            ;сдвинем на 16 разрядов в лево (умножим на 65536)
        mov   bx,(op_size/2)    ;загрузить (сторона прямоугольника/2)
        mov   bp,bx             ;сохраним регистр bx в регистре указателя базы
        ;push        bx
        movzx ecx,word op_top ;загрузить координату по у
        mov   eax,13          ;в eax - значения функции для вывода полосы т.е. по сути прямоугольника, который отображает включенный компонент чек бокс
        add   ecx,(op_size/4) ;добавить (сторона прямоугольника/3)
        shl   ecx,16          ;сдвинем на 16 разрядов в лево (умножим на 65536)
        mov   cx,bp           ;загрузим значения регистра указателя базы в cx
        int   0x40            ;вывод
ret				;выйти из процедуры

.mouse:                 ;обработка мыши 
pusha
	mov	eax,37   	;будем что то делать если у нас что - нить нажато
	mov	ebx,2		;внести в регистр значение 2
	int	0x40		;проверка не нажал ли пользователь кнопку мышки
	test	eax,eax    ;проверка если у нас в eax=0, то установим флаг и выйдем
	jnz	@f         ;перейти на нижнюю метку @@
	popa			;если ничего не произошло, то восстановим значения регистров из стека
	ret                     ;выход
@@:
	movzx	esi,word op_text_length ;загрузить кол-во символов в текстовой строке
	;Умножение на 6 Быстрое умножение можно воспользоваться любым мз методов, но на старых Процессорах (386,486,P1)быстрее будет с инструкцией Lea
	;lea	esi,[eax*2+eax]
	;shl	eax,1
	imul	esi,6               ; или можно и так умножить на 6
	xor	ebx,ebx
        add	esi,op_text_margin   ;добавить 3 - расстояние от чек бокса до надписи
	
	mov	eax,37		;получим координаты мышки 
	inc	ebx		;добавить 1
	int	0x40                ;получить координаты курсора относительно окна 

	movzx	ebx,word op_top           ;загрузить в bx значение координаты у
	cmp	ax,bx               ;сравнить с с координатой курсора
	jl	.mouse_end          ;SF <> OF если меньше 
	add	ebx,op_size          ;добавить размер 
	cmp	ax,bx               ;сравнить
	jg	.mouse_end          ;ZF = 0 и SF = OF если больше 
	
	shr	eax,16              ;разделим на 65536 или просто сдвинем биты на 16 значений
	movzx	ebx,word op_left          ;произведем аналогичное сравнение
	cmp	ax,bx		     ;сравнить регистры
	jl	.mouse_end	     ;если меньше
	add	ebx,op_size	     ;добавить длину стороны прямоугольника
	add	ebx,esi		     ;Учесть в значении по х еще и длину надписи к чекбоксу
	cmp	ax,bx		     ;стравнить регистры
	jg	.mouse_end	     ;если больше 
	mov	eax,option_group
        mov	[eax],edi

.mouse_end:
popa				  ;восстановить регистры из стека
ret				  ;выйти
}

struc option_box1 point_gr,left,top,text,text_length
{	;структура параметров для чек бокса
.option_group: dd point_gr	;указатель на размещение edi - идентификатора optibox
.left: dw left	;+0	;положение по х
.top: dw top	;+2	;положение по у
.text: dd text	     ;+16		адрес в коде программы где расположен текстр
.text_length: dw text_length ; +20	длина текста
}
struc option_box point_gr,left,top,color,border_color,text_color,text,text_length
{	;структура параметров для чек бокса
.option_group: dd point_gr	;указатель на размещение edi - идентификатора optibox
.left: dw left	;+0	;положение по х
.top: dw top	;+2	;положение по у
.color: dd color
.border_color: dd border_color; or [edi+4] ;цвет рамки checkbox ее можно задать самостоятельно
.text_color:   dd text_color;[edi+4]  ;цвет текста
.text: dd text	     ;+16		адрес в коде программы где расположен текстр
.text_length: dw text_length ; +20	длина текста
}

op_flag_en=10b
macro draw_option_boxes start,end	;рисовать чек боксы
{
	mov	edi,start			;Указатель на начало данных чек боксов т.е. на начало данных первого чекбокса
	mov	ecx,((end-start)/op_struc_size) ;Количество чек боксов
@@:
	call	option_box.draw                  ;Отобразить чек бокс
	add	edi,op_struc_size               ;Указатель на последующие чек боксы  т.е. +28
	loop	@b	;прыгнуть если в ecx/cx значение не 0 на верхнюю @@
}
macro mouse_option_boxes start,end	;установка чек боксов, в зависимости от события
{
	mov	edi,start		; Указатель на начало данных чек боксов т.е. на начало данных первого чекбокса
	mov	ecx,((end-start)/op_struc_size)	;Количество чек боксов
@@:
	call	option_box.mouse		;проверка мышки и обработка событий
	add	edi,op_struc_size	;Указатель на последующие чек боксы
	loop	@b	;прыгнуть если в ecx/cx значение не 0 на верхнюю @@

       	mov	edi,start		; Указатель на начало данных чек боксов т.е. на начало данных первого чекбокса
	mov	ecx,((end-start)/op_struc_size)	;Количество чек боксов

.home:
	mov	eax,[edi]	;op_optibox_gr
        mov	dword eax,[eax]

        pusha

	cmp	eax,edi
	jne	@f
	
	call	option_box.draw_op
	jmp	.nxtm
@@:     call	option_box.clear_op

.nxtm:  popa
	add	edi,op_struc_size               ;Указатель на последующие чек боксы  т.е. +28
        loop 	.home

}