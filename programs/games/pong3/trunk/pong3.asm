;
;   Pong Gaem 3Ver Mini Sample by Pavlushin Evgeni for ASCL
;   www.waptap@mail.ru
;
;   Not use bmpfile!

;******************************************************************************
format binary as ""

    use32
    org    0x0
    db     'MENUET01'              ; 8 byte id
    dd     0x01                    ; header version
    dd     START                   ; start of code
    dd     IM_END                  ; size of image
    dd     0x300000                ; memory for app
    dd     0x300000                ; esp
    dd     0x0, 0x0                ; I_Param , I_Path

;******************************************************************************

include '..\..\..\macros.inc'
include 'ascl.inc'
include 'ascgl.inc'
include 'ascgml.inc'

AREA_W = 640
AREA_H = 480


START:                          ; start of execution
        convbmp pongfile, tsoi
        bmptoimg pongfile, tsoi,pong
        getimg pong, 0, 0,  80, 4, img
        getimg pong, 0, 4,  80, 4, img2
        getimg pong, 5, 38, 15, 15, img3
        getimg pong, 0, 10, 80, 20, img5
        fullimg imgbr, 80, 4,  0x00000000  ;black for rocket
        fullimg imgbg, 80, 20, 0x00000000  ;black for grav
        fullimg imgbb, 12, 12, 0x00000000  ;black for ball

still:
        scevent red,key,button

        ;mcall 48, 4 ;get skin height
        ;sub [skin_h], 16
        ;div eax, 2
        ;mov [skin_h], eax

remember_old_coordinates:
        m2m [ply1x_old], [ply1x]
        m2m [ply1y_old], [ply1y]
        m2m [ply2x_old], [ply2x]
        m2m [ply2y_old], [ply2y]
        m2m [gravx_old], [gravx]
        m2m [gravy_old], [gravy]
        m2m [ballx_old], [ballx]
        m2m [bally_old], [bally]

move_ply1:
        correct [ply1x], [ply1rx], 4
        correct [ply1y], [ply1ry], 2

; automove ball
        mov     eax, [ballxv]
        add     [ballx], eax
        mov     eax, [ballyv]
        add     [bally], eax

;autoslow ball
        cmp     [ballxv], 0
        jl       ballb
balla:
        cmp     [ballxv], 2
        jng     balln
        dec     [ballxv]
        jmp     balln
ballb:
        cmp     [ballxv], -2
        jnl     balln
        inc     [ballxv]
        jmp     balln
balln:
        cmp     [ballyv], 2
        jng     by_n
        dec     [ballyv]
by_n:

;ball collusion of screen
        cmp     [ballx], AREA_W-12
        jna     xa_ok
        neg     [ballxv]
xa_ok:
        cmp     [ballx], 6
        jnb     xb_ok
        neg     [ballxv]
xb_ok:

;if ball far out of screen come back
; is not work already
        cmp     [bally], AREA_H-30 ;check RED fails
        jng     yax_ok
        inc     [scoreb]
        call    draw_score
        mov     [bally], 240
        mov     [ballx], 310
        mov     [ballyv], 2
        random 5, [ballxv]
        sub     [ballxv], 2
yax_ok:
        cmp     [bally], 0  ;check BLUE fails
        jnl     yax_ok2
        inc     [scorea]
        call    draw_score
        mov     [bally], 240
        mov     [ballx], 310
        mov     [ballyv], 2
        random 5, [ballxv]
        sub     [ballxv], 2
yax_ok2:

xorx:
        cmp     [ballxv], 0
        jne     norx
        random 5, [ballxv]
        sub     [ballxv], 2
        cmp     [ballxv], 0
        je      xorx
norx:

;test on collusion ply1 of ball
collusion_test:
        collimg img, [ply1x], [ply1y], img3, [ballx], [bally], eax
        cmp     eax, 1
        jne     not_coll
        neg     [ballyv]
        add     [bally], 4
;        neg     [ballxv]
not_coll:

;test on collusion com of ball
collusion_com:
        collimg img, [ply2x], [ply2y], img3, [ballx], [bally], eax
        cmp     eax, 1
        jne     not_collcom
        neg     [ballyv]
        sub     [bally], 4
;        neg dword [ballxv]
not_collcom:

;test on collusion gravity of ball
collusion_grav:
        collimg img, [gravx], [gravy], img3, [ballx], [bally], eax
        cmp     eax, 1
        jne     not_collg
        neg     [ballyv]

;        mov     [ballxv], -20

        cmp     [ballyv], 0
        jl      ab
        jg      bf
        jmp     not_collgx
ab:
        sub     [ballyv], 25
        jmp     not_collgx
bf:
        add     [ballyv], 25
not_collgx:
        cmp     [ballxv], 0
        jl      abx
        jg      bfx
        jmp     not_collg
abx:
        sub     [ballxv], 8 ;15
        jmp     not_collg
bfx:
        add     [ballxv], 8


;        mov     [ballyv], 20
not_collg:

;ply contorl
        cmp     [ply1rx], 560
        jna     plyok
        cmp     [ply1rx], 12000
        jna     paa
        mov     [ply1rx], 4
        jmp     plyok
paa:
        mov     [ply1rx], 560
plyok:


;com contorl
        cmp     [ply2x], 560
        jna     cplyok
        cmp     [ply2x], 12000
        jna     cpaa
        mov     [ply2x], 4
        jmp     cplyok
cpaa:
        mov     [ply2x], 560
cplyok:


;com move
cx_move:
        cmp     [bally], 160
        jna     cno_m
        mov     eax, [ballx]
        sub     eax, 30
        cmp     [ply2x], eax
        je      cno_m
        cmp     [ply2x], eax
        ja      cm_m
cm_p:
        add     [ply2x], 3
        jmp     cno_m
cm_m:
        sub     [ply2x], 3
cno_m:

;garvity
gravity:
        cmp     [gravtime], 0
        je      no_dg
        dec     [gravtime]
no_dg:

draw_gravity:
        cmp     [gravtime], 0
        je      nograv
        mov     eax, [ply1x]
        mov     ebx, [ply1y]
        add     ebx, 10
        mov     [gravx], eax
        mov     [gravy], ebx
        jmp     endgrav
nograv:
        mov     [gravx], 1000
        mov     [gravy], 1000
endgrav:

;next code checks were coordinates of player1 and player2
;changed or not
;    if yes => fill old
;    if no  => do not fill old
redraw_images:
		;player1
		;if (ply1x!=ply1x_old) || (ply1y!=ply1y_old) fill1
		mov    eax, [ply1x]
		cmp    [ply1x_old], eax
		jne    fill1
		mov    eax, [ply1y]
		cmp    [ply1y_old],eax
		jne    fill1
		jmp    no_fill1
	fill1:
        setimg [ply1x_old], [ply1y_old], imgbr
	no_fill1:
        setimg [ply1x], [ply1y], img
		
		;player2
		;if (ply2x!=ply2x_old) || (ply2y!=ply2y_old) fill2
		mov    eax, [ply2x]
		cmp    [ply2x_old], eax
		jne    fill2
		mov    eax, [ply2y]
		cmp    [ply2y_old],eax
		jne    fill2
		jmp    no_fill2
	fill2:
        setimg [ply2x_old], [ply2y_old], imgbr
	no_fill2:
        setimg [ply2x], [ply2y], img2
		
		;ball
        setimg [ballx_old], [bally_old], imgbb
        setimg [ballx], [bally], img3
		
		;grav
        setimg [gravx_old], [gravy_old], imgbg
        setimg [gravx], [gravy], img5

        delay 1             ;don't generate delay for fast speed programm

        jmp     still

  red:
        call    draw_window
        jmp     still

  key:                          ; key
        mov     eax, 2
        mcall
        cmp     ah, key_Left
        jne     no_l
        sub     [ply1rx], 32 ;16
no_l:
        cmp     ah, key_Right
        jne     no_r
        add     [ply1rx], 32 ;16
no_r:
        cmp     ah, key_Up
        jne     no_u
        sub     [ply1ry], 16
		cmp     [ply1ry], 0
		jb      no_u
		mov     [ply1ry], 0
no_u:
        cmp     ah, key_Down
        jne     no_d
        add     [ply1ry], 16
		cmp     [ply1ry], AREA_H-50
		jl      no_d
		mov     [ply1ry], AREA_H-50
no_d:
        cmp     ah, key_Space
        jne     no_sp
        mov     [gravtime], 100
no_sp:

        jmp     still

  button:
        mcall   17         ; get id
        cmp     ah, 1      ; button id=1 ?
        jne     noclose
        mcall   -1         ; close this program
  noclose:
        jmp     still


draw_window:
        mcall 12,1 ;start window redraw
        mcall 0, <10, AREA_W+5+9>, <10, 480+35>, 0x34000000,, wtitle 
        mcall 12,2 ;end window redraw

draw_score:
		mcall 13, <10, 100>, <AREA_H-17, 16>, 0
		outcount [scoreb], 10, AREA_H-17, 0x01000000 + cl_Blue, 3 shl 16
        outcount [scorea], 50, AREA_H-17, 0x01000000 + cl_Red, 3 shl 16
		ret

; DATA AREA
wtitle db 'PONG: use Arrow Keys and Space',0
;skin_h dd 25

ply1x_old dd ? 
ply1y_old dd ?
ply2x_old dd ?
ply2y_old dd ?
gravx_old dd ?
gravy_old dd ?
ballx_old dd ?
bally_old dd ?

xt              dd 100
yt              dd 100

gravtime        dd 10
gravx           dd 1000
gravy           dd 1000

ply1rx          dd 200
ply1ry          dd 40

ply1x           dd 200
ply1y           dd 40

ply2x           dd 200
ply2y           dd 400

ballx           dd 200
bally           dd 200

ballyv          dd 3
ballxv          dd 3

scorea          dd 0
scoreb          dd 0

counter         dd 0
tsoi            dd 0

pongfile        file 'pong.bmp'

IM_END:

temp            rb 20000

;real images
pong            rb 80*60*3+8
img             rb 32*32*3+8
img2            rb 32*32*3+8
img3            rb 32*32*3+8
img5            rb 32*32*3+8

;black to clean old images
imgbr           rb 80*4 *3+8
imgbg           rb 80*20*3+8
imgbb           rb 12*12*3+8

I_END:
