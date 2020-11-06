; Initially written by CH@YKIN EVGENY, 2009
; Then disassembled for fun by dunkaist, 2017
use32
    org 0
    db  'MENUET01'
    dd  1
    dd  start
    dd  I_END
    dd  stacktop
    dd  stacktop
    dd  0, 0

include 'proc32.inc'
include 'struct.inc'
include 'macros.inc'
include 'encoding.inc'

CLIENT_WIDTH  = 300
CLIENT_HEIGHT = 200

HEALTH_INIT = 24
HEALTH_BONUS = 3
HEALTH_LEVEL_UP = 297

BULLET_DAMAGE = 12
BULLETS_CNT = 4

NOTE_DAMAGE = 0x31
NOTE_SAVE = 0x63
NOTE_WIN_0 = 0x36
NOTE_WIN_1 = 0x35
NOTE_WIN_2 = 0x31
NOTE_LOST_0 = 0x0d
NOTE_LOST_1 = 0x09
NOTE_LOST_2 = 0x20

BULLET_SIZE = 7
SHIELD_SIZE = 20

DELAY_INIT = 10
DELAY_NOTE = 10

DELAYS_PER_GAME_STEP = 10

NW = 1  ; north west
SW = 2
NE = 3
SE = 4

macro draw_bullet off_on, x, y {
        mov     eax, 7
        mov     ebx, _pic_bullet_#off_on
        mov     ecx, (BULLET_SIZE SHL 16) + BULLET_SIZE
        mov     edx, x
        imul    edx, edx, 0x10000
        add     edx, y
        mcall
}

macro draw_shield side, x, y {
        mov     eax, 7
        mov     ebx, pic_shield_#side
        mov     ecx, (SHIELD_SIZE SHL 16) + SHIELD_SIZE
        mov     edx, x
        imul    edx, edx, 0x10000
        add     edx, y
        mcall
}

macro play_note note {
        mov     eax, note
        mov     dword [speaker.note], eax
        mov     eax, 55
        mov     ebx, eax
        mov     esi, speaker
        mcall
}

start:
        mcall   48, 3, sys_colors, sizeof.system_colors
        jmp     menu
menu:
        mcall   10
        cmp     eax, 1
        jz      .redraw
        cmp     eax, 2
        jz      .button
        cmp     eax, 3
        jz      .key
        jmp     menu
  .redraw:
        jmp     .draw
  .button:
        mcall   2
        cmp     ah, '1'
        jz      .init_game
        cmp     ah, '2'
        jz      .resume_game
        jmp     menu
  .init_game:
        mov     eax, 13
        mov     ebx, 0
        imul    ebx, ebx, 0x10000
        add     ebx, CLIENT_WIDTH
        mov     ecx, 0
        imul    ecx, ecx, 0x10000
        add     ecx, CLIENT_HEIGHT
        mov     edx, 0
        mcall
        mov     [game_started], 0
        mov     [shield_position], SE
        mov     [finish_draw], 1
        mov     [next_bullet_side_prng], 3
        mov     [next_bullet_side], SE
        mov     [bullet_1_pos], 1
        mov     [bullet_2_pos], 2
        mov     [bullet_3_pos], 3
        mov     [bullet_4_pos], 4
        mov     eax, NW
        mov     [bullet_1_side], eax
        mov     eax, SW
        mov     [bullet_2_side], eax
        mov     eax, NE
        mov     [bullet_3_side], eax
        mov     eax, SE
        mov     [bullet_4_side], eax
        mov     eax, 1
        mov     [bullet_pos], eax
        mov     eax, NW
        mov     [bullet_side], eax
        mov     eax, 1
        mov     [cur_bullet], eax
        mov     eax, 1
        mov     [delays_cnt], eax
        mov     eax, HEALTH_INIT
        mov     [health], eax
        mov     eax, DELAY_INIT
        mov     [delay], eax
        jmp     redraw_window
  .resume_game:
        mov     eax, 13
        mov     ebx, 0
        imul    ebx, ebx, 0x10000
        add     ebx, CLIENT_WIDTH
        mov     ecx, 0
        imul    ecx, ecx, 0x10000
        add     ecx, CLIENT_HEIGHT
        mov     edx, 0
        mcall
        jmp     redraw_window
  .key:
        mcall   17
        cmp     ah, 0x01        ; close button
        jnz     menu
  .exit:
        mcall   -1
  .draw:
        mov     eax, [finish_draw]
        cmp     eax, 1
        jz      .draw.start
        jmp     .draw.finish
  .draw.finish:
        mcall   12, 2
        jmp     .draw.start
  .draw.start:
        mov     eax, 0
        mov     [finish_draw], eax
        mcall   12, 1
        mcall   48, 4
        mov     [skin_height], eax
        mcall   48, 4
        mov     [skin_height], eax
        mov     eax, 0
        mov     ebx, (100 SHL 16) + CLIENT_WIDTH + 10
        mov     ecx, 100
        imul    ecx, ecx, 0x10000
        add     ecx, CLIENT_HEIGHT + 4
        add     ecx, [skin_height]
        mov     edx, 0x33000000
        mov     edi, window_title
        mcall
        mov     eax, 13
        mov     ebx, 0
        imul    ebx, ebx, 0x10000
        add     ebx, CLIENT_WIDTH + 1
        mov     ecx, 0
        imul    ecx, ecx, 0x10000
        add     ecx, CLIENT_HEIGHT
        mov     edx, 0
        mcall
        mcall   4, (140 SHL 16) +  60, 0x10bbbbbb, msg_ataka, msg_ataka.size
        mcall   4, (140 SHL 16) +  61, 0x10bbbbbb, msg_ataka, msg_ataka.size
        mcall   4, (141 SHL 16) +  61, 0x10bbbbbb, msg_ataka, msg_ataka.size
        mcall   4, (141 SHL 16) +  60, 0x10bbbbbb, msg_ataka, msg_ataka.size
        mcall   4, ( 55 SHL 16) + 190, 0x10444444, msg_copyright, msg_copyright.size
        mov     eax, 4
        mov     ebx, (34 SHL 16) + 160
        mov     ecx, 0x888888
        mov     edx, msg_control_by_numpad
        mov     esi, 40
  .print_line:
        mcall
        add     ebx, 10 ; one line down
        add     edx, 40
        cmp     byte [edx], 'x'
        jnz     .print_line
        mcall   4, (110 SHL 16) +  95, 0x10999999, msg_new_game, msg_new_game.size
        mov     eax, [game_started]
        cmp     eax, 0
        jz      menu
        mcall   4, (110 SHL 16) + 110, 0x10999999, msg_continue, msg_continue.size
        jmp     menu

redraw_window:
        mov     eax, [finish_draw]
        cmp     eax, 1
        jz      redraw_window.start
        mcall   12, 2
        jmp     redraw_window.start
redraw_window.start:
        mov     eax, 0
        mov     [finish_draw], eax
        mcall   12, 1
        mcall   48, 4
        mov     [skin_height], eax
        mov     eax, 0
        mov     ebx, (100 SHL 16) + CLIENT_WIDTH + 10
        mov     ecx, 100
        imul    ecx, ecx, 0x10000
        add     ecx, CLIENT_HEIGHT + 4
        add     ecx, [skin_height]
        mov     edx, 0x33000000
        mov     edi, window_title
        mcall
        jmp     draw_field

game_loop:
        mov     [game_started], 1
        mov     eax, [health]
        cmp     eax, HEALTH_LEVEL_UP
        jg      level_up
        cmp     eax, 0
        jl      lost
        jmp     game_alive
level_up:
        mov     eax, HEALTH_INIT
        mov     [health], eax
        mov     eax, [delay]
        cmp     eax, 10         ; delay for level_0
        jz      level_up_1
        mov     eax, [delay]
        cmp     eax, 8          ; delay for level_1
        jz      level_up_2
        mov     eax, [delay]
        cmp     eax, 7
        jz      level_up_3
        mov     eax, [delay]
        cmp     eax, 6
        jz      level_up_4
        mov     eax, [delay]
        cmp     eax, 5
        jz      level_up_5
        mov     eax, [delay]
        cmp     eax, 4
        jz      level_up_6
        mov     eax, [delay]
        cmp     eax, 3
        jz      win
        jmp     game_alive
lost:
        mcall   5, DELAY_NOTE
        play_note NOTE_LOST_0
        mcall   5, DELAY_NOTE
        play_note NOTE_LOST_1
        mcall   5, DELAY_NOTE
        play_note NOTE_LOST_2

        mov     [game_started], 0
        mcall   4, (105 SHL 16) + 100, 0xff0000, msg_you_lost, msg_you_lost.size
        mcall   5, 100
        mov     eax, SE
        mov     [shield_position], eax
        mov     eax, 1
        mov     [finish_draw], eax
        mov     eax, 3
        mov     [next_bullet_side_prng], eax
        mov     eax, SE
        mov     [next_bullet_side], eax
        mov     eax, 1
        mov     [bullet_1_pos], eax
        mov     eax, 2
        mov     [bullet_2_pos], eax
        mov     eax, 3
        mov     [bullet_3_pos], eax
        mov     eax, 4
        mov     [bullet_4_pos], eax
        mov     eax, NW
        mov     [bullet_1_side], eax
        mov     eax, SW
        mov     [bullet_2_side], eax
        mov     eax, NE
        mov     [bullet_3_side], eax
        mov     eax, SE
        mov     [bullet_4_side], eax
        mov     eax, 1
        mov     [bullet_pos], eax
        mov     eax, NW
        mov     [bullet_side], eax
        mov     eax, 1
        mov     [cur_bullet], eax
        mov     eax, 1
        mov     [delays_cnt], eax
        mov     eax, HEALTH_INIT
        mov     [health], eax
        mov     eax, DELAY_INIT
        mov     [delay], eax
        jmp     menu.draw

level_up_1:
        mov     eax, 8
        mov     [delay], eax
        jmp     game_alive
level_up_2:
        mov     eax, 7
        mov     [delay], eax
        jmp     game_alive
level_up_3:
        mov     eax, 6
        mov     [delay], eax
        jmp     game_alive
level_up_4:
        mov     eax, 5
        mov     [delay], eax
        jmp     game_alive
level_up_5:
        mov     eax, 4
        mov     [delay], eax
        jmp     game_alive
level_up_6:
        mov     eax, 3
        mov     [delay], eax
        jmp     game_alive
win:
        mcall   5, DELAY_NOTE
        play_note NOTE_WIN_0
        mcall   5, DELAY_NOTE
        play_note NOTE_WIN_1
        mcall   5, DELAY_NOTE
        play_note NOTE_WIN_2

        mcall   5, DELAY_NOTE

        mov     [game_started], 0
        mov     eax, DELAY_INIT
        mov     [delay], eax
        mcall   4, (105 SHL 16) + 100, 0xff0000, msg_you_won, msg_you_won.size
        mcall   5, 200
        jmp     menu.draw

game_alive:
        ; draw max health bar
        mov     eax, 13
        mov     ebx, 0
        imul    ebx, ebx, 0x10000
        add     ebx, CLIENT_WIDTH + 1
        mov     ecx, 0
        imul    ecx, ecx, 0x10000
        inc     ecx
        mov     edx, 0xff0000
        mcall
        ; draw left health bar
        mov     eax, 13
        mov     ebx, 0
        imul    ebx, ebx, 0x10000
        add     ebx, [health]
        mov     ecx, 0
        imul    ecx, ecx, 0x10000
        inc     ecx
        mov     edx, 0x0000ff
        mcall
        mov     eax, [next_bullet_side_prng]
        inc     eax
        mov     [next_bullet_side_prng], eax
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 1
        jz      ._1
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 2
        jz      ._2
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 3
        jz      ._3
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 4
        jz      ._4
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 5
        jz      ._5
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 6
        jz      ._6
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 7
        jz      ._7
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 8
        jz      ._8
        mov     eax, [next_bullet_side_prng]
        cmp     eax, 9
        jg      ._9
        mov     eax, 3
        mov     [next_bullet_side], eax
        jmp     check_event
._1:
        mov     eax, 1
        mov     [next_bullet_side], eax
        jmp     check_event
._2:
        mov     eax, 4
        mov     [next_bullet_side], eax
        jmp     check_event
._3:
        mov     eax, 3
        mov     [next_bullet_side], eax
        jmp     check_event
._4:
        mov     eax, 4
        mov     [next_bullet_side], eax
        jmp     check_event
._5:
        mov     eax, 1
        mov     [next_bullet_side], eax
        jmp     check_event
._6:
        mov     eax, 2
        mov     [next_bullet_side], eax
        jmp     check_event
._7:
        mov     eax, 1
        mov     [next_bullet_side], eax
        jmp     check_event
._8:
        mov     eax, 2
        mov     [next_bullet_side], eax
        jmp     check_event
._9:
        mov     eax, 3
        mov     [next_bullet_side], eax
        mov     eax, 1
        mov     [next_bullet_side_prng], eax
        jmp     check_event

check_event:
        mcall   5, [delay]
        mcall   11
        cmp     eax, 1
        jz      redraw_window
        cmp     eax, 2
        jz      .button
        cmp     eax, 3
        jz      .key
        jmp     draw_field
  .key:
        mov     al, 17
        mcall
        cmp     ah, 1
        jnz     draw_field
        mcall   -1

  .button:
        mov     al, 2
        mcall
        cmp     ah, 0xb0        ; arrow left / numpad 4
        jz      .move_shield_tl
        cmp     ah, 0xb5        ; end / numpad 1
        jz      .move_shield_bl
        cmp     ah, 0xb3        ; arrow right / numpad 6
        jz      .move_shield_tr
        cmp     ah, 0xb7        ; page down / numpad 3
        jz      .move_shield_br
        cmp     ah, 0xb0        ; never happens, fixme
        jz      menu.draw
        cmp     ah, 'p'         ; 'p' for pause
        jz      menu.draw
        cmp     ah, '4'
        jz      .move_shield_tl
        cmp     ah, '1'
        jz      .move_shield_bl
        cmp     ah, '6'
        jz      .move_shield_tr
        cmp     ah, '3'
        jz      .move_shield_br
        jmp     draw_field
  .move_shield_tl:
        mov     eax, 1
        mov     [shield_position], eax
        jmp     draw_field
  .move_shield_bl:
        mov     eax, 2
        mov     [shield_position], eax
        jmp     draw_field
  .move_shield_tr:
        mov     eax, 3
        mov     [shield_position], eax
        jmp     draw_field
  .move_shield_br:
        mov     eax, 4
        mov     [shield_position], eax
        jmp     draw_field
draw_field:
        mcall   18, 14
;        mcall   65, pic_bg, (CLIENT_WIDTH SHL 16) + CLIENT_HEIGHT, 0, 8, paletter, 0
        mov     eax, 65
        mov     ebx, pic_bg
        mov     ecx, (CLIENT_WIDTH SHL 16) + CLIENT_HEIGHT
        mov     edx, 0
        mov     esi, 8  ; indexed image
        mov     edi, palette
        mov     ebp, 0
        mcall
        mov     eax, [shield_position]
        cmp     eax, NW
        jz      .shield_nw
        cmp     eax, SW
        jz      .shield_sw
        cmp     eax, NE
        jz      .shield_ne
        cmp     eax, SE
        jz      .shield_se
  .shield_nw:
        draw_shield nw, 90, 70
        jmp     draw_bullets
  .shield_sw:
        draw_shield sw, 90, 105
        jmp     draw_bullets
  .shield_ne:
        draw_shield ne, CLIENT_HEIGHT, 70
        jmp     draw_bullets
  .shield_se:
        draw_shield se, CLIENT_HEIGHT, 105
        jmp     draw_bullets

draw_bullets:   ; off
        draw_bullet off,  90,  70
        draw_bullet off,  75,  58
        draw_bullet off,  59,  46
        draw_bullet off,  89, 117
        draw_bullet off,  74, 129
        draw_bullet off,  56, 144
        draw_bullet off, 211,  68
        draw_bullet off, 224,  57
        draw_bullet off, 236,  46
        draw_bullet off, 210, 119
        draw_bullet off, 223, 131
        draw_bullet off, 237, 143
        jmp     draw_bullets_on

draw_bullets_on:
        mov     eax, [cur_bullet]
        cmp     eax, 1
        jz      ._1
        cmp     eax, 2
        jz      ._2
        cmp     eax, 3
        jz      ._3
        cmp     eax, 4
        jz      ._4
  ._1:
        mov     eax, [bullet_1_pos]
        mov     [bullet_pos], eax
        mov     eax, [bullet_1_side]
        mov     [bullet_side], eax
        jmp     bullet_on_defined
  ._2:
        mov     eax, [bullet_2_pos]
        mov     [bullet_pos], eax
        mov     eax, [bullet_2_side]
        mov     [bullet_side], eax
        jmp     bullet_on_defined
  ._3:
        mov     eax, [bullet_3_pos]
        mov     [bullet_pos], eax
        mov     eax, [bullet_3_side]
        mov     [bullet_side], eax
        jmp     bullet_on_defined
  ._4:
        mov     eax, [bullet_4_pos]
        mov     [bullet_pos], eax
        mov     eax, [bullet_4_side]
        mov     [bullet_side], eax
        jmp     bullet_on_defined

bullet_on_defined:
        mov     eax, [bullet_side]
        cmp     eax, NW
        jz      bullet_nw
        cmp     eax, SW
        jz      bullet_sw
        cmp     eax, NE
        jz      bullet_ne
        cmp     eax, SE
        jz      bullet_se
bullet_nw:
        mov     eax, [bullet_pos]
        cmp     eax, 1
        jz      bullet_nw_dist._1
        cmp     eax, 2
        jz      bullet_nw_dist._2
        cmp     eax, 3
        jz      bullet_nw_dist._3
        cmp     eax, 4
        jz      bullet_nw_dist._4
        cmp     eax, 5
        jz      bullet_nw_dist._5
bullet_sw:
        mov     eax, [bullet_pos]
        cmp     eax, 1
        jz      bullet_sw_dist._1
        cmp     eax, 2
        jz      bullet_sw_dist._2
        cmp     eax, 3
        jz      bullet_sw_dist._3
        cmp     eax, 4
        jz      bullet_sw_dist._4
        cmp     eax, 5
        jz      bullet_sw_dist._5
bullet_ne:
        mov     eax, [bullet_pos]
        cmp     eax, 1
        jz      bullet_ne_dist._1
        cmp     eax, 2
        jz      bullet_ne_dist._2
        cmp     eax, 3
        jz      bullet_ne_dist._3
        cmp     eax, 4
        jz      bullet_ne_dist._4
        cmp     eax, 5
        jz      bullet_ne_dist._5
bullet_se:
        mov     eax, [bullet_pos]
        cmp     eax, 1
        jz      bullet_se_dist._1
        cmp     eax, 2
        jz      bullet_se_dist._2
        cmp     eax, 3
        jz      bullet_se_dist._3
        cmp     eax, 4
        jz      bullet_se_dist._4
        cmp     eax, 5
        jz      bullet_se_dist._5

bullet_nw_dist:
  ._1:
        draw_bullet on, 44, 33
        jmp     bullet_on_drawn
  ._2:
        draw_bullet on, 59, 46
        jmp     bullet_on_drawn
  ._3:
        draw_bullet on, 75, 58
        jmp     bullet_on_drawn
  ._4:
        draw_bullet on, 90, 70
        jmp     bullet_on_drawn
  ._5:
        jmp     bullet_on_drawn

bullet_sw_dist:
  ._1:
        draw_bullet on, 42, 156
        jmp     bullet_on_drawn
  ._2:
        draw_bullet on, 56, 144
        jmp     bullet_on_drawn
  ._3:
        draw_bullet on, 74, 129
        jmp     bullet_on_drawn
  ._4:
        draw_bullet on, 89, 117
        jmp     bullet_on_drawn
  ._5:
        jmp     bullet_on_drawn

bullet_ne_dist:
  ._1:
        draw_bullet on, 250, 34
        jmp     bullet_on_drawn
  ._2:
        draw_bullet on, 236, 46
        jmp     bullet_on_drawn
  ._3:
        draw_bullet on, 224, 57
        jmp     bullet_on_drawn
  ._4:
        draw_bullet on, 211, 68
        jmp     bullet_on_drawn
  ._5:
        jmp     bullet_on_drawn

bullet_se_dist:
  ._1:
        draw_bullet on, 251, 156
        jmp     bullet_on_drawn
  ._2:
        draw_bullet on, 237, 143
        jmp     bullet_on_drawn
  ._3:
        draw_bullet on, 223, 131
        jmp     bullet_on_drawn
  ._4:
        draw_bullet on, 210, 119
        jmp     bullet_on_drawn
  ._5:
        jmp     bullet_on_drawn

bullet_on_drawn:
        mov     eax, [cur_bullet]
        inc     eax
        mov     [cur_bullet], eax
        mov     eax, [cur_bullet]
        cmp     eax, BULLETS_CNT + 1
        jz      all_bullets_drawn
        jmp     draw_bullets_on
all_bullets_drawn:
        mov     eax, 1
        mov     [cur_bullet], eax
        jmp     check_for_game_step
check_for_game_step:
        mov     eax, [delays_cnt]
        cmp     eax, DELAYS_PER_GAME_STEP
        jz      game_step
        jmp     no_game_step
no_game_step:
        mov     eax, [delays_cnt]
        inc     eax
        mov     [delays_cnt], eax
        jmp     game_loop
game_step:
        mov     eax, 1
        mov     [delays_cnt], eax
        mov     eax, [bullet_1_pos]
        inc     eax
        mov     [bullet_1_pos], eax
        mov     eax, [bullet_2_pos]
        inc     eax
        mov     [bullet_2_pos], eax
        mov     eax, [bullet_3_pos]
        inc     eax
        mov     [bullet_3_pos], eax
        mov     eax, [bullet_4_pos]
        inc     eax
        mov     [bullet_4_pos], eax
        mov     eax, [bullet_1_pos]
        cmp     eax, 6
        jz      bullet_1_new
        mov     eax, [bullet_2_pos]
        cmp     eax, 6
        jz      bullet_2_new
        mov     eax, [bullet_3_pos]
        cmp     eax, 6
        jz      bullet_3_new
        mov     eax, [bullet_4_pos]
        cmp     eax, 6
        jz      bullet_4_new
        jmp     check_bullet_hit

bullet_1_new:
        play_note NOTE_SAVE
        mov     eax, 1
        mov     [bullet_1_pos], eax
        mov     eax, [next_bullet_side]
        mov     [bullet_1_side], eax
        jmp     check_bullet_hit
bullet_2_new:
        play_note NOTE_SAVE
        mov     eax, 1
        mov     [bullet_2_pos], eax
        mov     eax, [next_bullet_side]
        mov     [bullet_2_side], eax
        jmp     check_bullet_hit
bullet_3_new:
        play_note NOTE_SAVE
        mov     eax, 1
        mov     [bullet_3_pos], eax
        mov     eax, [next_bullet_side]
        mov     [bullet_3_side], eax
        jmp     check_bullet_hit
bullet_4_new:
        play_note NOTE_SAVE
        mov     eax, 1
        mov     [bullet_4_pos], eax
        mov     eax, [next_bullet_side]
        mov     [bullet_4_side], eax
        jmp     check_bullet_hit

check_bullet_hit:
        mov     eax, [bullet_1_pos]
        cmp     eax, 5
        jz      bullet_1_hit
        mov     eax, [bullet_2_pos]
        cmp     eax, 5
        jz      bullet_2_hit
        mov     eax, [bullet_3_pos]
        cmp     eax, 5
        jz      bullet_3_hit
        mov     eax, [bullet_4_pos]
        cmp     eax, 5
        jz      bullet_4_hit
        jmp     game_loop

bullet_1_hit:
        mov     eax, [bullet_1_side]
        cmp     eax, [shield_position]
        jz      save
        jmp     damage
bullet_2_hit:
        mov     eax, [bullet_2_side]
        cmp     eax, [shield_position]
        jz      save
        jmp     damage
bullet_3_hit:
        mov     eax, [bullet_3_side]
        cmp     eax, [shield_position]
        jz      save
        jmp     damage
bullet_4_hit:
        mov     eax, [bullet_4_side]
        cmp     eax, [shield_position]
        jz      save
        jmp     damage

damage:
        mov     eax, [health]
        sub     eax, BULLET_DAMAGE
        mov     [health], eax
        play_note NOTE_DAMAGE
        jmp     game_loop
save:
        mov     eax, [health]
        add     eax, HEALTH_BONUS
        mov     [health], eax
        play_note NOTE_SAVE
        jmp     game_loop

skin_height dd 0
shield_position dd SE
finish_draw dd 1        ; 0 -- finish, 1 -- start
next_bullet_side_prng dd 3
next_bullet_side dd SE
bullet_1_pos dd 1
bullet_2_pos dd 2
bullet_3_pos dd 3
bullet_4_pos dd 4
bullet_1_side dd NW
bullet_2_side dd SW
bullet_3_side dd NE
bullet_4_side dd SE
bullet_pos dd 1         ; nearest to the gun
bullet_side dd NW
cur_bullet dd NW
delays_cnt dd 1
health dd HEALTH_INIT
delay dd DELAY_INIT
game_started dd 0
speaker:
  .duration db 0x90
  .note     db 0x30
  .end      db 0x00
pic_bg:
file 'picture_bg.rgba'
pic_shield_sw:
file 'shield_sw.rgb'
pic_shield_nw:
file 'shield_nw.rgb'
pic_shield_ne:
file 'shield_ne.rgb'
pic_shield_se:
file 'shield_se.rgb'
_pic_bullet_off:
file 'bullet_off.rgb'
_pic_bullet_on:
file 'bullet_on.rgb'
sz msg_copyright, '(c) 2009 CH@YKIN EVGENY'
msg_new_game      cp866 '1  новая игра'
msg_new_game.size = $ - msg_new_game
msg_continue      cp866 '2  продолжить'
msg_continue.size = $ - msg_continue
msg_you_lost    cp866 '   вы проиграли'
msg_you_lost.size = $ - msg_you_lost
msg_you_won     cp866 'ПОБЕДА :)'
msg_you_won.size = $ - msg_you_won
msg_control_by_numpad cp866 'Управление клавишами  NumLock клавиатуры'
msg_control_by_numpad.size = $ - msg_control_by_numpad
msg_controls    cp866 '      1, 4, 6, 3 и пауза Р(Eng).                                                x'
msg_controls.size = $ - msg_controls
palette:
file 'palette.rgba'

sz msg_ataka,     'ATAKA'
window_title      db 'ATAKA  V 1.0',0
I_END:
sys_colors system_colors
rb 0x80d
stacktop:
