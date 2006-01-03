;
;   Phenix Dynamic Game Created by Pavlushin Evgeni for ASCL
;
;   100% Full relase!
;
;   www.waptap@mail.ru
;


;******************************************************************************
    use32
    org    0x0
    db     'MENUET01'              ; 8 byte id
    dd     0x01                    ; header version
    dd     START                   ; start of code
    dd     IM_END                  ; size of image
    dd     I_END                   ; memory for app
    dd     I_END                   ; esp
    dd     0x0 , 0x0               ; I_Param , I_Icon

;******************************************************************************


include 'lang.inc'
include 'ascl.inc'
include 'ascgl.inc'
include 'ascml.inc'


showmas:
    cmp [edi+8],dword 0
    je  noshow
    cmp [edi+8],dword 1
    jne no_gun
    mov eax,shoot
    jmp outp
no_gun:
    push edi
    random 3,ebx
    pop edi
    cmp ebx,0
    jne no_star
    mov eax,star
    jmp outp
no_star:
    cmp ebx,1
    jne no_star2
    mov eax,star2
    jmp outp
no_star2:
    mov eax,star3
outp:
    aimgtoimg eax,dword [edi],dword [edi+4],canvas,0x0
noshow:
    ret

showobjmas:
    cmp [edi+8],dword 0
    je  noshow1
    cmp [edi+4],dword 380
    jg  noshow1
    cmp [edi+4],dword 0
    jl  noshow1
    cmp [edi+8],dword 1
    jne no_warship1
    mov eax,warship1
    jmp outws
no_warship1:
    cmp [edi+8],dword 2
    jne no_warship2
    mov eax,warship2
    jmp outws
no_warship2:
    cmp [edi+8],dword 3
    jne no_meteor
    mov eax,meteor
    jmp outws
no_meteor:
    cmp [edi+8],dword 4
    jne no_box
    mov eax,box
    jmp outws
no_box:
outws:
    aimgtoimg eax,dword [edi],dword [edi+4],canvas,0x0
noshow1:
    ret


moveobjmas:
    cmp [edi+8],dword 0
    je  no_ws
    mov eax,[edi+12]
    add [edi],eax
    mov eax,[edi+16]
    add [edi+4],eax

    cmp dword [edi],600
    jng xok1
    sub dword [edi],20
    neg dword [edi+12]
    jmp xok2
xok1:
    cmp dword [edi],0
    jnl xok2
    add dword [edi],20
    neg dword [edi+12]
xok2:
    cmp dword [edi+4],400
    jng yok
    mov dword [edi+8],0
    ret
yok:
    cmp dword [edi+8],2  ;green ship
    jne no_grs
    cmp dword [edi+4],100
    jna no_grs
    cmp dword [edi+4],103
    jna grs
    cmp dword [edi+4],200
    jna no_grs
    cmp dword [edi+4],203
    jna  grs
    cmp dword [edi+4],300
    jna no_grs
    cmp dword [edi+4],303
    ja  no_grs
grs:
    neg dword [edi+12]
    mov [temp],edi
    findmas massive,findzero
;in edi off to free element
    jc  close_app
    mov esi,edi
    mov edi,[temp]
    mov eax,[edi]
    mov [esi],eax
    mov eax,[edi+4]
    mov [esi+4],eax
    mov [esi+8],dword 1
    mov [esi+12],dword 0
    mov [esi+16],dword 10
no_grs:

    cmp dword [edi+8],1  ;blue ship
    jne no_bls
    cmp dword [edi+4],50
    jna no_bls
    cmp dword [edi+4],64
    jna bls
    cmp dword [edi+4],100
    jna no_bls
    cmp dword [edi+4],114
    jna bls
    cmp dword [edi+4],150
    jna no_bls
    cmp dword [edi+4],164
    ja  no_bls
bls:
    mov [temp],edi
    findmas massive,findzero
    jc  close_app
    mov esi,edi
    mov edi,[temp]
    mov eax,[edi]
    mov [esi],eax
    mov eax,[edi+4]
    mov [esi+4],eax
    mov [esi+8],dword 2
    mov [esi+12],dword 0
    mov [esi+16],dword 5
no_bls:

no_ws:
    ret


delfarshoot:
    cmp [edi+4],dword 40
    jb  del
    cmp [edi+4],dword 400
    ja  del
    cmp [edi],dword 40
    jb  del
    cmp [edi],dword 600
    ja  del
    jmp nodel
del:
    mov [edi+8],dword 0
nodel:
    ret


movemas:
    cmp [edi+8],dword 0
    jne no_freeel
    ret
no_freeel:
    mov eax,[edi+12]
    add [edi],eax
    mov eax,[edi+16]
    add [edi+4],eax
    ret



endshowmas:
    cmp [edi+8],dword 0
    je  noshowem
    mov eax,star2
    aimgtoimg eax,dword [edi],dword [edi+4],canvas,0x0
noshowem:
    ret



endmovemas:
    cmp [edi+8],dword 0
    jne no_fr
    ret
no_fr:
    mov eax,[edi+12]
    add [edi],eax
    mov eax,[edi+16]
    add [edi+4],eax
    ret


findzero:
    cmp [edi+8],dword 0
    je  iz_zero
    xor eax,eax
    ret
iz_zero:
    mov eax,1
    ret

compobr:
    cmp [esi+8],dword 0
    je  no_crsh
    cmp [edi+8],dword 0
    je  no_crsh
    cmp [esi+16],dword 0
    jg  no_crsh

    mov eax,[esi]
    mov [temp],eax
    mov eax,[esi+4]
    mov [temp2],eax
    mov eax,[edi]
    mov [temp3],eax
    mov eax,[edi+4]
    mov [temp4],eax

    pushad
    collimg imgsize,[temp],[temp2],imgsize,[temp3],[temp4],[otv]
    popad
    cmp [otv],dword 0
    je  no_crsh

    cmp [edi+8],dword 2
    jne no_grship
    inc [gship]
    add [score],30
    jmp setzero
no_grship:
    cmp [edi+8],dword 1
    jne no_blship
    inc [bship]
    add [score],20
    jmp setzero
no_blship:
    cmp [edi+8],dword 3
    jne no_metr
    dec dword [edi+16]
    cmp dword [edi+16],0
    jne mok
    mov dword [edi+16],1
mok:
    mov [esi+8],dword 0
    ret
no_metr:

setzero:
    mov [esi+8],dword 0
    mov [edi+8],dword 0
no_crsh:
    ret

shipobjtest:
    cmp [edi+8],dword 0
    je  no_obj
    mov eax,[edi]
    mov [temp3],eax
    mov eax,[edi+4]
    mov [temp4],eax
    pushad
    collimg imgsize,[shipx],[shipy],imgsize,[temp3],[temp4],[otv]
    popad
    cmp [otv],dword 0
    je  no_obj
    cmp [edi+8],dword 4  ;if box
    jne no_fbox
    add [energy],5
    add [score],50
    mov [edi+8],dword 0
    inc [boxget]
    ret
no_fbox:
    sub [energy],16
    mov [edi+8],dword 0
no_obj:
    ret

shipguntest:
    cmp [edi+8],dword 0
    je  no_gobj
    cmp [edi+16],dword 0
    jl  no_gobj
    mov eax,[edi]
    mov [temp3],eax
    mov eax,[edi+4]
    mov [temp4],eax
    pushad
    collimg imgsize,[shipx],[shipy],imgsize,[temp3],[temp4],[otv]
    popad
    cmp [otv],dword 0
    je  no_gobj
    sub [energy],4
    mov [edi+8],dword 0
no_gobj:
    ret


START:                          ; start of execution

massize = 400
elemsize = 20

    mov [massive],dword massize
    mov [massive+4],dword elemsize

omassize = 100
oelemsize = 20

    mov [objmas],dword omassize
    mov [objmas+4],dword oelemsize


    mov eax,66
    mov ebx,1
    mov ecx,1
    int 0x40

    mov eax,26
    mov ebx,2
    mov ecx,1
    mov edx,keymap
    int 0x40

startgame:
    gif_hash_offset = gif_hash_area
    giftoimg gif_file_area2,canvas

    gif_hash_offset = gif_hash_area
    giftoimg gif_file_area,img_area

    getimg img_area,0,0,32,32,ship
    getimg img_area,32,0,32,32,shoot
    getimg img_area,64,0,32,32,warship1
    getimg img_area,96,0,32,32,warship2
    getimg img_area,128,0,32,32,meteor
    getimg img_area,160,0,32,32,star
    getimg img_area,192,0,32,32,star2
    getimg img_area,224,0,32,32,star3
    getimg img_area,0,32,32,32,box



main_menu:
    call draw_logowindow

stillm:
    wtevent redm,keym,buttonm
    jmp stillm
redm:
    call draw_logowindow
    jmp stillm
keym:
    mov  eax,2
    int  0x40
    jmp  stillm
buttonm:
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    je   close_app
    cmp  ah,2                   ; button id=1 ?
    je   start_game
    cmp  ah,3                   ; button id=1 ?
    je   help
    cmp  ah,4                   ; button id=1 ?
    je   close_app
    jmp  stillm

draw_logowindow:
    call draw_window
    setimg 5,21,canvas
    drawlbut 300,300,60,14,'START',2,0x990000,cl_Black
    drawlbut 300,320,60,14,'HELP',3,0x990000,cl_Black
    drawlbut 300,340,60,14,'EXIT',4,0x990000,cl_Black
    ret

;***********************
; Draw help menu
;***********************

help:
    call draw_helpwindow

stillh:
    wtevent redh,keyh,buttonh
    jmp stillh
redh:
    call draw_helpwindow
    jmp stillh
keyh:
    mov  eax,2
    int  0x40
    jmp  stillh
buttonh:
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    je   close_app
    cmp  ah,2                   ; button id=1 ?
    je   start_game
    cmp  ah,3                   ; button id=1 ?
    je   help
    cmp  ah,4                   ; button id=1 ?
    je   close_app
    cmp  ah,5                   ; button id=1 ?
    je   main_menu
    jmp  stillh

draw_helpwindow:
    call draw_window
    setimg 5,21,canvas

    drawfbox 40,50,580,380,cl_Grey

    mov ebp,4*7
    mov ebx,180*65536+90
    mov edx,helptext
    mov esi,50
    mov ecx,cl_White
    dec ebp
looht:
    mov eax,4
    int 0x40
    add edx,esi
    add ebx,10
    dec ebp
    jnz looht

    setimg 90,90,ship
    setimg 90,130,shoot
    setimg 90,170,star
    setimg 90,210,warship1
    setimg 90,250,warship2
    setimg 90,290,meteor
    setimg 90,330,box

    drawlbut 500,400,80,14,'<<BACK',5,0x990000,cl_Black

    jmp stillh

helptext:
    db 'Phenix                                            '
    db 'Controls: Num1 move left, Num3 move right         '
    db '          P-pause (use for screen shooting)       '
    db '                                                  '

    db 'Lazer cannon                                      '
    db 'Press Num5 for shoot                              '
    db 'Core fast, speed fast, reload slow                '
    db '                                                  '

    db 'Plazma cannon                                     '
    db 'Press Num2 for Plazma Nuke and Num8 for shoot     '
    db 'Core slow, speed medium, reload fast              '
    db '                                                  '

    db 'Blue warship                                      '
    db 'Speed fast                                        '
    db 'Attack method: plazma bomb                        '
    db '                                                  '

    db 'Green warship                                     '
    db 'Speed slow                                        '
    db 'Attack method: laser shoot                        '
    db '                                                  '

    db 'Meteor                                            '
    db 'Dangeros object!                                  '
    db 'SuperSheld                                        '
    db '                                                  '

    db 'Fly Box                                           '
    db 'Sheld pack, sheld +5, score +30                   '
    db 'Get for sheld level up!                           '
    db '                                                  '

start_game:

;    jmp end_gm

    mov [canvas],dword 640
    mov [canvas+4],dword 440

    call draw_window

;Main loop wait for event with 10msec
still:
;    scevent red,key,button ;for full speed

    timeevent 1,no_event,red,key,button
no_event:
    setimg 5,21,canvas

    cmp [pause_on],0
    jne still

    cmp [energy],0
    jl  game_over

    cmp [ctime],dword 0
    je  no_dct
    dec dword [ctime]
no_dct:

    cmp [xtime],dword 0
    je  no_dxt
    dec dword [xtime]
no_dxt:


;
;   Add to level new ships
;
    inc dword [pathtime]
    mov ebp,[levelpart]
    shl ebp,5
    add ebp,levels
    mov eax,[ebp]
    cmp [pathtime],eax ;500
    jne no_nextloc

randobjmasx:
    mov ebp,[levelpart]
    shl ebp,5
    add ebp,levels

    mov  ecx,[ebp+8]
    mov  [shiptype],2
    mov  [xmoving],9
    mov  [xaccel],4
    mov  [ymoving],3
    mov  [yaccel],2
    call add_ships

    mov  ecx,[ebp+12]
    mov  [shiptype],1
    mov  [xmoving],3
    mov  [xaccel],1
    mov  [ymoving],3
    mov  [yaccel],6
    call add_ships

    mov  ecx,[ebp+16]
    mov  [shiptype],3
    mov  [xmoving],5
    mov  [xaccel],2
    mov  [ymoving],5
    mov  [yaccel],2
    call add_ships

    mov  ecx,[ebp+20]
    mov  [shiptype],4
    mov  [xmoving],4
    mov  [xaccel],1
    mov  [ymoving],4
    mov  [yaccel],1
    call add_ships

    jmp newlocend

shiptype dd 0
xmoving  dd 0
ymoving  dd 0
xaccel   dd 0
yaccel   dd 0

add_ships:
looship:
    cmp ecx,0
    je  no_ships
    push ecx
    findmas objmas,findzero
;in edi off to free element
    jc  close_app
    mov ebp,[shiptype]
    mov dword [edi+8],ebp ;2 ;green ship
; random x
    push edi
    random 600,eax
    pop edi
    mov [edi],eax
; random y
    push edi
    mov ebp,[levelpart]
    shl ebp,5
    add ebp,levels
    mov esi,[ebp+4] ;get max range
    random esi,eax
    neg eax
    pop edi
    mov [edi+4],eax
; x moving
    push edi         ;planers
    random [xmoving],eax
    sub eax,[xaccel];4
    pop edi
    mov [edi+12],eax
; y moving
    push edi
    random [ymoving],eax     ;slow
    add eax,[yaccel] ;2
    pop edi
    mov [edi+16],eax
    pop ecx
    dec ecx
    jnz looship
no_ships:
    ret


levelpart dd 0
levels:
;level1
    dd 1,800,0,0,5,8,0,0     ;one at start
    dd 500,2000,4,20,30,0,0,0
    dd 500,2000,4,20,0,8,0,0
    dd 500,2000,10,0,0,4,0,0
    dd 500,4000,0,30,0,0,0,0
    dd 400,400,0,0,10,0,0,0
    dd 400,400,0,0,10,0,0,0
    dd 0,0,0,0,0,0,0,0        ;end of level
;level2
    dd 1,16000,0,30,0,0,0,0     ;one at start
    dd 200,8000,0,20,0,0,0,0
    dd 200,2000,0,10,0,8,0,0
    dd 200,4000,0,10,0,0,0,0
    dd 0,0,0,0,0,0,0,0        ;end of level
;level3
    dd 1,4000,0,20,30,8,0,0     ;one at start
    dd 400,4000,10,10,20,6,0,0
    dd 400,4000,0,20,10,2,0,0
    dd 400,4000,10,10,20,0,0,0
    dd 0,-1,0,0,0,0,0,0        ;end of game


newlocend:
    mov [pathtime],0
    inc [levelpart]
    jmp no_nextloc

endgame dd 0
objects dd 0
endtest:
    cmp dword [edi+8],0
    jne no_free
    ret
no_free:
    mov [endgame],0
    inc [objects]
    ret


no_nextloc:
    mov [objects],0
    mov [endgame],1
    readmas objmas,endtest
    cmp [endgame],1
    jne no_end_lev ;no_nextloc

    mov ebp,[levelpart]
    shl ebp,5
    add ebp,levels
    mov eax,[ebp+4]
    cmp eax,dword 0
    je  end_lev    ;end of level
    cmp eax,dword -1
    je  end_gm    ;end of game

no_end_lev:

    cmp [num5],dword 0
    je  no_addsh
    cmp [ctime],dword 0
    jne no_addsh
    cmp [lazer],dword 0
    je  no_addsh
    findmas massive,findzero
;in edi off to free element
    jc  close_app
    mov eax,[shipx]
    mov [edi],eax
    mov eax,[shipy]
    mov [edi+4],eax
    mov [edi+8],dword 1   ;show
    mov [edi+12],dword 0
    mov [edi+16],dword -12
    mov [ctime],dword 8   ;wait for cannon
    dec [lazer]
no_addsh:

    cmp [num8],dword 0
    je  no_addplx
    cmp [xtime],dword 256-16
    jae no_addplx
    cmp [plazma],0
    je  no_addplx
    findmas massive,findzero
;in edi off to free element
    jc  close_app
    mov eax,[shipx]
    mov [edi],eax
    mov eax,[shipy]
    mov [edi+4],eax
    mov [edi+8],dword 2   ;show
    add [xtime],dword 8  ;wait for cannon
    cmp [xtime],dword 256
    jna okx
    mov [xtime],256
okx:
    mov [edi+12],dword 0  ;wait for cannon
    mov [edi+16],dword -8  ;wait for cannon
    dec [plazma]
no_addplx:



    cmp [num2],dword 0
    je  no_addsh2
    cmp [xtime],dword 0
    jne no_addsh2
    cmp [plazma],0
    je  no_addsh2
    mov eax,[shipy]
    mov [temp3],eax
    mov [temp2],dword 5
loox2:
    mov [temp],dword 10
loox:
    findmas massive,findzero
;in edi off to free element
    jc  close_app
    random 25,eax
    mov ebp,eax
    sub eax,12
    add eax,[shipx]
    mov [edi],eax
    shr ebp,3
    random ebp,eax
    neg eax
    add eax,[temp3] ;[shipy]
    mov [edi+4],eax
    mov [edi+8],dword 2   ;show hstar
    random 5,eax
    sub eax,2
    mov [edi+12],eax   ;show hstar
    random 7,eax
    sub eax,8
    mov [edi+16],eax   ;show hstar
    dec [temp]
    jnz loox
    sub [temp3],30
    dec [temp2]
    jnz loox2
    mov [xtime],dword 256  ;wait for cannon
    sub [plazma],50
no_addsh2:


    cmp [num1],dword 0
    je  no_left
    sub dword [shipx],6
no_left:

    cmp [num3],dword 0
    je  no_right
    add dword [shipx],6
no_right:

;ship correct
    cmp [shipx],5
    jnl xl_ok
    mov [shipx],5
xl_ok:
    cmp [shipx],603
    jng xr_ok
    mov [shipx],603
xr_ok:


;clear scrbuf
    mov edi,canvas+8
    cld
    mov ecx,640*440*3/4
    mov eax,0
    rep stosd

    compmas objmas,massive,compobr

    readmas objmas,shipobjtest
    readmas massive,shipguntest

    readmas objmas,showobjmas
    readmas objmas,moveobjmas

    readmas massive,showmas
    readmas massive,movemas
    readmas massive,delfarshoot
;    findmas massive,findzero
;in edi off to free element
;    jc  close_app

    aimgtoimg ship,[shipx],[shipy],canvas,0x0

    drawfbox 150,5,64,5,0x000000
    mov eax,[xtime]
    sub eax,256
    neg eax
    shr eax,2
    drawfbox 150,5,eax,5,0x0000ff

    drawfbox 150,12,64,5,0x000000
    mov eax,[ctime]
    sub eax,8
    neg eax
    shl eax,3
    drawfbox 150,12,eax,5,0xffff00

    drawfbox 220,2,6*5+2 ,9,cl_Grey
    outcount [plazma],221,3,cl_Blue,5*65536
    drawfbox 220,11,6*5+2 ,9,cl_Grey
    outcount [lazer],221,12,0xffff00,5*65536


    drawfbox 280,6,6*5+2 ,9,cl_Grey
    outcount [gship],281,7,cl_Green,5*65536
    drawfbox 320,6,6*5+2 ,9,cl_Grey
    outcount [bship],321,7,cl_Blue,5*65536
    drawfbox 360,6,6*5+2 ,9,cl_Grey
    outcount [boxget],361,7,0xffaa00,5*65536

    drawfbox 400,2,6*5+2 ,9,cl_Grey
    outcount [objects],401,2,0x00ff00,5*65536
    drawfbox 400,11,6*5+2 ,9,cl_Grey
    outcount [energy],401,12,0xff0000,5*65536

    drawfbox 450,11,6*5+2 ,9,cl_Grey
    outcount [score],451,12,0xffff00,5*65536


    jmp  still

  red:
    call draw_window
    jmp still

  key:                          ; key
    mov  eax,2
    int  0x40                   ; read (eax=2)

    shr eax,8
    and eax,0xff
    mov ah,al

    cmp ah,153
    jne no_pause
    not [pause_on]
    jmp still
no_pause:


    cmp ah,76
    jne no_num5d
    mov [num5],1
    jmp still
no_num5d:
    cmp ah,204
    jne no_num5u
    mov [num5],0
    jmp still
no_num5u:

    cmp ah,80
    jne no_num2d
    mov [num2],1
    jmp still
no_num2d:
    cmp ah,208
    jne no_num2u
    mov [num2],0
    jmp still
no_num2u:

    cmp ah,79 ;key_Space 85 exit
    jne no_num1d
    mov [num1],1
    jmp still
no_num1d:
    cmp ah,207 ;key_Space 85 exit
    jne no_num1u
    mov [num1],0
    jmp still
no_num1u:

    cmp ah,81 ;key_Space 85 exit
    jne no_num3d
    mov [num3],1
    jmp still
no_num3d:
    cmp ah,209 ;key_Space 85 exit
    jne no_num3u
    mov [num3],0
    jmp still
no_num3u:

    cmp ah,72  ;key_Space 85 exit
    jne no_num8d
    mov [num8],1
    jmp still
no_num8d:
    cmp ah,200 ;key_Space 85 exit
    jne no_num8u
    mov [num8],0
    jmp still
no_num8u:

    jmp  still                  ; not testing

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    jne  noclose
close_app:
    mov  eax,-1                 ; close this program
    int  0x40
  noclose:
    jmp  still

draw_window:
    startwd
    window 40,40,(640+9),(440+26),window_Skinned+0x00
    label  8,8,'ASCL DYNAMIC GAME',cl_White+font_Big
;    setimg 5,22,img_area
    endwd
    ret

;**********************
;  Game Over process
;**********************

game_over:
;    close
    call draw_gowindow
stillgo:
    wtevent redgo,keygo,buttongo
    jmp stillgo
redgo:
    call draw_gowindow
    jmp stillgo
keygo:
    mov  eax,2
    int  0x40
    jmp  stillgo
buttongo:
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1
    je   close_app
    cmp  ah,4
    je   close_app
    cmp  ah,5
    je   main_menu
    cmp  ah,7
    je   restart_lev
    jmp  stillgo

restart_lev:
    mov [pathtime],0

prevpart:
    mov ebp,[levelpart]
    shl ebp,5
    add ebp,levels
    mov eax,[ebp]
    cmp eax,dword 1
    je  rest    ;end of level
    dec [levelpart]
    jmp prevpart


rest:
;massives reset - clear massives
    mov edi,massive+8
    cld
    mov ecx,massize*elemsize/4
    mov eax,0
    rep stosd

    mov edi,objmas+8
    cld
    mov ecx,omassize*oelemsize/4
    mov eax,0
    rep stosd

;counters reset
    mov [boxget],0
    mov [gship],0
    mov [bship],0

;ship reset
    mov [energy],100
    mov [shipx],300
    mov [shipy],400
    mov [lazer],1000
    mov [plazma],500


; reset keyboard
    mov [num1],0
    mov [num5],0
    mov [num3],0
    mov [num2],0
    mov [num8],0
    jmp start_game

draw_gowindow:
    startwd
    drawfbox 170,160,300,120,cl_Grey
    drawlbut 180,260,80,14,'(X) EXIT',4,0x990000,cl_Black
    drawlbut 280,260,80,14,'<MENU>',5,0x990000,cl_Black
    drawlbut 380,260,80,14,'RESTART',7,0x990000,cl_Black

    label 280,200,'  GAME OVER  ',cl_Black
    endwd
    ret


;**********************
;  End level process
;**********************

end_lev:
;    close
    call draw_scorewindow
stilleg:
    wtevent redeg,keyeg,buttoneg
    jmp stilleg
redeg:
    call draw_scorewindow
    jmp stilleg
keyeg:
    mov  eax,2
    int  0x40
    jmp  stilleg
buttoneg:
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1
    je   close_app
    cmp  ah,2
    je   next_lev
    cmp  ah,3
    je   help
    cmp  ah,4
    je   close_app
    cmp  ah,5
    je   main_menu
    jmp  stilleg

next_lev:
    mov [pathtime],0
    inc [levelpart]

;ship reset
    mov [energy],100
    mov [shipx],300
    mov [shipy],400
    mov [lazer],1000
    mov [plazma],500

;counters reset
    mov [boxget],0
    mov [gship],0
    mov [bship],0

; reset keyboard
    mov [num1],0
    mov [num5],0
    mov [num3],0
    mov [num2],0
    mov [num8],0
    jmp start_game

draw_scorewindow:
    call draw_window
    startwd
    drawfbox 170,130,300,190,cl_Grey
    drawlbut 180,300,80,14,'(X) EXIT',4,0x990000,cl_Black
    drawlbut 280,300,80,14,'<MENU>',5,0x990000,cl_Black
    drawlbut 380,300,80,14,'NEXT >>',2,0x990000,cl_Black

    setimg 180,140,warship1
    outcount [bship],241,151,cl_Blue,5*65536

    setimg 180,180,warship2
    outcount [gship],241,191,cl_Green,5*65536

    setimg 180,220,meteor
    setimg 180,260,box
    outcount [boxget],241,271,0xbbbb00,5*65536
    endwd
    ret


;**********************
;  End of game process
;**********************

end_gm:
    mov [pathtime],0
    call draw_window
    label 200 ,8,'YOU WIN PRESS ANY KEY TO EXIT',cl_White+font_Big

stilleg2:
;    scevent   redeg2,keyeg2,buttoneg2
    timeevent 1,no_event2,redeg2,keyeg2,buttoneg2

no_event2:
    setimg 5,21,canvas

;clear scrbuf
    mov edi,canvas+8
    cld
    mov ecx,640*440*3/4
    mov eax,0
    rep stosd

    aimgtoimg ship,320,220,canvas,0x0

    readmas massive,endshowmas
    readmas massive,endmovemas
    readmas massive,delfarshoot
;    findmas massive,findzero
;in edi off to free element
;    jc  close_app

    inc [pathtime]
    cmp [pathtime],30
    jne no_firework
    mov [pathtime],0
    random 400,eax
    mov [temp3],eax
    random 600,eax
    mov [temp2],eax
    mov [temp],dword 8;10
xloox:
    findmas massive,findzero
;in edi off to free element
    jc  close_app

    mov eax,[temp2] ;[shipx]
    mov [edi],eax
    mov eax,[temp3] ;[shipy]
    mov [edi+4],eax
    mov [edi+8],dword 2   ;show hstar
rerand:
    random 5,eax
    sub eax,2
    cmp eax,0
    je  rerand
    mov [edi+12],eax   ;show hstar
rerand2:
    random 7,eax
    sub eax,3
    cmp eax,0
    je  rerand2
    mov [edi+16],eax   ;show hstar
    dec [temp]
    jnz xloox
no_firework:
    jmp stilleg2

redeg2:
    jmp end_gm ;stilleg2
keyeg2:
    mov  eax,2
    int  0x40
    jmp  main_menu
buttoneg2:
    mov  eax,17                 ; get id
    int  0x40
    jmp  stilleg2






; DATA AREA
IM_END:
;global
pause_on dd 0

;massive
shipx dd 300
shipy dd 400

;guns
lazer  dd 1000
plazma dd 500

;keys
num1 dd 0
num5 dd 0
num3 dd 0
num2 dd 0
num8 dd 0

;enemy countres
gship  dd 0
bship  dd 0
boxget dd 0

energy dd 100
score  dd 0

;delay for cannon
ctime dd 0
;delay for nuke gun
xtime dd 0
;path time
pathtime dd 0
;
temp  dd 0
temp2 dd 0
temp3 dd 0
temp4 dd 0
otv dd 0

;for collimg
imgsize:
 dd 32
 dd 32

;massive:
;dd 400  ;elements num
;dd 20  ;size of element in bytes
;rb 400*20


keymap:
rb 1000

;gif_file_area ~21500
gif_file_area2:
file 'phenix.gif'
rb 50
gif_file_area:
file 'star2.gif';include gif file
rb 50          ;50 bytes temp back zone
img_area:
rb 256*64*3+8
ship:
rb 32*32*3+8
shoot:
rb 32*32*3+8
warship1:
rb 32*32*3+8
warship2:
rb 32*32*3+8
meteor:
rb 32*32*3+8
star:
rb 32*32*3+8
star2:
rb 32*32*3+8
star3:
rb 32*32*3+8
box:
rb 32*32*3+8

gif_hash_area:
rd 4096+1      ;hash area size for unpacking GIF

massive:
rd massize  ;elements num
rd elemsize  ;size of element in bytes
rb massize*elemsize

objmas:
rd omassize  ;elements num
rd oelemsize  ;size of element in bytes
rb omassize*oelemsize

canvas:
canvas_x:
 rd 1
canvas_y:
 rd 1
rb (640*440*3)+2000
I_END:
