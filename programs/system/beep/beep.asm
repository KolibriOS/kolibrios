; Beep - Speaker beep you if the video does not work
;        and you think that the system died.
;
; Copyright (c) 20012, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	   notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	   notice, this list of conditions and the following disclaimer in the
;	   documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	   names of its contributors may be used to endorse or promote products
;	   derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Marat Zakiyanov ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************
; 
; In a capitalist world - you ping to satellite.
; In Soviet Russia - sputnik beep to you!
;                               Russian folk art
;---------------------------------------------------------------------
	use32
	org 0x0

	db 'MENUET01'
	dd 0x01
	dd START
	dd IM_END
	dd I_END
	dd stacktop
	dd 0x0
	dd 0x0
;-------------------------------------------------------------------------------
include '../../macros.inc'
;-------------------------------------------------------------------------------
START:
	mcall	68,1
	mcall	5,1
	mcall	26,9
	cmp	[timer],eax
	ja	START
	add	eax,150
	mov	[timer],eax
	mcall	55, eax, , , Music
	jmp	START
;-------------------------------------------------------------------------------
Music:
	db 0x90, 0x37, 0
IM_END:
;-------------------------------------------------------------------------------
align 4
timer:
	rd 1
;-------------------------------------------------------------------------------
	rb 512
stacktop:
;-------------------------------------------------------------------------------
I_END:
;-------------------------------------------------------------------------------