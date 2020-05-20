;
;   DESKTOP CONTEXT MENU
;   written by Ivan Poddubny
;
;   Автор - Иван Поддубный
;   e-mail: ivan-yar@bk.ru
;
;   Compile with flat assembler
;
;------------------------------------------------------------------------------
; version:	1.1
; last update:  27/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      The program uses only 3404 bytes memory is now.
;               Optimisations and code refactoring.
;------------------------------------------------------------------------------
include 'lang.inc'
include '..\..\..\..\macros.inc'
;------------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stack_area	; esp
	dd 0		; boot parameters
	dd 0		; path
;------------------------------------------------------------------------------
START:
; получить системные цвета
	mcall	48,3,sc,sizeof.system_colors
; установим маску событий - нас интересует только мышь
	mcall	40,100000b
;------------------------------------------------------------------------------
align 4
still:		     ; главный цикл основного процесса
	mcall	10	; ждём события

	mcall	37,2	; какие нажаты кпопки?
	cmp	eax,ebx	; если не правая, возврат
	jne	still
;--------------------------------------
; это для отладки - если мышь в точке (0;0), закроемся
;	xor	ebx,ebx
;	mcall	37
;	test	eax,eax	; курсор в точке (0;0), т.е. eax = 0
;	je	exit
;--------------------------------------
; координаты курсора
	xor	ebx,ebx
	mcall	37

	mov	ebx,eax		; eax = cursor_x
	shr	eax,16		; ebx = cursor_y
	and	ebx,0xffff
	mov	[curx1],eax	; curx1 = cursor_x
	mov	[cury1],ebx	; cury1 = cursor_y
; кому принадлежит точка?
	mcall	34,[curx1],[cury1]
	cmp	al,1 ; 1 - ядро
	jne	still
;--------------------------------------
align 4
@@:		; подождём, пока пользователь не отпустил правую кнопку мыши
	mcall	37,2    ;   нажаты ли кнопки мыши?
	test	eax,ebx ; если отпустил, (eax != 2)
	jz	@f	;   идём в начало главного цикла

	mcall	68,1	; иначе переключимся на следующий поток системы и когда
	jmp	@b	; выполнение вернется этому потоку, проверим мышь опять
;--------------------------------------
align 4	
@@:
; если уже было открыто меню, нужно подождать, пока оно закроется:
	cmp	[menu_opened],0
	je	@f

	mcall	68,1	; переключимся на следующий поток системы 
			; более эффективный способ задержки чем mcall 5
	jmp	@b
;--------------------------------------
align 4
@@:
; а теперь можно смело запускать процесс (поток) меню
	mcall	51,1,start_wnd,stack_wnd
	jmp	still
;------------------------------------------------------------------------------	
align 4
exit_menu:	      ; если выходим из меню, надо записать в [menu_opened] 0
	mov	[menu_opened],0
;--------------------------------------
align 4
exit:		      ; сюда мы идём, когда выходим из основного процесса
	or	eax,-1	      ; eax = -1
	mcall
;------------------------------------------------------------------------------	
; здесь стартует процесс меню
;------------------------------------------------------------------------------	
align 4
start_wnd:
	mov	[menu_opened],1
; установим маску желаемых событий: меню + кнопки + перерисовка
	mcall	40,100101b
;------------------------------------------------------------------------------	
align 4
red:
	call	draw_window
;------------------------------------------------------------------------------	
align 4
still2: 	    ; главный цикл процесса меню
	mcall	10	; ждём события

	cmp	eax,1	    ; перерисовка?
	je	red
	
	cmp	eax,3	    ; кнопка?
	je	button
	
	cmp	eax,6	    ; мышь?
	je	mouse

	jmp	still2	    ; вернёмся в начало главного цикла
;------------------------------------------------------------------------------	
align 4
; ОБРАБОТЧИК МЫШИ
mouse:		  ; когда пользователь нажмёт кнопку мыши, закроемся
	mcall	37,2	; какие кнопки нажаты?
	test	eax,eax   ; никакие? - тогда прекрасно! вернёмся в главный цикл
	jz	still2

        mcall   37,0

        mov     esi, eax
        shr     esi, 16
        movzx   edi, ax
        mcall   9, procinfo, -1
	
        mov     eax, [procinfo.box.left]
        cmp     esi, eax
        jl      exit_menu

        add     eax, [procinfo.box.width]
        cmp     esi, eax
        jge     exit_menu

        mov     eax, [procinfo.box.top]
        cmp     edi, eax
        jl      exit_menu

        add     eax, [procinfo.box.height]
        cmp     edi, eax
        jge     exit_menu

        jmp     still2
;------------------------------------------------------------------------------	
align 4
; НАЖАТА КНОПКА
button:
	mcall	17	; получить идентификатор нажатой кнопки

	sub	ah,10	      ; сравниваем с 10
	jl	nofuncbtns    ; если меньше - закрываем меню

	movzx	ebx,ah	      ; получили номер программы в списке в ebx
	mov	esi,[startapps + ebx*4]
	mov	edi,start_info.path
	cld
;--------------------------------------
align 4
@@:
	lodsb
	stosb
	test	al,al
	jnz	@b
	mcall	70, start_info

;	mov	eax,5         ; подождём, пока программа запуститься
;	mov	ebx,1         ; а то её окно не будет отрисовано (баг в ядре???)
;	mcall          ; раскомментируйте эти строки, если у вас проблемы
		       ; с отрисовкой
;--------------------------------------
align 4
nofuncbtns:	      ; закрываем меню
	jmp	exit_menu
;------------------------------------------------------------------------------	
_BTNS_		  = 6	  ; количество кнопок ("пунктов меню")

if lang eq ru
  font		  = 0x00000000
  string_length   = 20		; длина строки
  wnd_x_size	  = 133 	; ширина окна
  title_pos	 = 36 shl 16 + 7
else
  font		  = 0x10000000
  string_length   = 12		; длина строки
  wnd_x_size	  = 105 	; ширина окна
  title_pos	 = 23 shl 16 + 7
end if
;------------------------------------------------------------------------------	
;*******************************
;********  РИСУЕМ ОКНО  ********
;*******************************
draw_window:
	mcall	12,1	; начинаем "рисовать"

	mov	eax,[curx1]	 ; текущие координаты курсора
	mov	[curx],eax	 ; запишем в координаты окна
	mov	eax,[cury1]
	mov	[cury],eax
; теперь будем считать координаты окна, чтобы оно за край экрана не вылезло
	mcall	14		; получим размер экрана

	mov	ebx,eax
	shr	eax,16			; в eax - x_screen
	and	ebx,0xffff		; в ebx - y_screen
	add	eax,-wnd_x_size		; eax = [x_screen - ширина окна]
	add	ebx,-_BTNS_*15-21	; ebx = [y_screen - высота окна]

	cmp	eax,[curx]
	jg	.okx			; если окно слишком близко к правому краю,
	add	[curx],-wnd_x_size	; сдвинем его влево на 100
;--------------------------------------
align 4
.okx:
	cmp	ebx, [cury]
	jg	.oky			; по вертикали точно также
	add	[cury], -_BTNS_*15-21
;--------------------------------------
align 4
.oky:
	xor	eax, eax	   ; функция 0 - создать окно
	mov	ebx, [curx]	   ;  ebx = [координата по x] shl 16 + [ширина]
	shl	ebx, 16
	add	ebx, wnd_x_size
	mov	ecx, [cury]	   ;  ecx = [координата по y] shl 16 + [высота]
	shl	ecx, 16
	add	ecx, _BTNS_*15+21
	mov	edx, [sc.work]	   ;  цвет рабочей области
	mov	esi, [sc.grab]	   ;  цвет заголовка
	or	esi, 0x81000000
	mov	edi, [sc.frame]    ;  цвет рамки
	mcall

	mov	eax, 4		   ; заголовок
	mov	ebx, title_pos	  ;  [x] shl 16 + [y]
	mov	ecx, [sc.grab_text];  шрифт и цвет (серый)
	or	ecx, 0x10000000

	push	ecx
	push	ecx
	xor	edx,edx
;--------------------------------------
align 4
.dec_color:
	sub	byte [esp+edx], 0x33
	jae	@f
	mov	byte [esp+edx], 0
;--------------------------------------
align 4
@@:
	inc	edx
	jnp	.dec_color
	pop	ecx
	mov	edx, title	  ;  адрес заголовка
	mov	esi, title.size   ;  длина заголовка ("M E N U")
	mcall
	pop	ecx
	add	ebx, 1 shl 16	   ;  сдвинем вправо на 1
	mcall

	mov	ebx, 1*65536+wnd_x_size-2  ; начинаем делать кнопки
	mov	ecx, 20*65536+15
	mov	edx, 10 or 0x40000000 ; бит 30 установлен => кнопка не рисуется

	mov	edi,_BTNS_	     ; количество кнопок (счётчик)
;--------------------------------------
align 4
newbtn:		     ; начало цикла
	mcall	8		;  создаём кнопку

			     ;  пишем текст на кнопке
	pushad		     ;   спасаем регистры
	shr	ecx, 16
	and	ebx, 0xffff0000
	add	ebx, ecx	     ;   ebx = [x] shl 16 + [y];
	add	ebx, 10*65536+4      ;   ebx += смещение относительно края кнопки;
	mov	ecx, [sc.work_text]  ;   шрифт и цвет
	or	ecx, font
	add	edx, -10	     ;   edx = номер кнопки;
	imul	edx, string_length   ;   edx *= длина строки;
	add	edx, text	     ;   edx += text;  теперь в edx адрес строки
	mov	esi, string_length   ;   в esi - длина строки
	mcall	4
	popad

	inc	edx		     ;  номер кнопки++;
	add	ecx,15*65536	     ;  увеличим смещение по y
	dec	edi		     ;  уменьшим счётчик
	jnz	newbtn		     ; если не ноль, повторим всё ещё раз

	mcall	12,2	; закончили "рисовать"
	ret			     ; возврат
;------------------------------------------------------------------------------	
align 4
; ДАННЫЕ ПРОГРАММЫ

  macro strtbl name, [string]
  {
   common
     label name dword
   forward
     local str
     dd str
   forward
     str db string
  }

  strtbl startapps	 ,\
    <"/sys/PIC4",0>	,\
    <"/sys/DESKTOP",0>	,\
    <"/sys/ICON",0>,\
    <"/sys/SETUP",0>	,\
    <"/sys/DEVELOP/BOARD",0> ,\
    <"/sys/CPU",0> 
    
  sz title, "KolibriOS"

  lsz text,\
    en, 'Background  ',\
    en, 'Desktop     ',\
    en, 'Icon manager',\
    en, 'Device setup',\
    en, 'Debug board ',\
    en, 'Processes   ',\
    \
    ru, 'Генератор обоев     ',\
    ru, 'Настройка окон      ',\
    ru, 'Управление иконками ',\
    ru, 'Настройка устройств ',\
    ru, 'Панель отладки      ',\
    ru, 'Процессы            ',\
    \
    et, 'Taust       ',\
    et, 'TЎЎlaud     ',\
    et, 'Ikooni hald.',\
    et, 'Seadme hald.',\
    et, 'Silumis aken',\
    et, 'Protsessid  '

;------------------------------------------------------------------------------	
align 4
start_info:
	.mode	dd 7
		dd 0
	.params dd 0
		dd 0
		dd 0
		db 0
		dd start_info.path
;------------------------------------------------------------------------------
IM_END:	
align 4
; НЕИНИЦИАЛИЗИРОВАННЫЕ ДАННЫЕ
  curx1		dd ?	; координаты курсора
  cury1		dd ?
  curx		dd ?	; координаты окна меню
  cury		dd ?

  menu_opened	db ?	; открыто меню или нет? (1-да, 0-нет)
;------------------------------------------------------------------------------	
align 4
start_info.path	rb 256
;------------------------------------------------------------------------------	
align 4
sc	system_colors	; системные цвета
;------------------------------------------------------------------------------	
align 4
procinfo process_information	; информация о процессе
;------------------------------------------------------------------------------	
align 4
	rb 512			; стэк для окна меню - хватит и 1 Кб
stack_wnd:
;------------------------------------------------------------------------------	
align 4
	rb 512
stack_area:
;------------------------------------------------------------------------------	
I_END:
;------------------------------------------------------------------------------	
; КОНЕЦ ПРОГРАММЫ
;------------------------------------------------------------------------------	
