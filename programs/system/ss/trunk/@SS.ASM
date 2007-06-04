;   SCREENSAVER APPLICATION by lisovin@26.ru
;
;   Compile with FASM for Menuet
;
   use32
   org    0x0

used_memory = 0x10000

   db     'MENUET01'     ; 8 byte id
   dd     0x01           ; header version
   dd     START          ; start of code
   dd     I_END          ; size of image
   dd     used_memory    ; memory for app
   dd     used_memory    ; esp
   dd     0x0 , 0x0      ; I_Param , I_Icon

include   'lang.inc'
include   '..\..\..\macros.inc'
purge mov       ; decrease kpack'ed size

;include   'debug.inc'

START:
     mov  eax,40        ; установить маску событий
     mov  ebx,110010b   ; реагируем на клавиатуру, мышь, отрисовку фона
     mcall

bgr_changed:

        mov     eax,14
        mcall
        mov     [y_max], ax
        shr     eax, 16
        mov     [x_max], ax
        shl     eax, 16
        mov     [top_right_corner], eax

still:
        movzx   ebx, [time]
        imul    ebx, 60*100
        mcall   23              ; ждём события в течение [time] минут
        test    eax, eax
        jz      create_ss_thread
        cmp     al, 2           ; key in buffer?
        jz      key
        cmp     al, 5           ; background redraw?
        jz      bgr_changed
; mouse event
        mcall   37,2            ; проверим кнопки
        and     al, 3
        cmp     al, 3           ; нажаты обе кнопки мыши?
        jnz     still
        mcall   37,0            ; проверим координаты
        cmp     [top_right_corner], eax
        jnz     still
create_setup:
        test    [params], 2
        jnz     still           ; окно настроек уже создано
        mcall   51,1,sthread,used_memory-0x1000
        or      [params], 2
        jmp     still
key:
        mcall                   ; eax = 2
        jmp     still

create_ss_thread:
        test    [params], 3
        jnz     still
        call    create_ss
        jmp     still

create_ss:
        mcall   51,1,thread,used_memory-0x2000
        or      [params], 1
        ret

 thread:
;    mov  eax,5
;    mov  ebx,eax
;    mcall
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
    mov  eax,70
    mov  ebx,fileinfo
    mcall
    mov  [filesize], ebx
 asminit1:
    mov  [stringstart],data_from_file
    mov  dword [stringlen],1
 newpage:
    mov  word [stringpos],10

 drawsswin:
    xor  eax,eax
    movzx ebx,[x_max]
    movzx ecx,[y_max]
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
    test eax,eax
    jnz  thr_end
    cmp  dword [type],0
    je   tstill
    cmp  dword [type],24
    je   drawssasm
    call draw_line
    jmp  tstill
 thr_end:
    and  [params], not 1
    or   eax,-1
    mcall

 drawssasm:
    mov  edi,[stringstart]
    add  edi,[stringlen]
    dec  edi
    lea  eax,[edi-data_from_file]
    cmp  eax,[filesize]
    ja   asminit1
    cmp  word [edi],0x0a0d
    je   addstring
    cmp  byte [edi],0x0a
    jne  noaddstring
    dec  edi
addstring:
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
    movzx esi,[x_max]
    movzx edi,[y_max]

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

    dec  eax                    ; redraw request ?
    je   sthread
    dec  eax                    ; key in buffer ?
    jne  sbutton
    mov  al,2
    mcall
    jmp  sstill

  sbutton:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  snoclose

    and  [params], not 2
    mov  eax,-1                 ; close this program
    mcall
  snoclose:
    cmp  ah,7
    jne  nosetfl
    xor  [params], 1
    call drawflag
    call drawtype
    call drawtime
    jmp  sstill
  nosetfl:
    test [params], 1
    jnz  sstill
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
    dec  eax
;    das
    jmp  timeupdn
  notimedown:
    cmp  ah,5
    jne  notimeup
    mov  al,[time]
    cmp  al,59 ; 0x59
    jae  sstill
    inc  eax
;    daa
    jmp  timeupdn
  notimeup:
    cmp  ah,6
    jne  noshow
    mov  eax,5
    mov  ebx,10;150
    mcall
    call create_ss
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

    mov  al,8
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

    mov  al,4                        ; function 4 : write text to window
    mov  ebx,15*65536+33             ; [x start] *65536 + [y start]
    mov  ecx,0x80ffffff
    mov  edx,setuptext
    mcall
    add  ebx,15
    add  edx,10
    mcall
    mov  ebx,169*65536+32
    mov  edx,buttext
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
    test [params], 1
    jz   noblue
    mov  edx,0x4e00e7
 noblue:
    mcall
    mov  al,4
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
    test [params], 1
    jz   noblue1
    mov  edx,0x4e00e7
 noblue1:
    mcall
    mov  al,47
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
    mov  al,4
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
buttext    db 'SHOW',0
flag       db 'V '
title      db 'SCREENSAVER SETUP',0
setuptext  db 'TYPE: < >',0
           db 'TIME: < >     MINUTES    NEVER',0
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

params  db 0     ;if bit 0 set-ssaver works if bit 1 set-setup works

fileinfo:
; used_memory-0x3000-data_from_file = ~50 Kb with current settings
  dd 0,0,0,used_memory-0x3000-data_from_file,data_from_file
  db '/RD/1/NETWORK/PPP.ASM',0

I_END:


; UNINITIALIZED DATA:

lcolor           dd ?
x_max            dw ?     ; размеры экрана
y_max            dw ?

top_right_corner dd ?
filesize        dd ?
data_from_file:
