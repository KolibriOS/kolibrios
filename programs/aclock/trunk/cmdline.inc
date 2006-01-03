; command line parsing code for aclock
;
; Copyright (c) 2003 Thomas Mathys
; killer@vantage.ch
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
%ifndef _CMDLINE_INC
%define _CMDLINE_INC


;********************************************************************
;	parse the command line
;	input		:	nothing
;	output		:	wndXPos, wndYPos, wndWidth, wndHeight
;				are changed.
;	destroys	:	nothing
;********************************************************************
parseCommandLine:
	pushad
	pushfd

	; terminate command line, just to be sure
	mov	byte [cmdLine + 256],0

	; go through all tokens
	mov	eax,cmdLine		; eax -> command line
.parseloop:
	mov	ebx,delimiters		; ebx -> token delimiter list
	call	strtok			; get next parameter
	or	eax,eax			; no more parameters ?
	jz	.nomoretokens
	mov	cl,[eax]		; get 1st char of parameter
	cmp	cl,'x'			; which parameter is it ?
	je	.param_x
	cmp	cl,'y'
	je	.param_y
	cmp	cl,'w'
	je	.param_w
	cmp	cl,'h'
	je	.param_h
	; if we reach this line it's an unknown parameter, ignore it
.nextparam:
	xor	eax,eax			; set eax = 0 to continue
	jmp	.parseloop		; after last token.
.nomoretokens:
	DBG_BOARD_PRINTDWORD [wndXPos]
	DBG_BOARD_PRINTCHAR 32
	DBG_BOARD_PRINTDWORD [wndYPos]
	DBG_BOARD_PRINTCHAR 32
	DBG_BOARD_PRINTDWORD [wndWidth]
	DBG_BOARD_PRINTCHAR 32
	DBG_BOARD_PRINTDWORD [wndHeight]
	DBG_BOARD_PRINTNEWLINE
	popfd
	popad
	ret

	; eax -> first character of the parameter
.param_x:
	push	eax
	call	parsePositionParam
	mov	[wndXPos],eax
	pop	eax
	jmp	.nextparam

	; eax -> first character of the parameter
.param_y:
	push	eax
	call	parsePositionParam
	mov	[wndYPos],eax
	pop	eax
	jmp	.nextparam

	; eax -> first character of the parameter
.param_w:
	push	eax
	call	parseSizeParam
	mov	[wndWidth],eax
	pop	eax
	jmp	.nextparam

	; eax -> first character of the parameter
.param_h:
	push	eax
	call	parseSizeParam
	mov	[wndHeight],eax
	pop	eax
	jmp	.nextparam

; parse position parameter
; input		:	eax = address of first character of parameter
; output	:	eax contains position
; destroys	:	nothing
parsePositionParam:
	push	ebx
	push	esi
	pushfd

	; is the second char of the parameter a '-' ?
	inc	eax
	xor	ebx,ebx			; assume it isn't
	cmp	byte [eax],'-'
	jne	.nominus
	mov	ebx,1			; yes -> set flag...
	inc	eax			; ...and move to next char
.nominus:

	; convert rest of parameter to doubleword
	mov	esi,eax
	call	string2dword

	; negate if necessary
	or	ebx,ebx
	jz	.rotationshyperboloid
	neg	eax
.rotationshyperboloid:

	popfd
	pop	esi
	pop	ebx
	ret

; parse dimension parameter
; input		:	eax = address of first char of parameter
; output	:	eax contains dimension
; destroys	:	nothing
parseSizeParam:
	push	esi
	pushfd
	lea	esi,[eax + 1]		; esi -> 2nd char of parameter
	call	string2dword
	popfd
	pop	esi
	ret


%endif

