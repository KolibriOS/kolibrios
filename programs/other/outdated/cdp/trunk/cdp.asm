;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                           ;
;    Audio CD player; code by Dmitry Yushko - dma@bn.by     ;
;                                                           ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "..\..\..\..\macros.inc"
include "lang.inc"

FALSE  equ 0
TRUE   equ 1

ESC_KEY   equ 27
LEFT_KEY  equ 176
RIGHT_KEY equ 179

NORMAL_PLAY  equ 0
REPEAT_TRACK equ 1
REPEAT_DISK  equ 2
SHUFFLE_DISK equ 3

COLOR_FUNC_BUTS equ 0x00dddddd

use32

                  org    0x0
                  db     'MENUET01'              ; 8 byte id
                  dd     0x01                      ; required os
                  dd     START                   ; program start
                  dd     I_END                   ; program image size
                  dd     0x2000                  ; required amount of memory
                  dd     0x2000                  ; esp = 0x7fff0
                  dd     0x0, 0x0              ; reserved=no extended header

START:
    call chk_cdrom                      ; start of execution
    call read_cd

  red:                          ; redraw
    call draw_window            ; at first, draw the window
still:

    mov  eax,23
    mov  ebx,10                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button

    call draw_info
    cmp  [curr_trk],0
    je   @f
    call current_trk_time
   @@:
    jmp  still


  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall

;======  hotkeys:
    cmp  ah,0x61
    jb   @f
    cmp  ah,0x7a
    ja   @f
    and  ah,11011111b
   @@:

    cmp  ah,'P' ;PLAY
    jne  no_key_play
    call play_acd
    jmp still
 no_key_play:

    cmp ah,'S' ;STOP
    jne no_key_stop
    mov [if_paused],FALSE
    call stop_playing
    jmp still
 no_key_stop:

    cmp ah,'N' ;NEXT
    jne no_key_next
    call play_next_trk
    jmp still
 no_key_next:

    cmp ah,'B' ;BACK
    jne no_key_back
    call play_back_trk
    jmp still
 no_key_back:

    cmp  ah,'F' ;FORWARD
    jne  no_key_fwd
    call fast_forward
    jmp still
 no_key_fwd:

    cmp ah,'R' ;REWIND
    jne no_key_rewind
    call fast_rewind
    jmp still
 no_key_rewind:

    cmp ah,'M' ;MODE
    jne no_key_mode
    call change_mode
    jmp still
 no_key_mode:


    cmp ah,'L' ;READ PLAYLIST
    jne no_key_list
    mov [if_paused],FALSE
    mov [curr_trk],0
    call stop_playing
    call chk_cdrom
    call read_cd
    jmp still
 no_key_list:

    cmp ah,50      ;F1 key
    jz  itsahelpkey

    cmp ah,'H' ;HELP
    jne no_key_help
   itsahelpkey:
    cmp [flag],4
    je  still
    cmp [flag],1
    jne was_it_ok_false
    mov [was_it_ok],TRUE
    jmp flag4_done
   was_it_ok_false:
    mov [was_it_ok],FALSE
   flag4_done:
    mov [flag],4
    mov [help_screen],1
    call draw_window
    jmp still
 no_key_help:


    cmp ah,ESC_KEY
    jne no_esc_key
    cmp [flag],4
    jne still
    cmp [was_it_ok],FALSE
    jne was_it_ok_true
    mov [flag],0
    jmp end_esc_key
   was_it_ok_true:
    mov [flag],1
   end_esc_key:
    call draw_window
 no_esc_key:

    cmp ah,LEFT_KEY
    jne no_left_key
    cmp [flag],4
    jne still
    cmp [help_screen],1
    jz  still
    dec [help_screen]
    call draw_window
   no_left_key:

    cmp ah,RIGHT_KEY
    jne no_right_key
    cmp [flag],4
    jne still
    cmp [help_screen],3
    jz  still
    inc [help_screen]
    call draw_window
   no_right_key:


    jmp  still


  button:                       ; button
    mov  eax,17
    mcall

    cmp  ah,1                   ; button id=1 ?
    jnz  no_but_close
    mov  eax,24
    mov  ebx,3
    mcall
    mov  eax,0xffffffff         ; close this program
    mcall
  no_but_close:

    cmp  ah,2
    jne  no_but_play
    call play_acd
    jmp still
 no_but_play:

    cmp ah,3
    jne no_but_stop
    mov [if_paused],FALSE
    call stop_playing
    jmp still
 no_but_stop:

    cmp ah,4
    jne no_but_reread
    mov [curr_trk],0
    call chk_cdrom
    call read_cd
    mov [if_paused],FALSE
    call stop_playing
    jmp still
 no_but_reread:

    cmp ah,5
    jne no_but_next
    call play_next_trk
    jmp still
   no_but_next:

    cmp ah,6
    jne no_but_back
    call play_back_trk
    jmp still
   no_but_back:

    cmp ah,7
    jne no_but_mode
    call change_mode
    jmp still
   no_but_mode:

    cmp ah,8
    jne no_but_frew
    call fast_rewind
    jmp still
   no_but_frew:

    cmp ah,9
    jne no_but_ffwd
    call fast_forward
    jmp still
   no_but_ffwd:

    cmp  ah,10
    jb   no_but_track
    cmp  ah,40
    ja   no_but_track
    call read_cd
    cmp  [flag],1
    jne  no_but_track
    mov  cl,ah
    sub  cl,10
    mov  [curr_trk],cl
    mov  cl,[max_trk]
    mov  [shuftab],cl
    call stop_playing
    call renew_shuftab
    call play_n_track
    call rem_time_trk
    jmp still
  no_but_track:

    jmp  still


change_mode:
    cmp [mode],3
    jne inc_mode
    mov [mode],0
    jmp end_but_mode
   inc_mode:
    inc [mode]
   end_but_mode:
    call draw_info
ret

play_next_trk:
    cmp [curr_trk],0
    je  @play_next_trk
    cmp [if_paused],TRUE
    je  @play_next_trk
    cmp [mode],NORMAL_PLAY
    jne play_next_mode1
    xor eax,eax
    mov al,[curr_trk]
    cmp [max_trk],al
    je  @play_next_trk
    inc [curr_trk]
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_next_trk
   play_next_mode1:
    cmp [mode],REPEAT_TRACK
    jne play_next_mode2
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_next_trk
   play_next_mode2:
    cmp [mode],REPEAT_DISK
    jne play_next_mode3
    xor eax,eax
    mov al,[curr_trk]
    cmp [max_trk],al
    jne  play_next_mode2_go
    mov [curr_trk],1
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_next_trk
   play_next_mode2_go:
    inc  [curr_trk]
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_next_trk
   play_next_mode3:
    cmp  [mode],SHUFFLE_DISK
    jne  @play_next_trk
   call shuffle_track
   @play_next_trk:
ret

play_back_trk:
    cmp [curr_trk],0
    je  @play_back_trk
    cmp [if_paused],TRUE
    je  @play_back_trk
    cmp [mode],NORMAL_PLAY
    jne play_back_mode1
    xor eax,eax
    mov al,[curr_trk]
    cmp al,1
    je  @play_back_trk
    dec [curr_trk]
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_back_trk
   play_back_mode1:
    cmp [mode],REPEAT_TRACK
    jne play_back_mode2
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_back_trk
   play_back_mode2:
    cmp [mode],REPEAT_DISK
    jne play_back_mode3
    xor eax,eax
    mov al,[curr_trk]
    cmp al,1
    jne  play_back_mode2_go
    mov al,[max_trk]
    mov [curr_trk],al
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_back_trk
   play_back_mode2_go:
    dec  [curr_trk]
    cmp [if_stopped],TRUE
    je @play_next_trk
    call play_n_track
    jmp  @play_back_trk
   play_back_mode3: ;(shuffle)
;   call shuffle_track
   @play_back_trk:
ret


current_trk_time:
    cmp [if_stopped],TRUE
    je  menshe
    call get_uptime
    mov ebx,[stimtrk]
    sub eax,ebx
   ; eax now is seconds from track start * 100
    xor edx,edx
    mov ecx,100
    div ecx
    mov [curr_trk_pg_time],eax
    mov ebx,[curr_trk_length]
;    add eax,1 ;{inc curr time on 1 sec)
    cmp eax,ebx
    jb  menshe
    call stop_playing
    cmp [mode],SHUFFLE_DISK
    jne @f
    call shuffle_track
   @@:
    cmp [mode],REPEAT_TRACK
    je  @@mode_repeat_1
    mov al,[max_trk]
    cmp [curr_trk],al
    jb  @@next_trk_ok
    cmp [mode],REPEAT_DISK
    jne menshe
    mov [curr_trk],0
   @@next_trk_ok:
    inc [curr_trk]
   @@mode_repeat_1:
    call play_n_track
   menshe:
ret


rem_time_trk:
    call get_uptime
    mov  [stimtrk],eax
    ret

fast_forward:
    cmp [if_stopped],TRUE
    je end_ffwd
    mov eax,[curr_trk_pg_time]
    add eax,5
    cmp eax,[curr_trk_length]
    jae end_ffwd
    cmp [stimtrk],500
    jbe end_ffwd
    sub [stimtrk],500
    call current_trk_time
    call play_from_x_time
   end_ffwd:
ret

fast_rewind:
    cmp [if_stopped],TRUE
    je end_frew
    cmp [curr_trk_pg_time],5
    jbe end_frew
    add [stimtrk],500
    call current_trk_time
    call play_from_x_time
   end_frew:
ret

renew_shuftab:
    mov  ecx,40
   @rn:
    mov  [shuftab+ecx],cl
    loop @rn
    mov  cl,[max_trk]
    mov  [shuftab],cl
ret


shuffle_track:
   call get_uptime
   ror  eax,16
   cmp  eax,0
   je   shuffle_track
   xor  ecx,ecx
   mov  cl,[shuftab]
   cmp  ecx,1
   je   @enddsk
   xor  edx,edx
   div  ecx
   cmp  edx,0
   je   shuffle_track
   xor  ecx,ecx
   mov  cl,[max_trk]
 @main_loop:
   xor  eax,eax
   mov  al,[shuftab+ecx]
   cmp  al,0
   je   @f
   dec  edx
   cmp  edx,0
   jne  @f
   mov  cl,[shuftab]
   dec  cl
   mov  [shuftab],cl
   mov  [shuftab+eax],0
   mov  [curr_trk],al
   call play_n_track
   jmp  @endofshuffle
 @@:
   loop @main_loop
   jmp  @endofshuffle
 @enddsk:
   call stop_playing
 @endofshuffle:

ret




play_from_x_time:
    xor  ecx,ecx
    mov  cl,[curr_trk]
    shl  cl,3
    add  cl,1
    add  ecx,cdp
    mov  ebx,[ecx]
    mov  ecx,ebx
    and  ecx,0x00ffffff

    mov  eax,[curr_trk_pg_time]
    xor  edx,edx
    mov  ebx,60
    div  ebx
    add  cl,al ;mins
    add  dl,ch
    xor eax,eax
    mov al,dl
    xor edx,edx
    div ebx
    add cl,al   ;real min
    mov ch,dl   ;real sec

    mov  eax,24
    mov  ebx,1
    mcall
    ret

play_n_track:
    mov  [if_paused],FALSE
    mov  [if_stopped],FALSE
    mov  [curr_trk_pg_time],0
    call draw_window
;    mov  eax,26
;    mov  ebx,9
;    mcall
    call get_uptime
    mov  [stimtrk],eax
    xor  ebx,ebx
    xor  ecx,ecx
    mov  cl,[curr_trk]
    inc  cl
    shl  cl,3
    add  cl,1
    add  ecx,cdp
    mov  ebx,[ecx]
    and  ecx,0x00ffffff
    mov  ecx,ebx
   ;get_minutes:
   and  ecx,0x000000ff
   mov  eax,ecx
   imul eax,60
   ;get_seconds:
   mov ecx,ebx
   and ecx,0x0000ff00
   shr ecx,8
   add eax,ecx
   ;eax now is next pos in secs
   mov [next_pos_sec],eax
 ;eax now is current pos in secs
    xor  ebx,ebx
    xor  ecx,ecx
    mov  cl,[curr_trk]
    shl  cl,3
    add  cl,1
    add  ecx,cdp
    mov  ebx,[ecx]
    and  ecx,0x00ffffff
    mov  ecx,ebx
   ;get_minutes:
   and  ecx,0x000000ff
   mov  eax,ecx
   imul eax,60
   ;get_seconds:
   mov ecx,ebx
   and ecx,0x0000ff00
   shr ecx,8
   add eax,ecx
   ;eax now is current pos in secs
   mov ecx,[next_pos_sec]
   sub ecx,eax
   ;eax now is length of trk in sec
   mov [curr_trk_length],ecx
;now play that!
    mov ecx,ebx
    mov  eax,24
    mov  ebx,1
    mcall
    ret


play_acd:
    call chk_cdrom
    call read_cd
    call draw_window
    call renew_shuftab
    mov  cl,[curr_trk]
    cmp  cl,0
    jnz  play_acd_trk_ok
    mov  cl,[max_trk]
    mov  [shuftab],cl
    mov  [curr_trk],1
    jmp  playing_no_pause
   play_acd_trk_ok:
;   start_chk_on_pause:
    cmp  [if_paused],TRUE
    jne   pause_playing
    mov  [if_stopped],FALSE
    mov  [if_paused],FALSE
    call current_trk_time
    mov  eax,[curr_trk_pg_time]
    mov  ebx,[paused_time]
    sub  eax,ebx
    imul eax,100
    add  [stimtrk],eax
    call current_trk_time
    call play_from_x_time
    call draw_window
    jmp  end_play_acd
   pause_playing:
    cmp  [curr_trk_pg_time],0
    je   playing_no_pause
    mov  eax,[curr_trk_pg_time]
    mov  [paused_time],eax
    mov  [if_paused],TRUE
    call stop_playing
    call draw_window
    jmp  end_play_acd
   playing_no_pause:
    mov  [if_paused],FALSE
    call rem_time_trk
    call  play_n_track
    call  draw_window
   end_play_acd:
    ret

stop_playing:
    mov eax, 24
    mov ebx,3
    mcall
    mov  cl,[max_trk]
    mov  [shuftab],cl
    mov [if_stopped],TRUE
    cmp [if_paused],TRUE
    je  end_stop_playing
    mov [curr_trk_pg_time],0
   end_stop_playing:
    call draw_window
ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_info:
    ;bar->
    mov eax,13
    mov ebx, 10 shl 16 + 41
    mov ecx,120 shl 16 + 9
    mov edx,0x00ffffff
    mcall
    mov ebx, 96 shl 16 + 11
    mcall
    mov ebx, 185 shl 16 + 11
    mcall
    mov ebx, 200 shl 16 + 11
    mcall
    mov ebx, 150 shl 16 + 11
    mcall
    mov ebx, 165 shl 16 + 11
    mcall
   ;bar<-

    mov  eax,4
    mov  ebx,10 shl 16 +120
    mov  ecx,0x00111111
    cmp  [mode],NORMAL_PLAY
    jne  info_mode_1
    mov  edx,mode_normal
    jmp  info_mode_end
   info_mode_1:
    cmp  [mode],REPEAT_TRACK
    jne  info_mode_2
    mov  edx,mode_repeat_1
    jmp  info_mode_end
   info_mode_2:
    cmp  [mode],REPEAT_DISK
    jne  info_mode_3
    mov  edx,mode_repeat_all
    jmp  info_mode_end
   info_mode_3:
    cmp  [mode],SHUFFLE_DISK
    jne  info_mode_end
    mov  edx,mode_shuffle
;    mov  ecx,0x00aaaaaa
;    mov  cl,[max_trk]
;    mov  [shuftab],cl
    jmp  info_mode_end
   info_mode_end:
    mov  esi,7
    mcall

  ;num info ->
    mov  eax,47
    xor  ebx,ebx
    mov  bl,0
    mov  bh,0
    or   ebx,0x20000            ;X0000 - number of digits to draw
    xor  ecx,ecx
    mov  cl, [curr_trk]            ;number to draw
    mov  edx,96 shl 16 + 120
    mov  esi,0x111111
    mcall
    mov  eax,[curr_trk_pg_time]
    xor  edx,edx
    mov  ecx,60
    div  ecx
    push edx
    mov  ecx,eax
    mov  eax,47
    mov  edx,150 shl 16 + 120
    mcall
    pop ecx
    mov  edx,165 shl 16 + 120
    mcall
    mov  eax,[curr_trk_length]
    xor  edx,edx
    mov  ecx,60
    div  ecx
    push edx
    mov  ecx,eax
    mov  eax,47
    mov  edx,185 shl 16 + 120
    mcall
    pop  ecx
    mov  edx,200 shl 16 + 120
    mcall
   ;num info <-
ret


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx, 50*65536+219         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+168         ; [y start] *65536 + [y size]
    mov  edx,0x04ffffff            ; color of work area RRGGBB
    mov  esi,0x8099bbff            ; color of grab bar  RRGGBB,8->color glide
    mov  edi,0x0099bbee            ; color of frames    RRGGBB
    mcall
                                   ; WINDOW TITLE
    mcall
        mov eax,71
        mov ebx,1
        mov ecx,labelt
        int 40h

    mov  eax,13                    ;bar
    mov  ebx,8 shl 16 + 204
    mov  ecx,28 shl 16 + 84
    mov  edx,0x000fe6f5
    mcall

    ;info ->
    mov  eax,4
    mov  ebx,63 shl 16 + 120
    mov  ecx,0x00111111
    mov  edx,playing_trk_info
    mov  esi,6
    mcall
    mov  ebx,120 shl 16 + 120
    mov  edx,playing_time_info
;    mov  esi,5
    dec  esi
    mcall
    mov  ebx,178 shl 16 + 120
    mov  edx,slash
    mov  esi,1
    mcall
    mov  ebx,196 shl 16 + 120
    mov  edx,column
;    mov  esi,1
    mcall
    mov  ebx,161 shl 16 + 120
    mov  edx,column
;    mov  esi,1
    mcall
    ;info <-

; button  MODE
    mov  eax,8
    mov  ebx,12*65536+20
    mov  ecx,135*65536+20
    mov  edx,7
    mov  esi,COLOR_FUNC_BUTS
    mcall
   ; text
    mov  eax,4
    mov  ebx,19*65536+142
    mov  ecx,0x100f73f5;ffff0f
    mov  edx,but_mode_lab
    mov  esi,1
    mcall

; button  BACK
    mov  eax,8
    mov  ebx,37*65536+20
    mov  ecx,135*65536+20
    mov  edx,6
    mov  esi,COLOR_FUNC_BUTS
    mcall
    mov [coord_x],51
    mov [coord_y],141
    call draw_left_triangle
    mov [coord_x],44
    call draw_vertical_line

; button  NEXT
    mov  eax,8
    mov  ebx,62*65536+20
    mov  ecx,135*65536+20
    mov  edx,5
    mov  esi,COLOR_FUNC_BUTS
    mcall
    mov [coord_x],68
    mov [coord_y],141
    call draw_right_triangle
    mov [coord_x],74
    call draw_vertical_line

; button  REWIND
    mov  eax,8
    mov  ebx,87*65536+20
    mov  ecx,135*65536+20
    mov  edx,8
    mov  esi,COLOR_FUNC_BUTS
    mcall
    mov [coord_x],102
    mov [coord_y],141
    call draw_left_triangle
    mov [coord_x],97
    call draw_left_triangle

; button   STOP
    mov  eax,8
    mov  ebx,112*65536+20
    mov  ecx,135*65536+20
    mov  edx,3
    mov  esi,COLOR_FUNC_BUTS
    mcall
    mov [coord_x],118
    mov [coord_y],142
    call draw_square


; button  PLAY
    mov  eax,8
    mov  ebx,137*65536+20
    mov  ecx,135*65536+20
    mov  edx,2
    mov  esi,COLOR_FUNC_BUTS
    mcall
    cmp [if_stopped],TRUE
    je  playing_paused
    cmp [if_paused],TRUE
    je  playing_paused
    mov [coord_x],144
    mov [coord_y],141
    call draw_vertical_line
    mov [coord_x],149
    call draw_vertical_line
    jmp end_draw_play
   playing_paused:
    mov [coord_x],144
    mov [coord_y],141
    call draw_right_triangle
   end_draw_play:


; button   FORWARD
    mov  eax,8
    mov  ebx,162*65536+20
    mov  ecx,135*65536+20
    mov  edx,9
    mov  esi,COLOR_FUNC_BUTS
    mcall
    mov [coord_x],167
    mov [coord_y],141
    call draw_right_triangle
    mov [coord_x],172
    call draw_right_triangle

; button  RE-READ PLAYLIST
    mov  eax,8
    mov  ebx,187*65536+20
    mov  ecx,135*65536+20
    mov  edx,4
    mov  esi,COLOR_FUNC_BUTS
    mcall
    mov  [coord_x],192
    mov  [coord_y],140
    call draw_vert_list_line
    dec  [coord_y]
    call draw_hor_list_line
    mov  [coord_y], 151
    call draw_hor_list_line
    mov  [coord_x],202
    mov  [coord_y],140
    call draw_vert_list_line
    mov  [coord_x],195
    mov  [coord_y], 142
    call draw_str_list_line
    mov  [coord_y],145
    call draw_str_list_line
    mov  [coord_y],148
    call draw_str_list_line

    cmp  [flag],1
    jne  flag2
;Draw tracs buttons
    xor  eax,eax
    xor  ebx,ebx
    mov  ecx,10
    mov  al,[cdp+3]
    mov  [max_trk],al
    xor  edi,edi
    mov  di,ax
    mov  [posx],12
    mov  [posy],32
    mov  [tracs],1
 draw_tracs_buttons:
    mov  eax,8
    xor  ebx,ebx
    mov  bl,[posx]
    shl  ebx,16
    add  ebx,15
    xor  ecx,ecx
    mov  cl,[posy]
    shl  ecx,16
    add  ecx,15
    xor  edx,edx
    mov  dx,[tracs]
    add  edx,10
    mov  esi,0xaaaaaa
    add  esi,edi
    mcall
   ;---draw tracs numbers
    mov  eax,47
    xor  ebx,ebx
    mov  bl,0
    or   ebx,0x20000            ;number of digits to draw
    xor  ecx,ecx
    mov  cx, [tracs]            ;number to draw
    xor  edx,edx
    mov  dl,[posx]
    add  dl,3
    shl  edx,16
    add  dl,[posy]
    add  dl,5
    mov  esi,0xffffff
    mcall
   ;---
    mov  al,[posx]
    add  al,20
    mov  [posx],al
    xor  eax,eax
    mov  ax,[tracs]
    mov  bl,10
    div  bl
    cmp  ah,0
    jnz  no_new_str
    mov  al,[posxstart]
    mov  [posx], al
    mov  al,[posy]
    add  al,20
    mov  [posy],al
  no_new_str:
    inc  [tracs]
    cmp  [tracs],41
    je   flag2
    dec  edi
    cmp  edi,0
    jnz  draw_tracs_buttons

  flag2:
    cmp [flag],2
    jne flag3
    mov eax,4
    mov ebx, 20 shl 16 +67
    mov ecx,0x10ffff00
    mov edx,define_cdrom
    mov esi,define_cdrom_len-define_cdrom
    mcall
  flag3:
    cmp [flag],3
    jne flag4
    mov eax,4
    mov ebx, 47 shl 16 +67
    mov ecx,0x10ffff00
    mov edx,no_cda
    mov esi,no_cda_len-no_cda
    mcall
  flag4:
    cmp [flag],4
    jne flag5
   ;help screen
   cmp [help_screen],1
   jnz @hs2
   mov edx,help1
   jmp @ehs
  @hs2:
   cmp [help_screen],2
   jnz @hs3
   mov edx,help2
   jmp @ehs
  @hs3:
   mov edx,help3
  @ehs:
   xor edi,edi
   mov ebx,25*65536+30
  new_line:
   mov eax,4
   mov ecx,0x111111
   mov esi,31
   mcall
  noline:
   add ebx,10
   add edx,31
   inc edi
   cmp [edx],byte 'x'
   jnz new_line
  flag5:
  call draw_info

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

ret

draw_right_triangle:
    mov ebx,[coord_x]
    mov ecx,[coord_y]
    mov edx,0x00111111
    mov esi,5
    mov eax,9
   start_draw_pixel:
    push ebx
    cmp  eax,5
    jb   y_menshe_5
    mov  esi,10
    sub  esi,eax
    jmp  draw_pixel
   y_menshe_5:
    mov  esi,eax
   draw_pixel:
    dec  esi
    inc  ebx
    push eax
    mov  eax,1
    mcall
    pop  eax
    cmp  esi,0
    jne  draw_pixel
    pop  ebx
    dec  eax
    inc  ecx
    cmp  eax,0
    jne  start_draw_pixel
ret

draw_square:
    mov ebx,[coord_x]
    mov ecx,[coord_y]
    mov edx,0x00111111
    mov eax,7
   q_start_draw_pixel:
    push ebx
    mov  esi,7
   q_draw_pixel:
    dec  esi
    inc  ebx
    push eax
    mov  eax,1
    mcall
    pop  eax
    cmp  esi,0
    jne  q_draw_pixel
    pop  ebx
    dec  eax
    inc  ecx
    cmp  eax,0
    jne  q_start_draw_pixel
ret

draw_left_triangle:
    mov ebx,[coord_x]
    mov ecx,[coord_y]
    mov edx,0x00111111
    mov esi,5
    mov eax,9
   l_start_draw_pixel:
    push ebx
    cmp  eax,5
    jb   l_y_menshe_5
    mov  esi,10
    sub  esi,eax
    jmp  l_draw_pixel
   l_y_menshe_5:
    mov  esi,eax
   l_draw_pixel:
    dec  esi
    dec  ebx
    push eax
    mov  eax,1
    mcall
    pop  eax
    cmp  esi,0
    jne  l_draw_pixel
    pop  ebx
    dec  eax
    inc  ecx
    cmp  eax,0
    jne  l_start_draw_pixel
ret

draw_vertical_line:
    mov  eax,2
    mov  ebx,[coord_x]
    mov  edx,0x00111111
  @@draw_2_line:
    mov  ecx,[coord_y]
    dec  ecx
    mov  esi,9
   start_draw_vline:
    inc  ecx
    push eax
    mov  eax,1
    mcall
    pop  eax
    dec  esi
    cmp  esi,0
    jne  start_draw_vline
    dec  eax
    inc ebx
    cmp  eax,0
    jne  @@draw_2_line
ret

draw_vert_list_line:
    mov  eax,1
    mov  ebx,[coord_x]
    mov  edx,0x00111111
    mov  ecx,[coord_y]
    dec  ecx
    mov  esi,11
   vlstart_draw_vline:
    inc  ecx
    mcall
    dec  esi
    cmp  esi,0
    jne  vlstart_draw_vline
    dec  eax
    inc ebx
ret

draw_hor_list_line:
    mov  eax,1
    mov  ebx,[coord_x]
    mov  edx,0x00111111
    mov  ecx,[coord_y]
    dec  ebx
    mov  esi,11
   hlstart_draw_vline:
    inc  ebx
    mcall
    dec  esi
    cmp  esi,0
    jne  hlstart_draw_vline
    dec  eax
    inc ebx
ret

draw_str_list_line:
    mov  eax,1
    mov  ebx,[coord_x]
    mov  edx,0x00111111
    mov  ecx,[coord_y]
    dec  ebx
    mov  esi,5
   slstart_draw_vline:
    inc  ebx
    mcall
    dec  esi
    cmp  esi,0
    jne  slstart_draw_vline
    dec  eax
    inc ebx
ret


 chk_cdrom:
    mov eax,24
    mov ebx,1
    mcall
    cmp eax,0
    je chk_cdrom_ok
    mov [flag],2
    call draw_window
    jmp  chk_cdrom_end
   chk_cdrom_ok:
    mov [flag],0
   chk_cdrom_end:
ret

read_cd:
    mov [if_stopped],TRUE
    push ax
    cmp [flag],2
    je  read_cd_end
    mov al,101
    mov [cdp+3],al
    mov eax,24
    mov ebx,2
    mov ecx, cdp
    mov edx,321
    mcall
    mov [flag],1
    mov al,100
    cmp [cdp+3],al
    jb  read_cd_end
    mov [flag],3
    call draw_window
 read_cd_end:
    pop ax
ret

get_uptime:
    push ebx
    mov  eax,26
    mov  ebx,9
    mcall
    pop  ebx
ret

; DATA AREA

paused_time dd 0
if_paused db FALSE
coord_x dd 0
coord_y dd 0
flag db 0
tracs dw 1
posx db 12
posy db 32
posxstart db 12
curr_trk db 0
max_trk db 0
stimtrk dd 0
help_screen db 0
next_pos_sec dd 0
curr_trk_length dd 0
curr_trk_pg_time dd 0
was_it_ok db FALSE
if_stopped db FALSE
mode db NORMAL_PLAY

shuftab  db 00,01,02,03,04,05,06,07,08,09
         db 10,11,12,13,14,15,16,17,18,19
         db 20,21,22,23,24,25,26,27,28,29
         db 30,31,32,33,34,35,36,37,38,39
         db 40

but_mode_lab: db 'M'

playing_time_info: db 'Time '
slash  db '/'
column db ':'
mode_normal db     'Normal '
mode_repeat_1 db   'Rep trk'
mode_repeat_all db 'Rep all'
mode_shuffle db    'Shuffle'
playing_trk_info: db 'Track '

define_cdrom: db 'Please, define your CD-ROM'
define_cdrom_len:

no_cda: db 'Audio CD not found'
no_cda_len:

labelt:
   db   'CD player',0
labellen:

help1: db 'HotKeys:                       '
       db 'H - this screen (Help)         '
       db 'P - Play/Pause current track   '
       db 'S - Stop playing               '
       db 'L - re-read playList           '
       db 'N - play Next track            '
       db 'B - play previous track (Back) '
       db '                        next ->'
       db 'x'
help2: db 'HotKeys:                       '
       db 'F - fast Forward track         '
       db 'R - fast Rewind track          '
       db 'M - change Mode                '
       db '                               '
       db '                               '
       db '                               '
       db '<- prev                 next ->'
       db 'x'
help3: db 'About:                         '
       db 'Audio CD Player ver 1.1beta-2  '
       db 'All questions, wishes and      '
       db 'advices please send to:        '
       db '       E-mail:  dma@bn.by      '
       db '       FidoNet: 2:450/258.75   '
       db '                               '
       db '<- prev                        '
       db 'x'
cdp:

 I_END:
