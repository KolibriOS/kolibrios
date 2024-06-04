;��������� ����������

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


use32			   ; ����������, ������������ 32 ��������� �������
    org 0x0		   ; ������� ����� ����, ������ 0x0
    db 'MENUET01'	; ������������� ������������ ����� (8 ����)
    dd 0x1	     ; ������ ������� ��������� ������������ �����
    dd start	     ; �����, �� ������� ������� ������� ����������
		       ; ����� �������� ���������� � ������
    dd i_end	    ; ������ ����������
    dd i_end  ; ����� ������������ ������, ��� ����� ������� 0�100 ����
    dd i_end  ; ���������� ������� ����� � ������� ������, ����� �� ����� ���������. ������� ����� � ��������� ������, ��������� ����
    dd 0x0, _s_current_dir		  ; ��������� �� ������ � �����������.
		 ;    ���� ����� ������� ������� ����, ���������� ����
		 ;    �������� � ����������� �� ��������� ������

		 ;    ��������� �� ������, � ������� ������� ����,
		 ;    ������ �������� ����������
;������� ����

;----------------- include -------------------------------------
include 'lang.inc' ; Language support for locales: ru_RU (CP866), en_US.
include 'ssmix.inc'

;---------------------------------------------------------------------------
;----------------------------- ������� ���� --------------------------------
;---------------------------------------------------------------------------

align 4
start:		   ;����� ����� � ���������

     ; ����� ����������: ����-����
     mov  eax, 66
     mov  ebx, 1
     mov  ecx, 1
     int  0x40

     ; ������������� ����
     mov  eax, 68
     mov  ebx, 11
     int  0x40
     cmp  eax, 1000000	; ���� ����� ������� �� ~1000�� (��� � �������)
     ja   heap_ok
     mov  eax, -1
     int  0x40	       ; ����� ��������� ���
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
     ; ��������� �������
     mov  eax,11
     int  0x40
     cmp  eax,1
     je   redraw_window
     cmp  eax,2
     je   key
     cmp  eax,3
     je   close_button

     ; �����
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

     ; �������� �� ����
     cmp  [_game_state], 1
     jne  still_no_game
     ; ����� �������� �������� ��������
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
     ; ������ ���������� ����
     call  game_over_timer
   still_no_game:

     jmp  still


key:			    ; ��������� ������� ������
    mov  eax,2
    int  0x40

    push eax
  key_SPACE:
    cmp  ah, SK_SPACE
    jne  key_SPACE_end
    cmp  [_game_state], 1
    je	 key_SPACE_do_a_barrel_roll
  key_SPACE_start_game: 	      ; ������ ����
    mov  [_game_state], 1
    call init_game_state
    jmp  key_SPACE_end
  key_SPACE_do_a_barrel_roll:	      ; ������� �����
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




close_button:		    ; ���������� ������

     call  ssmix_release
     call  sounds_free_memory
     ; ������� ����������
     mov  eax, -1
     int  0x40


;---------------------------------------------------------------------------
;----------------------------- ����������� ---------------------------------
;---------------------------------------------------------------------------


convert_bmp_backcolor:	    ; eax - ��������� �� ������ ������, ebx - ������ ������

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

; ������ ����� �� �������
read_sounds_file:

     mov  edi, _s_current_dir
     mov  al, 0
     mov  ecx, 4096
     repne scasb
     mov  dword[edi-1], 'data'
     mov  dword[edi+3], '.bin'
     mov  [_flag_bomberdatabin_ok], 1
     ; ��������� �� ����� ������ ���������� ������
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
     ; ��������� ������ � �������� � ������� ������ � �����, � �������� �� (��������) � ������ ������
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
     ; ��������� ����� � ����� � �������� �� � ������
     mov  ecx, [_count_sounds]
     mov  esi, _array_sounds
   .loop:
     push  ecx
     ; ����������� ������ �������������� ��������� ��� ������ �� �����
     mov  [_fi_func], 0
     mov  eax, [esi]
     mov  [_fi_pos], eax
     mov  eax, [esi+4]
     mov  [_fi_size], eax
     ; �������� ������ ��� ����, �������� ���������, � �������� ������ � ������� ������
     mov  ecx, [esi+4]
     mov  eax, 68
     mov  ebx, 12
     int  0x40
     mov  [esi], eax
     mov  ecx, [esi+4]
     add  ecx, eax
     mov  [esi+4], ecx
     ; ��������� ����� �� �����
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
     ; �������� ������ ��� ����, �������� ���������, � �������� ������ � ������� ������
     mov  eax, 68
     mov  ebx, 13
     mov  ecx, [esi]
     int  0x40
     pop   ecx
     add  esi, 8
     dec  ecx
     jnz  .loop


     ret


init_game_state:		     ; ������������� ������� ������
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

    ; ������������� ������� �������
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
    ; ������������� ������� �������� ��������
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

    ; �������� �����
    call draw_window

    ;��������� ���� ���������
    cmp  [_flag_bomberdatabin_ok], 1
    jne  @f
    stdcall  ssmix_playtrack, [_array_sounds+8*((( 4 ))-1)], [_array_sounds+8*((( 4 ))-1)+4], VOLUME_PLANE, VOLUME_PLANE, SSMIX_CHANMODE_REPEAT
  @@:
    mov  [_channel_sound_plane], eax

    mov  [_channel_sound_arw], -1

    ret



game_over:

    mov   [_game_over_time], 30   ; ������ �������� �� game over
    ;����� ������ GAME OVER
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


game_over_timer:	       ; ������ ���������� ����, ������� ��������� ����� ����� ���������� ����

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


next_level:		       ; ��������� �������

    push  eax
    mov   [_game_over_time], 1	 ; ���������� �������� ����
    mov   eax, [_level_num]	 ; �� ��������� �������
    inc   eax
    cmp   eax, MAX_LEVEL
    jbe   next_level_no_overmax
    mov   eax, MAX_LEVEL
  next_level_no_overmax:
    mov   [_level_num], eax
    pop   eax

    ret



check_level_complete:		; �������� �� ���������� ������

    push  eax
    push  ebx
    push  ecx
    push  edx

    mov  edx, 0        ; ���� ������� �������� � ������: 0 - ��� ��������, 1 - ���� ��������
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
    ; ������� �������� - ���� �� �������
    ;����� ������ LEVEL COMPLETE
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2 - 70)*65536 + 30
    mov  ecx,0x80000000
    mov  edx,_text_level_complete
    int  0x40
    ; ������� �������� ��������
    mov  [_addit_VY_plane], 3	   ; ������ �������� �������� ��������
    ; ��������� ������
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


;======================================= ������� ======================================


flight_control_plane:  ; ��������� ���������� ������� ��������

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

  ; ����� ��� ����������� � 2 ���� ���������
    mov eax, [_timer_x2]
    cmp eax, 0
    jne fcp_timex2_end

    mov  eax, [_plane_state]
    cmp  eax, 2 		   ; ���� ������ �������� ������
    jne  fcp_uturn_rl_end
    mov  eax, [_plane_state_step]
    mov  ebx, 4
    sub  ebx, eax
    mov  [_VX_plane], ebx	    ; �� ����������� �������� ������ �� �������� ���� ���������
    inc  eax			    ; ��������� �� ��������� ��� ���������
    mov  [_plane_state_step], eax
    cmp  eax, 9 		    ; ���������, ����������� �� ����
    jne  fcp_uturn_rl_end
    mov  [_plane_state], 1	    ; ���� �� �� ��������� ��������� �� ������ �����
  fcp_uturn_rl_end:


    mov  eax, [_plane_state]
    cmp  eax, 3 		   ; ���� ������ �������� �������
    jne  fcp_uturn_lr_end
    mov  eax, [_plane_state_step]
    mov  ebx, -4
    add  ebx, eax
    mov  [_VX_plane], ebx	    ; �� ����������� �������� ������ �� �������� ���� ���������
    inc  eax			    ; ��������� �� ��������� ��� ���������
    mov  [_plane_state_step], eax
    cmp  eax, 9 		    ; ���������, ����������� �� ����
    jne  fcp_uturn_lr_end
    mov  [_plane_state], 0	    ; ���� �� �� ��������� ��������� �� ������ �����
  fcp_uturn_lr_end:

    mov  eax, [_plane_state]
    cmp  eax, 4 		    ; ���� ������ ����� ������
    jne  fcp_barrel_r_end
    mov  eax, [_plane_state_step]
    inc  eax			    ; ��������� �� ��������� ��� ���������
    mov  [_plane_state_step], eax
    cmp  eax, 8 		    ; ���������, ����������� �� ����
    jne  fcp_barrel_r_end
    mov  [_plane_state], 0	    ; ���� �� �� ��������� ��������� �� ������ �����
  fcp_barrel_r_end :

    mov  eax, [_plane_state]
    cmp  eax, 5 		    ; ���� ������ ����� �����
    jne  fcp_barrel_l_end
    mov  eax, [_plane_state_step]
    inc  eax			    ; ��������� �� ��������� ��� ���������
    mov  [_plane_state_step], eax
    cmp  eax, 8 		    ; ���������, ����������� �� ����
    jne  fcp_barrel_l_end
    mov  [_plane_state], 1	    ; ���� �� �� ��������� ��������� �� ������ �����
  fcp_barrel_l_end:

    call animation_plane

  fcp_timex2_end:

    ; ����� ��� ����������� � 4 ���� ���������
    mov eax, [_timer_x4]
    cmp eax, 0
    jne fcp_timex4_end

    ; ��������� �������� ��������
    mov  eax,[_Y_plane]
    add  eax, 1
    mov  [_Y_plane], eax

  fcp_timex4_end:

    ; ������� �������� ��������

    mov  eax,[_Y_plane]
    mov  ebx,[_addit_VY_plane]
    add  eax, ebx
    mov  [_Y_plane], eax

    ; �������� ������ ��������, �������
    mov  eax,[_Y_plane]
    cmp  eax,  WINDOW_HEIGHT - GROUNG_HEIGHT - 32
    jl	 fcp_end_plane_landing
    mov  [_addit_VY_plane], 0	  ; ���������� ������� ��������
    mov  ebx, [_plane_state]
    cmp  ebx, 6
    je	fcp_end_plane_landing
    cmp  ebx, 0
    jne  fcp_horiz_fly
    mov  [_plane_state], 6	  ; ����� �������
    ;jmp  fcp_end_plane_landing
  fcp_horiz_fly:
    mov  eax,  WINDOW_HEIGHT - GROUNG_HEIGHT - 32
  fcp_end_plane_landing:
    cmp  eax, WINDOW_HEIGHT - GROUNG_HEIGHT - 32 + 6
    jle  fcp_no_plane_underground	  ; �������� �� ��, ����� ������� �� ���� "��� �����"
    mov  eax, WINDOW_HEIGHT - GROUNG_HEIGHT - 32 + 6
    mov  ebx, [_X_plane]
    cmp  ebx, WINDOW_WIDTH - 60-32
    jl	 fcp_no_end_level		    ; �������� �� ��, ��� ��������� �� ����� ���������� ������
    ; ���������� ������
    call  next_level			     ; ������� �� ��������� �������
  fcp_no_end_level:
  fcp_no_plane_underground:
    mov  [_Y_plane], eax

    ret



animation_plane:

    ; �������� ��������

    ; ����� ������
    mov  eax, [_plane_state]
    cmp  eax, 0
    jne  ap_state0_end
    mov  [_anim_pos_plane], 0
  ap_state0_end:

    ; ����� �����
    mov  eax, [_plane_state]
    cmp  eax, 1
    jne  ap_state1_end
    mov  [_anim_pos_plane], 8
  ap_state1_end:

    ; �������� ��� ���������� � �����
    mov  eax, [_plane_state]
    cmp  eax, 5
    ja	 ap_state2345_end
    cmp  eax, 2
    jb	 ap_state2345_end
    sub  eax, 2
    mov  ebx, eax	   ; �������� eax �� 9
    shl  eax, 3
    add  eax, ebx
    add  eax, [_plane_state_step]    ; ��������� ����� ����
    shl  eax, 2 		     ; �������� �� 4
    add  eax, _anim_array_uturn_rl   ; ���������� �������� ��� ������ ���������� �����
    mov  ebx, [eax]
    mov  [_anim_pos_plane], ebx      ; ����� ����������� ��� �����
  ap_state2345_end:

    ; ������� (������ ������)
    mov  eax, [_plane_state]
    cmp  eax, 6
    jne  ap_state6_end
    mov  [_anim_pos_plane], 22
  ap_state6_end:

     ret



draw_plane:

    ; ������� ���������� �����������
    mov  eax, 13
    mov  ebx,[_last_XY_plane]
    and  ebx, 0xffff0000
    add  ebx, 32
    mov  ecx,[_last_XY_plane]
    shl  ecx, 16
    add  ecx, 32
    mov  edx, BACK_COLOR
    int  0x40

    ; ������ ������ �������� ������ �� ������ ����� _anim_pos_plane
    mov  ebx, [_anim_pos_plane]
    shl  ebx, 10
    mov  eax, ebx
    add  ebx, eax
    add  ebx, eax
    add  ebx, bmp_plane

    ; ���������� ���������
    mov  edx,[_X_plane]
    shl  edx, 16
    add  edx,[_Y_plane]
    mov  [_last_XY_plane], edx

    ; ����� ��������
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
    add   eax, 32		; ����� � ����������� �� ����������� ������ ���������� ����� �������������� � �������
   pi_no_positive_vx:
    ; ������ � eax � ebx ���������� X,Y  ����� �������������� �������� � �������
    mov   edx, eax
    sub   eax, CITY_OFFSET
    ; �������� �� ����� �� ������� ������
    cmp   eax, 0
    jl	  pi_out_of_city
    cmp   eax, 32*20
    jg	  pi_out_of_city
    ; �������� �� �������������� � �������
    add   eax, 10	 ; ����� ���������� X �� 20
    mov   ecx, eax
    shr   eax, 4
    shr   ecx, 6
    sub   eax, ecx
    shr   ecx, 2
    add   eax, ecx
    shr   ecx, 2
    sub   eax, ecx
    dec   eax	      ; � eax ����� ������� ������ �� �������� ����� �����
    mov   esi, eax
    shl   esi, 1
    add   esi, _city	; � esi ��������� �� ������ ��������� � ������ �������
    mov   eax, edx
    ; ������ ������� ���������� Y ������� ��������� � ����������� Y ����� �������������� ��������
    xor   ecx, ecx
    mov   cx, [esi]
    mov   edx, ecx
    shl   ecx, 4
    shl   edx, 2
    add   ecx, edx
    mov   edx, WINDOW_HEIGHT - GROUNG_HEIGHT
    sub   edx, ecx	       ; � edx ���������� Y ������� ���������
    cmp   ebx, edx
    jl	  pi_no_crash
    ; ���� ������������ ���������
    mov   [_plane_health], 0
    call  bang_add	    ; ������ �����
    call  game_over	    ; ���������� ����
  pi_no_crash:

  pi_out_of_city:

    pop   edx
    pop   ecx
    pop   ebx
    pop   eax

    ret


; ================================== ����� ===========================================

bomb_drop:	    ; ��������� "�������� �����". ������� ����� �����, ������ ��� ��� ��������� ���������.
    ; �������� �� �������� ��������
    mov  eax, [_plane_health]
    cmp  eax, 0
    jle  bomb_drop_end
    ; ��������� �� ���������� ��������� �������� (������ �� ����� �����)
    mov  eax, [_plane_state]
    cmp  eax, 3
    ja	 bomb_drop_end
    ; ��������� �������� �� �������� ������� ����� ��������
    mov  eax, [_bomb_delay_time]
    cmp  eax, 0
    jne  bomb_drop_end
    mov  [_bomb_delay_time], BOMB_DELAY
    ; ����������, ������� �����
    mov  eax, [_bomb_count]
    cmp  eax, MAX_BOMBS-1
    jae  bomb_drop_end
    inc  eax
    mov  [_bomb_count], eax
    dec  eax
    shl  eax, 4
    add  eax, _bomb_array    ; ����� � ��� �������� ��������� �� ������ ������ � �����

    mov  ebx, [_X_plane]
    mov  [eax], ebx	       ; ������� ���������� X
    add  eax, 4
    mov  ebx, [_Y_plane]
    add  ebx, 30
    mov  [eax], ebx	       ; ������� ���������� Y

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
    mov  [eax], ebx	       ; ������� ������� �������� �� X

    add  eax, 4
    mov  ebx, 0
    mov  [eax], ebx	       ; ������� ������� �������� �� Y

    push  ecx
    mov   ecx, [_bomb_count]
    dec   ecx
    call  bombfly_sound_start
    pop   ecx

  bomb_drop_end:

    ret


bomb_proc:
    ; ������ ��������� ����� ���������� ����
    mov  eax, [_bomb_delay_time]
    cmp  eax, 0
    je	 bomb_proc_delay_timer_end
    dec  eax
    mov  [_bomb_delay_time], eax
  bomb_proc_delay_timer_end:
    ; ��������� �������� ����
    mov  eax, [_bomb_count]
    cmp  eax, 0
    je	 bomb_proc_end
    xor  ecx, ecx
  bomb_proc_loop:		 ; ���� ��������� ������� ����
    cmp  ecx, [_bomb_count]
    jae  bomb_proc_end
    mov  ebx, ecx
    shl  ebx, 4
    add  ebx,  _bomb_array	    ;  �������� ��������� �� ������ ������ � �����

    call bomb_hide   ; ������� ���������� ��������

    ; ���������

    ; ����������� �������� ��� �������� VY ������� �����

    add  ebx, 4*3
    mov  eax, [_timer_x2]
    cmp  eax, 0
    jne  bomb_proc_gravity_accel_end
    mov  eax, [ebx]
    add  eax, GRAVY_ACCEL
    mov  [ebx], eax
  bomb_proc_gravity_accel_end:
    ; ����������� �������� ��� ��������� X ������� �����
    sub  ebx, 4*3
    mov  eax, [ebx]
    add  ebx, 4*2
    mov  edx, [ebx]
    add  eax, edx
    sub  ebx, 4*2
    mov  [ebx], eax
    push eax
    ; ����������� �������� ��� ��������� Y ������� �����
    add  ebx, 4
    mov  eax, [ebx]
    add  ebx, 4*2
    mov  edx, [ebx]
    add  eax, edx
    sub  ebx, 4*2
    mov  [ebx], eax
    push eax

    ; �������������� ���� � �����
    sub  ebx, 4
    pop  edx	  ; ���������� Y
    pop  eax	  ; ���������� X
    ; �������� �� ����� ���� �� ������� ������� ����, ����� ������ ��������� ��� ������ �� �������
    cmp  eax, 10
    jle   bomb_proc_delete_bomb
    cmp  eax, WINDOW_WIDTH - 36
    jge   bomb_proc_delete_bomb
    ; �������� �� �������������� � �����
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
    call air_raid_warning_on   ; ��������� ��������� �������
    pop   ebx
    pop   eax
  bomb_proc_delete_bomb:
    call bomb_delete	      ; ������� �����
    dec  ecx		      ; �.�. ����� ������� - �������� �� ��� �� ������ �����
    mov  eax, [_bomb_count]
    dec  eax
    mov  [_bomb_count], eax   ; ��������� ���������� ����
    jmp  bomb_proc_draw_end   ; �� ������ ����� ���� ������� ��
  bomb_proc_interaction_end:
    call bomb_draw
  bomb_proc_draw_end:

    inc  ecx
    jmp  bomb_proc_loop
  bomb_proc_end:

    ret


bomb_delete:	   ; �������� ������ � �����, � ��� ����� ��������� �����
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
    add  ebx,  _bomb_array	    ;  �������� ��������� �� ������ ������ � �����
    dec  ecx
    mov  edx, ecx
    shl  edx, 4
    add  edx,  _bomb_array	    ;  �������� ��������� �� ������ ������ � ��������� �����
    inc  ecx
    ; ����������� ������ � ��������� ����� �� ����� ����������
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
    ; ������� � ��������� �����
    inc  ecx
    jmp  bomb_delete_loop
  bomb_delete_loop_end:
    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret

bomb_hide:  ; ������� ����� � ������, ebx - ��������� �� ������ ������ � �����

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


bomb_draw:	; ���������� �����, ebx - ��������� �� ������ ������ � �����

    push eax
    push ebx
    push ecx
    push edx

    ; ��������� � edx ��������� �����
    mov  edx, [ebx]
    shl  edx, 16
    add  ebx, 4
    mov  eax, [ebx]
    add  edx, eax

    ; ��������� ��������� �� �������� ����� � ebx (����� � ������� ������ ��������)
    add  ebx, 4
    mov  eax, [ebx]	  ; ����� � eax - �������������� �������� �����
    add  ebx, 4
    mov  ecx, [ebx]	  ; ����� � ecx - ������������ �������� �����
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
       ; ������ � ebx ����� �������� �����
    shl  ebx, 8
    mov  eax, ebx
    add  ebx, eax
    add  ebx, eax
    add  ebx, bmp_bomb	 ; ������ � ebx ��������� �� �������� �����

    ; ����� ��������
    mov  eax,7
    mov  ecx,16*65536+16
    int  0x40

    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret


; �������������� ���� � �����

bomb_check_detonation:	       ; ��������� ����� � ������������: eax - ���������� X, eab - ���������� Y
			       ; � ��� ������������� �������� ������ ���������� (��� ������ �����)
			       ; � eax ������� ��������� �������: 0 - ������ �� ��������, 1 - ��������� �����
			       ; (������� ����������)

    push  ecx
    push  edx

    add   eax, 8
    add   ebx, 8

    sub   eax, CITY_OFFSET
    ; �������� �� ����� �� ������� ������
    cmp   eax, 0
    jl	  bcd_out_of_city
    cmp   eax, 32*20
    jg	  bcd_out_of_city
    ; �������� �� �������������� � �������
    add   eax, 10	 ; ����� ���������� X �� 20
    mov   ecx, eax
    shr   eax, 4
    shr   ecx, 6
    sub   eax, ecx
    shr   ecx, 2
    add   eax, ecx
    shr   ecx, 2
    sub   eax, ecx
    dec   eax	      ; � eax ����� ������� ������ �� �������� ����� �����
    mov   esi, eax
    shl   esi, 1
    add   esi, _city	; � esi ��������� �� ������ ��������� � ������ �������

    mov   edx, WINDOW_HEIGHT - GROUNG_HEIGHT
    sub   edx, ebx
    mov   ebx, edx
    cmp   ebx, 0
    jg	  bcd_no_neg_value_Y
    xor   ecx, ecx
    mov   cx, [esi]
    xor   ebx, ebx
    jmp   bcd_damage	; ������� ����������
  bcd_no_neg_value_Y:
    add   ebx, 10	  ; ����� ���������� Y �� 20
    mov   ecx, ebx
    shr   ebx, 4
    shr   ecx, 6
    sub   ebx, ecx
    shr   ecx, 2
    add   ebx, ecx
    shr   ecx, 2
    sub   ebx, ecx	   ; � ebx ������  ����� "� �������" ��� ������������

    xor   ecx, ecx
    mov   cx, [esi]
    cmp   ebx, ecx	      ; �������� �� ��������� � ��������
    jg	  bcd_no_detonation
  bcd_damage:	     ; ����������
    ;���������� �������� � ������� ������ �����
    mov   edx, ecx
    mov   ecx, ebx
    sub   ecx, 2
    cmp   ecx, 0	     ; ������ �������� �� 0, ���� �� ���� � �����
    jge   bcd_no_neg_value_H
    xor   ecx, ecx
   bcd_no_neg_value_H:
    mov   [esi], cx	    ;  ������� �������� ������ ������� ����� ������
    ; �������� ���������� �������
    call  clear_tiles
    ; �������� �� ��������� � �������
    call ackack_check_bombing
    ; ���������� ��������� �������� �����
    ; �� ������ ������ �����: eax - ����� ������������ (����������� - ��� � ������� ��������������� ������ �����) ��������,
    ; ebx - ������ ��������� �����, ecx - ����� ������ ������������ ��������, edx - ������� ������ ������������ ��������,
    ; esi - ��������� �� �������� ������ ������������ �������� � �������
    dec   eax
    cmp   eax, 0
    jl	  bcd_damage_left_end
    sub   esi, 2
    xor   ecx, ecx
    mov   cx, [esi]	; � ecx ������ ������ ��������
    mov   edx, ecx
    push  ecx
    sub   ecx, ebx
    inc   ecx
    cmp   ecx, 2
    pop   ecx
    ja	  bcd_damage_left_end	  ; �������  |ecx-ebx|<=1
    dec   ecx
    cmp   ecx, 0	     ; ������ �������� �� 0, ���� �� ���� � �����
    jge   bcd_no_neg_value_HL
    xor   ecx, ecx
   bcd_no_neg_value_HL:
    mov   [esi], cx
    call  clear_tiles
    ; �������� �� ��������� � �������
    call ackack_check_bombing
  bcd_damage_left_end:

    ; ���������� ��������� �������� ������
    ; �� ������ ������ �����: eax - ����� ������  ��������,
    ; ebx - ������ ��������� �����, ecx - ����� ������ ������ ��������, edx - ������� ������ ������ ��������,
    ; esi - ��������� �� �������� ������ ������ �������� � �������
    inc   eax
    inc   eax
    cmp   eax, 32
    jge   bcd_damage_right_end
    add   esi, 4
    xor   ecx, ecx
    mov   cx, [esi]	; � ecx ������ ������� ��������
    mov   edx, ecx
    push  ecx
    sub   ecx, ebx
    inc   ecx
    cmp   ecx, 2
    pop   ecx
    ja	  bcd_damage_right_end	   ; �������  |ecx-ebx|<=1
    dec   ecx
    cmp   ecx, 0	     ; ������ �������� �� 0, ���� �� ���� � �����
    jge   bcd_no_neg_value_HR
    xor   ecx, ecx
   bcd_no_neg_value_HR:
    mov   [esi], cx
    call  clear_tiles
    ; �������� �� ��������� � �������
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


bombfly_sound_start:	; ecx - ����� ����� � �������

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


bombfly_sound_stop:    ; ecx - ����� ����� � �������

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

; ==================================== ������ =======================================


fill_circle:
    ; ���������� ������������ �����    (�������� ����������)
    ; ������� ���������:
    ; eax - ���������� ������ �� X
    ; ebx - ���������� ������ �� Y
    ; ecx - ������
    ; edx - ����
    mov  edi, eax
    mov  esi, ebx
    push edx	     ; ���� � ����

    mov  eax, ecx
    shl  eax, 1
    mov  edx, 3
    sub  edx, eax	  ; D:=3-2*R
    xor  ebx, ebx   ; ������ � ebx ����� ����� X, � ecx ����� Y, � edx ���������� D, � eax - ������������� ������

  fc_loop:
    cmp  ebx, ecx
    jg	 fc_loop_end
    ; ��������� �������
     pop  eax	    ; ���� �� �����
     push edx	    ; D � ����
     mov  edx, eax  ; � edx - ����
     push ebx
     push ecx
     ; ������ ������ �������
     mov  eax, edi
     sub  eax, ebx
     shl  eax, 16
     add  ebx, edi
     add  ebx, eax   ; �������� ����� ������� �� �
     add  ecx, esi
     mov  eax, ecx
     shl  eax, 16
     add  ecx, eax   ; �������� ����� ������� �� y
     mov  eax, 38
     int  0x40
     pop  ecx
     push ecx
     ; ������ ������ �������
     mov  eax, esi
     sub  eax, ecx
     mov  ecx, eax
     shl  eax, 16
     add  ecx, eax   ; �������� ����� ������� �� y
     mov  eax, 38
     int  0x40
     pop  ecx
     pop  ebx
     push ecx
     push ebx	; �������� ��������! ������� �������� ��������� � ���� ���������!
     ; ������ ������ �������
     mov  eax, edi
     sub  eax, ecx
     shl  eax, 16
     add  ecx, edi
     add  ecx, eax
     mov  eax, ebx
     mov  ebx, ecx    ; �������� ����� ������� �� �
     mov  ecx, eax    ; ��������! � ecx - ���������� x
     add  ecx, esi
     mov  eax, ecx
     shl  eax, 16
     add  ecx, eax   ; �������� ����� ������� �� y
     mov  eax, 38
     int  0x40
     pop  ecx
     push ecx
     ; ������ ��������� �������
     mov  eax, esi
     sub  eax, ecx
     mov  ecx, eax
     shl  eax, 16
     add  ecx, eax   ; �������� ����� ������� �� y
     mov  eax, 38
     int  0x40
     pop  ebx
     pop  ecx

     mov  eax, edx	 ; � eax - ����
     pop  edx		 ; D �� �����
     push eax		 ; ���� � ����
    ; ������� � ��������� �����
    cmp  edx, 0
    jge  fc_loop_D_more_0     ; ���� D<0
  fc_loop_D_low_0:	      ; ��
    mov  eax, ebx
    shl  eax, 2
    add  edx, eax
    add  edx, 4        ; D:=D+4*X+6
    jmp  fc_loop_cmpD_end
  fc_loop_D_more_0:	      ; �����
    mov  eax, ebx
    sub  eax, ecx
    shl  eax, 2
    add  edx, eax
    add  edx, 10      ; D:=D+4*(X-Y)+10
    dec  ecx
  fc_loop_cmpD_end:	      ; ����� ����
    inc  ebx
    jmp  fc_loop
  fc_loop_end:

    pop  edx	   ; ���� �� �����

    ret




bang_add:   ; �������� ����� �� �����, eax - ���������� X, eab - ���������� Y

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
    ; �������� �� ������� ������
    mov  edx, [ebx]
    cmp  edx, -1
    je	 bang_proc_loop_freecheck_end
    ; �������� �� ���������� ������
    and  edx, 0x0000FFFF
    cmp  edx, 0
    jne  bang_proc_loop_endcheck_end
    ; ����� ����� ��������� ���������� ������
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
    ; ����� ����� ��������� ������
    push  ebx
    push  ecx
    ; ������ ������ � ������� �����
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
    ; ������ ����������
    add  ebx, 4
    mov  eax, [ebx]
    mov  ebx, eax
    shr  eax, 16
    and  ebx, 0x0000FFFF
    ;������ ����
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

; ===================================== ������� =========================================
; ���������� ���������, ���������� �������, ��������� �������, ��������


ackack_draw:			   ; ��������� �������

    mov  ebx, _ackack
    xor  ecx, ecx
    xor  esi, esi	; ���� ������� ���� �� ����� ������� (����������� ��� � �����������)
  ackack_draw_loop:
    cmp  ecx, 32
    jae  ackack_draw_loop_end
    xor  eax, eax
    mov  ax, [ebx]
    cmp  eax, 1
    jne  ackack_draw_continue	   ; ���� �������� ������� ������ � ������� �� 1 �� �� ������ �������
    push  ebx
    push  ecx
    ; � ebx �������� ��������� �� ������ ���������
    mov   ebx, ecx
    shl   ebx, 1
    add   ebx, _city  ; �������� ��������� �� ������ ���������
    xor  eax, eax
    mov  ax, [ebx]    ; �������� ������ ���������
    cmp   eax, 0
    jle   ackack_draw_continue2      ; ���� ��������� ��� �� �� ������ �������
    mov   esi, 1
    call  ackack_bullet_start
    ; ��������� �������
    mov   ebx, bmp_ackack   ; �������� ��������� �� ��������
    cmp   edx, 0
    je	  ackack_draw_no_start_bullet
    add   ebx, 20*20*3	    ; �������� �� ��������� ��������
  ackack_draw_no_start_bullet:
    ; ������������ ��������� ��������  � edx
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
    sub   ecx, 20  ; ������� ���������� �� ������ ���� ���������
    add   edx, ecx ; �������� ����������
    mov   ecx, 20*0x00010000 + 20   ; ������ ������ ��������
    mov   eax, 7
    int   0x40		; ������� ��������
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
    ; ��������� ������
    call  air_raid_warning_off
  @@:

    ret


ackack_check_bombing: ; �������� �� ������ ������� ������� : eax - ����� �������

    push eax
    push ebx
    push ecx
    push edx
    ; �������� �� ��������� � ������� �������
    mov  ebx, eax
    shl  ebx, 1
    push  ebx
    add  ebx, _ackack
    mov  edx, ebx	; �������� ��������� ��������� ������� � edx
    xor  eax, eax
    mov  ax, [ebx]    ; � eax  �������� ��������� �������
    cmp  eax, 1
    jne  acb_no_bombing       ; �������� �� ������� ������� � ������ �������
    xor  eax, eax
    mov  [ebx], ax     ; ������� �������
    call ackack_next_wave
  acb_no_bombing:
    ; �������� �� ������ ����������� �������
    pop  ebx
    add  ebx, _city
    xor  eax, eax
    mov  ax, [ebx]    ; � eax  �������� ��������� �������
    cmp  eax, 0
    jg	 acb_no_destroy_building       ; �������� �� ������� ��������� � �������
    xor  eax, eax
    mov  [ebx], ax     ; ������� ������� ���� � �����������
  acb_no_destroy_building:
    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret



ackack_next_wave:	 ; ����� ����� �������

    push eax
    push ebx
    push ecx
    push edx

  anw_begin:
    mov  ebx, _ackack
    xor  ecx, ecx
    xor  edx, edx	; ���� �������� �� ���������
  anw_loop:
    cmp  ecx, 32
    jae  anw_loop_end
    xor  eax, eax
    mov  ax, [ebx]
    cmp  eax, 1
    jle  anw_continue	   ; ���� �������� ������� ������ <=1 �� ����������
    dec  eax
    mov  [ebx], ax
    cmp  eax, 1
    jg	 anw_no_active_ackack	  ; ���� �������� ������� ������ � ������� >1 �� ����������
    mov  edx, 1        ; ���������� ��� ���� ����������� �������
    jmp  anw_continue
  anw_no_active_ackack:
    cmp  edx, 1
    je	 anw_continue	   ; ���� ����� 1 �� ����������
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





ackack_bullet_start:	 ; ������ ��������  (�������: eax - ������ �������, ecx - ����� �������;
			 ;                   ��������: edx - ���� ������ ������� (0-���, 1 -��)

    push  eax
    push  ebx
    push  ecx

    cmp   [_flag_air_raid_warning], 1
    jne   abs_no_start_bullet

    ; � ebx �������� ��������� �� ��������� �������
    mov   edx, eax
    mov   ebx, ecx
    shl   ebx, 2
    add   ebx, _ackack_bullet  ; �������� ��������� �� ��������� �������
    mov   eax, [ebx]	; �������� ��������� �������
    cmp   eax, 0
    jne   abs_no_start_bullet	   ; ���� �� 0, �� �� ��������� �����
    ; ������ �������
    mov   eax, edx    ; �������� � eax ������ ���������
    ; ������� ���������� �������
    mov   ecx, eax
    shl   eax, 4
    shl   ecx, 2
    add   eax, ecx
    mov   ecx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub   ecx, eax
    sub   ecx, 20+BULLET_SIZE  ; ������� ���������� �� ������ ���� ��������� + ������ �������� (� ecx - ��������� ���������� �������)
    mov   [ebx], ecx  ; ������ ��������� ���������� �������

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


ackack_bullet_proc:	; ��������� ��������� � ��������� ��������

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
    ; �������� ������� �����������
    mov   ebx, ecx
    shl   ebx, 4
    shl   ecx, 2
    add   ebx, ecx
    add   ebx, CITY_OFFSET
    add   ebx, 7	     ; �������� �������
    mov   edx, ebx
    shl   ebx, 16
    add   ebx, edx   ; � ebx ���������� ������ � ����� ������� �� ��� X
    mov   ecx, eax
    mov   edx, ecx
    add   edx, BULLET_SIZE   ; ��������� ����� �������
    shl   ecx, 16
    add   ecx, edx   ; � ebx ���������� ������ � ����� ������� �� ��� Y
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
    je	 abp_no_bullet_interaction	; ���� �� ���� �������������� �� ������ �������
    xor  eax, eax
    mov  [ebx], eax
    jmp  abp_end_draw_bullet
  abp_no_bullet_interaction:

    ; ������ �������
    mov   ebx, ecx
    shl   ebx, 4
    shl   ecx, 2
    add   ebx, ecx
    add   ebx, CITY_OFFSET
    add   ebx, 7	     ; �������� �������
    mov   edx, ebx
    shl   ebx, 16
    add   ebx, edx   ; � ebx ���������� ������ � ����� ������� �� ��� X
    mov   ecx, eax
    mov   edx, ecx
    add   edx, BULLET_SIZE   ; ��������� ����� �������
    shl   ecx, 16
    add   ecx, edx   ; � ebx ���������� ������ � ����� ������� �� ��� Y
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


ackack_bullet_interaction_check:       ; �������� �������������� ��������
				       ;  (�������: eax - ���������� ������� �� Y, ecx - ����� �������;
				       ;  ��������: edx - ���� ��������� (0-���, 1 -��))


    push  eax
    push  ebx
    push  ecx

    ; ���� ������� ������ ����� �� ����������
    mov   ebx, [_plane_health]
    cmp   ebx, 0
    jle   abic_with_plane_no
    mov   ebx, [_plane_state]
    cmp   ebx, 4
    je	  abic_with_plane_no
    cmp   ebx, 5
    je	  abic_with_plane_no
    ; �������� �� ����������
    mov   ebx, [_Y_plane]
    cmp   eax, ebx
    jl	  abic_with_plane_no	 ; ���� ������ ���� ���������� ��������� �� ����������
    add   ebx, 20     ; � ebx - ���������� ��������� � �������
    cmp   eax, ebx
    jg	  abic_with_plane_no	 ; ���� ������ ���� ���������� ��������� �� ����������
    ; ������� ���������� X �������
    mov   edx, ecx
    shl   edx, 4
    shl   ecx, 2
    add   ecx, edx
    add   ecx, CITY_OFFSET
    add   ecx, 7    ; � ecx - ���������� X ������ �������
    mov   ebx, [_X_plane]
    add   ebx, 32
    cmp   ecx, ebx
    jg	  abic_with_plane_no
    mov   ebx, [_X_plane]
    add   ecx, 5
    cmp   ecx, ebx
    jl	  abic_with_plane_no
    ; ���� ���������!!!
    mov   ebx, [_plane_health]
    sub   ebx, 20
    mov   [_plane_health], ebx
    cmp   ebx, 0		; �������� �� game over
    jg	  abic_no_plane_crash
    mov   [_plane_health], 0
    mov   eax, [_X_plane]
    mov   ebx, [_Y_plane]
    call  bang_add	    ; ������ �����
    call  game_over	    ; ���������� ����
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

; ===================================== ��� =========================================

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


city_draw:	      ; ��������� ������

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
    ; ������������ ��������� �������� � edx
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
    add   edx, ecx ; �������� ����������
    mov   ecx, 20*0x00010000 + 20   ; ������ ������ ��������
    mov   eax, 7
    int   0x40		; ������� ��������
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


clear_tiles:	   ; �������� ���������� ������: eax - ����� �������, ecx - ������ ������ �� ���������, edx - ������� ������ �� ���������

    push eax
    push ebx
    push ecx
    push edx
    ; ������������ ���������� X ���� ��������������
    mov  ebx, eax
    shl  eax, 4
    shl  ebx, 2
    add  eax, ebx
    add  eax, CITY_OFFSET
    ;  ������������ ���������� Y1 ������� ���� ������ ������
    mov  ebx, ecx
    shl  ecx, 4
    shl  ebx, 2
    add  ecx, ebx
    mov  ebx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub  ebx, ecx
    mov  ecx, ebx
    add  ecx, 20
    ; ������������ ���������� Y2 �������� ���� ������� ������
    mov  ebx, edx
    shl  edx, 4
    shl  ebx, 2
    add  edx, ebx
    mov  ebx, WINDOW_HEIGHT-GROUNG_HEIGHT
    sub  ebx, edx
    mov  edx, ebx
    sub  edx, 20   ; ���� � ������� �������, ���� ��� ����
    ; ������� ���������� �������������� �� Y=Y2 � ��� ������ H=Y1-Y2
    sub  ecx, edx
    shl  edx, 16
    add  ecx, edx
    ; ������� ���������� �������������� �� X � ��� ������ 20
    shl  eax, 16
    add  eax, 20
    mov  ebx, eax
    ; ��������� �������������� ����� ����
    mov  eax, 13
    mov  edx, BACK_COLOR
    int  0x40

    pop  edx
    pop  ecx
    pop  ebx
    pop  eax

    ret


load_level:	   ; �������� ������, � eax ����� ������������ ������

    mov  eax, [_level_num]
    dec  eax
    shl  eax, 7 	    ; �������� �� ������ ������ ��� ������ ������ (128 ����)
    mov  ebx, levels
    add  ebx, eax	    ; � ebx ��������� �� ������ ������������ ������
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

; =================================== ������ ========================================

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

draw_window:			     ; ��������� ����

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

    ;���������
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-80)*65536 + 60
    mov  ecx,0x80000000
    mov  edx,_text_intro_title
    int  0x40

    ;�������� ����
    mov  eax,4
    mov  ebx,(WINDOW_WIDTH/2-200)*65536 + 120
    mov  ecx,0x80000000
    mov  edx,_text_intro_description1
    int  0x40


    ;������� ����������
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

    ; ����� ������

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

    ; ��������� ���������

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

    ; ������ ���� � �����

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
    mov  ebx, (WINDOW_WIDTH/2+35)*0x00010000 + 30
    mov  ecx, 370*0x00010000 + 15
    mov  edx, BACK_COLOR
    int  0x40

    mov  eax, 47
    mov  ebx, 0x80020000
    mov  ecx, [_set_volume]
    mov  edx, (WINDOW_WIDTH/2+35)*65536 + 370
    mov  esi,0x00000000
    int  0x40

    ret

;---------------------------------------------------------------------------
;-------------------------------- ������ -----------------------------------
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
_plane_state dd ?  ; 0 - ����� �������, 1- ����� ������, 2-�������� ������-������, 3-�������� �����-�������, 4 - ����� ������, 5 - ����� �����
_plane_state_step dd ?	 ; ����� ���� �������� ��������� (����� ��� ������������� � ���������)
_plane_health dd ?   ; �������� ��������

_bomb_count dd ?
_bomb_array rd 4*MAX_BOMBS
_bomb_delay_time dd ?	; �������� ������� �� ����� �����

_bang_array rd 2*MAX_BOMBS

_anim_pos_plane dd ?
; ������������������ �������� ��������
_anim_array_uturn_rl dd 0,1,2,3,4,5,6,7,8
_anim_array_uturn_lr dd 8,7,6,9,10,11,2,1,0
_anim_array_barrel_r dd 0,1,2,12,13,14,15,16,0
_anim_array_barrel_l dd 8,17,18,19,20,21,6,7,8


cur_level:
; ������, � ������� ��������� ������� ��������� ������. ���������� � ���� ����������� ������
_city rw 32
; ������, � ������� ��������� ������� ��������� �������. ���������� � ���� ����������� ������
_ackack rw 32
; ������, � ������� �������� �������� ������� �������� �������� �� �������
_ackack_bullet rd 32

; ����� �������� ������
_level_num dd 1
; ������� �������: 32 ����� �������� ������ �������� (������ "� ���������") ��� _level?_city
;                  � ������� ��������� ������� ��� _level?_ackack
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

; �����
_count_sounds dd ?
_array_sounds rd 2*MAX_SOUNDS

_channel_sound_plane dd ?    ; ����� ������ ����� ��������� ��������
_channel_sound_arw dd ?      ; ����� ������ ����� ������
_array_bombsoundchannels rd MAX_BOMBS	 ; ������ ������� �������� ������� ��� ����

_set_volume dd 8    ; ��������� 0..8

; ������

_s_current_dir rb 4096	 ; ���� � ������������ �����

file_info:	      ; �������������� ��������� ��� ������ � �������
  _fi_func dd ?
  _fi_pos dd ?
  dd 0
  _fi_size dd ?
  _fi_pbuff dd ?
  db 0
  _fi_pfilename dd ?

if lang eq ru_RU
	include 'lang-ru_RU.inc' ; Encoded as CP866
else ; Default to en_US
	include 'lang-en_US.inc'
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
rb 0x100 ; ������ ������ ��� �����.

i_end:

