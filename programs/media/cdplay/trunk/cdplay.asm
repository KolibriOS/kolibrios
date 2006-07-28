;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;    CD PLAYER - Compile with fasm
;

use32

                  org      0x0
                  db       'MENUET00'           ; 8 byte id
                  dd       38                   ; required os
                  dd       START                ; program start
                  dd       I_END                ; program image size
                  dd       0x1000               ; reguired amount of memory
                  dd       0x1000
                  dd       0x00000000           ; reserved=no extended header

include 'lang.inc'
include 'macros.inc'

START:

    call draw_window

still:

    mov  eax,10
    int  0x40

    cmp  eax,1
    jz   red
    cmp  eax,2
    jz   key
    cmp  eax,3
    jz   button
    jmp  still

  red:
    call draw_window
    jmp  still

  key:
    mov  eax,2
    int  0x40
    jmp  still

  button:
    mov  eax,17
    int  0x40

    cmp  ah,byte 41
    jnz  nostop
    call stop
    jmp  still
  nostop:

    cmp  ah,byte 42
    jnz  nogetinfo
    call getinfo
    jmp  still
  nogetinfo:

    pusha

    cmp  ah,100
    jnz  err

    jmp  noerr

  err:

    xor  ecx,ecx
    mov  cl,ah
    shl  ecx,3
    add  ecx,cdinfo
    add  ecx,1
    xor  ebx,ebx
    mov  ebx,[ecx]
    and  ebx,0x00ffffff
    mov  ecx,ebx

    mov  ebx,1
    mov  eax,24
    int  0x40

    cmp  eax,0
    je   noerr

  error:

    mov  eax,13
    mov  ebx,10*65536+215
    mov  ecx,115*65536+13
    mov  edx,0x0088aacc
    int  0x40

    mov  eax,dword 0x00000004      ; write text
    mov  ebx,12*65536+117
    mov  ecx,[tcolor]
    mov  edx,infotext2
    mov  esi,itl2-infotext2
    int  0x40

    mov  eax,5
    mov  ebx,200
    int  0x40

  noerr:

    popa

    shr  eax,8
    and  eax,255
    mov  ebx,0
    mov  ecx,10

    cmp  eax,100
    jnz  noend
    mov  eax,-1
    int  0x40
   noend:

   newc:

    cmp  eax,ecx
    jb   dri
    inc  ebx
    sub  eax,ecx

    jmp  newc


   dri:

    mov  ecx,48
    add  ebx,ecx
    mov  [infotext+0],bl
    add  eax,ecx
    mov  [infotext+1],al
    call drawinfo
    jmp  still


getinfo:

   mov  eax,24 ; get info
   mov  ebx,1
   mov  ecx,0x010100
   int  0x40

   cmp  eax,0
   jnz  gierror

   mov  eax,13
   mov  ebx,10*65536+215
   mov  ecx,115*65536+13
   mov  edx,0x0088aacc
   int  0x40
   mov  eax,dword 0x00000004      ; write text
   mov  ebx,12*65536+117
   mov  ecx,dword 0x00ffffff      ; 8b window nro - RR GG BB color
   mov  edx,infotext3             ; pointer to text beginning
   mov  esi,itl3-infotext3        ; text length
   int  0x40

   mov  eax,5
   mov  ebx,100*10
   int  0x40

   mov  eax,24 ; get info
   mov  ebx,2
   mov  ecx,cdinfo
   mov  edx,256
   int  0x40

   cmp  eax,0
   jz   gi1

  gierror:

   mov  eax,13
   mov  ebx,10*65536+215
   mov  ecx,115*65536+13
   mov  edx,0x0088aacc
   int  0x40

   mov  eax,dword 0x00000004      ; write text
   mov  ebx,12*65536+117
   mov  ecx,dword 0x00ffffff      ; 8b window nro - RR GG BB color
   mov  edx,infotext2             ; pointer to text beginning
   mov  esi,itl2-infotext2        ; text length
   int  0x40

   mov  eax,5
   mov  ebx,200
   int  0x40

 gi1:

   call drawinfo
   ret



stop:

   mov  eax,24 ; get info
   mov  ebx,3
   int  0x40

   ret



; info bar


drawinfo:


    ; end

    xor  eax,eax
    xor  ebx,ebx
    mov  ecx,10
    mov  al,[cdinfo+3]
    cld

  newco:

    cmp  eax,ecx
    jb   noco

    add  ebx,1
    sub  eax,ecx
    jmp  newco

  noco:

    add  al,48
    mov  [infotext+32],al

    add  bl,48
    mov  [infotext+31],bl


    ; start

    xor  eax,eax
    xor  ebx,ebx
    mov  al,[cdinfo+2]
    cld

  newco2:

    cmp  eax,ecx
    jb   noco2

    add  ebx,1
    sub  eax,ecx
    jmp  newco2

  noco2:

    add  al,48
    mov  [infotext+17],al

    add  bl,48
    mov  [infotext+16],bl

    mov  eax,13
    mov  ebx,10*65536+219
    mov  ecx,115*65536+13
    mov  edx,[col]
    sub  edx,0x101010
    int  0x40

    mov  eax,4                     ; write text
    mov  ebx,12*65536+117
    mov  ecx,dword 0x00ffffff      ; 8b window nro - RR GG BB color
    mov  edx,infotext              ; pointer to text beginning
    mov  esi,itl-infotext          ; text length
    int  0x40

    ret


draw_window:

    pusha

    mov  eax,12                    ; tell os about redraw
    mov  ebx,1
    int  0x40

    mov  eax,0                     ; define and draw window
    mov  ebx,170*65536+240
    mov  ecx,40*65536+135
    mov  edx,0x00b6aaff
    mov  esi,0x80b9aaff; bbee - 0x202020
    mov  edi,0x00b9aaff ;bbee
    mov  edx,[col]
    add  edx,0x00000000
    mov  esi,[col]
    add  esi,0x80000000
    mov  edi,[col]
    int  0x40

    mov  eax,4                          ; write text
    mov  ebx,8*65536+8
    mov  ecx,[tcolor]
    mov  edx,labelt
    mov  esi,labellen-labelt
    int  0x40

    mov  eax,8                          ; CLOSE BUTTON
    mov  ebx,(240-18)*65536+10
    mov  ecx,6  *65536+10
    mov  edx,100
    mov  esi,[col]
    sub  esi,0x302010
    int  0x40


    ; first row

    mov  eax,8                     ; button
    mov  edx,1                     ; button number
    mov  ebx,9*65536+21            ; button start x & size
    mov  ecx,30*65536+13           ; button start y & size
    mov  esi,[bcolor]              ; button color
   newbutton1:
    pusha
    int  0x40
    popa

    pusha
    mov  eax,dword 0x00000004
    and  ebx,65535*65536
    shr  ecx,16
    add  ebx,ecx
    add  ebx,6*65536+3
    mov  ecx,[tcolor]
    shl  edx,1
    add  edx,nro-2
    mov  esi,2
    int  0x40
    popa

    add  ebx,22*65536
    inc  edx
    cmp  edx,11
    jnz  newbutton1

    ; second row

    mov  eax,8                     ; button
    mov  edx,11                    ; button number
    mov  ebx,9*65536+21            ; button start x & size
    mov  ecx,50*65536+13           ; button start y & size
    mov  esi,[bcolor]              ; button color
   newbutton2:
    pusha
    int  0x40
    popa

    pusha
    mov  eax,dword 0x00000004
    and  ebx,65535*65536
    shr  ecx,16
    add  ebx,ecx
    add  ebx,6*65536+3
    mov  ecx,[tcolor]
    shl  edx,1
    add  edx,nro2-2-20
    mov  esi,2
    int  0x40
    popa


    add  ebx,22*65536
    inc  edx
    cmp  edx,21
    jnz  newbutton2

    ; third row

    mov  eax,8                     ; button
    mov  edx,21                    ; button number
    mov  ebx,9*65536+21            ; button start x & size
    mov  ecx,70*65536+13           ; button start y & size
    mov  esi,[bcolor]              ; button color
   newbutton3:
    pusha
    int  0x40
    popa

    pusha
    mov  eax,dword 0x00000004
    and  ebx,65535*65536
    shr  ecx,16
    add  ebx,ecx
    add  ebx,6*65536+3
    mov  ecx,[tcolor]
    shl  edx,1
    add  edx,nro3-2-40
    mov  esi,2
    int  0x40
    popa

    add  ebx,22*65536
    inc  edx
    cmp  edx,31
    jnz  newbutton3


    ; fourth row

    mov  eax,8                     ; button
    mov  edx,41                    ; button number
    mov  ebx,9*65536+109           ; button start x & size
    mov  ecx,90*65536+13           ; button start y & size
    mov  esi,0x00aabbcc            ; button color
   newbutton4:
    pusha
    int  0x40
    popa

    pusha
    mov  eax,4
    and  ebx,65535*65536
    shr  ecx,16
    add  ebx,ecx
    add  ebx,9*65536+3
    mov  ecx,[tcolor]
    sub  edx,41
    shl  edx,4
    add  edx,text
    mov  esi,16
    int  0x40
    popa

    add  ebx,110*65536
    inc  edx
    cmp  edx,43
    jnz  newbutton4

    call drawinfo

    mov  eax,12                    ; tell os about redraw end
    mov  ebx,2
    int  0x40

    popa
    ret




; DATA AREA

col         dd   0x7777aa
tcolor      dd   0xffffff
bcolor      dd   0xaabbcc
labelt:     db   'CD PLAYER'
labellen:
infotext:   db   '01 First Track: xx Last Track: xx  '
itl:
infotext2:  db   'DEFINE CD ROM BASE WITH SETUP      '
itl2:
infotext3:  db   'READING PLAYLIST - (10 secs)       '
itl3:
nro:        db   '01020304050607080910'
nro2:       db   '11121314151617181920'
nro3:       db   '21222324252627282930'
text:       db   '     STOP        READ PLAYLIST  '

cdinfo:

I_END:



