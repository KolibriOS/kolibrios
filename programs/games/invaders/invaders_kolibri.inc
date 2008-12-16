    use32
    org    0x0
    db	   'MENUET01'						  ; 8 byte id
    dd	   0x01 						  ; header version
    dd	   START						  ; start of code
    dd	   IM_END						  ; size of image
    dd	   I_END+1000						       ; memory for app
    dd	   I_END+1000						       ; esp
    dd	   0x0 , 0x0						  ; I_Param , I_Icon


KEY_RIGHT equ 179
KEY_LEFT equ 176
KEY_UP equ 178
KEY_P equ 'p'
KEY_DOWN equ 177
KEY_ENTER equ 13
KEY_ESC equ 27

START:
   mov	[drawroutine], draw_picture_kolibri
   call draw_window
   jmp	decodegif

draw_picture_kolibri:

    mov  eax,7							  ; Draw buffer to screen
    mov  ebx,vscreen+8
    mov  ecx,SCREEN_X*65536+SCREEN_Y
    mov  edx,SCREEN_X_POS*65536+SCREEN_Y_POS
    int  0x40

ret

waitandgetkey:

    mov  eax, 5
    mov  ebx, 1
    int  0x40

    mov  eax, 11						   ; check for event (no wait)
    int  0x40

    cmp  eax,1							  ; redraw request ?
    je	 red
    cmp  eax,2							  ; key in buffer ?
    je	 key
    cmp  eax,3							  ; button in buffer ?
    je	 button

    xor  ax,ax

    ret

  red:								  ; redraw
    call draw_window

    jmp  waitandgetkey

  key:								  ; key
    mov  eax,2							  ; read it
    int  0x40

    mov  al, ah

    ret

  button:							  ; button
    mov  eax,17 						  ; get id
    int  0x40

    cmp  ah,1							  ; button id=1 ?
    jne  noclose

  exit:
    mov  eax,-1 						  ; close this program
    int  0x40
  noclose:

    jmp  waitandgetkey


draw_window:
    mov  eax,12 						  ; function 12:tell os about windowdraw
    mov  ebx,1							  ; 1, start of draw
    int  0x40
								  ; DRAW WINDOW
    mov  eax,0							  ; function 0 : define and draw window
    mov  ebx,WINDOW_X*65536+10+SCREEN_X 			  ; [x start] *65536 + [x size]
    mov  ecx,WINDOW_Y*65536+30+SCREEN_Y 			  ; [y start] *65536 + [y size]
    mov  edx,0x04000000 					  ; color of work area RRGGBB,8->color gl
    mov  esi,0x805080d0 					  ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x005080d0						  ; color of frames    RRGGBB
    int  0x40

    mov  eax,71
    mov  ebx,1
    mov  ecx,header
    int  0x40

    call [drawroutine]

    mov  eax,12 						  ; function 12:tell os about windowdraw
    mov  ebx,2							  ; 2, end of draw
    int  0x40

ret

load_highscores:
ret

save_highscores:
ret

header          db 'Invaders',0