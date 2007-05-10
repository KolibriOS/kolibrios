VERSION equ 'ARCANOID II v. 0.30'
;               by jj
;        (jacek jerzy malinowski)
;
;   contact: 4nic8@casiocalc.org
;----------------------------------------
;   Compile with FASM for Menuet
;----------------------------------------

include 'lang.inc'
include '..\..\..\macros.inc'
include 'ascl.inc'
include 'ascgl.inc'
include 'asjc.inc'

X_SIZE equ 400
Y_SIZE equ 300

MAX_LEVEL equ 5

BACK_CL equ 0x00EFEF ; background color

;    THE MAIN PROGRAM:
use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x200000                ; memory for app
               dd     0x7fff0                 ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

START:                          ; start of execution

    bmptoimg bmp_file,img_bmp   ; loading ... ;]
    getimg img_bmp,0,0,10,10,img_ball
    getimg img_bmp,20,0,20,10,img_bonus
    getimg img_bmp,0,10,40,20,img_brick1
    getimg img_bmp,0,30,40,20,img_brick2
    getimg img_bmp,0,50,40,20,img_brick3
    getimg img_bmp,0,70,40,20,img_brick4

    call draw_window

still:
    if_e dword [level],0,.no_intro
       call intro
       jmp .no_game
    .no_intro:

    if_e dword [mode],2,.end_if1
       call level_info
       jmp .no_game
    .end_if1:

    if_e dword [mode],4,.end_if2
       call game_over
       jmp .no_game
    .end_if2:

    call fast_gfx ; <-- the main engine
    .no_game:

    mov  eax,11
    mcall

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
    cmp  ah,key_Esc ; if Esc ?
    jne  .no_q
      or eax,-1
      mcall
    .no_q:

    if_e dword [mode],4,.end_if6
       jmp still
    .end_if6:

    cmp  ah,key_Space
    jne  .no_space
      if_e dword [mode],2,.end_if1
         mov dword [mode],0
         jmp .no_space
      .end_if1:
      mov dword [mode],1
      call fast_gfx
    .no_space:
    xor ebx,ebx
    mov bl,ah
    if_e ebx,key_F1,.no_f1
      inc dword [del]
    .no_f1:
    if_e ebx,key_F2,.no_f2
      if_a dword [del],0,.end_if3
         dec dword [del]
      .end_if3:
    .no_f2:


    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

    mov  eax,-1                 ; close this program
    mcall
  noclose:

    jmp  still

;   *********************************************
;   ******* VIRTUAL SCREEN FUNCTIONS ************
;   *********************************************

show_screen:  ; flips the virtual screen to the window
    push_abc

    mov  eax,7
    mov  ebx,screen
    mov  ecx,X_SIZE*65536+Y_SIZE
    mov  edx,4*65536+20
    mcall

    pop_abc
ret

put_bmp_screen: ; eax - y , ebx - x, esi - bmp
    cmp ebx,X_SIZE-5
    jb .ok1
      ret
    .ok1:
    cmp eax,Y_SIZE-5
    jb .ok2
      ret
    .ok2:

    push_abc
    xor  ecx,ecx
    xor  edx,edx
    mov  edi,screen
    mov  ecx,3
    mul  ecx  ; xx = 3*y*X_SIZE+3*x
    mov  ecx,X_SIZE
    mul  ecx
    push eax ; #> 1
    mov  eax,ebx
    mov  ecx,3
    mul  ecx
    mov  ebx,eax
    pop  edx ; #< 1
    add  edx,ebx
    add  edi,edx ; sets the pointer to x,y of the screen

    mov  cx,[si] ; loops 1
    xor  ebx,ebx
    mov  ax,cx
    mov  dx,3
    mul  dx
    mov  bx,ax

    push ebx ;#>4

    add  si,4
    mov  ax,[si] ; loops 2
    mov  cx,[si]
    ;shr  ax,2
    mov  dx,3   ; dx = ax *3
    mul  dx
    mov  bx,ax
    add  si,4


    pop  ebx ;#<4
    .l_y:
       mov ax,cx
       cld
       mov  cx,bx
       rep movs byte [edi],[esi]
       add edi,X_SIZE
       add edi,X_SIZE
       add edi,X_SIZE
       sub edi,ebx
       mov cx,ax
    loop .l_y

    pop_abc
ret

rect_screen: ; eax - y , ebx - x, ecx - size x, edx - size y, si -color
    mov edi,ebx
    add ebx,ecx
    cmp ebx,X_SIZE
    jb .ok1
      ret
    .ok1:
    mov ebx,edi

    mov edi,eax
    add eax,edx
    cmp eax,Y_SIZE
    jb .ok2
      ret
    .ok2:
    mov eax,edi
    push_abc
    push ecx  ;#>2
    push edx  ;#>3

    xor  ecx,ecx
    xor  edx,edx
    mov  edi,screen
    mov  ecx,3
    mul  ecx  ; xx = 3*y*X_SIZE+3*x
    mov  ecx,X_SIZE
    mul  ecx
    push eax ; #> 1
    mov  eax,ebx
    mov  ecx,3
    mul  ecx
    mov  ebx,eax
    pop  edx ; #< 1
    add  edx,ebx

    add  edi,edx ; sets the pointer to x,y of the screen

    pop ecx ; #<3
    pop edx ; #<4
    mov  eax,esi
    .l_y:
       ;mov ax,cx
       push  ecx
       cld
       mov  ecx,edx
       .l_x:
         ;rep movs byte [edi],[esi]
         mov  word [edi],ax
         push eax
         shr  eax,16
         mov  byte [edi+2],al
         add  edi,3
         pop  eax
       loop .l_x

       add edi,X_SIZE
       add edi,X_SIZE
       add edi,X_SIZE
       sub edi,edx
       sub edi,edx
       sub edi,edx
       ;mov cx,ax
       pop  ecx
    loop .l_y

    pop_abc
ret

grad_rect_screen: ; eax - y , ebx - x, ecx - size x, edx - size y, si -color, d
    push edi  ;#>0
    mov edi,ebx
    add ebx,ecx
    cmp ebx,X_SIZE
    jb .ok1
      pop edi ;#<0
      ret
    .ok1:
    mov ebx,edi

    mov edi,eax
    add eax,edx
    cmp eax,Y_SIZE
    jb .ok2
      pop edi ;#<0
      ret
    .ok2:
    mov eax,edi

    pop edi ;#<0
    push_abc

    push edi  ;#>5
    push ecx  ;#>2
    push edx  ;#>3

    xor  ecx,ecx
    xor  edx,edx
    mov  edi,screen
    mov  ecx,3
    mul  ecx  ; xx = 3*y*X_SIZE+3*x
    mov  ecx,X_SIZE
    mul  ecx
    push eax ; #> 1
    mov  eax,ebx
    mov  ecx,3
    mul  ecx
    mov  ebx,eax
    pop  edx ; #< 1
    add  edx,ebx

    add  edi,edx ; sets the pointer to x,y of the screen

    pop ecx ; #<3
    pop edx ; #<2
    mov  eax,esi
    pop esi ; #<5
    .l_y:
       ;mov ax,cx
       push  ecx
       cld
       mov  ecx,edx
       .l_x:
         ;rep movs byte [edi],[esi]
         mov  word [edi],ax
         push eax
         shr  eax,16
         mov  byte [edi+2],al
         add  edi,3
         pop  eax
       loop .l_x

       add edi,X_SIZE
       add edi,X_SIZE
       add edi,X_SIZE
       sub edi,edx
       sub edi,edx
       sub edi,edx
       add eax,esi
       ;mov cx,ax
       pop  ecx
    loop .l_y

    pop_abc
ret


fill_screen: ; eax - screen color ( 0x00RRGGBB )
    push_abc
    mov  edi,screen
    cld
    mov  ecx,X_SIZE*Y_SIZE
    .lab1:
        mov  [edi],eax
        add  edi,3
    loop .lab1
    pop_abc
ret

grad_fill_screen: ; eax - screen color ( 0x00RRGGBB ), ebx - mack
    push_abc
    mov  edi,screen
    cld
    mov  ecx,Y_SIZE
    mov  dl,0
    .lab1:
       push ecx
       mov ecx,X_SIZE
       .lab2:
         mov  [edi],eax
         add  edi,3
       loop .lab2
       mov dh,1  ; dl = 1 - dl
       sub dh,dl
       mov dl,dh
       cmp dl,0
       jne .no_ch  ; if (dl==0)
         add  eax,ebx ; change gradient
       .no_ch:
       pop ecx
    loop .lab1
    pop_abc
ret


bmp_fill_screen: ; esi - pointer to a backgroung bmp
    push_abc
    mov  edi,screen
    cld
    mov  ecx,X_SIZE*Y_SIZE
      rep movs dword  [edi],[esi]
    pop_abc
ret

;___________________
intro:  ; INTRO    ;
    label 140,200,VERSION,0x100000FF
    label 120,220,'by jj (jacek jerzy malinowski)',0x050505
    label 100,240,'press SPACE to start a new game',0x10FF0800
    label 15,240,'F1 + delay',0xFFA8FF
    label 15,260,'F2 + delay',0xFFA8FF
    delay 10
ret

;___________________
level_info:
    label 170,230,'L E V E L',0x100000FF
    outcount [level],195,250,0x100000FF,2*65536
    label 100,270,'press SPACE to start the level',0x10FF0800
    delay 10
ret

;_________________________
game_over:  ; GAME OVER  ;
    mov  eax,0x00FF00
    mov  ebx,0xFF01
    .g_ok:
    call grad_fill_screen
    call show_screen  ; flips the screen
    label 120,150,'G  A  M  E    O  V  E  R',0x10050505
    label 140,200,'Thanks for playing',0x0FFF800
    delay 20
ret


;-----------------------------;
; THE MAIN THE GAME'S ENGINE ;
;-----------------------------;
fast_gfx:
    ; the background gradient
    if_e  dword [level],0,.no_0
      mov  eax,0xFF
      mov  ebx,0xFEFF
      jmp .g_ok
    .no_0:
    if_e  dword [level],1,.no_1
      mov  eax,BACK_CL
      mov  ebx,0xFFFF
      jmp .g_ok
    .no_1:
    if_e  dword [level],2,.no_2
      mov  eax,0xFF0000
      mov  ebx,0xFF00FF
      jmp .g_ok
    .no_2:

    mov  eax,BACK_CL
    mov  ebx,0xFFFF
    .g_ok:
    call grad_fill_screen

    mov  eax,37  ; get mouse position
    mov  ebx,1
    mcall
    shr  eax,16
    mov  [x],eax
    add  eax,[s_x]
    cmp  eax,X_SIZE  ; controls if the pad is in the screen
    jb   .ok
      cmp eax,0x7FFF ; if < 0
      jb  .upper
        mov [x],0
        jmp .ok
      .upper:        ; if > X_SIZE - pad size
      mov dword [x],X_SIZE-1
      mov eax,[s_x]
      sub dword [x],eax
    .ok:
    mov  ebx,[x]
    mov  eax,[y]
    mov  ecx,[s_x]
    mov  edx,15
    mov  esi,0xFF0000
    mov  edi,0xF0000F
    call grad_rect_screen

    call draw_level

    cmp dword [mode],1
    jne .no_go ; is the game started ?
      mov eax,[v_x]
      add dword [b_x],eax
      mov eax,[v_y]
      add dword [b_y],eax
      jmp .go
    .no_go:
      mov eax,[x] ; b_x = x + x_s/2
      mov ebx,[s_x]
      shr ebx,1
      add eax,ebx
      mov dword [b_x],eax
      mov eax,[y] ; b_y = y - 10
      sub eax,10
      mov dword [b_y],eax

      mov dword [v_x],1
      mov dword [v_y],-1
    .go:
    ;TEST WHERE IS THE BALL:
    cmp dword [b_x],0x7FFFFFFF
    jb .b_ok2 ; if out of the screen (left)
      mov dword [b_x],0
      mov eax,0
      sub eax,[v_x]
      mov [v_x],eax
    .b_ok2:
    cmp dword [b_x],X_SIZE-10
    jb .b_ok1 ; if out of the screen (right)
      mov dword [b_x],X_SIZE-11
      mov eax,0
      sub eax,[v_x]
      mov [v_x],eax
    .b_ok1:
    cmp dword [b_y],0x7FFFFFFF
    jb .b_ok3 ; if out of the screen (up)
      mov dword [b_y],0
      mov eax,0
      sub eax,[v_y]
      mov [v_y],eax
    .b_ok3:
    cmp dword [b_y],Y_SIZE-10
    jb .b_ok4 ; if out of the screen (down)
      mov dword [mode],0
      if_e dword [lives],0,.end_if5
         mov dword [mode],4 ; GAME OVER
         jmp still
      .end_if5:
         dec dword [lives]
      .end_else4:
      call draw_window
    .b_ok4:

    imgtoimg img_ball,dword [b_x],dword [b_y],screen_img

    call show_screen  ; flips the screen
    delay dword [del]

    call do_tests ; does all needed tests
ret
;----------------------;
; BALL & BRICKS EVENTS ;
;----------------------;
MAX_SPEED equ 3
do_tests:
    ; BALL <-> PAD
    mov eax,[b_x]
    add eax,10
    cmp eax,[x] ; if (b_x+10)>[pad x]
    jb .skip        ; &&
    mov eax,[b_x]
    mov ebx,[s_x]
    add ebx,[x]
    cmp eax,ebx ; if b_x < x + s_x
    ja .skip     ; &&
    mov eax,[b_y]
    add eax,10
    cmp eax,[y] ; if (b_y+10) > y
    jb .skip
    sub eax,15
    cmp eax,[y] ; if b_y < y+15
    ja .skip
      cmp dword [v_y],0x7FFFFF ; if v_y > 0
      ja .skip
      cmp dword [v_y],MAX_SPEED; speedup:
      ja .skip_s
        inc dword [speed_t]
        cmp dword [speed_t],5
        jb .skip_s
        inc dword [v_y]
        mov dword [speed_t],0
      .skip_s:
      inc dword [speed_t]
      mov eax,0
      sub eax,[v_y]
      mov [v_y],eax
      ;counting v_x:--------
      mov eax,[b_x]
      sub eax,[x]
      sub eax,5
      mov ecx,eax
      if_a eax,100,.end_if3
        mov eax,0
        sub eax,[v_x]
        mov [v_x],eax
        jmp .skip
      .end_if3:
      if_a eax,20,.end_if2
        sub eax,20
        shr eax,2
        mov [v_x],eax
        jmp .skip
      .end_if2:
        mov ebx,20
        sub ebx,ecx
        shr ebx,2
        mov dword [v_x],0
        sub dword [v_x],ebx
    .skip:

    ; BALL <-> BRICK
    mov dword [coliz],0
    call colision
    if_e dword [coliz],1,.end_if6
       ;abs dword [v_y]
       ;abs dword [v_x]
       ret
    .end_if6:
    add dword [b_x],10
    call colision
    sub dword [b_x],10
    if_e dword [coliz],1,.end_if7
       ;abs dword [v_y]
       ;abs dword [v_x]
       ch_sign dword [v_x]
       ret
    .end_if7:
    add dword [b_y],10
    call colision
    sub dword [b_y],10
    if_e dword [coliz],1,.end_if8
       ;abs dword [v_y]
       ;abs dword [v_x]
       ;ch_sign dword [v_y]
       ret
    .end_if8:
    add dword [b_x],10
    add dword [b_y],10
    call colision
    sub dword [b_x],10
    sub dword [b_y],10
    if_e dword [coliz],1,.end_if9
       ;abs dword [v_y]
       ;abs dword [v_x]
       ;ch_sign dword [v_x]
       ;ch_sign dword [v_y]

       ret
    .end_if9:


ret

colision:

    mov  esi,levels
    mov  eax,[level] ; eax = levels*100
    mov  bx,100
    mul  bx
    add  esi,eax
    ;--------------
    xor  edx,edx
    mov  eax,[b_x]
    mov  ebx,40
    div  ebx
    mov  ecx,eax
    push edx ;#>1

    xor edx,edx
    mov  eax,[b_y]
    mov  ebx,20
    div  ebx
    push edx ;#>2
    cmp  eax,9 ; out of the bricks board
    ja .ok2
    mov  ebx,10
    mul  ebx
    add  eax,ecx
    add  esi,eax

    cmp byte [esi],0 ; 0 - no brick
    je .ok2
      if_ne byte [esi],4,.end_if1
        dec byte [esi]
      .end_if1:
      mov dword [coliz],1
      pop ebx ;#<2
      pop eax ;#<1
      cmp ecx,8 ; x < 5 || x >35 - x inv
      jb  .inv
      cmp ecx,33
      ja  .inv
      jmp .no_inv
      .inv:
        mov eax,0
        sub eax,[v_x]
        mov [v_x],eax
        ;jmp .no_ok
      .no_inv:
      cmp ebx,6 ; if y < 5 || y>15 - y inv
      jb .inv_y
      cmp ebx,14
      ja .inv_y
      jmp .no_ok
        .inv_y:
        mov eax,0
        sub eax,[v_y]
        mov [v_y],eax
      .no_ok:
      jmp .ok
    .ok2:
      pop eax ;#<1
      pop eax ;#<2
    .ok:


ret

;-----------------------------------;
; DRAWS CURRENT LEVEL ON THE SCREEN ;
;-----------------------------------;
draw_level:
    mov  esi,levels
    mov  eax,[level] ; eax = levels*100
    mov  bx,100
    mul  bx
    add  esi,eax
    mov  ecx,10
    mov  eax,0
    mov dword [l_end],1
    .l_y:
      push ecx ;#>1
      mov ebx,0
      mov ecx,10
      .l_x:
        cmp byte [esi],1 ; if 1 ?
        push esi;#>2
        jne .no_1
          mov  esi,img_brick1
          call put_bmp_screen
          mov dword [l_end],0
        .no_1:
        cmp byte [esi],2 ; if 2 ?
        jne .no_2
          mov  esi,img_brick2
          call put_bmp_screen
          mov dword [l_end],0
        .no_2:
        cmp byte [esi],3 ; if 3 ?
        jne .no_3
          mov  esi,img_brick3
          call put_bmp_screen
          mov dword [l_end],0
        .no_3:
        cmp byte [esi],4 ; if 4 ?
        jne .no_4
          mov  esi,img_brick4
          call put_bmp_screen
        .no_4:

        add ebx,40
        pop esi ;#<2
        inc esi
      loop .l_x
      add eax,20 ;#<1
      pop  ecx
    loop .l_y
;----------------
; NEXT LEVEL
    if_e dword [l_end],1,.end_if ; all bricks are taken
    if_e dword [mode],1,.end_if
        add dword [level],1
        if_a dword [level],MAX_LEVEL,.end_if2
           mov dword [mode],4 ; game over
           jmp still
        .end_if2:
        call fast_gfx
        mov dword [mode],2
    .end_if:
ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    startwd

    window 100,100,X_SIZE+8,Y_SIZE+21,0x03ffffff
    label 8,8,VERSION,cl_White+font_Big
    label 200,8,'LIVES:',0x10ddeeff
    outcount dword [lives],250,8,0x10ddeeff,65536

    call fast_gfx

    endwd

    ret

;-----------;####################
; DATA AREA ;####################
;-----------;####################

 lives dd 5
 mode dd 0
 l_end dd 0 ; if 1 the level is over
; PAD x:
 x   dd 20
 y   dd Y_SIZE-20
; PAD length:
 s_x dd 40

; the ball stuff ;-)
 b_x dd 100
 b_y dd 250
 v_y dd 0
 v_x dd 3

 speed_t dd 0 ; 1/10 times speedup
 del dd 1 ; delay

 coliz dd 0 ; if 1 then colizion with a brick

; LEVELS:
level dd 0
levels:
;LEVEL 0:
db 0,0,0,0,0,0,0,0,0,0
db 0,4,0,0,4,4,0,0,0,4
db 4,0,4,0,4,0,4,0,4,0
db 4,0,4,0,4,0,4,0,4,0
db 4,4,4,0,4,4,0,0,4,0
db 4,0,4,0,4,0,4,0,4,0
db 4,0,4,0,4,0,4,0,0,4
db 0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0,0,0

;LEVEL 1:
db 1,1,1,1,1,1,1,1,1,1
db 0,3,0,0,3,3,0,0,0,3
db 3,0,3,0,3,0,3,0,3,0
db 3,0,3,0,3,0,3,0,3,0
db 3,3,3,0,3,3,0,0,3,0
db 3,0,3,0,3,0,3,0,3,0
db 3,0,3,0,3,0,3,0,0,3
db 2,2,2,2,2,2,2,2,2,2
db 1,1,1,1,1,1,1,1,1,1
db 1,1,1,1,1,1,1,1,1,1
;LEVEL 2:
db 3,3,3,3,0,0,3,3,3,3
db 3,1,1,1,0,0,1,1,1,3
db 3,1,2,1,3,3,1,2,1,3
db 0,1,0,1,3,3,1,0,1,0
db 2,1,2,1,1,1,1,2,1,2
db 0,1,0,1,2,2,1,0,1,0
db 2,1,2,1,1,1,1,2,1,2
db 0,1,0,1,1,1,1,0,1,0
db 0,0,0,1,0,0,1,0,0,0
db 0,0,0,1,0,0,1,0,0,0
;LEVEL 3:
db 1,2,3,1,2,3,1,3,2,1
db 2,3,1,2,3,1,3,3,1,2
db 3,1,2,3,1,2,3,1,2,3
db 1,2,3,1,2,3,1,3,2,1
db 2,3,1,2,3,1,3,3,1,2
db 3,1,2,3,1,2,3,1,2,3
db 1,2,1,2,1,2,1,2,1,2
db 1,0,1,0,1,0,1,0,1,0
db 0,0,3,0,0,0,0,3,0,0
db 0,0,3,0,0,0,0,3,0,0
;LEVEL 4:
db 0,0,0,1,1,1,1,0,0,0
db 0,0,1,2,2,2,2,1,0,0
db 1,1,1,2,2,2,2,1,1,1
db 1,0,1,0,2,2,0,1,0,1
db 0,1,1,2,2,2,2,1,1,0
db 0,0,1,2,2,2,2,1,0,0
db 0,0,1,2,2,2,2,1,0,0
db 0,0,1,2,3,3,2,1,0,0
db 0,0,1,2,2,2,2,1,0,0
db 0,0,0,1,1,1,1,0,0,0
;LEVEL 5:
db 1,1,1,1,1,1,1,1,1,1
db 1,2,0,0,3,2,0,0,2,1
db 1,2,0,0,2,3,0,0,2,1
db 2,2,0,0,3,2,0,0,2,2
db 0,0,0,0,2,3,0,0,0,0
db 0,0,0,1,1,1,1,0,0,0
db 0,0,1,1,0,0,1,1,0,0
db 0,0,1,1,0,0,1,1,0,0
db 2,1,2,1,2,1,2,1,2,1
db 1,2,1,2,1,2,1,2,1,2


; BITMAPs and IMAGEs
bmp_file:
    file 'arcanii.bmp'

img_bmp:
    rb 40*90*3+8
img_brick1:
    rb 40*20*3+8
img_brick2:
    rb 40*20*3+8
img_brick3:
    rb 40*20*3+8
img_brick4:
    rb 40*20*3+8
img_ball:
    rb 10*10*3+8
img_bonus:
    rb 20*10*3+8


screen_img:
    dd X_SIZE
    dd Y_SIZE
screen:
    rb X_SIZE*Y_SIZE*3

I_END:

