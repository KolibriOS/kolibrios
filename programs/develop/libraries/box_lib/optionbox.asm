; элемент Optionbox для библиотеки box_lib.obj
; на код применена GPL2 лицензия
; файл создан 13.02.2009 <Lrz>
; последняя модификация 12.09.2017 IgorA

align 16
option_box_draw:
	pusha

	mov   eax,dword[esp+36]
@@:
	mov   edi,dword[eax]
	test  edi,edi
	je    option_check
	call  option_box_draw_box
	add   eax,4
	jmp   @b 

option_check:
	mov   eax,dword[esp+36]
@@:
	mov   edi,dword[eax]
	test  edi,edi
	je    @f

	mov   ebx,dword[edi]
	mov   ebx,dword[ebx]
	cmp   edi,ebx
	jne   .clear_op

	pusha
	call  option_box_draw_op
	popa
	add   eax,4
	jmp   @b

.clear_op:
	pusha
	call  option_box_clear_op
	popa
	add   eax,4
	jmp   @b

@@:
	popa
	ret 4 


align 16
option_box_mouse:
	pusha
	mcall SF_MOUSE_GET,SSF_BUTTON
	test  eax,eax
	jnz @f
	popa
	ret 4

@@:
	mov   eax,dword[esp+36]
@@:
	mov   edi,dword[eax]
	test  edi,edi
	je    option_check
	call  option_box_mouse_op
	add   eax,4
	jmp   @b 

align 16
option_box_draw_box:
	pusha   ;сохраним все регистры

	movzx ebx,word op_left 
	shl   ebx,16 
	add   ebx,dword op_size 
	movzx ecx,word op_top 
	shl   ecx,16 
	add   ecx,dword op_size 
	mcall SF_DRAW_RECT,,,op_border_color ;рисуем рамку 

	add   ebx,1 shl 16 - 2 
	add   ecx,1 shl 16 - 2 
	mcall ,,,op_color ;закрашиваем внутренности чекбокса 

;	mov   eax,dword option_group
;	mov   eax,[eax]
;	cmp   eax,edi
;	jne   @f
;	call  option_box_draw_op  ;нарисовать включенный чек бокс


;----------------------------
;расчет куда будет произведен вывод текста
;----------------------------
@@:
	movzx ebx,word op_left ;загрузить значение х для чек бокса
	add   ebx,dword op_size
	add   ebx,dword op_text_margin ;добавить размер стороны и расстояние на котором начнется вывод текста
	shl   ebx,16       ;сдвинем на 16 разрядов в лево (умножим на 65536)
	mov   bx,word op_top ;загрузим значение по y
	mov   eax,op_size
	shr   eax,1
	add   eax,4
	add   ebx,eax
	sub   ebx,7        ;добавим значение длины стороны -9+2
	mov   esi,dword op_text_length
	mcall SF_DRAW_TEXT,,op_text_color,op_text_ptr ;Вывод надписи на канву
	popa               ;восстановить значения регистров из стека
	ret                ;выйдем из процедуры

option_box_clear_op:   ;очистка чек бокса
	mov   edx,dword op_color ;цвет внутри чек бокса
	jmp   @f         ;безусловный прыжок на нижнюю метку @@


option_box_draw_op:    ;нарисовать включенный чек бокс
	mov   edx,dword op_border_color ;загрузить цвет
@@:
	movzx ebx,word op_left ;загрузить координату по х
	mov   eax,dword op_size
	mov   bp,ax
	shr   eax,2
	push  ax

	push  ax
	add   ebx,eax           
	shl   ebx,16       ;сдвинем на 16 разрядов в лево (умножим на 65536)
	pop   ax
	lea   eax,[eax*2]
	sub   bp,ax        ;сохраним регистр bx в регистре указателя базы
	mov   bx,bp

	movzx ecx,word op_top ;загрузить координату по у
	pop   ax
	add   cx,ax         
	shl   ecx,16       ;сдвинем на 16 разрядов в лево (умножим на 65536)
	mov   cx,bp        ;загрузим значения регистра указателя базы в cx
	mcall SF_DRAW_RECT ;вывод полосы т.е. по сути прямоугольника, который отображает включенный компонент чек бокс
	ret                ;выйти из процедуры


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Обработчик mouse
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 16
option_box_mouse_op:                 ;обработка мыши 
pusha
	mov   esi,dword op_text_length ;загрузить кол-во символов в текстовой строке
	imul  esi,6                ;или можно и так умножить на 6
	add   esi,dword op_text_margin ;добавить 3 - расстояние от чек бокса до надписи

	mcall SF_MOUSE_GET,SSF_WINDOW_POSITION ;получить координаты курсора относительно окна 

	movzx ebx,word op_top      ;загрузить в bx значение координаты у
	cmp   ax,bx                ;сравнить с с координатой курсора
	jl    option_box_mouse_end ;SF <> OF если меньше 
	add   ebx,dword op_size    ;добавить размер 
	cmp   ax,bx                ;сравнить
	jg    option_box_mouse_end ;ZF = 0 и SF = OF если больше 

	shr   eax,16               ;разделим на 65536 или просто сдвинем биты на 16 значений
	movzx ebx,word op_left     ;произведем аналогичное сравнение
	cmp   ax,bx                ;сравнить регистры
	jl    option_box_mouse_end ;если меньше
	add   ebx,dword op_size    ;добавить длину стороны прямоугольника
	add   ebx,esi              ;Учесть в значении по х еще и длину надписи к чекбоксу
	cmp   ax,bx                ;стравнить регистры
	jg    option_box_mouse_end ;если больше 
	mov   eax,dword option_group
	mov   [eax],edi

option_box_mouse_end:
	popa                         ;восстановить регистры из стека
	ret                          ;выйти
