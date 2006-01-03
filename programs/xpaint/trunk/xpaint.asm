;
;   Show Mini Sample by Pavlushin Evgeni for ASCL
;   www.waptap@mail.ru
;

;bits brushes(right mouse button)!
;resize pucture(cross buttons)!
;Open BMP MIA MHI grafic files!
;Save as MIA MHI(zip from MHC -70% size)!
;Support full files 800*600
;Open and save dialogs is work!

; 27Ver Gif load support
; 28Ver Dialogs is work

;******************************************************************************
    use32
    org    0x0
    db     'MENUET01'              ; 8 byte id
    dd     0x01                    ; header version
    dd     START                   ; start of code
    dd     IM_END                  ; size of image
    dd     I_END                ; memory for app
    dd     I_END               ; esp
    dd     0x0 , 0x0         ; I_Param , I_Icon

;******************************************************************************
include 'lang.inc'
include 'macros.inc'
include 'ascl.inc'
include 'ascgl.inc'

 hashtable equ MHC_END
 ifile     equ MHC_END+65536*8 ;4
 ofile     equ (MHC_END+65536*8)+2000000 ;1000000

 gif_hash_offset = ofile

START:                          ; start of execution
    mov eax,40
    mov ebx,0100111b
    int 0x40

; clear picture color White
    mov ecx,1024*768*3
    mov eax,0xffffffff
    mov edi,image_area
    cld
    rep stosb

    call draw_window
    call setbrush

rstill:
    mov eax,37
    mov ebx,1
    int 0x40
    mov ebx,eax
    shr eax,16     ;x
    and ebx,0xffff ;y
    sub eax,5+6
    sub ebx,22+30+6

    cmp eax,640 ;for detect out of screen
    ja  still
    cmp ebx,480
    ja  still
    mov [xt],ebx
    mov [yt],eax

still:
    mov eax,40
    mov ebx,0100111b
    int 0x40

    wtevent red,key,button

    mov ebx,2
    mov eax,37
    int 0x40
    cmp eax,1
    je  tescor ;if left button not pressed!
    cmp eax,2
    je  cbr ;if right button pressed!
    jmp rstill
;    jmp still

cbr:
    cmp dword [brush],9 ;9 brush matrix
    jnae nocr
    mov [brush],-1 ;0
nocr:
    inc [brush]
    call setbrush
    jmp still

;Set brush
setbrush:
    mov eax,[brush]
    mov ebx,4*32
    mul ebx

    mov ecx,4*32
    mov esi,brushtable
    add esi,eax
    mov edi,matrix
    cld
    rep movsb

    delay 10
    ret

tescor:
    mov eax,37
    mov ebx,1
    int 0x40
    mov ebx,eax
    shr eax,16     ;x
    and ebx,0xffff ;y

    sub eax,5+6
    sub ebx,22+30+6

    push eax
    push ebx

    mov ecx,eax ;[yt]
    mov edx,ebx  ;[xt]

    mov eax,[xt]
    mov ebx,[yt]
    pop [xt]
    pop [yt]

    call line

;    mov [yt],eax
;    mov [xt],ebx
    call out_image

    jmp  still

  red:
    call draw_window
    jmp still

  key:                          ; key
    int  0x40                   ; read (eax=2)
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                 ; close this program
    int  0x40
noclose:
    cmp  ah,10
    jb   no_palbut
    cmp  ah,10+32;64
    ja   no_palbut
    mov  ecx,0
    mov  cl,ah
    sub  cl,10
    shl  ecx,2
    mov  ebx,[colortable+ecx]
    mov  [color],ebx

;Update color boxes
    drawfbox 20,32,20,15,[backcolor]
    drawfbox 12,28,20,15,[color]

    jmp  still
no_palbut:

    cmp  ah,6
    jne  no_left

    mov  esi,0
    mov  edi,0
    mov  ebp,[img_ysize]
x_pls:
    mov  ecx,0
    mov  edx,[img_xsize]
x_pl:
    mov  al,[image_area+edi+ecx]
    mov  [image_area+esi+ecx],al
    mov  al,[image_area+edi+ecx+1]
    mov  [image_area+esi+ecx+1],al
    mov  al,[image_area+edi+ecx+2]
    mov  [image_area+esi+ecx+2],al
    add  ecx,3
    dec  edx
    jnz  x_pl
    add  edi,[img_xsize]
    add  edi,[img_xsize]
    add  edi,[img_xsize]
    add  esi,[img_xsize]
    add  esi,[img_xsize]
    add  esi,[img_xsize]
    sub  esi,3
    dec  ebp
    jns  x_pls

    dec  [img_xsize]
    jmp  red
no_left:


    cmp  ah,7
    jne  no_right

    mov  eax,[img_xsize]
    mul  [img_ysize]
    mov  ebx,3
    mul  ebx
    mov  edi,eax

    mov  eax,[img_xsize]
    inc  eax
    mul  [img_ysize]
    mov  ebx,3
    mul  ebx
    mov  esi,eax

;    mov  esi,0
;    mov  edi,0
    mov  ebp,[img_ysize]
x_mns:
    mov  ecx,[img_xsize] ;0
    add  ecx,[img_xsize]
    add  ecx,[img_xsize]
    mov  edx,[img_xsize]
x_mn:
    mov  al,[image_area+edi+ecx]
    mov  [image_area+esi+ecx],al
    mov  al,[image_area+edi+ecx+1]
    mov  [image_area+esi+ecx+1],al
    mov  al,[image_area+edi+ecx+2]
    mov  [image_area+esi+ecx+2],al
    sub  ecx,3
    dec  edx
    jnz  x_mn
    mov  dword [image_area+esi+ecx+0],0xffffffff
    sub  edi,[img_xsize]
    sub  edi,[img_xsize]
    sub  edi,[img_xsize]
    sub  esi,[img_xsize]
    sub  esi,[img_xsize]
    sub  esi,[img_xsize]
    sub  esi,3
    dec  ebp
    jns  x_mns

    inc  [img_xsize]
    jmp  red
no_right:

    cmp  ah,8
    jne  no_up
    dec  [img_ysize]
    jmp  red
no_up:

    cmp  ah,9
    jne  no_down
    inc  [img_ysize]
    jmp  red
no_down:

;SAVE MIA
    cmp  ah,80
    jne  no_save
    savedialog draw_window, mia_is_save, still, string
mia_is_save:
    mov  dword [fileinfo],1 ;write
    mov  eax,[img_xsize]
    mul  [img_ysize]
    mov  edx,eax
    add  edx,eax
    add  edx,eax
    add  edx,8
    cmp  edx,1024*768*3+20000
    ja   still
    mov  dword [fileinfo+12],image  ;from read
    mov  dword [fileinfo+8],edx ;1
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
;    puttxt 20,4,string,40,cl_White
    jmp still
no_save:

;OPEN MIA
    cmp ah,81
    jne no_open
    opendialog draw_window, mia_is_open, still, string
mia_is_open:
    mov  dword [fileinfo],0 ;read
    mov  dword [fileinfo+12],image  ;from read
    mov  dword [fileinfo+8],1
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
    mov  eax,ebx
    shr  eax,9
    inc  eax
    mov  dword [fileinfo+8],eax
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
;    puttxt 20,4,string,40,cl_White
    jmp red
no_open:

    cmp  ah,82
    jne  no_save_mhc


    savedialog draw_window, mhi_is_save, still, string
mhi_is_save:
    call fill_filebufs
    mov  eax,[img_xsize]
    mul  [img_ysize]
    mov  edx,eax
    add  edx,eax
    add  edx,eax
    add  edx,8
    mov ecx,edx
    mov esi,image
    mov edi,ifile
    cld
    rep movsb
    mov eax,edx
    call compress
    cmp  edx,1024*768*3+20000
    ja   still
    mov  dword [fileinfo],1 ;write
    mov  dword [fileinfo+12],ofile  ;from read
    mov  dword [fileinfo+8],edx ;1
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
;    puttxt 20,4,string,40,cl_White
    jmp still

no_save_mhc:

;OPEN MHI
    cmp  ah,83
    jne  no_open_mhc

    opendialog draw_window, mhi_is_open, still, string
mhi_is_open:

    call fill_filebufs

    mov  dword [fileinfo],0 ;read
    mov  dword [fileinfo+12],ofile  ;ofile for decompress
    mov  dword [fileinfo+8],1
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
    push ebx ;push file size
    mov  eax,ebx
    shr  eax,9
    inc  eax
    mov  dword [fileinfo+8],eax
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40

;    puttxt 20,4,string,40,cl_White
;    delay 100
    pop  eax ;pop file size
    call decompress

    mov ecx,edx
    mov esi,ifile ;image
    mov edi,image ;ifile
    cld
    rep movsb

    jmp  red
no_open_mhc:

    cmp  ah,84
    jne  no_save_bmp
    jmp  still
no_save_bmp:

    cmp  ah,85
    jne  no_open_bmp

    opendialog draw_window, bmp_is_open, still, string

bmp_is_open:
    mov  dword [fileinfo],0  ;read
    mov  dword [fileinfo+12],MHC_END  ;from read

    mov  dword [fileinfo+8],1
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
    mov  eax,[MHC_END+2]
    shr  eax,9
    inc  eax
    mov  dword [fileinfo+8],eax
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40

;    puttxt 20,4,string,40,cl_White
    bmptoimg MHC_END,image

    jmp red

no_open_bmp:


    cmp  ah,86
    jne  no_save_gif
    jmp  still
no_save_gif:

    cmp  ah,87
    jne  no_open_gif

    opendialog draw_window, gif_is_open, still, string

gif_is_open:
    mov  dword [fileinfo],0  ;read
    mov  dword [fileinfo+12],MHC_END  ;from read

    mov  dword [fileinfo+8],1
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
    mov  eax,[MHC_END+2]
    shr  eax,9
    inc  eax
    mov  dword [fileinfo+8],eax
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40

    puttxt 20,4,string,40,cl_White
    giftoimg MHC_END,image

    jmp red

no_open_gif:



    cmp ah,100
    jne nor1

    mov  eax,[img_xsize]
    mul  [img_ysize]
    mov  edx,eax
    add  edx,eax
    add  edx,eax
    mov  esi,image+8
norx:
    mov al,byte [esi+2]
    and al,01b ;10000000b
    cmp al,0
    jna xe
;    mov al,255
    mov byte [esi],0;255
    mov byte [esi+1],0;0
    mov byte [esi+2],255;0
    jmp xa
xe:
    mov byte [esi],255
    mov byte [esi+1],255
    mov byte [esi+2],255
xa:
    add esi,3
    cmp esi,edx
    jnae norx
    jmp red
nor1:

    jmp still

fileinfo:
    dd 0
    dd 0
    dd 1
    dd MHC_END
    dd out_image_area ;16384
string:
times 256 db 0
;times 256 db 0
;times 256 db 0

;filename db    "IMAGE   MIA"
;mhcfilename db "IMAGE   MHI"

soi dd 0
pxs dd 0
pys dd 0

out_image:
    mov eax,[prcinfo.x_size]
    sub eax,21
    cmp eax,[img_xsize] ;640
    jna no_x
    mov eax,[img_xsize] ;640
no_x:
    mov [pxs],eax
    mov eax,[prcinfo.y_size]
    sub eax,68
    cmp eax,[img_ysize] ;480
    jna no_y
    mov eax,[img_ysize] ;480
no_y:
    mov [pys],eax

    mov eax,[pxs]
    cmp eax,[img_xsize] ;640
    jne trans
    mov eax,[pys]
    cmp eax,[img_ysize] ;480
    jne trans

    mov eax,7
    mov ebx,image_area
    mov ecx,[pxs] ;640
    shl ecx,16
    add ecx,[pys] ;480
    mov edx,(5+6)*65536+(22+6+30) ;+30 Reserve for buttons
    int 0x40
    ret

trans:
    mov ebp,[pys];  180
    mov esi,image_area
    mov edi,out_image_area
loox:
    mov ecx,[pxs] ;200*3
    add ecx,[pxs]
    add ecx,[pxs]
    cld
    rep movsb
    mov eax,[img_xsize] ;640
    sub eax,[pxs]
    add esi,eax
    add esi,eax
    add esi,eax

    dec ebp
    jnz loox

    mov eax,7
    mov ebx,out_image_area
    mov ecx,[pxs]
    shl ecx,16
    add ecx,[pys]

    mov edx,(5+6)*65536+(22+6+30) ;+30 Reserve for buttons
    int 0x40
    ret


;set put to massive
puti:
    mov eax,edx
    mov ebx,ecx

setput:
    pushad
    cmp eax,[img_xsize] ;640
    jae nosetput
    cmp ebx,[img_ysize] ;480
    jae nosetput
    mov edi,ebx
    mov ebx,3  ;x*3
    mul ebx
    mov esi,eax
    mov eax,[img_xsize] ;640*3 ;Y*640*3
    add eax,[img_xsize]
    add eax,[img_xsize]
    mov ebx,edi
    mul ebx
    add eax,esi

    mov ebp,[img_xsize]
    shl ebp,4
    add ebp,16
    mov esi,ebp
    add esi,ebp
    add esi,ebp

    sub eax,esi   ;(16+640*16)*3

    mov esi,0
mlix:
    push eax

    mov edi,[matrix+esi*4]
mloo:
    shr edi,1
    jnc nosp
;    cmp eax,640*480*3 ;for detect out of screen
;    jae  nosp
    push eax
    mov  eax,[img_xsize]
    mul  [img_ysize]
    mov  ebx,eax
    add  ebx,eax
    add  ebx,eax
    pop  eax
    cmp  eax,ebx
    jae  nosp

    mov ebx,[color]
    mov byte [image_area+eax],bl   ;0x00ff
    shr ebx,8
    mov byte [image_area+eax+1],bl ;0x33
    shr ebx,8
    mov byte [image_area+eax+2],bl ;0x33
nosp:
    add eax,3
    cmp edi,0
    jne mloo

    pop eax
    add eax,[img_xsize]   ;640*3
    add eax,[img_xsize]
    add eax,[img_xsize]
    inc esi
    cmp esi,32
    jnae mlix

nosetput:
    popad
    ret

matrix:
times 32 dd 0

brushtable:
;standart put
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
times 12 dd 0

;put size 2
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000011000000000000000b
  dd 00000000000000011000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
times 12 dd 0

;put size 3
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000111000000000000000b
  dd 00000000000000111000000000000000b
  dd 00000000000000111000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
times 12 dd 0

;put size 4
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000111000000000000000b
  dd 00000000000001111100000000000000b
  dd 00000000000001111100000000000000b
  dd 00000000000001111100000000000000b
  dd 00000000000000111000000000000000b
  dd 00000000000000000000000000000000b
times 12 dd 0


;put slash ld-ru
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000000000010000000000000b
  dd 00000000000000000100000000000000b
  dd 00000000000000001000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000100000000000000000b
  dd 00000000000001000000000000000000b
  dd 00000000000010000000000000000000b
times 12 dd 0

;put slash lu-rd
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000010000000000000000000b
  dd 00000000000001000000000000000000b
  dd 00000000000000100000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000001000000000000000b
  dd 00000000000000000100000000000000b
  dd 00000000000000000010000000000000b
times 12 dd 0

;pricel
times 8 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000011111110000000000000b
  dd 00000000000010010010000000000000b
  dd 00000000000010000010000000000000b
  dd 00000000111111010111111000000000b
  dd 00000000000010000010000000000000b
  dd 00000000000010010010000000000000b
  dd 00000000000011111110000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000010000000000000000b
times 8 dd 0

;krest
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000010000010000000000000b
  dd 00000000000001000100000000000000b
  dd 00000000000000101000000000000000b
  dd 00000000000000010000000000000000b
  dd 00000000000000101000000000000000b
  dd 00000000000001000100000000000000b
  dd 00000000000010000010000000000000b
times 12 dd 0

;krest
times 12 dd 0
  dd 00000000000000000000000000000000b
  dd 00000000000010000010000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000000000000000000000000b
  dd 00000000000010000010000000000000b
times 12 dd 0


;smile
times 8 dd 0
  dd 00000111110000000000000b
  dd 00011000001100000000000b
  dd 00100000000010000000000b
  dd 01000000000001000000000b
  dd 01000000000001000000000b
  dd 10001100011000100000000b
  dd 10001100011000100000000b
  dd 10000000000000100000000b
  dd 10000000000000100000000b
  dd 10010000000100100000000b
  dd 01001000001001000000000b
  dd 01000111110001000000000b
  dd 00100000000010000000000b
  dd 00011000001100000000000b
  dd 00000111110000000000000b
  dd 00000000000000000000000000000000b
times 8 dd 0

;round
  dd 00000000000111111111100000000000b
  dd 00000000011000000000011000000000b
  dd 00000001100000000000000110000000b
  dd 00000010000000000000000001000000b
  dd 00000100000000000000000000100000b
  dd 00001000000000000000000000010000b
  dd 00010000000000000000000000001000b
  dd 00100000000000000000000000000100b
  dd 00100000000000000000000000000100b
  dd 01000000000000000000000000000010b
  dd 01000000000000000000000000000010b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 10000000000000000000000000000001b
  dd 01000000000000000000000000000010b
  dd 01000000000000000000000000000010b
  dd 00100000000000000000000000000100b
  dd 00100000000000000000000000000100b
  dd 00010000000000000000000000001000b
  dd 00001000000000000000000000010000b
  dd 00000100000000000000000000100000b
  dd 00000010000000000000000001000000b
  dd 00000001100000000000000110000000b
  dd 00000000011000000000011000000000b
  dd 00000000000111111111100000000000b


;Draw line procedure
line:
	jmp n
previous_X	dw	-1
previous_Y	dw	-1
X_increment	dw	-1
Y_increment	dw	-1
n:
 push ax
 push bx
 push cx
 push dx
 pop cx ;yed
	pop dx ;xed
	pop [previous_Y]
	pop [previous_X]

	mov ax,cx
	sub ax,[previous_X]
	jns dx_pos
	neg ax
	mov [X_increment],1
	jmp dx_neg
dx_pos: mov [X_increment],-1
dx_neg: mov bx,dx
	sub bx,[previous_Y]
	jns dy_pos
	neg bx
	mov [Y_increment],1
	jmp dy_neg
dy_pos: mov [Y_increment],-1
dy_neg: shl ax,1
	shl bx,1
	pusha
	call puti
	popa
	cmp ax,bx
	jna dx_le_dy
	mov di,ax
	shr di,1
	neg di
	add di,bx
cycle:
	cmp cx,[previous_X]
	je exit_bres
	cmp di,0
	jl fractlt0
	add dx,[Y_increment]
	sub di,ax
fractlt0:
	add cx,[X_increment]
	add di,bx
	pusha
	call puti
	popa
	jmp cycle
dx_le_dy:
	mov di,bx
	shr di,1
	neg di
	add di,ax
cycle2:
	cmp dx,[previous_Y]
	je exit_bres
	cmp di,0
	jl fractlt02
	add cx,[X_increment]
	sub di,bx
fractlt02:
	add dx,[Y_increment]
	add di,ax
	pusha
	call puti
	popa
	jmp cycle2
exit_bres:
 ret


draw_window:
    startwd
    window 40,40,(586+8),(380+24),window_Skinned+0x00ffffff
    label 12,8,'2D EXAMPLE: ASCL XPAINT',cl_White+font_Big

;   Get proc info
    mov eax,9
    mov ebx,prcinfo
    mov ecx,-1
    int 0x40

    mov eax,[prcinfo.x_size]
    mov ebp,[prcinfo.y_size]
    sub eax,5+4
    sub ebp,22+4

    drawfbox 5,22,eax,ebp,0x00e0f0f4
;    call out_image

; draw resize buttons
    mov eax,8
    mov ebx,(341)*65536+9
    mov ecx,(22+6+6)*65536+6
    mov edx,6 ;start with 6
    mov esi,0x00aaaaaa
    int 0x40
    inc edx
    add ebx,15*65536
    int 0x40
    mov ebx,(340+10)*65536+6
    mov ecx,(22+6-2)*65536+8
    inc edx
    mov esi,0x00aaaaaa
    int 0x40
    inc edx
    add ecx,14*65536
    int 0x40

;Draw now and back color
    drawfbox 20,32,20,15,[backcolor]
    drawfbox 12,28,20,15,[color]


;Draw buttons color set
    mov eax,8
    mov ebx,(40+6)*65536+8
    mov ecx,(22+6)*65536+8
    mov edx,10 ;start with 10
    mov edi,0
nextbut:
    mov esi,dword [colorbuttable+edi*4]
    int 0x40
    add ecx,(10)*65536
    inc edx
    inc edi
    mov esi,dword [colorbuttable+edi*4]
    int 0x40
    sub ecx,(10)*65536
    add ebx,(10)*65536
    inc edx
    inc edi
    cmp edi,32   ;64 buttons, 2 string of 32 buttons
    jnae nextbut

;Save as not ziped image button
    mov eax,8
    mov ebx,(365+6)*65536+50
    mov ecx,(22+6)*65536+8
    mov edx,80 ;ID
    mov esi,cl_Grey
    int 0x40

label (365+7),(22+7),"SAVE MIA",cl_White

;Open not zipped image button
    mov eax,8
    mov ebx,(365+6)*65536+50
    mov ecx,(36+6)*65536+8
    mov edx,81 ;ID
    mov esi,cl_Grey
    int 0x40

label (365+7),(36+7),"OPEN MIA",cl_White

;Save as MHC ziped image button
    mov eax,8
    mov ebx,(420+6)*65536+50
    mov ecx,(22+6)*65536+8
    mov edx,82 ;ID
    mov esi,cl_Grey
    int 0x40

label (420+7),(22+7),"SAVE MHI",cl_White

;Open MHC zipped image button
    mov eax,8
    mov ebx,(420+6)*65536+50
    mov ecx,(36+6)*65536+8
    mov edx,83 ;ID
    mov esi,cl_Grey
    int 0x40

label (420+7),(36+7),"OPEN MHI",cl_White

;Save as Bitmap image button
    mov eax,8
    mov ebx,(475+6)*65536+50
    mov ecx,(22+6)*65536+8
    mov edx,84 ;ID
    mov esi,cl_Grey
    int 0x40

label (475+7),(22+7),"SAVE NOT",cl_White

;Open Bitmap image button
    mov eax,8
    mov ebx,(475+6)*65536+50
    mov ecx,(36+6)*65536+8
    mov edx,85 ;ID
    mov esi,cl_Grey
    int 0x40

label (475+7),(36+7),"OPEN BMP",cl_White

;Save as GIF image button
    mov eax,8
    mov ebx,(530+6)*65536+50
    mov ecx,(22+6)*65536+8
    mov edx,86 ;ID
    mov esi,cl_Grey
    int 0x40

label (530+7),(22+7),"SAVE NOT",cl_White

;Open GIF image button
    mov eax,8
    mov ebx,(530+6)*65536+50
    mov ecx,(36+6)*65536+8
    mov edx,87 ;ID
    mov esi,cl_Grey
    int 0x40

label (530+7),(36+7),"OPEN GIF",cl_White

;Draw filter buttons
    mov eax,8
    mov ebx,(590+6)*65536+6
    mov ecx,(22+6)*65536+6
    mov edx,100 ;start with 100
    mov edi,0
nextfbut:
    mov esi,0x00aa0000
    int 0x40
    add ecx,(8)*65536
    inc edx
    inc edi
    mov esi,0x0000aa00
    int 0x40
    add ecx,(8)*65536
    inc edx
    inc edi
    mov esi,0x000000aa
    int 0x40
    sub ecx,(8*2)*65536
    add ebx,(8)*65536
    inc edx
    inc edi
    cmp edi,8*3  ;24 buttons, 3 string of 8 buttons
    jnae nextfbut


    endwd
    call out_image
    ret


; Get from MHC Archiver by Nikita Lesnikov
; ======== compression/decompression engine ========

 compress:   ; File compression

 compress_filefound:

 jmp lzp_compress                    ; compress with order-2 LZP
 compress_dumpdata:

 ret

 decompress: ; File decompression

 decompress_filefound:

 cmp byte [ofile],0                     ; Invalid method!
 jz  right_method
 mov edx,0 ;0 size
 ret

 right_method:

 jmp lzp_decompress
 decompress_dumpdata:

 ret

 fill_filebufs:             ; Fill filebufs with garbage to simplify matching
 pusha
 cld
 mov eax,0xF7D9A03F         ; <- "magic number" :) just garbage...
 mov ecx,2000000/2 ;4
 mov edi,ifile
 rep stosd
 popa
 ret

; ==== algorithms section ====

; Method 0: LZP compression algorithm

 lzp_compress:           ; EDX - how much bytes to dump

 cld                     ; clear direction flag

 mov esi,ifile           ; init pointers
 mov edi,ofile

 push eax                ; write header: ID0+4bfilesize => total 5 bytes
 xor eax,eax
 stosb
 pop eax
 stosd

 pusha                   ; fill hash table
 mov eax,ifile
 mov edi,hashtable
 mov ecx,65536*2 ;*1
 rep stosd
 popa

 add eax,esi              ; calculate endpointer
 mov dword [endpointer],eax

 movsw                    ; copy three bytes
 movsb

 mov dword [controlp],edi
 inc edi

 mov byte [controld],0
 mov byte [controlb],0

 c_loop:
 cmp dword [endpointer],esi  ; check end of file
 ja  c_loop_ok
 jmp finish_c_loop
 c_loop_ok:

 call chash
 call compare
 jz   two_match_c

 lodsb
 mov byte [literal],al
 call chash
 call compare
 jz   lit_match_c

 mov  al,0
 call putbit
 mov  al,byte [literal]
 stosb
 movsb
 jmp  end_c_loop

 lit_match_c:
 mov al,1
 call putbit
 mov al,0
 call putbit
 mov al,byte [literal]
 stosb
 jmp encode_match

 two_match_c:
 mov al,1
 call putbit
 call putbit

 encode_match:
 call incpos
 call compare
 jz one_c
 mov al,0
 call putbit
 jmp end_c_loop
 one_c:

 call incpos
 mov  al,1
 call putbit

 call compare
 jnz ec1
 call incpos
 call compare
 jnz ec2
 call incpos
 call compare
 jnz ec3
 call incpos
 mov al,1
 call putbit
 call putbit
 call compare
 jnz ec4
 call incpos
 call compare
 jnz ec5
 call incpos
 call compare
 jnz ec6
 call incpos
 call compare
 jnz ec7
 call incpos
 call compare
 jnz ec8
 call incpos
 call compare
 jnz ec9
 call incpos
 call compare
 jnz ec10
 call incpos

 mov al,1
 call putbit
 call putbit
 call putbit
 xor  ecx,ecx

 match_loop_c:
 cmp  esi,dword [endpointer]
 jae   out_match_loop_c
 call compare
 jnz  out_match_loop_c
 inc  ecx
 call incpos
 jmp  match_loop_c
 out_match_loop_c:

 mov al,0xFF
 out_lg:
 cmp ecx,255
 jb  out_lg_out
 stosb
 sub ecx,255
 jmp out_lg
 out_lg_out:
 mov al,cl
 stosb
 jmp end_c_loop

 ec10:
 mov al,1
 call putbit
 call putbit
 mov al,0
 call putbit
 jmp end_c_loop

 ec9:
 mov al,1
 call putbit
 mov al,0
 call putbit
 mov al,1
 call putbit
 jmp end_c_loop

 ec8:
 mov al,1
 call putbit
 mov al,0
 call putbit
 call putbit
 jmp end_c_loop

 ec7:
 mov al,0
 call putbit
 mov al,1
 call putbit
 call putbit
 jmp end_c_loop

 ec6:
 mov al,0
 call putbit
 mov al,1
 call putbit
 mov al,0
 call putbit
 jmp end_c_loop

 ec5:
 mov al,0
 call putbit
 call putbit
 mov al,1
 call putbit
 jmp end_c_loop

 ec4:
 mov al,0
 call putbit
 call putbit
 call putbit
 jmp end_c_loop

 ec3:
 mov al,1
 call putbit
 mov al,0
 call putbit
 jmp end_c_loop

 ec2:
 mov al,0
 call putbit
 mov al,1
 call putbit
 jmp end_c_loop

 ec1:
 mov al,0
 call putbit
 call putbit

 end_c_loop:
 jmp c_loop

 finish_c_loop:

 mov eax,dword [controlp] ; store last tagbyte
 mov bl,byte [controld]
 mov [eax], byte bl

 sub edi,ofile ; calculate dump size
 mov edx,edi

 jmp compress_dumpdata

; LZP decompression algorithm

 lzp_decompress:                        ; EDX - how much bytes to dump

 cld

 mov edi,ifile
 mov esi,ofile+1

 pusha                   ; fill hash table
 mov eax,ifile
 mov edi,hashtable
 mov ecx,65536*2 ;*1
 rep stosd
 popa

 lodsd

 mov ebx,edi
 add ebx,eax
 mov dword [endpointer],ebx

 movsw
 movsb

 lodsb
 mov byte [controld],al
 mov byte [controlb],0

 d_loop:
 cmp dword [endpointer],edi
 ja d_loop_ok
 jmp finish_d_loop
 d_loop_ok:

 call getbit
 cmp  al,0
 jnz  match_d
 call dhash
 movsb
 call dhash
 movsb
 jmp end_d_loop

 match_d:

 call getbit
 cmp  al,0
 jnz  no_literal_before_match
 call dhash
 movsb
 no_literal_before_match:

 call dhash
 mov ecx,1
 call copymatch

 call getbit
 cmp  al,0
 jz   end_d_loop
 mov  ecx,1
 call copymatch
 call getbit
 cmp  al,0
 jz   dc2
 mov  ecx,2
 call copymatch
 call getbit
 cmp  al,0
 jz   end_d_loop
 mov  ecx,1
 call copymatch
 call getbit
 cmp  al,0
 jz   dc4
 mov  ecx,4
 call copymatch
 call getbit
 cmp  al,0
 jz   dc5
 call getbit
 cmp  al,0
 jz   dc6
 mov  ecx,3
 call copymatch

 do:
 lodsb
 xor  ecx,ecx
 mov  cl,al
 call copymatch
 cmp  al,0xFF
 jnz  end_do
 jmp do
 end_do:
 jmp end_d_loop

 dc6:
 mov ecx,2
 call copymatch
 jmp  end_d_loop

 dc5:
 call getbit
 cmp  al,0
 jz   ndc5
 mov  ecx,1
 call copymatch
 ndc5:
 jmp  end_d_loop

 dc4:
 call getbit
 cmp  al,0
 jz   ndc4
 call getbit
 mov  ecx,3
 cmp  al,1
 jz   ndcc4
 dec  ecx
 ndcc4:
 call copymatch
 jmp  end_d_loop
 ndc4:
 call getbit
 cmp  al,0
 jz   ndccc4
 mov  ecx,1
 call copymatch
 ndccc4:
 jmp  end_d_loop

 dc2:
 call getbit
 cmp al,0
 jz  ndc2
 mov ecx,1
 call copymatch
 ndc2:

 end_d_loop:
 jmp d_loop
 finish_d_loop:

 mov edx, dword [ofile+1]

 jmp decompress_dumpdata

; LZP subroutines

 putbit:                  ; bit -> byte tag, AL holds bit for output
 pusha
 mov cl,byte [controlb]
 shl al,cl
 mov bl,byte [controld]
 or  bl,al
 mov byte [controld],bl
 inc cl
 cmp cl,8
 jnz just_increment
 mov byte [controlb],0
 mov byte [controld],0
 push edi
 mov  edi, dword [controlp]
 mov  al,bl
 stosb
 pop  edi
 mov dword [controlp],edi
 popa
 inc edi
 ret
 just_increment:
 mov byte [controlb],cl
 popa
 ret

 getbit:                       ; tag byte -> bit, AL holds input
 push ecx
 mov al,byte [controld]
 mov cl,byte [controlb]
 shr al,cl
 and al,1
 inc cl
 cmp cl,8
 jnz just_increment_d
 mov byte [controlb],0
 push eax
 lodsb
 mov byte [controld],al
 pop  eax
 pop  ecx
 ret
 just_increment_d:
 mov byte [controlb],cl
 pop ecx
 ret

 chash:                        ; calculate hash -> mp -> fill position
 pusha
 xor  eax,eax
 mov  al, byte [esi-1]
 mov  ah, byte [esi-2]
 shl  eax,2
 add  eax,hashtable
 mov  edx,dword [eax]
 mov  dword [mp],edx
 mov  dword [eax],esi
 popa
 ret

 dhash:                        ; calculate hash -> mp -> fill position
 pusha
 xor  eax,eax
 mov  al, byte [edi-1]
 mov  ah, byte [edi-2]
 shl  eax,2
 add  eax,hashtable
 mov  edx,dword [eax]
 mov  dword [mp],edx
 mov  dword [eax],edi
 popa
 ret

 copymatch:                    ; ECX bytes from [mp] to [rp]
 push esi
 mov  esi,dword [mp]
 rep  movsb
 mov  dword [mp],esi
 pop  esi
 ret

 compare:                      ; compare [mp] with [cpos]
 push edi
 push esi
 mov  edi,dword [mp]
 cmpsb
 pop  esi
 pop  edi
 ret

 incpos:
 inc  dword [mp]
 inc  esi
 ret


; LZP algorithm data

 endpointer     dd      0
 controlp       dd      0
 controlb       db      0
 controld       db      0
 mp  dd 0
 literal        db      0

; the end... - Nikita Lesnikov (nlo_one)



; DATA AREA
newline dd 0
xt dd 100
yt dd 100
counter dd 0
tsoi dd 0
view dd 0
brush dd 0

color     dd 0x00000000
backcolor dd 0xffffff

colortable:
      dd 0x00000000  ;black
      dd 0x00FFFFFF  ;white
      dd 0x00808080  ;dark-gray
      dd 0x00C0C0C0  ;gray
      dd 0x00000080  ;dark-blue
      dd 0x000000ff  ;blue
      dd 0x00400080  ;dark-violet
      dd 0x008000ff  ;violet
      dd 0x00800080  ;dark-pink
      dd 0x00ff00ff  ;pink
      dd 0x00800040  ;
      dd 0x00ff0080  ;
      dd 0x00800000  ;brown
      dd 0x00ff0000  ;red
      dd 0x00802000  ;
      dd 0x00ff4000  ;dark-orange
      dd 0x00804000  ;
      dd 0x00ff8000  ;orange
      dd 0x00804000  ;
      dd 0x00ff8000  ;orange
      dd 0x00808000  ;oliva
      dd 0x00ffff00  ;yellow
      dd 0x00608000  ;
      dd 0x00C0ff00  ;
      dd 0x00408000  ;green
      dd 0x0080ff00  ;lime
      dd 0x00008000  ;
      dd 0x0000ff00  ;
      dd 0x00008040  ;dark-salat
      dd 0x0000ff80  ;salat
      dd 0x00008080  ;dark-water
      dd 0x0000ffff  ;water

colorbuttable:
      dd 0x001e1e1e  ;black
      dd 0x00dedede  ;white
      dd 0x00808080  ;dark-gray
      dd 0x00C0C0C0  ;gray
      dd 0x001e1e80  ;dark-blue
      dd 0x001e1ede  ;blue
      dd 0x00401e80  ;dark-violet
      dd 0x00801ede  ;violet
      dd 0x00801e80  ;dark-pink
      dd 0x00de1ede  ;pink
      dd 0x00801e40  ;
      dd 0x00de1e80  ;
      dd 0x00801e1e  ;brown
      dd 0x00de1e1e  ;red
      dd 0x0080201e  ;
      dd 0x00de401e  ;dark-orange
      dd 0x0080401e  ;
      dd 0x00de801e  ;orange
      dd 0x0080401e  ;
      dd 0x00de801e  ;orange
      dd 0x0080801e  ;oliva
      dd 0x00dede1e  ;yellow
      dd 0x0060801e  ;
      dd 0x00C0de1e  ;
      dd 0x0040801e  ;green
      dd 0x0080de1e  ;lime
      dd 0x001e801e  ;
      dd 0x001ede1e  ;
      dd 0x001e8040  ;dark-salat
      dd 0x001ede80  ;salat
      dd 0x00008080  ;dark-water
      dd 0x0000dede  ;water



prcinfo process_information

;IM_END:
image:
img_xsize dd 200
img_ysize dd 180
IM_END:
image_area:
;rb 1024*768*3

out_image_area = image_area+(1024*768*3)
;rb 640*480*3

MHC_END = image_area+(1024*768*3)*2
I_END = MHC_END+6000000  ;6 megs for work mhc algorythm
