; GIF VIEWER FOR MENUET v1.0
; Written in pure assembler by Ivushkin Andrey aka Willow
;
; Uses GIF_LITE 2.0
;
; Created:      August 31, 2004
; Last changed: August 25, 2006
;
; COMPILE WITH FASM

appname equ 'GIF  VIEWER'

; input line dimensions
INP_X equ 5 shl 16+680
INP_Y equ 5 shl 16+16
INP_XY equ 10 shl 16+10

use32

  org    0x0

  db     'MENUET01'
  dd     0x01
  dd     START
  dd     I_END
  dd     0x400000
  dd     0x400000
  dd     filename ;0x0
  dd     0x0

include 'lang.inc'
include '..\..\..\macros.inc' ; decrease code size (optional)

COLOR_ORDER equ PALETTE
GIF_SUPPORT_INTERLACED = 1
include 'gif_lite.inc'

START:
    cmp [filename],byte 0
    jne openfile2
;    jmp openfile2
openfile:
    xor  eax,eax
    mov  [entered], eax
    mov  [gif_img_count],eax
    mov  esi,fn_input
    mov  edi,filename
    mov  ecx,[inp_pos]
    rep  movsb
    stosb
openfile2:
    mov  ebx,file_info
    mov  eax,70
    mcall
    cmp  eax,6
    je   temp
    test eax,eax
    jnz  ok2
temp:
;    cmp  ebx,64
;    jbe  ok2

    xor  eax,eax
    mov  [entered], eax
    mov  esi,filename
    mov  edi,fn_input
    mov  ecx,256/4  ;[filename_len]
    rep  movsd

    mov  edi,fn_input
    mov  ecx,256
    repne scasb
    sub  edi,fn_input
    mov  [inp_pos],edi

;    test eax,eax
;    jnz  .ok2
;    cmp  ebx,64
;    jbe  .ok2
    mov  esi,workarea
    mov  edi,Image
    call ReadGIF
    test eax,eax
    jz   .ok
    and  [gif_img_count], 0
  .ok:
  ok2:
    and  dword[img_index],0

    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    mov  eax,48
    mcall

red:

;   *********************************************
;   *******  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА *******
;   *********************************************

draw_window:

    mov  ebx,1
    mov  eax,12
    mcall

    xor  eax,eax
    mov  ebx,50*65536+700
    mov  ecx,50*65536+500
    mov  edx,[sc.work]
    or   edx,0x33000000
    mov  edi,title
    mcall

    call draw_input

    xor  ecx,ecx
    call draw_subimage
    cmp  [gif_img_count],1
    jz   @f

    mov  ecx,[img_index]
    call draw_subimage
@@:

    mov  ebx,2
    mov  eax,12
    mcall

still:
        cmp     [gif_img_count], 1
        jbe     .infinite
        mov     ebx, [cur_anim_delay]
        test    ebx, ebx
        jz      .infinite
        mov     eax, 23
        mcall
        jmp     @f
.infinite:
        mov     eax, 10
        mcall
@@:
        dec     eax
        jz      red
        dec     eax
        jz      key
        dec     eax
        jz      button
    mov  eax,[gif_img_count]
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
    mcall
    cmp  ah,13
    je   is_input
    jmp  still

  button:
    mcall 17
    cmp  ah,1
    jnz  wait_input

  _close:
    or   eax,-1
    mcall

  is_input:             ; simple input line with backspace feature
    inc  [entered]      ; sorry - no cursor
  wait_input:
    call draw_input
    mov  eax,10
    mcall
    cmp  al,2
    jne  still
    mov  edi,[inp_pos]
;    mov  eax,2
    mcall
    shr  eax,8
    cmp  al,27
    je   still
    cmp  al,13
    je   openfile
    cmp  al,8
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
    mov  esi,0xe0e0e0
    cmp  [entered],0
    jne  highlight
    mov  esi,0x00aabbcc
  highlight:
    mov  ecx,INP_Y
    mov  edx,2
    mov  ebx,INP_X
    mov  eax,8
    mcall
    mov  ecx,0x00107a30
    mov  edx,fn_input
    mov  esi,[inp_pos]
    mov  ebx,INP_XY
    mov  eax,4
    mcall
    ret

draw_subimage:
    cmp  [gif_img_count],0
    jz   .enddraw
    mov  esi,Image
    mov  edi,gif_inf
    call GetGIFinfo
    test eax,eax
    jz   .enddraw
    mov  ecx, dword [edi+GIF_info.Width-2]
    mov  cx, [edi+GIF_info.Height]
    mov  ebx,eax
    mov  eax, [edi+GIF_info.Delay]
    mov  [cur_anim_delay],eax
    mov  edx, dword [edi+GIF_info.Left-2]
    mov  dx, [edi+GIF_info.Top]
    add  edx,5 shl 16 +25
    mov  esi, 8
    mov  edi, [edi+GIF_info.Palette]
    xor  ebp, ebp
    mov  eax, 65
    mcall
  .enddraw:
    ret

; Здесь находятся данные программы:

title db appname,0               ; строка заголовка

inp_pos    dd inp_end-fn_input
fn_input:
;   db '/hd/1/gif/smileys/sm100000.gif'
   db '/sys/meos.gif'
;    db '/hd/1/1/tex256.gif',0
;    db '/sys/tex256.gif'
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
sc system_colors

gif_img_count  rd 1
cur_anim_delay rd 1
img_index  rd 1
gif_inf  GIF_info

IncludeUGlobals

workarea rb 0x100000

Image:
