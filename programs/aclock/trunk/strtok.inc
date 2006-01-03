; some strtok-like function
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
%ifndef _STRTOK_INC
%define _STRTOK_INC


;********************************************************************
;	strtok
;	this function works like strtok from a c runtime library.
;	note that it is not threadsafe. it would be an easy task
;	to make it threadsafe, though:
;	.adx must be removed, instead the last search address is
;	stored at some location provided by the user (passed as
;	a third parameter in ecx or so)
;
;	input:
;
;	eax	:	address of string to be searched (asciiz), or
;			0 to get the next token of the current string
;	ebx	:	address of delimiter list (asciiz)
;
;	output:
;
;	eax	:	pointer to the next token, or 0 if there
;			aren't any tokens anymore.
;
;	destroys:	nothing
;
;********************************************************************
strtok:
	pushad
	pushfd

	; get start address
	; if the new start address is 0, and the old address (.adx)
	; is also 0, then there's nothing to do and we return 0.
	or	eax,eax			; new address =  0 ?
	jz	.nonewstring		; nope -> use old string
	mov	[.adx],eax		; yeah -> store new string adx
.nonewstring:
	mov	esi,[.adx]		; load string address
	or	esi,esi			; 0 ?
	jnz	.startadxok		; nope -> ok
	xor	eax,eax			; yeah -> return 0
	je	.bye
.startadxok:

	; skip leading delimiters
.skipdelimiters:
	lodsb				; read character
	mov	edi,ebx			; edi -> delimiter list
.foo:
	mov	cl,[edi]		; get delimiter
	inc	edi
	or	cl,cl			; end of delimiter list
	jz	.endofdelimiterlist
	cmp	al,cl			; if AL is a delimiter, then
	je	.skipdelimiters		; we need to skip it too...
	jmp	.foo			; otherwise try next delimiter
.endofdelimiterlist:

	; end of string reached without finding any non-delimiters ?
	or	al,al			; character = 0 ?
	jnz	.bar			; nope -> continue
	mov	dword [.adx],0		; yeah -> remember this
	xor	eax,eax			; and return 0
	jmp	.bye
.bar:

	; found the start of a token, let's store its address
	mov	edx,esi
	dec	edx			; edx = start address of token

	; find the end of the token
.abraham:
	lodsb				; get character
	mov	edi,ebx			; edi -> delimiter list
.bebraham:
	mov	cl,[edi]		; get delimiter
	inc	edi
	cmp	al,cl			; is AL a delimiter ?
	jne	.cebraham		; nope -> continue
	or	al,al			; terminating zero found ?
	jnz	.argle
	xor	esi,esi			; yeah -> remember this
	jmp	.bargle
.argle:
	mov	byte [esi-1],0		; nope -> mark end of token
.bargle:
	mov	[.adx],esi		; remember search address
	mov	eax,edx			; return token address
	jmp	.bye
.cebraham:
	or	cl,cl			; end of delimiter list ?
	jnz	.bebraham		; nope -> try next delimiter
	jmp	.abraham
	
	; write return value into stack, so that when popad
	; gets executed, eax will receive the return value.
.bye:
	mov	[esp+4*8],eax
	popfd
	popad
	ret
.adx	dd	0

%endif

