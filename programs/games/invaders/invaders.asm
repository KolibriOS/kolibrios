;=============================================================================;
; Hidnplayr's invaders, Compilable for DexOs and Kolibrios                    ;
;-----------------------------------------------------------------------------;
;                                                                             ;
; Copyright (C) hidnplayr 2007. All rights reserved.                          ;
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


SCREEN_X equ 640
SCREEN_Y equ 480

SHIP_X equ 32
SHIP_Y equ 32

SHIP_X_POS equ (SCREEN_X-SHIP_X)/2
SHIP_Y_POS equ SCREEN_Y-SHIP_Y-27

ENEMY_X equ 32
ENEMY_Y equ 32

ALIEN_X equ 48
ALIEN_Y equ 38
ALIEN_Y_POS equ 1

BOUNDARY equ 10
MOVEMENT equ 7

TRANSPARENCY equ 0x00000000

WINDOW_X equ 100
WINDOW_Y equ 100

BULLET_X equ 10
BULLET_Y equ 10

STARS_	 equ 226
STARLEVELS equ 3

ENEMY_STARTING_X equ 25
ENEMY_STARTING_Y equ 50

BULLETSPEED equ 12

SCREEN_X_POS equ 5
SCREEN_Y_POS equ 25

gif_hash_offset = gif_hash_area

include 'ascgl.inc'
include 'invaders_kolibri.inc'

decodegif:
	giftoimg gif_bullet,bullet
	giftoimg gif_bullet2,bullet2
	giftoimg gif_ship,ship
	giftoimg gif_enemy1,enemy1
	giftoimg gif_enemy2,enemy2
	giftoimg gif_enemy3,enemy3
	giftoimg gif_enemy4,enemy4
	giftoimg gif_enemy5,enemy5
	giftoimg gif_alien,alien
	giftoimg gif_menu1,menu1
	giftoimg gif_menu2,menu2
	giftoimg gif_menu3,menu3
	giftoimg gif_menu4,menu4
	giftoimg gif_logo,logo
	giftoimg gif_pause,pause_
	giftoimg gif_levelup,levelup
	giftoimg gif_gameover,gameover
	giftoimg gif_highscore,highscore
	giftoimg gif_smallfont,smallfont
	giftoimg gif_bigfont,bigfont
	giftoimg gif_numbers,numbers

	call  createstars
	call  drawit
	call  [drawroutine]

mainloop:

    cmp  byte[status],3 ; if game is paused,...
    je	 waitfordraw

    call drawit
    call [drawroutine]
    call checkbullet

   waitfordraw:

    call waitandgetkey ;;;
    test ah,ah
    jz	 mainloop

    cmp  byte[status],1      ;
    jne  nogame

	 cmp  ah, KEY_RIGHT
	 jnz  noright
	 cmp  dword[ship_x],SCREEN_X-SHIP_X-BOUNDARY
	 jge  mainloop
	 add  dword[ship_x],MOVEMENT

	 jmp  mainloop

       noright:
	 cmp  ah, KEY_LEFT
	 jnz  noleft
	 cmp  dword[ship_x],BOUNDARY
	 jle  mainloop
	 sub  dword[ship_x],MOVEMENT

	 jmp  mainloop

       noleft:
	 cmp  ah, KEY_UP
	 jnz  noup

	 cmp  dword[bullet_y],1
	 jg   mainloop

	 mov  eax,dword[ship_x]
	 add  eax,(SHIP_X-BULLET_X)/2
	 mov  dword[bullet_x],eax
	 mov  dword[bullet_y],SHIP_Y_POS;-BULLET_Y

	 jmp  mainloop

       noup:
	 cmp  ah,KEY_P
	 jnz  no_pause1

	 mov  byte[status],3
	 aimgtoimg pause_,150,180,vscreen,TRANSPARENCY,0
	 call [drawroutine]

	 jmp  mainloop

       no_pause1:

nogame:
    cmp  byte[status],0
    jne  nomenu

	 cmp  ah, KEY_DOWN
	 jnz  no_down

	 cmp  byte[menu],3
	 jne  no_menu1

	 mov  byte[menu],0
	 jmp  nomenu

       no_menu1:
	 inc  byte[menu]

       no_down:
	 cmp  ah, KEY_UP
	 jnz  no_up

	 cmp  byte[menu],0
	 jne  no_menu0

	 mov  byte[menu],3
	 jmp  nomenu

       no_menu0:
	 dec  byte[menu]

       no_up:

	 cmp  ah, KEY_ESC
	 je   exit

       no_esc:
	 cmp  ah, KEY_ENTER
	 jnz  no_enter

	exemenu:
		cmp  byte[menu],0  ;start
		jne  exemenu1

	      new_game:
			mov  dword[score],0
			mov  eax,[score]
			call convertscore

			mov  esi, level1
			mov  word[levelnumb],'01'
	      load_level:
			mov  byte[enemy_speed],1
			mov  dword[enemy_x], ENEMY_STARTING_X
			mov  dword[enemy_y], ENEMY_STARTING_Y

			mov  edi, enemy_table
			mov  ecx, 5
			rep  movsd

			mov  byte[status],1

			jmp  mainloop

	exemenu1:
		cmp  byte[menu],1  ;about
		jne  exemenu2
		mov  byte[status],4
		jmp  mainloop

	exemenu2:
		cmp  byte[menu],2  ;highscores
		jne  exemenu3
		mov  byte[status],5
		call load_highscores
		jmp  mainloop

	exemenu3:
		cmp  byte[menu],3  ;exit
		je   exit

      no_enter:
nomenu:
    cmp  byte[status],3
    jne  nopause

    cmp  ah, KEY_P
    jnz  nopause

    mov  byte[status],1

nopause:
    cmp  byte[status],6
    jne  nolevelup

    cmp  ah, KEY_ENTER
    jne  nolevelup

	inc  byte[level]

;        cmp  byte[level],5
;        jne  @f
;        mov  byte[level],0

;@@:
	inc  byte[levelnumb+1]
	cmp  byte[levelnumb+1],'9'
	jle  @f
	mov  byte[levelnumb+1],'0'
	inc  byte[levelnumb]

       @@:
	mov  eax,20
	mov  ah,byte[level]
	and  ah,7
	mul  ah
	add  eax,level1
	mov  esi,eax
	jmp  load_level

nolevelup:
    cmp  byte[status],7
    jne  nohighscore

	cmp   ah, KEY_ENTER
	jne   @f

	call  load_highscores
	mov   eax,dword[score]
	mov   ebx,gif_hash_area+140
    .findscore:
	cmp   ebx,gif_hash_area+100
	je    .topscore
	sub   ebx,4
	cmp   eax,dword[ebx]
	jg    .findscore

    .topscore:
	mov   esi,name
	mov   edi,gif_hash_area
	mov   ecx,10
	rep   movsb

	mov   eax,dword[score]
	mov   dword[gif_hash_area+100],eax

	call  save_highscores
	mov   byte[status],5

@@:
	cmp   ah,14
	jne   @f

	cmp   byte[namepos],0
	je    @f

	dec   byte[namepos]
	movzx ebx,byte[namepos]
	add   ebx,name
	mov   byte[ebx],0x11  ; this is a character we dont print

@@:
	cmp   byte[namepos],10
	jge   nohighscore

	cmp   al,'0'
	jl    nohighscore
	cmp   al,'9'
	jle   @f

	cmp   al,'z'
	jg    nohighscore
	cmp   al,'a'
	jge   @f

	cmp   al,'Z'
	jg    nohighscore
	cmp   al,'A'
	jl    nohighscore
@@:

	movzx ebx,byte[namepos]
	add   ebx,name
	mov   byte[ebx],al

	inc   byte[namepos]

	jmp   mainloop

nohighscore:
    cmp  byte[status],2 ; gameover
    jne  nogameover

    cmp  ah, KEY_ENTER
    jne  nogameover

	; test if score is high enough to put in highscore list...
	mov   byte[status],7

	jmp   mainloop

nogameover:
    cmp  byte[status],0
    je	 mainloop

    cmp  ah, KEY_ESC
    jnz  mainloop

    mov  byte[status],0
    mov  word[intro],0

    jmp  mainloop

 ;----------------------------------------------------;
 ; Drawing routine: create image in buffer            ;
 ;----------------------------------------------------;
drawit:
    mov  eax,0x00000000
    call fillscreen
    call drawstars

    cmp  byte[status],1
    jne  @f

	call drawbullet
	call drawenemys 					      ; Draw the enemy's to buffer
	aimgtoimg ship,dword[ship_x],SHIP_Y_POS,vscreen,TRANSPARENCY,0	; Draw the ship to buffer

	mov  esi,scoretext
	mov  ebx,0
	mov  ecx,SCREEN_Y-24
	call printtext

	mov  esi,leveltext
	mov  ebx,300
	call printtext

    ret

@@:
    cmp  byte[status],2 ; game over, dude !
    jne  @f

	aimgtoimg ship,dword[ship_x],SHIP_Y_POS,vscreen,TRANSPARENCY,0	; Draw the ship to buffer

	mov  esi,scoretext
	mov  ebx,0
	mov  ecx,SCREEN_Y-24
	call printtext

	mov  esi,leveltext
	mov  ebx,300
	call printtext
	aimgtoimg gameover,150,180,vscreen,TRANSPARENCY,0

	ret

@@:
    cmp  byte[status],4 ; about screen
    jne  @f

	mov  esi,msgAbout
	mov  ebx,50
	mov  ecx,100
	call printtext

	ret

@@:
    cmp  byte[status],6 ; level up!
    jne  @f
	aimgtoimg ship,dword[ship_x],SHIP_Y_POS,vscreen,TRANSPARENCY,0	; Draw the ship to buffer

	mov  esi,scoretext
	mov  ebx,0
	mov  ecx,SCREEN_Y-24
	call printtext

	mov  esi,leveltext
	mov  ebx,300
	call printtext
	aimgtoimg levelup,150,180,vscreen,TRANSPARENCY,0

	ret

@@:
    cmp  byte[status],0 ; menu!
    jne  @f

	aimgtoimg logo,50,80,vscreen,TRANSPARENCY,0


	cmp  byte[menu],0
	jne  .menu_0
	aimgtoimg menu1,30,200,vscreen,TRANSPARENCY,1
	jmp  .menu_1
    .menu_0:
	aimgtoimg menu1,30,200,vscreen,TRANSPARENCY,0
    .menu_1:
	cmp  byte[menu],1
	jne  .menu_2
	aimgtoimg menu2,80,250,vscreen,TRANSPARENCY,1
	jmp  .menu_3
    .menu_2:
	aimgtoimg menu2,80,250,vscreen,TRANSPARENCY,0
    .menu_3:
	cmp  byte[menu],2
	jne  .menu_4
	aimgtoimg menu3,120,300,vscreen,TRANSPARENCY,1
	jmp  .menu_5
    .menu_4:
	aimgtoimg menu3,120,300,vscreen,TRANSPARENCY,0
    .menu_5:
	cmp  byte[menu],3
	jne  .menu_6
	aimgtoimg menu4,150,350,vscreen,TRANSPARENCY,1
	jmp  .menu_7
    .menu_6:
	aimgtoimg menu4,150,350,vscreen,TRANSPARENCY,0
    .menu_7:

	cmp  word[intro],200
	je   .menu_75
	inc  word[intro]

    .menu_75:
	cmp  word[intro],0
	jl   .menu_8
	aimgtoimg enemy1,390,180,vscreen,TRANSPARENCY,0

	cmp  word[intro],15
	jl   .menu_8
	mov  esi,points_50
	mov  ebx,470
	mov  ecx,180
	call printtext

	cmp  word[intro],30
	jl   .menu_8
	aimgtoimg enemy2,390,220,vscreen,TRANSPARENCY,0

	cmp  word[intro],45
	jl   .menu_8
	mov  esi,points_100
	mov  ebx,450
	mov  ecx,220
	call printtext

	cmp  word[intro],60
	jl   .menu_8
	aimgtoimg enemy3,390,260,vscreen,TRANSPARENCY,0

	cmp  word[intro],75
	jl   .menu_8
	mov  esi,points_150
	mov  ebx,450
	mov  ecx,260
	call printtext

	cmp  word[intro],90
	jl   .menu_8
	aimgtoimg enemy4,390,300,vscreen,TRANSPARENCY,0

	cmp  word[intro],105
	jl   .menu_8
	mov  esi,points_200
	mov  ebx,450
	mov  ecx,300
	call printtext

	cmp  word[intro],120
	jl   .menu_8
	aimgtoimg enemy5,390,340,vscreen,TRANSPARENCY,0

	cmp  word[intro],135
	jl   .menu_8
	mov  esi,points_250
	mov  ebx,450
	mov  ecx,340
	call printtext

	cmp  word[intro],150
	jl   .menu_8
	aimgtoimg alien,380,380,vscreen,TRANSPARENCY,0

	cmp  word[intro],165
	jl   .menu_8
	mov  esi,points_1000
	mov  ebx,430
	mov  ecx,380
	call printtext

    .menu_8:
	ret
@@:
    cmp  byte[status],5 ;highscorelist
    jne  @f

	aimgtoimg highscore,60,40,vscreen,TRANSPARENCY,0

	mov  ebx,100		    ; print names
	mov  ecx,120
	mov  esi,gif_hash_area
	call printtext

	mov  edi,gif_hash_area+100  ; print scores
	mov  esi,scorenumb
	mov  ebx,420
	mov  ecx,120

.highscoreloop:
	mov  eax,[edi]
	push ecx
	call convertscore
	pop  ecx
	push esi
	call printtext
	pop  esi
	add  ecx,26
	add  edi,4
	cmp  edi,gif_hash_area+140
	jl  .highscoreloop

@@:
    cmp  byte[status],7 ;highscore
    jne  @f

	aimgtoimg highscore,60,40,vscreen,TRANSPARENCY,0

	mov  ebx,60
	mov  ecx,200
	mov  esi,entername
	call printtext

	mov  ebx,250
	mov  ecx,250
	mov  esi,name
	call printtext

	mov  esi,scoretext
	mov  ebx,0
	mov  ecx,SCREEN_Y-24
	call printtext

	mov  esi,leveltext
	mov  ebx,300
	call printtext
@@:

    ret


drawenemys:
; check if direction should change
	test byte[enemy_d],2
	jz   @f

	add  dword[enemy_y],5

	mov  eax,[enemy_y]
	shr  eax,5
	add  al, byte[level]
	mov  byte[enemy_speed],al

	and  byte[enemy_d],1

       @@:
; move the aliens to left or right
	movzx eax,byte[enemy_speed]
	test byte[enemy_d],1
	jz   other_dir

	sub  dword[enemy_x],eax
	jmp  no_other_dir

     other_dir:
	add  dword[enemy_x],eax
     no_other_dir:

; initialization
	mov  byte[alldeadb],1
	mov  edi,enemy_table
	mov  eax,dword[enemy_x]
	mov  dword[current_enemy_x],eax
	mov  eax,dword[enemy_y]
	mov  dword[current_enemy_y],eax

     loopit:
	cmp  byte[edi],1
	je   drawenemy1

	cmp  byte[edi],2
	je   drawenemy2

	cmp  byte[edi],3
	je   drawenemy3

	cmp  byte[edi],4
	je   drawenemy4

	cmp  byte[edi],5
	je   drawenemy5

	jmp  dead_alien

     drawenemy1:
	mov  byte[alldeadb],0
	pusha
	aimgtoimg enemy1,dword[current_enemy_x],dword[current_enemy_y],vscreen,TRANSPARENCY,0
	popa

	jmp  checknext

     drawenemy2:
	mov  byte[alldeadb],0
	pusha
	aimgtoimg enemy2,dword[current_enemy_x],dword[current_enemy_y],vscreen,TRANSPARENCY,0
	popa

	jmp  checknext

     drawenemy3:
	mov  byte[alldeadb],0
	pusha
	aimgtoimg enemy3,dword[current_enemy_x],dword[current_enemy_y],vscreen,TRANSPARENCY,0
	popa

	jmp  checknext

     drawenemy4:
	mov  byte[alldeadb],0
	pusha
	aimgtoimg enemy4,dword[current_enemy_x],dword[current_enemy_y],vscreen,TRANSPARENCY,0
	popa

	jmp  checknext

     drawenemy5:
	mov  byte[alldeadb],0
	pusha
	aimgtoimg enemy5,dword[current_enemy_x],dword[current_enemy_y],vscreen,TRANSPARENCY,0
	popa

;        jmp  checknext

     checknext:
	cmp  byte[enemy_d],2
	jge  dont_change_dir

	   movzx eax,byte[enemy_speed]

	   cmp	byte[enemy_d],0
	   jbe	change_dir

	   cmp	dword[current_enemy_x],eax
	   jg	dont_change_dir

	   mov	byte[enemy_d],2

	   jmp	dont_change_dir

	change_dir:
	   mov	ebx, SCREEN_X-ENEMY_X
	   sub	ebx, eax
	   cmp	dword[current_enemy_x],ebx
	   jl	dont_change_dir

	   mov	byte[enemy_d],3

	dont_change_dir:
	   cmp	dword[current_enemy_y],SHIP_Y_POS-ENEMY_Y-BOUNDARY
	   je	gameover_

     dead_alien:
	cmp  edi,enemy_table+20
	jge  alldead

	inc  edi
	add  dword[current_enemy_x],ENEMY_X+BOUNDARY
	mov  eax,dword[current_enemy_x]
	sub  eax,dword[enemy_x]
	cmp  eax,5*(ENEMY_X+BOUNDARY)
	jl   no_newline

	sub  dword[current_enemy_x],5*(ENEMY_X+BOUNDARY)
	add  dword[current_enemy_y],ENEMY_Y+BOUNDARY
      no_newline:
	jmp  loopit

     alldead:
	cmp  byte[alldeadb],0
	je   enemy_end

	mov  byte[status],6
	jmp  mainloop

     enemy_end:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	cmp  dword[alien_x],5							  ;
	jge  @f

	call random_generator
	cmp  eax,0xffffffff/50 ; one out of 500 chances that it appears during this frame
	jl   alien_end

	mov  dword [alien_x],SCREEN_X-ALIEN_X

       @@:
	push  eax

	mov  eax, SCREEN_X																												   ;        mov  eax, SCREEN_X
	sub  eax, dword [alien_x]

	cmp  eax, ALIEN_X
	jle  @f
	mov  eax, ALIEN_X
       @@:

	getimg alien,0,0,10,ALIEN_Y,alienpiece
	aimgtoimg alien,dword [alien_x],ALIEN_Y_POS,vscreen,TRANSPARENCY,0
	sub  dword[alien_x],5							  ;
										  ;
	pop  eax								  ;
										  ;
      alien_end:								  ;
										  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	ret

drawbullet:
	cmp  dword[bullet_y],BULLETSPEED
	jl   nobullet
	sub  dword[bullet_y],BULLETSPEED

	aimgtoimg bullet,dword[bullet_x],dword[bullet_y],vscreen,TRANSPARENCY,0

      nobullet:
	ret

checkbullet:
	cmp  dword[bullet_y],BULLETSPEED			      ; does the bullet hit top of the screen?
	jle  hidebullet 					      ; yes, hide bullet

;        ; check if bullet is inside the enemy field (you can disble this if you want)
;        mov  eax,dword[enemy_y]
;        cmp  dword[bullet_y],eax
;        jl   nohit
;
;        add  eax,4*(ENEMY_Y+BOUNDARY)
;        cmp  dword[bullet_y],eax
;        jg   nohit
;
;        mov  eax,dword[enemy_x]
;        cmp  dword[bullet_x],eax
;        jl   nohit
;
;        add  eax,5*(ENEMY_Y+BOUNDARY)
;        cmp  dword[bullet_x],eax
;        jg   nohit
;
	mov  edi,enemy_table
	mov  eax,dword[enemy_x]
	mov  dword[current_enemy_x],eax
	mov  eax,dword[enemy_y]
	mov  dword[current_enemy_y],eax

       check:
	cmp  byte[edi],0		 ; is the enemy at this position alive?
	je   nextcheck			 ; no, try next enemy
	; check if bullet hits current enemy

	mov  eax,dword[current_enemy_y]  ; move the enemy y position into eax
	cmp  dword[bullet_y],eax	 ; is the bullet's y position less than eax (enemy y pos)
	jl   nextcheck			 ; yes, bullet can't be colliding, check next enemy

	add  eax,ENEMY_Y		 ; add the width of the enemy to the enemy's y position (wich is still stored in eax)
	cmp  dword[bullet_y],eax	 ; is the bullet's y position greater than eax (the end of the enemy)
	jg   nextcheck			 ; yes, bullet can't be colliding, check next enemy

	mov  eax,dword[current_enemy_x]  ; now do the same but for the x positions
	cmp  dword[bullet_x],eax	 ;
	jl   nextcheck			 ;
					 ;
	add  eax,ENEMY_Y		 ;
	cmp  dword[bullet_x],eax	 ;
	jg   nextcheck			 ;

	jmp  hit

      nextcheck:
	inc  edi
	add  dword[current_enemy_x],ENEMY_X+BOUNDARY
	mov  eax,dword[current_enemy_x]
	sub  eax,dword[enemy_x]
	cmp  eax,5*(ENEMY_X+BOUNDARY)
	jl   no_newline_

	sub  dword[current_enemy_x],5*(ENEMY_X+BOUNDARY)
	add  dword[current_enemy_y],ENEMY_Y+BOUNDARY
      no_newline_:

	cmp  edi,enemy_table+20 	; is this the last enemy?
	jg   nohit			; yes, none of them was hit
	jmp  check			; no, check if enemy is alive and draw it

      hit:
	movzx ebx,byte[edi]		; mov the enemy number onto ebx
	add  dword[score],ebx		; add this number to the score dword

	mov  eax,[score]
	call convertscore

	mov  byte[edi],0		; hide the enemy
     hidebullet:
	mov  dword[bullet_y],1		; mov the bullet to top of screen (hide it)
	jmp  noalienhit

     nohit:
	mov  eax,[alien_x]		; check if we hit the big alien in the ufo
	cmp  [bullet_x],eax
	jl   noalienhit
	add  eax,ALIEN_X-BULLET_X
	cmp  [bullet_x],eax
	jg   noalienhit
	cmp  [bullet_y],ALIEN_Y_POS+ALIEN_Y
	jg   noalienhit

	add  dword[score],100/5
	mov  eax,[score]
	call convertscore

	mov  [alien_x],0

     noalienhit:

ret

convertscore:
	test al,1
	jz   .1
	mov  byte[scorenumb+5],'5'
	jmp  .2
.1:
	mov  byte[scorenumb+5],'0'
.2:
	shr  eax,1
	mov  ecx,10
	xor  edx,edx
	div  ecx
	add  dl,'0'
	mov  byte[scorenumb+4],dl
	xor  edx,edx
	div  ecx
	add  dl,'0'
	mov  byte[scorenumb+3],dl
	xor  edx,edx
	div  ecx
	add  dl,'0'
	mov  byte[scorenumb+2],dl
	xor  edx,edx
	div  ecx
	add  dl,'0'
	mov  byte[scorenumb+1],dl
	xor  edx,edx
	div  ecx
	add  dl,'0'
	mov  byte[scorenumb+0],dl

ret


fillscreen: ; eax - screen color ( 0x00RRGGBB )

	mov  edi,vscreen+8
	cld
	mov  ecx,SCREEN_X*SCREEN_Y
    .lab1:
	mov  [edi],eax
	add  edi,3
	loop .lab1

	ret


printtext:
	push  ebx

     loadbyte:
	movzx eax, byte[esi]
	test  eax, eax
	jnz   checkbyte

	pop   ebx

	ret

    checkbyte:
	cmp  al,13
	je   nextline
	cmp  al,' '
	je   space

	cmp  al,'0'
	jl   nextchar
	cmp  al,'9'
	jle  usenumbers

	cmp  al,'z'
	jg   nextchar
	cmp  al,'a'
	jge  usesmallfont

	cmp  al,'Z'
	jg   nextchar
	cmp  al,'A'
	jge  usebigfont

	jmp  nextchar


    usesmallfont:
	mov  edx,eax
	sub  edx,'a'
	mov  eax,12
	mul  edx
	mov  edx,eax

	pusha
	getimg smallfont,0,edx,20,12,char
	popa

	pusha
	add  ecx,4
	aimgtoimg char,ebx,ecx,vscreen,TRANSPARENCY,0
	popa

	add  ebx,20

	jmp  nextchar

    usebigfont:
	mov  edx,eax
	sub  edx,'A'
	mov  eax,20
	mul  edx
	mov  edx,eax

	pusha
	getimg bigfont,0,edx,28,20,char
	popa

	pusha
	aimgtoimg char,ebx,ecx,vscreen,TRANSPARENCY,0
	popa

	add  ebx,28

	jmp  nextchar

    usenumbers:
	mov  edx,eax
	sub  edx,'0'
	mov  eax,20
	mul  edx
	mov  edx,eax

	pusha
	getimg numbers,0,edx,16,20,char
	popa

	pusha
	aimgtoimg char,ebx,ecx,vscreen,TRANSPARENCY,0
	popa

	add  ebx,20

	jmp  nextchar

    space:
	add  ebx,20

    nextchar:
	inc  esi

	jmp  loadbyte

    nextline:
	pop  ebx
	push ebx
	add  ecx,26
	inc  esi

	jmp  loadbyte

gameover_:
    mov  byte[status],2

    ret


drawstars:
    mov   esi, STARS

.loop:
    cmp   esi, STARS+(STARS_*5)
    jge   .done

    movzx eax, byte[esi]    ; z (speed, brightness)
    movzx ebx, word[esi+1]  ; x
    movzx ecx, word[esi+3]  ; y
    add   bx , ax
    cmp   bx , SCREEN_X
    jl	  .moveit

    xor   ebx,ebx
    inc   ebx

    call  random_generator
    mov   ecx, [generator]
    and   ecx, SCREEN_Y-1
    inc   ecx
    mov   word[esi+3],cx

    call  random_generator
    and   al, STARLEVELS
    test  al,al
    jnz   @f
    inc   al
   @@:
    mov   [esi],al

.moveit:
    mov   word [esi+1],bx

    movzx eax, byte[esi]
    inc   eax
    mov   edx, 0xff/(STARLEVELS+1)
    mul   edx

    mov   ah,al
    shl   eax,8
    mov   al,ah
    mov   ebp, eax

    mov   eax, SCREEN_X
    mul   ecx
    add   eax, ebx
    mov   edx, 3
    mul   edx

    cmp   eax, SCREEN_X*SCREEN_Y*3
    jg	  @f
    add   eax, vscreen+8
    and   dword[eax],0xff000000
    or	  dword[eax],ebp
   @@:

    add   esi, 5
    jmp   .loop
.done:

    ret


createstars:

    mov   ebx, STARS
.loop:
    cmp   ebx, STARS+(STARS_*5)
    jge   .done

    call  random_generator
    and   al, STARLEVELS
    test  al,al
    jnz   @f
    inc   al
   @@:
    mov   byte[ebx],al

    call  random_generator
    and   eax, SCREEN_X-1
    inc   eax
    mov   word[ebx+1],ax

    call  random_generator
    and   eax, SCREEN_Y-1
    inc   eax
    mov   word[ebx+3],ax

    add   ebx, 5
    jmp   .loop
.done:

    ret


random_generator:  ; (pseudo random, actually :)

    xor   eax,[generator]
    imul  eax,214013
    xor   eax,0xdeadbeef
    rol   eax,9
    mov   [generator],eax
    ror   eax,16
    and   eax,0x7fff

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


msg1		db 'Vesa mode not supported',13,'Press any key to exit.',13,0
msgAbout	db 'Hidnplayrs invaders',13,'DexOS version',13,13,'released under GPL',13,'make this game better',13,'if you want to',0
msgdone 	db 'You have saved the planet!',0
entername	db 'Enter your name highscorer!',0
highscorefile	db 'invaders.dat',0
points_50	db '5 pt',0
points_100	db '10 pt',0
points_150	db '15 pt',0
points_200	db '20 pt',0
points_250	db '25 pt',0
points_1000	db '100 pt',0
ship_x		dd SHIP_X_POS
enemy_x 	dd 0
enemy_y 	dd 0
enemy_d 	db 0
current_enemy_x dd 0
current_enemy_y dd 0
bullet_x	dd 0
bullet_y	dd 1
score		dd 0
alldeadb	db 0
status		db 0	    ; status: 0=menu  1=game  2=gameover   3=paused  4=about 5=highscorelist 6=levelup 7=highscore...
menu		db 0	    ; menu:   0=start 1=about 2=highscores 3=exit...
generator	dd 0x45dd4d15
alien_x 	dd 0
drawroutine	dd 0
returnaddr	dd 0
intro		dw 0
scoretext	db 'score '
scorenumb	db 0,0,0,0,0,0,0
leveltext	db 'level '
levelnumb	db 0,0,0
lives		db 0
level		db 1
enemy_speed	db 1
namepos 	db 0
name		db 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x0d,0x00


gif_bullet    file 'bullet2.gif'
rb 4

gif_bullet2   file 'bullet2.gif'
rb 4

gif_ship      file 'ship.gif'
rb 4

gif_enemy1    file 'enemy1.gif'
rb 4

gif_enemy2    file 'enemy2.gif'
rb 4

gif_enemy3    file 'enemy3.gif'
rb 4

gif_enemy4    file 'enemy4.gif'
rb 4

gif_enemy5    file 'enemy5.gif'
rb 4

gif_alien     file 'alien.gif'
rb 4

gif_menu1     file 'menu1.gif'
rb 4

gif_menu2     file 'menu2.gif'
rb 4

gif_menu3     file 'menu3.gif'
rb 4

gif_menu4     file 'menu4.gif'
rb 4

gif_logo      file 'logo.gif'
rb 4

gif_pause     file 'pause.gif'
rb 4

gif_highscore file 'highscores.gif'
rb 4

gif_smallfont file 'font_small.gif'
rb 4

gif_bigfont   file 'font_capital.gif'
rb 4

gif_numbers   file 'numbers.gif'
rb 4

gif_levelup   file 'nextlevel.gif'
rb 4

gif_gameover  file 'gameover.gif'
rb 4

vscreen:
dd SCREEN_X
dd SCREEN_Y
rb SCREEN_X*SCREEN_Y*3+10

IM_END:

STARS:
rb STARS_*5

bullet:
rb BULLET_X*BULLET_Y*3+8+10

bullet2:
rb BULLET_X*BULLET_Y*3+8+10

ship:
rb SHIP_X*SHIP_Y*3+10

enemy1:
rb ENEMY_X*ENEMY_Y*3+10

enemy2:
rb ENEMY_X*ENEMY_Y*3+10

enemy3:
rb ENEMY_X*ENEMY_Y*3+10

enemy4:
rb ENEMY_X*ENEMY_Y*3+10

enemy5:
rb ENEMY_X*ENEMY_Y*3+10

alien:
rb ALIEN_X*ALIEN_Y*3+10

menu1:
rb 220*18*3+10

menu2:
rb 135*18*3+10

menu3:
rb 245*18*3+10

menu4:
rb 110*18*3+10

logo:
rb 40*540*3+10

pause_:
rb 40*320*3+10

levelup:
rb 40*320*3+10

gameover:
rb 40*320*3+10

highscore:
rb 40*530*3+10

smallfont:
rb 20*312*3+10

bigfont:
rb 28*520*3+10

numbers:
rb 16*200*3+10

char:
rb 28*20*3+20 ; biggest char's are 28x20

alienpiece:
rb ALIEN_X*ALIEN_Y*3+20

gif_hash_area:
rd 4096+1			   ;hash area size for unpacking GIF

I_END:

;include 'dex.inc'

