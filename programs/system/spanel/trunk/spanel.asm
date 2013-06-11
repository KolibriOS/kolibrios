;
;   PANEL SETUP
;
;------------------------------------------------------------------------------
; last update:  09/04/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Code optimizing and refactoring.
;
;------------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'		; 8 byte id
	dd 0x01			; header version
	dd START		; start of code
	dd IM_END		; size of image
	dd I_END	;0x8000		; memory for app
	dd stack_top		; esp
	dd 0x0			; boot parameters
	dd 0x0			; path
;------------------------------------------------------------------------------
include '../../../macros.inc'
include 'lang.inc'
;------------------------------------------------------------------------------
START:
;------------------------------------------------------------------------------
align 4
red:
	call	draw_window
;------------------------------------------------------------------------------
align 4
still:
	mcall	10

	cmp	eax,1	; redraw request ?
	je	red

	cmp	eax,2	; key in buffer ?
	je	key

	cmp	eax,3	; button in buffer ?
	je	button

	jmp	still
;------------------------------------------------------------------------------
align 4
key:
	mcall	2

	shr	eax,8
	cmp	eax,'0'
	jb	still

	cmp	eax,'9'
	jg	still

	mov	edi,[ent]
	add	edi,text
	mov	esi,edi
	inc	esi
	mov	ecx,3
	cld
	rep	movsb

	mov	[edi],al

	jmp	red
;------------------------------------------------------------------------------
align 4
button:
	mcall	17

	cmp	ah,1	; button id=1 ?
	jne	noclose

	mcall	-1	; close this program
;--------------------------------------
align 4
noclose:
	cmp	ah,10
	jne	no_apply

	mov	esi,text+17
	mov	edi,panel_ini_data_area	;I_END+10
	mov	ecx,12
;--------------------------------------
align 4
newfe:
	mov	ebx,[esi]
	mov	[edi],ebx
	mov	[edi+4],byte ';'
	add	edi,5
	add	esi,55
	loop	newfe

	mov	[edi],byte 'x'
	mcall	70,dat_write
	mov	esi,1
;--------------------------------------
align 4
newread:
	inc	esi
	mcall	9,proc_info,esi
	cmp	esi,eax
	jg	all_terminated

	mov	eax,[ebx+10]
	and	eax,not 0x20202000
	cmp	eax,'@PAN'
	jne	newread

	mov	eax,[ebx+14]
	and	eax,not 0x2020
	cmp	ax,'EL'
	jne	newread

	mcall	18,2,esi

	mcall	5,5

	mov	esi,1
	jmp	newread
;--------------------------------------
align 4
all_terminated:
	mcall	5,25

	mcall	70,panel_start
;--------------------------------------
align 4
no_apply:
	cmp	ah,11
	jb	still

	shr	eax,8
	sub	eax,11
	imul	eax,55
	add	eax,17
	mov	[ent],eax
	mov	[text+eax],dword '0000'
	jmp	red
;------------------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
;------------------------------------------------------------------------------
align 4
draw_window:
	mcall	12,1
; DRAW WINDOW
	xor	eax,eax
	xor	esi,esi
	mcall	,<100,385>,<100,190>,0x14ffffff,,labelt

	mcall	8,<25,335>,<162,12>,10,0x80a0c0	;0x6677cc

	mov	ebx,340*65536+20
	mov	ecx,34*65536+10
	inc	edx	;11 - button
;--------------------------------------
align 4
newb:
	mcall
	add	ecx,10*65536
	inc	edx
	cmp	edx,23
	jb	newb

	mov	ebx,25*65536+35           ; draw info text with function 4
	mov	ecx,0x224466
	mov	edx,text
	mov	esi,55
	mov	eax,4
;--------------------------------------
align 4
newline:
	mcall
	add	ebx,10
	add	edx,55
	cmp	[edx],byte 'x'
	jne	newline

	mcall	12,2
	ret
;------------------------------------------------------------------------------
align 4
; DATA AREA
if lang eq it
	text:
		db 'largehzza        0000  :  0 for full screen width     <'
		db 'pulsanti         0000  :  0 no frames  , 1 frames     <'
		db 'soften_up        0001  :  0 no         , 1 si         <'
		db 'soften_down      0001  :  0 no         , 1 si         <'
		db 'minimize_left    0001  :  0 no         , 1 si         <'
		db 'minimize_right   0001  :  0 no         , 1 si         <'
		db 'posizione icone  0100  :  posizione in pixel          <'
		db 'menu_enable      0001  :  0 no         , 1 si         <'
		db 'setup_enable     0001  :  0 no         , 1 si         <'
		db 'graph_text       0001  :  0 grafica    , 1 text       <'
		db 'soften_middle    0001  :  0 no         , 1 si         <'
		db 'icone            0001  :  0 start      , 1 attivato   <'
		db '                                                       '
		db '                        Applica                        '
		db 'x'

	labelt:
		db 'Setup pannello'
	labellen:
else
	text:
		db 'width            0000  :  0 for full screen width     <'
		db 'buttons          0000  :  0 no frames  , 1 frames     <'
		db 'soften_up        0001  :  0 no         , 1 yes        <'
		db 'soften_down      0001  :  0 no         , 1 yes        <'
		db 'minimize_left    0001  :  0 no         , 1 yes        <'
		db 'minimize_right   0001  :  0 no         , 1 yes        <'
		db 'icons_position   0100  :  position in pixels          <'
		db 'menu_enable      0001  :  0 no         , 1 yes        <'
		db 'setup_enable     0001  :  0 no         , 1 yes        <'
		db 'graph_text       0001  :  0 graphics   , 1 text       <'
		db 'soften_middle    0001  :  0 no         , 1 yes        <'
		db 'icons            0001  :  0 start      , 1 activate   <'
		db '                                                       '
		db '                         APPLY                         '
		db 'x'

	labelt:
		db 'Panel setup'
	labellen:
end if
;------------------------------------------------------------------------------
align 4
ent	dd  17
;------------------------------------------------------------------------------
align 4
panel_start:
	dd 7
	dd 0
	dd 0
	dd 0
	dd 0
	db '/RD/1/@PANEL',0
;------------------------------------------------------------------------------
align 4
dat_write:
	dd 2
	dd 0
	dd 0
	dd 5*12+1
	dd panel_ini_data_area	;I_END+10
	db 'PANEL.DAT',0
;------------------------------------------------------------------------------
IM_END:
;------------------------------------------------------------------------------
align 4
proc_info:
	rb 1024
;------------------------------------------------------------------------------
align 4
	rb 1024
stack_top:
;------------------------------------------------------------------------------
align 4
panel_ini_data_area:
	rb 61
;------------------------------------------------------------------------------
I_END:
;------------------------------------------------------------------------------
