;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SPDX-License-Identifier: GPL-2.0-only
; Unvwater demo
; Copyright (C) 2010-2025 KolibriOS team
;
; Initially programmed by Octavio Vega Fernandez (c) in OctASM
; http://octavio.vega.fernandez.googlepages.com/CV4.HTM
; Converted to FASM for KolibriOS by Asper
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

Screen_W dd 600-10 ;10 px for borders
Screen_H dd 400

align 4
STARTAPP:
	mcall SF_SYS_MISC,SSF_HEAP_INIT
	call OnResize
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

align 4
MAIN:
	xor esi, esi
	mov edi, [img]
	mov ecx, [Screen_W]
	imul ecx, [Screen_H]
l11:
	xor  ebx, ebx
	mov  edx, 303h
	sub  esi, [Screen_W]
	dec  esi
l2:
	cmp esi,0
	jge @f
		add esi, ecx
	@@:
	cmp esi, ecx
	jl @f
		sub esi, ecx
	@@:

	add  bl,  [esi+edi]
	adc  bh,  ah
	inc  esi
	dec  dh
	jnz  l2

	mov  dh, 3
	add  esi, [Screen_W]
	sub  esi, 3
	dec  dl
	jnz  l2

	sub  esi, [Screen_W]
	sub  esi, [Screen_W]
	inc  esi
	cmp esi,0
	jge @f
		add esi, ecx
	@@:
	cmp esi, ecx
	jl @f
		sub esi, ecx
	@@:

	mov  al, [edi+esi]
	sub  bx, ax
	shl  ax, 2
	sub  bx, ax
	shr  bx, 2

	mov  ax, bx
	shr  ax, 7
	sub  bx, ax
	mov  [edi+esi], bl
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

align 4
draw_window:
	mcall   SF_REDRAW, SSF_BEGIN_DRAW
	mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
	mov     [skin_h], eax
	lea	ecx, [70*65536+4+eax]
	add ecx, [Screen_H] ; [y start] *65536 + [y size] + [skin_height]
	mov	ebx, 100 shl 16
	add ebx, [Screen_W]
	add ebx, 9
	mov	edi, labelt
	mcall SF_CREATE_WINDOW,,, 0x53224466

	mcall SF_THREAD_INFO,procinfo,-1
	mov eax, [skin_h]
	add eax, 4
	sub eax, [procinfo.box.height]
	neg eax
	cmp eax, [Screen_H]
	je .end_h
	cmp eax,32 ;min height
	jge @f
		mov eax,32
	@@:
		mov [Screen_H],eax
		xor eax,eax
		mov [Screen_W],eax
	.end_h:
	
	mov eax,[procinfo.box.width]
	sub eax,9
	cmp eax,[Screen_W]
	je .resize_end
	cmp eax,64 ;min width
	jge @f
		mov eax,64
	@@:
	mov [Screen_W],eax

	call OnResize
	.resize_end:

	mcall   SF_REDRAW, SSF_END_DRAW
	ret


fail:
	; Type something here.
close_app:
	mcall   SF_TERMINATE_PROCESS

align 4
OnResize:
	mov ecx,[Screen_W]
	imul ecx,[Screen_H]
	mcall SF_SYS_MISC,SSF_MEM_REALLOC,,[img]
	mov byte[eax],1 ; set the coordinate of the start of the first wave:
					; eax+0 -> point(0,0), eax+H*W+W -> point(W,H)
	mov [img],eax
	ret

align 4
copy_buffer_to_video:
	pusha
	;mcall   SF_SYSTEM, SSF_WAIT_RETRACE
	mcall   SF_SLEEP, [delay]

	mov     eax, [skin_h]
	lea     edx, [5*65536+eax]

	mov     ecx, [Screen_W]
	shl     ecx, 16
	add     ecx, [Screen_H]
	;mov     edx, 5*65536+25 ;edx = x*65536+y
	mov     esi, 8
	mov     edi, Paleta
	xor     ebp, ebp
	mcall   SF_PUT_IMAGE_EXT, [img]
	popa
	ret

; DATA AREA

; Application Title
labelt   db   'UnvWater demo',0
delay    dd   0
skin_h   dd   25 ; Skin height.

I_END:
Paleta   rb 1024
img      dd 0
procinfo process_information
	rb 256
align 4
STACKTOP:
MEM:
