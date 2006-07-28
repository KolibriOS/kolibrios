; mos.inc 0.03
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
%ifndef _MOS_INC
%define _MOS_INC


;**********************************************************
; generates a menuetos 01 header
; takes 6 parameters:
;
; MOS_HEADER01 start, end, appmem, esp, i_param, i_icon
;**********************************************************

%macro MOS_HEADER01 6
	org	0x0
	db	'MENUET01'		; 8 byte id
	dd	0x01			; header version
	dd	%1			; start of code
	dd	%2			; image size
	dd	%3			; application memory
	dd	%4			; esp
	dd	%5			; i_param
	dd	%6			; i_icon
%endmacro


;**********************************************************
; MOS_DWORD
; packs 2 words into a double word
;**********************************************************

%define MOS_DWORD(hi,lo) ((((hi) & 0xffff) << 16) + ((lo) & 0xffff))


;**********************************************************
; MOS_RGB
; creates a menuet os compatible color (0x00RRGGBB)
;**********************************************************

%define MOS_RGB(r,g,b) ((((r) & 255) << 16) + (((g) & 255) << 8) + ((b) & 255))


;**********************************************************
; window stuff
;**********************************************************

	; default window colors
struc MOS_WNDCOLORS
	.frame:			resd	1
	.grab:			resd	1
	.grabButton:		resd	1
	.grabButtonText:	resd	1
	.grabText:		resd	1
	.work:			resd	1
	.workButton:		resd	1
	.workButtonText:	resd	1
	.workText:		resd	1
	.workGraphics:		resd	1
endstruc

	; skinned window borders
MOS_WND_SKIN_BORDER_LEFT	equ	5
MOS_WND_SKIN_BORDER_RIGHT	equ	5
MOS_WND_SKIN_BORDER_BOTTOM	equ	5


;**********************************************************
; process info structure
;**********************************************************

struc MOS_PROCESSINFO
	.CPUUsage: 		resd	1
	.windowStackPos:	resw	1
	.windowStackVal:	resw	1
	.reserved1:		resw	1
	.processName:		resb	12
	.memStart:		resd	1
	.memUsed:		resd	1
	.pid:			resd	1
	.wndXPos		resd	1
	.wndYPos		resd	1
	.wndWidth		resd	1
	.wndHeight		resd	1
	.reserved2:		resb	(1024 - 50)
endstruc



;**********************************************************
; system call numbers
;**********************************************************

MOS_SC_EXIT		equ	-1
MOS_SC_DEFINEWINDOW	equ	0
MOS_SC_PUTPIXEL		equ	1
MOS_SC_GETKEY		equ	2
MOS_SC_GETSYSCLOCK	equ	3
MOS_SC_WRITETEXT	equ	4
MOS_SC_DELAY		equ	5
MOS_SC_OPENFILEFLOPPY	equ	6	; obsolete
MOS_SC_PUTIMAGE		equ	7
MOS_SC_DEFINEBUTTON	equ	8
MOS_SC_GETPROCESSINFO	equ	9
MOS_SC_WAITEVENT	equ	10
MOS_SC_CHECKEVENT	equ	11
MOS_SC_REDRAWSTATUS	equ	12
MOS_SC_DRAWBAR		equ	13
MOS_SC_GETSCREENMAX	equ	14
MOS_SC_SETBACKGROUND	equ	15
MOS_SC_GETPRESSEDBUTTON	equ	17
MOS_SC_SYSTEMSERVICE	equ	18
MOS_SC_STARTPROGRAM	equ	19	; obsolete
MOS_SC_MIDIINTERFACE	equ	20
MOS_SC_DEVICESETUP	equ	21
MOS_SC_WAITEVENTTIMEOUT	equ	23
MOS_SC_CDAUDIO		equ	24
MOS_SC_SB16MIXER1	equ	25
MOS_SC_GETDEVICESETUP	equ	26
MOS_SC_WSS		equ	27
MOS_SC_SB16MIXER2	equ	28
MOS_SC_GETDATE		equ	29
MOS_SC_READHD		equ	30	; obsolete
MOS_SC_STARTPROGRAMHD	equ	31	; obsolete
MOS_SC_DELETEFILEFLOPPY	equ	32
MOS_SC_SAVEFILERAMDISK	equ	33	; obsolete
MOS_SC_READDIRRAMDISK	equ	34	; obsolete
MOS_SC_GETSCREENPIXEL	equ	35
MOS_SC_GETMOUSEPOSITION	equ	37
MOS_SC_DRAWLINE		equ	38
MOS_SC_GETBACKGROUND	equ	39
MOS_SC_SETEVENTMASK	equ	40
MOS_SC_GETIRQOWNER	equ	41
MOS_SC_GETDATAREADBYIRQ	equ	42
MOS_SC_SENDDATATODEVICE	equ	43
MOS_SC_PROGRAMIRQS	equ	44
MOS_SC_RESERVEFREEIRQ	equ	45
MOS_SC_RESERVEFREEPORTS	equ	46
MOS_SC_WRITENUMBER	equ	47
MOS_SC_WINDOWPROPERTIES	equ	48
MOS_SC_SHAPEDWINDOWS	equ	50
MOS_SC_CREATETHREAD	equ	51
MOS_SC_STACKDRIVERSTATE	equ	52
MOS_SC_SOCKETINTERFACE	equ	53
MOS_SC_SOUNDINTERFACE	equ	55
MOS_SC_WRITEFILEHD	equ	56	; obsolete
MOS_SC_DELETEFILEHD	equ	57
MOS_SC_SYSTREEACCESS	equ	58
MOS_SC_SYSCALLTRACE	equ	59
MOS_SC_IPC		equ	60
MOS_SC_DIRECTGRAPHICS	equ	61
MOS_SC_PCI		equ	62
MOS_SC_DEBUGBOARD	equ	63


;**********************************************************
; event numbers
;**********************************************************

MOS_EVT_NONE		equ	0
MOS_EVT_REDRAW		equ	1
MOS_EVT_KEY		equ	2
MOS_EVT_BUTTON		equ	3


;**********************************************************
; event bits
;**********************************************************

MOS_EVTBIT_REDRAW	equ	(1 <<  0)
MOS_EVTBIT_KEY		equ	(1 <<  1)
MOS_EVTBIT_BUTTON	equ	(1 <<  2)
MOS_EVTBIT_ENDREQUEST	equ	(1 <<  3)
MOS_EVTBIT_BGDRAW	equ	(1 <<  4)
MOS_EVTBIT_MOUSECHANGE	equ	(1 <<  5)
MOS_EVTBIT_IPCEVENT	equ	(1 <<  6)
MOS_EVTBIT_IRQ0		equ	(1 << 16)
MOS_EVTBIT_IRQ1		equ	(1 << 17)
MOS_EVTBIT_IRQ2		equ	(1 << 18)
MOS_EVTBIT_IRQ3		equ	(1 << 19)
MOS_EVTBIT_IRQ4		equ	(1 << 20)
MOS_EVTBIT_IRQ5		equ	(1 << 21)
MOS_EVTBIT_IRQ6		equ	(1 << 22)
MOS_EVTBIT_IRQ7		equ	(1 << 23)
MOS_EVTBIT_IRQ8		equ	(1 << 24)
MOS_EVTBIT_IRQ9		equ	(1 << 25)
MOS_EVTBIT_IRQ10	equ	(1 << 26)
MOS_EVTBIT_IRQ11	equ	(1 << 27)
MOS_EVTBIT_IRQ12	equ	(1 << 28)
MOS_EVTBIT_IRQ13	equ	(1 << 29)
MOS_EVTBIT_IRQ14	equ	(1 << 30)
MOS_EVTBIT_IRQ15	equ	(1 << 31)


;**********************************************************
; exit application (syscall -1)
;**********************************************************

; exit application
%macro MOS_EXIT 0
	mov eax,MOS_SC_EXIT
	int 0x40
%endmacro

; exit application, smaller version
%macro MOS_EXIT_S 0
	xor eax,eax
	dec eax
	int 0x40
%endmacro


;**********************************************************
; wait event stuff
; (MOS_SC_WAITEVENT, syscall 10)
;**********************************************************

; wait for event
; destroys	:	nothing
; returns	:	eax = event type
%macro MOS_WAITEVENT 0
	mov eax,MOS_SC_WAITEVENT
	int 0x40
%endmacro

; wait for event, smaller version
; destroys	:	flags
; returns	:	eax = event type
%macro MOS_WAITEVENT_S 0
	xor eax,eax
	mov al,MOS_SC_WAITEVENT
	int 0x40
%endmacro


;**********************************************************
; window redraw status stuff
; (MOS_SC_REDRAWSTATUS, syscall 12)
;**********************************************************

MOS_RS_STARTREDRAW	equ	1
MOS_RS_ENDREDRAW	equ	2

; start window redraw
; destroys: eax,ebx
%macro MOS_STARTREDRAW 0
	mov ebx,MOS_RS_STARTREDRAW
	mov eax,MOS_SC_REDRAWSTATUS
	int 0x40
%endmacro

; start window redraw, smaller version
; destroys: eax,ebx,flags
%macro MOS_STARTREDRAW_S 0
	xor ebx,ebx
	inc ebx
	xor eax,eax
	mov al,MOS_SC_REDRAWSTATUS
	int 0x40
%endmacro

; end window redraw
; destroys: eax,ebx
%macro MOS_ENDREDRAW 0
	mov ebx,MOS_RS_ENDREDRAW
	mov eax,MOS_SC_REDRAWSTATUS
	int 0x40
%endmacro

; end window redraw, smaller version
; destroys: eax,ebx,flags
%macro MOS_ENDREDRAW_S 0
	xor ebx,ebx
	mov bl,MOS_RS_ENDREDRAW
	xor eax,eax
	mov al,MOS_SC_REDRAWSTATUS
	int 0x40
%endmacro


;**********************************************************
; get screen max stuff (syscall 14)
;**********************************************************

; get screen dimensions in eax
; destroys: nothing
%macro MOS_GETSCREENMAX 0
	mov eax,MOS_SC_GETSCREENMAX
	int 0x40
%endmacro

; get screen dimensions in eax, smaller version
; destroys: flags
%macro MOS_GETSCREENMAX_S 0
	xor eax,eax
	mov al,MOS_SC_GETSCREENMAX
	int 0x40
%endmacro


;********************************************************************
;	opcode hacks
;********************************************************************

; nasm refuses to assemble stuff like
;	push dword 4.44
; with the following macro this becomes possible:
;	fpush32	9.81
; don't forget to use a decimal point. things like
;	fpush32 1
; will probably not do what you expect. instead, write:
;	fpush32 1.0
%macro fpush32 1
	db	0x68			; push imm32
	dd	%1
%endm

%endif

