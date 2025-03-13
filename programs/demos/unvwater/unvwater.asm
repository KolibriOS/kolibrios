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
	dd     0,0


include '../../proc32.inc'
include '../../macros.inc'
include '../../KOSfuncs.inc'

macro start_draw_window x,y,xsize,ysize,areacolor,caption;,capsize
{
	mcall SF_REDRAW, SSF_BEGIN_DRAW
	mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
	push	eax
	lea	ecx, [y*65536+ysize+eax]; [y start] *65536 + [y size] + [skin_height]
	mov	ebx, x*65536+xsize	; [x start] *65536 + [x size]
	mov	edx, areacolor		; color of work area RRGGBB
	mov	edi, caption;0x00ddeeff          ; color of frames    RRGGBB
	mcall SF_CREATE_WINDOW
	pop	eax
}

SCREEN_WIDTH   equ    (600-10) ;10 px for borders
SCREEN_HEIGHT  equ    400


STARTAPP:
	mcall SF_SYSTEM, SSF_GET_CPU_FREQUENCY
	shr  eax, 28
	mov  dword [delay], eax

init_palette:
	mov  edi, Paleta
	mov  eax, 0x40
@@:
	stosd
	inc  al
	jnz  @b

MAIN:
	xor esi, esi
	mov ecx, SCREEN_WIDTH*SCREEN_HEIGHT
l11:
	xor  ebx, ebx
	mov  edx, 303h
	sub  esi, SCREEN_WIDTH+1
l2:
	cmp esi,0
	jge @f
		add esi, ecx
	@@:
	cmp esi, ecx
	jl @f
		sub esi, ecx
	@@:

	add  bl,  [esi+img]
	adc  bh,  ah
	inc  esi
	dec  dh
	jnz  l2

	mov  dh, 3
	add  esi, SCREEN_WIDTH-3
	dec  dl
	jnz  l2

	sub  esi, 2*SCREEN_WIDTH-1
	cmp esi,0
	jge @f
		add esi, ecx
	@@:
	cmp esi, ecx
	jl @f
		sub esi, ecx
	@@:

	mov  al, [img+esi]
	sub  bx, ax
	shl  ax, 2
	sub  bx, ax
	shr  bx, 2

	mov  ax, bx
	shr  ax, 7
	sub  bx, ax
	mov  [img+esi], bl
	inc  esi

	cmp esi, ecx
	jl l11

	call  copy_buffer_to_video


still:
	mcall   SF_CHECK_EVENT      ; Test if there is an event in the queue.

	cmp     al,EV_REDRAW
	jz      red
	cmp     al,EV_KEY
	jz      key
	cmp     al,EV_BUTTON
	jz      button

	jmp     MAIN

red:
	call    draw_window
	jmp     MAIN


key:
	mcall   SF_GET_KEY
	cmp     ah, 27              ; Test Esc in ASCII
	je      close_app
	jmp     MAIN

button:
	mcall   SF_GET_BUTTON       ; Get pressed button code
	cmp     ah, 1               ; Test x button
	je      close_app
	jmp     MAIN

draw_window:
	start_draw_window 100,70,SCREEN_WIDTH+9,SCREEN_HEIGHT+4,0x54224466,labelt;, 14;labellen-labelt
	mov     dword [skin_h], eax
	mcall   SF_REDRAW, SSF_END_DRAW
	ret


fail:
	; Type something here.
close_app:
	mcall   SF_TERMINATE_PROCESS



copy_buffer_to_video:
	pusha
	;mcall   SF_SYSTEM, SSF_WAIT_RETRACE
	mcall   SF_SLEEP, [delay]

	mov     eax, dword [skin_h]
	lea     edx, [5*65536+eax]

	mov     ecx, SCREEN_WIDTH*65536+SCREEN_HEIGHT ;ecx = w*65536+h
	;mov     edx, 5*65536+25 ;edx = x*65536+y
	mov     esi, 8
	mov     edi, Paleta
	xor     ebp, ebp
	mcall   SF_PUT_IMAGE_EXT, img
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
	rb SCREEN_WIDTH*SCREEN_HEIGHT
	rb 256
align 4
STACKTOP:
MEM:
