
; Author: RU095@YANDEX.RU

use32
  org	 0x0
  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 0x300000
  dd	 0x27fff0
  dd	 0x0
  dd	 0x0

include 'lang.inc'
include '..\..\macros.inc' ; макросы облегчают жизнь ассемблерщиков!

include 'ASCL.INC'
include 'ASCGL.INC'

WND_COLOR equ 0x74000000

gif_hash_offset = gif_hash_area
COLOR_ORDER equ MENUETOS

DELAY equ  2	     ; Задержка перед переходом к следующему кадру в мсек

START:
;****************
; Открываем фаил
;****************
openfile:
;    and  [entered],0
;    mov ecx,499*3*4 ;at start
    xor ecx,ecx
    mov ecx,1

  ; Convert gif to animation format
    giftoani workarea,Image,ecx
    bmptoimg bullet_bmp, img_bullet
    bmptoimg level_bmp, img_level
    bmptoimg kirpich, img_area
    bmptoimg kirpich2, img_area2
    bmptoimg kirpich3, img_area3
    bmptoimg lest_bmp, img_lestniza
    bmptoimg water1_bmp, img_water1
    bmptoimg water2_bmp, img_water2
    bmptoimg door1_bmp, img_door1
    bmptoimg door2_bmp, img_door2


red:
    call timer
    call draw_window
;    call load_level
;    delay 200

still:
    timeevent DELAY,no_event, red, key, button

no_event:
    call draw_subimage
    jmp  still

  key:
 ; блока движения персонажа
    mov  eax,2
    int  0x40
     cmp  ah, 179	   ; право  4
     je go_mov
     cmp  ah, 176	   ; лево  1
     je go_mov
     cmp  ah, 178	   ; верх  3
     je go_mov
     cmp  ah, 177	   ; низ  2
     je go_mov
     cmp  ah, 120	   ; низ  2
     je go_fire
     cmp  ah, 122	   ; низ  2
     je go_fire
     cmp  ah, 239	   ; низ  2
     je go_fire_left
     cmp  ah, 231	   ; низ  2
     je go_fire


      jmp no_mov    ; if not left, right, up, down, then go NO MOV

    go_mov:
       ;mov ebx, [v_mov2]
       mov [v_mov2], ah
       sub word [v_mov2], 175	; отнимаем от кода клавиши 175 и остается 1,2,3 или 4 соответственно направления движения персонажа
      jmp no_mov    ; if not left, right, up, down, then go NO MOV

    go_fire_left:
       xor eax,eax
       mov ah, 122
       call fire
      jmp no_mov
    go_fire:
       call fire
    no_mov:
 ; конец блока движения персонажа
    jmp  still

  button:
    mov  eax, 17
    int  0x40

    cmp  ah,1
    jne  noclose
  _close:
    or	 eax,-1
    int  0x40
  noclose:
    jmp  still

;   *********************************************
;   *******  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА *******
;   *********************************************

draw_window:
    startwd

    mcall 48, 4
    mov  ecx, 50*65536+516
	add  ecx, eax
    mov  ebx, 50*65536+521
    mov  edx, WND_COLOR
    mov  edi, title
    xor  eax, eax
    int  0x40

    xor  ecx,ecx
    call draw_subimage
    cmp  [imgcount],1
    je	 .enddraw

    mov  ecx,[img_index]

    call draw_subimage

  .enddraw:

    endwd


ret

;*******************************************
; LABIRINT
;*******************************************

 labirint:

   ; вывод блоков лабиринта
   cycle:

    mov     eax, [Lab]
    shl     eax, 5	    ; сдвиг регистра в право на 5 позиции все равно что умножили на 32
    mov     [perem1], eax
    mov     eax, [Lab]
    shr     eax, 4	    ; сдвиг регистра в лево
    shl     eax, 5	    ; сдвиг обратно (все равно что целочисленное делениена 256)
    mov     [perem2], eax
    shl     eax, 4	    ; все равно что отнимаем от координат ненужный прирост -> У-256*X
    sbb  dword [perem1], eax;

  mov	 ebx, [Lab]
  sbb	 ebx, 1
  mov	  eax,[Dangeon + ebx]	 ; берет байт из дангеон + смещение

  cmp  ah, 1   ;
  je go_block

  cmp  ah, 0   ;
  je back_block

  cmp  ah, 12	; закраска фона за алмазом
  je back_block

  cmp  ah, 3   ;
  je boom_block

  cmp  ah, 16	;
  je door_block

  cmp  ah, 2   ; закраска фона за алмазом
  je cool_block

  jmp end_block
  go_block:
    imgtoimg img_area,dword [perem1],dword [perem2],canvas
  jmp end_block

  cool_block:
     imgtoimg img_area3,dword [perem1],dword [perem2],canvas
  jmp end_block


  back_block:
    imgtoimg img_area2,dword [perem1],dword [perem2],canvas
   jmp end_block

  door_block:
    imgtoimg img_door1,dword [perem1],dword [perem2],canvas

    mov eax, [gold]
    cmp ax, 10	      ; если собрано более 10 алмазов дверь открыта
    jna end_block
    imgtoimg img_door2,dword [perem1],dword [perem2],canvas
   jmp end_block


  boom_block:
    imgtoimg img_area,dword [perem1],dword [perem2],canvas
	   mov ebp,18 ; кадр
	   mov eax, dword [perem1]
	   mov dword [X_anime], eax
	   mov eax, dword [perem2]
	   mov dword [Y_anime], eax
	   call animashka

	     mov ecx, [timer1]	 ;
	     and ecx, 15
	     cmp ecx, 0
	     jne  end_block

	       mov    ebx, [Lab]
		mov	byte [Dangeon + ebx], 0    ; берет байт из дангеон + смещение


  end_block:

    add word [Lab],1
    mov     eax, [Lab]

   cmp	   eax, 256		      ;   if eax == 0
   je	   cycle_end		     ;   jump to cycle_end:
   jmp	   cycle		    ;

   cycle_end:

   mov	  eax, 0
   mov	  [Lab],eax

 ret   ; END labirint


;*******************************************
; ЛЕСТНИЦА
;*******************************************
 lestniza:
   ; вывод блоков лабиринта
   cycle2:

    mov     eax, [Lab]
    shl     eax, 5	    ; сдвиг регистра в право на 5 позиции все равно что умножили на 32
    mov     [perem1], eax
    mov     eax, [Lab]
    shr     eax, 4	    ; сдвиг регистра в лево
    shl     eax, 5	    ; сдвиг обратно (все равно что целочисленное делениена 256)
    mov     [perem2], eax
    shl     eax, 4	    ; все равно что отнимаем от координат ненужный прирост -> У-256*X
    sbb  dword [perem1], eax;


  mov	 ebx, [Lab]
  sbb	 ebx, 1
  mov	  eax,[Dangeon + ebx]	 ; берет байт из дангеон + смещение

  cmp  ah, 8   ;
  je go_lestniza

  jmp end_lestniza
  go_lestniza:

    imgtoimg img_lestniza,dword [perem1],dword [perem2],canvas
  end_lestniza:

    add word [Lab],1
    mov     eax, [Lab]

   cmp	   eax, 256		      ;   if eax == 0
   je	   cycle2_end		      ;   jump to cycle_end:
   jmp	   cycle2		     ;

   cycle2_end:

   mov	  eax, 0
   mov	  [Lab],eax


 ret   ; END lestniza



;*******************************************
; ВОДА     и  АЛМАЗЫ
;*******************************************
 water:

   ; вывод воды лабиринта
   cycle3:

    mov     eax, [Lab]
    shl     eax, 5	    ; сдвиг регистра в право на 5 позиции все равно что умножили на 32
    mov     [perem1], eax
    mov     eax, [Lab]
    shr     eax, 4	    ; сдвиг регистра в лево
    shl     eax, 5	    ; сдвиг обратно (все равно что целочисленное делениена 256)
    mov     [perem2], eax
    shl     eax, 4	    ; все равно что отнимаем от координат ненужный прирост -> У-256*X
    sbb  dword [perem1], eax;


  mov	 ebx, [Lab]
  sbb	 ebx, 1
  mov	  eax,[Dangeon + ebx]	 ; берет байт из дангеон + смещение

  cmp  ah, 4   ;
  je go_water

  cmp  ah, 12	;  рисование алмазов
  je go_almaz

  jmp end_water
  go_water:
  mov	  eax,0    ;
  mov	  eax, [timer1];
  and  eax, 15
  cmp  eax, 7	;
  jb go_water2

    imgtoimg img_water1,dword [perem1],dword [perem2],canvas
  jmp end_water

  go_water2:
    imgtoimg img_water2,dword [perem1],dword [perem2],canvas

  jmp end_water
  go_almaz:

  mov	  eax,0    ;
  mov	  eax, [timer1];
  and  eax, 31

   cmp eax, 10
   ja almaz2

   mov ebp,14 ; кадр
   mov eax, dword [perem1]
   mov dword [X_anime], eax
   mov eax, dword [perem2]
   mov dword [Y_anime], eax
   call animashka
   jmp end_water

   almaz2:
   cmp eax, 20
   ja almaz3

   mov ebp,15 ; кадр
   mov eax, dword [perem1]
   mov dword [X_anime], eax
   mov eax, dword [perem2]
   mov dword [Y_anime], eax
   call animashka
   jmp end_water

   almaz3:
   mov ebp,16 ; кадр
   mov eax, dword [perem1]
   mov dword [X_anime], eax
   mov eax, dword [perem2]
   mov dword [Y_anime], eax
   call animashka


  end_water:

    add word [Lab],1
    mov     eax, [Lab]

   cmp	   eax, 256		      ;   if eax == 0
   je	   cycle3_end		      ;   jump to cycle_end:
   jmp	   cycle3		     ;

   cycle3_end:

   mov	  eax, 0
   mov	  [Lab],eax


 ret   ; END water


;*******************************************
; Выводим следующий кадр из списка картинок
;*******************************************
draw_subimage:
  ;aframetoimg Image,32, 32,canvas,0x00ffffff ; НЕВИДИМЫЙ, прозрачный ЦВЕТ ИЛИ НЕ РИСУЕМЫЙ

  call labirint
  call lestniza
  call enemy_mas
  call enemy_mov
;-------------------------------------------------------------------------------
    mov ebx, [gold]
    cmp bx, 5		;увеличение скорости врагов если украл более 9
    jna lov_velo
     call enemy_mas
     call enemy_mov
    lov_velo:
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
    mov ebx, [gold]
    cmp bx, 9		;увеличение скорости врагов если украл более 9
    jna lov_velo2
     call enemy_mas
     call enemy_mov
    lov_velo2:
;-------------------------------------------------------------------------------
;  call enemy_mas
;  call enemy_mov
  call hiro
  call hiro_mov
  call water

  call bullet_mov
  call bullet_mov
  call bullet_mov
  call bullet_mas
  call bullet_mov
  call bullet_mov
  call bullet_mov
  call bullet_mas
  call proverka_gold
  call potop

  setimg 0,0,canvas

  call timer
  call tablo
ret

;@@@@@@@@@@@@@@@@@@
animashka:
;!!!ВАЖНО перед вызовом процедуры должен быть установлен ebx[номер кадра] и x,y

  mov edi,coords
  mov esi, Image ; гиф анимашка в памяти
;
;  mov ebp, dword [edi+8] ;ecx здесь конкретный номер выводимого кадра
;  inc ebp ; увеличение кадра
;  cmp ebp,3 ;  максимальное количество кадров !!!!!!!
;  jna ok_fr ; если выводимы кадр меньше максимального количества кадров
;!!!!!!!!!  mov ebp,2 ;обнуление кадра

ok_fr:
  mov dword [edi+8], ebp ;номер выводимого кадра
loo:
  cmp ebp,0  ; если кадр нулевой то сразу на вывод ?
  je  setpic
  movzx eax,word [esi+4]   ; если нет то увеличиваем х у ?
  movzx ebx,word [esi+6]
  mul ebx ;dword [esi+4] умножает ebx на eax и результат в eax edx
  mov ebx,3 ; количество кадров ???
  mul ebx
  add eax,8
  add esi,eax
  dec ebp
  jmp loo
setpic:

  pushad
; aframetoimg esi,dword [edi], dword [edi+4],canvas,0x00ffffff ; НЕВИДИМЫЙ, прозрачный ЦВЕТ ИЛИ НЕ РИСУЕМЫЙ
;  aframetoimg esi,32, 32,canvas,0x00ffffff ; НЕВИДИМЫЙ, прозрачный ЦВЕТ ИЛИ НЕ РИСУЕМЫЙ
  aframetoimg esi,dword [X_anime],dword [Y_anime],canvas,0x00ffffff ; НЕВИДИМЫЙ, прозрачный ЦВЕТ ИЛИ НЕ РИСУЕМЫЙ
  ;call enemy_mas
  popad
ret
;@@@@@@@@@@@@@@


 ;-------------- TIMER --------------
 timer:
     mov     eax,0    ;
     mov     eax,[timer1]    ;
     add     eax, 1;
     mov     [timer1], eax

     cmp     eax, 64   ;      таймер от 0 до 64
     je nul_timer
     jmp timer_dalee
     nul_timer :
     mov    [timer1], 0
     timer_dalee:
 ret
 ;-------------- TIMER --------------





;*******************************************
; ENEMY
;*******************************************

 enemy_mas:	  ; вывод ENEmy    ENEMYYYYYYYYYYYYYYY ENEMYYYYYYYYYYYYYY !!!

  mov word [Lab2], 0 ; все делать в WORD один элемент в массиве +2 следующий враг соотвтетственно 6*2= + 12

 cycle4:

  mov	 ebx, 0
  mov	 ecx, 0
  mov	 edx, 0
  mov	 esi, 0

  mov	 word bx, [Lab2]
  mov	 word ax,[Enemy+bx]    ; берет байт из Enemy + смещение

  cmp  ax, 1   ;   обычный враг номер 1
  je go_enemy

  jmp end_enemy
  go_enemy:

  add	 word bx, 2 ; X кооридината данного врага
  mov	 word ax, [Enemy + bx]
  mov	 word [X_enemy], ax

  add	 word bx, 2 ; Y кооридината данного врага
  mov	 word ax, [Enemy + ebx]
  mov	 word [Y_enemy], ax

  add	 word bx, 2 ; положение в массиве данного врага
  mov	 word ax, [Enemy + ebx]
  mov	 word [lab_enemy], ax

  add	 word bx, 2 ; направление движения данного врага
  mov	 word ax, [Enemy + ebx]
  mov	 word [mov_enemy], ax

  add	 word bx, 2 ; enemy LIFE
  mov	 word ax, [Enemy + ebx]
  mov	 word [life_enemy], ax

  cmp  ax, 499;            ; замороженные враги
  ja enemy_dalee
   mov ebp,2 ; кадр
   mov eax, dword [X_enemy]
   mov dword [X_anime], eax
   mov eax, dword [Y_enemy]
   mov dword [Y_anime], eax
   call animashka
   jmp enemy_bmp_end



  enemy_dalee:
  xor	  eax,eax    ;
  mov	  eax, [X_enemy]    ;
  and  eax, 15
  cmp  ax, 7;
  ja enemy_1

  jmp enemy_2

   enemy_1:

   mov ebp,0 ; кадр
   mov eax, dword [X_enemy]
   mov dword [X_anime], eax
   mov eax, dword [Y_enemy]
   mov dword [Y_anime], eax
   call animashka

   jmp enemy_bmp_end

   enemy_2:
   mov ebp,1 ; кадр
   mov eax, dword [X_enemy]
   mov dword [X_anime], eax
   mov eax, dword [Y_enemy]
   mov dword [Y_anime], eax
   call animashka
   enemy_bmp_end:

   end_enemy:

    mov     eax, 0
    mov     word ax, [Lab2]

   cmp	   ax, 96		    ;   if eax == 0
   je	   cycle4_end		      ;   jump to cycle_end:

    add     word [Lab2],12

   jmp	   cycle4		     ;

   cycle4_end:

 ret   ; END eNEMYYYYYYYYYYYYYYY ENEMYYYYYYYYYYYYYY !!!

;$$$$$$$$$$$$$$$$$$$$$$$$$$
;   ENEMY MOV
;$$$$$$$$$$$$$$$$$$$$$$$$$$
enemy_mov:
  mov word [Lab2], 0 ; все делать в WORD один элемент в массиве +2 следующий враг соотвтетственно 6*2= + 12

 cycle5:

  mov	 ebx, 0
  mov	 ecx, 0
  mov	 edx, 0
  mov	 esi, 0

  mov	 word bx, [Lab2]
  mov	 word ax,[Enemy+bx]    ; берет байт из Enemy + смещение

  cmp  ax, 1   ;   обычный враг номер 1
  je go_mov_enemy

  jmp end_mov_enemy
  go_mov_enemy:

  add	 word bx, 2 ; X кооридината данного врага
  mov	 word ax, [Enemy + bx]
  mov	 word [X_enemy], ax

  add	 word bx, 2 ; Y кооридината данного врага
  mov	 word ax, [Enemy + ebx]
  mov	 word [Y_enemy], ax

  add	 word bx, 2 ; положение в массиве данного врага
  mov	 word ax, [Enemy + ebx]
  mov	 word [lab_enemy], ax

  add	 word bx, 2 ; направление движения данного врага
  mov	 word ax, [Enemy + ebx]
  mov	 word [mov_enemy], ax

  add	 word bx, 2 ; enemy LIFE
  mov	 word ax, [Enemy + ebx]
  mov	 word [life_enemy], ax

  cmp  ax, 499;            ; замороженные враги
  ja  dalee_enemy_mov
  add ax, 1

  mov	 word [Enemy + ebx], ax

  jmp end_mov_enemy

  dalee_enemy_mov:
     ; блок поедания героя
      mov  ax, [Xpl]
      add  ax, 23
      cmp ax, [X_enemy]
	jna ne_em
	 sbb ax, 45
	 cmp ax, [X_enemy]
	  ja ne_em
	    mov  ax, [Ypl]
	    add  ax, 23
	    cmp ax, [Y_enemy]
	     jna ne_em
	     sbb ax, 45
	     cmp ax, [Y_enemy]
	  ja ne_em
	  call load_level  ; DEAD HIRO
	  delay 150

       ne_em:
     ;Блок попадания 1 пули

  mov	 word bx, 0
  mov	 word ax,[Bullet + ebx]
  cmp ax, 0
  je no_bullet

  add	 word bx, 2   ; [Xpl]
  mov	 word cx, [Bullet + ebx]

  add	 word bx, 2   ; [Ypl]
  mov	 word dx, [Bullet + ebx]

    cmp dx, [Y_enemy]
    jna no_bullet
      sbb dx, 32
      cmp dx, [Y_enemy]
      ja no_bullet
      cmp cx, [X_enemy]
       jna no_bullet
       sbb cx, 32
	cmp cx, [X_enemy]
	ja no_bullet
	mov    word bx, [Lab2]
	add    word bx, 10 ; enemy LIFE
	mov    word [Enemy + ebx],0

	mov    byte [Bullet], 0    ;

    no_bullet:


     ;БЛОК ПРИРОСТА ДВИЖЕНИЯ ВРАГОВ
	mov word ax, [mov_enemy]		      ;
	cmp ax, 1	     ; left                   ;
	jne next_2				      ;
	  sbb	 word [X_enemy], 1		      ;
	  mov	 word ax, [X_enemy]		      ;
	  mov	 word bx, [Lab2]		      ;
	  add	 word bx, 2 ; X кооридината данного врага
	  mov	 word [Enemy + bx], ax		      ;
						      ;
	next_2: 				      ;
	cmp ax, 2	     ; down                   ;
	jne next_3				      ;
	  add	 word [Y_enemy], 1		      ;
	  mov	 word ax, [Y_enemy]		      ;
	  mov	 word bx, [Lab2]		      ;
	  add	 word bx, 4 ; Y кооридината данного врага
	  mov	 word [Enemy + bx], ax		      ;
						      ;
	next_3: 				      ;
	cmp ax, 3	     ; up                     ;
	jne next_4				      ;
	  sbb	 word [Y_enemy], 1		      ;
	  mov	 word ax, [Y_enemy]		      ;
	  mov	 word bx, [Lab2]		      ;
	  add	 word bx, 4 ; Y кооридината данного врага
	  mov	 word [Enemy + bx], ax		      ;
						      ;
	next_4: 				      ;
	cmp ax, 4	     ; right                  ;
	jne next_end

	     ;Гравитационная заплатка   лечит трабл - когда враг двигается слева на право то пролетает один пустой блок
	      mov word bx, [X_enemy] ; Это условие позволяет добежать до позиции напротив блока
	      and ebx, 31    ; X в позиции напротив блока
	      cmp bx, 0     ;
	      jne n4_dalee   ;
		mov    bx,[lab_enemy]
		add    bx,16
		mov    ax, [Dangeon + bx]
		and   eax,15
		 cmp word ax, 0
		 je next_end
		 cmp word ax, 4
		 je next_end
	      ;конец заплатки

	   n4_dalee:
	;
	  add	 word [X_enemy], 1		      ;
	  mov	 word ax, [X_enemy]		      ;
	  mov	 word bx, [Lab2]		      ;
	  add	 word bx, 2 ; X кооридината данного врага
	  mov	 word [Enemy + bx], ax		      ;
						      ;
	next_end:
	  ;END БЛОК ПРИРОСТА ДВИЖЕНИЯ ВРАГ

	  ;

     ;  Рандомный выбор на леснице
	mov word bx, [X_enemy] ; Это условие позволяет добежать до позиции напротив блока
	and ebx, 31    ; X в позиции напротив блока
	cmp bx, 0     ;
	jne end_les   ;
	   mov word bx, [Y_enemy] ; Это условие позволяет добежать до позиции напротив блока
	   and ebx, 31	  ; X в позиции напротив блока
	   cmp bx, 0	 ;
	   jne end_les	 ;

		random 300, eax

		 cmp eax, 150
		 ja les2
		 jmp end_up_or_down
		les2:

		mov eax, [Ypl]
		cmp eax, [Y_enemy]
		jna bolee
		mov    word [mov_enemy],2		    ; вниз
		mov    word ax, [mov_enemy]		    ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 8  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;
		jmp end_les

		bolee:
		mov    word [mov_enemy],3		    ; up
		mov    word ax, [mov_enemy]		    ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 8  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;
		jmp end_les

		end_up_or_down:

		 random 300, eax
		 cmp eax, 150
		 ja les22
		 jmp end_les
		les22:

		mov eax, [Xpl]
		cmp eax, [X_enemy]
		jna bolee2
		mov    word [mov_enemy],4		    ; вниз
		mov    word ax, [mov_enemy]		    ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 8  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;
		jmp end_les

		bolee2:
		mov    word [mov_enemy],1		    ; up
		mov    word ax, [mov_enemy]		    ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 8  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;

	end_les:
     ;  end Рандомный выбор на леснице

     ;Гравитация
	mov word bx, [X_enemy] ; Это условие позволяет добежать до позиции напротив блока
	and ebx, 31    ; X в позиции напротив блока
	cmp bx, 0     ;
	jne end_grava	;

		mov    bx,[lab_enemy]
		add    bx,16
		mov    ax, [Dangeon + bx]

		and   eax,15
		 cmp word ax, 0
		 je grava
		 cmp word ax, 4
		 je grava
		 jmp end_grava
		grava:
		mov    word [mov_enemy], 2		    ; padenie ВРАГА
		mov    word ax, [mov_enemy]		    ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 8  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;

	end_grava:
     ;конец гравитации

	  ;блок расчета положения врага в лабиринте
	    mov eax, 0				  ;
	    mov eax, [Y_enemy]			      ;
	    shr eax, 5				  ;
	    shl eax, 4				  ;
	    mov ebx, 0				  ;
	    mov ebx, [X_enemy]			      ;
	    shr ebx, 5				  ;
	    add eax, ebx			  ;
	    mov [lab_enemy], eax		   ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 6  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;
	  ;блок расчета положения врага в лабиринте






       ;БЛОК ЗПАПРЕТА ДВижения
	mov word bx, [Y_enemy] ; Это условие позволяет добежать до позиции напротив блока
	and bx, 31    ; Y в позиции напротив блока
	cmp bx, 0     ;
	jne end_zapret	 ;
	   mov word ax, [X_enemy] ; Это условие позволяет добежать до позиции напротив блока
	   and ax, 31	 ; X в позиции напротив блока
	   cmp ax, 0	 ;
	   jne end_zapret   ;

	      mov word ax, [mov_enemy] ; Это условие позволяет добежать до позиции напротив блока

	      cmp ax, 0
	      jne duma1
		    ; Рандомный выбор движения
		 mov eax, [timer1]
		 and eax, 3
		 cmp eax, 0
		 jne dalee_duma1
		 add eax, 4
		 dalee_duma1:

		mov    word [mov_enemy], ax		     ;
		mov    word ax, [mov_enemy]		    ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 8  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;



	      duma1:
	      cmp ax, 4
	      jne duma2  ; прверка справа
		mov    bx,[lab_enemy]
		add    bx,1
		mov    ax, [Dangeon + bx]
		and   eax,3
		 cmp ax, 0
		 jne duma_stop
		 jmp duma_end

	      duma2:
	      cmp ax, 1
	      jne duma3  ; слева
		mov    bx,[lab_enemy]
		sbb    bx,1
		mov    ax, [Dangeon + bx]
		and   eax,3
		 cmp ax, 0
		 jne duma_stop
		 jmp duma_end

	      duma3:
	      cmp ax, 2
	      jne duma4  ; снизу
		mov    bx,[lab_enemy]
		add    bx,16
		mov    ax, [Dangeon + bx]
		and   eax,3
		 cmp ax, 0
		 jne duma_stop
		 jmp duma_end

	      duma4:
	      cmp ax, 3
	      jne duma_stop  ; сверху

		mov    bx,[lab_enemy]	  ; если кирпич над головой
		sbb    bx,16
		mov    ax, [Dangeon + bx]
		and   eax,3
		 cmp ax, 0		  ; если кирпич над головой
		 jne duma_stop

		mov    bx,[lab_enemy]	  ; если враг не на леснице
		mov    ax, [Dangeon + bx]
		and   eax,15
		cmp ax, 8
		 jne duma_stop

		 jmp duma_end

	      duma_stop:

		mov    word [mov_enemy], 0		    ; ОСТАНОВКА ВРАГА
		mov    word ax, [mov_enemy]		    ;
		mov    word bx, [Lab2]			    ;
		add    word bx, 8  ; направление движения данного врага
		mov    word [Enemy + bx], ax		    ;

	   duma_end:

	 end_zapret:
	 ;конец запрета

  end_mov_enemy:

    mov     eax, 0
    mov     word ax, [Lab2]

   cmp	   ax, 96		    ;   if eax == 0
   je	   cycle5_end		      ;   jump to cycle_end:

    add     word [Lab2],12

   jmp	   cycle5		     ;

   cycle5_end:


ret

;*****************************
;   HIRO
;*****************************

 hiro:

  mov ecx, [v_fire]   ;стрельба
  cmp ecx, 0
  je  next_anima
      cmp ecx, 1
	je wprawo
	 mov ebp,12 ; кадр
	 mov eax, dword [Xpl]
	 mov dword [X_anime], eax
	 mov eax, dword [Ypl]
	 mov dword [Y_anime], eax
	 call animashka
	     mov ecx, [timer1]	 ;стрельба
	     and ecx, 15
	     cmp ecx, 0
	     jne  end_hiro
	      mov dword [v_fire], 0
	  jmp end_hiro

	 wprawo:
	 mov ebp,11 ; кадр
	 mov eax, dword [Xpl]
	 mov dword [X_anime], eax
	 mov eax, dword [Ypl]
	 mov dword [Y_anime], eax
	 call animashka
	     mov ecx, [timer1]	 ;стрельба
	     and ecx, 15
	     cmp ecx, 0
	     jne  end_hiro
	      mov dword [v_fire], 0
	  jmp end_hiro

		      ;стрельба
   jmp end_hiro

 next_anima:
  mov eax, [v_mov]
  cmp eax, 1
  jne next_r
  mov	  eax,0    ;
  mov	  eax, [Xpl];
  and  eax, 15
  cmp  eax, 7	;
  ja next_l1

   mov ebp,3 ; кадр
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka
   jmp end_hiro

   next_l1:
   mov ebp,5 ; кадр
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka
       pusha
       mcall 55, eax, , , Music_mov   ; звук выстрела
       popa

   jmp end_hiro

  next_r:
  cmp eax, 4
  jne next_d

  mov	  eax,0    ;
  mov	  eax, [Xpl];
  and  eax, 15
  cmp  eax, 7	;
  ja next_d1

   mov ebp,4 ; кадр
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka

       pusha
       mcall 55, eax, , , Music_mov   ; звук выстрела
       popa
   jmp end_hiro

   next_d1:
   mov ebp,6 ; кадр
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka

   jmp end_hiro

  next_d:
  cmp eax, 2
  jne next_u
   mov ebp,13 ; кадр
    mov eax, [temp2]
    cmp eax, 8
    jne padaem
   mov ebp,10 ; кадр
    padaem:
    mov eax, [temp5]
    cmp eax, 8
    jne padaem2
   mov ebp,10 ; кадр
    padaem2:
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka
   jmp end_hiro

  next_u:
  cmp eax, 3
  jne next_s
  mov	  eax,0    ;
  mov	  eax, [Ypl];
  and  eax, 15
  cmp  eax, 7	;
  ja next_u1

   mov ebp,10 ; кадр
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka
   jmp end_hiro

   next_u1:
   mov ebp,9 ; кадр
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka

   jmp end_hiro

  next_s:
  cmp eax, 0
  jne next_s
   mov ebp,17; кадр
   mov eax, dword [Xpl]
   mov dword [X_anime], eax
   mov eax, dword [Ypl]
   mov dword [Y_anime], eax
   call animashka
   jmp end_hiro

 end_hiro:

 ret   ; END Hiro
;**************
;  end HIRO
;**************


;****************
;  Hiro MOV
;****************
hiro_mov:

     ;ПРЕГРАД

       ; вычисление соседних блоков
       mov    ebx, [hiro_lab]
       sbb    ebx,2
       mov    eax,[Dangeon + bx]    ;                 up
       mov    [temp1], ah	    ;               temp4
				    ;
       add    ebx,1		    ;        left    hiro  right
       mov    eax,[Dangeon + bx]    ;       temp1   temp2  temp3
       mov    [temp2], ah	    ;

;-------------------------------------------------------------------------------
;     Сбор алмазов
;-------------------------------------------------------------------------------
	mov ecx, [Ypl] ; Это условие позволяет добежать до позиции напротив блока
	and ecx, 31    ; Y в позиции напротив блока

	mov edx, [Xpl] ; Это условие позволяет добежать до позиции напротив блока
	and edx, 31    ; Y в позиции напротив блока
	or edx,ecx ; Мы четко спозиционированы по Х и У
	cmp edx, 0
	jne no_almaz

	  cmp ah, 12
	  jne no_almaz
	  xor cx, cx
	  add ebx,1  ; ЗАПЛАТКА !!!  без нее почемуто обнуляет не алмазы а соседние блоки
	  mov  [Dangeon + bx], ch
	  sbb ebx, 1  ; ЗАПЛАТКА !!!
	  mov ecx, [gold]
	  add ecx, 1
	  mov [gold], ecx
       pusha
       mcall 55, eax, , , Music_almaz	; звук Almaz
       popa
	  no_almaz:
;-------------------------------------------------------------------------------
;     Сбор алмазов
;-------------------------------------------------------------------------------
				    ;                down
       add    ebx,1		    ;               temp5
       mov    eax,[Dangeon + bx]    ;
       mov    [temp3], ah

       sbb    ebx,17		    ; !!!!!!!!!!!!
       mov    eax,[Dangeon + bx]    ; при отсутствии библиотеки  MACROS.INC   ebx=17 , при ее присутствии ebx=16
       mov    [temp4], ah

       add    ebx,32		    ;
       mov    eax,[Dangeon + bx]    ;
       mov    [temp5], ah
       ; вычисление соседних блоков





     ;разрешение на движение
     mov     eax,0		;
     mov     eax,[v_mov2]	;
     and     eax,7
     and     eax,2
     cmp eax, 2
     je smena

	mov ebx, [Ypl] ; Это условие позволяет добежать до позиции напротив блока
	and ebx, 31    ; Y в позиции напротив блока
	cmp ebx, 0     ;
	jne end_tamgna	 ;
	smena:

	 mov	 eax,0		    ;
	 mov	 eax,[v_mov2]	    ;
	 and	 eax,7
	 and	 eax,2
	 cmp eax, 0
	 je smena2

	 mov ebx, [Xpl] ; Это условие позволяет добежать до позиции напротив блока
	 and ebx, 31	; X в позиции напротив блока
	 cmp ebx, 0	;
	 jne end_tamgna   ;

	  smena2:
	  mov eax, [v_mov2]
	  and eax, 7
	  mov [v_mov], eax
     end_tamgna:
     ;разрешение на движение


     ;УПИРАНИЕ В ПРЕГРАДУ
	mov ebx, [Ypl] ;
	and ebx, 31    ; Y в позиции напротив блока
	cmp ebx, 0     ;
	jne end_stop   ;
	mov eax, [Xpl] ; без макроса этого блока не было он был внутри блока упираний
	and eax, 31    ; X в позиции напротив блока
	cmp eax, 0     ;
	jne end_stop   ;
     mov eax, [v_mov]

     cmp  eax, 2
       je stop_d
       jmp stop_u

       stop_d:
	mov ebx, [temp5]
	  ;заплатка 2 равно 1 если бетон то это как кирпич, иначе герой проходит сквозь бетон

	  cmp ebx, 2
	  jne no_zaplat1

	   sbb ebx,1

	  no_zaplat1:

	and ebx,3
	or eax, ebx

	 cmp eax, 2
	 je end_stop
	  mov ebx, 0
	  mov [v_mov], ebx

       stop_u:
       cmp  eax, 3
       jne stop_l

	mov ebx, [temp4]
	and ebx,3
	and eax, ebx

    ;    mov ebx, [Ypl] ; Это условие позволяет добежать до позиции напротив блока
    ;    and ebx, 31    ; Y в позиции напротив блока
    ;    cmp ebx, 0     ;
    ;    jne end_stop   ;


	 cmp eax, 0
	 je end_stop
	  mov ebx, 0
	  mov [v_mov], ebx

       stop_l:
       cmp  eax, 1
       jne stop_r

	mov ebx, [temp1]
	and ebx,3

    ;    mov eax, [Xpl] ; Это условие позволяет добежать до позиции напротив блока
    ;    and eax, 31    ; Y в позиции напротив блока
    ;    cmp eax, 0     ;
    ;    jne end_stop   ;

	 cmp ebx, 0
	 je end_stop
	  mov ebx, 0
	  mov [v_mov], ebx

       stop_r:
       cmp  eax, 4
       jne end_stop

	mov ebx, [temp3]
	and ebx,3

	 cmp ebx, 0
	 je end_stop
	  mov ebx, 0
	  mov [v_mov], ebx

      end_stop:
     ;ПРЕГРАДЫ


     ;GRAVITACION
	mov ebx, [Ypl] ; Это условие позволяет добежать до позиции напротив блока
	and ebx, 31    ; Y в позиции напротив блока

	mov eax, [Xpl] ; Это условие позволяет добежать до позиции напротив блока
	and eax, 31    ; Y в позиции напротив блока
	or eax,ebx ; Мы четко спозиционированы по Х и У
	cmp eax, 0
	jne end_grav

	  mov ebx, [temp2]    ;
	  cmp ebx, 0	      ; запрет на прыжки вверх
	  jne end_grav	      ;
	    mov ebx, [v_mov]  ;
	    cmp ebx, 3	      ;
	    jne dalee_gra     ;
	       mov ebx, 0     ;
	 mov [v_mov],ebx      ;
	 mov ebx, 0	      ;
	 mov [v_mov2],ebx     ;

	dalee_gra:
	add eax, [temp5]       ;
	cmp eax, 0	       ; падение в пустоту
	 jne dalee_g	       ;
	 mov eax, 2	       ;
	 mov [v_mov],eax       ;
	 mov eax, 0	       ;
	 mov [v_mov2],eax      ;

	 dalee_g:
	cmp eax, 4	       ;
	 jne dalee_g2	       ; падение в воду
	 mov eax, 2	       ;
	 mov [v_mov],eax       ;
	 mov eax, 0	       ;
	 mov [v_mov2],eax      ;

	 dalee_g2:
	cmp eax, 12		;
	 jne end_grav	       ; падение на алмаз
	 mov eax, 2	       ;
	 mov [v_mov],eax       ;
	 mov eax, 0	       ;
	 mov [v_mov2],eax      ;

       end_grav:
     ;end GRAVITACION




     ; ПРИРОСТ КООРДИНАТ ГЕРОЯ
     mov     eax,0		;
     mov     eax,[v_mov]	;
     cmp     eax, 2		;
     je h_down			;
   jmp proverka2		;
    jmp hiro_mov_end		;
	h_down: 		;
	 mov ebx, [Ypl] 	;
	 add ebx, 2		;                             3
	 mov [Ypl], ebx 	;                           1   4
    jmp hiro_mov_end		;
				;
   proverka2:			;
     cmp     eax, 3   ;         ;
     je h_up			;    PRIROST X Y
   jmp proverka3		;
    jmp hiro_mov_end		;
	h_up:			;
	 mov ebx, [Ypl] 	;
	 sbb ebx, 2		;
	 mov [Ypl], ebx 	;
    jmp hiro_mov_end		;
				;
   proverka3:			;
     cmp     eax, 1   ;         ;
     je h_left			;
   jmp proverka4		;
    jmp hiro_mov_end		;
	h_left: 		;
	 mov ebx, [Xpl] 	;
	 sbb ebx, 2		;
	 mov [Xpl], ebx 	;
    jmp hiro_mov_end		;
				;
   proverka4:			;
     cmp     eax, 4		;
     je h_right 		;
   jmp hiro_mov_end		;
    jmp hiro_mov_end		;
	h_right:		;
	 mov ebx, [Xpl] 	;
	 add ebx, 2		;
	 mov [Xpl], ebx 	;
    jmp hiro_mov_end	       ;прирост
     ; ПРИРОСТ КООРДИНАТ ГЕРОЯ

    hiro_mov_end:

    ; вычисляем положение героя в лабиринте
    mov eax, 0				  ;
    mov dword eax, [Ypl]			;
    shr eax, 5				  ;
    shl eax, 4				  ;
    mov ebx, 0				  ;
    mov dword ebx, [Xpl]			;
    shr ebx, 5				  ;
    add eax, ebx			  ;
    mov [hiro_lab], eax 		  ;
    ; вычисляем положение героя в лабиринте
ret
;****************
;  end Hiro mov
;****************


;*******************************************
; BULLET
;*******************************************
 bullet_mas:
  mov word [Lab3], 0 ; все делать в WORD один элемент в массиве +2 следующий враг соотвтетственно 6*2= + 12
 cycle7:
  mov	 word bx, [Lab3]
  mov	 word ax, [Bullet+bx]	 ; берет байт из Enemy + смещение

  cmp  ax, 1   ;   bullet ok
  je go_bullet

  jmp end_bullet
  go_bullet:

  add	 word bx, 2 ; X кооридината данного врага
  mov	 word ax, [Bullet + bx]
  mov	 word [X_bul], ax

  add	 word bx, 2 ; X кооридината данного врага
  mov	 word ax, [Bullet + bx]
  mov	 word [Y_bul], ax

    imgtoimg img_bullet, dword [X_bul], dword [Y_bul],canvas
  end_bullet:

    mov     eax, 0
    mov     word ax, [Lab3]

   cmp	   ax, 36		    ;   if eax == 0
   je	   cycle7_end		      ;   jump to cycle_end:

    add     word [Lab3],12

   jmp	   cycle7		     ;

   cycle7_end:
 ret  ; end BULLET
;-------------------------------------------------------------------------------


;*******************************************
; BULLET MOV
;*******************************************
 bullet_mov:
  mov word [Lab3], 0 ; все делать в WORD один элемент в массиве +2 следующий враг соотвтетственно 6*2= + 12

 cycle8:
  mov	 word bx, [Lab3]
  mov	 word ax, [Bullet+bx]	 ; берет байт из Enemy + смещение

  cmp  ax, 1   ;   bullet ok
  je go_bullet_mov

  jmp end_bullet_mov
  go_bullet_mov:

  add	 word bx, 2 ; X кооридината данного врага
  mov	 word ax, [Bullet + bx]
  mov	 word [X_bul], ax

  add	 word bx, 2 ; X кооридината данного врага
  mov	 word ax, [Bullet + bx]
  mov	 word [Y_bul], ax

  add	 word bx, 2 ;
  mov	 word ax, [Bullet + bx]
  mov	 word [mov_bullet], ax

    cmp  ax, 1
    jne bul_muv_left
      mov word ax, [X_bul]
      sbb ax, 2
      mov word bx, [Lab3]
      add word bx, 2
      mov word [Bullet + bx], ax

    jmp end_bullet_mov
    bul_muv_left:
      mov word ax, [X_bul]
      add ax, 2
      mov word bx, [Lab3]
      add word bx, 2
      mov word [Bullet + bx], ax

    end_bullet_mov:

    ; вычисляем положение BULLET в лабиринте
    mov eax, 0				  ;
    mov eax, [Y_bul]			  ;
    shr eax, 5				  ;
    shl eax, 4				  ;
    mov ebx, 0				  ;
    mov ebx, [X_bul]			  ;
    shr ebx, 5				  ;
    add eax, ebx			  ;
    mov [lab_bullet], ax		   ;
    ; вычисляем положение BULLET в лабиринте

		mov    word bx,[lab_bullet]
		mov    dx, [Dangeon + bx]
		and   edx,3
		 cmp dx, 0
		 je letit_dalee

		 cmp dx, 2
		 je beton
		   mov	  byte [Dangeon + bx], 3    ; РАЗРУШЕНИЕ КИРПИЧЕЙ
		  beton:
		   mov	  bx, [Lab3]
		   mov	  byte [Bullet + ebx], 0    ; берет байт из Enemy + смещение





    letit_dalee:
    mov     eax, 0
    mov     word ax, [Lab3]

   cmp	   ax, 36		    ;   if eax == 0
   je	   cycle8_end		      ;   jump to cycle_end:

    add     word [Lab3],12

   jmp	   cycle8		     ;

   cycle8_end:


ret  ; end BULLET
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; TABLO  TABLO TABLO  TABLO TABLO TABLO
;*****************************************
 tablo:
    ; WIWOD CIFR
   ;; mov     eax, 13
   ; mov     ebx, 140*65536 + 18
   ; mov     ecx, 10*65536 + 8
   ; mov     edx, 0x00ffffff
   ; int     0x40
   ; outcount [numer_level],140,10,0x224466,3*65536

    ; WIWOD CIFR
   ; mov     eax, 13
   ; mov     ebx, 160*65536 + 18
   ; mov     ecx, 10*65536 + 8
   ; mov     edx, 0x00ffffff
   ; int     0x40
   ; outcount [delta_level],160,10,0x224466,3*65536

    ; WIWOD CIFR
   ; mov     eax, 13
   ; mov     ebx, 180*65536 + 18
   ; mov     ecx, 10*65536 + 8
   ; mov     edx, 0x00ffffff
   ; int     0x40
   ; outcount [delta_level_end],180,10,0x224466,3*65536

 ret
;-------------------------------------------------------------------------------
fire:
  sbb  ah, 121
  mov  [v_fire], ah

  mov	 word bx, 0
  mov  cx, [Bullet + ebx]
  cmp cx, 0
   jne end_fire
       pusha
       mcall 55, eax, , , Music_bah   ; звук выстрела
       popa

  mov	 byte [Bullet + ebx], 1

  add	 word bx, 2 ; X кооридината данного врага
  mov	 ax, [Xpl]
  add	 ax, 8
  mov	 word [Bullet + ebx],ax

  add	 word bx, 2 ; X кооридината данного врага
  mov	 ax, [Ypl]
  add	 ax, 16
  mov	 word [Bullet + ebx],ax

  add	 word bx, 2 ;
  mov  ax,[v_fire]
  mov	 word [Bullet + ebx],ax

  end_fire:
ret
;-------------------------------------------------------------------------------


;------------------------Загрузка уровней---------------------------------------
load_level:
    imgtoimg img_level,32,32,canvas
  setimg 0,0,canvas

    xor eax,eax
    xor ebx,ebx
    xor edx,edx
    xor ecx,ecx
    mov dword [Bullet], eax
    mov dword [Bullet+2], eax
    mov dword [Bullet+4], eax
    mov dword [Bullet+6], eax
    mov dword [Bullet+8], eax
    mov dword [Bullet+10], eax
    mov byte [Lab], 1
    mov [lab_bullet], eax
    mov [mov_bullet], eax
    mov [X_bul], eax
    mov [Y_bul], eax
    mov [gold], eax
    mov dword [Xpl], 32
    mov dword [Ypl], 448
    mov dword [hiro_lab],0 ; проекция героя в лабиринт
    mov dword [v_mov], 0
    mov dword [v_mov2], 0
    mov dword [v_fire], 0

    xor eax,eax
    mov eax, [numer_level]
    shl eax, 8
    mov dword [delta_level], eax
    add eax, 256
    mov dword [delta_level_end], eax

   cycleL:

     xor ebx, ebx
     mov    ebx, [Lab]
     add ebx, [delta_level]
     mov     ax,[Dangeon2 + ebx]    ; берет байт из дангеон + смещение

     sbb ebx, [delta_level]
     mov      [Dangeon + ebx], ax    ; берет байт из дангеон + смещение
     add word [Lab],1
     mov     eax, [Lab]


   cmp	   eax, [delta_level_end]		    ;   if eax == 0
   je	   cycleL_end		      ;   jump to cycle_end:
   jmp	   cycleL		     ;
   cycleL_end:

   mov	  eax, 0
   mov	  [Lab],eax

 call enemy_plaz

ret
;-------------------------------------------------------------------------------

;************************************
;GOLD
proverka_gold:
mov eax, [gold]
 cmp ax, 10
  jna net_deneg
   mov eax, [temp2] ; добежать до двери
   cmp ax, 16
    jne net_deneg
       mov ax, [numer_level]  ; увеличение левела

       add ax, 1
       mov [numer_level],  ax
       mov [demon_kolvo], ax   ;

     call load_level
     delay 100

  net_deneg:
ret

;-------------------------------------------------------------------------------
; УТОП
potop:
   mov eax, [temp2] ; добежать до двери
   cmp ax, 4
    jne jiwoy
       mov dword [gold], 0  ; увеличение левела

     call load_level
     delay 100
   jiwoy:
ret

;************************************
;положение врагов на новом уровне
enemy_plaz:
  mov word [Lab2], 0 ; все делать в WORD один элемент в массиве +2 следующий враг соотвтетственно 6*2= + 12
  mov ax, [demon_kolvo]
  mov [demon], ax

 cycle55:
  mov	 ebx, 0
  mov	 ecx, 0
  mov	 edx, 0
  mov	 esi, 0

	  mov cx, [demon]
	  cmp cx, 0
	   je end_demon
	  mov	 word bx, [Lab2]		      ;
	  mov	 word [Enemy + bx], 1		     ;
	  sbb cx, 1
	  mov [demon], cx
	  end_demon:

	  random 14, eax
	  add eax, 1
	  shl eax,5
	  mov	 word bx, [Lab2]		      ;
	  add	 word bx, 2 ; X кооридината данного врага
	  mov	 word [Enemy + bx], ax		      ;
						      ;
	  random 12, eax
	  add eax, 1
	  shl eax,5
	  mov	 word bx, [Lab2]		      ;
	  add	 word bx, 4 ; Y кооридината данного врага
	  mov	 word [Enemy + bx], ax		      ;

	  mov	 word bx, [Lab2]		      ;
	  add	 word bx, 8 ; направление движения
	  mov	 word [Enemy + bx], 0		     ;
	  add	 word bx, 2 ; жизнь
	  mov	 word [Enemy + bx], 500 	      ;


    mov     eax, 0
    mov     word ax, [Lab2]

   cmp	   ax, 96		    ;   if eax == 0
   je	   cycle55_end		       ;   jump to cycle_end:

    add     word [Lab2],12

   jmp	   cycle55		      ;

   cycle55_end:

ret


I_END:	; конец программы



;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  Xpl:	     dd 32  ; координаты героя
  Ypl:	     dd 448
  hiro_lab:  dd 0 ; проекция героя в лабиринт
  numer_level: dd 0
  delta_level: dd 0
  delta_level_end: dd 256
  demon: dd 0
  demon_kolvo: dd 1

  gold: dd 0

  X_enemy:    dd 0
  Y_enemy:    dd 0


  X_bul:    dd 0
  Y_bul:    dd 0

  X_anime:    dd 0
  Y_anime:    dd 0

  mov_bullet:	 dw 0
  lab_bullet:	dw 0
  mov_enemy:	dw 0
  lab_enemy:	dw 0
  life_enemy:	 dw 0

  anime : dd 0 ; переменная для анимации героя и пр
  anime2 : dd 0 ; переменная для анимации героя и пр

  temp1: dd 0
  temp2: dd 0
  temp3: dd 0
  temp4: dd 0
  temp5: dd 0


  v_mov:     dd 0  ; направления движения персонажа
  v_mov2:     dd 0  ; направления движения персонажа
  v_fire:     dd 0  ; направления движения персонажа
  Lab:	     dd 0
  Lab2:      dw 0
  Lab3:      dw 0
  perem1:    dd 0
  perem2:    dd 0

  timer1 dd 0 ; таймер используемый для анимации воды, врагов   --- бегает от 0 до 10

;-------------------------------------------------------------------------------
;1 обычная стена, 2 непробиваемая, 4 вода, 8 лесница, 12 алмаз, 0 пустота    16 - exit
;-------------------------------------------------------------------------------
Dangeon:
	  db  2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2
	  db  2,1,1,1,1,1,1,0,12,0,0,0,0,0,0,2
	  db  2,0,12,12,0,0,0,8,1,8,1,1,1,0,0,2
	  db  2,8,1,1,1,1,1,8,1,8,0,0,0,0,0,2
	  db  2,8,0,0,0,0,0,8,1,8,2,4,4,2,0,2
	  db  2,8,2,4,4,4,2,8,1,8,2,2,2,2,0,2
	  db  2,0,2,2,2,2,2,8,1,8,0,0,0,0,0,2
	  db  2,0,1,0,0,0,0,8,1,1,1,1,1,1,8,2
	  db  2,0,1,1,8,1,1,1,0,0,0,0,0,0,8,2
	  db  2,12,1,1,8,0,12,0,1,8,1,1,8,1,1,2
	  db  2,0,0,0,8,8,1,8,0,8,0,0,0,0,0,2
	  db  2,12,1,8,1,8,1,1,8,1,1,1,1,1,1,2
	  db  2,0,1,8,12,12,0,0,0,0,0,0,0,0,0,2
	  db  2,0,1,1,1,1,1,8,1,1,1,0,0,0,0,2
	  db  2,0,0,0,12,0,0,8,0,0,0,16,0,12,12,2
	  db  2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2

Dangeon2:
	  db  2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2
	  db  2,1,1,0,1,1,1,0,12,0,0,0,0,0,0,2
	  db  2,0,12,12,0,0,0,8,1,8,1,1,1,0,0,2
	  db  2,8,1,1,1,1,1,8,1,8,0,0,0,0,0,2
	  db  2,8,0,0,0,0,0,8,1,8,2,4,4,2,0,2		      ;0
	  db  2,8,2,4,4,4,2,8,1,8,2,2,2,2,0,2
	  db  2,0,2,2,2,2,2,8,1,8,0,0,0,0,0,2
	  db  2,0,1,0,0,0,0,8,1,1,1,1,1,1,8,2
	  db  2,0,1,1,8,1,1,1,0,0,0,0,0,0,8,2
	  db  2,12,1,1,8,0,12,0,1,8,1,1,8,1,1,2
	  db  2,0,0,0,8,8,1,8,0,8,0,0,0,0,0,2
	  db  2,12,1,8,1,8,1,1,8,1,1,1,1,1,1,2
	  db  2,0,1,8,12,12,0,0,0,0,0,0,0,0,0,2
	  db  2,0,1,1,1,1,1,8,1,1,1,0,0,0,0,2
	  db  2,0,0,0,12,0,0,8,0,0,0,16,0,12,12,2
	  db  2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2
	  db  2,0,0,0,0,0,0,0,0,0,0,16,0,0,0,2
	  db  2,12,1,8,1,0,0,0,0,0,0,1,8,0,0,2
	  db  2,0,1,8,1,0,0,0,0,0,0,1,0,8,0,2		      ;1
	  db  2,12,1,8,1,1,0,0,0,0,1,1,0,8,0,2
	  db  2,0,1,8,1,0,12,12,12,12,0,1,8,8,0,2
	  db  2,12,1,8,1,0,1,1,1,1,0,1,8,0,0,2
	  db  2,0,1,8,0,0,0,0,0,0,0,0,8,0,0,2
	  db  2,0,1,1,1,1,8,1,8,1,1,1,1,1,8,2
	  db  2,0,0,0,12,0,8,0,8,0,0,0,12,0,8,2
	  db  2,0,1,1,1,1,1,0,1,8,1,1,8,1,1,2
	  db  2,0,0,0,0,0,0,0,0,8,1,0,8,0,0,2
	  db  2,0,1,1,1,1,1,8,1,1,1,1,1,1,8,2
	  db  2,0,0,12,0,0,0,8,0,0,12,0,0,0,8,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2
	  db  2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2
	  db  2,0,0,0,0,0,0,0,0,12,0,12,0,0,0,2
	  db  2,0,0,0,0,0,0,8,1,1,1,1,1,1,0,2		     ;2
	  db  2,0,8,0,12,12,0,8,0,0,0,0,0,2,4,2
	  db  2,0,8,1,1,1,1,0,0,0,0,0,0,2,2,2
	  db  2,0,8,0,0,0,0,12,1,1,1,1,0,0,0,2
	  db  2,0,8,1,0,1,0,12,0,0,0,0,0,0,0,2
	  db  2,0,8,0,1,1,12,12,0,0,0,8,1,1,1,2
	  db  2,0,8,0,12,0,0,12,0,0,0,8,0,12,0,2
	  db  2,0,8,0,0,0,8,1,8,0,0,1,8,1,8,2
	  db  2,0,8,0,0,8,1,1,1,8,0,0,8,0,8,2
	  db  2,0,1,1,1,1,1,1,1,1,1,1,1,1,8,2
	  db  2,0,0,0,0,0,0,0,16,0,0,0,0,0,8,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,0,0,0,0,12,0,0,0,0,0,0,16,0,0,2
	  db  2,0,8,1,1,1,1,1,1,1,1,1,1,1,0,2
	  db  2,0,8,0,0,0,0,0,12,0,0,0,0,0,0,2
	  db  2,0,8,0,0,0,0,1,8,1,0,0,0,0,0,2
	  db  2,0,8,0,0,0,1,1,8,1,1,0,0,0,0,2
	  db  2,0,8,0,12,1,1,1,8,1,1,1,12,0,0,2
	  db  2,0,8,0,1,1,12,1,8,1,12,1,1,0,0,2
	  db  2,0,0,1,1,1,1,1,8,1,1,1,1,0,0,2		       ;3
	  db  2,0,1,0,0,0,0,0,8,1,0,0,0,1,0,2
	  db  2,0,1,0,1,1,1,1,8,1,1,1,0,1,0,2
	  db  2,0,1,1,12,1,12,1,8,1,1,1,0,1,0,2
	  db  2,0,1,1,1,1,1,1,8,1,1,1,0,1,0,2
	  db  2,0,1,1,12,1,12,1,8,1,1,1,12,1,0,2
	  db  2,0,0,0,0,0,0,0,8,0,0,0,0,0,0,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,0,12,0,0,0,0,0,0,0,0,0,16,0,0,2
	  db  2,0,1,8,1,1,1,1,1,1,8,1,1,1,8,2
	  db  2,0,0,8,0,0,0,0,0,0,8,0,0,0,8,2
	  db  2,0,1,1,1,1,1,8,1,1,1,1,1,1,0,2
	  db  2,0,0,12,0,0,12,8,0,0,12,0,0,0,0,2
	  db  2,12,1,1,1,8,1,1,1,1,8,1,1,1,8,2
	  db  2,0,0,0,0,8,0,0,0,0,8,0,0,0,8,2
	  db  2,8,1,1,1,1,1,1,8,1,1,1,1,1,0,2			 ;4
	  db  2,8,0,0,0,12,0,0,8,0,0,12,0,0,0,2
	  db  2,0,1,8,1,1,1,1,1,1,1,1,8,1,0,2
	  db  2,0,0,8,12,0,0,0,0,12,0,0,8,0,0,2
	  db  2,8,1,1,1,1,1,1,8,1,1,1,1,1,8,2
	  db  2,8,12,0,0,0,0,0,0,0,0,0,0,12,8,2
	  db  2,8,2,4,4,4,4,4,4,4,4,4,4,2,8,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,8,1,8,12,8,1,8,1,8,1,8,1,8,1,2
	  db  2,8,2,8,1,8,1,8,1,8,1,8,12,8,12,2
	  db  2,8,1,8,1,8,2,8,12,8,1,8,1,8,1,2
	  db  2,8,12,8,0,8,0,8,0,8,0,8,0,8,0,2
	  db  2,1,8,1,1,1,1,1,8,1,1,1,1,1,8,2
	  db  2,0,8,0,0,0,0,0,8,0,0,12,0,0,8,2
	  db  2,0,8,12,0,2,12,0,8,1,1,1,1,1,8,2
	  db  2,0,1,1,8,1,1,1,8,0,0,0,0,0,8,2			;5
	  db  2,0,1,0,8,0,0,0,8,0,0,12,0,0,8,2
	  db  2,0,1,0,8,0,0,0,8,1,1,1,1,1,8,2
	  db  2,0,1,12,8,0,0,2,8,0,0,0,0,0,8,2
	  db  2,8,1,1,1,1,8,1,1,1,1,8,1,1,1,2
	  db  2,8,0,0,0,0,8,2,0,0,2,8,0,0,0,2
	  db  2,0,8,0,12,8,8,2,4,4,2,8,0,0,16,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,0,8,0,8,0,8,0,8,0,0,8,0,8,16,2
	  db  2,1,8,1,8,1,8,1,8,1,1,8,1,8,1,2
	  db  2,0,8,0,8,0,8,0,8,8,0,8,0,8,0,2
	  db  2,1,1,8,1,1,1,1,1,8,1,1,8,1,1,2
	  db  2,0,0,8,8,0,12,8,0,8,8,12,8,8,0,2
	  db  2,1,1,1,8,1,1,8,1,1,8,1,1,8,1,2
	  db  2,0,8,0,8,0,12,8,0,0,8,0,12,8,0,2 		 ;6
	  db  2,1,8,1,1,1,1,1,8,1,1,1,1,1,1,2
	  db  2,0,8,0,12,8,0,0,8,12,0,8,0,0,0,2
	  db  2,1,8,1,1,8,1,1,8,1,1,8,1,1,1,2
	  db  2,0,0,8,0,8,8,0,8,8,0,8,8,0,0,2
	  db  2,1,1,8,1,1,8,1,1,8,1,1,8,1,1,2
	  db  2,0,0,8,12,0,8,0,12,8,0,12,8,12,12,2
	  db  2,8,8,1,1,1,1,1,1,1,1,1,1,1,1,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,0,12,0,12,0,12,12,0,12,0,12,0,0,0,2
	  db  2,1,1,1,8,1,1,1,1,1,1,8,1,1,1,2
	  db  2,0,0,0,8,0,0,12,0,0,0,8,0,0,0,2
	  db  2,1,1,1,1,1,1,1,8,1,1,1,1,1,1,2
	  db  2,0,0,0,12,0,0,0,8,12,0,0,0,0,0,2
	  db  2,1,1,1,8,1,1,1,1,1,1,8,1,1,1,2
	  db  2,0,0,0,8,0,0,0,0,0,0,8,0,0,0,2			 ; 7
	  db  2,1,1,1,1,1,1,1,1,1,1,1,1,8,1,2
	  db  2,0,0,0,0,0,0,0,0,0,0,0,0,8,0,2
	  db  2,1,1,1,1,1,1,8,1,1,1,1,1,8,1,2
	  db  2,0,0,0,0,0,0,8,0,0,0,0,0,8,0,2
	  db  2,1,1,8,1,8,1,1,8,1,8,1,1,1,1,2
	  db  2,0,0,8,0,8,12,0,8,0,8,12,0,0,16,2
	  db  2,8,1,1,1,1,1,1,1,1,1,1,1,1,1,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,0,0,12,0,0,0,0,0,0,0,0,0,16,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,8,0,8,0,12,0,0,8,0,8,0,0,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,0,8,12,0,0,8,0,0,8,0,0,0,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,0,0,0,0,0,0,0,8,0,0,0,12,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,8,0,0,0,8,12,0,0,8,0,8,12,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,0,8,0,0,0,0,12,8,0,12,8,0,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,0,8,0,8,12,0,0,8,12,0,8,12,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
	  db  2,0,0,16,0,0,0,0,0,16,0,0,0,16,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,8,8,8,8,8,12,8,8,8,8,8,8,8,8,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,8,8,8,12,8,8,8,8,8,8,8,8,0,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,12,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,8,8,8,8,8,12,8,8,8,8,8,12,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,8,0,8,8,8,8,8,12,8,0,12,8,8,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,0,8,8,8,8,12,0,0,8,12,0,8,12,0,2
	  db  2,8,8,8,8,8,8,8,8,8,8,8,8,8,8,2
	  db  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
Bullet: dw 0,0,0,0,0,0
	dw 0,0,0,0,0,0
	dw 0,0,0,0,0,0
	dw 0,0,0,0,0,0
;-------------------------------------------------------------------------------


Enemy:	dw  1,448,128,0,0,500	;враги класс,Х,У,положение в лабиринте,направление движения,жизнь
	dw  0,32,32,0,0,0
	dw  0,64,32,0,0,0
	dw  0,416,32,0,0,0
	dw  0,256,256,0,0,0
	dw  0,256,224,0,0,0
	dw  0,320,192,0,0,0
	dw  0,32,256,0,0,0
	dw  0,448,448,0,0,0

;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if lang eq ru
title db 'ПОДЗЕМЕЛЬЕ для КолибриОС <RU095@YANDEX.RU> z/x стрелять',0
else
title db 'Dungeon for KolibriOS <RU095@YANDEX.RU> z/x - shoot',0
endf

bullet_bmp:
file 'bullet.bmp'
rb 1

level_bmp:
file 'level2.bmp'
rb 1

kirpich:
file 'kirpich.bmp'
rb 1

kirpich3:
file 'kirpich2.bmp'
rb 1

kirpich2:
file 'oldkirpich.bmp'
rb 1

lest_bmp:
file 'lestnica.bmp'
rb 1

water1_bmp:
file 'water1.bmp'
rb 1

water2_bmp:
file 'water2.bmp'
rb 1

door1_bmp:
file 'door1.bmp'
rb 1

door2_bmp:
file 'door2.bmp'
rb 1


img_level:
rb 448*448*3+8

;img_ded1:
;rb 448*448*3+8

img_area:
rb 32*32*3+8

img_area2:
rb 32*32*3+8

img_area3:
rb 32*32*3+8

img_bullet:
rb 4*8*3+8

img_lestniza:
rb 32*32*3+8

img_water1:
rb 32*32*3+8

img_water2:
rb 32*32*3+8

img_door1:
rb 32*32*3+8

img_door2:
rb 32*32*3+8

img_enemy1:
rb 32*32*3+8

img_enemy2:
rb 32*32*3+8


Music_bah:
  db  0x0f, 0x09, 0

Music_mov:
  db  0x00, 0x00, 0

Music_almaz:
  db  0x09, 0x03, 0

entered    rd 1

imgcount  rd 1	     ; кол-во всего кадров в анимации
img_index  rd 1      ; номер текущего кадра

;hashtable rd 4096    ; рабочая область для РеадГИФ процедуры
workarea:
file 'aniall.gif'

;temp rb 0x100000 ; место для ГИФ фаила
rb 10

canvas:
xs dd 512
ys dd 512
   rb 512*512*3

gif_hash_area:
rd 4096+1

coords:
rd 2

Image:
