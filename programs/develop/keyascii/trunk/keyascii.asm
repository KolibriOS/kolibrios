;   Author: M. Lisovin
;   Compile with FASM for Menuet
;

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x1000                  ; memory for app
               dd     0x1000                  ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\macros.inc'

START:                          ; start of execution
  red: 
     call draw_window

still:

    mov  eax,10                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
    mov  [keyid],ah
    shr  eax,16
    mov  [scan_keyid],al
    call draw_window
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

    or   eax,-1                 ; close this program
    mcall
  noclose:

    jmp  still

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:
; function 12:tell os about windowdraw ; 1, start of draw
	mcall	12,1

                                   ; DRAW WINDOW
	mov	eax,0                     ; function 0 : define and draw window
	mov	ebx,100*65536+270         ; [x start] *65536 + [x size]
	mov	ecx,100*65536+100          ; [y start] *65536 + [y size]
	mov	edx,0x34ffffff            ; color of work area RRGGBB,8->color gl
	mov	edi,title
	mcall

; function 4 : write text to window
	xor	ecx,ecx
	mcall	4,<33,8>,,text1,6
	mcall	,<85,8>,,text2,9
	mcall	,<8,28>,,tdec,4
	add	ebx,23
	mcall	,,,thex

	movzx  ecx,byte [keyid]
	mcall	47,0x30000,,<40,28>,0x224466
	add	edx,23
	mov	bh,1
	mcall

	mov	bh,0
	movzx	ecx,byte [scan_keyid]
	mcall	,,,<100,28>
	add	edx,23
	mov	bh,1
	mcall

; function 12:tell os about windowdraw ; 2, end of draw
	mcall	12,2
	ret


; DATA AREA
 text1: db 'ASCII:'
 text2: db 'SCANCODE:'
 tdec: db 'DEC:'
 thex: db 'HEX:'
 title: db 'KEYBOARD ASCIICODES-PRESS ANY KEY',0
I_END:
 keyid: rb 1
 scan_keyid: rb 1


