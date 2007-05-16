;   SCREENSAVER APPLICATION by lisovin@26.ru
;
;   Compile with FASM for Menuet
;
   use32
   org    0x0

   db     'MENUET01'     ; 8 byte id
   dd     0x01           ; header version
   dd     START          ; start of code
   dd     I_END          ; size of image
   dd     0x40000        ; memory for app (256 Kb)
   dd     0x40000        ; esp
   dd     0x0 , 0x0      ; I_Param , I_Icon

include   'lang.inc'
include   '..\..\..\macros.inc'

;include   'debug.inc'

START:
     mov  eax,14
     mcall
     and  eax,0xFFFF0000
     mov  [top_right_corner],eax

     mov  eax,40        ; установить маску событий
     mov  ebx,110010b   ; реагируем на клавиатуру, мышь, отрисовку фона
     mcall

     mov  eax,14        ; получим размеры экрана
     mcall
     mov  [y_max],ax
     shr  eax,16
     mov  [x_max],ax

  event:              ; обработчик событий (всех)
    mov  eax,26
    mov  ebx,9
    mcall         ; получить время системы
;    mov  eax,3
;    mcall
    mov  [evtime],eax ; запомнить его
    mov  eax,37
    mov  ebx,2
    mcall
    cmp  eax,3        ; продолжим, если нажата средняя кнопка мыши
    jne  still
    mov  eax,37       ; проверим координаты
    mov  ebx,0
    mcall
    cmp  [top_right_corner],eax
    je   create_setup ; создаём окно настройки, если мышь в верхнем правом
                      ; углу экрана



still:                  ; основной цикл программы

    mov  eax,23         ; ждём события в течение 1 секунды
    mov  ebx,100
    mcall

    bt   dword [params],0  ; ssaver works?
    jc   event
    bt   dword [params],1  ; setup works?
    jc   event

    cmp  eax,2          ; key in buffer ?
    je   event
    cmp  eax,5
    je   event
    cmp  eax,6
    je   event

    mov  eax,26
    mov  ebx,9
    mcall
    sub  eax,[evtime]
    xor  edx,edx
    mov  ebx,60*100     ; поделим на 60*100, получим время в минутах
    div  ebx
    cmp  al,[time]
    jb   still

; current_time-evtime/100/60 = время в минутах с прошлого события

    call create_ss
    jmp  still



  create_setup:
    bt dword [params],1
    jc   still
    mov  eax,51
    mov  ebx,1
    mov  ecx,sthread
    mov  edx,0x3F000
    mcall
    bts dword [params],1
    jmp  still

  create_ss:
    mov  eax,51
    mov  ebx,1
    mov  ecx,thread
    mov  edx,0x3E000
    mcall
    bts dword [params],0
    ret

 thread:
    mov  eax,5
    mov  ebx,eax
    mcall
    mov  eax,40
    mov  ebx,100010b
    mcall
    cmp  dword [type],0
    je   drawsswin
    cmp  dword [type],24
    je   asminit
    mov  dword [delay],1
    mov  [lx1],10         ; for "draw line"
    mov  [lx2],40
    mov  [ly1],50
    mov  [ly2],100
    mov  [addx1],1
    mov  [addx2],1
    mov  [addy1],1
    mov  [addy2],1
    jmp  drawsswin
 asminit:                 ; for "assembler"
    mov  dword [delay],25
    mov  eax,58
    mov  ebx,fileinfo
    mcall
 asminit1:
    mov  eax,data_from_file
    mov  [stringstart],eax
    mov  dword [stringlen],1
 newpage:
    mov  word [stringpos],10

 drawsswin:
    xor  eax,eax
    mov  ebx,eax
    mov  ecx,eax
    mov  bx,[x_max]
    mov  cx,[y_max]
    inc  ebx
    inc  ecx
    mov  edx,0x01000000
    mcall
    mov  eax,13
    xor  edx,edx
    mcall
 tstill:
    mov  eax,23
    mov  ebx,[delay]
    mcall
    cmp  eax,2
    je   thr_end
    cmp  eax,6
    je   thr_end
    cmp  dword [type],0
    je   tstill
    cmp  dword [type],24
    je   drawssasm
    call draw_line
    jmp  tstill
 thr_end:
    btr  dword [params],0
    mov  eax,26
    mov  ebx,9
    mcall
    mov  [evtime],eax
    or   eax,-1
    mcall

 drawssasm:
    mov  edi,[stringstart]
    add  edi,[stringlen]
    dec  edi
    mov  eax,edi
    sub  eax,data_from_file
    cmp  eax,10450
    ja   asminit1
    cmp word [edi],0x0a0d
    jne  noaddstring
    add  word [stringpos],10
    add  edi,2
    mov  [stringstart],edi
    mov dword [stringlen],1
    mov  ax,[stringpos]
    cmp  ax,[y_max]
    jb   tstill
    jmp  newpage
 noaddstring:
    mov  eax,4
    mov  ebx,10*65536
    mov  bx,[stringpos]
    mov  ecx,0x104ba010
    mov  edx,[stringstart]
    mov  esi,[stringlen]
    mcall
    inc dword [stringlen]
    cmp byte [edi],byte ' '
    je   drawssasm
    jmp  tstill

draw_line:

    xor  esi,esi
    xor  edi,edi
    mov  si,[x_max]
    mov  di,[y_max]

    mov  eax,[addx1]
    add  [lx1],eax
    mov  eax,[addy1]
    add  [ly1],eax

    mov  eax,[addx2]
    add  [lx2],eax
    mov  eax,[addy2]
    add  [ly2],eax

    cmp  [lx1],1
    jge  dl1
    mov  [addx1],1
  dl1:
    cmp  [lx2],1
    jge  dl2
    mov  [addx2],1
  dl2:
    cmp  [lx1],esi
    jbe  dl3
    mov  [addx1],0xffffffff
  dl3:
    cmp  [lx2],esi
    jbe  dl4
    mov  [addx2],0xffffffff
  dl4:

    cmp  [ly1],1
    jge  dl5
    mov  [addy1],1
  dl5:
    cmp  [ly2],2
    jge  dl6
    mov  [addy2],1
  dl6:
    cmp  [ly1],edi
    jbe  dl7
    mov  [addy1],-1
  dl7:
    cmp  [ly2],edi
    jbe  dl8
    mov  [addy2],-1
  dl8:

    mov  eax,[lx2]
    cmp  [lx1],eax
    jz   dnol

    mov  bx,word [lx1]
    shl  ebx,16
    mov  bx,word [lx2]

    mov  cx,word [ly1]
    shl  ecx,16
    mov  cx,word [ly2]

    mov  eax,38
    mov  edx,[lcolor]
    and  edx,0xffffff
    mcall

   dnol:

    add  [lcolor],0x010201

    ret


sthread:                        ; start of execution

     call sdraw_window

sstill:

    mov  eax,10                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw request ?
    je   sthread
    cmp  eax,2                  ; key in buffer ?
    je   sstill
    cmp  eax,3                  ; button in buffer ?
    je   sbutton

    jmp  sstill

  sbutton:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  snoclose

    btr dword [params],1
    mov  eax,-1                 ; close this program
    mcall
  snoclose:
    cmp  ah,7
    jne  nosetfl
    btc dword [params],0
    call drawflag
    call drawtype
    call drawtime
    jmp  sstill
  nosetfl:
    bt dword [params],0
    jc   sstill
    cmp  ah,2
    jne  notypedown
    mov  eax,[type]
    test  eax,eax
    je   sstill
    sub  eax,12
    jmp  typeupdn
  notypedown:
    cmp  ah,3
    jne  notypeup
    mov  eax,[type]
    cmp  eax,24
    jae  sstill
    add  eax,12
    jmp  typeupdn
  notypeup:
    cmp  ah,4
    jne  notimedown
    mov  al,[time]
    cmp  al,1
    jbe  sstill
    dec  al
;    das
    jmp  timeupdn
  notimedown:
    cmp  ah,5
    jne  notimeup
    mov  al,[time]
    cmp  al,59 ; 0x59
    jae  sstill
    inc  al
;    daa
    jmp  timeupdn
  notimeup:
    cmp  ah,6
    jne  noshow
    mov  eax,5
    mov  ebx,150
    mcall
    call create_ss
    jmp  sstill
  noshow:
    jmp  sstill

  timeupdn:
    mov  [time],al
    call drawtime
    jmp  sstill
  typeupdn:
    mov  [type],eax
    call drawtype
    jmp  sstill

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


sdraw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,100*65536+215         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+70          ; [y start] *65536 + [y size]
    mov  edx,0x13400088            ; color of work area RRGGBB,8->color gl
    mov  edi,title
    mcall

    mov  eax,8
    mov  ebx,47*65536+10
    mov  ecx,31*65536+10
    mov  edx,2
    mov  esi,0x702050
    mcall
    push ebx
    add  ebx,13*65536
    mov  edi,ebx
    inc  edx
    mcall
    pop  ebx
    add  ecx,15*65536
    inc  edx
    mcall
    mov  ebx,edi
    inc  edx
    mcall
    mov  ebx,160*65536+40
    mov  ecx,28*65536+14
    inc  edx
    mcall

    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,15*65536+33             ; [x start] *65536 + [y start]
    mov  ecx,0xffffff
    mov  edx,setuptext
    mov  esi,9
    mcall
    add  ebx,15
    add  edx,esi
    mov  esi,30
    mcall
    mov  ebx,169*65536+32
    mov  edx,buttext
    mov  esi,4
    mcall

    call drawtype
    call drawtime
    call drawflag

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret

drawtype:
    mov  eax,13
    mov  ebx,80*65536+75
    mov  ecx,30*65536+12
    mov  edx,0xffffff
    bt   dword [params],0
    jnc  noblue
    mov  edx,0x4e00e7
 noblue:
    mcall
    mov  eax,4
    mov  ebx,82*65536+32
    xor  ecx,ecx
    mov  edx,typetext
    add  edx,[type]
    mov  esi,12
    mcall
    ret

drawtime:
    mov  eax,13
    mov  ebx,80*65536+15
    mov  ecx,45*65536+12
    mov  edx,0xffffff
    bt   dword [params],0
    jnc  noblue1
    mov  edx,0x4e00e7
 noblue1:
    mcall
    mov  eax,47
    mov  edx,82*65536+47
    xor  esi,esi
    movzx ecx,byte [time]
    mov  ebx,0x00020000
    mcall
    ret

drawflag:
    mov  eax,8
    mov  ebx,150*65536+10
    mov  ecx,45*65536+10
    mov  edx,7
    mov  esi,0xe0e0e0
    mcall
    mov  eax,4
    mov  ebx,153*65536+47
    xor  ecx,ecx
    mov  esi,1
    mov  edx,flag
    bt   dword [params],0
    jc   setf
    inc  edx
  setf:
    mcall
    ret

; DATA AREA
buttext    db 'SHOW'
flag       db 'V '
title      db 'SCREENSAVER SETUP',0
setuptext  db 'TYPE: < >TIME: < >     MINUTES    NEVER'
typetext   db 'BLACK SCREENCOLOR LINES ASSEMBLER   '
type       dd 12
time       db 15     ; время до запуска заставки в минутах
delay      dd 100

lx1   dd  10
lx2   dd  40

ly1   dd  50
ly2   dd  100

addx1 dd  1
addx2 dd  1

addy1 dd  1
addy2 dd  1

stringlen   dd 1
stringstart dd 0
stringpos   dw 10

params  dd 0     ;if bit 0 set-ssaver works if bit 1 set-setup works

fileinfo:
  dd 0,0,21,data_from_file,work_area
  db '/RD/1/SS.ASM',0

I_END:


; UNINITIALIZED DATA:

lcolor           dd ?
evtime           dd ?     ; время предыдущего события
x_max            dw ?     ; размеры экрана
y_max            dw ?

top_right_corner dd ?
work_area:
rb 0x10000
data_from_file: