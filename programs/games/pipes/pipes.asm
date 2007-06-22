;
;   pipes for menuet {and now kolibri}
;   v1.2
;   2006 by Mario Birkner
;
;   l.mod. 27.08.06/15:11
;
;   Compile with FASM
;
bgcolor  equ  0x0074744A      ;thx
fgcolor  equ  0x00E7C750      ;to
fg2color equ  0x00E0B0A0      ;colorref
fg3color equ  0x007F7F55
btcolor  equ  0x005B6200

include '..\..\macros.inc'

use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     I_END		      ; size of image
	       dd     0x100000		      ; memory for app
	       dd     0x7fff0		      ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

START:				; start of execution

     call draw_window
     call draw_board

still:

    mov  eax,10 		; wait here for event
    mcall

    cmp  eax,1			; redraw request ?
     je  red
    cmp  eax,2			; key in buffer ?
     je  key
    cmp  eax,3			; button in buffer ?
     je  button

    jmp  still

  red:				; redraw
    call draw_window
    call draw_board
    call draw_message
    jmp  still

  key:				; key
    mov  eax,2			; just read it and ignore
    mcall
    jmp  still
  button:			; button
    call get_input
    jmp  still



get_input:
pusha
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  .noclose

    mov  eax,-1 		; close this program
    mcall
  .noclose:
    cmp  ah,4
    jne  .moderate
    mov  [diffic],1
    jmp  .enddiffic
   .moderate:
    cmp  ah,3
    jne  .easy
    mov  [diffic],3
    jmp  .enddiffic
   .easy:
    cmp  ah,2
    jne  .board
    mov  [diffic],5
   .enddiffic:
    mov  [score],0
    mov  [speed],40
    mov  [level],1
    mov  [stat],0
    mov  [time],0
    call draw_window
    call scramble_board
    call draw_board
    call countdown
    call wassermarsch
    jmp  .getno
  .board:
    cmp  [stat],2
    jge  .getno
    shr  eax,8			; -> 24bit id
    cmp  eax,10
    jle  .getno
    cmp  eax,150
    jg	 .getno
    sub  eax,10
    mov  edi,eax
    add   edi,map
    cmp   [edi], byte 1
    jg	  .nogerade
    xor   byte [edi], 1
    call  draw_board
    jmp   .getno
  .nogerade:
    cmp   [edi], byte 6
    jge   .getno
    cmp   [edi], byte 5
    jne   .rota
    sub   byte [edi],4
  .rota:
    inc   byte [edi]
    call  draw_board
  .getno:
popa
ret
;//// end of event detection
get_direction:		    ;Setzt Richtungs-Konstanten
pusha			    ;IN:
mov eax,[esp+28]	    ;eax  -  Richtung IN
mov ebx,[esp+16]	    ;ebx  -  Teilchen (Map-Wert)
cmp ebx,0		    ;OUT:
jne .no0		    ;eax  -  Richtung OUT
  cmp eax,14
  jne .o0
  jmp .setout
  .o0:
  cmp eax,-14
  jne .col
  jmp .setout
.no0:
cmp ebx,1
jne .no1
  cmp eax,1
  jne .o1
  jmp .setout
  .o1:
  cmp eax,-1
  jne .col
  jmp .setout
.no1:
cmp ebx,2
jne .no2
  cmp eax,14
  jne .o2
  sub eax,13
  jmp .setout
 .o2:
  cmp eax,-1
  jne .col
  sub eax,13
  jmp .setout
.no2:
cmp ebx,3
jne .no3
  cmp eax,-14
  jne .o3
  add eax,15
  jmp .setout
 .o3:
  cmp eax,-1
  jne .col
  add eax,15
  jmp .setout
.no3:
cmp ebx,4
jne .no4
  cmp eax,-14
  jne .o4
  add eax,13
  jmp .setout
 .o4:
  cmp eax,1
  jne .col
  add eax,13
  jmp .setout
.no4:
cmp ebx,5
jne .no5
  cmp eax,14
  jne .o5
  sub eax,15
  jmp .setout
 .o5:
  cmp eax,1
  jne .col
  sub eax,15
  jmp .setout
.no5:
cmp ebx,6
jne .no6
  jmp .setout
.no6:
cmp ebx,7
jne .no7
  mov eax,14
  jmp .setout
.no7:
cmp ebx,8
jne .no8
  cmp eax,14
  jne .col
  mov [stat],1
  jmp .setout
.no8:
cmp ebx,16	  ; cross 2x
jne .col
  add [score],10  ; + 10 bonus points
  jmp .setout
.col:
xor eax,eax
.setout:
xor ebx,ebx
mov [esp+28],eax
mov [esp+16],ebx
popa
ret

countdown:
pusha
xor  eax,eax
mov  al,[diffic]
imul eax,10
mov  [time],eax
.udown:
call show_score
mov  ecx,10
.down:
mov  eax,5
mov  ebx,10
mcall
mov  eax,11
mcall
cmp  eax,1
jne  .nored
call draw_window
call draw_board
jmp  .nothing
.nored:
cmp  eax,3
jne  .nothing
call get_input
.nothing:
cmp  [stat],0	      ;bugfix 210806
jnz  .exitsub	      ;bugfix 210806
dec  ecx
jnz  .down
dec  [time]
jnz   .udown
.exitsub:	      ;bugfix 210806
popa
ret

wassermarsch:
pusha
   .restart:
     mov  esi,map+16	      ;start position
     mov  eax, 14	      ;start-richtung
   .findway:
     movzx ebx, byte [esi]
     call  get_direction
     test  eax,eax
     jz   .collision
     push  eax
      xor   eax,eax
      mov   al,6
      sub   al,[diffic]
      add   [score],eax 	 ;points/item = 6 - difficulty
      mov   ecx,dword [speed]
      add   byte [esi],10
      .down:
      mov   eax,5
      mov   ebx,2
      mcall
      mov   eax,11
      mcall
      cmp   eax,1
      jne   .nored
      call  draw_window
      .nored:
      cmp   eax,3
      jne   .noevnt
      call  get_input
      .noevnt:
      dec   ecx
      jnz   .down
     pop   eax

     add   esi,eax
     call  draw_board
     call  show_score
     jmp   .findway
   .collision:
    cmp [stat],1
    jne .loose
    call draw_message
    mov   eax,5
    mov   ebx,500
    mcall
    mov [stat],0
    inc [level]
    cmp [speed],6		 ;waterflowdelay < 6 ?
    jle .skipsub
    sub [speed],2
   .skipsub:
    call draw_window
    call scramble_board
    call draw_board
    call countdown
    jmp  .restart
   .loose:
    mov  [stat],2
    call draw_message
popa
ret

show_score:
pusha
mov  eax,13			 ;clear time and score area
mov  ebx,50 shl 16 +15
mov  ecx,395 shl 16 +15
mov  edx,bgcolor
mcall
add  ebx,60 shl 16 + 20
mcall
add  ebx,80 shl 16
mcall
mov  eax,47
mov  ebx,0x20000
mov  ecx,[time]
mov  edx,50*65536+398
mov  esi,fg2color
mcall
mov  ebx,0x50000
mov  ecx,[score]
add  edx,60 shl 16
mcall
mov  ebx,0x20000
mov  ecx,[level]
add  edx,80 shl 16
mcall

popa
ret



scramble_board:
pusha
mov edi,map+16 ;startpunkt
mov eax,7      ;wieder-
stosb	       ;herstellen

mov ebx, 0x00000007  ;modul         m max-wert
.loop_through:
mov   esi,edi
lodsb
cmp   eax, 9
 je   .skip
inc   eax
xor   edx, edx
div   ebx	    ;modulo -> edx
mov   eax, edx
cmp   eax,6
jne   .skip
dec   [half]
movzx eax, byte [half]
jnz   .skip
mov   [half], byte 7
.skip:
stosb
cmp edi,map+125 ;endpunkt erhalten
jge .exit
jmp .loop_through
.exit:
mov  eax,8
stosb
popa
ret


gen_image:
pusha
    xor   ebx,ebx	   ;default: kein wasser
    movzx eax,byte [map]   ;erstes byte der map lesen (position)
    inc   byte [map]	   ;position inkrementieren
    add   eax,map	   ;zur position die map-adresse addieren
    movzx  esi,byte [eax]
    cmp   esi,10
    jl	  .nowater
    sub   esi,10	  ;map-werte+10 sind mit wasser gefuellt
    mov   ebx,1
    cmp   esi,16
    jne   .nowater
    sub   esi,10
 .nowater:
   imul  esi,3072	  ;mapwert * 32*32*3 = image-adresse
    add  esi,images
    mov  edi,0x10000
    mov  ecx,32*32*3
 .gendd:		  ;RGB-Image im Speicher generieren
    mov   eax,dword [esi] ;byte aus imagemap lesen
    shl   eax,8
    shr   eax,8
    cmp   ebx,0
    jz	  .nowcolor
    mov   ebx,eax
    cmp   ebx,0x00B0B5B0
    jne   .nog1
    jmp   .wcolor
 .nog1:
    cmp   ebx,0x00A0A5A0
    jne   .nog2
    jmp   .wcolor
 .nog2:
    cmp   ebx,0x00909590
    jne   .nog3
    jmp   .wcolor
 .nog3:
    cmp   ebx,0x00808580
    jne   .nog4
    jmp   .wcolor
 .nog4:
    cmp   ebx,0x00707570
    jne   .nowcolor
    jmp   .wcolor
 .wcolor:
    add   eax,0x40
 .nowcolor:
    add  esi,3
    stosd
    dec  edi
    loop .gendd
popa
ret



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_message:
pusha
    cmp  [stat],0
    je	.nomessage
    cmp  [stat],3
    je	.nomessage
    mov  eax,13
    mov  ebx,146 shl 16 + 200
    mov  ecx,190 shl 16 + 40
    mov  edx,0x0
    mcall
    add  ebx,2 shl 16 - 4
    add  ecx,2 shl 16 - 4
    mov  edx,fgcolor
    mcall

    cmp   [stat],1
     je   .winmessage
    mov   eax,4
    mov   ebx,186 shl 16 +200
    mov   edx,lbl_gameover+1
    movzx esi,byte [lbl_gameover]
    mov   ecx,btcolor
    add   ecx,0x10000000
    mcall
    add   ebx,8 shl 16 +17
    mov   edx,lbl_yscore+1
    movzx esi,byte [lbl_yscore]
    mov   ecx,btcolor
    mcall
    mov   esi,ecx	;color
    mov   edx,ebx	;pos
    add   edx,80 shl 16
    mov   ebx,0x50000	 ;type
    mov   ecx,[score]	 ;inp
    mov   eax,47
    mcall
    jmp   .nomessage
   .winmessage:
    mov   eax,4
    mov   ebx,152 shl 16 +200
    mov   edx,lbl_win+1
    movzx esi,byte [lbl_win]
    mov   ecx,btcolor
    add   ecx,0x10000000
    mcall
    mov   ebx,152 shl 16 +217
    add   edx,esi
    mov   ecx,btcolor
    mcall
   .nomessage:
popa
ret

draw_board:
pusha
 mov  ebx,15*65536+32
 mov  ecx,50*65536+32
 mov  edx,15*65536+50		 ;Spielfeldposition
 mov  esi,10			  ;Spielfeldgroesse Y
 .vloop:
  mov  edi,14			 ;Spielfeldgroesse X
  .hloop:
    call gen_image
    push edx
    mov  eax,8
    movsx edx, byte [map]
    add  edx,9		    ;button-id = map-pos + 10;gen_image inkrements
    add  edx,0x50000000     ;no button image - no esi need
    mcall
    pop  edx
    push ebx
    push ecx
    mov  eax,7
    mov  ebx,0x10000
    mov  ecx,32 shl 16 +32
    mcall
    pop  ecx
    pop  ebx
    add  edx,33 shl 16
    add  ebx,33 shl 16
    dec  edi
    jnz  .hloop
  sub  edx,14*(33 shl 16)	 ;Spielfeldgroesse X
  sub  ebx,14*(33 shl 16)
  add  edx,33
  add  ecx,33 shl 16
  dec  esi
  jnz  .vloop
  mov  [map], byte 1		 ;Map-Position zuruecksetzen
popa
ret


draw_window:
pusha

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    mov  eax,0			   ; function 0 : define and draw window
    mov  ebx,100*65536+492	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+420	   ; [y start] *65536 + [y size]
    mov  edx,bgcolor		   ; color of work area RRGGBB,8->color gl
    or   edx,0x14000000
    mov  edi,title
    mcall

    mov   eax,8
    mov   ebx,84*65536+72
    mov   ecx,28*65536+15
    mov   edx,2
    mov   esi,btcolor
    mcall
    add   ebx,76 shl 16
    inc   edx
    mcall
    add   ebx,76 shl 16
    inc   edx
    mcall

    mov   eax,4
    mov   ebx,26 shl 16 +32
    mov   ecx,fgcolor
    mov   edx,lbl_toolbar+1
    movsx esi, byte [lbl_toolbar]
    mcall
    mov   ebx,18 shl 16 +398
    mov   edx,lbl_score+1
    movsx esi, byte [lbl_score]
    mcall
    mov   ebx,350 shl 16 +405
    mov   ecx,fg3color
    mov   edx,lbl_copy+1
    movsx esi,byte [lbl_copy]
    mcall

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    popa
    ret


; DATA AREA


title  db   'PIPES',0
lbl_gameover:
     db 19
     db 'G a m e   O v e r !'
lbl_win:
     db 32
     db '          G r e a t !           '
     db "        Let's goin'on!          "
lbl_yscore:
     db 11
     db 'Your Score:'
lbl_toolbar:
     db 43
     db 'New Game:     Easy       Moderate      Hard'
lbl_copy:
     db 23
     db 'v1.2 2006,Mario Birkner'
lbl_score:
     db 28
     db   'Time:    Score:       Level:'
stat	db 3  ;0=gameplay 1=won 2-lost 3=stopped
speed	db 0
time	dd 0
diffic	db 0  ;1=hard 3=moderate 5=easy 8=dedicated to Wildwest - try it out!
score	dd 0
level	dd 1
half	db 1  ;reduces the random-crosses

map:	   ;14*10 blocks + position
     db 1  ;<- act. position
     db 9,9,9,9,9,9,9,9,9,9,9,9,9,9
     db 9,7,1,3,2,0,1,1,0,3,4,4,3,9
     db 9,5,0,2,2,1,3,0,3,1,1,6,4,9
     db 9,4,0,4,6,0,3,3,2,6,0,1,2,9
     db 9,3,0,1,2,4,6,4,5,1,2,4,1,9
     db 9,5,3,2,6,3,2,1,2,1,2,6,0,9
     db 9,4,0,2,3,0,4,1,2,3,2,3,4,9
     db 9,2,0,4,5,6,3,1,3,0,4,1,0,9
     db 9,1,0,3,5,4,2,2,4,1,6,0,8,9
     db 9,9,9,9,9,9,9,9,9,9,9,9,9,9
images:
file 'pipes.raw'
I_END:




