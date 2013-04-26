;
;   Arcanoid Gaem 4Ver Mini Sample by Pavlushin Evgeni for ASCL
;   www.waptap@mail.ru   www.cyberdeck.fatal.ru www.deck4.narod.ru
;   Play again and exit button added
;
;   3Ver Play again, Exit button.
;   4Ver Next level function.
;
;******************************************************************************
    use32
    org    0x0
    db     'MENUET01'              ; 8 byte id
    dd     0x01                    ; header version
    dd     START                   ; start of code
    dd     IM_END                  ; size of image
    dd     0x300000                ; memory for app
    dd     0x300000                ; esp
    dd     0x0 , 0x0         ; I_Param , I_Icon

;******************************************************************************

include 'lang.inc'
include '..\..\..\macros.inc'
include 'ascl.inc'
include 'ascgl.inc'
include 'ascgml.inc'


START:                          ; start of execution
    bmptoimg arc_file,pong
    getimg pong,0,0,80,4,img
    getimg pong,0,4,80,4,img2
    getimg pong,5,38,15,15,img3
    getimg pong,0,8,80,20,img5
    fullimg img4 , 80 ,20 ,0x00000000   ;black for rocket
    fullimg img6 , 15 ,15 ,0x00000000   ;black for ball
    fullimg img7a , 60,20,0x0000cf00
    fullimg img7b , 60,20,0x00af0000
    fullimg img7c , 60,20,0x000000cf
    fullimg img8 , 60,20,0x00000000

    call draw_window

still:

    scevent red,key,button

    cmp [againbut],1
    je  stl2

    outcount dword [scoreb],256,8,cl_Blue,5*65536
    outcount dword [scorea],332,8,cl_Red,5*65536
    outcount dword [level],368,8,cl_White,2*65536

del_images:
    setimg dword [ply1x],dword [ply1y],img4
    setimg dword [ballx],dword [bally],img6
    setimg dword [gravx],dword [gravy],img4

del_blocks:
    mov ecx,0
xxx:
    pushad
    mov esi,dword [mass+ecx]
    mov edi,dword [mass+ecx+4]
    mov ebp,dword [mass+ecx+8]
    cmp ebp,0
    jne notptx
    setimg esi,edi,img8
notptx:
    popad
    add ecx,12
    cmp ecx,[blocks_max]
    jne xxx



move_ply1:

    correct [ply1x],[ply1rx],2
    correct [ply1y],[ply1ry],2

;automove ball
    mov eax,dword [ballxv]
    add dword [ballx],eax
    mov eax,dword [ballyv]
    add dword [bally],eax

;autoslow ball for rocket gravitation (Space key)
    cmp dword [ballxv],0
    jl ballb
balla:
    cmp dword [ballxv],2
    jng balln
    dec dword [ballxv]
    jmp balln
ballb:
    cmp dword [ballxv],-2
    jnl balln
    inc dword [ballxv]
    jmp balln
balln:
    cmp dword [ballyv],2
    jng by_n
    dec dword [ballyv]
by_n:

;ball collusion of screen
    cmp dword [ballx],400-12
    jna xa_ok
    neg dword [ballxv]
xa_ok:
    cmp dword [ballx],6
    jnb xb_ok
    neg dword [ballxv]
xb_ok:

    cmp dword [bally],30
    jnb yb_ok
    neg dword [ballyv]
yb_ok:


;if ball far out of screen come back
    cmp dword [bally],466
    jng yax_ok
    call draw_window
    dec dword [scoreb]
    mov eax,[ply1ry]
    sub eax,6
    mov dword [bally],eax ;240
    mov eax,[ply1rx]
    add eax,30
    mov dword [ballx],eax ;200
    mov dword [ballyv],2
    random 3,dword [ballxv]
    sub dword [ballxv],1
yax_ok:

xorx:
    cmp dword [ballxv],0
    jne norx
    random 3,dword [ballxv]
    sub dword [ballxv],1
    cmp dword [ballxv],0
    je  xorx
norx:

;test on collusion ply1 of ball
collusion_test:
    collimg img,[ply1x],[ply1y],img3,[ballx],[bally],eax
    cmp eax,1
    jne not_coll
    neg dword [ballyv]
    sub dword [bally],4
;    neg dword [ballxv]
not_coll:

;test on collusion gravity of ball
collusion_grav:
    collimg img,[gravx],[gravy],img3,[ballx],[bally],eax
    cmp eax,1
    jne not_collg
    sub dword [bally],30
    neg dword [ballyv]
    cmp dword [ballyv],0
    jl  ab
    jg  bf
    jmp not_collgx
ab:
    sub dword [ballyv],10
    jmp not_collgx
bf:
    add dword [ballyv],10
not_collgx:
    cmp dword [ballxv],0
    jl  abx
    jg  bfx
    jmp not_collg
abx:
    sub dword [ballxv],0 ;8 ;15
    jmp not_collg
bfx:
    add dword [ballxv],0 ;8


;    mov dword [ballyv],20
not_collg:

;ply contorl
    control 12,316,[ply1rx]
    control 380,470,[ply1ry]

;garvity
gravity:
    cmp dword [gravtime],0
    je no_dg
    dec dword [gravtime]
no_dg:

draw_gravity:
    cmp dword [gravtime],0
    je  nograv
    mov eax,dword [ply1x]
    mov ebx,dword [ply1y]
    sub ebx,30
    mov dword [gravx],eax
    mov dword [gravy],ebx
    jmp endgrav
nograv:
    mov dword [gravx],1000
    mov dword [gravy],1000
endgrav:

redraw_images:
    setimg dword [ply1x],dword [ply1y],img2
    setimg dword [ballx],dword [bally],img3
    setimg dword [gravx],dword [gravy],img5

draw_blocks:
    mov ecx,0
xxx2:
    pushad
    mov esi,dword [mass+ecx]
    mov edi,dword [mass+ecx+4]
    mov ebp,dword [mass+ecx+8]
    cmp ebp,0
    je  notpt
    cmp ebp,1
    jne no_a
    setimg esi,edi,img7a
    jmp notpt
no_a:
    cmp ebp,2
    jne no_b
    setimg esi,edi,img7b
    jmp notpt
no_b:
    cmp ebp,3
    jne no_c
    setimg esi,edi,img7c
    jmp notpt
no_c:

notpt:
    popad
    add ecx,12
    cmp ecx,[blocks_max]
    jne xxx2

;collusion ball of blocks
coll_blocks:
    mov [temp3],0
    mov ecx,0
testloop:
    pushad
    mov ebp,dword [mass+ecx+8]
    cmp ebp,0
    jne testcol
    jmp notest
testcol:
    mov [temp3],1
    mov esi,dword [mass+ecx]
    mov edi,dword [mass+ecx+4]
    mov [temp1],esi
    mov [temp2],edi
    push ecx
    collimg img8,[temp1],[temp2],img3,[ballx],[bally],eax
    pop ecx
    cmp eax,1
    jne notest
    mov dword [mass+ecx+8],0
;    neg [ballxv]
    neg [ballyv]
    add [scorea],30
    call draw_window
    jmp end_col
notest:
    popad
    add ecx,12
    cmp ecx,[blocks_max]
    jne testloop
end_col:

    cmp [delay_cnt],0
    jne no_delay
    mov [delay_cnt],1
    delay 1             ;don't generate delay for fast speed programm
no_delay:
    dec [delay_cnt]

win_test:
    cmp [temp3],1
    je stl
;    inc [level]
    mov [nextlev],1
    mov [againbut],1
    call draw_window
;    label 160,200,'You Win!',cl_Green+font_Big
;    label 130,220,'Youre Score:',cl_Green+font_Big
;    outcount dword [scorea],230,220,cl_Green,5*65536
;    label 130,234,'Youre Lives:',cl_Green+font_Big
;    outcount dword [scoreb],230,234,cl_Green,5*65536
;    delay 600  ;wait 2sec
;    close      ;exit from program
stl:

lose_test:
    cmp [scoreb],0
    jne stl2
;    call draw_window
;    label 160,200,'You Lose!',cl_Red+font_Big
;    label 130,220,'Youre Score:',cl_Red+font_Big
;    outcount dword [scorea],230,220,cl_Red,5*65536
;    delay 300  ;wait 2sec
;    close      ;exit from program
;    mov ebx,10*65536+40
;    mov ebx,10*65536+20
;    mov edx,2
;    mov esi,0x0000ff00
;    mov eax,8
;    mcall
    mov [level],0
    mov [nextlev],0
    mov [againbut],1
    call draw_window

stl2:

    jmp  still

  red:
    call draw_window
    jmp  still

  key:                          ; key
    mov eax,2
    mcall
    cmp ah,key_Left
    jne no_l
    sub dword [ply1rx],50 ;24 ;16
no_l:
    cmp ah,key_Right
    jne no_r
    add dword [ply1rx],50 ;24 ;16
no_r:
    cmp ah,key_Up
    jne no_u
    sub dword [ply1ry],20
no_u:
    cmp ah,key_Down
    jne no_d
    add dword [ply1ry],20
no_d:
    cmp ah,key_Space
    jne no_sp
    mov dword [gravtime],100
no_sp:
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall
    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                 ; close this program
    mcall
noclose:
    cmp ah,2
    jne  noplayagain
    mov [xt],100
    mov [yt],100
    mov [gravtime],10
    mov [gravx],1000
    mov [gravy],1000
    mov [ply1rx],160
    mov [ply1ry],460
    mov [ply1x],160
    mov [ply1y],400
    mov [ballx],200
    mov [bally],300
    mov [ballyv],2
    mov [ballxv],1

    cmp [nextlev],1
    je  noch
    mov [scorex],0
    mov [scorea],0
    mov [scoreb],3
    jmp noch2
noch:
    inc [scoreb]
    inc [level]
    mov eax,[maxlev]
    cmp [level],eax
    jna noch2
    mov [level],eax
noch2:

    mov eax,18*4*5
    mul [level]
    mov ebp,eax
    add ebp,levels
    mov ecx,18*4*5   ;bytes
loo:
    mov eax,dword [ebp+ecx]
    mov dword [mass+ecx],eax
    sub ecx,4
    jnz loo

    mov [againbut],0

    call draw_window
noplayagain:
    jmp  still


draw_window:
    startwd
    window 0,0,400+8,480+24,window_Skinned
    if lang eq it
        label 12,8,'ARCANOID: Usa le freccie    Vite      Punti',cl_White+font_Big
    else
        label 12,8,'ARCANOID: USE ARROW KEYS    LIVES      SCORE',cl_White+font_Big
    end if


    cmp [againbut],0
    je  no_againbut

    cmp [nextlev],1
    je  nlev
    if lang eq it
        label 160,200,'Hai perso!',cl_Red+font_Big
        label 130,220,'Punteggio:',cl_Red+font_Big
    else
        label 160,200,'You Lose!',cl_Red+font_Big
        label 130,220,'Youre Score:',cl_Red+font_Big
    end if
    outcount dword [scorea],230,220,cl_Red,5*65536
    mov ebx,150*65536+80
    mov ecx,240*65536+12
    mov edx,2
    mov esi,0x0000aa00
    mov eax,8
    mcall
    mov ecx,260*65536+12
    mov edx,1
    mcall
    if lang eq it
        label 152,244,'Rigioca',cl_Red+font_Big
    else
        label 152,244,'Play again?',cl_Red+font_Big
    end if
    jmp elev
nlev:
    if lang eq it
        label 160,200,'Hai vinto!',cl_Green+font_Big
        label 130,220,'Punteggio:',cl_Green+font_Big
    else
        label 160,200,'You Win!',cl_Green+font_Big
        label 130,220,'Youre Score:',cl_Green+font_Big
    end if
    outcount dword [scorea],230,220,cl_Green,5*65536
    mov ebx,150*65536+120  ;mov ebx,150*65536+80
    mov ecx,240*65536+12
    mov edx,2
    mov esi,0x0000aa00
    mov eax,8
    mcall
    mov ecx,260*65536+12
    mov edx,1
    mcall
    if lang eq it
        label 152,244,'Prossimo Livello',cl_Red+font_Big
    else
        label 152,244,'Next level?',cl_Red+font_Big
    end if
elev:
    if lang eq it
        label 178,264,'Esci',cl_Red+font_Big
    else
        label 178,264,'Exit?',cl_Red+font_Big
    end if

no_againbut:

    endwd
    ret


; DATA AREA
againbut dd 0

xt dd 100
yt dd 100

gravtime dd 10
gravx dd 1000
gravy dd 1000

ply1rx dd 160
ply1ry dd 460

ply1x dd 160
ply1y dd 400

ballx dd 200
bally dd 300

ballyv dd 2
ballxv dd 1

temp1 dd 0
temp2 dd 0
temp3 dd 0

scorex dd 0
scorea dd 0
scoreb dd 3

level dd 0
nextlev dd 0
maxlev dd 2

counter dd 0
tsoi dd 0

delay_cnt dd 0

blocks_max dd 6*5*12 ;size in bytes 5*3 dd

mass:
     dd  30,200,0 ,90,200,0 ,150,200,0 ,210,200,0 ,270,200,0 ,330,200,0
     dd  30,180,0 ,90,180,0 ,150,180,0 ,210,180,0 ,270,180,0 ,330,180,0
     dd  30,160,0 ,90,160,0 ,150,160,0 ,210,160,0 ,270,160,0 ,330,160,0
     dd  30,140,0 ,90,140,1 ,150,140,1 ,210,140,1 ,270,140,0 ,330,140,0
     dd  30,120,0 ,90,120,0 ,150,120,0 ,210,120,0 ,270,120,0 ,330,120,0

levels:
     dd  30,200,0 ,90,200,0 ,150,200,0 ,210,200,2 ,270,200,0 ,330,200,0
     dd  30,180,2 ,90,180,1 ,150,180,1 ,210,180,2 ,270,180,2 ,330,180,0
     dd  30,160,1 ,90,160,3 ,150,160,1 ,210,160,2 ,270,160,2 ,330,160,2
     dd  30,140,2 ,90,140,1 ,150,140,1 ,210,140,2 ,270,140,2 ,330,140,0
     dd  30,120,0 ,90,120,0 ,150,120,0 ,210,120,2 ,270,120,0 ,330,120,0
;level2
     dd  30,200,0 ,90,200,3 ,150,200,0 ,210,200,2 ,270,200,2 ,330,200,2
     dd  30,180,0 ,90,180,1 ,150,180,0 ,210,180,2 ,270,180,2 ,330,180,2
     dd  30,160,0 ,90,160,3 ,150,160,0 ,210,160,2 ,270,160,2 ,330,160,2
     dd  30,140,0 ,90,140,1 ,150,140,0 ,210,140,2 ,270,140,2 ,330,140,2
     dd  30,120,0 ,90,120,3 ,150,120,0 ,210,120,2 ,270,120,2 ,330,120,2
;level3
     dd  30,200,1 ,90,200,3 ,150,200,1 ,210,200,2 ,270,200,2 ,330,200,2
     dd  30,180,2 ,90,180,1 ,150,180,1 ,210,180,2 ,270,180,2 ,330,180,2
     dd  30,160,1 ,90,160,3 ,150,160,1 ,210,160,2 ,270,160,2 ,330,160,2
     dd  30,140,2 ,90,140,1 ,150,140,1 ,210,140,2 ,270,140,2 ,330,140,2
     dd  30,120,1 ,90,120,3 ,150,120,1 ,210,120,2 ,270,120,2 ,330,120,2

arc_file:
file 'arc.bmp'

rb 20000

IM_END:

pong:
rb 80*60*3+8
img:
rb 32*32*3+8
img2:
rb 32*32*3+8
img3:
rb 32*32*3+8
img4:
rb 80*20*3+8
img5:
rb 80*20*3+8
img6:
rb 15*15*3+8
img7a:
rb 60*20*3+8
img7b:
rb 60*20*3+8
img7c:
rb 60*20*3+8
img8:
rb 60*40*3+8

I_END:
