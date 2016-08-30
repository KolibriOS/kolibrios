;
;   application     :  Tiny ray tracer
;   compilator	    :  Fasm
;   system	    :  MenuetOS64/KolibriOS
;   author	    :  Maciej Guba aka macgub
;   email	    :  macgub3@wp.pl
;   web 	    :  http://macgub.hekko.pl

timeout equ 3
XRES equ 500	    ; window size
YRES equ 500
maxx = XRES
maxy = YRES
use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     I_END		      ; size of image
	       dd     I_END		      ; memory for app
	       dd     I_END		    ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

START:				; start of execution

     call draw_window

still:

    mov  eax,23 		; wait here for event
    mov  ebx,timeout
    int  0x40
;    mov eax,11 		  ; check for event no wait
;    int 0x40

    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button

    jmp  noclose

  red:				; redraw
    call draw_window
    jmp  still

  key:				; key
    mov  eax,2			; just read it and ignore
    int  0x40
    jmp  still

  button:			; button
    mov  eax,17 		; get id
    int  0x40

    cmp  ah,1			; button id=1 ?
    jne  noclose

    mov  eax,-1 		; close this program
    int  0x40
  noclose:

; mov eax,13
; mov ebx,20*65536+maxx-25
; mov ecx,20*65536+maxy-25
; xor edx,edx
; int 0x40

 mov edi,screen
 mov ecx,maxx*maxy*3/4
 xor eax,eax
 cld
 rep stosd

 add [deg_counter],1
 cmp [deg_counter],360
 jne @f
 mov [deg_counter],0
@@:
 fninit
 fld	 [one_deg]
 fimul	 [deg_counter]
 fsincos
 fstp	 [sin]
 fstp	 [cos]

 mov  ecx,MAX_SPHERES	;MAX_LIGHTS
 mov  esi,non_rot_sphere  ;light
 mov  edi,sphere

.rotary:


 fld  dword[esi]
 fsub [xo]
 fmul [cos]
 fld  dword[esi+8]
 fsub [zo]
 fmul [sin]
 fchs
 faddp
 fadd [xo]   ; top of stack - new 'x'
 fstp dword[edi]

 fld  dword[esi+8]
 fsub [zo]
 fmul [cos]
 fld  dword[esi]
 fsub dword[xo]
 fmul [sin]
 faddp
 fadd [zo]
 fstp dword[edi+8]
; fld  dword[esi+4]
; fstp dword[edi+4]
 push dword[esi+4]
 pop  dword[edi+4]
 mov  dword[edi+12],0.0

 add  esi,12
 add  edi,16
 sub  ecx,1
 jnz  .rotary

 mov  ecx,MAX_LIGHTS
 mov  esi,non_rot_light
 mov  edi,light

.rotary_lights:


 fld  dword[esi]
 fsub [xo]
 fmul [cos]
 fld  dword[esi+4]
 fsub [yo]
 fmul [sin]
 fchs
 faddp
 fadd [xo]   ; top of stack - new 'x'
 fstp dword[edi]

 fld  dword[esi]
 fsub [xo]
 fmul [sin]
 fchs
 fld  dword[esi+4]
 fsub dword[yo]
 fmul [cos]
 faddp
 fadd [yo]
 fstp dword[edi+4]
; fld  dword[esi+8]
; fstp dword[edi+8]
 push dword[esi+8]
 pop  dword[edi+8]
 mov  dword[edi+12],0.0

 add  esi,12
 add  edi,16
 sub  ecx,1
 jnz  .rotary_lights



 call main_loop

 mov eax,7
 mov ebx,screen
 mov ecx,maxx*65536+maxy
 xor edx,edx
 int 0x40





jmp still



include 'ray.inc'

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 2, end of draw
    int  0x40

    mov  eax,48            ; get skin height
    mov  ebx,4
    int  0x40

    lea  ecx,[eax + (100 shl 16) + maxy+4]
    mov  edi,title
    xor  eax,eax
    mov  ebx,100*65536+maxx+9   ; [x start] *65536 + [x size]
    mov  edx,0x74000000    ; window type
	int  0x40

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    int  0x40

    ret

title db 'Ray tracing',0
xo dd 0.5
yo dd 0.5
zo dd 0.5
deg_counter dw 0
one_deg dd 0.017453
include 'dataray.inc'
sin dd ?
cos dd ?
screen rb XRES * YRES * 3
mem_stack:
 rb 65536
I_END:




