;
;   Pong Gaem 2Ver Mini Sample by Pavlushin Evgeni for ASCL
;   www.waptap@mail.ru
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
    dd     temp_area , 0x0         ; I_Param , I_Icon

;******************************************************************************
;5941 ;4523(with new random)



include 'lang.inc'
include 'ascl.inc'
include 'ascgl.inc'

START:                          ; start of execution
    call draw_window

    loadbmp '/RD/1/MFAR.BMP',temp_area,I_END,tsoi
    bmptoimg I_END,tsoi,img
    loadbmp '/RD/1/COPY.BMP',temp_area,I_END,tsoi
    bmptoimg I_END,tsoi,img2
    loadbmp '/RD/1/SMILE.BMP',temp_area,I_END,tsoi
    bmptoimg I_END,tsoi,img3

    fullimg img4 , 32 ,32 ,0x00000000   ;black

    loadbmp '/RD/1/MBAR_I3.BMP',temp_area,I_END,tsoi
    bmptoimg I_END,tsoi,img5


still:

    mov  eax,11                 ; scan event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

out_scorea:
    mov eax,47
    mov ebx,5*65536
    mov ecx,[scorea]
    mov edx,300*65536+8
    mov esi,cl_Blue
    int 0x40
out_scoreb:
    mov eax,47
    mov ebx,5*65536
    mov ecx,[scoreb]
    mov edx,350*65536+8
    mov esi,cl_Red
    int 0x40

del_images:
    setimg dword [ply1x],dword [ply1y],img4
    setimg dword [ply2x],dword [ply2y],img4
    setimg dword [ballx],dword [bally],img4
    setimg dword [gravx],dword [gravy],img4


move_images:

x_move:
    mov eax,dword [ply1rx]
    cmp dword [ply1x],eax
    je  no_m
    cmp dword [ply1x],eax
    ja  m_m
m_p:
    add dword [ply1x],4
    jmp no_m
m_m:
    sub dword [ply1x],4
no_m:


y_move:
    mov eax,dword [ply1ry]
    cmp dword [ply1y],eax
    je  no_m2
    cmp dword [ply1y],eax
    ja  m_m2
m_p2:
    add dword [ply1y],2
    jmp no_m2
m_m2:
    sub dword [ply1y],2
no_m2:

;automove ball
    mov eax,dword [ballxv]
    add dword [ballx],eax
    mov eax,dword [ballyv]
    add dword [bally],eax


;autoslow ball
    cmp dword [ballxv],0
    jl ballb
balla:
    cmp dword [ballxv],2
    jng  balln
    dec dword [ballxv]
;   mov dword [ballxv],2
   jmp balln

ballb:
    cmp dword [ballxv],-2
    jng  balln
    inc dword [ballxv]
;   mov dword [ballxv],-2
   jmp balln

balln:

    cmp dword [ballyv],2
    jng by_n
    dec dword [ballyv]
by_n:

;test ball on collusion of screen
;    cmp dword [bally],480+16-32
;    jna ya_ok
;    neg dword [ballyv]
;ya_ok:
;    cmp dword [bally],30
;    jnb yb_ok
;    neg dword [ballyv]
;yb_ok:
    cmp dword [ballx],640-32
    jna xa_ok
    neg dword [ballxv]
xa_ok:
    cmp dword [ballx],6
    jnb xb_ok
    neg dword [ballxv]
xb_ok:

;if ball far out of screen come back
; is not work already
    cmp dword [bally],466
    jng yax_ok
    call draw_window
    inc dword [scoreb]
    mov dword [bally],240
    mov dword [ballx],310
    mov dword [ballyv],2
    random 5,dword [ballxv]
    sub dword [ballxv],2
yax_ok:
    cmp dword [bally],30
    jnl yax_ok2
    call draw_window
    inc dword [scorea]
    mov dword [bally],240
    mov dword [ballx],310
    mov dword [ballyv],2
    random 5,dword [ballxv]
    sub dword [ballxv],2
yax_ok2:

xorx:
    cmp dword [ballxv],0
    jne norx
    random 5,dword [ballxv]
    sub dword [ballxv],2
    cmp dword [ballxv],0
    je  xorx
norx:

;test on collusion ply1 of ball
collusion_test:
    collimg img,[ply1x],[ply1y],img3,[ballx],[bally],eax
    cmp eax,1
    jne not_coll
    neg dword [ballyv]
    add dword [bally],18
;    neg dword [ballxv]
not_coll:

;test on collusion com of ball
collusion_com:
    collimg img,[ply2x],[ply2y],img3,[ballx],[bally],eax
    cmp eax,1
    jne not_collcom
    neg dword [ballyv]
    sub dword [bally],18
;    neg dword [ballxv]
not_collcom:

;test on collusion gravity of ball
collusion_grav:
    collimg img,[gravx],[gravy],img3,[ballx],[bally],eax
    cmp eax,1
    jne not_collg
    neg dword [ballyv]

;    mov dword [ballxv],-20

    cmp dword [ballyv],0
    jl  ab
    jg  bf
    jmp not_collgx
ab:
    sub dword [ballyv],30
    jmp not_collgx
bf:
    add dword [ballyv],30
not_collgx:
    cmp dword [ballxv],0
    jl  abx
    jg  bfx
    jmp not_collg
abx:
    sub dword [ballxv],10
    jmp not_collg
bfx:
    add dword [ballxv],10


;    mov dword [ballyv],20
not_collg:

;com move
cx_move:
    cmp dword [bally],200
    jna cno_m
    mov eax,dword [ballx]
    cmp dword [ply2x],eax
    je  cno_m
    cmp dword [ply2x],eax
    ja  cm_m
cm_p:
    add dword [ply2x],3
    jmp cno_m
cm_m:
    sub dword [ply2x],3
cno_m:

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
    add ebx,20
    mov dword [gravx],eax
    mov dword [gravy],ebx
    jmp endgrav
nograv:
    mov dword [gravx],1000
    mov dword [gravy],1000
endgrav:

redraw_images:
    setimg dword [ply1x],dword [ply1y],img
    setimg dword [ply2x],dword [ply2y],img2 ;2
    setimg dword [ballx],dword [bally],img3
    setimg dword [gravx],dword [gravy],img5


    delay 2             ;don't generate delay for fast speed programm

    jmp  still

  red:
    call draw_window
    jmp  still

  key:                          ; key
    mov eax,2
    int 0x40
    cmp ah,key_Left
    jne no_l
    sub dword [ply1rx],16
no_l:
    cmp ah,key_Right
    jne no_r
    add dword [ply1rx],16
no_r:
    cmp ah,key_Up
    jne no_u
    sub dword [ply1ry],16
no_u:
    cmp ah,key_Down
    jne no_d
    add dword [ply1ry],16
no_d:
    cmp ah,key_Space
    jne no_sp
    mov dword [gravtime],100
no_sp:

    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                 ; close this program
    int  0x40
  noclose:
    jmp  still


draw_window:
    startwd
    window 0,0,640+8,480+24,window_Skinned
    label 12,8,'PONG: USE ARROW KEYS              SCORE',cl_White+font_Big
    endwd
    ret


; DATA AREA
xt dd 100
yt dd 100

gravtime dd 10
gravx dd 1000
gravy dd 1000

ply1rx dd 200
ply1ry dd 50

ply1x dd 200
ply1y dd 50

ply2x dd 200
ply2y dd 400

ballx dd 200
bally dd 200

ballyv dd 3
ballxv dd 3

scorea dd 0
scoreb dd 0

counter dd 0
tsoi dd 0

IM_END:

img:
rd 32*32*3+8
img2:
rb 32*32*3+8
img3:
rb 32*32*3+8
img4:
rb 32*32*3+8
img5:
rb 32*32*3+8

temp_area:
rb 0x15000

I_END:
