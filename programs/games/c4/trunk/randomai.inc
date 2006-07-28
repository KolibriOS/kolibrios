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

%infdef _RANDOMAI_INC
%define _RANDOMAI_INC

	section .text



;**********************************************************
; randomaiGetMove
; getmove routine for random "ai" player
;
; input		:	eax = cpu level (ignored by this player:)
; output	:	eax = move
; destroys	:	everything
;**********************************************************
randomaiGetMove

.l:
	call rand		; get random number in the range [1,7]
	xor edx,edx		; !
	mov ebx,7
	div ebx
	inc edx
	BOARDISVALIDMOVE edx	; is this a valid move ?
	jz .l			; no -> try again

	mov eax,edx		; return move	
	ret

%endif	