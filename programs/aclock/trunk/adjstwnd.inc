; adjustWindowDimensions
; adjust menut window dimensions to get a certain work area size.
; or so. who on earth cares anyway, i certinaly don't, i'm just
; writing this code because i've got to kill time somehow... 
;
; Copyright (c) 2002 Thomas Mathys
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
%ifndef _ADJSTWND_INC
%define _ADJSTWND_INC


	;window types
	ADJSTWND_TYPE_SKINNED	equ	0


;********************************************************************
;	adjust window dimensions to get a certain work area size
;
;	-	first the window width and height are adjusted
;		and clamped if they're too large for the screen.
;	-	then the window positions are adjusted, if the
;		window goes out of the screen.
;
;	input:
;	eax	window type, one of the ADJSTWND_TYPE_xxx constants
;	ebx	window x position
;	ecx	window y position
;	edx	desired work area width
;	esi	desired work area height
;
;	output:
;	eax	return code. 0 = ok, -1 = invalid window type
;	ebx	adjusted window x position
;	ecx	adjusted window y position
;	edx	window width to get desired work area width
;	esi	window height to get desired work area height
;
;	destroys:
;	nothing
;
;	normally x and y are the upper left corner of the window,
;	relative to the upper left corner of the screen.
;	if you pass a negative x or y it will be treated as the
;	lower right corner of the window, relative to the lower
;	right corner of the screen.
;********************************************************************
adjustWindowDimensions:
	push	edi
	push	ebp
	pushfd

	; adjust window dimensions, depending on the window type
	cmp	eax,ADJSTWND_TYPE_SKINNED
	je	.adjust_skinned
	mov	eax,-1			; invalid window type,
	jmp	.bye			; return error code

	; clamp window dimensions
.clamp:
	mov	eax,MOS_SC_GETSCREENMAX ; get screen dimensions
	int	0x40
	mov	edi,eax			; edi = screen width
	shr	edi,16
	mov	ebp,eax			; ebp = screen height
	and	ebp,0xffff
	cmp	edx,edi			; window width > screen width ?
	jna	.widthok
	mov	edx,edi			; yes -> use screen width
.widthok:
	cmp	esi,ebp			; wnd height > screen height ?
	jna	.heightok
	mov	esi,ebp			; yes -> use screen height
.heightok:

	; adjust x position
	or	ebx,ebx			; do the lower right corner
	jns	.foo			; stuff if x is negative.
	add	ebx,edi
	sub	ebx,edx
.foo:
	or	ebx,ebx			; x < 0 ?
	jns	.xnotnegative
	xor	ebx,ebx			; yes -> x = 0
.xnotnegative:
	mov	eax,ebx			; x + width > screen width ?
	add	eax,edx
	cmp	eax,edi
	jna	.xok
	sub	eax,edi			; yes -> adjust
	sub	ebx,eax
.xok:

	; adjust y position
	or	ecx,ecx			; do the lower right corner
	jns	.bar			; stuff if y is negative.
	add	ecx,ebp
	sub	ecx,esi
.bar:
	or	ecx,ecx			; y < 0 ?
	jns	.ynotnegative
	xor	ecx,ecx			; yes -> y = 0
.ynotnegative:
	mov	eax,ecx			; y + height > screen height ?
	add	eax,esi
	cmp	eax,ebp
	jna	.yok
	sub	eax,ebp			; yes -> adjust
	sub	ecx,eax
.yok:

.done:
	xor	eax,eax
.bye:
	popfd
	pop	ebp
	pop	edi
	ret

.adjust_skinned:
	; adjust width (edx)
	add	edx,MOS_WND_SKIN_BORDER_LEFT+MOS_WND_SKIN_BORDER_RIGHT
	; adjust height (esi). we need the skin height to do this.
	push	ebx
	mov	eax,MOS_SC_WINDOWPROPERTIES
	mov	ebx,4
	int	0x40
	lea	esi,[esi+eax+MOS_WND_SKIN_BORDER_BOTTOM]
	pop	ebx
	jmp	.clamp

%endif

