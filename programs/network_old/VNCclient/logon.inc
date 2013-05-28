red_logon:
    call draw_window_logon	 ; at first, draw the window

still_logon:			 ; main cycle of application begins here
    mov  eax,10     ; wait here for event
    mcall

checkevent_logon:	  ; Check what event was called _logon: this will be used to return from textbox focus

    dec  eax	    ; redraw request ?
    jz	 red_logon
    dec  eax	    ; key in buffer ?
    jz	 key_logon
    dec  eax	    ; button in buffer ?
    jz	 button_logon

    jmp  still_logon

  key_logon:		 ; key event handler
    mov  al,2		 ; eax was zero so will now be 2
    mcall		 ; just read it and ignore

    cmp  ah,13
    jne  still_logon	 ; return to main loop

    ret 		 ; enter key was pressed => return to logon

  button_logon: 	 ; eax was zero so will now be 17
    mov  al,17		 ; get id
    mcall

    cmp  ah,1		 ; close ?
    jz	 close_logon
    cmp  ah,2		 ; logon ?
    je	 connect_logon
    cmp  ah,5		 ; first ?
    jz	 dstbtn_logon

  srcbtn_logon:
    mov  dword[addr],first
    jmp  rk_logon

  dstbtn_logon:
    mov  dword[addr],second

  rk_logon:
    mov  edi,[addr]	 ; load the address of the string
    xor  al,al		 ; mov  al,0          ; the symbol we will search for
    mov  ecx,STRLEN+1	 ; length of the string (+1)
    cld 		 ; search forward
  repne  scasb		 ; do search now
    inc  ecx		 ; we've found a zero or ecx became 0
    mov  eax,STRLEN+1
    sub  eax,ecx	 ; eax = address of <0> character
    mov  [temp],eax	 ; position

    cmp  dword[addr],dword second
    jne  @f
    mov  dword [passlen],eax
   @@:

    call print_text_logon

    mov  edi,[addr]	 ; address of string
    add  edi,[temp]	 ; cursor position

  .waitev_logon:
    mov  eax,10 	 ; wait for event
    mcall
    cmp  eax,2		 ; button presed ?
    jne  checkevent_logon    ; a key is pressed or redraw is nessesary, goto checkevent
    mcall		 ; eax = 2, read button
    shr  eax,8
    cmp  eax,8
    jnz  .nobs_logon	 ; BACKSPACE
    cmp  edi,[addr]
    jz	 .waitev_logon
    dec  edi
    mov  byte[edi],0

    cmp  dword[addr],second
    jne  @f
    dec  [passlen]
   @@:

    call print_text_logon
    jmp  .waitev_logon
  .nobs_logon:
    cmp  eax,13        ; ENTER
    je	 still_logon
    cmp  eax,192
    jne  .noclear_logon
    xor  al,al
    mov  edi,[addr]
    mov  ecx,STRLEN
    rep  stosb
    mov  edi,[addr]
    call print_text_logon
    jmp  .waitev_logon

  .noclear_logon:
    mov  [edi],al

    cmp  dword[addr],second
    jne  @f
    inc  [passlen]
   @@:

    call print_text_logon

    inc  edi
    mov  esi,[addr]
    add  esi,STRLEN
    cmp  esi,edi
    jnz  .waitev_logon

    jmp  still_logon


; print strings (source & destination)
print_text_logon:
pusha

    mov  eax, 8
    mov  ebx, 105*65536+200
    mov  ecx, 31*65536+13
    mov  edx, 4
    mov  esi, 0xEBEBEB
    mcall

    cmp  byte[mode],0
    je	 @f

    mov  ecx, 49*65536+12
    inc  edx
    mcall

   @@:
    mov  eax, 4 			; function 4 _logon: write text to window
    mov  ebx, 107*65536+34		; [x start] *65536 + [y start]
    xor  ecx, ecx			; color of text RRGGBB
    mov  edx, first			; pointer to text beginning
    mov  esi, STRLEN			; text length
    mcall

    cmp  byte[mode],0
    je	 dont_draw_pass

    add  ebx,16
    mov  edi,[passlen]

   @@:
    cmp  edi,0
    jle  dont_draw_pass

    dec  edi
    mov  edx, passchar
    mov  esi, 1
    mcall
    add  ebx,6*65536
    jmp  @r

  dont_draw_pass:

popa
    ret

close_logon:
    or	eax,-1
    mcall

connect_logon:
    ret

draw_window_logon:

    mcall 12, 1      ; start window draw
    pusha
					; DRAW WINDOW
    xor  eax, eax			; function 0 _logon: define and draw window
    mov  ebx, 160*65536+330		; [x start] *65536 + [x size]
    mov  ecx, 160*65536+100		; [y start] *65536 + [y size]
    mov  edx, 0x13DDDDDD		; color of work area RRGGBB
    mov  edi, title			; WINDOW LABEL
    mcall

    mov  eax, 8 			; LOGON BUTTON
    mov  ebx, 220*65536+85
    mov  ecx, 63*65536+16
    mov  edx, 2
    mov  esi, 0xCCCCCC
    mcall

    
    call print_text_logon
					
    cmp  byte[mode], 0
    je	 servermode_

    mov  eax, 4 			; function 4 write text to window
    mov  ebx, 25*65536+33		; [x start] *65536 + [y start]
    xor  ecx, ecx
    mov  edx, userstr			; pointer to text beginning
    mov  esi, passstr-userstr		; text length
    mcall

    add  bl,19
    mov  edx, passstr			; pointer to text beginning
    mov  esi, connect-passstr		; text length
    mcall

    jmp  drawtherest_

   servermode_:

    mov  eax, 4 			; function 4 write text to window
    mov  ebx, 25*65536+33		; [x start] *65536 + [y start]
    xor  ecx, ecx
    mov  edx, serverstr 		; pointer to text beginning
    mov  esi, userstr-serverstr 	; text length
    mcall

   drawtherest_:

    mov  ebx, 240*65536+67		; [x start] *65536 + [y start]
    mov  edx, connect			; pointer to text beginning
    mov  esi, connect_e-connect 	; text length
    mcall

    popa
    inc   ebx
    mcall

    ret


; DATA AREA
title  db 'Kolibrios VNC client by HIDNPLAYR',0

first:	      db '192.168.1.5'
	      rb STRLEN
second:       rb STRLEN

passchar      db '*'
passlen       dd 0

addr	      dd 0
temp	      dd 0
mode	      db 0 ; 0 = connection details, 1 = authentication

serverstr:    db 'server:'
userstr:      db 'username:'
passstr:      db 'password:'
connect:      db 'connect !'
connect_e:

I_END_logon:
