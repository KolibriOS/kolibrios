;   Picture browser by lisovin@26.ru
;   Modified by Ivan Poddubny - v.0.3
;   Compile with FASM for Menuet

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
include    'lang.inc'
include    'macros.inc'
;******************************************************************************

tmp_param dd 0

START:                          ; start of execution

; check for parameters
   cmp   dword [temp_area],'BOOT'
   jne   .no_boot
   call  load_image
   call  convert
   call  background
   or    eax,-1
   int   0x40
 .no_boot:

   cmp   byte [temp_area],0
   jz    .no_param
   mov   edi,string      ; clear string
   mov   ecx,43*3        ;   length of a string
   xor   eax,eax         ;   symbol <0>
   cld
   rep   stosb

   mov   edi,temp_area   ; look for <0> in temp_area
   mov   ecx,43*3+1      ;   strlen
   repne scasb
   add   edi,-temp_area  ;   get length of the string
   dec   edi

   mov   ecx,edi
   mov   esi,temp_area
   mov   edi,string
   rep   movsb           ; copy string from temp_area to "string" (filename)

   call  draw_window
   mov   [tmp_param],0xdeadbeef
 .no_param:


   mov  ecx,-1           ; get information about me
   call getappinfo

   mov  edx,[process_info+30] ; теперь в edx наш идентификатор
   mov  ecx,eax

  @@:
   call getappinfo
   cmp  edx,[process_info+30]
   je   @f  ; если наш PID совпал с PID рассматриваемого процесса, мы нашли себя
   dec  ecx ; иначе смотрим следующий процесс
   jne  @b  ; возвращаемся, если не все процессы рассмотрены
  @@:

; теперь в ecx номер процесса
    mov  [process],ecx

    cmp  [tmp_param],0xdeadbeef
    jne  @f
    jmp  kopen
  @@:

    call draw_window

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  still

  red:
    bt   [status],2
    jnc  @f
    mov eax,18
    mov ebx,3
    mov ecx,[process]
    int 0x40
    btr [status],2
    jmp still
   @@:
    call draw_window
    jmp still

  key:                          ; key
    int  0x40
    cmp  ah,6
    je   kfile
    cmp  ah,15
    je   kopen
    cmp  ah,9
    je   kinfo
    cmp  ah,2
    je   kbgrd
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

    mov  eax,-1                 ; close this program
    int  0x40
  noclose:
    cmp  ah,2
    jne  nofile
  kfile:
    bt   dword [status],0
    jc   still
    bts  dword [status],0
    mov  eax,51
    mov  ebx,1
    mov  ecx,thread1
    mov  edx,0x29fff0
    int  0x40
    jmp  still
  nofile:
    cmp  ah,3
    jne  noopen

 kopen:
    mov ecx,-1
    call getappinfo
    mov ebx,dword [I_END+42]
    mov ecx,dword [I_END+46]
    add ebx,10*65536-15
    add ecx,50*65536-55
    mov edx,0xffffff
    mov eax,13
    int 0x40

    call load_image

  open1:
    cmp word [I_END],word 'BM'
    jne  still
    call convert
    call drawimage
    jmp  still
  noopen:

    cmp  ah,4
    jne  noinfo
  kinfo:
    bt   dword [status],1
    jc   still
    bts  dword [status],1
    mov  eax,51
    mov  ebx,1
    mov  ecx,thread2
    mov  edx,0x2afff0
    int  0x40
    jmp  still
  noinfo:

; УСТАНОВИТЬ ФОН
    cmp  ah,5
    jne  still
  kbgrd:
    bt dword [status],3
    jc   still
    bts dword [status],3
    mov  eax,51
    mov  ebx,1
    mov  ecx,thread3
    mov  edx,0x2bfff0
    int  0x40
    jmp  still
    ;call background

 getappinfo:
    mov  eax,9
    mov  ebx,I_END
    int  0x40
    ret


load_image:
    mov  dword [fileinfo+8],1 ; how many blocks to read (1)
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
    cmp  [I_END+2],dword 512  ; размер файла (file size)
    jbe  open1
    mov  eax,[I_END+2]
    shr  eax,9 ; поделим на 512 и прибавим 1 - получим число блоков
    inc  eax

    mov  dword [fileinfo+8],eax
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
ret


  drawimage:
    cmp  word [I_END],word 'BM'
    jne  nodrawimage
    mov  eax,7
    mov  ebx,[soi]
    mov  ecx,[I_END+18]
    shl  ecx,16
    add  ecx,[I_END+22]
    mov  edx,10*65536+50
    int  0x40
  nodrawimage:
    ret

; УСТАНОВИТЬ ФОН
  background:
    cmp  word [I_END],word 'BM'
    jne  @f
    mov  eax,15
    mov  ebx,1
    mov  ecx,[I_END+18] ; ширина
    mov  edx,[I_END+22] ; высота
    int  0x40

    mov  esi, ecx
    imul esi, edx
    imul esi, 3
    mov  ebx,5
    mov  ecx,[soi]
    xor  edx,edx
;;;    mov  esi, ;640*480*3
    int  0x40

    dec  ebx    ;tile/stretch
    mov  ecx,dword [bgrmode]
    int  0x40

    dec  ebx
    int  0x40
   @@:
    ret

  convert:
    movzx eax,word [I_END+28]
    mul dword [I_END+18]
    mov  ebx,32
    div  ebx
    test edx,edx
    je   noaddword
    inc  eax
  noaddword:
    mov  [dwps],eax  ;dwps-doublewords per string
    shl  eax,2
    mov  [bps],eax   ;bps-bytes per string

    cmp dword [I_END+34],0
    jne  yespicsize  ;if picture size is defined
    mul dword [I_END+22]
    mov dword [I_END+34],eax

  yespicsize:
    mov  eax,I_END
    push eax
    add  eax, [I_END+2];file size
    inc  eax
    mov  [soi],eax   ;soi-start of image area for drawing
    pop  eax
    add  eax, [I_END+10]
    mov  [sop],eax   ;sop-start of picture in file
    add  eax, [I_END+34]
    mov  [eop],eax   ;eop-end of picture in file
    mov  eax, [I_END+18]
    mov  ebx,3
    mul  ebx             ;3x pixels in eax

    mov  edi,[soi]   ;initializing
    mov  esi,[eop]
    sub  esi,[bps]


  nextstring:
    push edi
    cmp word [I_END+28],24
    jne  convertno32

    mov  ecx,[dwps]
    cld
    rep movsd
  convert1:
    pop  edi
    sub  esi,[bps]
    sub  esi,[bps]
    cmp  esi,[sop]
    jb   nomorestring
    add  edi,eax
    jmp  nextstring

  nomorestring:
    ret

  convertno32:
    mov  ebx,I_END
    add  ebx, [I_END+14]
    add  ebx,14          ;start of color table
    push esi
    add  esi,[bps]
    mov  [eos],esi
    pop  esi
  nextelem:
    push eax
    movzx eax,byte [esi]
    cmp word [I_END+28],4
    je   convert4bpp
    cmp word [I_END+28],1
    je   convert1bpp
    call converttable
  convert2:
    pop  eax
    inc  esi
    cmp  esi,[eos]
    jae  convert1
    add  edi,3
    jmp  nextelem

  convert4bpp:
    shl  ax,4
    shr  al,4
    push ax
    movzx eax,ah
    call converttable
    add  edi,3
    pop  ax
    movzx eax,al
    call converttable
    jmp  convert2

  convert1bpp:
    mov  ecx,eax
    mov  edx,7
  nextbit:
    xor  eax,eax
    bt   ecx,edx
    jnc  noaddelem
    inc  eax
  noaddelem:
    push edx
    call converttable
    pop  edx
    dec  edx
    cmp  edx,0xffffffff
    je   convert2
    add  edi,3
    jmp  nextbit

  converttable:
    shl  eax,2
    add  eax,ebx
    mov  edx, [eax]
    mov dword [edi],edx
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,350                   ; [x start] *65536 + [x size]
    mov  ecx,400                   ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    int  0x40

    mov  eax,8
    mov  ebx,10*65536+46
    mov  ecx,25*65536+20
    mov  edx,2
    mov  esi,0x780078
  newbutton:
    int  0x40
    add  ebx,48*65536
    inc  edx
    cmp  edx,6
    jb   newbutton

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,12                    ; text length
    int  0x40

    mov  ebx,14*65536+32
    mov  edx,buttext
    mov  esi,26
    int  0x40

    call drawimage

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA

labelt:
         db 'MeView v.0.3'

lsz buttext,\
    en,   ' FILE   OPEN   INFO   BGRD',\
    ru,   ' ФАЙЛ  ОТКР   ИНФО   ФОН  '

status   dd 0  ;bit0=1 if file thread is created
bps      dd 0
dwps     dd 0
soi      dd 0
sop      dd 0
eop      dd 0
eos      dd 0
process  dd 0

thread1:                        ; start of thread1

     call draw_window1

still1:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   thread1
    cmp  eax,2                  ; key in buffer ?
    je   key1
    cmp  eax,3                  ; button in buffer ?
    je   button1

    jmp  still1

  key1:                         ; key
    int  0x40
    cmp  ah,179
    jne  noright
    mov  eax,[pos]
    cmp  eax,41
    ja   still1
    inc  eax
    mov  [pos],eax
    call drawstring
    jmp  still1
  noright:
    cmp  ah,176
    jne  noleft
    mov  eax,[pos]
    test eax,eax
    je   still1
    dec  eax
    mov  [pos],eax
    call drawstring
    jmp  still1
  noleft:
    cmp  ah,182        ;del
    jne  nodelet
    call shiftback
    call drawstring
    jmp  still1
  nodelet:
    cmp  ah,8          ;zaboy
    jne  noback
    mov  eax,[pos]
    test eax,eax
    je   still1
    dec  eax
    mov  [pos],eax
    call shiftback
    call drawstring
    jmp  still1
  noback:
    cmp  ah,13
    jne  noenter
  enter1:
    mov  al,byte ' '
    mov  edi,string
    mov  ecx,43
    cld
    repne scasb
    dec  edi
    mov  byte [edi],0
    jmp  close1
  noenter:
    cmp  ah,27
    jne  noesc
    jmp  enter1
  noesc:
    cmp  dword [pos],42
    jae  still1

    mov  edi,string
    add  edi,42
    mov  esi,edi
    dec  esi
    mov  ecx,42
    sub  ecx,[pos]
    std
    rep  movsb

    shr  eax,8
    mov  esi,string
    add  esi,[pos]
    mov  byte [esi],al
    inc  dword [pos]
    call drawstring

    jmp  still1

  button1:                      ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose1
    jmp  enter1
  close1:
    bts  dword [status],2
    btr  dword [status],0
    mov  eax,-1                 ; close this program
    int  0x40
  noclose1:
    cmp  ah,2
    jne  nosetcur
    mov  eax,37
    mov  ebx,1
    int  0x40
    shr  eax,16
    sub  eax,21
    xor  edx,edx
    mov  ebx,6
    div  ebx
    mov  [pos],eax
    call drawstring
    jmp  still1
  nosetcur:
    jmp  still1


  shiftback:
    mov  edi,string
    add  edi,[pos]
    mov  esi,edi
    inc  esi
    mov  ecx,43
    sub  ecx,[pos]
    cld
    rep movsb
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window1:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,100*65536+300         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+80          ; [y start] *65536 + [y size]
    mov  edx,0x03780078            ; color of work area RRGGBB,8->color gl
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt1               ; pointer to text beginning
    mov  esi,4                     ; text length
    int  0x40

    call drawstring

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

 drawstring:
    pusha
    mov  eax,8             ;invisible button
    mov  ebx,21*65536+258
    mov  ecx,40*65536+15
    mov  edx,0x40000002
    int  0x40

    mov  eax,13             ;bar
    mov  edx,0xe0e0e0
    int  0x40
    push eax                ;cursor
    mov  eax,6*65536
    mul  dword [pos]
    add  eax,21*65536+6
    mov  ebx,eax
    pop  eax
    mov  edx,0x6a73d0
    int  0x40
    mov  eax,4              ;path
    mov  ebx,21*65536+44
    xor  ecx,ecx
    mov  edx,string
    mov  esi,43
    int  0x40


    popa
    ret

; DATA AREA

lsz labelt1,\
   en,  'File',\
   ru,  'Файл'

pos: dd 6
fileinfo:
     dd 0
     dd 0
     dd 1          ;number of blocks  of 512 bytes
     dd I_END
     dd temp_area
string:
; db '/HARDDISK/FIRST/1/DICK.BMP                  '
  db '/hd/1/menuet/pics/new.bmp                   '
  db '                                            '
  db '                                            '

thread2:                          ; start of info thread

     call draw_window2

still2:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   thread2
    cmp  eax,2                  ; key in buffer ?
    je   close2
    cmp  eax,3                  ; button in buffer ?
    je   button2

    jmp  still2

  button2:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose2
  close2:
    btr dword [status],1
    bts dword [status],2
    mov  eax,-1                 ; close this program
    int  0x40
  noclose2:

    jmp  still2




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window2:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,100*65536+330         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+90          ; [y start] *65536 + [y size]
    mov  edx,0x03780078            ; color of work area RRGGBB,8->color gl
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt2               ; pointer to text beginning
    mov  esi,labelt2.size          ; text length
    int  0x40

    mov  ebx,10*65536+30
    mov  edx,string
    mov  esi,43
    int  0x40
    mov  edx,fitext
    mov  esi,14
    add  ebx,70*65536+10
 followstring:
    int  0x40
    add  ebx,10
    add  edx,esi
    cmp  ebx,80*65536+70
    jbe  followstring
    mov  eax,47
    mov  edx,200*65536+40
    mov  esi,ecx
    mov  ecx, [I_END+2]
    call digitcorrect
    int  0x40
    add  edx,10
    mov  ecx, [I_END+18]
    call digitcorrect
    int  0x40
    add  edx,10
    mov  ecx, [I_END+22]
    call digitcorrect
    int  0x40
    add  edx,10
    movzx ecx,word [I_END+28]
    call digitcorrect
    int  0x40

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

 digitcorrect:
    xor  ebx,ebx
    mov  bh,6
    cmp  ecx,99999
    ja   c_end
    dec  bh
    cmp  ecx,9999
    ja   c_end
    dec  bh
    cmp  ecx,999
    ja   c_end
    dec  bh
    cmp  ecx,99
    ja   c_end
    dec  bh
    cmp  ecx,9
    ja   c_end
    dec  bh
 c_end:
    bswap ebx
    ret


; DATA AREA

lsz labelt2,\
    en,   'File info',\
    ru,   'Информация о файле'

lsz fitext,\
     en, 'FILE SIZE     ',\
     en, 'X SIZE        ',\
     en, 'Y SIZE        ',\
     en, 'BITS PER PIXEL',\
                          \
     ru, 'Размер файла  ',\
     ru, 'Ширина        ',\
     ru, 'Высота        ',\
     ru, 'Бит на пиксел '

thread3:                          ; start of bgrd thread

     call draw_window3

still3:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   thread3
    cmp  eax,2                  ; key in buffer ?
    je   key3
    cmp  eax,3                  ; button in buffer ?
    je   button3

    jmp  still3

  key3:
    int  0x40
    cmp  ah,27
    je   close3
    cmp  ah,13
    je   kok
    cmp  ah,178 ;up
    jne  nofup
    cmp  dword [bgrmode],1
    je   fdn
  fup:
    dec dword [bgrmode]
    jmp  flagcont
  nofup:
    cmp  ah,177 ;down
    jne  still3
    cmp dword [bgrmode],2
    je   fup
  fdn:
    inc dword [bgrmode]
    jmp  flagcont


  button3:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose3
  close3:
    btr dword [status],3
    bts dword [status],2
    mov  eax,-1                 ; close this program
    int  0x40
  noclose3:
    cmp  ah,4
    jne  nook
   kok:
    call background
    jmp  close3
  nook:
    cmp  ah,2
    jb   still3
    cmp  ah,3
    ja   still3
    dec  ah
    mov byte [bgrmode],ah
   flagcont:
    call drawflags
    jmp  still3




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window3:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,100*65536+200         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+100         ; [y start] *65536 + [y size]
    mov  edx,0x03780078            ; color of work area RRGGBB,8->color gl
    int  0x40

    mov  eax,8
    mov  ebx,70*65536+40
    mov  ecx,70*65536+20
    mov  edx,4
    mov  esi,0xac0000
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt3               ; pointer to text beginning
    mov  esi,14                    ; text length
    int  0x40
    add  ebx,38*65536+20
    mov  ecx,0xddeeff
    mov  edx, bgrdtext
    mov  esi, bgrdtext.size
    int  0x40
    add  ebx,40*65536+15
    mov  edx, tiled
    mov  esi, tiled.size
    int  0x40
    add  ebx,15
    mov  edx, stretch
    mov  esi, stretch.size ;7
    int  0x40
    add  ebx,18
    mov  edx, ok_btn
    mov  esi, ok_btn.size ;2
    int  0x40

    call drawflags

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

 drawflags:
    mov  eax,8
    mov  ebx,70*65536+10
    mov  ecx,40*65536+10
    mov  edx,2
    mov  esi,0xe0e0e0
    int  0x40
    add  ecx,15*65536
    inc  edx
    int  0x40
    mov  eax,4
    mov  ebx,73*65536+42
    xor  ecx,ecx
    mov  edx,vflag
    mov  esi,1
    cmp  dword [bgrmode],1
    je   nodownflag
    add  ebx,15
 nodownflag:
    int  0x40
    ret


; DATA AREA
vflag: db 'x'
bgrmode: dd 1

lsz labelt3,\
    en,   'Background set',\
    ru,   "Установка фона"

lsz bgrdtext,\
    en, 'SET AS BACKGROUND:',\
    ru, 'Тип обоев:'

lsz tiled,\
    en, 'TILED',\
    ru, 'замостить'

lsz stretch,\
    en, 'STRETCH',\
    ru, 'растянуть'

lsz ok_btn,\
    en, 'Ok',\
    ru, 'Ok'



IM_END:
temp_area:
rb 0x10000
I_END:
process_info:
