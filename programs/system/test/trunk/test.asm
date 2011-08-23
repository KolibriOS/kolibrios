;
;    PROTECTION TEST
;

use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     I_END		      ; size of image
	       dd     0x10000		      ; memory for app
	       dd     0xfff0		      ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

include '../../../macros.inc'

START:				; start of execution

    call draw_window		; at first, draw the window

still:

    mcall 10		     ; wait here for event

    cmp  eax,1			; redraw request ?
    jz	 red
    cmp  eax,2			; key in buffer ?
    jz	 key
    cmp  eax,3			; button in buffer ?
    jz	 button

    jmp  still

  red:				; redraw
    call draw_window

    jmp  still

  key:				; key
    mov  eax,2			; just read it and ignore
    int  0x40

    jmp  still

  button:			; button
    mov  eax,17
    int  0x40

    cmp  ah,1			; button id=1 ?
    jnz  noclose
    mov  eax,-1 	; close this program
    int  0x40
  noclose:

    cmp  ah,2
    jnz  notest2
    cli
  notest2:

    cmp  ah,3
    jnz  notest3
    sti
  notest3:

    cmp  ah,4
    jnz  notest4
     mov  [0x10000],byte 1
   notest4:

    cmp  ah,5
    jnz  notest5
    jmp  dword 0x10000
  notest5:

    cmp  ah,6
    jnz  notest6
    mov  esp,0
    push eax
  notest6:

    cmp  ah,7
    jnz  notest7
    in	 al,0x60
  notest7:

    cmp  ah,8
    jnz  notest8
    out  0x60,al
  notest8:




    jmp  still


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

	;mcall  48, 3, sys_colors, 40

    mcall 12, 1

    mcall 0, <200,292>, <200,230>, 0x14FFFFFF,,tlabel

    mov  eax,8			   ; function 8 : define and draw button
    mov  ebx,32*65536+10	    ; [x start] *65536 + [x size]
    mov  ecx,75*65536+10	    ; [y start] *65536 + [y size]
    mov  edx,2			   ; button id
    mov  esi,0x6888B8		   ; button color RRGGBB
  newb:
    int  0x40
    add  ecx,20*65536
    inc  edx
    cmp  edx,9
    jb	 newb

    cld
    mov  ebx,26*65536+37	   ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline


    mcall 12, 2 		   ; function 12:tell os about windowdraw


    ret

; DATA AREA


text:

    db 'Application uses 0x10000 bytes of memory'
    db '                                        '
    db 'Open debug board for rezult information '
    db '                                        '
    db '     CLI                                '
    db '                                        '
    db '     STI                                '
    db '                                        '
    db '     MOV [0x10000],BYTE 1               '
    db '                                        '
    db '     JMP DWORD 0x10000                  '
    db '                                        '
    db '     MOV ESP,0 & PUSH EAX               '
    db '                                        '
    db '     IN  Al,0x60                        '
    db '                                        '
    db '     OUT 0x60,AL                        '
    db 'x                                       '



tlabel:
    db	 'Kolibri protection test',0


I_END:
