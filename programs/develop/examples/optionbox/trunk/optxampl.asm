;
;   EXAMPLE APPLICATION
;
;   Compile with FASM for Menuet
;
include 'macros.inc'

use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     I_END		      ; size of image
	       dd     0x1000		      ; memory for app
	       dd     0x1000		      ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

START:				; start of execution

red:
     call draw_window

still:

    push 10 		; wait here for event
    pop  eax
    int  0x40

    dec  eax		; redraw request ?
    jz   red
    dec  eax		; key in buffer ?
    jz   key

  button:			; button
    mov  al,17 		; get id
    int  0x40

    cmp ah,2
    je button2

    cmp ah,3
    je button3

    cmp ah,4
    je button4

    cmp ah,5
    je button5

    cmp  ah,1		; button id=1 ?
    jne  noclose

    or   eax,-1 		; close this program
    int  0x40

  noclose:
    jmp  still

  button2:	;option 1
   call set_optionbox
  jmp still

  button3:	;option 2
   call set_optionbox
  jmp still

  button4:	;option 3
   call set_optionbox
  jmp still

  button5:
   mov	eax,4			         ; function 4 : write text to window
   mov	ebx,170*65536+35	         ; [x start] *65536 + [y start]
   mov	ecx,[sc.work_button_text]  ; font 1 & color ( 0xF0RRGGBB )
   mov	edx,options-1
   add	dl, [optionbox_checked]	   ; pointer to text beginning
   mov	esi,1	   ; text length
   int	0x40

  jmp still
  
key:				      ; key
   mov  al,2			; just read it and ignore
   int  0x40
   jmp  still


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40


    mov  eax,12 		         ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    int  0x40

				   ; DRAW WINDOW
    xor  eax,eax			   ; function 0 : define and draw window
    mov  ebx,100*65536+300	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+120	   ; [y start] *65536 + [y size]
    mov  edx,[sc.work] 	         ; color of work area RRGGBB,8->color gl
    or   edx,0x33000000
    mov  edi,header              ; WINDOW LABEL
    int  0x40

    mov  eax,8			   ; function 8 : define and draw button
    mov  ebx,165*65536+15	   ; [x start] *65536 + [x size]
    mov  ecx,30*65536+15	   ; [y start] *65536 + [y size]
    mov  edx,5			   ; button id
    mov  esi,[sc.work_button]	   ; button color RRGGBB
    int  0x40

  draw_optionboxes:
    ;draw optionbox1
    mov eax, 45 shl 16 + 20	   ; [x start] shl 16 + [y start]
    mov cl, 2			   ; button id
    mov edx, optiontext1	   ; pointer to text beginning
    call optionbox

    ;draw optionbox2
    mov eax, 45 shl 16 + 40	   ; [x start] shl 16 + [y start]
    mov cl, 3			   ; button id
    mov edx, optiontext2	   ; pointer to text beginning
    call optionbox

    ;draw optionbox3
    mov eax, 45 shl 16 + 60	   ; [x start] shl 16 + [y start]
    mov cl, 4			   ; button id
    mov edx, optiontext3	   ; pointer to text beginning
    call optionbox


    mov  eax,12 		         ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    int  0x40

    ret

include "optbox.inc"

; DATA AREA

optiontext1: db 'OptionBox1',0
optiontext2: db 'OptionBox2',0
optiontext3: db 'OptionBox3',0

options: db '0123'

header   db   'OptionBox example',0

I_END:

sc     system_colors

