;=============================================================================;
; Hidnplayr's invaders for Kolibrios                                          ;
;-----------------------------------------------------------------------------;
;                                                                             ;
; Copyright (C) hidnplayr 2007-2014. All rights reserved.                     ;
;                                                                             ;
; Invaders is distributed in the hope that it will be useful, but WITHOUT ANY ;
; WARRANTY. No author or distributor accepts responsibility to anyone for the ;
; consequences of using it or for whether it serves any particular purpose or ;
; works at all, unless he says so in writing. Refer to the GNU General Public ;
; License (the "GPL") for full details.                                       ;
; Everyone is granted permission to copy, modify and redistribute KolibriOS,  ;
; but only under the conditions described in the GPL. A copy of this license  ;
; is supposed to have been given to you along with KolibriOS so you can know  ;
; your rights and responsibilities. It should be in a file named COPYING.     ;
; Among other things, the copyright notice and this notice must be preserved  ;
; on all copies.                                                              ;
;                                                                             ;
; see copying.txt                                                             ;
;                                                                             ;
; contact me on hidnplayr@gmail.com                                           ;
;                                                                             ;
;-----------------------------------------------------------------------------;

format binary as ""

; Screen size
SCREEN_X        = 640
SCREEN_Y        = 480

; Ship size
SHIP_X          = 32
SHIP_Y          = 32

; Ship begin position
SHIP_X_POS      = (SCREEN_X-SHIP_X)/2
SHIP_Y_POS      = SCREEN_Y-SHIP_Y-27

; Enemy size
ENEMY_X         = 32
ENEMY_Y         = 32

; Alien size and position
ALIEN_X         = 48
ALIEN_Y         = 38
ALIEN_Y_POS     = 1

;
BOUNDARY        = 10
MOVEMENT        = 7             ; pixels/frame

TRANSPARENCY    = 0x00ffffff    ; color used as transparant

; Window start position
WINDOW_X        = 100
WINDOW_Y        = 100

; Bullet size
BULLET_X        = 10
BULLET_Y        = 10

; Number of stars
STARS_          = 226

; Number of star levels (depth)
STARLEVELS      = 3

ENEMY_STARTING_X = 25
ENEMY_STARTING_Y = 50

BULLETSPEED     = 12            ; pixels/frame

        use32
        org     0x0
        db      'MENUET01'      ; 8 byte id
        dd      0x01            ; header version
        dd      START           ; start of code
        dd      IM_END          ; size of image
        dd      I_END+1000      ; memory for app
        dd      I_END+1000      ; esp
        dd      0x0, 0x0        ; I_Param , I_Path

include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'

KEY_RIGHT       = 179
KEY_LEFT        = 176
KEY_UP          = 178
KEY_P           = 'p'
KEY_DOWN        = 177
KEY_ENTER       = 13
KEY_ESC         = 27

proc aimgtoimg img, x, y, canvas, acolor

        pusha
; Calculate offset on canvas in edi
        mov     eax, [y]
        mov     ecx, [canvas]
        mul     dword[ecx]      ; canvas xsize
        add     eax, [x]

        lea     edi, [eax*2 + eax + 8]
        add     edi, [canvas]

; get img size in ecx and edx
        mov     esi, [img]
        mov     ecx, dword[esi+0] ; img x size
        mov     edx, dword[esi+4] ; img y size

; caluclate number of bytes between 2 lines in ebx
        mov     ebx, [canvas]
        mov     ebx, [ebx]      ; canvas xsize
        sub     ebx, [esi]      ; img xsize
        lea     ebx, [ebx*2 + ebx]

; get image start ptr in esi
        add     esi, 8

  .loop2:
        push    ecx edx
        mov     edx, [acolor]
  .loop:
        mov     eax, [esi]
        and     eax, 0x00ffffff
        cmp     eax, edx
        je      @f
        mov     word[edi], ax
        shr     eax, 16
        mov     byte[edi+2], al
  @@:
        add     esi, 3
        add     edi, 3
        dec     ecx
        jnz     .loop
        pop     edx ecx

        add     edi, ebx
        dec     edx
        jnz     .loop2

        popa
        ret

endp

proc aimgtoimg2 img, x, y, canvas, acolor

        pusha
; Calculate offset on canvas in edi
        mov     eax, [y]
        mov     ecx, [canvas]
        mul     dword[ecx]      ; canvas xsize
        add     eax, [x]

        lea     edi, [eax*2 + eax + 8]
        add     edi, [canvas]

; get img size in ecx and edx
        mov     esi, [img]
        mov     ecx, dword[esi+0] ; img x size
        mov     edx, dword[esi+4] ; img y size

; caluclate number of bytes between 2 lines in ebx
        mov     ebx, [canvas]
        mov     ebx, [ebx]      ; canvas xsize
        sub     ebx, [esi]      ; img xsize
        lea     ebx, [ebx*2 + ebx]

; get image start ptr in esi
        add     esi, 8

  .loop2:
        push    ecx edx
        mov     edx, [acolor]
  .loop:
        mov     eax, [esi]
        and     eax, 0x00ffffff
        cmp     eax, edx
        je      @f
        mov     byte[edi+2], al
        shr     eax, 8
        mov     word[edi], ax
  @@:
        add     esi, 3
        add     edi, 3
        dec     ecx
        jnz     .loop
        pop     edx ecx

        add     edi, ebx
        dec     edx
        jnz     .loop2

        popa
        ret

endp

proc getimg imgsrc, x, y, xs, ys, imgdest

        pusha

        mov     esi, [imgsrc]
        mov     eax, dword[esi+0]       ; xsize
        mov     ebx, [y]
        mul     ebx                     ; xsize*y
        add     eax, [x]                ; xsize*y+x
        lea     eax, [eax+2*eax]        ; (xsize*y+x)*3

        mov     edx, dword[esi+0]       ; xsize
        sub     edx, [xs]               ; xsize-xs
        lea     edx, [edx*2+edx]        ; (xsize-xs)*3

        lea     esi, [esi + eax + 8]    ; imgsrc + (xsize*y+x)*3 + 8

        mov     edi, [imgdest]
        mov     ecx, [xs]
        mov     dword[edi+0], ecx       ; xsize
        mov     ebx, [ys]
        mov     dword[edi+4], ebx       ; ysize
        add     edi, 8                  ; imgdest + 8

        cld
  .loop:
        movsw
        movsb
        dec     ecx
        jnz     .loop

        add     esi, edx
        mov     ecx, [xs]
        dec     ebx
        jnz     .loop

        popa
        ret

endp

macro decodeimg source, size, dest {

        invoke  img.decode, source, size, 0
        or      eax, eax
        jz      exit
        push    [eax + Image.Width]
        pop     dword[dest+0]
        push    [eax + Image.Height]
        pop     dword[dest+4]
        push    eax
        invoke  img.to_rgb2, eax, dest+8
        pop     eax
        invoke  img.destroy, eax
}


START:
        mcall   68, 11

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jz      @f
exit:
        mcall   -1
  @@:

        call    draw_window

        decodeimg gif_bullet,gif_bullet.size,img_bullet
        decodeimg gif_bullet2,gif_bullet2.size,img_bullet2
        decodeimg gif_ship,gif_ship.size,img_ship
        decodeimg gif_enemy1,gif_enemy1.size,img_enemy1
        decodeimg gif_enemy2,gif_enemy2.size,img_enemy2
        decodeimg gif_enemy3,gif_enemy3.size,img_enemy3
        decodeimg gif_enemy4,gif_enemy4.size,img_enemy4
        decodeimg gif_enemy5,gif_enemy5.size,img_enemy5
        decodeimg gif_alien,gif_alien.size,img_alien
        decodeimg gif_menu1,gif_menu1.size,img_menu1
        decodeimg gif_menu2,gif_menu2.size,img_menu2
        decodeimg gif_menu3,gif_menu3.size,img_menu3
        decodeimg gif_menu4,gif_menu4.size,img_menu4
        decodeimg gif_logo,gif_logo.size,img_logo
        decodeimg gif_pause,gif_pause.size,img_pause
        decodeimg gif_levelup,gif_levelup.size,img_levelup
        decodeimg gif_gameover,gif_gameover.size,img_gameover
        decodeimg gif_highscore,gif_highscore.size,img_highscore
        decodeimg gif_smallfont,gif_smallfont.size,img_smallfont
        decodeimg gif_bigfont,gif_bigfont.size,img_bigfont
        decodeimg gif_numbers,gif_numbers.size,img_numbers

        call    init_starfield
        call    render_frame
        call    draw_to_screen

mainloop:

        call    render_frame
        call    draw_to_screen
        call    bullet_collision_detection

        cmp     [status], 3     ; if game is paused,...
        jne     .wait

        mcall   10
        jmp     .switch

  .wait:
        mcall   5, 1            ; wait 1/100 s

        mcall   11              ; check for events
  .switch:
        test    eax, eax
        jz      mainloop
        dec     eax
        jz      .redraw
        dec     eax
        jz      .key
        dec     eax
        jz      .button

  .redraw:
        call    draw_window
        jmp     .wait

  .button:
        mcall   17              ; get button id

        cmp     ah, 1
        jne     .wait
        mcall   -1

  .key:
        mcall   2               ; get key code
        test    ah, ah
        jz      mainloop

        cmp     [status], 1
        je      key_game
        cmp     [status], 0
        je      key_menu
        cmp     [status], 3
        je      key_pause
        cmp     [status], 6
        je      key_levelup
        cmp     [status], 7
        je      key_highscore
        cmp     [status], 2
        je      key_gameover

        cmp     ah, KEY_ESC
        jne     .no_escape

        mov     [status], 0
        mov     [intro], 0
  .no_escape:

        jmp     mainloop


key_game:
        cmp     ah, KEY_RIGHT
        jnz     .no_right

        cmp     [ship_x], SCREEN_X-SHIP_X-BOUNDARY
        jge     mainloop
        add     [ship_x], MOVEMENT
        jmp     mainloop
  .no_right:

        cmp     ah, KEY_LEFT
        jne     .no_left

        cmp     [ship_x], BOUNDARY
        jle     mainloop
        sub     [ship_x], MOVEMENT
        jmp     mainloop
  .no_left:

        cmp     ah, KEY_UP
        jne     .no_up

        cmp     [bullet_y], 1
        jg      mainloop

        mov     eax, [ship_x]
        add     eax, (SHIP_X-BULLET_X)/2
        mov     [bullet_x], eax
        mov     [bullet_y], SHIP_Y_POS;-BULLET_Y
        jmp     mainloop
  .no_up:

        cmp     ah, KEY_P
        jne     no_pause

        mov     [status], 3
        stdcall aimgtoimg, img_pause, 150, 180, vscreen, TRANSPARENCY
        call    draw_to_screen

        jmp     mainloop
  no_pause:
        cmp     ah, KEY_ESC
        jne     .no_escape

        mov     [status], 0
        mov     [intro], 0
  .no_escape:
        jmp     mainloop


key_menu:
        cmp     ah, KEY_DOWN
        jne     .no_down

        cmp     [menu], 3
        jne     @f
        mov     [menu], 0
        jmp     mainloop
  @@:
        inc     [menu]
        jmp     mainloop
  .no_down:

        cmp     ah, KEY_UP
        jnz     .no_up

        cmp     [menu], 0
        jne     @f
        mov     [menu], 3
        jmp     mainloop
  @@:
        dec     [menu]
        jmp     mainloop
  .no_up:

        cmp     ah, KEY_ESC
        jne     @f
        mcall   -1
  @@:

        cmp     ah, KEY_ENTER
        jnz     .no_enter

        cmp     [menu], 0       ;start
        je      new_game

        cmp     [menu], 1       ;about
        jne     @f
        mov     [status], 4
        jmp     mainloop
  @@:

        cmp     [menu], 2       ;highscores
        jne     @f
        mov     [status], 5
        call    load_highscores
        jmp     mainloop
  @@:

        cmp     byte[menu], 3   ;exit
        jne     @f
        mcall   -1
  @@:
  .no_enter:
        jmp     mainloop


key_pause:
        cmp     ah, KEY_P
        jnz     no_pause

        mov     [status], 1
  .nopause:
        jmp     mainloop


key_levelup:

        cmp     ah, KEY_ENTER
        jne     .no_enter

        inc     [level]

        inc     byte[levelnumb+1]
        cmp     byte[levelnumb+1], '9'
        jle     @f
        mov     byte[levelnumb+1], '0'
        inc     byte[levelnumb]

  @@:
        mov     eax,20
        mov     ah,byte[level]
        and     ah,7
        mul     ah
        add     eax,level1
        mov     esi,eax
        jmp     load_level

  .no_enter:
        cmp     ah, KEY_ESC
        jne     .no_escape

        mov     [status], 0
        mov     [intro], 0
  .no_escape:
        jmp     mainloop


key_highscore:

        cmp     ah, KEY_ENTER
        jne     @f

        call    load_highscores
        mov     eax, [score]
        mov     ebx, highscorebuffer+140
  .findscore:
        cmp     ebx, highscorebuffer+100
        je      .topscore
        sub     ebx, 4
        cmp     eax, dword[ebx]
        jg      .findscore

  .topscore:
        mov     esi, name
        mov     edi, highscorebuffer
        mov     ecx, 10
        rep     movsb

        mov     eax, [score]
        mov     dword[highscorebuffer+100], eax

        call    save_highscores
        mov     [status], 5

  @@:
        cmp     ah, 14
        jne     @f

        cmp     byte[namepos],0
        je      @f

        dec     byte[namepos]
        movzx   ebx,byte[namepos]
        add     ebx,name
        mov     byte[ebx], 0x11  ; this is a character we dont print

  @@:
        cmp     byte[namepos],10
        jge     mainloop

        cmp     al,'0'
        jl      mainloop
        cmp     al,'9'
        jle     @f

        cmp     al,'z'
        jg      mainloop
        cmp     al,'a'
        jge     @f

        cmp     al,'Z'
        jg      mainloop
        cmp     al,'A'
        jl      mainloop
  @@:

        movzx   ebx, byte[namepos]
        add     ebx, name
        mov     byte[ebx], al

        inc     byte[namepos]

        jmp     mainloop


key_gameover:
        cmp     ah, KEY_ENTER
        jne     .no_enter

        ; TODO: test if score is high enough to put in highscore list...
        mov     [status],7
        jmp     mainloop

  .no_enter:
        jmp     mainloop


new_game:

        mov     [score], 0
        mov     eax, [score]
        call    convertscore

        mov     word[levelnumb], '01'
        mov     esi, level1

load_level:
        mov     [enemy_speed], 1
        mov     [enemy_x], ENEMY_STARTING_X
        mov     [enemy_y], ENEMY_STARTING_Y

        mov     edi, enemy_table
        mov     ecx, 5
        rep     movsd

        mov     [status],1

        jmp     mainloop


draw_window:

        mcall   12, 1           ; Start of window draw

        mov     ebx, WINDOW_X shl 16 + 9 + SCREEN_X                     ; [x start] shl 16 + [x size]
        mov     ecx, WINDOW_Y shl 16 + 25 + SCREEN_Y                    ; [y start] shl 16 + [y size]
        mov     edx, 0x64000000                                         ; color of work area RRGGBB
        mov     esi, 0x805080d0                                         ; color of grab bar  RRGGBB
        mov     edi, 0x005080d0                                         ; color of frames    RRGGBB
        mcall   0

        mcall   71, 1, title

        call    draw_to_screen

        mcall   12, 2           ; End of window draw

        ret



draw_to_screen:

        ; Draw buffer to the screen
        mov     ebx, vscreen+8
        mov     ecx, SCREEN_X shl 16 + SCREEN_Y
        mov     edx, 0 shl 16 + 0
        mcall   7

        ret


load_highscores:

        ret


save_highscores:

        ret


render_frame:

        mov     eax, 0x00000000
        call    fillscreen
        call    render_starfield

        cmp     [status], 1
        je      render_game
        cmp     [status], 2
        je      render_gameover
        cmp     [status], 4
        je      render_about
        cmp     [status], 6
        je      render_levelup
        cmp     [status], 0
        je      render_menu
        cmp     [status], 5
        je      render_highscorelist
        cmp     [status], 7
        je      render_highscore

        ret


render_game:

        call    render_bullet
        call    render_enemies                                                  ; Draw the enemies to buffer
        stdcall aimgtoimg, img_ship, [ship_x], SHIP_Y_POS, vscreen, TRANSPARENCY; Draw the ship to buffer

        mov     esi, scoretext
        mov     ebx, 0
        mov     ecx, SCREEN_Y-24
        call    printtext

        mov     esi, leveltext
        mov     ebx, 300
        call    printtext

        ret


render_gameover:

        stdcall aimgtoimg, img_ship, [ship_x], SHIP_Y_POS, vscreen, TRANSPARENCY; Draw the ship to buffer

        mov     esi, scoretext
        mov     ebx, 0
        mov     ecx, SCREEN_Y-24
        call    printtext

        mov     esi, leveltext
        mov     ebx, 300
        call    printtext
        stdcall aimgtoimg, img_gameover, 150, 180, vscreen, TRANSPARENCY

        ret


render_about:

        mov     esi, msgAbout
        mov     ebx, 50
        mov     ecx, 100
        call    printtext

        ret


render_levelup:

        stdcall aimgtoimg, img_ship, [ship_x], SHIP_Y_POS, vscreen, TRANSPARENCY; Draw the ship to buffer

        mov     esi, scoretext
        mov     ebx, 0
        mov     ecx, SCREEN_Y-24
        call    printtext

        mov     esi, leveltext
        mov     ebx, 300
        call    printtext
        stdcall aimgtoimg, img_levelup, 150, 180, vscreen, TRANSPARENCY

        ret


render_menu:

        stdcall aimgtoimg, img_logo, 50, 80, vscreen, TRANSPARENCY

        cmp     [menu], 0
        jne     .menu_0
        stdcall aimgtoimg2, img_menu1, 30, 200, vscreen, TRANSPARENCY
        jmp     .menu_1
  .menu_0:
        stdcall aimgtoimg, img_menu1, 30, 200, vscreen, TRANSPARENCY
  .menu_1:
        cmp     [menu], 1
        jne     .menu_2
        stdcall aimgtoimg2, img_menu2, 80, 250, vscreen, TRANSPARENCY
        jmp     .menu_3
  .menu_2:
        stdcall aimgtoimg, img_menu2, 80, 250, vscreen, TRANSPARENCY
  .menu_3:
        cmp     [menu], 2
        jne     .menu_4
        stdcall aimgtoimg2, img_menu3, 120, 300, vscreen, TRANSPARENCY
        jmp     .menu_5
  .menu_4:
        stdcall aimgtoimg, img_menu3, 120, 300, vscreen,TRANSPARENCY
  .menu_5:
        cmp     [menu], 3
        jne     .menu_6
        stdcall aimgtoimg2, img_menu4, 150, 350, vscreen, TRANSPARENCY
        jmp     .menu_7
  .menu_6:
        stdcall aimgtoimg, img_menu4, 150, 350, vscreen, TRANSPARENCY
  .menu_7:

        cmp     [intro], 200
        je      .menu_75
        inc     [intro]

  .menu_75:
        cmp     [intro], 0
        jl      .menu_8
        stdcall aimgtoimg, img_enemy1, 390, 180, vscreen, TRANSPARENCY

        cmp     [intro], 15
        jl      .menu_8
        mov     esi, points_50
        mov     ebx, 470
        mov     ecx, 180
        call    printtext

        cmp     [intro],30
        jl      .menu_8
        stdcall aimgtoimg, img_enemy2, 390, 220, vscreen, TRANSPARENCY

        cmp     [intro], 45
        jl      .menu_8
        mov     esi, points_100
        mov     ebx, 450
        mov     ecx, 220
        call    printtext

        cmp     [intro], 60
        jl      .menu_8
        stdcall aimgtoimg, img_enemy3, 390, 260, vscreen, TRANSPARENCY

        cmp     [intro], 75
        jl      .menu_8
        mov     esi, points_150
        mov     ebx, 450
        mov     ecx, 260
        call    printtext

        cmp     [intro],90
        jl      .menu_8
        stdcall aimgtoimg, img_enemy4, 390, 300, vscreen, TRANSPARENCY

        cmp     [intro], 105
        jl      .menu_8
        mov     esi, points_200
        mov     ebx, 450
        mov     ecx, 300
        call    printtext

        cmp     [intro], 120
        jl      .menu_8
        stdcall aimgtoimg, img_enemy5, 390, 340, vscreen, TRANSPARENCY

        cmp     [intro],135
        jl      .menu_8
        mov     esi, points_250
        mov     ebx, 450
        mov     ecx, 340
        call    printtext

        cmp     [intro],150
        jl      .menu_8
        stdcall aimgtoimg, img_alien, 380, 380, vscreen, TRANSPARENCY

        cmp     [intro],165
        jl      .menu_8
        mov     esi, points_1000
        mov     ebx, 430
        mov     ecx, 380
        call    printtext

    .menu_8:
        ret


render_highscorelist:

        stdcall aimgtoimg, img_highscore, 60, 40, vscreen, TRANSPARENCY

        mov     ebx, 100                        ; print names
        mov     ecx, 120
        mov     esi, highscorebuffer
        call    printtext

        mov     edi, highscorebuffer+100        ; print scores
        mov     esi, scorenumb
        mov     ebx, 420
        mov     ecx, 120

  .loop:
        mov     eax,[edi]
        push    ecx
        call    convertscore
        pop     ecx
        push    esi
        call    printtext
        pop     esi
        add     ecx, 26
        add     edi, 4
        cmp     edi, highscorebuffer+140
        jl      .loop

        ret


render_highscore:

        stdcall aimgtoimg, img_highscore, 60, 40, vscreen, TRANSPARENCY

        mov     ebx, 60
        mov     ecx, 200
        mov     esi, entername
        call    printtext

        mov     ebx, 250
        mov     ecx, 250
        mov     esi, name
        call    printtext

        mov     esi, scoretext
        mov     ebx, 0
        mov     ecx, SCREEN_Y-24
        call    printtext

        mov     esi, leveltext
        mov     ebx, 300
        call    printtext

        ret


render_enemies:
; check if direction should change
        test    [enemy_d], 2
        jz      @f

        add     [enemy_y], 5

        mov     eax, [enemy_y]
        shr     eax, 5
        add     al, [level]
        mov     [enemy_speed], al

        and     [enemy_d], 1

       @@:
; move the aliens to left or right
        movzx   eax, [enemy_speed]
        test    [enemy_d], 1
        jz      .other_dir

        sub     [enemy_x], eax
        jmp     .no_other_dir

  .other_dir:
        add     [enemy_x], eax
  .no_other_dir:

; initialization
        mov     [alldeadb],1
        mov     edi, enemy_table
        mov     eax, [enemy_x]
        mov     [current_enemy_x], eax
        mov     eax, [enemy_y]
        mov     [current_enemy_y], eax

  .loopit:
        movzx   eax, byte[edi]
        test    al, al
        jz      .next_alien
        cmp     al, 5
        ja      .next_alien
        dec     eax
        mov     eax, [enemy_img_list+eax*4]

  .drawenemy:
        mov     [alldeadb], 0
        stdcall aimgtoimg, eax, [current_enemy_x], [current_enemy_y], vscreen, TRANSPARENCY
;        jmp  checknext

  .checknext:
        cmp     [enemy_d], 2
        jge     .dont_change_dir

        movzx   eax, [enemy_speed]

        cmp     [enemy_d], 0
        jbe     .change_dir

        cmp     dword[current_enemy_x],eax
        jg      .dont_change_dir

        mov     [enemy_d], 2
        jmp     .dont_change_dir

  .change_dir:
        mov     ebx, SCREEN_X-ENEMY_X
        sub     ebx, eax
        cmp     dword[current_enemy_x],ebx
        jl      .dont_change_dir

        mov     [enemy_d], 3

  .dont_change_dir:
        cmp     [current_enemy_y], SHIP_Y_POS-ENEMY_Y-BOUNDARY
        jle     .next_alien                                     ;;;;;;

        mov     [status], 2
        ret

  .next_alien:
        cmp     edi, enemy_table+20
        jge     .alldead

        inc     edi
        add     dword[current_enemy_x],ENEMY_X+BOUNDARY
        mov     eax,dword[current_enemy_x]
        sub     eax,dword[enemy_x]
        cmp     eax,5*(ENEMY_X+BOUNDARY)
        jl      .no_newline

        sub     [current_enemy_x], 5*(ENEMY_X+BOUNDARY)
        add     [current_enemy_y], ENEMY_Y+BOUNDARY
  .no_newline:
        jmp     .loopit

  .alldead:
        cmp     [alldeadb], 0
        je      .enemy_end

        mov     [status], 6
        ret

  .enemy_end:
        cmp     [alien_x], 5
        jge     @f

        call    random_generator
        cmp     eax,0xffffffff/50 ; one out of 500 chances that it appears during this frame
        jl      .alien_end
        mov     [alien_x], SCREEN_X-ALIEN_X
  @@:
        push    eax

        mov     eax, SCREEN_X                                                                                                                                                                                                                                 ;        mov  eax, SCREEN_X
        sub     eax, dword [alien_x]

        cmp     eax, ALIEN_X
        jle     @f
        mov     eax, ALIEN_X
  @@:

;        stdcall getimg, img_alien, 0, 0, 10, ALIEN_Y, img_alienpiece
        stdcall aimgtoimg, img_alien, [alien_x], ALIEN_Y_POS, vscreen, TRANSPARENCY
        sub     [alien_x], 5

        pop     eax

  .alien_end:
        ret



render_bullet:
        cmp     [bullet_y], BULLETSPEED
        jl      .nobullet
        sub     [bullet_y], BULLETSPEED

        stdcall aimgtoimg, img_bullet, [bullet_x], [bullet_y], vscreen, TRANSPARENCY

  .nobullet:
        ret



bullet_collision_detection:

        cmp     [bullet_y], BULLETSPEED         ; does the bullet hit top of the screen?
        jle     .hidebullet                     ; yes, hide bullet

        mov     edi, enemy_table
        mov     eax, [enemy_x]
        mov     [current_enemy_x], eax
        mov     eax, [enemy_y]
        mov     [current_enemy_y], eax

  .check:
        cmp     byte[edi],0                     ; is the enemy at this position alive?
        je      .nextcheck                      ; no, try next enemy
        ; check if bullet hits current enemy

        mov     eax, [current_enemy_y]          ; move the enemy y position into eax
        cmp     [bullet_y], eax                 ; is the bullet's y position less than eax (enemy y pos)
        jl      .nextcheck                      ; yes, bullet can't be colliding, check next enemy

        add     eax, ENEMY_Y                    ; add the width of the enemy to the enemy's y position (wich is still stored in eax)
        cmp     [bullet_y], eax                 ; is the bullet's y position greater than eax (the end of the enemy)
        jg      .nextcheck                      ; yes, bullet can't be colliding, check next enemy

        mov     eax, [current_enemy_x]          ; now do the same but for the x positions
        cmp     [bullet_x], eax                 ;
        jl      .nextcheck                      ;
                                                ;
        add     eax, ENEMY_Y                    ;
        cmp     [bullet_x], eax                 ;
        jg      .nextcheck                      ;

        jmp     .hit

  .nextcheck:
        inc     edi
        add     [current_enemy_x], ENEMY_X+BOUNDARY
        mov     eax, [current_enemy_x]
        sub     eax, [enemy_x]
        cmp     eax, 5*(ENEMY_X+BOUNDARY)
        jl      .no_newline

        sub     [current_enemy_x], 5*(ENEMY_X+BOUNDARY)
        add     [current_enemy_y], ENEMY_Y+BOUNDARY
  .no_newline:

        cmp     edi, enemy_table+20             ; is this the last enemy?
        jg      .nohit                          ; yes, none of them was hit
        jmp     .check                          ; no, check if enemy is alive and draw it

  .hit:
        movzx   ebx, byte[edi]                  ; mov the enemy number onto ebx
        add     [score], ebx                    ; add this number to the score dword

        mov     eax,[score]
        call    convertscore

        mov     byte[edi],0                     ; hide the enemy
  .hidebullet:
        mov     [bullet_y], 1                   ; mov the bullet to top of screen (hide it)
        jmp     .noalienhit

  .nohit:
        mov     eax, [alien_x]                  ; check if we hit the big alien in the ufo
        cmp     [bullet_x], eax
        jl      .noalienhit
        add     eax, ALIEN_X-BULLET_X
        cmp     [bullet_x], eax
        jg      .noalienhit
        cmp     [bullet_y], ALIEN_Y_POS+ALIEN_Y
        jg      .noalienhit

        add     [score], 100/5
        mov     eax, [score]
        call    convertscore

        mov     [alien_x], 0

  .noalienhit:
        ret



convertscore:

        test    al,1
        jz      .1
        mov     byte[scorenumb+5],'5'
        jmp     .2
  .1:
        mov     byte[scorenumb+5],'0'
  .2:
        shr     eax,1
        mov     ecx,10
        xor     edx,edx
        div     ecx
        add     dl,'0'
        mov     byte[scorenumb+4],dl
        xor     edx,edx
        div     ecx
        add     dl,'0'
        mov     byte[scorenumb+3],dl
        xor     edx,edx
        div     ecx
        add     dl,'0'
        mov     byte[scorenumb+2],dl
        xor     edx,edx
        div     ecx
        add     dl,'0'
        mov     byte[scorenumb+1],dl
        xor     edx,edx
        div     ecx
        add     dl,'0'
        mov     byte[scorenumb+0],dl

        ret


fillscreen: ; eax - screen color ( 0x00RRGGBB )

        mov     edi, vscreen+8
        cld
        mov     ecx, SCREEN_X*SCREEN_Y
    .lab1:
        mov     [edi], eax
        add     edi, 3
        loop    .lab1

        ret


printtext:

        push    ebx

  .loop:
        lodsb
        test    al, al
        jz      .done
        cmp     al, 13
        je      .nextline
        cmp     al,' '
        je      .space

        cmp     al,'0'
        jl      .loop
        cmp     al,'9'
        jle     .usenumbers

        cmp     al,'z'
        jg      .loop
        cmp     al,'a'
        jge     .usesmallfont

        cmp     al,'Z'
        jg      .loop
        cmp     al,'A'
        jge     .usebigfont
        jmp     .loop

  .usesmallfont:
        movzx   edx, al
        sub     edx, 'a'
        mov     eax, 12
        mul     edx

        stdcall getimg, img_smallfont, 0, eax, 20, 12, img_char
        push    ecx
        add     ecx, 4
        stdcall aimgtoimg, img_char, ebx, ecx, vscreen, TRANSPARENCY
        pop     ecx
        add     ebx, 20
        jmp     .loop

  .usebigfont:
        movzx   edx, al
        sub     edx, 'A'
        mov     eax, 20
        mul     edx

        stdcall getimg, img_bigfont, 0, eax, 28, 20, img_char
        stdcall aimgtoimg, img_char, ebx, ecx, vscreen, TRANSPARENCY
        add     ebx, 28
        jmp     .loop

  .usenumbers:
        movzx   edx, al
        sub     edx, '0'
        mov     eax, 20
        mul     edx

        stdcall getimg, img_numbers, 0, eax, 16, 20, img_char
        stdcall aimgtoimg, img_char, ebx, ecx, vscreen, TRANSPARENCY
        add     ebx, 20
        jmp     .loop

  .space:
        add     ebx, 20
        jmp     .loop

  .nextline:
        pop     ebx
        push    ebx
        add     ecx, 26
        jmp     .loop

  .done:
        pop     ebx
        ret


init_starfield:

        mov     ebx, STARS
  .loop:
        cmp     ebx, STARS+(STARS_*5)
        jge     .done

        call    random_generator
        and     al, STARLEVELS
        test    al,al
        jnz     @f
        inc     al
  @@:
        mov     byte[ebx],al

        call    random_generator
        and     eax, SCREEN_X-1
        inc     eax
        mov     word[ebx+1],ax

        call    random_generator
        and     eax, SCREEN_Y-1
        inc     eax
        mov     word[ebx+3],ax

        add     ebx, 5
        jmp     .loop
  .done:
        ret


render_starfield:

        mov     esi, STARS
  .loop:
        cmp     esi, STARS+(STARS_*5)
        jge     .done

        movzx   eax, byte[esi]    ; z (speed, brightness)
        movzx   ebx, word[esi+1]  ; x
        movzx   ecx, word[esi+3]  ; y
        add     bx, ax
        cmp     bx, SCREEN_X
        jl      .moveit

        xor     ebx, ebx
        inc     ebx

        call    random_generator
        mov     ecx, [generator]
        and     ecx, SCREEN_Y-1
        inc     ecx
        mov     word[esi+3], cx

        call    random_generator
        and     al, STARLEVELS
        test    al, al
        jnz     @f
        inc     al
   @@:
        mov     [esi], al

  .moveit:
        mov     word[esi+1], bx

        movzx   eax, byte[esi]
        inc     eax
        mov     edx, 0xff/(STARLEVELS+1)
        mul     edx

        mov     ah, al
        shl     eax, 8
        mov     al, ah
        mov     ebp, eax

        mov     eax, SCREEN_X
        mul     ecx
        add     eax, ebx
        mov     edx, 3
        mul     edx

        cmp     eax, SCREEN_X*SCREEN_Y*3
        jg      @f
        add     eax, vscreen+8
        and     dword[eax], 0xff000000
        or      dword[eax], ebp
  @@:

        add     esi, 5
        jmp     .loop

  .done:
        ret

random_generator:  ; (pseudo random, actually :)

        xor     eax, [generator]
        imul    eax, 214013
        xor     eax, 0xdeadbeef
        rol     eax, 9
        mov     [generator], eax
        ror     eax, 16
        and     eax, 0x7fff

        ret




level1:
db 4,4,4,4,4
db 3,3,3,3,3
db 2,2,2,2,2
db 1,1,1,1,1

level2:
db 4,1,3,1,4
db 4,3,2,3,4
db 0,4,1,4,0
db 0,0,2,0,0

level3:
db 1,5,5,5,1
db 1,2,2,2,1
db 3,1,2,1,3
db 4,3,1,3,4

level4:
db 4,5,2,5,4
db 5,3,3,3,5
db 4,5,4,5,4
db 1,5,5,5,1

level5:
db 5,4,3,4,5
db 5,4,4,4,5
db 4,5,4,5,4
db 3,5,1,5,3

level6:
db 1,2,5,4,5
db 5,4,5,2,1
db 1,2,5,4,5
db 1,1,5,1,1

level7:
db 1,2,3,2,1
db 1,3,3,3,1
db 3,4,3,4,3
db 5,5,5,5,5

level8:
db 1,2,3,4,5
db 3,5,3,5,4
db 4,2,3,2,3
db 5,4,3,2,1

enemy_table:
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0
db 0,0,0,0,0

msgAbout        db 'Hidnplayrs invaders',13,'KolibriOS version',13,13,'released under GPL',13,'make this game better',13,'if you want to',0
title           db 'Invaders',0
msgdone         db 'You have saved the planet!',0
entername       db 'Enter your name highscorer!',0
highscorefile   db 'invaders.dat',0
points_50       db '5 pt',0
points_100      db '10 pt',0
points_150      db '15 pt',0
points_200      db '20 pt',0
points_250      db '25 pt',0
points_1000     db '100 pt',0
ship_x          dd SHIP_X_POS
enemy_x         dd 0
enemy_y         dd 0
enemy_d         db 0
current_enemy_x dd 0
current_enemy_y dd 0
bullet_x        dd 0
bullet_y        dd 1
score           dd 0
alldeadb        db 0
status          db 0        ; status: 0=menu  1=game  2=gameover   3=paused  4=about 5=highscorelist 6=levelup 7=highscore...
menu            db 0        ; menu:   0=start 1=about 2=highscores 3=exit...
generator       dd 0x45dd4d15
alien_x         dd 0
;drawroutine     dd 0
returnaddr      dd 0
intro           dw 0
scoretext       db 'score '
scorenumb       db 0,0,0,0,0,0,0
leveltext       db 'level '
levelnumb       db 0,0,0
lives           db 0
level           db 1
enemy_speed     db 1
namepos         db 0
name            db 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x0d,0x00

enemy_img_list:
        dd      img_enemy1
        dd      img_enemy2
        dd      img_enemy3
        dd      img_enemy4
        dd      img_enemy5


gif_bullet      file 'bullet2.gif'
  .size = $ - gif_bullet
gif_bullet2     file 'bullet2.gif'
  .size = $ - gif_bullet2
gif_ship        file 'ship.gif'
  .size = $ - gif_ship
gif_enemy1      file 'enemy1.gif'
  .size = $ - gif_enemy1
gif_enemy2      file 'enemy2.gif'
  .size = $ - gif_enemy2
gif_enemy3      file 'enemy3.gif'
  .size = $ - gif_enemy3
gif_enemy4      file 'enemy4.gif'
  .size = $ - gif_enemy4
gif_enemy5      file 'enemy5.gif'
  .size = $ - gif_enemy5
gif_alien       file 'alien.gif'
  .size = $ - gif_alien
gif_menu1       file 'menu1.gif'
  .size = $ - gif_menu1
gif_menu2       file 'menu2.gif'
  .size = $ - gif_menu2
gif_menu3       file 'menu3.gif'
  .size = $ - gif_menu3
gif_menu4       file 'menu4.gif'
  .size = $ - gif_menu3
gif_logo        file 'logo.gif'
  .size = $ - gif_logo
gif_pause       file 'pause.gif'
  .size = $ - gif_pause
gif_highscore   file 'highscores.gif'
  .size = $ - gif_highscore
gif_smallfont   file 'font_small.gif'
  .size = $ - gif_smallfont
gif_bigfont     file 'font_capital.gif'
  .size = $ - gif_bigfont
gif_numbers     file 'numbers.gif'
  .size = $ - gif_numbers
gif_levelup     file 'nextlevel.gif'
  .size = $ - gif_levelup
gif_gameover    file 'gameover.gif'
  .size = $ - gif_gameover

align 16
@IMPORT:

library                         \
        libimg , 'libimg.obj'

import  libimg                     , \
        libimg.init , 'lib_init'   , \
        img.decode  , 'img_decode' , \
        img.to_rgb2 , 'img_to_rgb2', \
        img.destroy , 'img_destroy'


vscreen:
                dd SCREEN_X
                dd SCREEN_Y
                rb SCREEN_X*SCREEN_Y*3

IM_END:

STARS           rb STARS_*5

img_bullet      rb BULLET_X*BULLET_Y*3+8
img_bullet2     rb BULLET_X*BULLET_Y*3+8
img_ship        rb SHIP_X*SHIP_Y*3+8
img_enemy1      rb ENEMY_X*ENEMY_Y*3+8
img_enemy2      rb ENEMY_X*ENEMY_Y*3+8
img_enemy3      rb ENEMY_X*ENEMY_Y*3+8
img_enemy4      rb ENEMY_X*ENEMY_Y*3+8
img_enemy5      rb ENEMY_X*ENEMY_Y*3+8
img_alien       rb ALIEN_X*ALIEN_Y*3+8
img_menu1       rb 220*18*3+8
img_menu2       rb 135*18*3+8
img_menu3       rb 245*18*3+8
img_menu4       rb 110*18*3+8
img_logo        rb 40*540*3+8
img_pause       rb 40*320*3+8
img_levelup     rb 40*320*3+8
img_gameover    rb 40*320*3+8
img_highscore   rb 40*530*3+8
img_smallfont   rb 20*312*3+8
img_bigfont     rb 28*520*3+8
img_numbers     rb 16*200*3+8

img_char        rb 28*20*3+8
img_alienpiece  rb ALIEN_X*ALIEN_Y*3+8

highscorebuffer rb 1024

I_END: