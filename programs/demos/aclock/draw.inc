; drawing code for aclock
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


TMR1_FACTOR	dd	0.45
TMR2_FACTOR	dd	0.426315789
SECR_FACTOR	dd	0.378947368
MINR_FACTOR	dd	0.355263158
HOURR_FACTOR	dd	0.189473684
DATE_FACTOR	dd	0.1


monthNames:
		db "Jan"
		db "Feb"
		db "Mar"
		db "Apr"
		db "May"
		db "Jun"
		db "Jul"
		db "Aug"
		db "Sep"
		db "Oct"
		db "Nov"
		db "Dec"


;********************************************************************
;	draws the clock
;	input		:	nothing
;	output		:	nothing
;	destroys	:	nothing
;********************************************************************
proc drawClock
	locals
		i dd ?
		TMR1X dd ?
		TMR1Y dd ?
		TMR2X dd ?
		TMR2Y dd ?
		SECRX dd ?
		SECRY dd ?
		MINRX dd ?
		MINRY dd ?
		HOURRX dd ?
		HOURRY dd ?
		workwidth dd ?
		workheight dd ?
		foo dd ?
	endl
	pushad
	pushfd

	; get window dimensions
	mcall SF_THREAD_INFO,procInfo,-1

	; calculate work area size (width/height = ecx/edx)
	; if the work area is too small (maybe the window is shaded)
	; we don't draw anything.
	mcall SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT ; get skin height (eax)
	mov	ecx,[procInfo.box.width]
	sub	ecx,MOS_WND_SKIN_BORDER_LEFT+MOS_WND_SKIN_BORDER_RIGHT
	mov	edx,[procInfo.box.height]
	sub	edx,eax
	sub	edx,MOS_WND_SKIN_BORDER_BOTTOM
	cmp	ecx,0			; width too small ?
	jle	.bye
	cmp	edx,0			; height too small ?
	jnle	.continue
.bye:
	jmp	.byebye
.continue:
	mov	[workwidth],ecx		; save for later (for fpu)
	mov	[workheight],edx

	; calculate center of clock (x/y = esi/edi)
	mov	esi,[procInfo.box.width]
	shr	esi,1
	mov	edi,[procInfo.box.height]
	sub	edi,MOS_WND_SKIN_BORDER_BOTTOM
	sub	edi,eax
	shr	edi,1
	add	edi,eax

	; clear work area
	pushad
	mov	ebx,(MOS_WND_SKIN_BORDER_LEFT)*0x10000	; x start
	or	ebx,ecx					; width
	inc	ebx
	mov	ecx,eax					; y start
	shl	ecx,16					; (=skin height)
	or	ecx,edx					; height
	inc	ecx
	mov	edx,[wndColors.work]
	mcall SF_DRAW_RECT
	popad

	; calculate second hand radii
	fild	dword [workwidth]
	fmul	dword [SECR_FACTOR]
	fstp	dword [SECRX]
	fild	dword [workheight]
	fmul	dword [SECR_FACTOR]
	fstp	dword [SECRY]

	; calculate minute hand radii
	fild	dword [workwidth]
	fmul	dword [MINR_FACTOR]
	fstp	dword [MINRX]
	fild	dword [workheight]
	fmul	dword [MINR_FACTOR]
	fstp	dword [MINRY]

	; calculate hour hand radii
	fild	dword [workwidth]
	fmul	dword [HOURR_FACTOR]
	fstp	dword [HOURRX]
	fild	dword [workheight]
	fmul	dword [HOURR_FACTOR]
	fstp	dword [HOURRY]

	; calculate tick mark radii
	fild	dword [workwidth]
	fmul	dword [TMR1_FACTOR]
	fstp	dword [TMR1X]
	fild	dword [workheight]
	fmul	dword [TMR1_FACTOR]
	fstp	dword [TMR1Y]
	fild	dword [workwidth]
	fmul	dword [TMR2_FACTOR]
	fstp	dword [TMR2X]
	fild	dword [workheight]
	fmul	dword [TMR2_FACTOR]
	fstp	dword [TMR2Y]

	; get system clock (edx)
	mcall SF_GET_SYS_TIME
	mov	edx,eax

	; draw second hand
	push	edx
	mov	eax,edx
	shr	eax,16
	call	bcdbin
	mov	ecx,eax			; save seconds for later
	; 2*pi/60
	stdcall	getHandCoords,edi,esi,[SECRY],[SECRX],0.104719755,eax,ecx
	shl	ebx,16
	or	ebx,esi
	shl	ecx,16
	or	ecx,edi
	mov	edx,[wndColors.work_text]
	mcall SF_DRAW_LINE
	pop	ecx
	pop	edx

	; draw minute hand
	push	edx
	mov	eax,edx
	shr	eax,8
	call	bcdbin
	mov	edx,60
	mul	edx
	add	eax,ecx
	mov	ecx,eax			; save for later
	; 2*pi/60/60
	stdcall	getHandCoords,edi,esi,[MINRY],[MINRX],0.001745329,eax,ecx
	shl	ebx,16
	or	ebx,esi
	shl	ecx,16
	or	ecx,edi
	mov	edx,[wndColors.work_text]
	mcall SF_DRAW_LINE
	pop	ecx
	pop	edx

	; draw hour hand
	push	edx
	mov	eax,edx
	call	bcdbin
	cmp	eax,11			; % 12 (just to be sure)
	jnae	.hoursok
	sub	eax,12
.hoursok:
	mov	edx,60*60
	mul	edx
	add	eax,ecx
	; 2*pi/60/60/12
	stdcall	getHandCoords,edi,esi,[HOURRY],[HOURRX],0.000145444,eax
	shl	ebx,16
	or	ebx,esi
	shl	ecx,16
	or	ecx,edi
	mov	edx,[wndColors.work_text]
	mcall SF_DRAW_LINE
	pop	edx
	
	; draw tick marks
	mov	dword [i],11		; draw 12 marks
.drawtickmarks:
	; calculate start point
	; 2*pi/12
	stdcall	getHandCoords,edi,esi,[TMR1Y],[TMR1X],0.523598776,[i]
	mov	eax,ebx			; save in eax and edx
	mov	edx,ecx
	; 2*pi/12
	stdcall	getHandCoords,edi,esi,[TMR2Y],[TMR2X],0.523598776,[i]
	shl	eax,16
	shl	edx,16
	or	ebx,eax			; ebx = x start and end
	or	ecx,edx			; ecx = y start and end
	mov	edx,[wndColors.work_text]
	mcall SF_DRAW_LINE
	dec	dword [i]
	jns	.drawtickmarks

	DATE_WIDTH	=48

	; calculate text start position
	mov	eax,[procInfo.box.width]
	sub	eax,DATE_WIDTH		; x = (wndwidth-textwidth)/2
	shr	eax,1			; eax = x
	fild	dword [workheight]	; y = DATE_FACTOR*workheight...
	fmul	dword [DATE_FACTOR]
	mov	[foo],edi		; ... + y_clockcenter
	fiadd	dword [foo]
	fistp	dword [foo]
	mov	ebx,[foo]		; ebx = y

	; draw text at all ?
	cmp	dword [workwidth],DATE_WIDTH	; text too wide ?
	jb	.goodbye
	mov	ecx,ebx				; text too high ?
	add	ecx,10-1
	mov	edx,[procInfo.box.height]
	sub	edx,MOS_WND_SKIN_BORDER_BOTTOM
	cmp	ecx,edx
	jnae	.yousuck
.goodbye:
	jmp	.bye
.yousuck:


	; ebx = (x << 16) | y
	shl	eax,16
	or	ebx,eax
	
	; get date (edi)
	mcall SF_GET_SYS_DATE
	mov	edi,eax

	; display month
	mov	eax,edi			; get month
	shr	eax,8
	call	bcdbin
					; ebx contains already position
	mov	ecx,[wndColors.work_text]
	lea	edx,[monthNames-3+eax*2+eax]; -3 because eax = 1..12 =]
	mov	esi,3			; text length
	mcall SF_DRAW_TEXT

	; display date
	add	ebx,(3*6+3) shl 16
	mov	eax,edi			; get date
	shr	eax,16
	call	bcdbin
	mov	edx,ebx			; position must be in edx
	mov	ebx,0x00020000		; number, display two digits
	mov	ecx,eax			; number to display
	mov	esi,[wndColors.work_text]
	mcall SF_DRAW_NUMBER

	; display year. the way we avoid the y2k bug is even
	; simpler, yet much better than in the last version:
	; now we simply display the last two digits and let the
	; user decide wether it's the year 1903 or 2003 =]
	add	edx,(2*6+3) shl 16
	mov	eax,edi			; get year
	call	bcdbin
	mov	ebx,0x00020000		; number, display two digits
	mov	ecx,eax			; number to display
					; edx contains already position
	mov	esi,[wndColors.work_text]
	mcall SF_DRAW_NUMBER

.byebye:
	popfd
	popad
	;leave
	ret
endp


;**********************************************************
; bcdbin
; converts a 8 bit bcd number into a 32 bit binary number
;
; in		al = 8 bit bcd number
; out		eax = 32 bit binary number
; destroys	dl,flags
;**********************************************************
bcdbin:
	push	edx
	pushfd
	mov	dl,al			; save bcd number
	shr	al,4			; convert upper nibble
	mov	ah,10
	mul	ah
	and	dl,15			; add lower nibble
	add	al,dl
	and	eax,255			; !
	popfd
	pop	edx
	ret


;********************************************************************
; getHandCoords
; calculates the end point of a hand
;
; input (on stack, push from top to bottom):
; ANGLE		angle (integer)
; DEG2RAD	conversion factor for ANGLE (32 bit real)
; RADIUSX	x radius (32 bit real)
; RADIUSY	y radius (32 bit real)
; CENTERX	x center of the clock (integer)
; CENTERY	y center of the clock (integer)
;
; output:
; ebx		x coordinate in bits 0..15, bits 16..31 are zero
; ecx		y coordinate in bits 0..15, bits 16..31 are zero
;
; destroys:
; nothing
;********************************************************************
proc getHandCoords CENTERY:dword, CENTERX:dword, RADIUSY:dword, RADIUSX:dword, DEG2RAD:dword, ANGLE:dword
	pushfd

	fild dword [ANGLE]		; get angle
	fmul dword [DEG2RAD]	; convert to radians
	fsincos
	fmul dword [RADIUSY]	; -y * radius + clockcy
	fchs
	fiadd dword [CENTERY]
	fistp dword [CENTERY]	
	fmul dword [RADIUSX]	; x * radius + clockcx
	fiadd dword [CENTERX]
	fistp dword [CENTERX]
	
	mov ebx,[CENTERX]
	mov ecx,[CENTERY]

	popfd
	;leave
	ret
endp


