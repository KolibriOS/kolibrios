;
;    PONG for MENUET v1.0
;    2001 by Mario Birkner, Germany
;    cyflexx@digitalrice.com
;
;    PONG for MENUET is
;    a small PONG-clone for MenuetOS
;
; HINT: If the Paddle moves too slow,increase the
;       typematic Rate in your BIOS
;

include '..\..\..\macros.inc'

CK_UP1 equ 113
CK_DOWN1 equ 97
CK_UP2 equ 130+48
CK_DOWN2 equ 129+48

use32

                org     0x0

                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x2000                  ; required amount of memory
                dd      0x2000                  ; esp = 0x7FFF0
                dd      0x00000000              ; reserved=no extended header



START:

    call draw_window


still:

    mov  eax,10
    add  eax,[control]
    mcall
    cmp  eax,1
    je   red
    cmp  eax,2
    je   key
    cmp  eax,3
    je   button
    cmp  [control],1
    jne  still

    mov  eax,5
    mov  ebx,[delay]
    mcall

    jmp  move

 red:
    call draw_window
    cmp  [control],1
    jne  still
    call clall
    call drawpad
    jmp  still
 key:
    mov  eax,2
    mcall
    cmp  [control],1
    jne  still

   up1:
    cmp  ah,CK_UP1
    jne  dn1
    cmp  [posya],52*65536+64
    je   still
    sub  [posya],4*65536
    call cl0
    call drawpad
    jmp  still
   dn1:
    cmp  ah,CK_DOWN1
    jne  up2
    cmp  [posya],140*65536+64
    je   still
    add  [posya],4*65536
    call cl0
    call drawpad
    jmp  still
   up2:
    cmp  ah,CK_UP2
    jne  dn2
    cmp  [posyb],52*65536+64
    je   still
    sub  [posyb],4*65536
    call cl1
    call drawpad
    jmp  still
   dn2:
    cmp  ah,CK_DOWN2
    jne  still
    cmp  [posyb],140*65536+64
    je   still
    add  [posyb],4*65536
    call cl1
    call drawpad
    jmp  still

  button:
    mov  eax,17
    mcall

    cmp  ah,1
    jne  button2
    mov  eax,-1
    mcall

    jmp  still

  button2:
    cmp  ah,2
    jne  still
    mov  [control],1
    mov  [scp1],0
    mov  [scp2],0
    jmp  res
 move:
    mov  eax,[mposx]
    mov  ebx,[mposy]
    add  [bposx],eax
    add  [bposy],ebx
    cmp  [bposx],16*65536+16
     je  pf1
    cmp  [bposx],272*65536+16
     je  pf2
    cmp  [bposy],191*65536+16
     je  bot
    cmp  [bposy],48*65536+16
     je  top
    cmp  [bposx],247*65536+16
     je  rig
     jg  padr
    cmp  [bposx],55*65536+16
     je  lef
     jl  padl
    jmp  draw

 padr:
    mov  ecx,[posyb]
    jmp  pad
 padl:
    mov  ecx,[posya]
 pad:
    mov  eax,[bposy]
    sub  eax,16
    mov  ebx,eax
    add  ebx,17*65536
    sub  ecx,64
    mov  edx,ecx
    add  edx,65*65536
    cmp  eax,edx
    je   top
    cmp  ebx,ecx
    je   bot
    jmp  draw

  bot:
    mov  [mposy],-65536
    jmp  draw
  top:
    mov  [mposy],65536
    jmp  draw
  rig:
    mov  eax,[posyb]
    sub  eax,1*65536-64 ;verhindert das der ball ins paddle eindringt
    mov  ebx,[posyb]
    add  ebx,64*65536-64
    mov  ecx,[bposy]
    add  ecx,16*65536-16
    mov  edx,[bposy]
    sub  edx,16
    cmp  ecx,eax
     je  blr
     jl  draw
    cmp  edx,ebx
     je  blr
     jl  blr
     jg  draw
   blr:
    mov  [mposx],-65536
    jmp  draw

  lef:
    mov  eax,[posya]
    sub  eax,1*65536+64
    mov  ebx,[posya]
    add  ebx,64*65536-64
    mov  ecx,[bposy]
    add  ecx,16*65536-16
    mov  edx,[bposy]
    add  edx,16
    cmp  ecx,eax
     je  bll
     jl  draw
    cmp  edx,ebx
     je  bll
     jl  bll
     jg  draw
   bll:
    mov  [mposx],65536
    jmp  draw

drawpad:
; draw paddle
    mov  edx,[posya]
    shr  edx,16
    add  edx,32*65536
    mov  ecx,24*65536+64
    mov  ebx,paddle
    mov  eax,7
    mcall
    mov  edx,[posyb]
    shr  edx,16
    add  edx,264*65536
    mov  eax,7
    mov  ebx,paddle
    mov  ecx,24*65536+64
    mcall
    ret

draw:
; draw ball
    mov  eax,13
    mov  ebx,-65536+2
    mov  ecx,-65536+2
    add  ebx,[bposx]
    add  ecx,[bposy]
    xor  edx,edx
    mcall
    add  ebx,65536-2
    add  ecx,65536-2
    mov  edx,ebx
    and  edx,0xffff0000
    mov  esi,ecx
    shr  esi,16
    add  edx,esi
    mov  esi,ebx
    shl  esi,16
    mov  edi,ecx
    and  edi,0x0000ffff
    add  esi,edi
    mov  ecx,esi
    mov  ebx,ball
    mov  eax,7
    mcall
    jmp  still
cls:
    mov  eax,13
    mov  edx,0x00000000
    mcall
    ret
cl0:
    mov  ebx,32*65536+24
    mov  ecx,[posya]
    sub  ecx,4*65536+60
    call cls
    mov  ecx,[posya]
    add  ecx,64*65536-60
    call cls
    ret
cl1:
    mov  ebx,264*65536+24
    mov  ecx,[posyb]
    sub  ecx,4*65536+60
    call cls
    mov  ecx,[posyb]
    add  ecx,64*65536-60
    call cls
    ret
clall:
    mov  ebx,16*65536+288
    mov  ecx,47*65536+161
    call cls
    ret
pf1:
    inc  [scp2]
    jmp  res
pf2:
     inc  [scp1]
 res:
    cmp  [scp2],20
     je  over
    cmp  [scp1],20
     je  over
    mov  [bposx],152*65536+16 ;default position ball
    mov  [bposy],119*65536+16
    call draw_window
    call clall
    call drawpad
    jmp  draw
over:
    mov  [control],0
    call draw_window
    mov  eax,4
    mov  ebx,120*65536+100
    mov  ecx,0x00ffdd00
    mov  esi,14
    cmp  [scp1],20
    jne  win
    mov  edx,w1
    mcall
    jmp  still
 win:
    mov  edx,w2
    mcall
    jmp  still

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12
    mov  ebx,1
    mcall


    mov  eax,0
    mov  ebx,100*65536+320
    mov  ecx,100*65536+250
    mov  edx,0x144873a0    ;70d0
    mov  edi,labelt
    mcall

    mov  eax,8
    mov  ebx,20*65536+80
    mov  ecx,220*65536+20
    mov  edx,2
    mov  esi,0x900000
    mcall

    mov  eax,4
    mov  ebx,38*65536+227
    mov  ecx,0x00FFFFFF
    mov  edx,b0lab
    mov  esi,8
    mcall

    mov  eax,4
    mov  ebx,25*65536+30
    mov  ecx,0x00ffdd00
    mov  edx,welcome
    mov  esi,40
    mcall

    mov  eax,4
    mov  ebx,220*65536+228
    mov  ecx,0x00ffdd00
    mov  edx,scotext
    mov  esi,11
    mcall

    mov  eax,47
    mov  ebx,0x00020000
    mov  ecx,[scp1]
    mov  edx,268 shl 16+228
    mov  esi,0x0000ddff
    mcall

    mov  eax,47
    mov  ebx,0x00020000
    mov  ecx,[scp2]
    mov  edx,285 shl 16+228
    mov  esi,0x0000ddff
    mcall

    mov  eax,12
    mov  ebx,2
    mcall

    ret


; DATA AREA
posya   dd 96*65536+64  ;default position paddle
posyb   dd 96*65536+64
bposx   dd 152*65536+16 ;default position ball
bposy   dd 119*65536+16
mposx   dd 65536        ;richtung ball
mposy   dd 65536
control dd 0x0
delay   dd 0x1          ;delay betw. frames
scp1    dd 0x0
scp2    dd 0x0


scotext:
     db  'SCORE:    :'
welcome:
     db  'PLAYER1: Q , A            PLAYER2:  , '
b0lab:
     db  'NEW GAME'
labelt:
     db  'PONG FOR MENUET v1.0',0
w1:
     db  'Player 1 wins!'
w2:
     db  'Player 2 wins!'
clsign:
     db   'x'

ball:
file "ball.raw"

paddle:
file "paddle.raw"

I_END:
