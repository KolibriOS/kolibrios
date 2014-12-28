;заголовок приложения

SK_SPACE equ 0x39
SK_CTRL  equ 0x1D
SK_UP	 equ 0x48
SK_DOWN  equ 0x50
SK_LEFT  equ 0x4B
SK_RIGHT equ 0x4D
SK_ESC	 equ 0x01


INIT_PLANE_X equ 20
INIT_PLANE_Y equ 20
BACK_COLOR equ 0x9bcdfa
GROUND_COLOR equ 0x808080
BANG_COLOR equ 0xB0140E
ANIM_PLANE_BMP_PIXELS_COUNT equ 22*32*32
WINDOW_WIDTH equ 800
WINDOW_HEIGHT equ 600
GROUNG_HEIGHT equ 100
CITY_OFFSET equ (WINDOW_WIDTH-32*20)/2
MAX_BOMBS equ 8
GRAVY_ACCEL equ 1
BOMB_DELAY equ 12
MAX_LEVEL equ 5
ACKACK_BULLET_SPEED equ 17
BULLET_SIZE equ 7
MAX_SOUNDS equ 8

VOLUME_BOMBFLY equ 16
VOLUME_BOMBBANG equ 28
VOLUME_ACKACK equ 24
VOLUME_PLANE equ 48
VOLUME_ARW equ 16


use32			   ; транслятор, использующий 32 разрядных команды
    org 0x0		   ; базовый адрес кода, всегда 0x0
    db 'MENUET01'	; идентификатор исполняемого файла (8 байт)
    dd 0x1	     ; версия формата заголовка исполняемого файла
    dd start	     ; адрес, на который система передаёт управление
		       ; после загрузки приложения в память
    dd i_end	    ; размер приложения
    dd i_end  ; Объем используемой памяти, для стека отведем 0х100 байт
    dd i_end  ; расположим позицию стека в области памяти, сразу за телом программы. Вершина стека в диапазоне памяти, указанном выше
    dd 0x0, _s_current_dir		  ; указатель на строку с параметрами.
		 ;    если после запуска неравно нулю, приложение было
		 ;    запущено с параметрами из командной строки

		 ;    указатель на строку, в которую записан путь,
		 ;    откуда запущено приложение
;Область кода

;----------------- include -------------------------------------
include 'lang.inc'
include 'ssmix.inc'

;---------------------------------------------------------------------------
;----------------------------- главный цикл --------------------------------
;---------------------------------------------------------------------------

align 4
start:		   ;Точка входа в программу

     ; режим клавиатуры: скан-коды
     mov  eax, 66
     mov  ebx, 1
     mov  ecx, 1
     int  0x40

     ; инициализация кучи
     mov  eax, 68
     mov  ebx, 11
     int  0x40
     cmp  eax, 1000000	; куча нужна минимум на ~1000кб (это с запасом)
     ja   heap_ok
     mov  eax, -1
     int  0x40	       ; иначе закрываем все
   heap_ok:

     mov    eax, bmp_plane
     mov    ebx, 736*32*3
     call   convert_bmp_backcolor
     mov    eax, bmp_bomb
     mov    ebx, 96*16*3
     call   convert_bmp_backcolor
     mov    eax, bmp_tile
     mov    ebx, 20*20*3
     call   convert_bmp_backcolor
     mov    eax, bmp_ackack
     mov    ebx, 40*20*3
     call   convert_bmp_backcolor

     call  read_sounds_file
     call  ssmix_init

     mov  [_game_state], 0

redraw_window:
     call draw_window
     cmp  [_game_state], 0
     jne   still_no_intro
     call draw_intro
  still_no_intro:

still:
     ; обработка событий
     mov  eax,11
     int  0x40
     cmp  eax,1
     je   redraw_window
     cmp  eax,2
     je   key
     cmp  eax,3
     je   close_button

     ; пауза
     mov  eax,5
     mov  ebx,1
     int  0x40

     mov  eax, 26
     mov  ebx, 9
     int  0x40
     mov  ebx, eax
     sub  eax, [_last_value_timecount]
     cmp  eax, [_delay]
     jbe  still
     mov  [_last_value_timecount], ebx

     call timer_x2
     call timer_x4

     ; проверка на игру
     cmp  [_game_state], 1
     jne  still_no_game
     ; вызов процедур игрового процесса
     mov  eax, [_plane_health]
     cmp  eax, 0
     jle  still_no_plane
     call flight_control_plane
     call plane_interaction
     call draw_plane
   still_no_plane:
     call draw_plane_health
     call bomb_proc
     call bang_proc
     call ground_draw
     call city_draw
     call ackack_draw
     call ackack_bullet_proc
     ; таймер завершения игры
     call  game_over_timer
   still_no_game:

     jmp  still


key:			    ; обработка нажатия клавиш
    mov  eax,2
    int  0x40

    push eax
  key_SPACE:
    cmp  ah, SK_SPACE
    jne  key_SPACE_end
    cmp  [_game_state], 1
    je	 key_SPACE_do_a_barrel_roll
  key_SPACE_start_game: 	      ; запуск игры
    mov  [_game_state], 1
    call init_game_state
    jmp  key_SPACE_end
  key_SPACE_do_a_barrel_roll:	      ; сделать бочку
    mov  eax, [_plane_state]
    cmp  eax, 0
    je	 key_SPACE_do_a_barrel_roll_r
    cmp  eax, 1
    je	 key_SPACE_do_a_barrel_roll_l
    jmp  key_SPACE_end
  key_SPACE_do_a_barrel_roll_r:
    mov  eax,[_X_plane]
    cmp  eax, WINDOW_WIDTH-130-32
    jg	 key_SPACE_end
    mov  [_plane_state], 4
    mov  [_plane_state_step], 0
    jmp  key_SPACE_end
  key_SPACE_do_a_barrel_roll_l:
    mov  eax,[_X_plane]
    cmp  eax, 130
    jl	 key_SPACE_end
    mov  [_plane_state], 5
    mov  [_plane_state_step], 0
  key_SPACE_end:
    pop  eax

  key_CTRL:
    cmp  ah, SK_CTRL
    jne  key_CTRL_end
    call bomb_drop
  key_CTRL_end:

  key_UP:
    cmp  ah, SK_UP
    jne  key_UP_end
    cmp  [_game_state], 0
    jne  key_UP_end
    mov  ebx, [_level_num]
    inc  ebx
    mov  [_level_num], ebx
    cmp  ebx, MAX_LEVEL
    jbe  key_UP_no_max
    mov  [_level_num], MAX_LEVEL
  key_UP_no_max:
    call draw_level_num
  key_UP_end:

  key_DOWN:
    cmp  ah, SK_DOWN
    jne  key_DOWN_end
    cmp  [_game_state], 0
    jne  key_DOWN_end
    mov  ebx, [_level_num]
    dec  ebx
    mov  [_level_num], ebx
    cmp  ebx, 1
    jae  key_DOWN_no_min
    mov  [_level_num], 1
  key_DOWN_no_min:
    call draw_level_num
  key_DOWN_end:


  key_LEFT:
    cmp  ah, SK_LEFT
    jne  key_LEFT_end
    cmp  [_game_state], 0
    jne  key_LEFT_end
    mov  ebx, [_set_volume]
    dec  ebx
    cmp  ebx, 0
    jge  @f
    mov  ebx, 0
  @@:
    mov  [_set_volume], ebx
    shl  ebx, 4
    stdcall ssmix_setvolume, ebx, ebx
    call draw_volume_num
  key_LEFT_end:


  key_RIGHT:
    cmp  ah, SK_RIGHT
    jne  key_RIGHT_end
    cmp  [_game_state], 0
    jne  key_RIGHT_end
    mov  ebx, [_set_volume]
    inc  ebx
    cmp  ebx, 8
    jle  @f
    mov  ebx, 8
  @@:
    mov  [_set_volume], ebx
    shl  ebx, 4
    stdcall ssmix_setvolume, ebx, ebx
    call draw_volume_num
  key_RIGHT_end:


  key_ESC:
    cmp  ah, SK_ESC
    jne  key_ESC_end
    jmp  close_button
  key_ESC_end:

    jmp  still




close_button:		    ; завершение работы

     call  ssmix_release
     call  sounds_free_memory
     ; закрыть приложение
     mov  eax, -1
     int  0x40


;---------------------------------------------------------------------------
;----------------------------- подрограммы ---------------------------------
;---------------------------------------------------------------------------


convert_bmp_backcolor:	    ; eax - указаталь на начало данных, ebx - размер данных

     push  ecx
   cbb_loop:
     cmp   ebx, 0
     jle   cbb_loop_end
     mov   ecx, [eax]
     and   ecx, 0x00FFFFFF
     cmp   ecx, 0x00FFFFFF
     jne   @f
     mov   ecx, BACK_COLOR
     mov   [eax], cl
     shr   ecx, 8
     mov   [eax+1], cl
     shr   ecx, 8
     mov   [eax+2], cl
   @@:
     add   eax, 3
     sub   ebx, 3
     jmp   cbb_loop
   cbb_loop_end:
     pop   ecx

     ret

; чтение файла со звуками
read_sounds_file:

     mov  edi, _s_current_dir
     mov  al, 0
     mov  ecx, 4096
     repne scasb
     mov  dword[edi-1], 'data'
     mov  dword[edi+3], '.bin'
     mov  [_flag_bomberdatabin_ok], 1
     ; прочитать из файла данных количество звуков
     mov  [_fi_func], 0
     mov  [_fi_pos], 8
     mov  [_fi_size], 4
     mov  [_fi_pbuff], _count_sounds
     mov  [_fi_pfilename], _s_current_dir
     mov  eax, 70
     mov  ebx, file_info
     int  0x40
     cmp  ebx, -1
     jne  @f
     mov  [_flag_bomberdatabin_ok], 0
     jmp  .end
   @@:
     ; прочитать данные о смещении и размере звуков в файле, и записать их (временно) в массив звуков
     mov  ecx, [_count_sounds]
     shl  ecx, 3
     mov  [_fi_func], 0
     mov  [_fi_pos], 12
     mov  [_fi_size], ecx
     mov  [_fi_pbuff], _array_sounds
     mov  [_fi_pfilename], _s_current_dir
     mov  eax, 70
     mov  ebx, file_info
     int  0x40
     ; прочитать звуки в файле и записать их в память
     mov  ecx, [_count_sounds]
     mov  esi, _array_sounds
   .loop:
     push  ecx
     ; подготовить данные информационной структуры для чтения из файла
     mov  [_fi_func], 0
     mov  eax, [esi]
     mov  [_fi_pos], eax
     mov  eax, [esi+4]
     mov  [_fi_size], eax
     ; выделить память под звук, получить указатель, и изменить данные в массиве звуков
     mov  ecx, [esi+4]
     mov  eax, 68
     mov  ebx, 12
     int  0x40
     mov  [esi], eax
     mov  ecx, [esi+4]
     add  ecx, eax
     mov  [esi+4], ecx
     ; прочитать сэмпл из файла
     mov  [_fi_pbuff], eax
     mov  [_fi_pfilename], _s_current_dir
     mov  eax, 70
     mov  ebx, file_info
     int  0x40
     pop   ecx
     add  esi, 8
     dec  ecx
     jnz  .loop
   .end:

     ret


sounds_free_memory:

     mov  ecx, [_count_sounds]
     mov  esi, _array_sounds
   .loop:
     push  ecx
     ; выделить память под звук, получить указатель, и изменить данные в массиве звуков
     mov  eax, 68
     mov  ebx, 13
     mov  ecx, [esi]
     int  0x40
     pop   ecx
     add  esi, 8
     dec  ecx
     jnz  .loop


     ret


init_game_state:		     ; инициализация игровых данных
    mov  [_game_over_time], 0
    mov  [_X_plane], INIT_PLANE_X
    mov  [_Y_plane], INIT_PLANE_Y
    mov  [_VX_plane], 4
    mov  [_plane_state], 0
    mov  [_anim_pos_plane], 0
    mov  [_plane_health], 100
    mov  [_timer_x2], 0
    mov  [_timer_x4], 0
    mov  [_bomb_count], 0
    mov  [_bomb_delay_time], 0
    mov  [_addit_VY_plane], 0
    mov  [_flag_air_raid_warning], 0

    call load_level

    ; инициализация массива взрывов
    mov  ebx, _bang_array
    xor  ecx, ecx
  igs_bang_array_loop:
    cmp  ecx, MAX_BOMBS
    jae  igs_bang_array_loop_end
    mov  eax, -1
    mov  [ebx], eax
    add  ebx, 8
    inc  ecx
    jmp  igs_bang_array_loop
 igs_bang_array_loop_end:
    ; инициализация массива зенитных снарядов
    mov  ebx, _ackack_bullet
    xor  ecx, ecx
  igs_bullet_array_loop:
    cmp  ecx, 32
    jae  igs_bullet_array_loop_end
    mov  eax, 0
    mov  [ebx], eax
    add  ebx, 4
    inc  ecx
    jmp  igs_bullet_array_loop
 igs_bullet_array_loop_end:

    ; очистить экран
    call draw_window

    ;запустить звук пропелера
    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    stdcall  ssmix_playtrack, [_array_sounds+8*((( 4 ))-1)], [_array_sounds+8*((( 4 ))-1)+4], VOLUME_PLANE, VOLUME_PLANE, SSMIX_CHANMODE_REPEAT
  @@:
    mov  [_channel_sound_plane], eax

    mov  [_channel_sound_arw], -1

    ret



game_over:

    mov   [_game_over_time], 30   ; задаем задержку на game over
    ;вывод текста GAME OVER
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2 - 50)*65536 + 30
    mov  ecx,0x80000000
    mov  edx,_text_game_over
    int  0x40

    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    stdcall  ssmix_stoptrack, [_channel_sound_plane]
  @@:

    ret


game_over_timer:	       ; таймер завершения игры, создает небольшую паузу перед остановкой игры

    mov   ebx, [_game_over_time]
    cmp   ebx, 0
    je	  got_end
    dec   ebx
    mov   [_game_over_time], ebx
    cmp   ebx, 0
    jg	  got_end
    mov   [_game_state], 0
    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    call  ssmix_stopalltracks
  @@:
    call  draw_window
    call  draw_intro
  got_end:

    ret


next_level:		       ; следующий уровень

    push  eax
    mov   [_game_over_time], 1	 ; быстренько завершим игру
    mov   eax, [_level_num]	 ; на следующий уровень
    inc   eax
    cmp   eax, MAX_LEVEL
    jbe   next_level_no_overmax
    mov   eax, MAX_LEVEL
  next_level_no_overmax:
    mov   [_level_num], eax
    pop   eax

    ret



check_level_complete:		; проверка на завершение уровня

    push  eax
    push  ebx
    push  ecx
    push  edx

    mov  edx, 0        ; флаг наличия строений в городе: 0 - нет строений, 1 - есть строения
    mov  ebx, _city
    xor  ecx, ecx
  clc_loop:
    cmp  ecx, 32
    jae  clc_loop_end
    xor  eax, eax
    mov  ax, [ebx]
    cmp  eax, 0
    je	 clc_no_buildings
    mov  edx, 1
    jmp  clc_loop_end
  clc_no_buildings:
    add  ebx, 2
    inc  ecx
    jmp  clc_loop
  clc_loop_end :
    cmp  edx, 0
    jne  clc_end
    ; уровень завершен - идем на посадку
    ;вывод текста LEVEL COMPLETE
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2 - 70)*65536 + 30
    mov  ecx,0x80000000
    mov  edx,_text_level_complete
    int  0x40
    ; быстрое снижение самолета
    mov  [_addit_VY_plane], 3	   ; задать скорость быстрого снижения
    ; выключить сирену
    call air_raid_warning_off
  clc_end:

    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret


draw_plane_health:

   mov	ebx, 5*0x00010000 + 100
   mov	ecx, 5*0x00010000 + 10
   mov	edx, 0x00000000
   mov	eax, 13
   int	0x40

   mov	ebx, [_plane_health]
   add	ebx, 5*0x00010000
   mov	edx, 0x00008000
   int	0x40

   ret


air_raid_warning_on:
   push eax
   cmp	[_flag_air_raid_warning], 1
   je	.end
   mov	[_flag_air_raid_warning], 1
   cmp	[_flag_bomberdatabin_ok], 1
   jne	@f
   stdcall  ssmix_playtrack, [_array_sounds+8*((( 5 ))-1)], [_array_sounds+8*((( 5 ))-1)+4], VOLUME_ARW, VOLUME_ARW, SSMIX_CHANMODE_REPEAT
 @@:
   mov	[_channel_sound_arw], eax
 .end:
   pop	eax
   ret


air_raid_warning_off:
   cmp	[_flag_air_raid_warning], 0
   je	.end
   mov	[_flag_air_raid_warning], 0
   cmp	[_flag_bomberdatabin_ok], 1
   jne	.end
   stdcall  ssmix_stoptrack, [_channel_sound_arw]
 .end:
   ret


;======================================= САМОЛЕТ ======================================


flight_control_plane:  ; процедура управления полетом самолета

    mov  eax,[_X_plane]
    add  eax, [_VX_plane]
    mov  [_X_plane], eax

    cmp  eax, WINDOW_WIDTH-60-32
    jl	 fcp_set_utunr_rl
    mov  ebx, [_plane_state]
    cmp  ebx, 0
    jne  fcp_set_utunr_rl
    mov  [_plane_state],2
    mov  [_plane_state_step], 0
  fcp_set_utunr_rl:

    cmp  eax, 60
    jg	 fcp_set_utunr_lr
    mov  ebx, [_plane_state]
    cmp  ebx, 1
    jne  fcp_set_utunr_lr
    mov  [_plane_state],3
    mov  [_plane_state_step], 0
  fcp_set_utunr_lr:

  ; далее код выполняется в 2 раза медленней
    mov eax, [_timer_x2]
    cmp eax, 0
    jne fcp_timex2_end

    mov  eax, [_plane_state]
    cmp  eax, 2 		   ; если делаем разворот налево
    jne  fcp_uturn_rl_end
    mov  eax, [_plane_state_step]
    mov  ebx, 4
    sub  ebx, eax
    mov  [_VX_plane], ebx	    ; то расчитываем скорость исходя из текущего шага состояния
    inc  eax			    ; переходим на следующий шаг состояния
    mov  [_plane_state_step], eax
    cmp  eax, 9 		    ; проверяем, закончились ли шаги
    jne  fcp_uturn_rl_end
    mov  [_plane_state], 1	    ; если да то переводим состояние на прямой полет
  fcp_uturn_rl_end:


    mov  eax, [_plane_state]
    cmp  eax, 3 		   ; если делаем разворот направо
    jne  fcp_uturn_lr_end
    mov  eax, [_plane_state_step]
    mov  ebx, -4
    add  ebx, eax
    mov  [_VX_plane], ebx	    ; то расчитываем скорость исходя из текущего шага состояния
    inc  eax			    ; переходим на следующий шаг состояния
    mov  [_plane_state_step], eax
    cmp  eax, 9 		    ; проверяем, закончились ли шаги
    jne  fcp_uturn_lr_end
    mov  [_plane_state], 0	    ; если да то переводим состояние на прямой полет
  fcp_uturn_lr_end:

    mov  eax, [_plane_state]
    cmp  eax, 4 		    ; если делаем бочку вправо
    jne  fcp_barrel_r_end
    mov  eax, [_plane_state_step]
    inc  eax			    ; переходим на следующий шаг состояния
    mov  [_plane_state_step], eax
    cmp  eax, 8 		    ; проверяем, закончились ли шаги
    jne  fcp_barrel_r_end
    mov  [_plane_state], 0	    ; если да то переводим состояние на прямой полет
  fcp_barrel_r_end :

    mov  eax, [_plane_state]
    cmp  eax, 5 		    ; если делаем бочку влево
    jne  fcp_barrel_l_end
    mov  eax, [_plane_state_step]
    inc  eax			    ; переходим на следующий шаг состояния
    mov  [_plane_state_step], eax
    cmp  eax, 8 		    ; проверяем, закончились ли шаги
    jne  fcp_barrel_l_end
    mov  [_plane_state], 1	    ; если да то переводим состояние на прямой полет
  fcp_barrel_l_end:

    call animation_plane

  fcp_timex2_end:

    ; далее код выполняется в 4 раза медленней
    mov eax, [_timer_x4]
    cmp eax, 0
    jne fcp_timex4_end

    ; медленное снижение самолета
    mov  eax,[_Y_plane]
    add  eax, 1
    mov  [_Y_plane], eax

  fcp_timex4_end:

    ; быстрое снижение самолета

    mov  eax,[_Y_plane]
    mov  ebx,[_addit_VY_plane]
    add  eax, ebx
    mov  [_Y_plane], eax

    ; контроль высоты самолета, посадка
    mov  eax,[_Y_plane]
    cmp  eax,  WINDOW_HEIGHT - GROUNG_HEIGHT - 32
    jl	 fcp_end_plane_landing
    mov  [_addit_VY_plane], 0	  ; прекратить быстрое снижение
    mov  ebx, [_plane_state]
    cmp  ebx, 6
    je	fcp_end_plane_landing
    cmp  ebx, 0
    jne  fcp_horiz_fly
    mov  [_plane_state], 6	  ; режим посадки
    ;jmp  fcp_end_plane_landing
  fcp_horiz_fly:
    mov  eax,  WINDOW_HEIGHT - GROUNG_HEIGHT - 32
  fcp_end_plane_landing:
    cmp  eax, WINDOW_HEIGHT - GROUNG_HEIGHT - 32 + 6
    jle  fcp_no_plane_underground	  ; проверка на то, чтобы самолет не ушел "под землю"
    mov  eax, WINDOW_HEIGHT - GROUNG_HEIGHT - 32 + 6
    mov  ebx, [_X_plane]
    cmp  ebx, WINDOW_WIDTH - 60-32
    jl	 fcp_no_end_level		    ; проверка на то, что докатился до конца посадочной полосы
    ; завершение уровня
    call  next_level			     ; переход на следующий уровень
  fcp_no_end_level:
  fcp_no_plane_underground:
    mov  [_Y_plane], eax

    ret



animation_plane:

    ; анимация самолета

    ; полет вправо
    mov  eax, [_plane_state]
    cmp  eax, 0
    jne  ap_state0_end
    mov  [_anim_pos_plane], 0
  ap_state0_end:

    ; полет влево
    mov  eax, [_plane_state]
    cmp  eax, 1
    jne  ap_state1_end
    mov  [_anim_pos_plane], 8
  ap_state1_end:

    ; анимация для разворотов и бочек
    mov  eax, [_plane_state]
    cmp  eax, 5
    ja	 ap_state2345_end
    cmp  eax, 2
    jb	 ap_state2345_end
    sub  eax, 2
    mov  ebx, eax	   ; умножить eax на 9
    shl  eax, 3
    add  eax, ebx
    add  eax, [_plane_state_step]    ; прибавить номер шага
    shl  eax, 2 		     ; умножить на 4
    add  eax, _anim_array_uturn_rl   ; получается смещение для номера требуемого кадра
    mov  ebx, [eax]
    mov  [_anim_pos_plane], ebx      ; здесь вытаскиваем сам номер
  ap_state2345_end:

    ; посадка (только вправо)
    mov  eax, [_plane_state]
    cmp  eax, 6
    jne  ap_state6_end
    mov  [_anim_pos_plane], 22
  ap_state6_end:

     ret



draw_plane:

    ; стереть предыдущее изображение
    mov  eax, 13
    mov  ebx,[_last_XY_plane]
    and  ebx, 0xffff0000
    add  ebx, 32
    mov  ecx,[_last_XY_plane]
    shl  ecx, 16
    add  ecx, 32
    mov  edx, BACK_COLOR
    int  0x40

    ; расчет адреса картинки исходя из номера кадра _anim_pos_plane
    mov  ebx, [_anim_pos_plane]
    shl  ebx, 10
    mov  eax, ebx
    add  ebx, eax
    add  ebx, eax
    add  ebx, bmp_plane

    ; подготовка координат
    mov  edx,[_X_plane]
    shl  edx, 16
    add  edx,[_Y_plane]
    mov  [_last_XY_plane], edx

    ; вывод картинки
    mov  eax,7
    mov  ecx,32*65536+32
    int  0x40

    ret


plane_interaction:

    push  eax
    push  ebx
    push  ecx
    push  edx

    mov   eax, [_X_plane]
    mov   ebx, [_Y_plane]
    add   ebx, 16
    mov   ecx, [_VX_plane]
    cmp   ecx, 0
    jl	  pi_no_positive_vx
    add   eax, 32		; здесь в зависимости от направления полета определяем точку взаимодействия с городом
   pi_no_positive_vx:
    ; теперь в eax и ebx координаты X,Y  точки взаимодействия самолета с городом
    mov   edx, eax
    sub   eax, CITY_OFFSET
    ; проверка на вылет за пределы города
    cmp   eax, 0
    jl	  pi_out_of_city
    cmp   eax, 32*20
    jg	  pi_out_of_city
    ; проверка на взаимодействие с городом
    add   eax, 10	 ; делим координату X на 20
    mov   ecx, eax
    shr   eax, 4
    shr   ecx, 6
    sub   eax, ecx
    shr   ecx, 2
    add   eax, ecx
    shr   ecx, 2
    sub   eax, ecx
    dec   eax	      ; в eax номер столбца города по которому летит бомба
    mov   esi, eax
    shl   esi, 1
    add   esi, _city	; в esi указатель на высоту постройки в данном столбце
    mov   eax, edx
    ; теперь сравним координату Y вершины постройки с координатой Y точки взаимодействия самолета
    xor   ecx, ecx
    mov   cx, [esi]
    mov   edx, ecx
    shl   ecx, 4
    shl   edx, 2
    add   ecx, edx
    mov   edx, WINDOW_HEIGHT - GROUNG_HEIGHT
    sub   edx, ecx	       ; в edx координата Y вершины постройки
    cmp   ebx, edx
    jl	  pi_no_crash
    ; если столкновение произошло
    mov   [_plane_health], 0
    call  bang_add	    ; делаем взрыв
    call  game_over	    ; завершение игры
  pi_no_crash:

  pi_out_of_city:

    pop   edx
    pop   ecx
    pop   ebx
    pop   eax

    ret


; ================================== БОМБЫ ===========================================

bomb_drop:	    ; процедура "бросания бомбы". создает новую бомбу, задает для нее начальные параметры.
    ; проверка на здоровье самолета
    mov  eax, [_plane_health]
    cmp  eax, 0
    jle  bomb_drop_end
    ; проверить на допустимые состояния самолета (нельзя во время бочки)
    mov  eax, [_plane_state]
    cmp  eax, 3
    ja	 bomb_drop_end
    ; проверить выдержан ли интервал времени между бросками
    mov  eax, [_bomb_delay_time]
    cmp  eax, 0
    jne  bomb_drop_end
    mov  [_bomb_delay_time], BOMB_DELAY
    ; собственно, бросаем бомбу
    mov  eax, [_bomb_count]
    cmp  eax, MAX_BOMBS-1
    jae  bomb_drop_end
    inc  eax
    mov  [_bomb_count], eax
    dec  eax
    shl  eax, 4
    add  eax, _bomb_array    ; здесь в еах получили указатель на начало данных о бомбе

    mov  ebx, [_X_plane]
    mov  [eax], ebx	       ; задание координаты X
    add  eax, 4
    mov  ebx, [_Y_plane]
    add  ebx, 30
    mov  [eax], ebx	       ; задание координаты Y

    add  eax, 4
    mov  ebx, [_VX_plane]
    cmp  ebx, 0
    jge  bomb_drop_pos_dir_vx
  bomb_drop_neg_dir_vx:
    neg  ebx
    shr  ebx, 1
    neg  ebx
    jmp  bomb_drop_dir_vx_end
  bomb_drop_pos_dir_vx:
    shr  ebx, 1
  bomb_drop_dir_vx_end:
    mov  [eax], ebx	       ; задание вектора скорости по X

    add  eax, 4
    mov  ebx, 0
    mov  [eax], ebx	       ; задание вектора скорости по Y

    push  ecx
    mov   ecx, [_bomb_count]
    dec   ecx
    call  bombfly_sound_start
    pop   ecx

  bomb_drop_end:

    ret


bomb_proc:
    ; таймер интервала между бросаниями бомб
    mov  eax, [_bomb_delay_time]
    cmp  eax, 0
    je	 bomb_proc_delay_timer_end
    dec  eax
    mov  [_bomb_delay_time], eax
  bomb_proc_delay_timer_end:
    ; обработка движения бомб
    mov  eax, [_bomb_count]
    cmp  eax, 0
    je	 bomb_proc_end
    xor  ecx, ecx
  bomb_proc_loop:		 ; цикл обработки падения бомб
    cmp  ecx, [_bomb_count]
    jae  bomb_proc_end
    mov  ebx, ecx
    shl  ebx, 4
    add  ebx,  _bomb_array	    ;  получили указатель на начало данных о бомбе

    call bomb_hide   ; стереть предыдущее значение

    ; обработка

    ; выполняется пересчет для скорости VY текущей бомбы

    add  ebx, 4*3
    mov  eax, [_timer_x2]
    cmp  eax, 0
    jne  bomb_proc_gravity_accel_end
    mov  eax, [ebx]
    add  eax, GRAVY_ACCEL
    mov  [ebx], eax
  bomb_proc_gravity_accel_end:
    ; выполняется пересчет для координат X текущей бомбы
    sub  ebx, 4*3
    mov  eax, [ebx]
    add  ebx, 4*2
    mov  edx, [ebx]
    add  eax, edx
    sub  ebx, 4*2
    mov  [ebx], eax
    push eax
    ; выполняется пересчет для координат Y текущей бомбы
    add  ebx, 4
    mov  eax, [ebx]
    add  ebx, 4*2
    mov  edx, [ebx]
    add  eax, edx
    sub  ebx, 4*2
    mov  [ebx], eax
    push eax

    ; взаимодействие бомб с миром
    sub  ebx, 4
    pop  edx	  ; координата Y
    pop  eax	  ; координата X
    ; проверка на выход бомб за пределы игровой зоны, бомбы просто удаляются при вылете за границы
    cmp  eax, 10
    jle   bomb_proc_delete_bomb
    cmp  eax, WINDOW_WIDTH - 36
    jge   bomb_proc_delete_bomb
    ; проверка на взаимодейтсвие с миром
    push  eax
    push  ebx
    mov  ebx, edx
    call  bomb_check_detonation
    cmp   eax, 1
    pop   ebx
    pop   eax
    je	bomb_proc_interaction_with_world
    cmp  edx, WINDOW_HEIGHT - GROUNG_HEIGHT
    jae  bomb_proc_interaction_with_world
    jmp   bomb_proc_interaction_end
  bomb_proc_interaction_with_world:
    push  eax
    push  ebx
    mov  ebx, edx
    call bang_add
    call air_raid_warning_on   ; включение воздушной тревоги
    pop   ebx
    pop   eax
  bomb_proc_delete_bomb:
    call bomb_delete	      ; удаляем бомбу
    dec  ecx		      ; т.к. бомбы удалена - остаемся на том же номере бомбы
    mov  eax, [_bomb_count]
    dec  eax
    mov  [_bomb_count], eax   ; уменьшаем количество бомб
    jmp  bomb_proc_draw_end   ; не рисуем бомбу если удалили ее
  bomb_proc_interaction_end:
    call bomb_draw
  bomb_proc_draw_end:

    inc  ecx
    jmp  bomb_proc_loop
  bomb_proc_end:

    ret


bomb_delete:	   ; удаление данных о бомбе, в есх номер удаляемой бомбы
    push eax
    push ebx
    push ecx
    push edx
    call  bombfly_sound_stop
    inc  ecx
  bomb_delete_loop:
    cmp  ecx, [_bomb_count]
    jae  bomb_delete_loop_end
    mov  ebx, ecx
    shl  ebx, 4
    add  ebx,  _bomb_array	    ;  получили указатель на начало данных о бомбе
    dec  ecx
    mov  edx, ecx
    shl  edx, 4
    add  edx,  _bomb_array	    ;  получили указатель на начало данных о следующей бомбе
    inc  ecx
    ; перемещение данных о следующей бомбе на место предыдущей
    mov  eax, [ebx]
    mov  [edx], eax
    add  ebx,4
    add  edx,4
    mov  eax, [ebx]
    mov  [edx], eax
    add  ebx,4
    add  edx,4
    mov  eax, [ebx]
    mov  [edx], eax
    add  ebx,4
    add  edx,4
    mov  eax, [ebx]
    mov  [edx], eax
    ; переход к следующей бомбе
    inc  ecx
    jmp  bomb_delete_loop
  bomb_delete_loop_end:
    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret

bomb_hide:  ; стереть бомбу с экрана, ebx - указатель на начало данных о бомбе

    push eax
    push ebx
    push ecx
    push edx
    mov  eax, 13
    mov  edx, ebx
    mov  ebx, [edx]
    shl  ebx, 16
    add  ebx, 16
    add  edx, 4
    mov  ecx, [edx]
    shl  ecx, 16
    add  ecx, 16
    mov  edx, BACK_COLOR
    int  0x40
    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret


bomb_draw:	; отрисовать бомбу, ebx - указатель на начало данных о бомбе

    push eax
    push ebx
    push ecx
    push edx

    ; получение в edx координат бомбы
    mov  edx, [ebx]
    shl  edx, 16
    add  ebx, 4
    mov  eax, [ebx]
    add  edx, eax

    ; получение указателя на картинку бомбы в ebx (сразу с выбором нужной картинки)
    add  ebx, 4
    mov  eax, [ebx]	  ; здесь в eax - горизонтальная скорость бомбы
    add  ebx, 4
    mov  ecx, [ebx]	  ; здесь в ecx - вертикальная скорость бомбы
    xor  ebx, ebx
    cmp  ecx, 3
    jl	 bomb_draw_midspeed_end
    inc  ebx
  bomb_draw_midspeed_end:
    cmp  ecx, 9
    jl	 bomb_draw_highspeed_end
    inc  ebx
  bomb_draw_highspeed_end:
    cmp  eax, 0
    jge  bomb_draw_left_dir_end
    add  ebx, 3
  bomb_draw_left_dir_end:
       ; теперь в ebx номер картинки бомбы
    shl  ebx, 8
    mov  eax, ebx
    add  ebx, eax
    add  ebx, eax
    add  ebx, bmp_bomb	 ; теперь в ebx указатель на картинку бомбы

    ; вывод картинки
    mov  eax,7
    mov  ecx,16*65536+16
    int  0x40

    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret


; взаимодействие бомб с миром

bomb_check_detonation:	       ; проверить бомбу с координатами: eax - координата X, eab - координата Y
			       ; и при необходимости провести нужные разрушения (или пустой взрыв)
			       ; в eax вернуть результат события: 0 - ничего не произшло, 1 - произошел взрыв
			       ; (корявая реализация)

    push  ecx
    push  edx

    add   eax, 8
    add   ebx, 8

    sub   eax, CITY_OFFSET
    ; проверка на вылет за пределы города
    cmp   eax, 0
    jl	  bcd_out_of_city
    cmp   eax, 32*20
    jg	  bcd_out_of_city
    ; проверка на взаимодействие с городом
    add   eax, 10	 ; делим координату X на 20
    mov   ecx, eax
    shr   eax, 4
    shr   ecx, 6
    sub   eax, ecx
    shr   ecx, 2
    add   eax, ecx
    shr   ecx, 2
    sub   eax, ecx
    dec   eax	      ; в eax номер столбца города по которому летит бомба
    mov   esi, eax
    shl   esi, 1
    add   esi, _city	; в esi указатель на высоту постройки в данном столбце

    mov   edx, WINDOW_HEIGHT - GROUNG_HEIGHT
    sub   edx, ebx
    mov   ebx, edx
    cmp   ebx, 0
    jg	  bcd_no_neg_value_Y
    xor   ecx, ecx
    mov   cx, [esi]
    xor   ebx, ebx
    jmp   bcd_damage	; наносим разрушения
  bcd_no_neg_value_Y:
    add   ebx, 10	  ; делим координату Y на 20
    mov   ecx, ebx
    shr   ebx, 4
    shr   ecx, 6
    sub   ebx, ecx
    shr   ecx, 2
    add   ebx, ecx
    shr   ecx, 2
    sub   ebx, ecx	   ; в ebx высота  бомбы "в плитках" над поверхностью

    xor   ecx, ecx
    mov   cx, [esi]
    cmp   ebx, ecx	      ; проверка на попадание в строение
    jg	  bcd_no_detonation
  bcd_damage:	     ; разрушения
    ;разрушение столбика в который попала бомба
    mov   edx, ecx
    mov   ecx, ebx
    sub   ecx, 2
    cmp   ecx, 0	     ; просто проверка на 0, чтоб не ушло в минус
    jge   bcd_no_neg_value_H
    xor   ecx, ecx
   bcd_no_neg_value_H:
    mov   [esi], cx	    ;  возврат значения высоты столбца после взрыва
    ; зачистка взорванной области
    call  clear_tiles
    ; проверка на попадание в зенитки
    call ackack_check_bombing
    ; разрушение соседнего столбика слева
    ; на данный момент имеем: eax - номер центрального (центральный - это в который непосредственно попала бомба) столбика,
    ; ebx - высота попадания бомбы, ecx - новая высота центрального столбика, edx - прежняя высота центрального столбика,
    ; esi - указатель на значение высоты центрального столбика в массиве
    dec   eax
    cmp   eax, 0
    jl	  bcd_damage_left_end
    sub   esi, 2
    xor   ecx, ecx
    mov   cx, [esi]	; в ecx высота левого столбика
    mov   edx, ecx
    push  ecx
    sub   ecx, ebx
    inc   ecx
    cmp   ecx, 2
    pop   ecx
    ja	  bcd_damage_left_end	  ; условие  |ecx-ebx|<=1
    dec   ecx
    cmp   ecx, 0	     ; просто проверка на 0, чтоб не ушло в минус
    jge   bcd_no_neg_value_HL
    xor   ecx, ecx
   bcd_no_neg_value_HL:
    mov   [esi], cx
    call  clear_tiles
    ; проверка на попадание в зенитки
    call ackack_check_bombing
  bcd_damage_left_end:

    ; разрушение соседнего столбика справа
    ; на данный момент имеем: eax - номер левого  столбика,
    ; ebx - высота попадания бомбы, ecx - новая высота левого столбика, edx - прежняя высота левого столбика,
    ; esi - указатель на значение высоты левого столбика в массиве
    inc   eax
    inc   eax
    cmp   eax, 32
    jge   bcd_damage_right_end
    add   esi, 4
    xor   ecx, ecx
    mov   cx, [esi]	; в ecx высота правого столбика
    mov   edx, ecx
    push  ecx
    sub   ecx, ebx
    inc   ecx
    cmp   ecx, 2
    pop   ecx
    ja	  bcd_damage_right_end	   ; условие  |ecx-ebx|<=1
    dec   ecx
    cmp   ecx, 0	     ; просто проверка на 0, чтоб не ушло в минус
    jge   bcd_no_neg_value_HR
    xor   ecx, ecx
   bcd_no_neg_value_HR:
    mov   [esi], cx
    call  clear_tiles
    ; проверка на попадание в зенитки
    call ackack_check_bombing
  bcd_damage_right_end:
    call  check_level_complete
  bcd_detonation:
    mov   eax, 1
    jmp   bcd_end
  bcd_no_detonation:
    mov   eax, 0
    jmp   bcd_end
  bcd_out_of_city:
    mov   eax, 1
    cmp   ebx, WINDOW_HEIGHT - GROUNG_HEIGHT
    jge   bcd_end
    mov   eax, 0
  bcd_end:
    pop   edx
    pop   ecx

    ret


bombfly_sound_start:	; ecx - номер бомбы в массиве

    push eax ebx ecx
    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    stdcall  ssmix_playtrack, [_array_sounds+8*((( 1 ))-1)], [_array_sounds+8*((( 1 ))-1)+4], VOLUME_BOMBFLY, VOLUME_BOMBFLY, SSMIX_CHANMODE_SINGLE_WITHOUT_RESET
  @@:
    mov  ebx, ecx
    shl  ebx, 2
    add  ebx,  _array_bombsoundchannels
    mov  [ebx], eax
    pop  ecx ebx eax

    ret


bombfly_sound_stop:    ; ecx - номер бомбы в массиве

    push eax ebx ecx
    mov  ebx, ecx
    shl  ebx, 2
    add  ebx,  _array_bombsoundchannels
    mov  eax, [ebx]
    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    stdcall  ssmix_stoptrack, eax
  @@:
    inc  ecx
  .delete_loop:
    cmp  ecx, MAX_BOMBS
    jae  .delete_loop_end
    mov  ebx, ecx
    shl  ebx, 2
    add  ebx,  _array_bombsoundchannels
    mov  eax, [ebx]
    mov  [ebx-4], eax
    inc  ecx
    jmp  .delete_loop
  .delete_loop_end:
    pop  ecx ebx eax

    ret

; ==================================== ВЗРЫВЫ =======================================


fill_circle:
    ; построение заполненного круга    (алгоритм Брезенхема)
    ; входные параметры:
    ; eax - координата центра по X
    ; ebx - координата центра по Y
    ; ecx - радиус
    ; edx - цвет
    mov  edi, eax
    mov  esi, ebx
    push edx	     ; цвет в стек

    mov  eax, ecx
    shl  eax, 1
    mov  edx, 3
    sub  edx, eax	  ; D:=3-2*R
    xor  ebx, ebx   ; теперь в ebx будет коорд X, в ecx коорд Y, в edx переменная D, в eax - промежуточные данные

  fc_loop:
    cmp  ebx, ecx
    jg	 fc_loop_end
    ; отрисовка линиями
     pop  eax	    ; цвет из стека
     push edx	    ; D в стек
     mov  edx, eax  ; в edx - цвет
     push ebx
     push ecx
     ; рисуем первый отрезок
     mov  eax, edi
     sub  eax, ebx
     shl  eax, 16
     add  ebx, edi
     add  ebx, eax   ; получили концы отрезка по х
     add  ecx, esi
     mov  eax, ecx
     shl  eax, 16
     add  ecx, eax   ; получили концы отрезка по y
     mov  eax, 38
     int  0x40
     pop  ecx
     push ecx
     ; рисуем второй отрезок
     mov  eax, esi
     sub  eax, ecx
     mov  ecx, eax
     shl  eax, 16
     add  ecx, eax   ; получили концы отрезка по y
     mov  eax, 38
     int  0x40
     pop  ecx
     pop  ebx
     push ecx
     push ebx	; обратить внимание! порядок загрузки координат в стек поменялся!
     ; рисуем третий отрезок
     mov  eax, edi
     sub  eax, ecx
     shl  eax, 16
     add  ecx, edi
     add  ecx, eax
     mov  eax, ebx
     mov  ebx, ecx    ; получили концы отрезка по х
     mov  ecx, eax    ; внимание! в ecx - координата x
     add  ecx, esi
     mov  eax, ecx
     shl  eax, 16
     add  ecx, eax   ; получили концы отрезка по y
     mov  eax, 38
     int  0x40
     pop  ecx
     push ecx
     ; рисуем четвертый отрезок
     mov  eax, esi
     sub  eax, ecx
     mov  ecx, eax
     shl  eax, 16
     add  ecx, eax   ; получили концы отрезка по y
     mov  eax, 38
     int  0x40
     pop  ebx
     pop  ecx

     mov  eax, edx	 ; в eax - цвет
     pop  edx		 ; D из стека
     push eax		 ; цвет в стек
    ; переход в следующей точке
    cmp  edx, 0
    jge  fc_loop_D_more_0     ; если D<0
  fc_loop_D_low_0:	      ; то
    mov  eax, ebx
    shl  eax, 2
    add  edx, eax
    add  edx, 4        ; D:=D+4*X+6
    jmp  fc_loop_cmpD_end
  fc_loop_D_more_0:	      ; иначе
    mov  eax, ebx
    sub  eax, ecx
    shl  eax, 2
    add  edx, eax
    add  edx, 10      ; D:=D+4*(X-Y)+10
    dec  ecx
  fc_loop_cmpD_end:	      ; конец если
    inc  ebx
    jmp  fc_loop
  fc_loop_end:

    pop  edx	   ; цвет из стека

    ret




bang_add:   ; добавить взрыв от бомбы, eax - координата X, eab - координата Y

    push  eax
    push  ebx
    push  ecx
    push  edx

    shl  eax, 16
    add  eax, ebx
    mov  ebx, _bang_array
    xor  ecx, ecx
  bang_add_loop:
    cmp  ecx, MAX_BOMBS
    jae  bang_add_loop_end
    mov  edx, [ebx]
    cmp  edx, -1
    jne  bang_add_loop_freecheck_end
    mov  edx,  5*65536 + 3
    mov  [ebx], edx
    add  ebx, 4
    mov  [ebx], eax
    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    stdcall  ssmix_playtrack, [_array_sounds+8*((( 2 ))-1)], [_array_sounds+8*((( 2 ))-1)+4], VOLUME_BOMBBANG, VOLUME_BOMBBANG, SSMIX_CHANMODE_SINGLE
  @@:
    jmp  bang_add_loop_end
  bang_add_loop_freecheck_end:
    add  ebx, 8
    inc  ecx
    jmp  bang_add_loop
  bang_add_loop_end:


    pop   edx
    pop   ecx
    pop   ebx
    pop   eax

    ret


bang_proc:

    mov  ebx, _bang_array
    xor  ecx, ecx
  bang_proc_loop:
    cmp  ecx, MAX_BOMBS
    jae  bang_proc_loop_end
    ; проверка на наличие взрыва
    mov  edx, [ebx]
    cmp  edx, -1
    je	 bang_proc_loop_freecheck_end
    ; проверка на завершение взрыва
    and  edx, 0x0000FFFF
    cmp  edx, 0
    jne  bang_proc_loop_endcheck_end
    ; здесь пишем обработку завершения взрыва
    push  ebx
    push  ecx
    mov  ecx, [ebx]
    shr  ecx, 16
    add  ebx, 4
    mov  eax, [ebx]
    mov  ebx, eax
    shr  eax, 16
    and  ebx, 0x0000FFFF
    mov  edx, BACK_COLOR
    call fill_circle
    pop   ecx
    pop   ebx
    mov  eax, -1
    mov  [ebx], eax
    jmp  bang_proc_loop_continue
  bang_proc_loop_endcheck_end:
    ; здесь пишем обработку взрыва
    push  ebx
    push  ecx
    ; задаем радиус и считаем новый
    mov  eax, [ebx]
    mov  edx, eax
    shr  eax, 16
    mov  ecx, eax
    and  edx, 0x0000FFFF
    mov  eax, edx
    shl  eax, 3
    add  eax, ecx
    shl  eax, 16
    dec  edx
    add  eax, edx
    mov  [ebx], eax
    ; задаем координаты
    add  ebx, 4
    mov  eax, [ebx]
    mov  ebx, eax
    shr  eax, 16
    and  ebx, 0x0000FFFF
    ;задаем цвет
    mov  edx, BANG_COLOR
    call fill_circle
    pop   ecx
    pop   ebx
  bang_proc_loop_freecheck_end:
  bang_proc_loop_continue:
    add  ebx, 8
    inc  ecx
    jmp  bang_proc_loop
  bang_proc_loop_end:

    ret

; ===================================== ЗЕНИТКИ =========================================
; управление зенитками, добавление зениток, отрисовка зениток, стрельба


ackack_draw:			   ; отрисовка зениток

    mov  ebx, _ackack
    xor  ecx, ecx
    xor  esi, esi	; флаг наличия хотя бы одной зенитки (действующей или в перспективе)
  ackack_draw_loop:
    cmp  ecx, 32
    jae  ackack_draw_loop_end
    xor  eax, eax
    mov  ax, [ebx]
    cmp  eax, 1
    jne  ackack_draw_continue	   ; если значение текущей ячейки в массиве не 1 то не рисуем зенитку
    push  ebx
    push  ecx
    ; в ebx получаем указатель на высоту постройки
    mov   ebx, ecx
    shl   ebx, 1
    add   ebx, _city  ; получили указатель на высоту постройки
    xor  eax, eax
    mov  ax, [ebx]    ; получили высоту постройки
    cmp   eax, 0
    jle   ackack_draw_continue2      ; если постройки нет то не рисуем зенитку
    mov   esi, 1
    call  ackack_bullet_start
    ; отрисовка зенитки
    mov   ebx, bmp_ackack   ; получили указатель на картинку
    cmp   edx, 0
    je	  ackack_draw_no_start_bullet
    add   ebx, 20*20*3	    ; картинка со треляющей зениткой
  ackack_draw_no_start_bullet:
    ; формирование координат картинки  в edx
    mov   edx, ecx
    shl   edx, 4
    shl   ecx, 2
    add   edx, ecx
    add   edx, CITY_OFFSET
    shl   edx, 16
    mov   ecx, eax
    shl   eax, 4
    shl   ecx, 2
    add   eax, ecx
    mov   ecx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub   ecx, eax
    sub   ecx, 20  ; подняли координату на плитку выше постройки
    add   edx, ecx ; получили координаты
    mov   ecx, 20*0x00010000 + 20   ; задали размер картинки
    mov   eax, 7
    int   0x40		; выводим картинку
  ackack_draw_continue2:
    pop   ecx
    pop   ebx
  ackack_draw_continue:
    add  ebx, 2
    inc  ecx
    jmp  ackack_draw_loop
  ackack_draw_loop_end:

    cmp  esi, 0
    jne  @f
    ; выключить сирену
    call  air_raid_warning_off
  @@:

    ret


ackack_check_bombing: ; проверка на подрыв зениток бомбами : eax - номер столбца

    push eax
    push ebx
    push ecx
    push edx
    ; проверка на попадание в столбец зенитки
    mov  ebx, eax
    shl  ebx, 1
    push  ebx
    add  ebx, _ackack
    mov  edx, ebx	; запомним указатель состояния зенитки в edx
    xor  eax, eax
    mov  ax, [ebx]    ; в eax  получили состояние зенитки
    cmp  eax, 1
    jne  acb_no_bombing       ; проверка на наличие зенитки в данном столбце
    xor  eax, eax
    mov  [ebx], ax     ; убираем зенитку
    call ackack_next_wave
  acb_no_bombing:
    ; проверка на полное уничножение столбца
    pop  ebx
    add  ebx, _city
    xor  eax, eax
    mov  ax, [ebx]    ; в eax  получили состояние зенитки
    cmp  eax, 0
    jg	 acb_no_destroy_building       ; проверка на наличие постройки в столбце
    xor  eax, eax
    mov  [ebx], ax     ; убираем зенитку даже в перспективе
  acb_no_destroy_building:
    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret



ackack_next_wave:	 ; новая волна зениток

    push eax
    push ebx
    push ecx
    push edx

  anw_begin:
    mov  ebx, _ackack
    xor  ecx, ecx
    xor  edx, edx	; флаг контроля за зенитками
  anw_loop:
    cmp  ecx, 32
    jae  anw_loop_end
    xor  eax, eax
    mov  ax, [ebx]
    cmp  eax, 1
    jle  anw_continue	   ; если значение текущей ячейки <=1 то пропускаем
    dec  eax
    mov  [ebx], ax
    cmp  eax, 1
    jg	 anw_no_active_ackack	  ; если значение текущей ячейки в массиве >1 то пропускаем
    mov  edx, 1        ; показываем что есть действующая зенитка
    jmp  anw_continue
  anw_no_active_ackack:
    cmp  edx, 1
    je	 anw_continue	   ; если флага 1 то пропускаем
    mov  edx, 2
  anw_continue:
    add  ebx, 2
    inc  ecx
    jmp  anw_loop
  anw_loop_end:
    cmp  edx, 2
    je	 anw_begin

    pop  edx
    pop  ecx
    pop  ebx
    pop  eax
    ret





ackack_bullet_start:	 ; запуск снарядов  (входные: eax - высота столбца, ecx - номер столбца;
			 ;                   выходные: edx - флаг нового запуска (0-нет, 1 -да)

    push  eax
    push  ebx
    push  ecx

    cmp   [_flag_air_raid_warning], 1
    jne   abs_no_start_bullet

    ; в ebx получаем указатель на положение снаряда
    mov   edx, eax
    mov   ebx, ecx
    shl   ebx, 2
    add   ebx, _ackack_bullet  ; получили указатель на положение снаряда
    mov   eax, [ebx]	; получили положение снаряда
    cmp   eax, 0
    jne   abs_no_start_bullet	   ; если не 0, то не запускаем новый
    ; запуск снаряда
    mov   eax, edx    ; помещаем в eax высоту постройки
    ; считаем координату вершины
    mov   ecx, eax
    shl   eax, 4
    shl   ecx, 2
    add   eax, ecx
    mov   ecx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub   ecx, eax
    sub   ecx, 20+BULLET_SIZE  ; подняли координату на плитку выше постройки + высота снаяряда (в ecx - начальная координата снаряда)
    mov   [ebx], ecx  ; задали начальную координату снаряду

    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    stdcall  ssmix_playtrack, [_array_sounds+8*((( 3 ))-1)], [_array_sounds+8*((( 3 ))-1)+4], VOLUME_ACKACK,VOLUME_ACKACK , SSMIX_CHANMODE_SINGLE
  @@:

    mov   edx, 1
    jmp   abs_end_start_bullet
  abs_no_start_bullet:
    mov   edx, 0
  abs_end_start_bullet:

    pop   ecx
    pop   ebx
    pop   eax

    ret


ackack_bullet_proc:	; процедура обработки и отрисовки снарядов

    mov  ebx, _ackack_bullet
    xor  ecx, ecx
  abp_loop:
    cmp  ecx, 32
    jae  abp_loop_end
    mov  eax, [ebx]
    cmp  eax, 0
    je	 abp_continue
    jl	 abp_low_zero
    push  ebx
    push  ecx
    ; стирание старого изображения
    mov   ebx, ecx
    shl   ebx, 4
    shl   ecx, 2
    add   ebx, ecx
    add   ebx, CITY_OFFSET
    add   ebx, 7	     ; смещение снаряда
    mov   edx, ebx
    shl   ebx, 16
    add   ebx, edx   ; в ebx координаты начала и конца отрезка по оси X
    mov   ecx, eax
    mov   edx, ecx
    add   edx, BULLET_SIZE   ; прибавили длину снаряда
    shl   ecx, 16
    add   ecx, edx   ; в ebx координаты начала и конца отрезка по оси Y
    mov   edx, BACK_COLOR
    mov   eax, 38
    int   0x40
    add   ebx, 0x00050005
    int   0x40

    pop   ecx
    pop   ebx
    push  ebx
    push  ecx

    mov  eax, [ebx]
    sub  eax, ACKACK_BULLET_SPEED
    mov  [ebx], eax
    cmp  eax, 0
    jle  abp_end_draw_bullet

    call ackack_bullet_interaction_check
    cmp  edx, 0
    je	 abp_no_bullet_interaction	; если не было взаимодействие то рисуем снаряды
    xor  eax, eax
    mov  [ebx], eax
    jmp  abp_end_draw_bullet
  abp_no_bullet_interaction:

    ; рисуем снаряды
    mov   ebx, ecx
    shl   ebx, 4
    shl   ecx, 2
    add   ebx, ecx
    add   ebx, CITY_OFFSET
    add   ebx, 7	     ; смещение снаряда
    mov   edx, ebx
    shl   ebx, 16
    add   ebx, edx   ; в ebx координаты начала и конца отрезка по оси X
    mov   ecx, eax
    mov   edx, ecx
    add   edx, BULLET_SIZE   ; прибавили длину снаряда
    shl   ecx, 16
    add   ecx, edx   ; в ebx координаты начала и конца отрезка по оси Y
    mov   edx, 0x00000000
    mov   eax, 38
    int   0x40
    add   ebx, 0x00050005
    int   0x40
  abp_end_draw_bullet:

    pop   ecx
    pop   ebx
    jmp  abp_continue
  abp_low_zero:
    xor  eax, eax
    mov  [ebx], eax
  abp_continue:
    add  ebx, 4
    inc  ecx
    jmp  abp_loop
  abp_loop_end:

    ret


ackack_bullet_interaction_check:       ; проверка взаимодействия снарядов
				       ;  (входные: eax - координата снаряда по Y, ecx - номер столбца;
				       ;  выходные: edx - флаг попадания (0-нет, 1 -да))


    push  eax
    push  ebx
    push  ecx

    ; если самолет делает бочку то пропускаем
    mov   ebx, [_plane_health]
    cmp   ebx, 0
    jle   abic_with_plane_no
    mov   ebx, [_plane_state]
    cmp   ebx, 4
    je	  abic_with_plane_no
    cmp   ebx, 5
    je	  abic_with_plane_no
    ; проверка на координаты
    mov   ebx, [_Y_plane]
    cmp   eax, ebx
    jl	  abic_with_plane_no	 ; если сняряд выше координаты попадяния то пропускаем
    add   ebx, 20     ; в ebx - координата попадания в самолет
    cmp   eax, ebx
    jg	  abic_with_plane_no	 ; если сняряд ниже координаты попадяния то пропускаем
    ; считаем координату X вершины
    mov   edx, ecx
    shl   edx, 4
    shl   ecx, 2
    add   ecx, edx
    add   ecx, CITY_OFFSET
    add   ecx, 7    ; в ecx - координата X левого снаряда
    mov   ebx, [_X_plane]
    add   ebx, 32
    cmp   ecx, ebx
    jg	  abic_with_plane_no
    mov   ebx, [_X_plane]
    add   ecx, 5
    cmp   ecx, ebx
    jl	  abic_with_plane_no
    ; есть попадание!!!
    mov   ebx, [_plane_health]
    sub   ebx, 20
    mov   [_plane_health], ebx
    cmp   ebx, 0		; проверка на game over
    jg	  abic_no_plane_crash
    mov   [_plane_health], 0
    mov   eax, [_X_plane]
    mov   ebx, [_Y_plane]
    call  bang_add	    ; делаем взрыв
    call  game_over	    ; завершение игры
  abic_no_plane_crash:
    mov   edx, 1
    jmp   abic_with_plane_end
  abic_with_plane_no:
    mov   edx, 0
  abic_with_plane_end:

    pop   ecx
    pop   ebx
    pop   eax
    ret

; ===================================== МИР =========================================

ground_draw:

    mov  eax, 13
    mov  ebx, 0*0x00010000 + (WINDOW_WIDTH-10)
    mov  ecx, (WINDOW_HEIGHT-GROUNG_HEIGHT)*0x00010000 + (GROUNG_HEIGHT-5-20)
    mov  edx, GROUND_COLOR
    int  0x40

    mov  eax, 38
    mov  ebx, 0*0x00010000 + (WINDOW_WIDTH-10)
    mov  ecx, (WINDOW_HEIGHT-GROUNG_HEIGHT)*0x00010000 + (WINDOW_HEIGHT-GROUNG_HEIGHT)
    mov  edx, 0x000000
    int  0x40

    ret


city_draw:	      ; отрисовка города

    mov  ebx, _city
    xor  edx, edx
  city_draw_loop:
    cmp  edx, 32
    jae  city_draw_loop_end
    xor  eax, eax
    mov  ax, [ebx]
    push  ebx
    push  edx
    mov   ebx, bmp_tile
  city_draw_loop2:
    cmp  eax, 0
    jle  city_draw_loop2_end
    push  eax
    push  edx
    ; формирование координат картинки в edx
    mov   ecx, edx
    shl   edx, 4
    shl   ecx, 2
    add   edx, ecx
    add   edx, CITY_OFFSET
    shl   edx, 16
    mov   ecx, eax
    shl   eax, 4
    shl   ecx, 2
    add   eax, ecx
    mov   ecx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub   ecx, eax
    add   edx, ecx ; получили координаты
    mov   ecx, 20*0x00010000 + 20   ; задали размер картинки
    mov   eax, 7
    int   0x40		; выводим картинку
    pop   edx
    pop   eax
    dec  eax
    jmp  city_draw_loop2
  city_draw_loop2_end:

    pop   edx
    pop   ebx
    add  ebx, 2
    inc  edx
    jmp  city_draw_loop
  city_draw_loop_end:

    ret


clear_tiles:	   ; зачистка взорванных плиток: eax - номер столбца, ecx - нижняя плитка из удаляемых, edx - верхняя плитка из удаляемых

    push eax
    push ebx
    push ecx
    push edx
    ; формирование координаты X угла прямоугольника
    mov  ebx, eax
    shl  eax, 4
    shl  ebx, 2
    add  eax, ebx
    add  eax, CITY_OFFSET
    ;  формирование координаты Y1 нижнего края нижней плитки
    mov  ebx, ecx
    shl  ecx, 4
    shl  ebx, 2
    add  ecx, ebx
    mov  ebx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub  ebx, ecx
    mov  ecx, ebx
    add  ecx, 20
    ; формирование координаты Y2 верхнего края верхней плитки
    mov  ebx, edx
    shl  edx, 4
    shl  ebx, 2
    add  edx, ebx
    mov  ebx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub  ebx, edx
    mov  edx, ebx
    sub  edx, 20   ; чтоб и зенитки стирать, если они есть
    ; задание координаты прямоугольника по Y=Y2 и его высоты H=Y1-Y2
    sub  ecx, edx
    shl  edx, 16
    add  ecx, edx
    ; задание координаты прямоугольника по X и его ширины 20
    shl  eax, 16
    add  eax, 20
    mov  ebx, eax
    ; отрисовка прямоугольника цвета фона
    mov  eax, 13
    mov  edx, BACK_COLOR
    int  0x40

    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret


load_level:	   ; загрузка уровня, в eax номер загружаемого уровня

    mov  eax, [_level_num]
    dec  eax
    shl  eax, 7 	    ; умножить на размер данных для одного уровня (128 байт)
    mov  ebx, levels
    add  ebx, eax	    ; в ebx указатель на начало загружаемого уровня
    mov  edx, cur_level
    mov  ecx, 0
  load_level_loop:
    cmp  ecx, 128/4
    jae  load_level_loop_end
    mov  eax, [ebx]
    mov  [edx], eax
    add  ebx, 4
    add  edx, 4
    inc  ecx
    jmp  load_level_loop
  load_level_loop_end:

  load_level_end:

    ret

; =================================== РАЗНОЕ ========================================

timer_x4:
    mov  eax, [_timer_x4]
    inc  eax
    mov  [_timer_x4],eax
    cmp  eax, 4
    ja	 timer_x4_do
    jmp  timer_x4_end
  timer_x4_do:
    mov  [_timer_x4],0
  timer_x4_end:
    ret


timer_x2:
    mov  eax, [_timer_x2]
    inc  eax
    mov  [_timer_x2],eax
    cmp  eax, 2
    ja	 timer_x2_do
    jmp  timer_x2_end
  timer_x2_do:
    mov  [_timer_x2],0
  timer_x2_end:
    ret

draw_window:			     ; отрисовка окна

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  eax,0
    mov  ebx,100*65536+WINDOW_WIDTH
    mov  ecx,100*65536+WINDOW_HEIGHT
    mov  edx,0x34000000+BACK_COLOR
    mov  edi,_window_caption
    int  0x40

    mov  eax,12
    mov  ebx,2
    int  0x40

    ret



draw_intro:

    ;заголовок
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-80)*65536 + 60
    mov  ecx,0x80000000
    mov  edx,_text_intro_title
    int  0x40

    ;описание игры
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-200)*65536 + 120
    mov  ecx,0x80000000
    mov  edx,_text_intro_description1
    int  0x40


    ;клавиши управления
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-60)*65536 + 200
    mov  ecx,0x80000000
    mov  edx,_text_intro_key_controls
    int  0x40

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-70)*65536 + 230
    mov  ecx,0x80000000
    mov  edx,_text_intro_key_ctrl
    int  0x40

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-70)*65536 + 245
    mov  ecx,0x80000000
    mov  edx,_text_intro_key_space
    int  0x40

    ; выбор уровня

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-175)*65536 + 300
    mov  ecx,0x80000000
    mov  edx,_text_intro_select_level
    int  0x40

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-25)*65536 + 320
    mov  ecx,0x80000000
    mov  edx,_text_intro_level_num
    int  0x40

    call draw_level_num

    ; установка громкости

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-175)*65536 + 350
    mov  ecx,0x80000000
    mov  edx,_text_intro_set_volume
    int  0x40

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-25)*65536 + 370
    mov  ecx,0x80000000
    mov  edx,_text_intro_volume_num
    int  0x40
    call draw_volume_num

    ; запуск игры и выход

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-80)*65536 + 400
    mov  ecx,0x80FF0000
    mov  edx,_text_intro_start_space
    int  0x40

    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-30)*65536 + 460
    mov  ecx,0x80000000
    mov  edx,_text_intro_exit_key
    int  0x40


   ret


draw_level_num:

    mov  eax, 13
    mov  ebx, (WINDOW_WIDTH/2+25)*0x00010000 + 30
    mov  ecx, 320*0x00010000 + 15
    mov  edx, BACK_COLOR
    int  0x40

    mov  eax, 47
    mov  ebx, 0x80020000
    mov  ecx, [_level_num]
    mov  edx, (WINDOW_WIDTH/2+25)*65536 + 320
    mov  esi,0x00000000
    int  0x40

    ret

draw_volume_num:

    mov  eax, 13
    mov  ebx, (WINDOW_WIDTH/2+25)*0x00010000 + 30
    mov  ecx, 370*0x00010000 + 15
    mov  edx, BACK_COLOR
    int  0x40

    mov  eax, 47
    mov  ebx, 0x80020000
    mov  ecx, [_set_volume]
    mov  edx, (WINDOW_WIDTH/2+25)*65536 + 370
    mov  esi,0x00000000
    int  0x40

    ret

;---------------------------------------------------------------------------
;-------------------------------- данные -----------------------------------
;---------------------------------------------------------------------------

_game_state dd ?
_delay dd 6
_timer_x2 dd ?
_timer_x4 dd ?
_game_over_time  dd ?
_flag_air_raid_warning dd ?
_flag_bomberdatabin_ok dd ?
_last_value_timecount dd ?

_X_plane dd ?
_Y_plane dd ?
_last_XY_plane dd ?
_VX_plane dd ?
_addit_VY_plane dd ?
_plane_state dd ?  ; 0 - полет направо, 1- полет налево, 2-разворот справа-налево, 3-разворот слева-направо, 4 - бочка вправо, 5 - бочка влево
_plane_state_step dd ?	 ; номер шага текущего состояния (нужно для синхнопизации с анимацией)
_plane_health dd ?   ; здоровье самолета

_bomb_count dd ?
_bomb_array rd 4*MAX_BOMBS
_bomb_delay_time dd ?	; задержка времени на сброс бомбы

_bang_array rd 2*MAX_BOMBS

_anim_pos_plane dd ?
; последовательности картинок анимации
_anim_array_uturn_rl dd 0,1,2,3,4,5,6,7,8
_anim_array_uturn_lr dd 8,7,6,9,10,11,2,1,0
_anim_array_barrel_r dd 0,1,2,12,13,14,15,16,0
_anim_array_barrel_l dd 8,17,18,19,20,21,6,7,8


cur_level:
; массив, в котором храниться текущее состояние города. изначально в него загрузаются уровни
_city rw 32
; массив, в котором храниться текущее состояние зениток. изначально в него загрузаются уровни
_ackack rw 32
; массив, в котором хранятся значения текущих координа снарядов от зениток
_ackack_bullet rd 32

; номер текущего уровня
_level_num dd 1
; задание уровней: 32 числа означают высоту строений (высота "в квадратах") для _level?_city
;                  и порядок появления зениток для _level?_ackack
levels:
_level1_city dw    0, 0, 0, 6, 5, 6, 5, 6, 3, 3, 3, 3, 3, 6, 7, 8,   8, 7, 6, 3, 3, 3, 3, 3, 6, 5, 6, 5, 6, 0, 0, 0
_level1_ackack	dw 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

_level2_city dw    0, 0, 0, 5, 5, 2, 5, 7, 7, 2, 7, 9, 9, 9, 2, 9,   9, 2, 9, 9, 9, 7, 2, 7, 7, 5, 2, 5, 5, 0, 0, 0
_level2_ackack	dw 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 5, 0,   0, 6, 0, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 0, 0

_level3_city dw    0, 0, 0,12,12,12, 9, 9, 9,12,12,12, 9, 7, 5, 3,   3, 5, 7, 9,12,12,12, 9, 9, 9,12,12,12, 0, 0, 0
_level3_ackack	dw 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 4, 0, 3, 0, 2, 1,   1, 2, 0, 3, 0, 5, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0

_level4_city dw    0, 0, 0,12,12,12, 3,12,12,12, 3, 1, 1, 6,12,18,  18,12, 6, 1, 1, 3,12,12,12, 3,12,12,12, 0, 0, 0
_level4_ackack	dw 0, 0, 0, 0, 5, 0, 1, 0, 0, 0, 1, 0, 3, 0, 0, 2,   6, 0, 0, 3, 0, 1, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0

_level5_city dw    0, 5,10,10,10,10,15,15,15,15,17,17,17,17, 1,15,  15, 1,17,17,17,17,15,15,15,15,10,10,10,10, 5, 0
_level5_ackack	dw 0, 8, 0, 9, 9, 0, 3, 3, 3, 3, 0, 0, 0, 0, 5, 1,   2, 5, 0, 0, 0, 0, 4, 4, 4, 4, 0,10,10, 0, 7, 0

; звуки
_count_sounds dd ?
_array_sounds rd 2*MAX_SOUNDS

_channel_sound_plane dd ?    ; номер канала звука пропелера самолета
_channel_sound_arw dd ?      ; номер канала звука сирены
_array_bombsoundchannels rd MAX_BOMBS	 ; массив номеров звуковых каналов для бомб

_set_volume dd 8    ; громкость 0..8

; разное

_s_current_dir rb 4096	 ; путь к исполняемому файлу

file_info:	      ; информационная структура для работы с файлами
  _fi_func dd ?
  _fi_pos dd ?
  dd 0
  _fi_size dd ?
  _fi_pbuff dd ?
  db 0
  _fi_pfilename dd ?

if lang eq ru
	include 'ruslang.inc'
else
	include 'englang.inc'
end if

bmp_plane:
file "plane.bmp":54

bmp_bomb:
file "bomb.bmp":54

bmp_tile:
file "tile.bmp":54

bmp_ackack:
file "ackack.bmp":54


align 16
rb 0x100 ; резерв памяти для стека.

i_end:

