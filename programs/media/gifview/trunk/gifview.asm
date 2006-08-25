; GIF VIEWER FOR MENUET v1.0
; Written in pure assembler by Ivushkin Andrey aka Willow
;
; Uses GIF_LITE 2.0
;
; Created:      August 31, 2004
; Last changed: August 25, 2006
;
; COMPILE WITH FASM

WND_COLOR equ 0x02aabbcc
; input line dimensions
INP_X equ 10 shl 16+680
INP_Y equ 25 shl 16+16
INP_XY equ 15 shl 16+30

use32

  org    0x0

  db     'MENUET01'
  dd     0x01
  dd     START
  dd     I_END
  dd     0x300000
  dd     0x27fff0
  dd     filename ;0x0
  dd     0x0

include 'lang.inc'
include 'macros.inc' ; decrease code size (optional)
;include '/hd/1/meos/debug.inc'
include 'debug.inc'
COLOR_ORDER equ MENUETOS

DELAY equ 20         ; animation speed

;include '/hd/1/gif/gif_lite.inc'
include 'gif_lite.inc'

START:
    cmp [filename],byte 0
    jne openfile2
;    jmp openfile2
openfile:
    and  [entered],0
    xor  eax,eax
    mov  [imgcount],eax
    mov  esi,fn_input
    mov  edi,filename
    mov  ecx,[inp_pos]
    rep  movsb
    stosb
openfile2:
    mov  eax,70
    mov  ebx,file_info
    int  0x40
    cmp  eax,6
    je   temp
    test eax,eax
    jnz  ok2
temp:
;    cmp  ebx,64
;    jbe  ok2

    and  [entered],0
    xor  eax,eax
    mov  [imgcount],eax
    mov  esi,filename
    mov  edi,fn_input
    mov  ecx,256/4  ;[filename_len]
    rep  movsd

    mov  edi,fn_input
    mov  ecx,256
    xor  eax,eax
    repne scasb
    sub  edi,fn_input
    dec  edi
    mov  [inp_pos],edi
    inc  [inp_pos]

;    test eax,eax
;    jnz  .ok2
;    cmp  ebx,64
;    jbe  .ok2
    mov  esi,workarea
    mov  edi,Image
    mov  eax,hashtable
    call ReadGIF
    test eax,eax
    jz   .ok
    xor  ecx,ecx
  .ok:
    mov  [imgcount],ecx
  ok2:
    and  dword[img_index],0

red:

    call draw_window

still:
        cmp     [imgcount], 1
        jnz     .delay
        mov     eax, 10
        int     0x40
        jmp     @f
.delay:
    mov  ebx,DELAY
    mov  eax,23
    int  0x40
@@:
        dec     eax
        jz      red
        dec     eax
        jz      key
        dec     eax
        jz      button
    mov  eax,[imgcount]
    cmp  eax,1
    je   still
    inc  [img_index]
    cmp  eax,[img_index]
    jne  redsub
    and  [img_index],0
  redsub:
    mov  ecx,[img_index]
    call draw_subimage
    jmp  still

  key:
    mov  eax,2
    int  0x40
    cmp  ah,13
    je   is_input
    jmp  still

  button:
    mov  eax,17
    int  0x40

    cmp  ah,1
    jne  noclose
  _close:
    or   eax,-1
    int  0x40

  noclose:
  is_input:             ; simple input line with backspace feature
    inc  [entered]      ; sorry - no cursor
  wait_input:
    call draw_input
    mov  eax,10
    int  0x40
    cmp  eax,2
    jne  still
    mov  edi,[inp_pos]
    mov  eax,2
    int  0x40
    shr  eax,8
    cmp  eax,27
    je   still
    cmp  eax,13
    je   openfile
    cmp  eax,8
    je   backsp
    mov  [fn_input+edi],al
    inc  [inp_pos]
    jmp  wait_input
  backsp:
    test edi,edi
    jz   wait_input
    dec  [inp_pos]
    jmp  wait_input
;    jmp  still

;****************************************
;******* DRAW CONTENTS OF INPUT LINE ****
;****************************************
draw_input:
    push edi
    cmp  [entered],0
    jne  highlight
    mov  esi,WND_COLOR
    jmp  di_draw
  highlight:
    mov  esi,0xe0e0e0
  di_draw:
    mov  eax,8
    mov  ebx,INP_X
    mov  ecx,INP_Y
    mov  edx,2
    int  0x40
    mov  eax,4
    mov  ecx,0x00107a30            ; шрифт 1 и цвет ( 0xF0RRGGBB )
    mov  ebx,INP_XY
    mov  edx,fn_input
    mov  esi,[inp_pos]
    int  0x40
    pop  edi
    ret

;   *********************************************
;   *******  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА *******
;   *********************************************

draw_window:

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  eax,0
    mov  ebx,50*65536+700
    mov  ecx,50*65536+500
    mov  edx,WND_COLOR
    mov  esi,0x805080d0
    mov  edi,0x005080d0
    int  0x40


    mov  eax,4
    mov  ebx,8*65536+8
    mov  ecx,0x10ddeeff
    mov  edx,zagolovok
    mov  esi,zag_konets-zagolovok
    int  0x40

    mov  eax,8
    mov  ebx,(700-19)*65536+12
    mov  ecx,5*65536+12
    mov  edx,1
    mov  esi,0x6688dd
    int  0x40

    call draw_input

    xor  ecx,ecx
    call draw_subimage
    cmp  [imgcount],1
    je   .enddraw

    mov  ecx,[img_index]
    call draw_subimage
  .enddraw:
    mov  eax,12
    mov  ebx,2
    int  0x40
    ret

draw_subimage:
    cmp  [imgcount],0
    jz   .enddraw
    mov  esi,Image
    mov  edi,gif_inf
    call GetGIFinfo
    test eax,eax
    jz   .enddraw
    movzx ebx,[gif_inf.Width]
    shl  ebx,16
    movzx ecx,[gif_inf.Height]
    add  ecx,ebx
    mov  ebx,eax
    movzx  eax,[gif_inf.Top]
    movzx  edx,[gif_inf.Left]
    shl  edx,16
    add  edx,eax
    add  edx,10 shl 16 +45
    mov  eax,7
    int  0x40
  .enddraw:
    ret

; Здесь находятся данные программы:

; интерфейс программы двуязычный - задайте язык в macros.inc

zagolovok:               ; строка заголовка
if lang eq ru
     db   'ПРОСМОТР GIF'
else
     db   'GIF  VIEWER'
end if
zag_konets:              ; и её конец

inp_pos    dd inp_end-fn_input
fn_input:
;   db '/hd/1/gif/smileys/sm100000.gif'
   db '/rd/1/meos.gif'
;    db '/hd/1/1/tex256.gif',0
;    db '/rd/1/tex256.gif'
inp_end:
     rb 256-(inp_end-fn_input)

file_info:
   dd 0
   dd 0
   dd 0
   dd 0x100000;0x200000
   dd workarea;0x100000
I_END:  ; конец программы
filename:
;   db '/hd/1/gif/smileys/sm112000.gif',0
;   db '/hd/1/gif/test.gif',0
;   db '/hd/1/gif/explode1.gif',0
;   db '/hd/1/gif/tapeta.gif',0
;   db '/hd/1/gif/meos.gif',0
   rb 257
;filename_len dd 0
entered    rd 1

imgcount  rd 1
img_index  rd 1
gif_inf  GIF_info

hashtable rd 4096
workarea rb 0x100000

Image:
