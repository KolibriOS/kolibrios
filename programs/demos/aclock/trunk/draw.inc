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
%ifndef _DRAW_INC
%define _DRAW_INC


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
drawClock:
	%push	drawClock_context
	%stacksize flat
	%assign	%$localsize 0

	%local	i:dword,		\
		TMR1X:dword,		\
		TMR1Y:dword,		\
		TMR2X:dword,		\
		TMR2Y:dword,		\
		SECRX:dword,		\
		SECRY:dword,		\
		MINRX:dword,		\
		MINRY:dword,		\
		HOURRX:dword,		\
		HOURRY:dword,		\
		workwidth:dword,	\
		workheight:dword,	\
		foo:dword
	
	enter	%$localsize,0
	pushad
	pushfd

	; get window dimensions
	mov	eax,MOS_SC_GETPROCESSINFO
	mov	ebx,procInfo
	mov	ecx,-1
	int	0x40

	; calculate work area size (width/height = ecx/edx)
	; if the work area is too small (maybe the window is shaded)
	; we don't draw anything.
	mov	eax,MOS_SC_WINDOWPROPERTIES
	mov	ebx,4			; get skin height (eax)
	int	0x40
	mov	ecx,[procInfo + MOS_PROCESSINFO.wndWidth]
	sub	ecx,MOS_WND_SKIN_BORDER_LEFT+MOS_WND_SKIN_BORDER_RIGHT
	mov	edx,[procInfo + MOS_PROCESSINFO.wndHeight]
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
	mov	esi,[procInfo + MOS_PROCESSINFO.wndWidth]
	shr	esi,1
	mov	edi,[procInfo + MOS_PROCESSINFO.wndHeight]
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
	mov	edx,[wndColors + MOS_WNDCOLORS.work]
	mov	eax,MOS_SC_DRAWBAR
	int	0x40
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
	mov	eax,MOS_SC_GETSYSCLOCK
	int	0x40
	mov	edx,eax

	; draw second hand
	push	edx
	mov	eax,edx
	shr	eax,16
	call	bcdbin
	mov	ecx,eax			; save seconds for later
	push	ecx
	push	eax
	fpush32	0.104719755		; 2*pi/60
	push	dword [SECRX]
	push	dword [SECRY]
	push	esi
	push	edi
	call	getHandCoords
	mov	eax,MOS_SC_DRAWLINE
	shl	ebx,16
	or	ebx,esi
	shl	ecx,16
	or	ecx,edi
	mov	edx,[wndColors + MOS_WNDCOLORS.workText]
	int	0x40
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
	push	ecx
	push	eax
	fpush32 0.001745329		; 2*pi/60/60
	push	dword [MINRX]
	push	dword [MINRY]
	push	esi
	push	edi
	call	getHandCoords
	mov	eax,MOS_SC_DRAWLINE
	shl	ebx,16
	or	ebx,esi
	shl	ecx,16
	or	ecx,edi
	mov	edx,[wndColors + MOS_WNDCOLORS.workText]
	int	0x40
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
	push	eax
	fpush32	0.000145444		; 2*pi/60/60/12
	push	dword [HOURRX]
	push	dword [HOURRY]
	push	esi
	push	edi
	call	getHandCoords
	mov	eax,MOS_SC_DRAWLINE
	shl	ebx,16
	or	ebx,esi
	shl	ecx,16
	or	ecx,edi
	mov	edx,[wndColors + MOS_WNDCOLORS.workText]
	int	0x40
	pop	edx
	
	; draw tick marks
	mov	dword [i],11		; draw 12 marks
.drawtickmarks:
	push	dword [i]		; calculate start point
	fpush32	0.523598776		; 2*pi/12
	push	dword [TMR1X]
	push	dword [TMR1Y]
	push	esi
	push	edi
	call	getHandCoords
	mov	eax,ebx			; save in eax and edx
	mov	edx,ecx
	push	dword [i]
	fpush32	0.523598776		; 2*pi/12
	push	dword [TMR2X]
	push	dword [TMR2Y]
	push	esi
	push	edi
	call	getHandCoords
	shl	eax,16
	shl	edx,16
	or	ebx,eax			; ebx = x start and end
	or	ecx,edx			; ecx = y start and end
	mov	edx,[wndColors + MOS_WNDCOLORS.workText]
	mov	eax,MOS_SC_DRAWLINE
	int	0x40
	dec	dword [i]
	jns	.drawtickmarks

	%define	DATE_WIDTH	48

	; calculate text start position
	mov	eax,[procInfo+MOS_PROCESSINFO.wndWidth]
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
	mov	edx,[procInfo+MOS_PROCESSINFO.wndHeight]
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
	mov	eax,MOS_SC_GETDATE
	int	0x40
	mov	edi,eax

	; display month
	mov	eax,edi			; get month
	shr	eax,8
	call	bcdbin
					; ebx contains already position
	mov	ecx,[wndColors+MOS_WNDCOLORS.workText]
	lea	edx,[monthNames-3+eax*2+eax]; -3 because eax = 1..12 =]
	mov	esi,3			; text length
	mov	eax,MOS_SC_WRITETEXT
	int	0x40

	; display date
	add	ebx,MOS_DWORD(3*6+3,0)
	mov	eax,edi			; get date
	shr	eax,16
	call	bcdbin
	mov	edx,ebx			; position must be in edx
	mov	ebx,0x00020000		; number, display two digits
	mov	ecx,eax			; number to display
	mov	esi,[wndColors+MOS_WNDCOLORS.workText]
	mov	eax,MOS_SC_WRITENUMBER
	int	0x40

	; display year. the way we avoid the y2k bug is even
	; simpler, yet much better than in the last version:
	; now we simply display the last two digits and let the
	; user decide wether it's the year 1903 or 2003 =]
	add	edx,MOS_DWORD(2*6+3,0)
	mov	eax,edi			; get year
	call	bcdbin
	mov	ebx,0x00020000		; number, display two digits
	mov	ecx,eax			; number to display
					; edx contains already position
	mov	esi,[wndColors+MOS_WNDCOLORS.workText]
	mov	eax,MOS_SC_WRITENUMBER
	int	0x40

.byebye:
	popfd
	popad
	leave
	ret
	%pop


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
getHandCoords:

ANGLE	equ	28
DEG2RAD	equ	24
RADIUSX	equ	20
RADIUSY	equ	16
CENTERX	equ	12
CENTERY	equ	8

	enter	0,0
	pushfd

	fild dword [ebp+ANGLE]		; get angle
	fmul dword [ebp+DEG2RAD]	; convert to radians
	fsincos
	fmul dword [ebp+RADIUSY]	; -y * radius + clockcy
	fchs
	fiadd dword [ebp+CENTERY]
	fistp dword [ebp+CENTERY]	
	fmul dword [ebp+RADIUSX]	; x * radius + clockcx
	fiadd dword [ebp+CENTERX]
	fistp dword [ebp+CENTERX]
	
	mov ebx,[ebp+CENTERX]
	mov ecx,[ebp+CENTERY]

	popfd
	leave
	ret	4*6


%endif

