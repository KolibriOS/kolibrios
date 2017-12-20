LEVELCONV equ 10
SQ equ 8
FLDSIZE equ 49
FLDSIZE2 equ FLDSIZE*8
DELAY  equ 20
TICKS  equ 10
LEV_START equ 1
MOBILITY equ 9;13

  use32
  org     0x0

  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     run                   ; start of code
  dd     I_END                   ; size of image
  dd     stak+0x400             ; memory for app
  dd     stak+0x400             ; esp
  dd     0x0 , 0x0               ; I_Param , I_Icon
include "macros.inc"
;include "debug.inc"

run:
if  LEVELCONV eq 1
    call compress_levels
    jmp  close
end if
    mcall 3
    mov  cl,16
    ror  eax,cl
    mov  [generator],eax    ; random generator from Tetris
reset:
    mov  [levptr],level
    mov  [levnum],LEV_START
  if ~ LEV_START eq 1
    mov  ecx,[levnum]
    dec  ecx
    mov  esi,[levptr]
    jecxz start
  .lp:
    movzx eax,word[esi]
    and  eax,0x7fff
    add  esi,eax ; limit
    loop .lp
    mov  [levptr],esi
;    sub  esi,level
;    dpd  esi
  end if
start:
    mov  ecx,[generator]
    and  ecx,0xff
  .shuf:
    call random
    loop .shuf
    xor  eax,eax
    and  [locked],eax
    and  [captured],eax
    mov  [ticks],TICKS
    mov  [mode],-1
    mov  ebx,FLDSIZE
    mov  eax,[generator]
    xor  edx,edx
    div  ebx
    mov  cl,dl
    call random
    mov  eax,[generator]
    xor  edx,edx
    div  ebx
    mov  ch,dl
    xchg eax,ecx
    movzx ecx,al
    imul cx,bx
    shr  ax,8
    add  cx,ax
    mov  esi,[levptr]
    mov  [dot],ecx
    call decompress_level
    mov  ebx,8
    call  nokey
red:
    call  draw_window
;    and  [event],0

still:

  .flush:
    mcall 2
    test al,al
    jz   .flush

    mov  ebx,DELAY
    mov  eax,[levnum]
    shr  eax,1
    sub  ebx,eax
    mov  eax,23
    cmp  [mode],0
    jl   .wait
    mov  eax,10
  .wait:
    mcall
  .evt:
    mov  ebx,8
    test eax,eax
    jne  .ev_yes
    dec  [ticks]
    jne  still
;    mov  [ticks],TICKS
;    cmp  al,[event]
    jmp   key.ex
;    and  [event],al
;    jmp  still
  .ev_yes:
    cmp  eax,2
    je   key
    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,3                  ; button in buffer ?
    je   button
key:
;    mov  [event],al
    mcall 2
	and  eax, 0xffff
    cmp  ah,13
    je   button.mode
    cmp  ah,27
    je   button.mode
    cmp  [mode],-1
    jne  still
    cmp  ah,104
    jne  .nohelp
    mov  [mode],4
    jmp  red
  .nohelp:
    cmp  ah,112
    jne  .nopau
    mov  [mode],3
    jmp  red
  .nopau:
    shr  eax,8
;    dpd  eax
    cmp  eax,' '
    je   .ex
    cmp  eax,176
    jb   still
    cmp  eax,179
    ja   still
    cmp  [locked],0
    jnz  still
    lea  ebx,[eax-176]
  .ex:
    mov  [ticks],TICKS
    call nokey
    call evolution
    call  red_field
    jmp  still

button:                       ; BUTTON - only close supported
    mcall 17
    cmp  ah,2
    jne  close
    mcall 8,,,2+1 shl 31
;    mov  [event],2
  .mode:
    mov  [ticks],TICKS
    mov  al,[mode]
    cmp  al,1
    je   reset
    cmp  al,2
    je   reset
    mov  [mode],-1
    test  al,al
    jnz  .no0
    inc  [levnum]
    jmp  start
  .no0:
    jmp  red
  close:
    or   eax,-1
    int  0x40

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
    mov  [ticks],TICKS
    mcall 12,1
    mcall 0,<100,FLDSIZE2+16>,<100,FLDSIZE2+38>,0x14008000, , header
 
  if lang eq ru
    mcall 47,0x20000,[levnum],<128,8>, 0
  else
    mcall 47,0x20000,[levnum],<117,8>, 0
  end if
    call  red_field
    cmp   [mode],0
    jl    .edraw
    mov   ecx,(FLDSIZE2-20) shl 16+20
    cmp   [mode],4
    jne    .nohelp
    mov   ecx,100 shl 16+FLDSIZE2-35-100+38
  .nohelp:
    mcall 13,<5+8,FLDSIZE2-8>,,0xf5deb3;0xffff00
    mcall 8,<FLDSIZE2-40,35>,<FLDSIZE2-18,15>,2,0xc0c000
    mcall 4,<363,FLDSIZE2-20+6>,0x100000ff,msg_ok,2
    mov   ebx,(5+8+35)shl 16+FLDSIZE2-20+6
    movzx ecx,[mode]
    mov   edx,messages+4
    cmp   ecx,4
    jne   .nxt
    mov   edx,desc+4
    xor   ecx,ecx
    mov   ebx,20 shl 16+110
  .nxt:
    mov   esi,[edx-4]
    jecxz .drw
    dec   ecx
    lea   edx,[edx+esi+4]
    jmp   .nxt
  .drw:
    cmp   esi,-1
    je    .edraw
    mcall ,,0x000000ff
    mov   ecx,1
    add   ebx,18
    cmp   [mode],4
    je    .nxt
  .edraw:
    mcall 12,2
    ret

red_field:
    mov   edi,field
    mov   ebp,28 shl 16+SQ ; y
    mov   ecx,FLDSIZE
  .lp1:
    push  ecx
    mov   ecx,FLDSIZE
    mov   ebx,8 shl 16+SQ  ; x
  .lp2:
    push  ecx
    mov   edx,0x8000
    cmp   byte[edi],0
    je    .zero
    cmp   byte[edi],2
    jne   .nored
    mov   edx,0xff0000
    cmp   [locked],0
    jz    .zero
    mov   edx,0xa00000
    jmp   .zero
  .nored:
    mov   edx,0
  .zero:
    mcall 13,,ebp
    inc   edi
    pop   ecx
    add   ebx,8 shl 16
    loop  .lp2
    pop   ecx
    add   ebp,8 shl 16
    loop  .lp1

    mov   eax,[dot]
    mov   cl,FLDSIZE
    div   cl

;    movzx ebx,ah
;    shl   ebx,19
;    add   ebx,8 shl 16+7
;    movzx ecx,al
;    shl   ecx,19
;    add   ecx,28 shl 16+7
;    push  eax
;    mcall 13,,,0xff
;    pop   eax

    movzx ebx,ah
    shl   ebx,19
    add   ebx,8 shl 16
    shld  edx,ebx,16
    add   dx,SQ-1
    mov   bx,dx
    movzx ecx,al
    shl   ecx,19
    add   ecx,28 shl 16
    shld  edx,ecx,16
    add   dx,SQ-1
    mov   cx,dx
    mcall 38,,,0xffffff
    ror   ecx,16
    mcall
    ret

nokey:
    xor  eax,eax
    mov  edi,buff
    mov  ecx,FLDSIZE*FLDSIZE+3
    push ecx edi
    rep  stosb
    pop  edi
    ;mov  esi,field
    mov  edi,field
    mov  edx,FLDSIZE
    pop  ecx
  .llp:
    mov  eax,2
    repne scasb
    jecxz .exx
    and  byte[edi-1],0
    push ecx
    lea  eax,[edi-field-1]
    div  dl
    call get_cell
    mov  byte[buff+ecx],2
    pop  ecx
    loop .llp
  .exx:
    mov  edi,field
    mov  esi,buff
    mov  ecx,FLDSIZE*FLDSIZE
  .lp4:
    lodsb
    cmp  al,2
    jne  .skip3
    mov  [edi],al
  .skip3:
    inc  edi
    loop  .lp4

    ret

get_cell:
; ax - source cell [x][y]
; ebx - direction
;     4 2 5
;     0 . 3
;     6 1 7
; out - ecx cell ptr

    push eax ebx
    add  ax,[ebx*2+dirs]
    mov  ebx,FLDSIZE
  .c0:
    cmp  al,bl
    jb   .c1
    sub  al,bl
    jmp  .c0
  .c1:
    cmp  ah,bl
    jb   .c2
    sub  ah,bl
    jmp  .c1
  .c2:
    movzx ecx,al
    imul cx,bx
    shr  ax,8
    add  cx,ax
    pop  ebx eax
    ret

evolution:
    xor   edi,edi
    and   [locked],edi
    mov   edx,FLDSIZE
    mov   ecx,FLDSIZE*FLDSIZE
  .l1:
    push  ecx
    mov   eax,edi
    div   dl
    mov   ecx,8
    xor   ebx,ebx
    mov   word[neib],bx ; neighbour count
  .l2:
    push  ecx
    call  get_cell
    movzx esi,byte[field+ecx]
    test   esi,esi ; 0?
    jz    .skip2
    inc   byte[neib-1+esi]
  .skip2:
    inc   ebx
    pop   ecx
    loop  .l2
    mov   cl,[neib]
    add   cl,[neib+1]
    cmp   cl,2
    jne   .no2
    mov   al,[field+edi]
    jmp   .writebuf
  .no2:
    xor   al,al
    cmp   cl,3
    jne   .writebuf
    inc   al
    mov   cl,[neib+1]
    cmp   cl,[neib]
    jb    .writebuf
    inc   al
  .writebuf:
    mov   [buff+edi],al
    pop   ecx
    inc   edi
    loop  .l1

    mov   esi,buff
    mov   edi,field
    mov   ecx,FLDSIZE*FLDSIZE
    rep   movsb
    call  square_check
    call  dot_move
    ret

square_check:
    mov   ecx,FLDSIZE*FLDSIZE+3
    mov   edx,FLDSIZE
    mov   edi,field
    xor   eax,eax
    pusha
    and   dword[cells],eax
    and   [locked],eax
  .nored:
    mov   al,[edi]
    inc   edi
    dec   ecx
    jecxz .ex1
    test   al,al
    jz    .nored
    inc   [cells-2+eax*2]
    jmp   .nored
  .ex1:
    mov   ax,[cells+2]
    cmp   ax,[cells]
    ja    dot_move.next_lev
    cmp   [cells+2],4
    je    .sq_check
    add   esp,32
    cmp   [cells+2],0
    jne   .loc
    mov   [mode],1
    mov   dword[esp+4],red
  .loc:
    inc   [locked]
    ret
  .sq_check:
    popa
  .nored2:
    mov   eax,2
    repne scasb
    jecxz .loc
    lea   eax,[edi-field-1]
    div   dl
    xor   dh,dh
    push  ecx
    mov   ebx,1
    call  get_cell
    add   dh,[field+ecx]
    mov   ebx,3
    call  get_cell
    add   dh,[field+ecx]
    mov   ebx,7
    call  get_cell
    add   dh,[field+ecx]
    pop   ecx
    cmp   dh,6
    jne   .nored2
    ret

random:
    mov  eax, [generator]
    sub  eax,0x43ab45b5    ; next random number
    ror  eax,1
    xor  eax,0x32c4324f
    ror  eax,1
    mov  [generator],eax
    ret

dot_move:
    call random
    mov  eax,[generator]
    xor  edx,edx
    mov  ebx,MOBILITY
    div  ebx
    cmp  edx,8
    jb   .nostay
    mov  edx,16
  .nostay:
    mov  ebx,edx
    shr  ebx,1
    mov  eax,[dot]
;    dpd  eax
    mov  cl,FLDSIZE
    div  cl
    call get_cell
    mov  [dot],ecx
    cmp  byte[field+ecx],2
    jne  .nocap
    inc  [captured]
    cmp  [captured],2
    jne  .ex
  .next_lev:
    mov  [mode],0
    pop  eax
    mov  eax,[levptr]
    cmp  word[eax],0
    jne  .nxt
    mov  [mode],2
  .nxt:
    mov  dword[esp],red
    ret
  .nocap:
    and  [captured],0
  .ex:
    ret

if LEVELCONV eq 1
fileinfo:

     dd   2
     dd   0x0
     dd   0x0
.fsize dd   10000
.ptr dd   0x20000
     db   '/sys/newlev.bin',0

macro flush
{
  mov  [edi],dh
  inc  edi
}

compress_levels:
    mov  esi,raw_level
    mov  edi,I_END
    mov  [fileinfo.ptr],edi
    mov  ecx,(raw_level_size-raw_level)/(FLDSIZE*FLDSIZE*4) ; 19
  .lp1:
    push ecx
    mov  ecx,FLDSIZE*FLDSIZE
    mov  ebx,edi
    mov  eax,[esi]
;    movzx eax,byte[esi]
    movzx edx,al
    dpd  edx
    shl  eax,15
    stosw
  .lp2:
    lodsd ;lodsb
    cmp  al,0xd
    jne  .nored
    flush
    xor  al,al
    stosb
    add  esi,4
    dec  ecx
    xor  dh,dh
    jmp  .eloop
  .nored:
    cmp  al,dl
    jne  .change
    inc  dh
    cmp  dh,0xff
    jb   .eloop
    flush
    xor  dh,dh
    jmp  .eloop
  .change:
    flush
    xor  dl,1
    mov  dh,1
  .eloop:
    loop .lp2
    flush
    mov  eax,edi
    sub  eax,ebx
    add  [ebx],ax
    pop  ecx
    loop .lp1
    xor  eax,eax
    stosw
    sub  edi,I_END
    mov  [fileinfo.fsize],edi
    mcall 70,fileinfo
    ret

raw_level:
    file 'noname1.dat'
raw_level_size:

end if

decompress_level:
; esi - level begin
    mov  edi,field
    movzx edx,word[esi]
    xor  ecx,ecx
    mov  eax,edx
    and  edx,0x7fff
    add  edx,esi ; limit
    mov  [levptr],edx
    add  esi,2
    shr  eax,15
  .next:
    cmp  esi,edx
    jae  .exloop
    movzx ebx,byte[esi]
    inc  esi
    test ebx,ebx
    jne  .nored
    rep  stosb
    xor  al,1
    mov  word[edi],0x0202
    add  edi,2
    jmp  .next
  .nored:
    add  ecx,ebx
    cmp  ebx,0xff
    je   .next
    rep  stosb
    xor  al,1
    jmp  .next
  .exloop:
    rep  stosb
    ret

; DATA AREA

FS1 equ (FLDSIZE-1)
dirs  dw  FS1 shl 8,\
          1,\
          FS1,\
          1 shl 8,\
          FS1 shl 8+FS1,\
          1 shl 8+FS1,\
          FS1 shl 8+1,\
          1 shl 8+1,\
          0
level:
file 'rlevels.bin'

msg_ok db 'OK'
if lang eq ru
header db 'Red Square - Уровень       h - Помощь, p - Пауза', 0

messages mstr 'Отлично! Вы переходите на следующий уровень.',\
              'Красные клетки уничтожены. Вы проиграли.',\
              'Вы прошли все уровни. Поздравляем!',\
              'Игра приостановлена...'

desc mstr   '   Вы играете красным квадратиком. Цель игры - выжить среди',\
            'развивающихся черных клеток. Если Вам удастся поймать белый',\
            'крестик, Вы пройдете уровень.',\
            '   Чтобы поймать белый крестик, он должен быть накрыт одной',\
            'из красных клеток на протяжении 2 поколений подряд.',\
            '   Если красный квадрат разрушен, это еще не проигрыш. Может',\
            'быть, Вам повезет и если в процессе эволюции количество Ваших',\
            'красных клеток станет больше, чем черных, уровень будет',\
            "пройден.",\
            '---',\
            'Джон Хортон Конуэй изобрел прекрасную игру. Почти каждый',\
            'программист начинает свою практику с нее. Однако реальные',\
            'возможности этой игры еще плохо изучены.',\
            '---',\
            'Оригинал игры под Win32 написал Владимир Привалов, 2002',\
            'Запрограммировал на ассемблере Willow, 2005'
else
header db 'Red Square - Level         h - Help,  p - Pause', 0

messages mstr 'Well done! You are on the next level',\
              'Your red cells are vanished! Game over.',\
              "You've completed the game. Congratulations!",\
              'Game paused...'

desc mstr   '   The main goal of the game for your red square is to stay',\
            'alive going through the evolving black cells. If you manage to',\
            'catch the white cross, you will pass the level.',\
            '   To catch the white cross, it must be covered by red cell',\
            'during 2 generations.',\
            "   If your red square is corrupted, you haven't lost yet. You",\
            'may have a luck, and if your red cells is developing and the',\
            'quantity of black cells is bigger than that of black cells,',\
            "you'll pass the level.",\
            '---',\
            'John Horton Conway has created a great game. Almost every',\
            'programmer begins his professional work from it. But the real',\
            'possibilities of this game have not discovered yet.',\
            '---',\
            'Original game under Win32 by Vladimir Privalov, 2002',\
            'Programmed in assembly by Willow, 2005'
end if
I_END:
neib db ?,?
cells dw ?,?
mode db ?
levptr dd ?
levnum dd ?
reds dd ?
locked dd ?
generator dd ?
dot  dd ?
ticks dd ?
captured dd ?
if LEVELCONV eq 1
  os_work rb 4096
end if
field:
    rb FLDSIZE*FLDSIZE
rd 1
buff:
    rb FLDSIZE*FLDSIZE
stak:
