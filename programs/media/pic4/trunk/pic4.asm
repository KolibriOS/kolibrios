;
;    BACKGROUND SET  - Compile with fasm
;
  use32
  org     0x0
  db      'MENUET01'    ; 8 byte id
  dd      0x01          ; version
  dd      START         ; program start
  dd      I_END         ; image size
  dd      0x80000      ; reguired amount of memory
  dd      0x80000       ; stack pointer
  dd      I_Param,0

  include 'lang.inc'
  include 'macros.inc'

START:

    call check_parameters
    call draw_window

    call load_texture
    call draw_image

still:

    mov  eax,10                 ; wait here for event
    int  0x40
    cmp  eax,1
    jz   red
    cmp  eax,2
    jz   key
    cmp  eax,3
    jz   button
    jmp  still

  key:
    mov  eax,2
    int  0x40
    jmp  still

  red:
    call draw_window
    jmp  still

  button:
    mov  eax,17
    int  0x40

    shr  eax,8
    and  eax,255

    cmp  eax,101                ; tiled
    jne  no101
    mov  eax,15
    mov  ebx,4
    mov  ecx,1
    int  0x40
    mov  eax,15
    mov  ebx,3
    int  0x40
    jmp  still
  no101:

    cmp  eax,102               ; stretch
    jne  no102
    mov  eax,15
    mov  ebx,4
    mov  ecx,2
    int  0x40
    mov  eax,15
    mov  ebx,3
    int  0x40
    jmp  still
  no102:

    cmp  eax,1           ; end program
    jnz  noproend
    or   eax,-1
    int  0x40
  noproend:

    cmp  eax,11
    jz   bg
    cmp  eax,12
    jz   bg
    cmp  eax,13
    jz   bg

    cmp  eax,121
    jb   no_bg_select
    cmp  eax,133
    jg   no_bg_select
    sub  eax,121
    shl  eax,2
    add  eax,arrays
    mov  eax,[eax]
    mov  [usearray],eax
    call load_texture
    call draw_image
    jmp  still
  no_bg_select:

    cmp  eax,14+20
    jge  bg4

    jmp  bg2


check_parameters:

    cmp  [I_Param],dword 'BOOT'
    je   @f
    ret
  @@:

    call load_texture

    mov  eax,15
    mov  ebx,1
    mov  ecx,256
    mov  edx,256
    int  0x40

    mov  eax,15
    mov  ebx,5
    mov  ecx,0x40000 ; <<< 0x40000 for blue, 0x40000+1 for red,
                       ; <<< 0x40000+2 for green background at boot
    mov  edx,0
    mov  esi,256*3*256
    int  0x40

    mov  eax,15
    mov  ebx,4
    mov  ecx,2
    int  0x40

    mov  eax,15
    mov  ebx,3
    int  0x40

    mov  eax,-1
    int  0x40



set_picture:

    mov  eax,image+99-3*16
    mov  ebx,0x40000+255*3+255*3*256
  newpix:
    mov  ecx,[eax]
    mov  [ebx],cx
    shr  ecx,16
    mov  [ebx+2],cl
    add  eax,3
    sub  ebx,3
    cmp  ebx,0x40002
    jge  newpix

    ret


load_texture:

    call  gentexture
    call  set_picture

    ret


; set background

bg:

    mov  edi,0x40000

    cmp  eax,12
    jnz  bb1
    mov  edi,0x40000+1
  bb1:
    cmp  eax,13
    jnz  bb2
    mov  edi,0x40000+2
  bb2:

    mov  eax,15
    mov  ebx,1
    mov  ecx,256
    mov  edx,256
    int  0x40

    mov  eax,15
    mov  ebx,5
    mov  ecx,edi
    mov  edx,0
    mov  esi,256*256*3
    int  0x40

    mov  eax,15
    mov  ebx,3
    int  0x40

    jmp  still


; colored background

bg2:

    push eax

    mov  eax,15
    mov  ebx,1
    mov  ecx,8
    mov  edx,8
    int  0x40

    mov  eax,[esp]

    sub  eax,14
    shl  eax,2

    mov  edx,[colors+eax]

    mov  esi,32*32*4
    mov  edi,0
    mov  ecx,0
  dbl2:
    mov  eax,15
    mov  ebx,2
    int  0x40
    add  ecx,3
    inc  edi
    cmp  edi,esi
    jb   dbl2


    mov  eax,15
    mov  ebx,3
    int  0x40

    jmp  still


; shaped background

bg4:

    sub  eax,14+20
    shl  eax,3
    add  eax,shape
    mov  ecx,[eax+0]
    mov  edx,[eax+4]

    mov  eax,15
    mov  ebx,1
    int  0x40

    mov  eax,15
    mov  ebx,3
    int  0x40

    jmp  still


; *********************************************
; ******* CELLULAR TEXTURE GENERATION *********
; **** by Cesare Castiglia (dixan/sk/mfx) *****
; ********* dixan@spinningkids.org   **********
; *********************************************
; * the algorythm is kinda simple. the color  *
; * component for every pixel is evaluated    *
; * according to the squared distance from    *
; * the closest point in 'ptarray'.           *
; *********************************************

gentexture:

  mov ecx,0          ; ycounter
  mov edi,0          ; pixel counter

  mov ebp,[usearray]

 ylup:
    mov ebx,0

; call precalcbar

 xlup:
  push edi
  mov edi, 0
  mov esi, 512000000           ; abnormous initial value :)

 pixlup:
   push esi
;   add edi,4
   mov eax,ebx                 ; evaluate first distance
   sub eax, [ebp+edi]          ; x-x1
   call wrappit
   imul eax
   mov esi, eax                ; (x-x1)^2
   mov eax, ecx
   add edi,4
   sub eax, [ebp+edi]          ; y-y1
   call wrappit
   imul eax                    ; (y-y1)^2
   add eax,esi                 ; (x-x1)^2+(y-y1)^2
   pop esi

   cmp esi,eax
   jb  ok                      ; compare and take the smaller one
   mov esi,eax

  ok:
   add edi,4
   cmp [ebp+edi],dword 777
   jne pixlup

   mov eax,esi                 ; now evaluate color...

;   cmp eax,255*24
;   jbe ok2
;   imul eax,12
; ok2:

   mov edi,24            ; 50 = max shaded distance
   idiv edi

   pop edi
   mov [image+51+edi],eax
   add edi,3

  add ebx,1              ; bounce x loop
  cmp ebx,256            ; xsize
  jne xlup

  add ecx,1
  cmp ecx,256            ; ysize
  jne ylup

  ret

wrappit:
  cmp eax,0              ; this makes the texture wrap
  jg noabs
  neg eax
  noabs:
  cmp eax,128
  jb nowrap
  neg eax
  add eax,256
  nowrap:
  ret

;precalcbar:
;  pusha
;  mov eax,1
;  mov ebx,ecx
;  add ebx,18
;  mov ecx,44
;  mov edx,0x00000060
;     bar:
;     add ecx,2
;     add edx,0x00020100
;     int 0x40
;     cmp ecx,298
;     jb bar
;  popa
;  ret

; *********************************************
; ******* WINDOW DEFINITIONS AND DRAW *********
; *********************************************


draw_image:

    mov  eax,7
    mov  ebx,0x40000
    mov  ecx,256*65536+255
    mov  edx,19*65536+65;55
    int  0x40

    ret


y_add  equ  44 ; 30
y_s    equ  13

y_add2 equ  340 ;325
set    equ  0 ;15

button_color     equ  0A0A0A0h ; 207090 306090h
wnd_color        equ  3B0B0B0h ; 34090B0h


draw_window:

    mov eax, 12                   ; tell os about draw
    mov ebx, 1
    int 0x40

    mov eax, 0                    ; define and draw window
    mov ebx, 220*65536+293
    mov ecx, 50*65536+408
    mov edx, wnd_color
    int 0x40

    call draw_image

    mov  eax,8                     ; Blue button
    mov  ebx,(set+195+27)*65536+17
    mov  ecx,y_add*65536+y_s
    mov  edx,11
    mov  esi,0x004444cc
    int  0x40
    mov  eax,8                     ; Red button
    mov  ebx,(set+213+27)*65536+17
    mov  edx,12
    mov  esi,0x00cc4444
    int  0x40
    mov  eax,8                     ; Green button
    mov  ebx,(set+258)*65536+17
    mov  edx,13
    mov  esi,0x0044cc44
    int  0x40

    mov  eax, 8                     ; tiled
    mov  ebx, 96*65536+63
    mov  ecx, y_add*65536+y_s
    mov  edx, 101
    mov  esi, button_color
    int  0x40

    mov  eax, 8                     ; stretch
    mov  ebx, 160*65536+61
    mov  edx, 102
    int  0x40

    mov  eax, 4                    ; window header
    mov  ebx, 8*65536+8
    mov  ecx, 0x10ffffff
    mov  edx, header
    mov  esi, header.size
    int  0x40

    mov  ebx, 220*65536+30
    mov  ecx, 0
    mov  edx, apply_text
    mov  esi, apply_text.size
    int  0x40

    mov  ebx, 19*65536+326
    mov  edx, image_buttons_text
    mov  esi, image_buttons_text.size
    int  0x40

    mov  ebx, 19*65536+(y_add2+27)
    mov  edx, simple_text
    mov  esi, simple_text.size
    int  0x40

    mov  ecx, (y_add2)*65536+20
    mov  ebx, (18)*65536+25
    mov  edx, 121
    mov  esi, button_color
    mov  edi, 9
    mov  eax, 8
  @@:
    int  0x40
    add  ebx, 29*65536
    inc  edx
    dec  edi
    jnz  @b


    mov  edx, 34+4
    mov  edi, 4
    mov  eax, 8
    mov  ebx, 18*65536+18
    mov  ecx, y_add*65536+y_s
  @@:
    int  0x40
    inc  edx
    add  ebx, 19*65536
    dec  edi
    jnz  @b


    ;-----------------------
    mov  edx,14                    ; button number
    mov  ebx,(18)*65536+17         ; button start x & size
    mov  ecx,(y_add2+40)*65536+14          ; button start y & size

  newcb:
    mov  esi,[(edx-14)*4+colors]

    mov  eax,8
    int  0x40

    inc  edx
    add  ebx,20*65536
    add  esi,5*256*256

    cmp  edx,27
    jnz  newcb
    ;-----------------------

    mov  eax, 4
    mov  ebx, 8*65536+4+y_add
    mov  ecx, 0
    mov  edx, la2
    mov  esi, la2.size
    int  0x40

    mov  eax,12
    mov  ebx,2
    int  0x40

    ret



; DATA SECTION

lsz header,\
    ru, "Генератор фона рабочего стола",\
    en, "BACKGROUND"

lsz apply_text,\
    ru, "Применить:",\
    en, "Apply:"

lsz image_buttons_text,\
    ru, "Выберите образец:",\
    en, "Select pattern:"

lsz simple_text,\
    ru, "Одноцветный фон:",\
    en, "Single-color background:"

lsz la2,\
    ru, "                ЗАМОСТИТЬ РАСТЯНУТЬ",\
    en, "                 TILED     STRETCH"


xx   db    'x'

colors:
    dd  0x770000
    dd  0x007700
    dd  0x000077
    dd  0x777700
    dd  0x770077
    dd  0x007777
    dd  0x777777
    dd  0x335577
    dd  0x775533
    dd  0x773355
    dd  0x553377
    dd  0x000000
    dd  0xcccccc


shape:

    dd  1024,64
    dd  1024,32
    dd  2048,32
    dd  4096,32

    dd  512,16
    dd  1024,16
    dd  2048,16
    dd  4096,16

    dd  64,32
    dd  64,16
    dd  32,32
    dd  8,8
    dd  16,16
    dd  64,64

usearray dd ptarray

arrays dd ptarray,ptarray2,ptarray3,ptarray4,ptarray5,ptarray6
        dd ptarray7,ptarray8,ptarray9

ptarray:

    dd  150,50
    dd  120,30
    dd  44,180
    dd  50,66
    dd  27,6
    dd  95,212
    dd  128,177
    dd  201,212
    dd  172,201
    dd  250,100
    dd  24,221
    dd  11,123
    dd  248,32
    dd  34,21
    dd  777     ; <- end of array

ptarray2:

    dd  0,0,50,50,100,100,150,150,200,200,250,250
    dd  50,150,150,50,200,100,100,200
    dd  777

ptarray3:

    dd  55,150,150,55,200,105,105,200
    dd  30,30,220,220
    dd  777

ptarray4:

    dd  196,0,196,64,196,128,196,196
    dd  64,32,64,96,64,150,64,228
    dd  777

ptarray5:

    dd  196,0,196,64,196,128,196,196
    dd  64,0,64,64,64,128,64,196
    dd  777

ptarray6:

    dd  49,49,128,50,210,50
    dd  50,128,128,128,210,128
    dd  50,210,128,210,210,210

    dd  777

ptarray7:

    dd  0,0
    dd  196,196,64,64
    dd  128,0
    dd  0,128
    dd  64,64,196,64
    dd  196,196,64,196
    dd  128,128

    dd  777

ptarray8:

    dd  0, 128
    dd  0, 128
    dd  128, 0
    dd  0, 128
    dd  128, 0
    dd  0, 128
    dd  128, 0
    dd  0, 128
    dd  128, 0
    dd  128, 128

    dd  777

ptarray9:


     dd  0,248,64,128,128,64,196,48,160,160,94,224,240,96,5,5,777



I_Param:

image:


I_END: