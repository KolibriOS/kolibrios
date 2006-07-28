;
;    3D ТЕКСТУРИРОВАНЫЙ ДВИЖОК  3D TEXTURED ENGINE
;    АВТОР: Павлюшин Евгений    AUTOR: Pavlushin Evgeni
;
;    Компилируется с помощью FASM ассемблера для MenuetOS
;    Compile with FASM assembler for MenuetOS
;
; 20.11.04   Fast point calc & triangle draw
;            Little matrix no (trangle massive)
;            Fast triangle, del triangle out of screen
; 16.12.04   Fast triangle with MMX
; 20.12.04   Out triangle fatal bug's deleted, "black zones" deleted
;            Matrix move support
; 24.12.04   Fast keyboard scanning
;            Z-ground level map work
;            Texture draw correction deleted "black zones"
; 04.01.05   Moveing texture
; 05.01.05   Water dynamic texture
; 06.01.05   Texture pack (many textures in one file)
; 07.01.05   Z-sorting
; 08.01.05   Triangle good clipping calculating (speed up)
;            4 byte Z-sorting, more dynamic of water
; 09.01.05   Texture map from 256 color bmp file
;            Pixel Z-buffer, good speed!
; 21.01.05   Models buffer add.
; 25.01.05   Models buffer update, Add 2 new models.
; 29.01.05   Dynamic model array
; 01.02.05   1 picture MipMap calculation
; 04.02.05   All picture MipMap calculation, proc speed up.
;            Simple Triangel MipMap chose.
; 05.02.05   Normal Triangel MipMap chose.
; 12.02.05   Best QUALITY of Textured TRIANGEL!
; 13.02.05   add new models. All camera axis calculating
; 16.02.05   Simple model turning
; 17.02.05   Advanced model turning, model tmpoints massive deleted
;            New map size 128, add new models.
; 25.02.05   Left side clipping bug deleted
; 26.02.05   Optimization of textured line algorythm
; 24.04.05   Test pixel z-buffer it's work. Use new textri algorythm
; 30.04.05   Fast Y-clipping

use32

SCREEN_X equ 800 ;320  ;Screen size easy change
SCREEN_Y equ 600 ;200
DSCREEN_X equ SCREEN_X-1 ;320  ;For triangle clipping
DSCREEN_Y equ SCREEN_Y-1 ;200
MATRIX_XSIZE equ 64;32;64;100  ;Ground matrix size
MATRIX_YSIZE equ 64;32;64;100
SECTOR_SIZE  equ 100           ;Size of matrix sector
MAP_XSIZE equ 128               ;Rezolution of map file
MAP_YSIZE equ 128

    org    0x0
    db     'MENUET01'              ; 8 байт интдефикатор
    dd     0x01                    ; версия заголовка
    dd     START                   ; адресс начала кода
    dd     I_END                   ; размер программы
    dd     I_END+(SCREEN_X*SCREEN_Y*3+50000)   ; размер памяти для прогграммы
    dd     I_END+(SCREEN_X*SCREEN_Y*3+10000)   ; esp
    dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include 'ascgl.inc'
include 'ascl.inc'

START:                          ; начало приложения
; Draw window at first
     call draw_window

     ; Decoding groud texture and ground level map
     gif_hash_offset = gif_hash_area
     ;texture's
     giftoimg texpack_gif,texpack_img-8
     ;ground level map
     giftoimg gif_file_area5,glm_img_area

;get 8bitmap from 256 colors bmp file
     mov ebp,128;64
     mov esi,0x043a+128*128-4;64*64-4
     sub esi,128;64
     mov edi,0
     mov ecx,128;64
texmap_unp:
     mov al,[bmp_texmap+esi]
     mov [img_area+edi],al
     inc esi
     inc edi
     dec ecx
     jnz texmap_unp
     mov ecx,128;64
     sub esi,256;128
     dec ebp
     jnz texmap_unp


;Unpack textures

     mov esi,texpack_img
     mov edi,texture_img
     mov edx,16
     mov ecx,16
tpuckloop:
     pushad
     call unpuck
     popad
     add esi,32*3
     add edi,4095
     dec ecx
     jnz tpuckloop
     add esi,31*32*16*3
     mov ecx,16
     dec edx
     jnz tpuckloop

;
; Unpuck one texture procendure
;
     jmp skip_unpuck
unpuck:
     mov ecx,32
     mov edx,32
tunploop:
     mov ax,[esi]
     mov [edi],ax
     mov al,[esi+2]
     mov [edi+2],al
     add esi,3
     add edi,3
     dec ecx
     jnz tunploop
     mov ecx,32
     add esi,(32*16*3)-(32*3)
     dec edx
     jnz tunploop
     ret
skip_unpuck:


;calculating all mipmaps
    mov esi,texture_img
    mov ecx,256   ;quantity of textures
mmaploop:
    mov ebp,32    ;max mipmap texture
    pusha
    call texgen
    popa
    add esi,4095  ;next mipmap block
    dec ecx
    jnz mmaploop

    jmp skip_texgen

;********************************
;   MIPMAP TEXTURE GENERATION
;
; in put:
;   esi - offset to texture
;   ebp - mipmap max size (32 for this sample)
;********************************

texgen:
    push esi
    mov eax,ebp
    imul eax,ebp
    imul eax,3
    add eax,esi
    mov edi,eax

    mov eax,ebp
    shr eax,1
    mov dh,al
    mov dl,al
    mov cl,dl

    mov ecx,ebp
    mov eax,ebp
    shl eax,1
    add ecx,eax

miploop:
; Red
    xor bx,bx  ;for add
    xor ax,ax  ;for add
    mov al,[esi+0]
    add bx,ax
    mov al,[esi+3+0]
    add bx,ax
    mov al,[esi+ecx+0]
    add bx,ax
    mov al,[esi+ecx+3+0]
    add bx,ax
    shr bx,2   ;/4
    mov [edi+0],bl
;Green
    xor bx,bx  ;for add
    xor ax,ax  ;for add
    mov al,[esi+1]
    add bx,ax
    mov al,[esi+3+1]
    add bx,ax
    mov al,[esi+ecx+1]
    add bx,ax
    mov al,[esi+ecx+3+1]
    add bx,ax
    shr bx,2   ;/4
    mov [edi+1],bl
;Blue
    xor bx,bx  ;for add
    xor ax,ax  ;for add
    mov al,[esi+2]
    add bx,ax
    mov al,[esi+3+2]
    add bx,ax
    mov al,[esi+ecx+2]
    add bx,ax
    mov al,[esi+ecx+3+2]
    add bx,ax
    shr bx,2   ;/4
    mov [edi+2],bl

    add esi,6
    add edi,3
    dec dl
    jnz miploop
    mov ax,bp
    shr ax,1
    mov dl,al
    add esi,ecx
    dec dh
    jnz miploop
    pop esi

    mov eax,ebp
    imul eax,ebp
    imul eax,3
    add esi,eax
    shr ebp,1
    cmp ebp,1
    jne texgen
    ret
skip_texgen:


;Copy dynamic water texture
; ????????????
    mov ecx,32*32*3
    mov esi,texture_img+4095 ;32*32*3
    mov edi,texture_limg
    cld
    rep movsb

	; init sine wave for dynamic water texture
    finit
    mov	edi,sinwave
    mov	ecx,32;256
    isv_loop:
    fld	[angle]
    fld	st
    fsin
    fmul	[mul_wave]
    fistp	word [edi]
    fadd	[d_angle]
    fstp	[angle]
    add	edi,2
    dec	ecx
    jnz	isv_loop


;Initalize keyboard
    mov  eax,66
    mov  ebx,1
    mov  ecx,1
    int  0x40

    mov  eax,26
    mov  ebx,2
    mov  ecx,1
    mov  edx,keymap+100
    int  0x40

;Build triangle matrix
    mov  esi,points
    mov  eax,-(MATRIX_XSIZE/2)*SECTOR_SIZE
    mov  ebx,-(MATRIX_YSIZE/2)*SECTOR_SIZE
    mov  ebp,img_area+8
loomat:
    mov  [esi],eax        ;x-set
    mov  [esi+4],ebx      ;y-set
    mov  [esi+8],ecx      ;z-set
    add  ebp,3
    add  esi,4*3
    add  eax,SECTOR_SIZE
    cmp  eax,((MATRIX_YSIZE/2)+1)*SECTOR_SIZE
    jnge loomat
    mov  eax,-(MATRIX_YSIZE/2)*SECTOR_SIZE
    add  ebx,SECTOR_SIZE
    cmp  ebx,((MATRIX_XSIZE/2)+1)*SECTOR_SIZE
    jnge loomat

; Create z-ground good algorythm not already yet (64x64 map)
    mov esi,glm_img_area+8
    mov edi,ground_level_map
loox:
; draw_courner_points
    mov eax,[esi]
    call get_z
;    mov [edi],eax

    mov eax,[esi+((MATRIX_XSIZE-1)*4)]
    call get_z
;    mov [edi+((MATRIX_XSIZE)*4)],eax

    mov eax,[esi+(((MATRIX_XSIZE)*(MATRIX_YSIZE-1)+1)*4)]
    call get_z
;    mov [edi+(((MATRIX_XSIZE+1)*(MATRIX_YSIZE)-0)*4)],eax

    mov eax,[esi+(((MATRIX_XSIZE)*(MATRIX_YSIZE)-1)*4)]
    call get_z
;    mov [edi+(((MATRIX_XSIZE+1)*(MATRIX_YSIZE+1)-1)*4)],eax


    jmp skip_gz
get_z:
    xor ebx,ebx
    xor ecx,ecx
    mov bl,al
    add ecx,ebx
    mov bl,ah
    add ecx,ebx
    shr eax,16
    mov bl,al
    add ecx,ebx
    mov eax,ecx
    xor edx,edx
    mov ebx,3
    cdq
    div ebx
    neg eax
    ret
skip_gz:

; z-ground livel facking method (65x65 map)
    mov esi,glm_img_area+8
    mov edi,ground_level_map
    mov ebp,(MAP_XSIZE+1)*(MAP_YSIZE+1)
looglm:
    mov eax,[esi]
;    and eax,0x000000ff
    call get_z
    mov [edi],eax
    add esi,3
    add edi,1
    dec ebp
    jnz looglm

;Fill model massive
    mov ecx,[model_mas_start]
    imul ecx,8
    add ecx,4
    mov esi,model_mas_start
    mov edi,model_mas
    cld
    rep movsd

    jmp skip_moddata
model_mas_start:
dd 12               ;quantity of models
dd 0,0,-150,0,0,0,0,bunker
dd 60,-250,-190,0,0,64,0,tank
dd 0,180,-150,0,0,0,0,cannon
dd 0,480,-150,0,0,0,0,outpost
dd 260,60,-150,0,0,0,0,bunker
dd 60,260,-150,0,0,0,0,outpost
dd 210,410,-150,0,0,0,0,cannon
dd 160,260,-150,0,0,0,0,tree
dd 100,-360,-150,0,0,192,0,gqfa
dd 10,580,-150,0,0,0,0,repear
dd 460,160,-100,0,0,0,0,red_flag
dd 60,360,-170,0,0,40,0,cannon
skip_moddata:


;Main loop
still:                          ; основной цикл
    mov  eax,11                 ; проверка состояния окна
    int  0x40

    cmp  eax,1                  ; окно сдвинули его нужно перерисовать
    je   red
    cmp  eax,2                  ; нажата клавиша на клавиатуре
    je   key
    cmp  eax,3                  ; нажата кнопка в окне
    je   button

;    delay 10
;    cmp  [autorot],0  ;frize no fps show when autorot off
;    je   still

cycle:
    call clrscr                 ; clear screen buffer
    call clrzbuf                ; clear z-buffer
    call render_scene           ; calculating scene
    call dispimg                ; show buffer

    fps_show_frequency=0
    fps 10,10,cl_White,cl_Black


;turn model on z-axis
    inc dword [model_mas+4*6]
    and dword [model_mas+4*6],011111111b


; Sin wave dynamic texture for water
; jmp ndt
 xor edi,edi
	mov	dx,32
	mov	bp,word [sin_pos]
dp_ver:
	mov	cx,32 ;320
	mov	si,word [sin_pos]
dp_hor:
 and ebp,0000ffffh
	mov	ax,word [sinwave+ebp]
	add	ax,cx
	and	ax,31

 and esi,0000ffffh
	mov	bx,word [sinwave+esi]
	add	bx,dx
	and	bx,31
	shl	bx,5
	add	bx,ax

 push bx
 imul bx,3
 and ebx,0000ffffh
	mov	ax,[texture_limg+ebx]
 mov [texture_img2+edi],ax
	mov	al,[texture_limg+ebx+2]
 mov [texture_img2+edi+2],al
 pop bx

 add edi,3
	add	si,2
	and	si,63;511
	
	dec	cx
	jnz	dp_hor
	add	bp,2
	and	bp,63;511
	dec	dx
	jnz	dp_ver

; update sine position for next frame
	add	word [sin_pos],2
	and	word [sin_pos],63;511

;Move water texture
    jmp ndt
    mov  esi,texture_limg
    mov  ecx,32*32-1
loodt:
    mov  al,byte [esi]
    mov  bl,byte [esi+1]
    mov  dl,byte [esi+2]
    mov  ah,byte [esi+3]
    mov  bh,byte [esi+4]
    mov  dh,byte [esi+5]
    mov  byte [esi],ah
    mov  byte [esi+1],bh
    mov  byte [esi+2],dh
    mov  byte [esi+3],al
    mov  byte [esi+4],bl
    mov  byte [esi+5],dl
    add  esi,3
    dec  ecx
    jnz  loodt
ndt:

;Creat mipmap pack for dynamic texture
    mov ebp,32
    mov esi,texture_img+4095
    call texgen


    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x0000ff00            ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,keymap                ; pointer to text beginning
    mov  esi,100       ; text length
    int  0x40
    add  edx,100
    add  ebx,10
    mov  esi,60       ; text length
    mov  ecx,0x00dddddd            ; font 1 & color ( 0xF0RRGGBB )
    int  0x40
    mov  edx,usemap
    mov  esi,60       ; text length
    mov  ecx,0x0000ff00
    int  0x40
    jmp rx
          ;01234567890123456789012345678901234567890123456789
usemap db ' E               wer  u   []  asd           zxc   '
       db '                                                  '
rx:

    cmp byte [keymap+1],0
    je  n_esc
    jmp exit
n_esc:

    cmp byte [keymap+22],0
    je  n_u
    mov [Xangle],0
    mov [Yangle],0
    mov [Zangle],0
n_u:

; t,y - mipmap cntrol
    cmp byte [keymap+20],0
    je  n_t
    inc [mipzoom]
n_t:
    cmp byte [keymap+21],0
    je  n_y
    dec [mipzoom]
n_y:
    cmp byte [keymap+23],0
    je  n_i
    mov byte [keymap+23],0 ; reset key
    cmp [mipmapwork],1
    je  i_1
i_0:
    mov [mipmapwork],1
    jmp n_i
i_1:
    mov [mipmapwork],0
n_i:


    cmp byte [keymap+26],0
    je  n_lsk
    add [Xcam],1
n_lsk:
    cmp byte [keymap+27],0
    je  n_rsk
    sub [Xcam],1
n_rsk:
    cmp byte [keymap+18],0
    je  n_e
    add [Yangle],1
n_e:
    cmp byte [keymap+45],0
    je  n_x
    sub [Yangle],1
n_x:
    cmp byte [keymap+31],0
    je  n_s
    add [Xangle],1
n_s:
    cmp byte [keymap+32],0
    je  n_d
    sub [Xangle],1
n_d:
    cmp byte [keymap+44],0
    je  n_z
    add [Zangle],1
n_z:
    cmp byte [keymap+46],0
    je  n_c
    sub [Zangle],1
n_c:
    cmp byte [keymap+17],0
    je  n_w
    add [Zcam],25 ;250
n_w:
    cmp byte [keymap+19],0
    je  n_r
    sub [Zcam],25 ;250
n_r:

    cmp byte [keymap+75],0
    je  n_lk
    add [Zcamangle],1
    and [Zcamangle],011111111b
n_lk:
    cmp byte [keymap+77],0
    je  n_rk
    sub [Zcamangle],1
    and [Zcamangle],011111111b
n_rk:

    cmp byte [keymap+79],0
    je  n_num1
    add [Xcamangle],1
    and [Xcamangle],011111111b
n_num1:
    cmp byte [keymap+81],0
    je  n_num3
    sub [Xcamangle],1
    and [Xcamangle],011111111b
n_num3:
    cmp byte [keymap+71],0
    je  n_num7
    add [Ycamangle],1
    and [Ycamangle],011111111b
n_num7:
    cmp byte [keymap+73],0
    je  n_num9
    sub [Ycamangle],1
    and [Ycamangle],011111111b
n_num9:



    cmp byte [keymap+30],0
    je  n_a
    mov byte [keymap+30],0 ; reset key
    cmp [autorot],1
    je  a_1
a_0:
    mov [autorot],1
    jmp n_a
a_1:
    mov [autorot],0
n_a:

; for camera
;    mov     ebx,[Xcamangle]
;    call    GetSinCos
;    mov     [Xcamsin],eax
;    mov     [Xcamcos],ebx

;    mov     ebx,[Ycamangle]
;    call    GetSinCos
;    mov     [Ycamsin],eax
;    mov     [Ycamcos],ebx

    mov     ebx,[Zcamangle]
    call    GetSinCos
    mov     [Zcamsin],eax
    mov     [Zcamcos],ebx

    mov eax,[Zcamsin]
    mov ebx,[Zcamcos]
;    mov ecx,[Xcamsin]
;    mov edx,[Xcamcos]
;    mov esi,[Ycamsin]
;    mov edi,[Ycamcos]

    sar eax,4
    sar ebx,4
;    sar ecx,4
;    sar edx,4
;    sar esi,4
;    sar edi,4

    cmp byte [keymap+72],0
    je  n_uk
    sub [Xcam],eax
    sub [Ycam],ebx

;    sub [Zcam],ecx
;    sub [Ycam],edx

;    sub [Xcam],esi
;    add [Zcam],edi

n_uk:
    cmp byte [keymap+80],0
    je  n_dk
    add [Xcam],eax
    add [Ycam],ebx

;    add [Zcam],ecx
;    add [Ycam],edx

;    add [Xcam],esi
;    sub [Zcam],edi

n_dk:

    xor ebp,ebp
move_test:
    cmp [Xcam],-SECTOR_SIZE/2
    jnl ok1
    add [Xcam],SECTOR_SIZE
    dec [Xmap]
    jmp move_test
ok1:
    cmp [Xcam],SECTOR_SIZE/2
    jng ok2
    sub [Xcam],SECTOR_SIZE
    inc [Xmap]
    jmp ok1
ok2:
    cmp [Ycam],-SECTOR_SIZE/2
    jnl ok3
    add [Ycam],SECTOR_SIZE
    dec [Ymap]
    jmp ok2
ok3:
    cmp [Ycam],SECTOR_SIZE/2
    jng ok4
    sub [Ycam],SECTOR_SIZE
    inc [Ymap]
    jmp ok3
ok4:

    and [Xangle],011111111b
    and [Yangle],011111111b
    and [Zangle],011111111b

    jmp still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40

    shr eax,8
    and eax,0xff
    mov ah,al

    mov ebx,0
    mov bl,ah
    cmp bl,224
    je  noisa  ; ignore Ext code
    cmp bl,170
    je  noisa  ; ignore Key Up code
    cmp bl,128
    ja  isa
    mov [keymap+ebx],byte 'X'  ; set press marker to key id
    jmp noisa
isa:
    sub bl,128
    mov [keymap+ebx],byte 0    ; reset key marker
noisa:
    jmp n_a

    jmp still ; cycle

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40
    cmp ah,1
    jz exit
    jmp  cycle

exit:
    mov eax,-1
    int 0x40

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,0*65536+SCREEN_X-1         ; [x start] *65536 + [x size]
    mov  ecx,0*65536+SCREEN_Y-1         ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x005080d0            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x005080d0            ; color of frames    RRGGBB
    int  0x40
                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov eax,12
    mov ebx,2
    int 0x40
    ret

dispimg:
    mov eax,7
    mov ebx,I_END ;zbuffer
    mov ecx,SCREEN_X*65536+SCREEN_Y
    xor edx,edx  ;0*65536+0
    int 0x40
;    ret

    mov eax,8
    mov ebx,(SCREEN_X-30)*65536+20
    mov ecx,10*65536+20
    mov edx,1
    mov esi,0x0000aa00
    int 0x40
    ret

clrscr:                         ; clear screen buffer
;    cld
;    mov edi,I_END
;    xor eax,eax
;    mov ecx,SCREEN_X*SCREEN_Y*3/4
;    rep stosd
;    ret

; clear screen buffer with MMX technology +1,5 fps
    mov edi,I_END
    mov ecx,SCREEN_X*SCREEN_Y*3/32
    xor eax,eax
    movd mm0,eax
    movd mm1,eax
    movd mm2,eax
    movd mm3,eax
csloo:
    movq qword [edi],mm0
    movq qword [edi+8],mm1
    movq qword [edi+16],mm2
    movq qword [edi+24],mm3
    add edi,32
    dec ecx
    jnz csloo
    ret

clrzbuf:                         ; clear zbuf
    cld
    mov edi,zbuffer
    mov eax,-1
    mov ecx,SCREEN_X*SCREEN_Y*3/4
    rep stosd
    ret


;    mov [@@atx1],dword 0xffffffff
;    mov [@@aty1],dword 0xffffffff
;    movq mm0,qword [@@atx1]
;    movq mm1,qword [@@atx1]
;    movq mm2,qword [@@atx1]
;    movq mm3,qword [@@atx1]
;czbloo:
;    movq qword [edi],mm0
;    movq qword [edi+8],mm1
;    movq qword [edi+16],mm2
;    movq qword [edi+24],mm3
;    add edi,32
;    dec ecx
;    jnz czbloo
    ret

@@atx1: dd 0
@@aty1: dd 0

;===========================================================================
;
;   3D-system example. Use the following formulas to rotate a point:
;
;        Rotate around x-axis
;        Y = Y * COS(xang) - Z * SIN(xang) / 256
;        Z = Y * SIN(xang) + Z * COS(xang) / 256
;
;        Rotate around y-axis
;        X = X * COS(yang) - Z * SIN(yang) / 256
;        Z = X * SIN(yang) + Z * COS(yang) / 256
;
;        Rotate around z-axis
;        X = X * COS(zang) - Y * SIN(zang) / 256
;        Y = X * SIN(zang) + Y * COS(zang) / 256
;
;   Divide by 256 coz we have multiplyd our sin values with 256 too.
;   This example isn't too fast right now but it'll work just fine.
;
;===========================================================================

;***************************************************************************
;                      \\\   MAIN 3D LOOP   ///
;                         ******************
render_scene:

;********************
; Turn matrix points
;********************

    cmp   [autorot],0
    je    no_autorot
    call  UpdateAngles        ; Calculate new angles
no_autorot:
    call  SetRotation         ; Find sine & cosine of those angles

    mov  edi,tpoints
    mov  esi,points
    mov [mapoff],-1      ;-1 at start
    mov ebp,[Ymap]
    imul ebp,MAP_XSIZE+1
    mov eax,[Xmap]
    add ebp,eax

    mov  ecx,(MATRIX_XSIZE+1)*(MATRIX_YSIZE+1)
ShowLoop:
    push ecx
    push esi
    mov  eax,[esi]
    sub  eax,[Xcam]
    mov  [X],eax
    mov  eax,[esi+4]
    sub  eax,[Ycam]
    mov  [Y],eax
    mov  eax,[ebp+ground_level_map]    ;color
    and  eax,0x000000ff
    sub  eax,[Zcam]
    mov  [Z],eax
    push ebp
    push edi
    call TranslatePoint         ; Rotates the point using above formulas
    pop  edi
    mov  [edi],ebp    ;x
    mov  [edi+4],eax  ;y
    mov  eax,[Z]      ;z
    add  eax,[Zoff]
    mov  [edi+8],eax
    pop  ebp
    pop  esi
    pop  ecx

    add  esi,4*3 ;next in point
    add  edi,4*3 ;next out point

    inc [mapoff]
    cmp [mapoff],MATRIX_XSIZE+1
    jne no_shift_glm
    mov [mapoff],dword 0
    add ebp,(MAP_XSIZE-MATRIX_XSIZE)
no_shift_glm:
    add ebp,1

    dec  ecx
    jnz  ShowLoop


; Show, how many polygons on screen.
    outcount [massize],50,10,cl_White,8*65536


;*****************
;  out triangles
;*****************

    mov [mapoff],-1      ;-1 at start
    mov [massize],0      ;restet triangle massive counter

    mov ebp,[Ymap]
    imul ebp,MAP_XSIZE;64;3*64  ;MAP_XSIZE
    mov eax,[Xmap]
    add ebp,eax

    mov edi,tpoints
    mov ecx,MATRIX_YSIZE ;64
lootpy:
    push ecx
    mov ecx,MATRIX_XSIZE ;64
lootpx:
;draw four angle (two triangles)

Z_MAX = 10    ;maximal z range for triangles

; get texture offset at start
    inc [mapoff]
    cmp [mapoff],MATRIX_XSIZE
    jne no_shift
    mov [mapoff],dword 0
    add ebp,(MAP_XSIZE-MATRIX_XSIZE)
no_shift:
    xor eax,eax
    mov al,[ebp+img_area];+8]
    inc ebp
    imul eax,4095;32*32*3
    add eax,texture_img
    mov ebx,eax

;first triangle
    mov eax,[edi+8]          ;z1
    cmp eax,Z_MAX
    jl  no_add1
    mov [@@tz1],eax
    mov eax,[edi+8+12]   ;z2
    cmp eax,Z_MAX
    jl  no_add1
    mov [@@tz2],eax
    mov eax,[edi+8+(4*3*(MATRIX_XSIZE+2))]   ;z3
    cmp eax,Z_MAX
    jl  no_add1
    mov [@@tz3],eax

    cmp dword [edi],SCREEN_X
    ja  p11
    cmp dword [edi+4],SCREEN_Y
    jna yes_add1
p11:
    cmp dword [edi+12],SCREEN_X
    ja  p12
    cmp dword [edi+4+12],SCREEN_Y
    jna yes_add1
p12:
    cmp dword [edi+(4*3*(MATRIX_XSIZE+2))],SCREEN_X
    ja  p13
    cmp dword [edi+4+(4*3*(MATRIX_XSIZE+2))],SCREEN_Y
    jna yes_add1
p13:
    jmp no_add1
yes_add1:

    movq mm0,qword [edi]        ;x1
    movq mm1,qword [edi+12]     ;x2
    movq mm2,qword [edi+(4*3*(MATRIX_XSIZE+2))]     ;x3
    movq qword [@@tx1],mm0
    movq qword [@@tx2],mm1
    movq qword [@@tx3],mm2

    mov eax,ebx;+1 shl 31;[ebp]
    mov [@@tex_off],eax ;0x0000ff00
    inc [massize]
    pushad
    mov [@@tex_x1],0
    mov [@@tex_y1],0
    mov [@@tex_x3],(32 shl 16) -1;128
    mov [@@tex_y3],(32 shl 16) -1;128
    mov [@@tex_x2],0
    mov [@@tex_y2],(32 shl 16) -1;128
    call textured_triangle
    popad

no_add1:



;second triangle
    mov eax,[edi+8]          ;z1
    cmp eax,Z_MAX
    jl  no_add2
    mov [@@tz1],eax
    mov eax,[edi+8+(4*3*(MATRIX_XSIZE+1))]   ;z2
    cmp eax,Z_MAX
    jl  no_add2
    mov [@@tz2],eax
    mov eax,[edi+8+(4*3*(MATRIX_XSIZE+2))]   ;z3
    cmp eax,Z_MAX
    jl  no_add2
    mov [@@tz3],eax

    cmp dword [edi],SCREEN_X
    ja  p21
    cmp dword [edi+4],SCREEN_Y
    jna yes_add2
p21:
    cmp dword [edi+(4*3*(MATRIX_XSIZE+1))],SCREEN_X
    ja  p22
    cmp dword [edi+4+(4*3*(MATRIX_XSIZE+1))],SCREEN_Y
    jna yes_add2
p22:
    cmp dword [edi+(4*3*(MATRIX_XSIZE+2))],SCREEN_X
    ja  p23
    cmp dword [edi+4+(4*3*(MATRIX_XSIZE+2))],SCREEN_Y
    jna yes_add2
p23:
    jmp no_add2
yes_add2:

    movq mm0,qword [edi]            ;x1
    movq mm1,qword [edi+(4*3*(MATRIX_XSIZE+1))]     ;x2
    movq mm2,qword [edi+(4*3*(MATRIX_XSIZE+2))]     ;x3
    movq qword [@@tx1],mm0
    movq qword [@@tx2],mm1
    movq qword [@@tx3],mm2


    mov eax,ebx
    mov [@@tex_off],eax ;0x0000ff00
    inc [massize]
;    add esi,4*10

    pushad
    mov [@@tex_x1],0
    mov [@@tex_y1],0
    mov [@@tex_x3],(32 shl 16) -1;128
    mov [@@tex_y3],(32 shl 16) -1;128
    mov [@@tex_x2],(32 shl 16) -1
    mov [@@tex_y2],0
    call textured_triangle
    popad


no_add2:

    add edi,4*3     ;next triangle
    dec ecx
    jnz lootpx
    add edi,4*3     ;next string
    pop ecx
    dec ecx
    jnz lootpy

;**********************************
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;**********************************
; For good models quality -5% speed
;    call clrzbuf

    jmp skip_modmas
mmcnt       dd 0 ;counter of model read number
modeloff    dd 0 ;offset of model structure
modelmoff   dd 0 ;offset in model mas
modeltmpoff dd 0 ;tmpoints offset
; massive of models
; format: X,Y,Z coordoinats,offset to model structure

skip_modmas:

; Rendering models on map

    mov  eax,[model_mas]
    mov  [mmcnt],eax

ModelRenderLoop:

    mov  ebx,[mmcnt]
    dec  ebx
    imul ebx,4*8
    add  ebx,model_mas+4
    mov  ebp,ebx   ;edi=model offset X,Y,Z,Xturn,Yturn,Zturn,zero,modeloff

    mov     ebx,[ebp+4*3];[Xangle]         ; Grab angle
    call    GetSinCos           ; Get the sine&cosine
    mov     [Xsin],eax           ; Save sin
    mov     [Xcos],ebx           ; Save cos
    mov     ebx,[ebp+4*4];[Yangle]
    call    GetSinCos
    mov     [Ysin],eax
    mov     [Ycos],ebx
    mov     ebx,[ebp+4*5];[Zangle]
    call    GetSinCos
    mov     [Zsin],eax
    mov     [Zcos],ebx

    mov  ebx,[mmcnt]
    imul ebx,4*8
    add  ebx,model_mas+4-4
    mov  eax,[ebx]

    mov  [modeloff],eax
    sub  ebx,4*7;3
    mov  [modelmoff],ebx

    mov edi,modelpoints
    mov  esi,eax ;bunker+8
    add  esi,8
    mov  ecx,[eax] ;[bunker] ;MODEL_POINTS

TurnModelLoop:
    push ecx
    push esi
    mov  eax,[esi]
    mov  [X],eax
    mov  eax,[esi+4]
    mov  [Y],eax
    mov  eax,[esi+8]
    mov  [Z],eax
    push ebp
    push edi

;RotatePoint

    mov esi,[Y]
    mov ebx,[Xcos]
    mov edi,[Z]
    mov ecx,[Xsin]
    call math
    mov [Y],esi
    mov [Z],edi
    mov esi,[X]
    mov ebx,[Ycos]
    mov ecx,[Ysin]
    call math
    mov [X],esi
    mov [Z],edi
    mov ebx,[Zcos]
    mov edi,[Y]
    mov ecx,[Zsin]
    call math
    mov [X],esi
    mov [Y],edi

    pop  edi

    mov  eax,[X]
    mov  [edi],eax
    mov  eax,[Y]
    mov  [edi+4],eax

    mov  eax,[Z]      ;z
;    add  eax,[Zoff]
    mov  [edi+8],eax
    pop  ebp
    pop  esi
    pop  ecx

    add  esi,4*3 ;next in point
    add  edi,4*3 ;next out point

    dec  ecx
    jnz  TurnModelLoop

modxxx:


;Turn model off map

    call SetRotation

    mov  ebx,[mmcnt]
    imul ebx,4*8
    add  ebx,model_mas+4-4
    mov  eax,[ebx]

    mov  [modeloff],eax
    sub  ebx,4*7;3
    mov  [modelmoff],ebx

    mov edi,modelpoints
    mov  ecx,[eax] ;[bunker] ;MODEL_POINTS

ShowModelLoop:
    push ecx
    mov  eax,[edi] ;esi]

    mov  ebx,[modelmoff]
    sub  eax,[ebx] ;[Xmod]

    sub  eax,[Xcam]
    mov  ebx,[Xmap]
    imul ebx,SECTOR_SIZE
    sub  eax,ebx
    mov  [X],eax
    mov  eax,[edi+4] ;esi+4]

    mov  ebx,[modelmoff]
    sub  eax,[ebx+4] ;[Ymod]

    sub  eax,[Ycam]
    mov  ebx,[Ymap]
    imul ebx,SECTOR_SIZE
    sub  eax,ebx
    mov  [Y],eax
    mov  eax,[edi+8];esi+8]

    mov  ebx,[modelmoff]
    sub  eax,[ebx+8] ;[Zmod]

    sub  eax,[Zcam]
    mov  [Z],eax
    push ebp
    push edi
    call TranslatePoint         ; Rotates the point using above formulas
    pop  edi
    mov  [edi],ebp    ;x
    mov  [edi+4],eax  ;y
    mov  eax,[Z]      ;z
    add  eax,[Zoff]
    mov  [edi+8],eax
    pop  ebp
    pop  ecx

    add  edi,4*3 ;next out point

    dec  ecx
    jnz  ShowModelLoop


;    add [model_mas+8],dword 3



    mov esi,[massize]
    imul esi,4*10

    mov eax,[modeloff]
    mov ecx,[eax+4] ;MODEL_TRIANGELS

;calc tmpoints offset
    mov edi,modelpoints
    mov [modeltmpoff],edi

;calc postlink offset
    mov edi,[eax]
    imul edi,3*4;*2  ;X,Y,Z (3) points dd format (4) 2 masives (2)
    add edi,eax
    add edi,8       ;skip dd points, dd triangels
msloo:

    mov ebx,[edi]
    imul ebx,4*3
    add ebx,[modeltmpoff] ;tmpoints
    mov eax,[ebx]
    mov [@@tx1],eax
    mov eax,[ebx+4]
    mov [@@ty1],eax
    mov eax,[ebx+8]
    mov [@@tz1],eax

    mov ebx,[edi+4]
    imul ebx,4*3
    add ebx,[modeltmpoff] ;tmpoints
    mov eax,[ebx]
    mov [@@tx2],eax
    mov eax,[ebx+4]
    mov [@@ty2],eax
    mov eax,[ebx+8]
    mov [@@tz2],eax

    mov ebx,[edi+8]
    imul ebx,4*3
    add ebx,[modeltmpoff] ;tmpoints
    mov eax,[ebx]
    mov [@@tx3],eax
    mov eax,[ebx+4]
    mov [@@ty3],eax
    mov eax,[ebx+8]
    mov [@@tz3],eax


    cmp dword [@@tz1],Z_MAX
    jl  no_add
    cmp dword [@@tz2],Z_MAX
    jl  no_add
    cmp dword [@@tz3],Z_MAX
    jl  no_add
    cmp dword [@@tx1],SCREEN_X
    ja  pm1
    cmp dword [@@ty1],SCREEN_Y
    jna yes_madd
pm1:
    cmp dword [@@tx2],SCREEN_X
    ja  pm2
    cmp dword [@@ty2],SCREEN_Y
    jna yes_madd
pm2:
    cmp dword [@@tx3],SCREEN_X
    ja  pm3
    cmp dword [@@ty3],SCREEN_Y
    jna yes_madd
pm3:
    jmp no_add
yes_madd:




    mov ebx,[edi+12]
    dec ebx
    js  ttex
    mov eax,1 shl 31
    jmp posit
ttex:
    add ebx,2
    mov eax,0
    neg ebx
posit:
    imul ebx,4095;32*32*3
    add ebx,texture_img
    mov [@@tex_off],ebx

    pushad
    mov [@@tex_x1],0
    mov [@@tex_y1],0
    mov [@@tex_x3],(32 shl 16) -1;128
    mov [@@tex_y3],(32 shl 16) -1;128
    cmp eax,0
    je  nez
    mov [@@tex_x2],0
    mov [@@tex_y2],(32 shl 16) -1
    jmp isz
nez:
    mov [@@tex_x2],(32 shl 16) -1
    mov [@@tex_y2],0
isz:
    call textured_triangle
    popad

;    mov [esi+8+24+4],eax
;    add esi,4*10
    inc [massize]
no_add:
    add edi,4*4; *9
    dec ecx
    jnz msloo

    dec [mmcnt]
    jnz ModelRenderLoop


    jmp skip_mdata

bunker:
dd 8      ;model points
dd 10     ;model triagels
;mpoints:
dd -105,-105,0, -105,105,0, 105,105,0,  105,-105,0
dd -70,-70,-50, -70,70,-50, 70,70,-50,  70,-70,-50

;  4-------7 Points structure
;  |\0---3/|
;  | |   | |
;  | |   | |
;  |/1---2\|
;  5-------6

;pointslink:
;dd   0,1,2, -3,  0,3,2, 3  ;far side
dd   4,0,1, -5,  4,5,1, 5   ;left side
dd   6,2,3, -5,  6,7,3, 5   ;right side
dd   4,0,3, -5,  4,7,3, 5   ;up side
dd   5,1,2, -5,  5,6,2, 5   ;down side
dd   4,5,6, -6,  4,7,6, 6   ;far side

;model 2 outpost
outpost:
dd 8      ;model points
dd 10     ;model triagels
;mpoints:
dd -45,-45,0,   -45,45,0,   45,45,0,   45,-45,0
dd -30,-30,-20, -30,30,-20, 30,30,-20, 30,-30,-20
;pointslink:
;dd   0,1,2, -3,  0,3,2, 3  ;far side
dd   4,0,1, -8,  4,5,1, 8   ;left  side
dd   6,2,3, -8,  6,7,3, 8   ;right side
dd   4,0,3, -8,  4,7,3, 8   ;up side
dd   5,1,2, -8,  5,6,2, 8   ;down side
dd   4,5,6, -7,  4,7,6, 7   ;near side

;model 3 cannon
cannon:
dd 12      ;model points
dd 12     ;model triagels
;mpoints:
dd -10,-20,0,   -10,20,0,   10,20,0,   10,-20,0
dd -10,-10,-15, -10,10,-15, 10,10,-15, 10,-10,-15
dd -2,15,-8,    2,15,-8,    -2,45,-8,  2,45,-8
;pointslink:
;dd   0,1,2, -3,  0,3,2, 3  ;far side
dd   4,0,1, -10,  4,5,1, 10   ;left  side
dd   6,2,3, -10,  6,7,3, 10   ;right side
dd   4,0,3, -10,  4,7,3, 10   ;up side
dd   5,1,2, -10,  5,6,2, 10   ;down side
dd   4,5,6, -11,  4,7,6, 11   ;near side
dd   8,9,10,-10,  9,10,11,10  ;cannon 1

;model 4 red flag
red_flag:
dd 12      ;model points
dd 6*2     ;model triagels
;mpoints:
dd -1,-1,0,   -1,1,0,    1,1,0,    1,-1,0
dd -1,-1,-30, -1,1,-30,  1,1,-30,  1,-1,-30
dd 1,1,-30,  10,1,-30,  10,1,-20, 1,1,-20
;pointslink:
dd   4,0,1, -10,  4,5,1, 15   ;left  side
dd   6,2,3, -10,  6,7,3, 15   ;right side
dd   4,0,3, -10,  4,7,3, 15   ;up side
dd   5,1,2, -10,  5,6,2, 15   ;down side
dd   4,5,6, -10,  4,7,6, 15   ;near side
dd   8,11,10,-49, 8,9,10,49   ;flag

repear:
dd 8      ;model points
dd 10     ;model triagels
;mpoints:
dd -45,-45,0,   -45,45,0,   45,45,0,   45,-45,0
dd -30,-30,-20, -30,30,-20, 30,30,-20, 30,-30,-20
;pointslink:
dd   4,0,1, -5,  4,5,1, 5   ;left side
dd   6,2,3, -5,  6,7,3, 5   ;right side
dd   4,0,3, -5,  4,7,3, 5   ;up side
dd   5,1,2, -5,  5,6,2, 5   ;down side
dd   4,5,6, -3,  4,7,6, 3   ;far side

;model 5 tree (elka)
tree:
dd 16      ;model points
dd 8*2     ;model triagels
;mpoints:
dd -2,-2,0,   -2,2,0,    2,2,0,    2,-2,0
dd -1,-1,-8, -1,1,-8,  1,1,-8,  1,-1,-8
dd -10,-10,-8,  -10,10,-8,   10,10,-8,   10,-10,-8
dd -1,-1,-40, -1,1,-40,  1,1,-40,  1,-1,-40
;pointslink:
dd   4,0,1, -16,  4,5,1, 16   ;left  side
dd   6,2,3, -16,  6,7,3, 16   ;right side
dd   4,0,3, -16,  4,7,3, 16   ;up side
dd   5,1,2, -16,  5,6,2, 16   ;down side

dd   12,8,9, -14,  12,13,9, 14   ;left  side
dd   14,10,11, -14,  14,15,11, 14   ;right side
dd   12,8,11, -14,  12,15,11, 14   ;up side
dd   13,9,10, -14,  13,14,10, 14   ;down side

;model tank
tank:
dd 20      ;model points
dd 12+10     ;model triagels
;mpoints:
dd -10,-20,-10, -10,20,-10,   10,20,-10,  10,-20,-10
dd -10,-10,-20, -10,10,-20,   10,10,-20,  10,-10,-20
dd -2,15,-15,    2,15,-15,    -2,45,-15,   2,45,-15

dd -20,-20,0,   -20,20,0,   20,20,0,   20,-20,0
dd -20,-30,-10, -20,30,-10, 20,30,-10, 20,-30,-10

;pointslink:
;dd   0,1,2, -3,  0,3,2, 3  ;far side
dd   4,0,1, -10,  4,5,1, 10   ;left  side
dd   6,2,3, -10,  6,7,3, 10   ;right side
dd   4,0,3, -10,  4,7,3, 10   ;up side
dd   5,1,2, -10,  5,6,2, 10   ;down side
dd   4,5,6, -11,  4,7,6, 11   ;near side
dd   8,9,10,-10,  9,10,11,10  ;cannon 1

dd   16,12,13, -11,  16,17,13, 11   ;left  side
dd   18,14,15, -11,  18,19,15, 11   ;right side
dd   16,12,15, -11,  16,19,15, 11   ;up side
dd   17,13,14, -11,  17,18,14, 11   ;down side
dd   16,17,18, -11,  16,19,18, 11   ;near side

;Test model
gqfa:  ;Good quality four angle
dd 5     ;model points
dd 4     ;model triagels
;mpoints:
;dd -45,-145,0,   -45,145,0,   45,45,0,   45,-45,0
dd -0,-105,0,   -0,105,0,   45,45,0,   45,-45,0
dd 30,0,0
;pointslink:
dd   1,4,0, 50  ;left side
dd   1,4,2, 51 ;right side
dd   3,4,2, -50 ;up side
dd   3,4,0, -51  ;down side

skip_mdata:

;***************
; Add new models
;***************
;    jmp no_addmodel

    random 20,eax
    sub eax,10
    imul eax,SECTOR_SIZE
    add eax,SECTOR_SIZE/2
    mov [temp1],eax
    random 20,eax
    sub eax,10
    imul eax,SECTOR_SIZE
    add eax,SECTOR_SIZE/2
    mov [temp2],eax

    mov eax,dword [model_mas]
    cmp eax,40;00
    jae no_addmodel
    imul eax,4*8;
    add eax,4+model_mas
    inc dword [model_mas]
    mov ebx,[temp1]
    mov [eax],ebx
    mov ebx,[temp2]
    mov [eax+4],ebx
    mov [eax+8],dword -170
    mov ebx,0
    mov [eax+12],ebx
    mov [eax+16],ebx
    mov [eax+20],ebx
    mov ebx,tree;red_flag
    mov [eax+28],ebx
    jmp skip_mdata     ;use for auto filling at start
no_addmodel:

    ret


UpdateAngles:
; Calculates new x,y,z angles
; to autorotate around

   mov     eax,[Xangle]         ; Load current angles
   mov     ebx,[Yangle]
   mov     ecx,[Zangle]

   add     eax,[DeltaX]         ; Add velocity
   and     eax,011111111b        ; Range from 0..255
   mov     [Xangle],eax         ; Update X
   add     ebx,[DeltaY]         ; Add velocity
   and     ebx,011111111b        ; Range from 0..255
   mov     [Yangle],ebx         ; Update Y
   add     ecx,[DeltaZ]         ; Add velocity
   and     ecx,011111111b        ; Range from 0..255
   mov     [Zangle],ecx         ; Update Z
   ret

GetSinCos:
; Needed : bx=angle (0..255)
; Returns: ax=Sin   bx=Cos
    push    ebx                  ; Save angle (use as pointer)
    shl     ebx,2                ; Grab a word so bx=bx*2
    mov     eax,[SinCos + ebx]    ; Get sine
    pop     ebx                  ; Restore pointer into bx
    push    eax                  ; Save sine on stack
    add     ebx,64               ; Add 64 to get cosine
    and     ebx,11111111b        ; Range from 0..255
    shl     ebx,2                ; *2 coz it's a word
    mov     eax,[SinCos + ebx]    ; Get cosine
    mov     ebx,eax               ; Save it   bx=Cos
    pop     eax                  ; Restore   ax=Sin
    ret

; Get sin & cos of x,y,z angle
SetRotation:
    mov     ebx,[Xangle]         ; Grab angle
    call    GetSinCos           ; Get the sine&cosine
    mov     [Xsin],eax           ; Save sin
    mov     [Xcos],ebx           ; Save cos

    mov     ebx,[Yangle]
    call    GetSinCos
    mov     [Ysin],eax
    mov     [Ycos],ebx

    mov     ebx,[Zangle]
    call    GetSinCos
    mov     [Zsin],eax
    mov     [Zcos],ebx
; for camera
    mov     ebx,[Xcamangle]
    call    GetSinCos
    mov     [Xcamsin],eax
    mov     [Xcamcos],ebx

    mov     ebx,[Ycamangle]
    call    GetSinCos
    mov     [Ycamsin],eax
    mov     [Ycamcos],ebx

    mov     ebx,[Zcamangle]
    call    GetSinCos
    mov     [Zcamsin],eax
    mov     [Zcamcos],ebx

    ret

TranslatePoint:            ; Rotates the point around x,y,z
; Gets original x,y,z values
; This can be done elsewhere

    mov esi,[X]
    mov edi,[Y]
    mov ebx,[Zcamcos]
    mov ecx,[Zcamsin]
    call math
    mov [X],esi
    mov [Y],edi

    mov esi,[Y]
    mov edi,[Z]
    mov ebx,[Xcamcos]
    mov ecx,[Xcamsin]
    call math
    mov [Y],esi
    mov [Z],edi

    mov esi,[X]
    mov edi,[Z]
    mov ebx,[Ycamcos]
    mov ecx,[Ycamsin]
    call math
    mov [X],esi
    mov [Z],edi

    mov esi,[Y]
    mov ebx,[Xcos]
    mov edi,[Z]
    mov ecx,[Xsin]
    call math
    mov [Y],esi
    mov [Z],edi
    mov esi,[X]
    mov ebx,[Ycos]
    mov ecx,[Ysin]
    call math
    mov [X],esi
    mov [Z],edi
    mov ebx,[Zcos]
    mov edi,[Y]
    mov ecx,[Zsin]
    call math

;*************
;  ShowPoint
;*************

; Calculates screenposition and
; plots the point on the screen
    mov     eax,[Xoff]           ; Xoff*X / Z+Zoff = screen x
    mov     ecx,esi
    imul    ecx

    mov     ebx,[Z]
    add     ebx,[Zoff]           ; Distance

    cmp ebx,0
    je notout
    idiv    ebx
    add     eax,[Mx]             ; Center on screen
    mov     ebp,eax              ;ebp =Xp

    mov     eax,[Yoff]           ; Yoff*Y / Z+Zoff = screen y
    mov     ecx,edi
    imul    ecx

    cmp ebx,0
    je notout
    idiv    ebx                  ;eax =Yp
    add     eax,[My]             ; Center on screen

notout:
    ret

math:
    mov     eax,esi
    imul    ebx               ; ax = X * Cos(zang)
    mov     ebp,eax
    mov     eax,edi
    imul    ecx               ; ax = Y * Sin(zang)
    sub     ebp,eax            ; bp = X * Cos(zang) - Y * Sin(zang)
    sar     ebp,8             ; bp = X * Cos(zang) - Y * Sin(zang) / 256

    mov     eax,esi
    mov     esi,ebp
    imul    ecx               ; ax = X * Sin(zang)
    mov     ebp,eax
    mov     eax,edi
    imul    ebx               ; ax = Y * Cos(zang)
    add     ebp,eax            ; bp = X * SIN(zang) + Y * COS(zang)
    sar     ebp,8             ; bp = X * SIN(zang) + Y * COS(zang) / 256
    mov     edi,ebp
    ret

SinCos:
dd 0,6,13,19,25,31,38,44,50,56
dd 62,68,74,80,86,92,98,104,109,115
dd 121,126,132,137,142,147,152,157,162,167
dd 172,177,181,185,190,194,198,202,206,209
dd 213,216,220,223,226,229,231,234,237,239
dd 241,243,245,247,248,250,251,252,253,254
dd 255,255,256,256,256,256,256,255,255,254
dd 253,252,251,250,248,247,245,243,241,239
dd 237,234,231,229,226,223,220,216,213,209
dd 206,202,198,194,190,185,181,177,172,167
dd 162,157,152,147,142,137,132,126,121,115
dd 109,104,98,92,86,80,74,68,62,56
dd 50,44,38,31,25,19,13,6,0,-6
dd -13,-19,-25,-31,-38,-44,-50,-56,-62,-68
dd -74,-80,-86,-92,-98,-104,-109,-115,-121,-126
dd -132,-137,-142,-147,-152,-157,-162,-167,-172,-177
dd -181,-185,-190,-194,-198,-202,-206,-209,-213,-216
dd -220,-223,-226,-229,-231,-234,-237,-239,-241,-243
dd -245,-247,-248,-250,-251,-252,-253,-254,-255,-255
dd -256,-256,-256,-256,-256,-255,-255,-254,-253,-252
dd -251,-250,-248,-247,-245,-243,-241,-239,-237,-234
dd -231,-229,-226,-223,-220,-216,-213,-209,-206,-202
dd -198,-194,-190,-185,-181,-177,-172,-167,-162,-157
dd -152,-147,-142,-137,-132,-126,-121,-115,-109,-104
dd -98,-92,-86,-80,-74,-68,-62,-56,-50,-44
dd -38,-31,-25,-19,-13,-6

mipzoom dd 0
mipmapwork dd 0
temp1 dd 0
temp2 dd 0

;  4.24.2005 Textured triangle algorythm
;  created by Pavlushin Evgeni waptap[at]mail.ru
;  on base tex3 from Mikolaj Felix mfelix@polbox.com

align 512
@@tx1 dd 0 ;equ [bp+4]
@@ty1 dd 0 ;equ [bp+6]
align 512
@@tx2 dd 0 ;equ [bp+8]
@@ty2 dd 0 ;equ [bp+10]
align 512
@@tx3 dd 0 ;equ [bp+12]
@@ty3 dd 0 ;equ [bp+14]

@@tz1 dd 0
@@tz2 dd 0
@@tz3 dd 0

@@z_dy12 dd 0
@@z_dy13 dd 0
@@z_dy23 dd 0

@@tex_off dd 0 ;equ [bp+16]
@@tex_x1 dd 0 ;equ [bp+18]
@@tex_y1 dd 0 ;equ [bp+20]
@@tex_x2 dd 0 ;equ [bp+22]
@@tex_y2 dd 0 ;equ [bp+24]
@@tex_x3 dd 0 ;equ [bp+26]
@@tex_y3 dd 0 ;equ [bp+28]

@@dx12 dd 0 ;equ [bp-2]
@@dx13 dd 0 ;equ [bp-4]
@@dx23 dd 0 ;equ [bp-6]

@@tex_dx12 dd 0 ;equ [bp-8]
@@tex_dy12 dd 0 ;equ [bp-10]
@@tex_dx13 dd 0 ;equ [bp-12]
@@tex_dy13 dd 0 ;equ [bp-14]
@@tex_dx23 dd 0 ;equ [bp-16]
@@tex_dy23 dd 0 ;equ [bp-18]

@@scan_x1 dd 0
@@scan_y1 dd 0
@@scan_x2 dd 0
@@scan_y2 dd 0

@@scan_z1 dd 0
@@scan_z2 dd 0

SHIFT=16

textured_triangle:

	mov eax,[@@ty1]
	cmp eax,[@@ty3]
	jle tt_check1

	xchg eax,[@@ty3]
	mov [@@ty1],eax

	mov eax,[@@tx1]
	xchg eax,[@@tx3]
	mov [@@tx1],eax

	mov eax,[@@tz1]
	xchg eax,[@@tz3]
	mov [@@tz1],eax

	mov eax,[@@tex_y1]
	xchg eax,[@@tex_y3]
	mov [@@tex_y1],eax

	mov eax,[@@tex_x1]
	xchg eax,[@@tex_x3]
	mov [@@tex_x1],eax
tt_check1:
	mov eax,[@@ty2]
	cmp eax,[@@ty3]
	jle tt_check2

	xchg eax,[@@ty3]
	mov [@@ty2],eax

	mov eax,[@@tx2]
	xchg eax,[@@tx3]
	mov [@@tx2],eax

	mov eax,[@@tz2]
	xchg eax,[@@tz3]
	mov [@@tz2],eax

	mov eax,[@@tex_y2]
	xchg eax,[@@tex_y3]
	mov [@@tex_y2],eax

	mov eax,[@@tex_x2]
	xchg eax,[@@tex_x3]
	mov [@@tex_x2],eax
tt_check2:
	mov eax,[@@ty1]
	cmp eax,[@@ty2]
	jle tt_check3

	xchg eax,[@@ty2]
	mov [@@ty1],eax

	mov eax,[@@tx1]
	xchg eax,[@@tx2]
	mov [@@tx1],eax

	mov eax,[@@tz1]
	xchg eax,[@@tz2]
	mov [@@tz1],eax

	mov eax,[@@tex_y1]
	xchg eax,[@@tex_y2]
	mov [@@tex_y1],eax

	mov eax,[@@tex_x1]
	xchg eax,[@@tex_x2]
	mov [@@tex_x1],eax
tt_check3:

	mov ebx,[@@ty2]
	sub ebx,[@@ty1]
	jnz tt_dx12_make

	mov [@@dx12],0
	mov [@@tex_dx12],0
	mov [@@tex_dy12],0
	mov [@@z_dy12],0
	jmp tt_dx12_done
tt_dx12_make:
	mov eax,[@@tx2]
	sub eax,[@@tx1]
	shl eax,SHIFT
	cdq
	idiv ebx
	mov [@@dx12],eax			; dx12 = (x2-x1)/(y2-y1)

	mov eax,[@@tex_x2]
	sub eax,[@@tex_x1]
	cdq
	idiv ebx
	mov [@@tex_dx12],eax		; tex_dx12 = (tex_x2-tex_x1)/(y2-y1)

	mov eax,[@@tex_y2]
	sub eax,[@@tex_y1]
	cdq
	idiv ebx
	mov [@@tex_dy12],eax		; tex_dy12 = (tex_y2-tex_y1)/(y2-y1)

	mov eax,[@@tz2]
	sub eax,[@@tz1]
	shl eax,SHIFT
	cdq
	idiv ebx
	mov [@@z_dy12],eax	

tt_dx12_done:

	mov ebx,[@@ty3]
	sub ebx,[@@ty1]
	jnz tt_dx13_make

	mov [@@dx13],0
	mov [@@tex_dx13],0
	mov [@@tex_dy13],0
 mov [@@z_dy13],0
	jmp tt_dx13_done
tt_dx13_make:
	mov eax,[@@tx3]
	sub eax,[@@tx1]
	shl eax,SHIFT
	cdq
	idiv ebx
	mov [@@dx13],eax			; dx13 = (x3-x1)/(y3-y1)

	mov eax,[@@tex_x3]
	sub eax,[@@tex_x1]
	cdq
	idiv ebx
	mov [@@tex_dx13],eax		; tex_dx13 = (tex_x3-tex_x1)/(y3-y1)

	mov eax,[@@tex_y3]
	sub eax,[@@tex_y1]
	cdq
	idiv ebx
	mov [@@tex_dy13],eax		; tex_dy13 = (tex_y3-tex_x1)/(y3-y1)

	mov eax,[@@tz3]
	sub eax,[@@tz1]
	shl eax,SHIFT
	cdq
	idiv ebx
	mov [@@z_dy13],eax		
tt_dx13_done:

	mov ebx,[@@ty3]
	sub ebx,[@@ty2]
	jnz tt_dx23_make

	mov [@@dx23],0
	mov [@@tex_dx23],0
	mov [@@tex_dy23],0
	mov [@@z_dy23],0
	jmp tt_dx23_done
tt_dx23_make:
	mov eax,[@@tx3]
	sub eax,[@@tx2]
	shl eax,SHIFT
	cdq
	idiv ebx
	mov [@@dx23],eax			; dx23 = (x3-x2)/(y3-y2)

	mov eax,[@@tex_x3]
	sub eax,[@@tex_x2]
	cdq
	idiv ebx
	mov [@@tex_dx23],eax		; tex_dx23 = (tex_x3-tex_x2)/(y3-y2)

	mov eax,[@@tex_y3]
	sub eax,[@@tex_y2]
	cdq
	idiv ebx
	mov [@@tex_dy23],eax		; tex_dy23 = (tex_y3-tex_y2)/(y3-y2)

	mov eax,[@@tz3]
	sub eax,[@@tz2]
	shl eax,SHIFT
	cdq
	idiv ebx
	mov [@@z_dy23],eax		
tt_dx23_done:


	mov eax,[@@tx1]
	shl eax,SHIFT
	mov ebx,eax
	mov ecx,[@@ty1]

 mov edx,[@@tz1]
 shl edx,SHIFT
 mov [@@scan_z1],edx
 mov [@@scan_z2],edx

	mov edx,[@@tex_x1]
	mov [@@scan_x1],edx
	mov [@@scan_x2],edx
	mov edx,[@@tex_y1]
	mov [@@scan_y1],edx
	mov [@@scan_y2],edx

; ****************
 mov edx,[@@ty1]   ;skip equals
 cmp [@@ty2],edx
 je tt_loop1_end

 mov ebp,[@@ty1]
 cmp ebp,0
 jg no_up_clip
 neg ebp
 mov edx,[@@ty2]
 cmp edx,0
 jg no_sbx
 neg edx
 sub ebp,edx
no_sbx:

	mov edx,[@@tex_dx13]
 imul edx,ebp
	add [@@scan_x1],edx
	mov edx,[@@tex_dx12]
 imul edx,ebp
	add [@@scan_x2],edx
	mov edx,[@@tex_dy13]
 imul edx,ebp
	add [@@scan_y1],edx
	mov edx,[@@tex_dy12]
 imul edx,ebp
	add [@@scan_y2],edx

	mov edx,[@@z_dy13]
 imul edx,ebp
	add [@@scan_z1],edx
	mov edx,[@@z_dy12]
 imul edx,ebp
	add [@@scan_z2],edx

 mov edx,[@@dx13]
 imul edx,ebp
	add eax,edx
 mov edx,[@@dx12]
 imul edx,ebp
	add ebx,edx
	add ecx,ebp
no_up_clip:

 cmp [@@ty2],0
 jl  tt_loop1_end

tt_loop1:
 cmp ecx,SCREEN_Y
 jge tt_loop2_end

 pushad
	mov edx,[@@scan_y2]
	mov [@@tex_ly2],edx ;push dx
	mov edx,[@@scan_x2]
	mov [@@tex_lx2],edx ;push dx
	mov edx,[@@scan_y1]
	mov [@@tex_ly1],edx ;push dx
	mov edx,[@@scan_x1]
	mov [@@tex_lx1],edx ;push dx

 mov edx,[@@scan_z1]
 mov [@@lz1],edx
 mov edx,[@@scan_z2]
 mov [@@lz2],edx

	mov [@@ly],ecx ;push cx
	mov edx,ebx
	sar edx,SHIFT
	mov [@@lx2],edx ;push dx
	mov edx,eax
	sar edx,SHIFT
	mov [@@lx1],edx ; push dx
	call textured_horizontal_line
 popad

	mov edx,[@@tex_dx13]
	add [@@scan_x1],edx
	mov edx,[@@tex_dx12]
	add [@@scan_x2],edx
	mov edx,[@@tex_dy13]
	add [@@scan_y1],edx
	mov edx,[@@tex_dy12]
	add [@@scan_y2],edx

	mov edx,[@@z_dy13]
	add [@@scan_z1],edx
	mov edx,[@@z_dy12]
	add [@@scan_z2],edx

	add eax,[@@dx13]
	add ebx,[@@dx12]
	inc ecx
	cmp ecx,[@@ty2]
	jl tt_loop1

tt_loop1_end:

	mov ebx,[@@tx2]
	shl ebx,SHIFT
	mov ecx,[@@ty2]

 mov edx,[@@tz2]
 shl edx,SHIFT
 mov [@@scan_z2],edx

	mov edx,[@@tex_x2]
	mov [@@scan_x2],edx
	mov edx,[@@tex_y2]
	mov [@@scan_y2],edx

 mov ebp,[@@ty2]
 cmp ebp,0
 jg no_down_clip
 neg ebp
 dec ebp

	mov edx,[@@tex_dx13]
 imul edx,ebp
	add [@@scan_x1],edx
	mov edx,[@@tex_dx23]
 imul edx,ebp
	add [@@scan_x2],edx
	mov edx,[@@tex_dy13]
 imul edx,ebp
	add [@@scan_y1],edx
	mov edx,[@@tex_dy23]
 imul edx,ebp
	add [@@scan_y2],edx

	mov edx,[@@z_dy13]
 imul edx,ebp
	add [@@scan_z1],edx
	mov edx,[@@z_dy23]
 imul edx,ebp
	add [@@scan_z2],edx

 mov edx,[@@dx13]
 imul edx,ebp
	add eax,edx
 mov edx,[@@dx23]
 imul edx,ebp
	add ebx,edx
 add ecx,ebp
no_down_clip:

tt_loop2:
 cmp ecx,SCREEN_Y
 jge tt_loop2_end

 pushad
	mov edx,[@@scan_y2]
	mov [@@tex_ly2],edx ;push dx
	mov edx,[@@scan_x2]
	mov [@@tex_lx2],edx ;push dx
	mov edx,[@@scan_y1]
	mov [@@tex_ly1],edx ;push dx
	mov edx,[@@scan_x1]
	mov [@@tex_lx1],edx ;push dx

 mov edx,[@@scan_z1]
 mov [@@lz1],edx
 mov edx,[@@scan_z2]
 mov [@@lz2],edx

	mov [@@ly],ecx ;push cx
	mov edx,ebx
	sar edx,SHIFT
	mov [@@lx2],edx ;push dx
	mov edx,eax
	sar edx,SHIFT
	mov [@@lx1],edx ; push dx
	call textured_horizontal_line
 popad

	mov edx,[@@tex_dx13]
	add [@@scan_x1],edx
	mov edx,[@@tex_dx23]
	add [@@scan_x2],edx
	mov edx,[@@tex_dy13]
	add [@@scan_y1],edx
	mov edx,[@@tex_dy23]
	add [@@scan_y2],edx

	mov edx,[@@z_dy13]
	add [@@scan_z1],edx
	mov edx,[@@z_dy23]
	add [@@scan_z2],edx

	add eax,[@@dx13]
	add ebx,[@@dx23]
	inc ecx
	cmp ecx,[@@ty3]
	jl tt_loop2

tt_loop2_end:
	ret

align 512
@@lx1 dd 0
align 512
@@lx2 dd 0
align 512
@@ly dd  0

@@lz1 dd 0
@@lz2 dd 0
@@z_dx dd 0

align 512
@@tex_loff dd 0 ;equ [bp+10]
@@tex_lx1 dd 0 ;equ [bp+12]
@@tex_ly1 dd 0 ;equ [bp+14]
@@tex_lx2 dd 0 ;equ [bp+16]
@@tex_ly2 dd 0 ;equ [bp+18]

align 512
@@tex_ldx dd 0 ;equ [bp-2]
@@tex_ldy dd 0 ;equ [bp-4]

align 1024
textured_horizontal_line:

	mov eax,[@@lx1]
	cmp eax,[@@lx2]
	je thl_quit
	jl thl_ok

	xchg eax,[@@lx2]
	mov [@@lx1],eax

 mov eax,[@@lz1]
	xchg eax,[@@lz2]
	mov [@@lz1],eax

	mov eax,[@@tex_lx1]
	xchg eax,[@@tex_lx2]
	mov [@@tex_lx1],eax

	mov eax,[@@tex_ly1]
	xchg eax,[@@tex_ly2]
	mov [@@tex_ly1],eax

thl_ok:

 cmp [@@lx2],0
 jle thl_quit
 mov eax,SCREEN_X
 cmp [@@lx1],eax
 jge thl_quit

	mov ecx,[@@lx2]
	sub ecx,[@@lx1]

 ; Uneversal method
 mov edi,[@@ly]
 mov eax,SCREEN_X   ;di = ly*320+lx1
 imul edi
 add eax,[@@lx1]
 imul eax,3
 mov edi,eax
 add edi,I_END

 ;Right side clipping
 mov eax,SCREEN_X
 cmp [@@lx2],eax
 jnge x1ok
 mov edx,SCREEN_X
 dec edx
 sub edx,[@@lx1]
 cmp edx,0
 jle thl_quit
 mov ecx,edx
x1ok:

 ;Left side clipping
 cmp [@@lx1],0
 jg  x2ok
 mov ecx,[@@lx2]     ;cmp lx2 screen x if above cut (for future)
 mov edi,[@@ly]
 mov eax,SCREEN_X
 imul edi,eax
 imul edi,3
 add edi,I_END
x2ok:

	mov ebx,[@@lx2]
	sub ebx,[@@lx1]

	mov eax,[@@tex_lx2]
	sub eax,[@@tex_lx1]
	cdq
	idiv ebx
	mov [@@tex_ldx],eax			; tex_dx = (tex_x2-tex_x1)/(x2-x1)

	mov eax,[@@tex_ly2]
	sub eax,[@@tex_ly1]
	cdq
	idiv ebx
	mov [@@tex_ldy],eax			; tex_dy = (tex_y2-tex_y1)/(x2-x1)

	mov eax,[@@lz2]
	sub eax,[@@lz1]
	cdq
	idiv ebx
	mov [@@z_dx],eax			; tex_dx = (tex_x2-tex_x1)/(x2-x1)

 ; Left clipping post correction
 cmp [@@lx1],0
 jg  no_lcpc
 mov eax,[@@lx1]
 neg eax
 mov ebp,[@@tex_ldx]
 imul ebp,eax
 add [@@tex_lx1],ebp
 mov ebp,[@@tex_ldy]
 imul ebp,eax
 add [@@tex_ly1],ebp
 mov ebp,[@@z_dx]
 imul ebp,eax
 add [@@lz1],ebp
no_lcpc:

	inc ecx ;for equal correction
	mov edx,[@@tex_lx1]
	mov ebx,[@@tex_ly1]
 mov esi,[@@lz1]

thl_loop:
	mov ebp,esi
 shr ebp,SHIFT
 mov eax,dword [edi-I_END+zbuffer]
 cmp ax,bp
 jb  no_set
 mov [edi-I_END+zbuffer],bp

 mov eax,edx
; and eax,0x0fff0000
 ror ebx,16
 mov ax,bx
 ror ebx,16
 shl ax,11    ; 8 for 256*256, 9 for 128*128 ...
 shr eax,11
 lea eax,[eax*2+eax] ; equ imul eax,3
	add eax,[@@tex_off]
 mov ebp,eax

	mov ax,word [ebp]
 mov [edi],ax
	mov al,byte [ebp+2]
 mov [edi+2],al

no_set:
 add edi,3

 add esi,[@@z_dx]
	add edx,[@@tex_ldx]
	add ebx,[@@tex_ldy]
	dec ecx
	jnz thl_loop
thl_quit:
 ret


@@rgb  dd 0
@@rgbax  dw 0
@@rgbbl  db 0

; === DATA ===

d_angle  dd 0.19634954 ;pi/16
angle    dd 0.0
mul_wave dd 1.5

sin_pos dw 0
sinwave rw 256

Xmap   dd 0
Ymap   dd 0
mapoff dd 0

Xcam   dd 0
Ycam   dd 0
Zcam   dd -400

Xcamangle dd 0
Ycamangle dd 0
Zcamangle dd 0

Xcamsin   dd 0
Xcamcos   dd 0
Ycamsin   dd 0
Ycamcos   dd 0
Zcamsin   dd 0
Zcamcos   dd 0

X      dd ?             ; X,Y,Z variable for formula
Y      dd ?
Z      dd ?

Xp dd ?
Yp dd ?

Xangle dd 0             ; Angle to rotate around x
Yangle dd 0
Zangle dd 0

DeltaX dd 1             ; x,y,z rotation angle
DeltaY dd 1
DeltaZ dd 1

Xoff   dd 256           ; x-cord
Yoff   dd 256           ; y-cord
Zoff   dd 0;800         ; Distance from viewer

Xsin   dd ?             ; Sine and cosine of angle to rotate around
Xcos   dd ?
Ysin   dd ?
Ycos   dd ?
Zsin   dd ?
Zcos   dd ?

Mx     dd SCREEN_X/2            ; Center  of the screen
My     dd SCREEN_Y/2

autorot db 0                    ;Auto rotation flag
massize dd 0                    ;Size of triangle massive
id      dd 0
temp    dd 0

; DATA AREA
labelt:
     db   '3D TEXTURED ENGINE'
labellen:


;Texture pusck 32*32 256
texpack_gif:
file 'TEXPACK.gif'
rb 50

;Ground texture
bmp_texmap:
file 'TEXMAP.bmp'
rb 50


; Ground livel map
gif_file_area5:
file 'MAP.gif' ;level map
rb 50

rb 8
texture_limg:
rb 4095
texture_img:
rb 4095
texture_img2:
rb 4095

rb (4095)*16*16

img_area:
rb 128*128*3+8

glm_img_area:
rb (MAP_XSIZE+1)*(MAP_YSIZE+1)*3+8

rb 8

texpack_img:
rb 512*512*3+8

keymap:
rb 1000

model_mas:
rb 4+8*100

; Hash area for decoding GIF
gif_hash_area:
rd 4096+1

MAX_MODEL_POINTS = 100
modelpoints:
rb MAX_MODEL_POINTS*3*4

; Matrix points massive
points:
; Massive of turn matrix points
tpoints=points+((MATRIX_XSIZE+1)*(MATRIX_YSIZE+1)*3*4)
; Ground level map massive
ground_level_map=tpoints+((MATRIX_XSIZE+1)*(MATRIX_YSIZE+1)*3*4)
; zbuffer
zbuffer=ground_level_map+((MAP_XSIZE+1)*(MAP_YSIZE+1)*4)
I_END=zbuffer+(SCREEN_X*SCREEN_Y)*3
