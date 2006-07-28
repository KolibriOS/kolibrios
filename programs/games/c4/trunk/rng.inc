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

%ifndef _RNG_INC
%define _RNG_INC

	section .data


; random seed
seed		dd	0



	section .text


;**********************************************************
; randomize
; initialize random number generator.
; initialization is done using the get system clock syscall
; 
; input		:	nothing
; output	:	nothing
; destroys	:	nothing
;**********************************************************
randomize:
	push eax
	mov eax,MOS_SC_GETSYSCLOCK
	int 0x40
	mov [seed],eax
	pop eax
	ret



;**********************************************************
; rand
; return an unsigned 32 bit "random" number
;
; input		:	nothing
; output	:	eax = unsigned 32 bit random number
; destroys	:	nothing
;**********************************************************
rand:
	pushfd
	push edx
	mov eax,32719
	mul dword [seed]
	add eax,123
	xchg al,ah
	rol eax,16
	xchg al,ah
	mov [seed],eax
	pop edx
	popfd
	ret
	
%endif	