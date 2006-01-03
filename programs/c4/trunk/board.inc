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

%ifndef _BOARD_INC
%define _BOARD_INC


;**********************************************************
; magic numbers
;**********************************************************

EMPTY		equ	0
PLAYER1		equ	1
PLAYER2		equ	2

; board dimensions (+2 for border)
BWIDTH		equ	9
BHEIGHT		equ	8



;**********************************************************
; uninitialized data
;**********************************************************

	section .bss
	
board		resd	BHEIGHT*BWIDTH		; the board
free		resd	BWIDTH			; # of free fields for each column
totalfree	resd	1			; total # of free fields
currentplayer	resd	1			; player to make next move
lastmove	resd	1			; last move done on the board
						; (0 if no last move available)



;**********************************************************
; code
;**********************************************************
	
	section .text
	

;**********************************************************
; boardReset
; reset board for new game
; 
; input		:	nothing
; output	:	nothing
; destroys	:	nothing
;**********************************************************
boardReset:
	pushfd
	pushad
	
	; clear all fields
	mov edi,board
	mov ecx,BHEIGHT * BWIDTH
	mov eax,EMPTY
	rep stosd
		
	; init free array
	mov edi,free
	mov ecx,BWIDTH
	mov eax,BHEIGHT - 2
	rep stosd
	
	mov dword [totalfree],(BWIDTH-2)*(BHEIGHT-2)
	mov dword [currentplayer],PLAYER1
	mov dword [lastmove],0
	
	popad
	popfd
	ret



;**********************************************************
; BOARDISVALIDMOVE
; checks wether a move is valid
;
; input		:	32 bit register containing a move
;			number (1..7)
; output	:	zero flag set -> move is invalid
;			zero flag clear -> move is valid
; destroys	:	nothing
;**********************************************************
%macro BOARDISVALIDMOVE 1
	cmp dword [free+%1*4],0
%endmacro



;**********************************************************
; boardMakeMove
; make a move
;
; input		:	eax = move number (1..7)
;			ebx = PLAYER1/PLAYER2
; output	:	nothing
; destroys	:	edi, flags
;**********************************************************

boardMakeMove:
	mov edi,[free+eax*4]		; place stone
	imul edi,BWIDTH
	add edi,eax
	mov [board+edi*4],ebx
	
	dec dword [free+eax*4]		; one stone more in this column
	dec dword [totalfree]		; one stone more on the board
	mov [lastmove],eax		; store move for undo
	ret



;**********************************************************
; boardUndoMove
; undo a move
;
; input		:	eax = move number to undo (1..7)
; output	:	move is undone
; destroys	:	edi, flags
;**********************************************************
boardUndoMove:
	inc dword [free+eax*4]		; one stone less in this column
	inc dword [totalfree]		; one stone less on the board
	mov dword [lastmove],0		; no last move available
	
	mov edi,[free+eax*4]		; remove stone from board
	imul edi,BWIDTH
	add edi,eax
	mov dword [board+edi*4],EMPTY
	ret



;**********************************************************
; BOARDSWITCHPLAYERS
; switch current player
;
; input		:	nothing
; output	:	current player is switched
; destroys	:	flags
;**********************************************************
%macro BOARDSWITCHPLAYERS 0
	xor dword [currentplayer],(PLAYER1 ^ PLAYER2)
%endmacro



;**********************************************************
; BOARDGETOTHERPLAYER
; get other player =)
;
; input		:	32 bit register or variable
; output	:	player changed
; destroys	:	flags
;**********************************************************
%macro BOARDGETOTHERPLAYER 1
	xor %1,(PLAYER1 ^ PLAYER2)
%endmacro



;**********************************************************
; BOARDISFULL
; check wether board is full
;
; input		:	nothing
; output	:	zero flag set -> board is full
;			zero flag clear -> board isn't full
;**********************************************************
%macro BOARDISFULL 0
	cmp dword [totalfree],0
%endmacro



;**********************************************************
; boardIsWin
; check wether the current board state is a win for the
; given player
;
; input		:	eax = player
; output	:	eax = nonzero -> win for player
; destroys	:	everything
;**********************************************************

boardIsWin:

	; check wether last move is available
	cmp dword [lastmove],0
	jne .lastmoveok
	xor eax,eax
	ret
.lastmoveok:

	; calculate the address of the most recently placed stone (edi)
	mov edi,[lastmove]
	mov esi,[free + edi*4]
	inc esi
	imul esi,BWIDTH
	add edi,esi
	shl edi,2
	add edi,board
	
	; check vertically
	xor ecx,ecx			; reset counter
	mov esi,edi			; start at last stone
.vertical:
	cmp [esi],eax			; loop as long the stone [esi] belongs to player
	jne .verticalend
	add esi,BWIDTH*4		; move to next stone
	inc ecx				; one stone more in row
	jmp short .vertical
.verticalend:
	cmp ecx,4			; 4 or more stones ?
	jl .nope1			; nope -> continue
	mov eax,1			; yeah -> tell caller =)
	ret
.nope1:

	; check horizontally
	xor ecx,ecx
	mov esi,edi
.horizontal1:				; left
	cmp [esi],eax
	jne .horizontalend1
	sub esi,4
	inc ecx
	jmp short .horizontal1
.horizontalend1:
	lea esi,[edi+4]			; right
.horizontal2:
	cmp [esi],eax
	jne .horizontalend2
	add esi,4
	inc ecx
	jmp short .horizontal2
.horizontalend2:
	cmp ecx,4
	jl .nope2
	mov eax,1
	ret
.nope2:

	; diagonally forward
	xor ecx,ecx
	mov esi,edi
.forward1:
	cmp [esi],eax
	jne .forwardend1
	sub esi,(1+BWIDTH)*4
	inc ecx
	jmp short .forward1
.forwardend1:
	lea esi,[edi+(1+BWIDTH)*4]
.forward2:
	cmp [esi],eax
	jne .forwardend2
	add esi,(1+BWIDTH)*4
	inc ecx
	jmp short .forward2
.forwardend2:	
	cmp ecx,4
	jl .nope3
	mov eax,1
	ret
.nope3:

	; diagonally backward
	xor ecx,ecx
	mov esi,edi
.backward1:
	cmp [esi],eax
	jne .backwardend1
	add esi,(1-BWIDTH)*4
	inc ecx
	jmp short .backward1
.backwardend1:
	lea esi,[edi+(BWIDTH-1)*4]
.backward2:
	cmp [esi],eax
	jne .backwardend2
	add esi,(BWIDTH-1)*4
	inc ecx
	jmp short .backward2
.backwardend2:
	cmp ecx,4
	jl .nope4
	mov eax,1
	ret
.nope4:

	; no win for this player
	xor eax,eax
	ret


%endif