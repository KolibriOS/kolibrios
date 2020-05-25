; C4
; Copyright (c) 2002 Thomas Mathys
; killer@vantage.ch
;
; This file is part of C4.
;
; C4 is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; C4 is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with C4; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	bits 32
	%include 'mos.inc'
	section .text
	%include 'lang_nasm.inc' ;fedesco


	MOS_HEADER01 start,end



;**********************************************************
; magic numbers
;**********************************************************

; initial player types
PL1TYPE_INIT		equ	0
PL2TYPE_INIT		equ	4

; window
WND_WIDTH		equ	259
WND_HEIGHT		equ	300
WND_WORKCOLOR		equ	0

; button dimensions
BUTTON_HEIGHT		equ	12

BUTTON_NEW_X		equ	14
BUTTON_NEW_Y		equ	30
%ifidn lang, 'it'
	BUTTON_NEW_WIDTH equ 56 + 28
%else
	BUTTON_NEW_WIDTH equ 56
%endif

BUTTON_SPIN_WIDTH	equ	8
BUTTON_PL1DN_X		equ	228
BUTTON_PL1DN_Y		equ	30
BUTTON_PL1UP_X		equ	(BUTTON_PL1DN_X + BUTTON_SPIN_WIDTH + 1)
BUTTON_PL1UP_Y		equ	BUTTON_PL1DN_Y

BUTTON_PL2DN_X		equ	BUTTON_PL1DN_X
BUTTON_PL2DN_Y		equ	(BUTTON_PL1DN_Y + 20)
BUTTON_PL2UP_X		equ	(BUTTON_PL2DN_X + BUTTON_SPIN_WIDTH + 1)
BUTTON_PL2UP_Y		equ	BUTTON_PL2DN_Y

; label dimensions
%ifidn lang, 'it'
	LABEL_PL1_X      equ   90 + 10
%else
	LABEL_PL1_X      equ   90
%endif
LABEL_PL1_Y		equ	(1 + BUTTON_PL1DN_Y + (BUTTON_HEIGHT-8)/2)
LABEL_PL2_X		equ	LABEL_PL1_X
LABEL_PL2_Y		equ	(1 + BUTTON_PL2DN_Y + (BUTTON_HEIGHT-8)/2)
%ifidn lang, 'it'
        LABEL_PL1TYPE_X      equ   (LABEL_PL1_X + 10*6 - 4)
%else
        LABEL_PL1TYPE_X      equ   (LABEL_PL1_X + 10*6)
%endif
LABEL_PL1TYPE_Y		equ	LABEL_PL1_Y
LABEL_PL2TYPE_X		equ	LABEL_PL1TYPE_X
LABEL_PL2TYPE_Y		equ	LABEL_PL2_Y
LABEL_STATUS_X		equ	14
LABEL_STATUS_Y		equ	279
LABEL_STATUS_WIDTH	equ	220
LABEL_STATUS_HEIGHT	equ	12



; board and stones
STONESIZE		equ	32			; stone height and width
GRIDX			equ	14			; upper left corner
GRIDY			equ	70
GRIDSPACING		equ	(STONESIZE + 1)		; space between lines
GRIDHEIGHT		equ	(6*GRIDSPACING+1)	; total grid width and height
GRIDWIDTH		equ	(7*GRIDSPACING+1)
GRIDCOLOR		equ	MOS_RGB(128,128,128)



; button id's
BT_QUIT			equ	1
BT_NEW			equ	2
BT_PLAYER1DN		equ	3
BT_PLAYER1UP		equ	4
BT_PLAYER2DN		equ	5
BT_PLAYER2UP		equ	6


start:
	jmp main

%include "pcx.inc"
%include "windows.inc"
%include "board.inc"
%include "rng.inc"
; %include "randomai.inc"
%include "ai.inc"



;**********************************************************
; main program
;**********************************************************
main:
	call randomize
	call defineWindow
	call decrunchImages
	call newGame

.msgpump:
	; wait for event
	mov ebx,1
	mov eax,MOS_SC_WAITEVENTTIMEOUT
	int 0x40

	; process events
	cmp eax,MOS_EVT_REDRAW
	je short .redraw
	cmp eax,MOS_EVT_KEY
	je short .key
	cmp eax,MOS_EVT_BUTTON
	je short .button

	call pollMouse
	call gameLoop
	jmp short .msgpump

.redraw:
	call defineWindow
	jmp short .msgpump
.key:
	call keyboardInput
	jmp short .msgpump
.button:
	call handleButton
	jmp short .msgpump



;**********************************************************
; button handling function
;**********************************************************
handleButton:
	mov eax,MOS_SC_GETPRESSEDBUTTON		; get button id
	int 0x40

	cmp al,1				; button pressed ?
	je short .bye				; nope -> nothing to do

	cmp ah,BT_QUIT				; which button has been pressed ?
	je short .quit
	cmp ah,BT_NEW
	je short .new
	cmp ah,BT_PLAYER1DN
	je short .player1dn
	cmp ah,BT_PLAYER1UP
	je short .player1up
	cmp ah,BT_PLAYER2DN
	je short .player2dn
	cmp ah,BT_PLAYER2UP
	je short .player2up
.bye:
	ret
.quit:
	MOS_EXIT
.new:
	call newGame
	ret
.player1dn:
	mov eax,[player1_type]		; get current type
	or eax,eax			; already zero ?
	jz .bla
	dec eax				; nope -> decrement
	mov [player1_type],eax		; write back
	mov edi,label_pl1type		; and update label
	call updatePlayerType
.bla:
	ret
.player1up:
	mov eax,[player1_type]		; get current type
	cmp eax,NPLAYERTYPES-1		; already max ?
	je .bla2
	inc eax				; nope -> increment
	mov [player1_type],eax		; write back
	mov edi,label_pl1type		; update label
	call updatePlayerType
.bla2:
	ret
.player2dn:
	mov eax,[player2_type]		; get current type
	or eax,eax			; already zero ?
	jz .bla3
	dec eax				; nope -> decrement
	mov [player2_type],eax		; write back
	mov edi,label_pl2type		; and update label
	call updatePlayerType
.bla3:
	ret
.player2up:
	mov eax,[player2_type]
	cmp eax,NPLAYERTYPES-1
	je .bla4
	inc eax
	mov [player2_type],eax
	mov edi,label_pl2type
	call updatePlayerType
.bla4:
	ret



;**********************************************************
; window definition function
;**********************************************************
defineWindow:
	MOS_STARTREDRAW

	mov edi,window
	call drawWindow

	mov edi,buttons
	mov ecx,NBUTTONS
	call drawButtons

	mov edi,labels
	mov ecx,NLABELS
	call drawLabels

	xor eax,eax
	call drawBoard

	MOS_ENDREDRAW
	ret



;**********************************************************
; updateStatusText
;
; input		:	esi = ptr to new string
; output	:	status bar is updated
; destroys	:	everything
;**********************************************************
updateStatusText:

	; different text ?
	cmp [statusbar + LABEL.caption],esi
	je .bye							; nope -> bye
	mov dword [statusbar + LABEL.caption],esi		; yeah -> save & redraw

	; clear background
     	mov ebx,MOS_DWORD(LABEL_STATUS_X,LABEL_STATUS_WIDTH)
     	mov ecx,MOS_DWORD(LABEL_STATUS_Y,LABEL_STATUS_HEIGHT)
     	xor edx,edx
     	mov eax,MOS_SC_DRAWBAR
     	int 0x40

     	; redraw label
	mov edi,statusbar
	mov ecx,1
	call drawLabels
.bye:
	ret



;**********************************************************
; updatePlayerType
; update player type label
; input:	eax = new type
;		edi = address label structure to update
;**********************************************************
updatePlayerType:
	mov ebx,PLAYERTYPELEN				; calculate type string address
	mul ebx
	add eax,playertypes
	mov [edi + LABEL.caption],eax			; write address
	mov ecx,1					; and redraw label
	call drawLabels
	ret



;**********************************************************
; board drawing stuff
;**********************************************************

; drawBoard
; draw whole board
;
; input		:	eax nonzero = clear board background
drawBoard:

	; clear background ?
	or eax,eax
	jz .noclear
	mov ebx,MOS_DWORD(GRIDX,GRIDWIDTH)
	mov ecx,MOS_DWORD(GRIDY,GRIDHEIGHT)
	mov edx,WND_WORKCOLOR
	mov eax,MOS_SC_DRAWBAR
	int 0x40
.noclear:
	call drawGrid
	call drawStones
	ret



drawGrid:

	; vertical lines
	mov ebx,MOS_DWORD(GRIDX,GRIDX)
	mov ecx,MOS_DWORD(GRIDY,GRIDY+GRIDHEIGHT-1)
	mov edx,GRIDCOLOR
	mov eax,MOS_SC_DRAWLINE
	mov esi,8
.vlines:
	int 0x40
	add ebx,MOS_DWORD(GRIDSPACING,GRIDSPACING)
	dec esi
	jnz .vlines

	; horizontal lines
	mov ebx,MOS_DWORD(GRIDX,GRIDX+GRIDWIDTH-1)
	mov ecx,MOS_DWORD(GRIDY,GRIDY)
	mov esi,7
.hlines:
	int 0x40
	add ecx,MOS_DWORD(GRIDSPACING,GRIDSPACING)
	dec esi
	jnz .hlines

	ret


drawStones:
	mov ebx,6
.col:
	mov ecx,7
.row:
	call drawStone
	loop .row
	dec ebx
	jnz .col
	ret



; ecx = column (1..7)
; ebx = row (1..6)
drawStone:
	pushad

	; see which image to draw.
	; the image offset is stored in ebp
	mov eax,BWIDTH			; calculate address
	mul ebx
	add eax,ecx
	mov eax,[board+eax*4]		; get stone ?
	cmp eax,EMPTY			; empty field -> nothing to do
	je .bye
	mov ebp,redstone		; assume red stone
	cmp eax,PLAYER1			; red ?
	je .stoneok			; yeah -> continue
	mov ebp,bluestone		; nope -> use blue stone
.stoneok:

	; calculate image position (edx)
	mov eax,GRIDSPACING
	dec ecx
	mul ecx
	add eax,GRIDX + 1
	shl eax,16
	mov ecx,eax
	mov eax,GRIDSPACING
	dec ebx
	mul ebx
	add eax,GRIDY + 1
	mov cx,ax
	mov edx,ecx

	; put image (position is already in edx)
	mov ebx,ebp				; image address
	mov ecx,MOS_DWORD(STONESIZE,STONESIZE)	; image dimensions
	mov eax,MOS_SC_PUTIMAGE
	int 0x40

.bye:
	popad
	ret



decrunchImages:
	mov esi,redpcx			; red stone
	mov edi,redstone
	mov ebx,REDPCXSIZE
	call loadPCX
	mov esi,bluepcx			; blue stone
	mov edi,bluestone
	mov ebx,BLUEPCXSIZE
	call loadPCX
	ret



resetInput:
	mov dword [playerinput],0	; no player input
	mov dword [mouseinput],0
	ret



;**********************************************************
; newGame
; set up everything for a game
;
; input		:	nothing
; output	:	nothing
; destroys	:	everything
;**********************************************************
newGame:
	call boardReset			; reset and redraw board
	mov eax,1
	call drawBoard
	call resetInput			; reset input
	mov dword [gameover],0		; game is running
	ret



;**********************************************************
; pollMouse
; mouse polling routine
;
; input		:	nothing
; output	:	playerinput will be updated, if
;			the player clicked on a valid
;			field
; destroys	:	everything
;**********************************************************
pollMouse:
	mov ebx,2
	mov eax,MOS_SC_GETMOUSEPOSITION
	int 0x40
	and eax,1
	jz .mousenotpressed
.mousepressed:
	mov dword [mouseinput],0
	call isActiveApp
	or al,al
	jz .notactive1
	call getMouseCol
	mov [mouseinput],eax
.notactive1:
	ret
.mousenotpressed:
	call isActiveApp
	or al,al
	jz .notactive2
	call getMouseCol
	cmp eax,[mouseinput]
	jne .nonewinput
	cmp dword [playerinput],0
	jne .nonewinput
	mov [playerinput],eax
.nonewinput:
.notactive2:
	mov dword [mouseinput],0
	ret



;**********************************************************
; getMouseCol
; calculate in which column the mouse is. or so.
;
; input		:	nothing
; output	:	eax = 0 -> mouse outside board
;			eax = 1..7 -> column
; destroys	:	everything
;**********************************************************
getMouseCol:

	mov ebx,1			; get mouse position, window relative
	mov eax,MOS_SC_GETMOUSEPOSITION
	int 0x40

	movzx ebx,ax			; y clipping
	cmp ebx,GRIDY
	jl .outside
	cmp ebx,GRIDY + GRIDHEIGHT - 1
	jg .outside

	shr eax,16			; calculate column from x coordinate
	sub eax,GRIDX
	js .outside			; negative -> outside of board (left)
	cdq				; !
	mov ebx,GRIDSPACING
	div ebx
	cmp eax,BWIDTH-3		; right outside of board ?
	jg .outside			; yes -> bye

	inc eax				; xform into range [1,7]
	ret
.outside:
	xor eax,eax
	ret



;**********************************************************
; isActiveApp
; check wether we're the active application
;
; input		:	nothing
; output	:	al nonzero -> we are the active app
; destroys	:	everything
;**********************************************************
isActiveApp:

%define	PROCINFO (ebp-MOS_PROCESSINFO_size)

	enter MOS_PROCESSINFO_size,0

	; get process information
	mov eax,MOS_SC_GETPROCESSINFO
	lea ebx,[ebp-MOS_PROCESSINFO_size]
	mov ecx,-1
	int 0x40

	; set al to 1 if we are the active application
	cmp ax,[PROCINFO+MOS_PROCESSINFO.windowStackPos]
	sete al

	leave
	ret
%undef PROCINFO



;**********************************************************
; keyboardInput
; keyboard input handler, called from main loop
;
; input		:	nothing
; output	:	playerinput is updated
; destroys	:	everything
;**********************************************************
keyboardInput:
	mov eax,MOS_SC_GETKEY		; get key
	int 0x40
	or al,al			; key available ?
	jnz .bye			; no -> bye
	cmp dword [playerinput],0	; unprocessed input available ?
	jne .bye			; yes -> bye

	sub ah,'1'			; valid key ?
	cmp ah,BWIDTH-3
	ja .bye				; treat as unsigned : keys below '1' will
					; be greater too =)

	mov al,ah			; save input
	and eax,255
	inc eax
	mov [playerinput],eax

.bye:
	ret



;**********************************************************
; gameLoop
; game logic code or however you wish to call it.
; actually this is not a loop, but is called from
; the main loop
;**********************************************************
gameLoop:

	; if the game is over, return
	cmp dword [gameover],0
	je .gamerunning
	ret
.gamerunning:

	call updatePlayerStatusText

	; get move
	call getMoveForCurrentPlayer
	or eax,eax
	jnz .moveok
	ret			; no move available -> bye
.moveok:

	; make move and update board graphics
	mov ebx,[currentplayer]	; ebx = current player, eax contains already move
	call boardMakeMove
	call drawStones

	; check wether game is over (either by a win or because the board is full)
	mov eax,[currentplayer]			; win for current player ?
	call boardIsWin
	or eax,eax
	jz .nowin				; no -> continue
	mov esi,player1wins			; yes -> display message...
	cmp dword [currentplayer],PLAYER1
	je .blubb
	mov esi,player2wins
.blubb:
	call updateStatusText
	mov dword [gameover],1			; ...and end game
	ret
.nowin:
	BOARDISFULL				; board full, but no win ?
	jnz .notfull				; no -> continue
	mov esi,itisadraw			; yes -> display message...
	call updateStatusText
	mov dword [gameover],1			; ...and end game
	ret
.notfull:

	; switch players and return to main loop
	BOARDSWITCHPLAYERS
	ret



;**********************************************************
; getMoveForCurrentPlayer
; returns the move made by the current player
; (either cpu or human)
;
; input		:	nothing
; output	:	eax = 0 -> no move made. this is
;			usually the case for human players,
;			when no valid input is available.
;			else eax = move number
;**********************************************************
getMoveForCurrentPlayer:

	; get type of current player
	mov eax,[player1_type]
	cmp dword [currentplayer],PLAYER1
	je .ok
	mov eax,[player2_type]
.ok:

	; get move for human/cpu player
	or eax,eax
	jnz .cpu
.human:
	mov eax,[playerinput]	; get input
	or eax,eax		; input available ?
	jz .nomove		; no -> return no move available
	call resetInput		; !
	BOARDISVALIDMOVE eax	; valid move `?
	jz .nomove		; no -> return no move available
	ret			; valid move available -> return it (eax)

.cpu:
	call dword [aicode]	; call ai machine. cpu level is already in eax
	ret
.nomove:
	xor eax,eax
	ret



;**********************************************************
; update status bar : which player's turn it is
;**********************************************************
updatePlayerStatusText:
	cmp dword [currentplayer],PLAYER2
	je .player2
	mov esi,player1hmnprmpt
	cmp dword [player1_type],0
	je .statustextok
	mov esi,player1cpuprmpt
	jmp short .statustextok
.player2:
	mov esi,player2hmnprmpt
	cmp dword [player2_type],0
	je .statustextok
	mov esi,player2cpuprmpt
.statustextok:
	call updateStatusText
	ret



;**********************************************************
; initialized data
;**********************************************************

	section .data

;
; window definition
;
windowtitle	db	"C4",0
window:
istruc WND
	at WND.xposandsize,	dd	MOS_DWORD(0,WND_WIDTH)
	at WND.yposandsize,	dd	MOS_DWORD(0,WND_HEIGHT)
	at WND.workcolor,	dd	0x14000000 | WND_WORKCOLOR
	at WND.grabcolor,	dd	0
	at WND.framecolor,	dd	0
	at WND.caption,		dd	windowtitle
	at WND.captioncolor,	dd	0
	at WND.flags,		dd	WND_CENTER | WND_DEFAULT_GRABCOLOR | WND_DEFAULT_FRAMECOLOR | WND_DEFAULT_CAPTIONCOLOR
iend

;
; button table
;
buttons:
istruc BUTTON						; new
	at BUTTON.xposandsize
	dd MOS_DWORD(BUTTON_NEW_X,BUTTON_NEW_WIDTH)
	dd MOS_DWORD(BUTTON_NEW_Y,BUTTON_HEIGHT)
	dd BT_NEW
	dd BUTTON_COLOR_WORK
iend
istruc BUTTON						; player 1 down
	at BUTTON.xposandsize
	dd MOS_DWORD(BUTTON_PL1DN_X,BUTTON_SPIN_WIDTH)
	dd MOS_DWORD(BUTTON_PL1DN_Y,BUTTON_HEIGHT)
	dd BT_PLAYER1DN
	dd BUTTON_COLOR_WORK
iend
istruc BUTTON						; player 1 up
	at BUTTON.xposandsize
	dd MOS_DWORD(BUTTON_PL1UP_X,BUTTON_SPIN_WIDTH)
	dd MOS_DWORD(BUTTON_PL1UP_Y,BUTTON_HEIGHT)
	dd BT_PLAYER1UP
	dd BUTTON_COLOR_WORK
iend
istruc BUTTON						; player 2 down
	at BUTTON.xposandsize
	dd MOS_DWORD(BUTTON_PL2DN_X,BUTTON_SPIN_WIDTH)
	dd MOS_DWORD(BUTTON_PL2DN_Y,BUTTON_HEIGHT)
	dd BT_PLAYER2DN
	dd BUTTON_COLOR_WORK
iend
istruc BUTTON						; player 2 up
	at BUTTON.xposandsize
	dd MOS_DWORD(BUTTON_PL2UP_X,BUTTON_SPIN_WIDTH)
	dd MOS_DWORD(BUTTON_PL2UP_Y,BUTTON_HEIGHT)
	dd BT_PLAYER2UP
	dd BUTTON_COLOR_WORK
iend
NBUTTONS	equ	(($-buttons)/BUTTON_size)


;
; label table
;
%ifidn lang, 'it'
       newgame db   "Nuova partita",0
%else
       newgame db   "New game",0
%endif
down		db	"<",0
up		db	">",0
%ifidn lang, 'it'
       pl1		db	"Giocatore 1:",0
       pl2		db	"Giocatore 2:",0
%else
       pl1		db	"Player 1:",0
       pl2		db	"Player 2:",0
%endif

%ifidn lang, 'it'
	playertypes:
			db	"Umano",0
	PLAYERTYPELEN	equ	($ - playertypes)
			db	"CPU 1 ",0
			db	"CPU 2 ",0
			db	"CPU 3 ",0
			db	"CPU 4 ",0
			db	"CPU 5 ",0
			db	"CPU 6 ",0
			db	"CPU 7 ",0
			db	"CPU 8 ",0
%else
	playertypes:
			db	"Human       ",0
	PLAYERTYPELEN	equ	($ - playertypes)
			db	"CPU level 1 ",0
			db	"CPU level 2 ",0
			db	"CPU level 3 ",0
			db	"CPU level 4 ",0
			db	"CPU level 5 ",0
			db	"CPU level 6 ",0
			db	"CPU level 7 ",0
			db	"CPU level 8 ",0
%endif

NPLAYERTYPES	equ	(($-playertypes)/PLAYERTYPELEN)


labels:
istruc LABEL						; new
	at LABEL.position
	dd MOS_DWORD(BUTTON_NEW_X+4,1+BUTTON_NEW_Y+(BUTTON_HEIGHT-8)/2)
	dd newgame
	dd LABEL_COLOR_WORKBUTTON
	dd LABEL_BGCOLOR_TRANSPARENT
iend
istruc LABEL						; player 1 down
	at LABEL.position
	dd MOS_DWORD(BUTTON_PL1DN_X+(BUTTON_SPIN_WIDTH-4)/2,1+BUTTON_PL1DN_Y+(BUTTON_HEIGHT-8)/2)
	dd down
	dd LABEL_COLOR_WORKBUTTON
	dd LABEL_BGCOLOR_TRANSPARENT
iend
istruc LABEL						; player 1 up
	at LABEL.position
	dd MOS_DWORD(1+BUTTON_PL1UP_X+(BUTTON_SPIN_WIDTH-4)/2,1+BUTTON_PL1UP_Y+(BUTTON_HEIGHT-8)/2)
	dd up
	dd LABEL_COLOR_WORKBUTTON
	dd LABEL_BGCOLOR_TRANSPARENT
iend
istruc LABEL						; player 2 down
	at LABEL.position
	dd MOS_DWORD(BUTTON_PL2DN_X+(BUTTON_SPIN_WIDTH-4)/2,1+BUTTON_PL2DN_Y+(BUTTON_HEIGHT-8)/2)
	dd down
	dd LABEL_COLOR_WORKBUTTON
	dd LABEL_BGCOLOR_TRANSPARENT
iend
istruc LABEL						; player 2 up
	at LABEL.position
	dd MOS_DWORD(1+BUTTON_PL2UP_X+(BUTTON_SPIN_WIDTH-4)/2,1+BUTTON_PL2UP_Y+(BUTTON_HEIGHT-8)/2)
	dd up
	dd LABEL_COLOR_WORKBUTTON
	dd LABEL_BGCOLOR_TRANSPARENT
iend
istruc LABEL						; player 1
	at LABEL.position
	dd MOS_DWORD(LABEL_PL1_X,LABEL_PL1_Y)
	dd pl1
	dd MOS_RGB(255,255,255)
	dd LABEL_BGCOLOR_TRANSPARENT
iend
istruc LABEL						; player 2
	at LABEL.position
	dd MOS_DWORD(LABEL_PL2_X,LABEL_PL2_Y)
	dd pl2
	dd MOS_RGB(255,255,255)
	dd LABEL_BGCOLOR_TRANSPARENT
iend
statusbar:						; status bar
istruc LABEL
	at LABEL.position
	dd MOS_DWORD(LABEL_STATUS_X,LABEL_STATUS_Y)
	dd 0
	dd MOS_RGB(255,255,255)
	dd LABEL_BGCOLOR_TRANSPARENT
iend
label_pl1type:
istruc LABEL
	at LABEL.position
	%ifidn lang, 'it'
		dd MOS_DWORD(LABEL_PL1TYPE_X + 18,LABEL_PL1TYPE_Y)
	%else
		dd MOS_DWORD(LABEL_PL1TYPE_X,LABEL_PL1TYPE_Y)
	%endif
	dd playertypes+PL1TYPE_INIT*PLAYERTYPELEN
	dd MOS_RGB(255,255,255)
	dd MOS_RGB(0,0,0)
iend
label_pl2type:
istruc LABEL
	at LABEL.position
	%ifidn lang, 'it'
		dd MOS_DWORD(LABEL_PL2TYPE_X + 18,LABEL_PL2TYPE_Y)
	%else
		dd MOS_DWORD(LABEL_PL2TYPE_X,LABEL_PL2TYPE_Y)
	%endif
	dd playertypes+PL2TYPE_INIT*PLAYERTYPELEN
	dd MOS_RGB(255,255,255)
	dd MOS_RGB(0,0,0)
iend
NLABELS		equ	(($-labels)/LABEL_size)


; player types
player1_type	dd	PL1TYPE_INIT
player2_type	dd	PL2TYPE_INIT


; status messages
%ifidn lang, 'it'
	player1hmnprmpt	db	"Turno del giocatore 1",0
	player2hmnprmpt db	"Turno del giocatore 2",0
	player1cpuprmpt	db	"Attendi, giocatore 1 sta pensando...",0
	player2cpuprmpt	db	"Attendi, giocatore 2 sta pensando...",0
	itisadraw	db	"Pareggio",0
	player1wins	db	"Vince giocatore 1",0
	player2wins	db	"Vince Giocatore 2",0
%else
	player1hmnprmpt	db	"Make your move, player 1.",0
	player2hmnprmpt db	"Make your move, player 2.",0
	player1cpuprmpt	db	"Player 1 is thinking, please wait...",0
	player2cpuprmpt	db	"Player 2 is thinking, please wait...",0
	itisadraw	db	"It's a draw.",0
	player1wins	db	"Player 1 wins.",0
	player2wins	db	"Player 2 wins.",0
%endif


; pointer to ai player. future releases C4 might
; or might not support different ai players =)
aicode		dd	aiGetMove


; button images
redpcx:		incbin	"red.pcx"
REDPCXSIZE	equ	($ - redpcx)
bluepcx:	incbin	"blue.pcx"
BLUEPCXSIZE	equ	($ - bluepcx)



;**********************************************************
; uninitialized data
;**********************************************************

	section .bss

; player input
; 0 	:	no input available
; 1..7	:	column to drop stone into
playerinput	resd	1

mouseinput	resd	1
gameover	resd	1

redstone	resb	STONESIZE*STONESIZE*3
bluestone	resb	STONESIZE*STONESIZE*3

end:
