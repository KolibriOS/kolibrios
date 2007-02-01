; <Lrz> 01.02.2007 доработка edit_box, исправил баги.
; <Lrz> 26.01.2007 нормальная работа компанента исправил фукцию .check_offset
; <Lrz> 24.01.2007 пофиксел баги вызваные не правильным кодом )).
; <Lrz> 22.01.2007 избавился от глюков при работе с выделением и удаление, корректная работа клавишей del & backspase
; <Lrz> 20.01.2007 реализовал выделение текста по shift
; <Lrz> 12.12.2006 реализовал поддержку кнопки insert и сегодя компонент почти работает!!!!
; <Lrz> 07.12.2006 продолжается работа над переработкой компонента,изменениям подверглись многие функции, переработан вывод текста, а така же алгоритм внесения символов.
; <Lrz> 03.09.2006 по возможности отказался от 16 битной арифметики, добавил новые баги
; <Lrz> 09.08.2006 произведена оптимизация работы конпок DEL и Backspace, уменьшен размер выполняемого кода.
; <Lrz> 21.07.2006 добавлена функция кнопки Del, теперь можно удалять символы при помощи данной клавиши
; Автор: Евтихов Максим (Maxxxx32) email: maxxxxm@mail.ru
; Дата последних изменений: 13.06.06 10:40
;  Напишите в исходном коде своей программы use_edit_box,
; это вставит необходимые процедуры в код вашей программы.
;  Процедуры:
;   edit_box.draw - полная перерисовка;
;   edit_box.key - обработка клавиатуры;
;   edit_box.mouse - обработка мыши;
;   edit_box.focus - установка фокуса;
;   edit_box.blur - его размывание;
;   edit_box.get_n - получить количество прорисовываемых символов.
;  При вызове любых из этих процедур в регистре edi следует
; оставить указатель на структуру.
;  Пример структуры:
; some_edit edit_box 100,10,30,0x00ffffff,0,0x00aaaaaa,0,255,some_edit_text
; длина, высота, верх, цвет фона, цвет рамки, если выбран,
; цвет рамки, если не выбран, максимальное количество символов,
; указатель на буфер, где будут хранится символы. Буфер должен
; оканчиваться нулем, например если максимальное количество 255:
;  some_edit_text:
;       rb      256 ;255+1

;  Пример вызова процедуры:
;       mov     edi,some_edit
;       call    edit_box.draw

;  При вызове процедуры edit_box.key код клавиши должен
; находится в ah, то есть перед вызовом этой процедуры надо вызвать
; 2-ую сисемную функцию, например:
;       mov     eax,2
;       int     0x40
;       mov     edi,some_edit1
;       call    edit_box.key
;       mov     edi,some_edit2
;       call    edit_box.key
;  Перед вызовом обработчика мыши следует проверить, является ли окно
; активным.
;  Если щелчок мыши был пройзведен за пределами edit box
; фокус теряется.
macro use_edit_box
{
edit_box:
ed_width	equ [edi]		;ширина компонента
ed_left 	equ [edi+4]             ;положение по оси х
ed_top		equ [edi+8]             ;положение по оси у
ed_color	equ [edi+12]		;цвет фона компонента
ed_focus_border_color	equ [edi+16]	;цвет рамки компонента
ed_blur_border_color	equ [edi+20]    ;цвет не активного компонента
ed_text_color	equ [edi+24]            ;цвет текста
ed_max		equ [edi+28]                    ;кол-во символов которые можно максимально ввести
ed_text 	equ [edi+32]                    ;указатель на буфер
ed_flags	equ [edi+36]            ;флаги
ed_size equ [edi+38]                    ;кол-во символов
ed_pos	equ [edi+42]                    ;позиция курсора
ed_offset	equ [edi+46]            ;смещение
cl_curs_x	equ [edi+50]		;предыдущее координата курсора по х
cl_curs_y	equ [edi+54]		;предыдущее координата курсора по у
ed_shift_pos	equ [edi+58]
ed_height=14 ; высота
shift_color=0x6a9480
ed_insert=10000b
ed_insert_cl=1111111111101111b
;==========================================================
;=== процедура прорисовки =================================
;==========================================================
.draw:
pusha
;--- рисуем рамку ---
	call	.draw_border		; Функция стабильна
.draw_bg_cursor_text:
;--- изменяем смещение, если надо ---
	call	.check_offset		;вычисление позиции курсора стабильна
;--- рисуем внутреннюю область ---
	call	.draw_bg                ;нарисовать прямоугольник рабочей области
.draw_cursor_text:
;--- рисуем курсор ---
	;--- может его не надо рисовать ----
	test	word ed_flags,ed_focus
	je	@f
	call	.draw_cursor
@@:
	call	.draw_text
popa
ret

;.str:
;	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
;	 ;преревод числа в ASCII строку взодные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
;	cmp eax,ecx  ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
;	jb @f
;	xor edx,edx  ;очистить edx
;	div ecx      ;разделить - остаток в edx
;	push edx     ;положить в стек
;	;dec edi	     ;смещение необходимое для записи с конца строки
;	call .str;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
;	pop eax
;	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
;	;sbb al,$69  ;- честно данная инструкция меня заставляет задуматься т.е. я не знаю как это работает
;	;das	    ;после данной команды как бы происходит уменьшение al на 66h  (в книге написано другое)
;	or al,0x30  ;данная команда короче  чем две выше 
;	stosb	    ;записать элемент из регистра al в ячеку памяти es:edi
;
;	ret	    ;вернуться чень интересный ход т.к. пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться

;----------------------------------------------------------
;--- процедура прорисовки текста --------------------------
;----------------------------------------------------------
.draw_text:
;--- вычисляем, сколько помещается символов ---
;--- чтобы мусор не рисовать ---
	call 	.get_n
	mov	esi,ed_size
        mov	ebx,ed_offset
	sub	esi,ebx
	cmp	eax,esi
	jae	@F
	mov	esi,eax		;чтобы не выходить за пределы экрана
;--- рисуем текст ---
@@: 	mov	eax,4
	mov	ebx,ed_left
	mov	edx,ed_offset
	add	ebx,2
	shl	ebx,16
	mov	bx,ed_top
	add	ebx,4
	mov	ecx,ed_text_color
	add	edx,ed_text
	int	0x40
ret
;----------------------------------------------------------
;--- процедура прорисовки фона ----------------------------
;входные данные
;eax
;edx - color
;----------------------------------------------------------
.draw_bg_eax:
	mov	ecx,ed_top
	add	ecx,1
	shl	ecx,16
	mov	cx,13
	mov	eax,13
	int	0x40
ret

	jmp	@f
.draw_bg:
	mov	ebx,ed_left
	add	ebx,1
	shl	ebx,16
	mov	bx,ed_width
	sub	ebx,1

@@:
	mov	ecx,ed_top
	mov	eax,13
	add	ecx,1
	shl	ecx,16
	mov	edx,ed_color
	mov	cx,ed_height
	sub	ecx,1
        int	0x40
ret

;----------------------------------------------------------
;--- процедура получения количества символов в текущей щирине компонента --------------
;----------------------------------------------------------
.get_n:
	mov	eax,ed_width	;получем ширину компонента
	xor	edx,edx   ;результат распологается в паре edx:eax в eax - остаток
	sub	eax,4		;вычтим 4
	mov	ebx,6		;загрузми делитель
	div	ebx		;размделим на 6
ret

;----------------------------------------------------------
;--- процедура рисования курсора --------------------------
;----------------------------------------------------------
.clear_cursor:
	mov	edx,ed_color
        test    word ed_flags,ed_shift
	je	@f
       	mov	edx,shift_color
@@:    	
	mov	ebx,cl_curs_x
	mov	ecx,cl_curs_y
	jmp	.draw_curs
.draw_cursor:

	mov	edx,ed_text_color
@@:
	mov	ebx,ed_pos
	mov	ecx,ed_offset
	sub	ebx,ecx

	lea     ebx,[ebx*2+ebx]
	shl     ebx,1
	;imul	ebx,6
	add	bx,ed_left
	mov	ecx,ed_top
	inc	ebx
	add	ecx,2
	mov	ebp,ebx
	shl	ebx,16
	mov	bx,bp
	mov	ebp,ecx
	
	shl	ecx,16
	mov	cx,bp
	add	cx,ed_height-4
	
	mov	cl_curs_x,ebx
	mov	cl_curs_y,ecx
.draw_curs:	         
	mov	eax,38
	int	0x40
ret

;----------------------------------------------------------
;--- процедура рисования рамки ----------------------------
;----------------------------------------------------------
.draw_border:
;--- цвет рамки ---
	test	dword ed_flags,ed_focus
	mov	edx,ed_focus_border_color
	jne     @f
	mov	edx,ed_blur_border_color
@@:
;--- сверху ---
	mov	eax,38
	mov	ebx,ed_left
	mov 	ecx,ebx
	shl	ebx,16
	mov	bx,cx
	add	bx,ed_width
	mov	ecx, ed_top
	mov	esi,ecx
	shl	ecx,16
	mov	cx,si
	int	0x40

;--- снизу ---
	mov	esi,ecx
	add	ecx,ed_height
	mov	ebp,ecx
	shl	ecx,16
	mov	cx,bp
	int	0x40
 
;--- слева ---
	mov	cx,si
	mov	ebp,ebx
	sub	bx,ed_width
	int	0x40

;--- справа ---
	mov	ebx,ebp
	shl	ebx,16
	mov	bx,bp
	int	0x40
ret
;----------------------------------------------------------
;--- проверка, зашел ли курсор за границы и, если надо, ---
;--- изменяем смещение ------------------------------------
;--- eax = -1, если не изменилось или eax = 0, если
; изменилось
;----------------------------------------------------------
.check_offset:
pushad
	mov	ecx,ed_pos
	mov	ebx,ed_offset
	cmp	ebx,ecx
	ja	.sub_8

	push	ebx
	call	.get_n			;получим кол-во символов в паре регистров edx:eax 
	pop	ebx
	mov	edx,ebx
	add	edx,eax		;ed_offset+width editbox
	cmp	edx,ecx
	ja	@f

	mov	edx,ed_size
	cmp	edx,ecx
	je      .add_end

	sub	edx,ecx
	cmp	edx,8
	jbe	.add_8
	add	ebx,8
	jmp	.chk_d

.sub_8: cmp	ecx,0
	je	.sub_min
	cmp	ebx,8
        jbe	.sub_min
	sub	ebx,8	;ebx=ed_offset
	jmp	.chk_d
.sub_min:
        xor	ebx,ebx
	jmp	.chk_d

.add_end:sub	edx,eax
	mov	ebx,edx
	jmp	.chk_d
.add_8:	add	ebx,edx

;	;-----------  отладка
;	pushad
;	mov	[data_of_code],0
;	mov eax,ebx;ed_offset
;	mov edi,data_of_code
;	call .str
;	;рисование фона
;	mov eax,13
;	mov ebx,178*65536+36
;	mov ecx,28*65536+10
;	xor edx,edx
;	int 0x40
;	;вывод значения на экран
;	mov eax,4
;	mov ebx,180*65536+30
;	mov ecx,0x10DDBBCC
;	mov edx,data_of_code
;	mov esi,4
;	int 0x40
;	popad
;	;----------- отладка

.chk_d:	mov	ed_offset,ebx

	call    .draw_bg
	mov	dword [esp+28],-1
popad
	ret

@@:
	mov	dword [esp+28],0
popad
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	mov	ebx,eax
;	mov	eax,ed_pos
;	xor 	edx,edx
;	idiv	ebx
;	xor 	edx,edx
;	imul    eax,ebx
;	mov	ed_offset,eax
;	pop	ebx
;	cmp	ax,bx
;	je	@f
;;;;;;;; данные для закрашивания всей полосы прямоугольника
;	mov	ebx,ed_left
;	inc	ebx
;	shl	ebx,16
;	mov	bx,ed_width
;	dec 	ebx
;	mov	edx,ed_color
;	call    .draw_bg_eax
;	mov	dword [esp+28],0
;@@:	popad
;	ret
;==========================================================
;=== обработка клавиатуры =================================
;==========================================================
.key:
pusha
	test	word ed_flags,ed_focus ; если не в фокусе, выходим
	je     .no_figure ;@f

	xor	ecx,ecx
	push	eax	
	inc	ecx
	mov	eax,66
	mov	ebx,3
	int	0x40
	test	al,0x03
	je	@f
        or      word ed_flags,ed_shift

@@:     pop	eax
;----------------------------------------------------------
;--- проверяем, что нажато --------------------------------
;----------------------------------------------------------
	cmp	ah,8
	jz	.backspace
	cmp	ah,0xb6
	jz	.delete
	cmp	ah,176
	jz	.left
	cmp	ah,179
	jz	.right
	cmp	ah,180
	jz	.home
	cmp	ah,181
	jz	.end
	cmp	ah,185	;insert
	jz	.insert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Заглушка на обработку клавиш вверх и вниз
;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
	cmp	ah,177
        jz	.no_figure
	cmp	ah,178
	jz	.no_figure
	cmp	ah,27   ;ESC - клавиша ))
	jz	.no_figure
;--- нажата другая клавиша ---
	test	word ed_flags,ed_figure_only  ; только цифры ?
	jz	@f
	cmp	ah,'0'
	jb	.no_figure
	cmp	ah,'9'
	ja	.no_figure


;проверка на shift
@@:	test	word ed_flags,ed_shift_on
	je	@f
	;Входные данные edx=ed_size;ecx=ed_pos
	push	eax
	mov	edx,ed_size
	mov	ecx,ed_pos
        call	.del_char
;;;;
        mov	eax,ed_shift_pos
        mov	ebx,ed_size
	sub	ebx,eax
	mov	ed_size,ebx
	pop	eax

        ; проверяем, находится ли курсор в конце
@@:	mov	ecx,ed_size
	mov	edx, ed_max
	cmp	ecx,edx
	jae	.no_figure
	mov	ebx, ed_pos
	cmp	ebx,edx
	jl	@f ; если меньше или равно
.no_figure:
	popa
	ret
.insert: test	word ed_flags,ed_insert	;not 	word ed_insert
         je     .insert_1
         and	word ed_flags,ed_insert_cl
	jmp	.no_figure
.insert_1:
	or	word ed_flags,ed_insert
        jmp	.no_figure
.ins_v:
	dec	dword [ebp+38];ed_size    ;processing is insert
	sub	esi,ecx
	add	esi,ebx
        mov	edi,esi
	jmp	.In_k
@@:     ; сдвигаем символы после курсора вправо
	mov	ecx,ed_size
	push	edi eax
	mov	ebp,edi
	mov	esi,ed_text     ; Указатель на буфер
				;Будем работать со строкой
	add	esi,ecx		;add ed_size добавим max size
	mov	edi,esi

       	cmp 	ecx,ebx         ;Если у нас позиция курсора = текущему размеру напечатанных символов т.е. курсор стоит в конце
	je	.In_k

	test	word [ebp+36],ed_insert ;IF insert is enable  т.к. edi изменен адресуем через ebp
	jne	.ins_v

	sub	ecx,ebx         ;Найдем кол-во символов для передвижения.
	inc	edi		;Сместим наши символы в право
	std
	inc 	ecx
   @@:	
	;--------
	lodsb
	stosb
	;--------
	loop	@b
.In_k:	cld
	pop eax
	mov al,ah
	stosb
	pop edi
; вставляем код клавиши туда, где курсор
	; увеличиваем значение размера и позиции
	inc	 dword ed_size
	inc	 dword ed_pos
	call	.draw_all2
	jmp	.shift;.draw_cursor_text
.delete:	
	mov	edx,ed_size
	mov	ecx,ed_pos
	cmp	edx,ecx
	jg	.bac_del
	test	word ed_flags,ed_shift_on
	jne	.del_bac
	popa
	ret
.bac_del:	
	call 	.del_char
	jmp 	.draw_all
        
;--- нажата клавиша backspace ---
.backspace:
	; проверяем, курсор у левого края ?
	mov	edx,ed_size
	mov	ecx,ed_pos
	test	ecx,ecx
	jnz	.del_bac
        test	word ed_flags,ed_shift_on
        jne	.bac_del
	popa
	ret
.del_bac:
	cmp	edx,ecx	;if ed_pos=ed_size
	je	@f
	dec	ecx
	call 	.del_char
@@:     test	word ed_flags,ed_shift_on
        jne	.bac_del
	dec	 dword ed_pos

.draw_all:
	push	.shift;.draw_cursor_text;eax

        test	word ed_flags,ed_shift_on
        je	@f
        mov	eax,ed_shift_pos
        mov	ebx,ed_size
	sub	ebx,eax
	mov	ed_size,ebx
	jmp	.draw_all2

@@:	dec	 dword ed_size

.draw_all2:
	and    word ed_flags,ed_shift_cl
	call	.clear_cursor
	call	.check_offset
;;;;;;;;;;;;;;;
	test	eax,eax
	jne      @f
	call	.draw_bg
	ret
;;;;;;;;;;;;;;;
@@:	call	.get_n
	push	eax
	mov	ebx,ed_offset
	add	eax,ebx	;eax = w_off= ed_offset+width
	mov	ebx,ed_size
	
	cmp	eax,ebx
	jb	@f
	mov	eax,ed_pos
	sub	ebx,eax
	mov	ecx,ed_offset
	sub	eax,ecx
	jmp	.nxt
@@:	mov	ebx,ed_pos
	push	ebx
	sub	eax,ebx
	mov	ebx,eax	;It is don't optimal
	
	pop	eax	;ed_pos
       	mov	ecx,ed_offset
	sub	eax,ecx

.nxt:
	mov	ebp,eax  ;проверка на выход закрашиваемой области за пределы длины
	add	ebp,ebx
	pop	edx
	cmp	ebp,edx
	je	@f
	inc	ebx

@@:     mov	edx,ebx
	lea	ebx,[eax*2+eax]
	shl	ebx,1
	add	ebx,ed_left
	inc	ebx
	shl	ebx,16
	lea	ecx,[edx*2+edx]
	shl	ecx,1	
	mov	bx,cx
	mov	edx,ed_color
	call	.draw_bg_eax
	ret
;	jmp	.shift
;--- нажата клавиша left ---
.left:  call	.sh_enable
	mov	ebx,ed_pos
	test	ebx,ebx
	jz	.nd_k
	call	.clear_cursor
	dec	 dword ed_pos
	jmp	.nd_k
;--- нажата клавиша right ---
.right: call	.sh_enable
	mov	ebx,ed_pos
	cmp	ebx,ed_size
	je	.nd_k
	call	.clear_cursor
	inc	dword  ed_pos
	jmp	.nd_k
.home:  call	.sh_enable
	mov	ebx,ed_pos
	test	ebx,ebx
	jz	.nd_k
	call	.clear_cursor
	xor	eax,eax
	mov	ed_pos,eax
	jmp	.nd_k

.end:   call	.sh_enable
	mov	ebx,ed_pos
	cmp	ebx,dword ed_size
	je	.nd_k
	call	.clear_cursor
	mov	eax,ed_size
	mov	ed_pos,eax
.nd_k:	call	.check_offset
	;test	eax,eax
	;jz	.draw_bg_cursor_text
	call	.draw_cursor

.shift: ;;;;;;;SHIFT
		
    	call	.draw_bg	;не оптимально
	test      word ed_flags,ed_shift
        je	.f_exit
;;;;;;SHIFT end
	mov 	eax,dword	ed_pos
	mov	ebx,dword	ed_shift_pos
	cmp	eax,ebx
	jae	.sh_n
	push	eax  ;меньшее в eax
	push	ebx  ;большее
	jmp	.sh_n1
	             ;если иначе
.sh_n: 	push	ebx  
	push	eax
.sh_n1: 
;        jmp	.draw_all2
;;;;;;;;;;;;;;;;;;;;;;;;;
	;call	.clear_cursor
	call	.check_offset
	call	.get_n
	mov	edx,eax	;size of ed_box
;	push	eax
	mov	ecx,ed_offset
	add	eax,ecx	;eax = w_off= ed_offset+width
	mov	ebp,eax ;save
	pop	ebx     ;большее
	pop	eax     ;меньшее

	
	cmp	eax,ecx         ;сравнение с меньшего с offset.
	jae	.f_f            ;если больше
	xor	eax,eax
	cmp	ebp,ebx		;cравним размер w_off с большим
	jb	@f
	sub	ebx,ecx
	jmp     .nxt_f
@@:	mov	ebx,ebp
	sub	ebx,ecx	
	jmp     .nxt_f

.f_f:	
	sub	eax,ecx
	cmp	ebp,ebx		;cравним размер w_off с большим
	jle	@f
	sub	ebx,ecx
	sub	ebx,eax
	jmp     .nxt_f
@@:
	mov	ebx,ebp	
	sub	ebx,ecx
	sub	ebx,eax
.nxt_f:
	mov	edx,ebx
	lea	ebx,[eax*2+eax]
	shl	ebx,1
	add	ebx,ed_left
	inc	ebx
	shl	ebx,16
	lea	ecx,[edx*2+edx]
	shl	ecx,1	
	mov	bx,cx
	mov	edx,shift_color
	call	.draw_bg_eax
	and    word ed_flags,1111111111111011b
        jmp	.draw_cursor_text
;;;;;;;;;;;;;;;;;;;;;

.f_exit:	and    word ed_flags,ed_shift_cl
        jmp	.draw_cursor_text

;проверка состояния shift был ли он нажат раньше?
.sh_enable:
	test    word ed_flags,ed_shift
	je	@f
	test	word ed_flags,ed_shift_on
	jne	.sh_ext_en
       	push	dword ed_pos
	pop	dword ed_shift_pos
	or	word ed_flags,ed_shift_on
	ret

@@:     and	word ed_flags,ed_shift_cl
.sh_ext_en:	ret

        ;- удаление символа
	;Входные данные edx=ed_size;ecx=ed_pos
.del_char:
	mov     esi,ed_text
        test	word ed_flags,ed_shift_on
        je	@f
	mov	eax,dword ed_shift_pos
	mov	ebx,esi
        cmp	eax,ecx
	jae	.dh_n

	mov	ed_pos,eax	;что бы не было убегания курсора
	mov	ebp,ecx
	sub	ebp,eax
	add	ebx,eax  ;eax меньше 
	sub	edx,ecx
	add	esi,ecx


	mov	ed_shift_pos,ebp
        jmp	.del_ch_sh
	             ;если иначе
.dh_n: 	
	mov	ebp,eax
	sub	ebp,ecx
	add	ebx,ecx
	sub	edx,eax
	add	esi,eax
	mov	ed_shift_pos,ebp
	jmp	.del_ch_sh

@@:	add	esi,ecx ;указатель + смещение к реальному буфферу
	mov	ebx,esi
	inc 	esi
	cld

	sub	edx,ecx
.del_ch_sh:

	push 	edi
	mov 	edi,ebx
@@:
	lodsb
	stosb
	dec edx
	jns @b
	
	pop edi
	ret
;==========================================================
;=== обработка мыши =======================================
;==========================================================
.mouse:
pusha
;----------------------------------------------------------
;--- получаем состояние кнопок мыши -----------------------
;----------------------------------------------------------
	mov	eax,37
	mov	ebx,2
	int	0x40
;----------------------------------------------------------
;--- проверяем состояние ----------------------------------
;----------------------------------------------------------
	test	eax,1
	jnz	.mouse_left_button
@@:     popa
	ret

.mouse_left_button:
;----------------------------------------------------------
;--- получаем координаты мыши -----------------------------
;----------------------------------------------------------
	mov	eax,37
	xor	ebx,ebx
	inc	ebx
	int	0x40
;----------------------------------------------------------
;--- проверяем, попадает ли курсор в edit box -------------
;----------------------------------------------------------
	mov	ebx,ed_top
	cmp	ax,bx
	jl	._blur;.mouse_end_no_focus

	add	bx,ed_height
	cmp	ax,bx
	jg	._blur;.mouse_end_no_focus

	shr	eax,16

	mov	bx,ed_left
	cmp	ax,bx
	jl	._blur;.mouse_end_no_focus

	add	bx,ed_width
	cmp	ax,bx
	jg	._blur;.mouse_end_no_focus
;--- изменяем позицию курсора ---
	push	eax
	call	.clear_cursor
	pop	eax
	xor	dx,dx
	sub	ax,ed_left
	add	ax,2
	mov	bx,6
	div	bx
	add	ax,ed_offset
	cmp	ax,ed_size
	jna	 @f
	mov	ax,ed_size
@@:
	mov	ed_pos,ax
	call	.check_offset
	call	.draw_cursor
;----------------------------------------------------------
;--- процедура установки фокуса ---------------------------
;----------------------------------------------------------
	bts	word ed_flags,1
	jc	.mouse_pressed
	call	.draw_cursor
	call	.draw_border
.mouse_pressed:
popa
ret
;.mouse_end_no_focus:
;        call    .blur
;popa
;ret
;.blur:
;pusha
._blur:
	test	word ed_flags,ed_always_focus
	jne	@f
	btr	word ed_flags,1 ; если не в фокусе, выходим
	jnc     @f
	call	.clear_cursor
	call	.draw_border
@@:
popa
ret

;----------------------------------------------------------
;--- процедура размывания фокуса --------------------------
;----------------------------------------------------------
;.blur:
;pusha
;._blur:
	;btr     ed_flags,1
	;jnc     @f
	;call    .draw_border
	;call    .clear_cursor
;@@:
;popa
;ret
}
ed_figure_only=1000000000000000b
ed_always_focus=100000000000000b
ed_focus=10b
ed_shift_on=1000b
ed_shift=100b
ed_shift_cl=1111111111110011b
macro draw_edit_boxes start,end
{
	mov	edi,start
	mov	ecx,((end-start)/ed_struc_size)
@@:
	call	edit_box.draw
	add	edi,ed_struc_size
	loop	@b
}

macro mouse_edit_boxes start,end
{
	mov	edi,start
	mov	ecx,((end-start)/ed_struc_size)
@@:
	call	edit_box.mouse
	add	edi,ed_struc_size
	loop	@b
}

macro key_edit_boxes start,end
{
	mov	edi,start
	mov	ecx,((end-start)/ed_struc_size)
@@:
	call	edit_box.key
	add	edi,ed_struc_size
	loop	@b
}
ed_struc_size=62
struc  edit_box width,left,top,color,focus_border_color,\
       blur_border_color,text_color,max,text,flags,size
{
.width dd width
.left dd left
.top dd top
.color dd color
.focus_border_color dd focus_border_color
.blur_border_color dd blur_border_color
.text_color dd text_color
.max dd max
.text dd text
.flags dw flags+0
.size dd size+0
.pos dd 0
.offset dd 0
.cl_curs_x dd 0
.cl_curs_y dd 0
.shift	   dd 0
}

macro edit_boxes_set_sys_color start,end,color_table
{
	mov	edi,start
	mov	ecx,((end-start)/ed_struc_size)
	mov	esi,color_table
@@:
	mov	eax,[esi+36]
	mov	ebx,[esi+20]
	mov	ed_focus_border_color,eax
	shr	bh,1
	shr	bl,1
	shr	ah,1
	shr	al,1
	add	ah,bh
	add	al,bl
	ror	eax,16
	ror	ebx,16
	shr	bl,1
	shr	al,1
	add	al,bl
	ror	eax,16
	mov	ed_blur_border_color,eax
	add	edi,ed_struc_size
	loop	@b
}

macro draw_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.draw
}

macro mouse_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.mouse
}

macro key_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.key
}