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

%ifndef _AI_INC
%define _AI_INC


INFTY		equ	1000000000



	section .data

; table used to perform some primitive move "ordering":
; middle columns which are usually more important are
; searched first.
moveorder	dd	1,7,2,6,3,5,4

; table used for static evaluation.
; this table is taken from 4st attack: it is ways better
; than the table i used before =)
evaltable:	dd	 0, 0, 0, 0, 0, 0, 0, 0, 0
		dd	 0, 3, 4, 5, 7, 5, 4, 3, 0
		dd	 0, 4, 6, 8,10, 8, 6, 4, 0
		dd	 0, 5, 8,12,13,12, 8, 5, 0
		dd	 0, 5, 8,12,13,12, 8, 5, 0
		dd	 0, 4, 6, 8,10, 8, 6, 4, 0
		dd	 0, 3, 4, 5, 7, 5, 4, 3, 0
		dd	 0, 0, 0, 0, 0, 0, 0, 0, 0



	section .bss

cpulevel	resd	1	; level of current cpu player
bestval		resd	1	; value of best move found so far
nbestmoves	resd	1	; # of best moves found so far
bestmoves	resd	7	; array to hold all best moves



	section .text



;**********************************************************
; aiGetMove
; returns a move for a computer player
;
; input		:	eax = cpu level ( >= 1)
; output	:	eax = move
; destroys	:	everything
;**********************************************************
aiGetMove:

	; initialize variables
	mov [cpulevel],eax
	mov dword [bestval],-INFTY
	mov dword [nbestmoves],0
	
	; try every move
	mov ecx,6
.evalmoves:

	; get move to make from move order table
	mov eax,[moveorder+ecx*4]
	
	; if this is an invalid move, continue with next move
	BOARDISVALIDMOVE eax
	jz .nextmove

	; make move for current player
	mov ebx,[currentplayer]
	call boardMakeMove
	
	; evaluate move
	push eax			; save move #
	push ecx			; save loop counter
	push dword [cpulevel]		; ply
	mov ebx,[currentplayer]		; player
	BOARDGETOTHERPLAYER ebx
	push ebx
	push dword -INFTY		; alpha
	push dword INFTY		; beta
	call alphabeta
	neg eax				; damn, how could i forget this ???
	mov ebx,eax			; save result for later
	pop ecx				; restore loop counter
	pop eax				; restore move #
	
	; undo move (eax = move #)
	call boardUndoMove
	
	; let's see wether we have a new best move
	cmp ebx,[bestval]		; new best value found ?
	jle .nonewbestval
	mov [bestval],ebx		; yes -> save it
	mov dword [nbestmoves],1	; delete everything that was in the list
	mov [bestmoves+0],eax		; save move number in list
	jmp short .nextmove		; continue with next move
.nonewbestval:
	cmp ebx,[bestval]		; another best value found ?
	jne .nextmove
	mov ebx,[nbestmoves]		; yes -> add move to list
	mov [bestmoves+ebx*4],eax
	inc dword [nbestmoves]

.nextmove:
	dec ecx
	js .done
	jmp .evalmoves
.done:
	
	; randomly pick one of the best moves
	call rand			; rand() % nbestmoves
	xor edx,edx
	div dword [nbestmoves]
	mov eax,[bestmoves+edx*4]	; get move from list
	ret

	; test code : pick first move from list
	mov eax,[bestmoves+0]
	ret



;**********************************************************
; alphabeta
;
; input		:	see below
; output	:	eax = move value
; destroys	:	everything
;**********************************************************
alphabeta:

%define ply	(ebp+20)
%define player	(ebp+16)
%define alpha	(ebp+12)
%define beta	(ebp+ 8)

	enter 0,0
	
	; win for other player -> end search
	mov eax,[player]
	BOARDGETOTHERPLAYER eax
	call boardIsWin
	or eax,eax
	jz .nowin
	mov eax,-1000000
	mov ebx,[ply]
	shl ebx,10
	sub eax,ebx
	leave
	ret 4*4
.nowin:
	; board full but no win -> draw -> end search
	BOARDISFULL
	jnz .notfull
	xor eax,eax
	leave
	ret 4*4
.notfull:
	; max search depth reached -> do static evaluation
	cmp dword [ply],0
	je .staticeval
	

	; for each move
	mov ecx,6
.evalmoves:

	; while (alpha < beta)
	mov eax,[alpha]
	cmp eax,[beta]
	jge .done

	; pick move from move order table
	mov eax,[moveorder+ecx*4]
	
	; invalid move ? if so, continue with next move
	BOARDISVALIDMOVE eax
	jz .nextmove
	
	; make move for current player
	mov ebx,[player]
	call boardMakeMove
	
	; evaluate move
	push eax
	push ecx
	mov eax,[ply]			; ply = ply-1
	dec eax
	push eax
	mov ebx,[player]		; player = other player
	BOARDGETOTHERPLAYER ebx
	push ebx
	mov ecx,[beta]			; alpha = -beta
	neg ecx
	push ecx
	mov edx,[alpha]			; beta = -alpha
	neg edx
	push edx
	call alphabeta
	neg eax
	
	; new alpha ?
	cmp eax,[alpha]
	jle .nonewalpha
	mov [alpha],eax			; yes -> save it
.nonewalpha:	
	pop ecx
	pop eax
	
	; undo move
	call boardUndoMove

.nextmove:				; evaluate next move
	dec ecx
	jns .evalmoves
	
.done:
	mov eax,[alpha]
	leave
	ret 4*4
		
	; static evaluation
.staticeval:
	xor eax,eax
	mov esi,BWIDTH*BHEIGHT-1
.l:
	mov ebx,[board+esi*4]		; get stone from board
	cmp ebx,[player]		; player's stone ?
	jne .notplayer			; nope -> go on
	add eax,[evaltable+esi*4]	; yes -> add stone value to eax
	jmp .next			; next stone
.notplayer:
	cmp ebx,EMPTY			; other player's stone ?
	je .empty			; nope -> go on
	sub eax,[evaltable+esi*4]	; yes -> sub stone value from eax
.empty:
	
.next:					; next stone
	dec esi
	jns .l
	leave				; eax contains static value
	ret 4*4

%undef ply
%undef player
%undef alpha
%undef beta

%endif