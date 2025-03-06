; aclock 1.1
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

use32
	org 0
	db 'MENUET01'
	dd 1,main,image_end,memory_end,stacktop,cmdLine,0

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'


;********************************************************************
;	configuration stuff
;********************************************************************
	; skinned window borders
MOS_WND_SKIN_BORDER_LEFT	= 5
MOS_WND_SKIN_BORDER_RIGHT	= 5
MOS_WND_SKIN_BORDER_BOTTOM	= 5

	; default window position/dimensions (work area)
	DEFAULT_XPOS	=-20
	DEFAULT_YPOS	=20
	DEFAULT_WIDTH	=110
	DEFAULT_HEIGHT	=110

	; minimal size (horizontal and vertical) of work area
	MIN_WIDTH	=100
	MIN_HEIGHT	=100


	; these includes introduce code and thus mustn't stand
	; before the menuet header =)
	include 'dbgboard.inc'
	include 'strfunct.inc'
	include 'cmdline.inc'
	include 'adjstwnd.inc'
	include 'draw.inc'

;********************************************************************
;	main program
;********************************************************************
main:
	call	getDefaultWindowColors
	call	parseCommandLine

	; check minimal window dimensions
	cmp	dword [wndWidth],MIN_WIDTH
	jae	.widthok
	mov	dword [wndWidth],MIN_WIDTH
.widthok:
	cmp	dword [wndHeight],MIN_HEIGHT
	jae	.heightok
	mov	dword [wndHeight],MIN_HEIGHT
.heightok:

	; adjust window dimensions
	mov	eax,ADJSTWND_TYPE_SKINNED
	mov	ebx,[wndXPos]
	mov	ecx,[wndYPos]
	mov	edx,[wndWidth]
	mov	esi,[wndHeight]
	call	adjustWindowDimensions
	mov	[wndXPos],ebx
	mov	[wndYPos],ecx
	mov	[wndWidth],edx
	mov	[wndHeight],esi

	call	drawWindow
.msgpump:
;	call	drawClock

	; wait up to a second for next event
	mcall SF_WAIT_EVENT_TIMEOUT,100

	test	eax,eax
	jne	.event_occured
	call	drawClock

  .event_occured:
	cmp	eax,EV_REDRAW
	je	.redraw
	cmp	eax,EV_KEY
	je	.key
	cmp	eax,EV_BUTTON
	je	.button
	jmp	.msgpump

.redraw:
	call	drawWindow
	jmp	.msgpump
.key:
	mcall SF_GET_KEY
	jmp	.msgpump
.button:
	mcall SF_TERMINATE_PROCESS
	jmp	.msgpump


;********************************************************************
;	get default window colors
;	input		:	nothing
;	output		:	wndColors contains default colors
;	destroys	:	nothing
;********************************************************************
getDefaultWindowColors:
	pushad
	pushfd
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,wndColors,sizeof.system_colors
	popfd
	popad
	ret


;********************************************************************
;	define and draw window
;	input		nothing
;	output		nothing
;	destroys	flags
;********************************************************************
	align	4
drawWindow:
	pusha

	; start window redraw
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	; create window
	mov	ebx,[wndXPos]
	shl	ebx,16
	or	ebx,[wndWidth]
	mov	ecx,[wndYPos]
	shl	ecx,16
	or	ecx,[wndHeight]
	mov	edx,[wndColors.work]
	or	edx,0x53000000
	mov	edi,w_label
	mcall SF_CREATE_WINDOW

	call drawClock
	
	; end window redraw
	mcall SF_REDRAW,SSF_END_DRAW
	popa
	ret


;********************************************************************
;	initialized data
;********************************************************************

	; window position and dimensions.
	; dimensions are for work area only.
wndXPos		dd	DEFAULT_XPOS
wndYPos		dd	DEFAULT_YPOS
wndWidth	dd	DEFAULT_WIDTH
wndHeight	dd	DEFAULT_HEIGHT

	; window label
w_label:		db	"Clock",0
.end:
LABEL_LEN	equ	(w_label.end-w_label-1)

	; token delimiter list for command line
delimiters	db	9,10,11,12,13,32,0

image_end:


;********************************************************************
;	uninitialized data
;********************************************************************
align 4
wndColors system_colors
procInfo process_information

	; space for command line. at the end we have an additional
	; byte for a terminating zero, just to be sure...
cmdLine		rb	257

align 4
	rb	1024
stacktop:
memory_end:

