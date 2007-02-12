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

	bits 32
	%include 'mos.inc'
	section .text


;********************************************************************
;	configuration stuff
;********************************************************************

	%define APPNAME		"AClock 1.1"
	%define STACKSIZE	1024

	; default window position/dimensions (work area)
	%define DEFAULT_XPOS	-20
	%define DEFAULT_YPOS	20
	%define DEFAULT_WIDTH	100
	%define DEFAULT_HEIGHT	100

	; minimal size (horizontal and vertical) of work area
	%define MIN_WIDTH	100
	%define MIN_HEIGHT	100


;********************************************************************
;	header
;********************************************************************
	
	MOS_HEADER01 main,image_end,memory_end,stacktop-4,cmdLine,0

	; these includes introduce code and thus mustn't stand
	; before the menuet header =)
	%include 'dbgboard.inc'
	%include 'strlen.inc'
	%include 'str2dwrd.inc'
	%include 'strtok.inc'
	%include 'cmdline.inc'
	%include 'adjstwnd.inc'
	%include 'draw.inc'

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
	call	drawClock

	; wait up to a second for next event
	mov	eax,MOS_SC_WAITEVENTTIMEOUT
	mov	ebx,100
	int	0x40

	cmp	eax,MOS_EVT_REDRAW
	je	.redraw
	cmp	eax,MOS_EVT_KEY
	je	.key
	cmp	eax,MOS_EVT_BUTTON
	je	.button
	jmp	.msgpump

.redraw:
	call	drawWindow
	jmp	.msgpump
.key:
	mov	eax,MOS_SC_GETKEY
	int	0x40
	jmp	.msgpump
.button:
	mov	eax,MOS_SC_EXIT
	int	0x40
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
	mov	eax,MOS_SC_WINDOWPROPERTIES
	mov	ebx,3
	mov	ecx,wndColors
	mov	edx,MOS_WNDCOLORS_size
	int	0x40
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
	mov	eax,MOS_SC_REDRAWSTATUS
	mov	ebx,1
	int	0x40

	; create window
	mov	eax,MOS_SC_DEFINEWINDOW
	mov	ebx,[wndXPos]
	shl	ebx,16
	or	ebx,[wndWidth]
	mov	ecx,[wndYPos]
	shl	ecx,16
	or	ecx,[wndHeight]
	mov	edx,[wndColors+MOS_WNDCOLORS.work]
	or	edx,0x03000000
	mov	esi,[wndColors+MOS_WNDCOLORS.grab]
	mov	edi,[wndColors+MOS_WNDCOLORS.frame]
	int	0x40

	; draw window label
	mov	eax,MOS_SC_WRITETEXT
	mov	ebx,MOS_DWORD(8,8)
	mov	ecx,[wndColors+MOS_WNDCOLORS.grabText]
	mov	edx,label
	mov	esi,LABEL_LEN
	int	0x40
		
	call drawClock
	
	; end window redraw
	mov	eax,MOS_SC_REDRAWSTATUS
	mov	ebx,2
	int	0x40
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
label		db	APPNAME,0
LABEL_LEN	equ	($-label-1)

	; token delimiter list for command line
delimiters	db	9,10,11,12,13,32,0

	; don't insert anything after this label
image_end:


;********************************************************************
;	uninitialized data
;********************************************************************
	section .bss

wndColors	resb	MOS_WNDCOLORS_size
procInfo	resb	MOS_PROCESSINFO_size

	; space for command line. at the end we have an additional
	; byte for a terminating zero, just to be sure...
cmdLine		resb	257

	alignb	4
stack		resb	STACKSIZE
stacktop:

	; don't insert anything after this label
memory_end:

