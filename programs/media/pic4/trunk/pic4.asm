;
;    BACKGROUND SET  - Compile with fasm
;
  use32
  org     0x0
  db      'MENUET01'    ; 8 byte id
  dd      0x01          ; version
  dd      START         ; program start
  dd      I_END         ; image size
  dd      mem           ; reguired amount of memory
  dd      mem           ; stack pointer
  dd      I_Param,0

  include 'lang.inc'
  include '..\..\..\macros.inc'
purge mov       ; decrease kpack'ed size

START:
    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    mcall

    cmp  dword [I_Param], 'BOOT'
    jz   OnBoot

    call draw_window

    call load_texture
    call draw_image


still:

    mov  eax,10                 ; wait here for event
    mcall

    dec  eax
    jz   red
    dec  eax
    jnz  button

  key:
    mov  al,2
    mcall
    jmp  still

  red:
    call draw_window
    jmp  still

  button:
    mov  al,17
    mcall

    shr  eax,8

    cmp  al,101                  ; tiled
    jne  no101
    mov  ecx,1
setbgrmode:
    mov  eax,15
    mov  ebx,4
    mcall
    dec  ebx
    mcall
    jmp  still
  no101:

    mov  ecx, 2
    cmp  al, 102
    jz   setbgrmode
  no102:

    cmp  al,1           ; end program
    jnz  no_end
    or   eax,-1
    mcall
  no_end:

    cmp  al,11
    jz   bg
    cmp  al,12
    jz   bg
    cmp  al,13
    jz   bg

    cmp  al,121
    jb   no_bg_select
    cmp  al,133
    ja   no_bg_select
    mov  eax,[arrays + (eax-121)*4]
    mov  [usearray],eax
    call load_texture
    call draw_image
    jmp  still
  no_bg_select:

    cmp  al,14+20
    jge  bg4

    jmp  bg2


OnBoot:

    call load_texture

    mov  eax,15
    mov  ebx,1
    mov  ecx,256
    mov  edx,256
    mcall

    mov  ebx,5
    mov  ecx,image + 0 ; <<< +0 for blue, +1 for red,
                       ; <<< +2 for green background at boot
    mov  edx,0
    mov  esi,256*3*256
    mcall

    dec  ebx
    mov  ecx,2
    mcall

    dec  ebx
    mcall

    mov  eax,-1
    mcall


; set background

bg:

    mov  edi,0x40000

    cmp  al,12
    jb   bb1
    inc  edi
  bb1:
    cmp  al,13
    jb   bb2
    inc  edi
  bb2:

    mov  eax,15
    mov  ebx,1
    mov  ecx,256
    mov  edx,256
    mcall

    mov  ebx,5
    mov  ecx,edi
    mov  edx,0
    mov  esi,256*256*3
    mcall

    mov  ebx,3
    mcall

    jmp  still


; colored background

bg2:
        mov     edi, eax
        mcall   15,4,1
        mcall   ,1,,1
        lea     ecx, [(edi-14)*3+fill]
        mcall   ,5,,0,3*1*1
        mcall   ,3

    jmp  still


; shaped background

bg4:

    shl  eax,3
    add  eax,shape - (14+20)*8
    mov  ecx,[eax+0]
    mov  edx,[eax+4]

    mov  eax,15
    mov  ebx,1
    mcall

    mov  ebx,3
    mcall

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

load_texture:
gentexture:

  xor ecx,ecx        ; ycounter
  xor edi,edi        ; pixel counter

  mov ebp,[usearray]

 ylup:
    xor ebx,ebx

 xlup:
  push edi
  xor edi, edi
  mov esi, 512000000           ; abnormous initial value :)

 pixlup:
   push esi
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

   mov edi,24            ; 50 = max shaded distance
   idiv edi

   pop edi
   mov [image+edi],eax
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

; *********************************************
; ******* WINDOW DEFINITIONS AND DRAW *********
; *********************************************


draw_image:

    mov  eax,7
    mov  ebx,image
    mov  ecx,256*65536+255
    mov  edx,14*65536+40;55
    mcall

    ret


y_add  equ  19 ; 30
y_s    equ  13

y_add2 equ  315 ;325
set    equ  0 ;15

draw_window:

    mov eax, 12                   ; tell os about draw
    mov ebx, 1
    mcall

    xor eax, eax                    ; define and draw window
    mov ebx, 220*65536+293
    mov ecx, 50*65536+408
    mov edx, [sc.work]
    or  edx, 0x34000000
    mov edi, title
    mcall

    call draw_image

    mov  eax,8                     ; Blue button
    mov  ebx,(set+190+27)*65536+17
    mov  ecx,y_add*65536+y_s
    mov  edx,11
    mov  esi,0x005555bb
    mcall
    ;mov  eax,8                     ; Red button
    mov  ebx,(set+208+27)*65536+17
    mov  edx,12
    mov  esi,0x00bb5555
    mcall
    ;mov  eax,8                     ; Green button
    mov  ebx,(set+253)*65536+17
    mov  edx,13
    mov  esi,0x0055bb55
    mcall

    ;mov  eax, 8                     ; tiled
    mov  ebx, 90*65536+63
    mov  ecx, y_add*65536+y_s
    mov  edx, 101
    mov  esi, [sc.work_button]
    mcall

    ;mov  eax, 8                     ; stretch
    mov  ebx, 154*65536+61
    mov  edx, 102
    mcall

    mov  eax, 4
    mov  ebx, 215*65536+5
    mov  ecx, [sc.work_text]
    mov  edx, apply_text
    mov  esi, apply_text.size
    mcall

    mov  ebx, 14*65536+301
    mov  edx, image_buttons_text
    mov  esi, image_buttons_text.size
    mcall

    mov  ebx, 14*65536+(y_add2+27)
    mov  edx, simple_text
    mov  esi, simple_text.size
    mcall

    mov  ecx, (y_add2)*65536+20
    mov  ebx, (13)*65536+25
    mov  edx, 121
    mov  esi, [sc.work_button]
    mov  edi, 9
    mov  eax, 8
  @@:
    mcall
    add  ebx, 29*65536
    inc  edx
    dec  edi
    jnz  @b


    mov  edx, 34+4
    mov  edi, 4
    ;mov  eax, 8
    mov  ebx, 13*65536+18
    mov  ecx, y_add*65536+y_s
  @@:
    mcall
    inc  edx
    add  ebx, 19*65536
    dec  edi
    jnz  @b


    ;-----------------------
    mov  eax,8
    mov  edx,14                            ; button number
    mov  ebx,(13)*65536+17                 ; button start x & size
    mov  ecx,(y_add2+40)*65536+14          ; button start y & size

  newcb:
    mov  esi,[(edx-14)*4+colors]

    mcall

    inc  edx
    add  ebx,20*65536

    cmp  edx,27
    jnz  newcb
    ;-----------------------

    mov  eax, 4
    mov  ebx, 94*65536+4+y_add
    mov  ecx, [sc.work_button_text]
    mov  edx, la2
    mov  esi, la2.size
    mcall

    mov  eax,12
    mov  ebx,2
    mcall

    ret



; DATA SECTION

if lang eq ru
    title db 'Генератор фона рабочего стола',0
else
    title db 'Background',0
end if

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
    ru, "ЗАМОСТИТЬ  РАСТЯНУТЬ",\
    en, "  TILED     STRETCH"


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
    dd  0x111111
    dd  0xcccccc

fill:
    db  0x00,0x00,0x77
    db  0x00,0x77,0x00
    db  0x77,0x00,0x00
    db  0x00,0x77,0x77
    db  0x77,0x00,0x77
    db  0x77,0x77,0x00
    db  0x77,0x77,0x77
    db  0x77,0x55,0x33
    db  0x33,0x55,0x77
    db  0x55,0x33,0x77
    db  0x77,0x33,0x55
    db  0x11,0x11,0x11
    db  0xcc,0xcc,0xcc

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

    dd  105,205
    dd  135,225
    dd  211,75
    dd  205,189
    dd  228,249
    dd  160,43
    dd  127,78
    dd  54,43
    dd  83,54
    dd  5,155
    dd  231,34
    dd  244,132
    dd  7,223
    dd  221,224
    dd  777     ; <- end of array

ptarray2:

    dd  255,255,205,205,155,155,105,105,55,55,5,5
    dd  205,105,105,205,55,155,155,55
    dd  777

ptarray3:

    dd  200,105,105,200,55,150,150,55
    dd  225,225,35,35
    dd  777

ptarray4:

    dd  59,255,59,191,59,127,59,59
    dd  191,223,191,159,191,105,191,27
    dd  777

ptarray5:

    dd  59,255,59,191,59,127,59,59
    dd  191,255,191,191,191,127,191,59
    dd  777

ptarray6:

    dd  206,206,127,205,45,205
    dd  205,127,127,127,45,127
    dd  205,45,127,45,45,45

    dd  777

ptarray7:

    dd  255,255
    dd  59,59,191,191
    dd  127,255
    dd  255,127
    dd  191,191,59,191
    dd  59,59,191,59
    dd  127,127

    dd  777

ptarray8:

    dd  255, 127
    dd  255, 127
    dd  127, 255
    dd  255, 127
    dd  127, 255
    dd  255, 127
    dd  127, 255
    dd  255, 127
    dd  127, 255
    dd  127, 127

    dd  777

ptarray9:


     dd  255,7,191,127,127,191,59,207,95,95,161,31,15,159,250,250,777


I_END:
sc system_colors

I_Param:

image:
        rb      256*256*3 + 4

align 100h
rb 100h ; it is enough for stack in this program
mem:
