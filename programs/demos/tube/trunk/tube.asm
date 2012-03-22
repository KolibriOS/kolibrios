
;  (м) ( ) м ) ( )   256b intro by baze/3SC for Syndeecate 2001   use NASM to
;  плп лмл ллл ллм   loveC: thanks, Serzh: eat my socks dude ;]   compile the
;  ( ) ( ) ( ) ( )   e-mail: baze@stonline.sk, web: www.3SC.sk    source code

;  Menuet port by VT

appname equ 'TUBE - FPU'

use32
             org  0x0

             db   'MENUET01'
             dd   0x01
             dd   START
             dd   I_END
             dd   0x40000
             dd   0x40000
             dd   0,0

include '..\..\..\macros.inc'

START:

   call draw_window

   call init_tube

   push ebx

still:

   pop  ebx

   call MAIN

   push ebx

   mov  eax,23
   mov  ebx,1
   mcall

   cmp  eax,1
   jne  no_red
   call draw_window
   jmp  still
  no_red:

   cmp  eax,0
   je   still

   or   eax,-1
   mcall

SCREEN  equ 160
PIXBUF  equ 200h
EYE     equ EYE_P-2


MAIN:

 add    bh,10;8
 mov    edi,PIXBUF
 fadd   dword [di-PIXBUF+TEXUV-4]
 push   di
 mov    dx,-80

TUBEY:

 mov    bp,-160

TUBEX:

 mov    si,TEXUV
 fild   word [si-TEXUV+EYE]
 mov    [si],bp
 fild   word [si]
 mov    [si],dx
 fild   word [si]
 mov    cl,2

ROTATE:

 fld    st3
 fsincos
 fld    st2
 fmul   st0,st1
 fld    st4
 fmul   st0,st3
 db     0xde,0xe9 ; fsubp   st1,st0
 db     0xd9,0xcb ; fxch    st3
 fmulp  st2,st0
 fmulp  st3,st0
 faddp  st2,st0
 db     0xd9,0xca ; fxch    st2

 loop   ROTATE

 fld    st1
 db     0xdc,0xc8 ; fmul    st0,st
 fld    st1
 db     0xdc,0xc8 ; fmul    st0,st
 faddp  st1,st0
 fsqrt
 db     0xde,0xfb ; fdivp   st3,st0
 fpatan
 fimul  word [si-4]
 fistp  word [si]
 fimul  word [si-4]
 fistp  word [si+1]
 mov    si,[si]

 lea    ax,[bx+si]
 add    al,ah
 and    al,64
 mov    al,-5
 jz     STORE_1

 shl    si,2
 lea    ax,[bx+si]
 sub    al,ah
 mov    al,-16
 jns    STORE_1

 shl    si,1
 mov    al,-48

STORE_1:

; add    al,[ebx+esi+0x80000]
 add    [di],al
 inc    di

 inc    bp
 cmp    bp,160

EYE_P:

 jnz    TUBEX
 inc    dx
 cmp    dx,80
 jnz    TUBEY

 call   display_image

 pop    si
 mov    ch,SCREEN*320/256

BLUR:

 inc    si
 sar    byte [si],2
 loop   BLUR

 ret



display_image:

  pusha

  mov esi,PIXBUF
  mov edi,0x10000
 newp:
  movzx edx,byte [esi]
  shl edx,4
;  mov dh,dl
  mov [edi],edx

  add edi,3
  inc esi

  cmp esi,320*160+PIXBUF
  jbe newp

  mov eax,7
  mov ecx,320*65536+160
  xor edx,edx
  mov ebx,0x10000
  mcall

  popa
  ret



draw_window:

     pusha

     mov  eax,12
     mov  ebx,1
     mcall
     xor  eax,eax
     mov  ebx,100*65536+329
     mov  ecx,100*65536+186
     mov  edx,0x74000000
     mov  edi,title
     mcall
     mov  eax,12
     mov  ebx,2
     mcall
     popa
     ret

title db appname,0

db 41,0,0xC3,0x3C

TEXUV:

init_tube:

  mov ecx,256

PAL1:

  mov dx,3C8h
  mov ax,cx
  inc dx
  sar al,1
  js PAL2
  mul al
  shr ax,6

PAL2:

  mov al,0
  jns PAL3
  sub al,cl
  shr al,1
  shr al,1

PAL3:

  mov bx,cx
  mov [ebx+0x1000],bh
  loop PAL1
  mov  ecx,256

TEX:

  mov bx,cx
  add ax,cx
  rol ax,cl
  mov dh,al
  sar dh,5
  adc dl,dh
  adc dl,[ebx+255+0x1000]
  shr dl,1
  mov [ebx+0x1000],dl
  not bh
  mov [ebx+0x1000],dl
  loop TEX

  fninit
  fldz

  ret


I_END:




