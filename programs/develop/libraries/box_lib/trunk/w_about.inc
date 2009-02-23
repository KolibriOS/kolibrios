;---------------------------------------------------------------------
thread3:

     call draw_window3

still3:

    mov  eax,10 		; wait here for event
    mcall

    cmp  eax,1			; redraw request ?
    je	 thread3
    cmp  eax,2			; key in buffer ?
    je	 key3
    cmp  eax,3			; button in buffer ?
    je	 button3

    jmp  still3

  key3:
    mcall
    cmp  ah,27
    je	 close3
    jmp  still3


  button3:			 ; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
	je   close3
	cmp  ah,2
    jne  noclose3
  close3:
    mov  eax,-1 		; close this program
    mcall
  noclose3:
     jmp  still3




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window3:


    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    xor  eax,eax		   ; function 0 : define and draw window
    mov  ebx,100*65536+200	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+100	   ; [y start] *65536 + [y size]
    mov  edx,0x03eeeeee 	   ; color of work area RRGGBB,8->color gl
    mcall

    mcall 4,<10,40>,0x80000000,header_1
    
    mov  ebx,70*65536+40
    mov  ecx,70*65536+20
    mov  edx,2
    mov  esi,0xdddddd
    mcall 8

    add  ebx,15 shl 16
	shr  ecx,16
	mov  bx,cx
	add  ebx,6
	
	mov  ecx,0
    mov  edx, ok_btn
    mov  esi,2
    mcall 4
	
				   ; WINDOW LABEL
    mcall 71,1, labelt3
	
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret
;---------------------------------------------------------------------