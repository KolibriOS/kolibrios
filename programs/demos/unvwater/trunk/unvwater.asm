;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; A unvwater demo
; Programmed by Octavio Vega Fernandez
; http://octavio.vega.fernandez.googlepages.com/CV4.HTM
; Converted to KolibriOS, By Asper
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

use32
        org     0x0

        db     'MENUET01'        ; header
        dd     1                 ; version
        dd     STARTAPP          ; program start
        dd     I_END             ; program image size
        dd     MEM               ; size memory for program
        dd     STACKTOP          ; pointer of stack
        dd     0
        dd     0

;macro start_draw_window x,y,xsize,ysize,areacolor,caption;,capsize
;{
;        mov     eax, 12                 ; function 12:tell os about windowdraw
;        mov     ebx, 1                  ; 1, start of draw
;        int     0x40
;        ; DRAW WINDOW
;        mov     eax, 48                 ; function 48.4 : get skin height
;        mov     ebx, 4
;        int     0x40
;        push    eax
;        lea     ecx, [y*65536+ysize+eax]; [y start] *65536 + [y size] + [skin_height]
;        xor     eax, eax                ; function 0 : define and draw window
;        mov     ebx, x*65536+xsize      ; [x start] *65536 + [x size]
;        mov     edx, areacolor          ; color of work area RRGGBB
;        ;mov     esi, 0x00334455        ; color of grab bar  RRGGBB
;        mov     edi, caption;0x00ddeeff          ; color of frames    RRGGBB
;        int     0x40
;        pop     eax
;}
;
;macro end_draw_window
;{
;        mov     eax, 12                 ; end of redraw
;        mov     ebx, 2
;        int     0x40
;}

include "aspapi.inc"
SCREEN_WIDTH   equ    100h
SCREEN_HEIGHT  equ    100h


STARTAPP:

  mov  eax, 18    ;Get CPU speed
  mov  ebx, 5
  int  0x40
  shr  eax, 28
  mov  dword [delay], eax

init_palette:
  mov  edi, Paleta
  ;xor  eax, eax
  mov  eax, 0x40
@@:
  stosd
  inc  al
  jnz  @b

MAIN:
l1:
  xor  esi, esi
l11:
  xor  ebx, ebx
  mov  edx, 303h
  sub  esi, 101h
l2:
  and  esi, 0xFFFF ;esi=si
  add  bl,  [esi+img]
  adc  bh,  ah
  inc  esi
  dec  dh
  jnz  l2

  mov  dh, 3
  add  esi, 100h-3
  dec  dl
  jnz  l2
  sub  esi, 1ffh
  and  esi, 0xFFFF ;esi=si

  mov  al, [img+esi]
  sub  bx, ax
  shl  ax, 2
  sub  bx, ax
  shr  bx, 2

  mov  ax, bx
  shr  ax, 7
  sub  bx, ax
  mov  [img+esi], bl
  inc  si
  jnz  l11

  call  copy_buffer_to_video


still:
        mov     eax, 11             ; Test if there is an event in the queue.
        int     0x40

        cmp     al,1                  ; redraw request ?
        jz      red
        cmp     al,2                  ; key in buffer ?
        jz      key
        cmp     al,3                  ; button in buffer ?
        jz      button

        jmp     MAIN

red:
        call    draw_window
        jmp     MAIN


key:
        mov     eax, 2
        int     0x40
        cmp     ah, 27              ; Test Esc in ASCII
        je      close_app
        jmp     MAIN

button:
        mov     eax, 17             ; Get pressed button code
        int     0x40
        cmp     ah, 1               ; Test x button
        je      close_app
        jmp     MAIN

draw_window:
        start_draw_window 100,70,SCREEN_WIDTH+9,SCREEN_HEIGHT+4,0x54224466,labelt;, 14;labellen-labelt
        mov     dword [skin_h], eax
        end_draw_window
ret


fail:
        ; Type something here.
close_app:
        mov     eax, -1  ; close this program
        int     0x40



copy_buffer_to_video:
        pusha
    ;    mov     eax, 18 ;@WAITVSYNC();
    ;    mov     ebx, 14
    ;    int     0x40
        mov     eax, 5  ;delay
        mov     ebx, dword [delay]
        int     0x40

        mov     eax, dword [skin_h]
        lea     edx, [5*65536+eax]

        mov     eax, 65
        mov     ebx, img
        mov     ecx, SCREEN_WIDTH*65536+SCREEN_HEIGHT ;ecx = w*65536+h
       ; mov     edx, 5*65536+25 ;edx = x*65536+y
        mov     esi, 8
        mov     edi, Paleta
        xor     ebp, ebp
        int     0x40
        popa
ret

; DATA AREA

; Application Title
labelt   db   'UnvWater demo',0
delay    dd   0
skin_h   dd   25 ; Skin height.

I_END:
Paleta   rb 1024
img      db 1
         rb 10000h

         rb 256
STACKTOP:
MEM:
