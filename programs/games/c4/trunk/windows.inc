; windows.inc
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

%ifndef _WINDOWS_INC
%define _WINDOWS_INC


WND_CENTER			equ	(1 << 0)
WND_DEFAULT_WORKCOLOR		equ	(1 << 1)
WND_DEFAULT_GRABCOLOR		equ	(1 << 2)
WND_DEFAULT_FRAMECOLOR		equ	(1 << 3)
WND_DEFAULT_CAPTIONCOLOR	equ	(1 << 4)
WND_DEFAULT_COLORS		equ	(WND_DEFAULT_WORKCOLOR | WND_DEFAULT_GRABCOLOR | WND_DEFAULT_FRAMECOLOR | WND_DEFAULT_CAPTIONCOLOR)


struc WND
	.xposandsize	resd	1	; x position and size (like syscall)
	.yposandsize	resd	1	; y position and size (like syscall)
	.workcolor	resd	1	; work area color (like syscall)
	.grabcolor	resd	1	; grab bar color (like syscall)
	.framecolor	resd	1	; frame color (like syscall)
	.caption	resd	1	; pointer to caption (zero terminated)
					; can be zero, if no caption is desired.
	.captioncolor	resd	1	; caption color
	.flags		resd	1	; combination of WND_xxx flags, or zero.
endstruc


BUTTON_COLOR_GRAB		equ	0x01000000
BUTTON_COLOR_WORK		equ	0x02000000


struc BUTTON
	.xposandsize	resd	1	; x position and size (like syscall)
	.yposandsize	resd	1	; y position and size (like syscall)
	.id		resd	1	; button id
	.color		resd	1	; button color. can be a real color
					; or one of the BUTTON_COLOR_xxx constants
endstruc


LABEL_COLOR_GRABBUTTON	equ	0x01000000	; use grab button text default color
LABEL_COLOR_GRAB	equ	0x02000000	; use grab text default color
LABEL_COLOR_WORKBUTTON	equ	0x03000000	; use work button text default color
LABEL_COLOR_WORK	equ	0x04000000	; use work text default color
LABEL_BGCOLOR_TRANSPARENT equ	0x01000000	; transparent
LABEL_BGCOLOR_WORK	equ	0x02000000	; use work area color


struc LABEL
	.position	resd	1	; position, x in upper word, y in lower word
	.caption	resd	1	; pointer to caption (zero terminated)
					; if this is field is zero, the label will
					; not be drawn.
	.color		resd	1	; text color, or a LABEL_COLOR_xxx constant
	.bgcolor	resd	1	; background color, or a LABEL_BGCOLOR_xxx constant
endstruc


	section .text


;***********************************************************
; draw a window
;
; input:	edi = pointer to a WND structure
; output:	nothing
; destroys:	nothing
; notes:	you must call begin redraw/end redraw
;		yourself, before and after calling
;		this function.
;***********************************************************
	
drawWindow:

%define WNDCOLORS ebp-MOS_WNDCOLORS_size

	enter MOS_WNDCOLORS_size,0
	pushfd
	pushad

	; get default window colors
	mov ebx,3
	lea ecx,[WNDCOLORS]
	mov edx,MOS_WNDCOLORS_size
	mov eax,MOS_SC_WINDOWPROPERTIES
	int 0x40

	;
	; window position
	;
	test dword [edi + WND.flags],WND_CENTER		; center window ?
	jnz short .center
	mov ebx,[edi + WND.xposandsize]			; nope -> just load dimensions
	mov ecx,[edi + WND.yposandsize]
	jmp short .positionok
.center:						; so let's center this window...
	MOS_GETSCREENMAX				; get screen dimensions
	mov ebx,eax					; xpos = (screenx-width)/2
	shr ebx,16
	sub bx,[edi + WND.xposandsize]
	jns short .xok
	xor ebx,ebx
.xok:
	shl ebx,15					; / 2, move result to hi-word
	mov bx,[edi + WND.xposandsize]
	movzx ecx,ax					; same for ypos
	sub cx,[edi + WND.yposandsize]
	jns short .yok
	xor ecx,ecx
.yok:
	shl ecx,15
	mov cx,[edi + WND.yposandsize]
.positionok:						; ebx/ecx contain dimensions

	;
	; colors
	;
	push edi
	mov edx,[edi + WND.workcolor]
	test dword [edi + WND.flags],WND_DEFAULT_WORKCOLOR
	jz short .workcolorok
	and edx,0xff000000
	or edx,[WNDCOLORS + MOS_WNDCOLORS.work]
.workcolorok:
	mov esi,[edi + WND.grabcolor]
	test dword [edi + WND.flags],WND_DEFAULT_GRABCOLOR
	jz short .grabcolorok
	and esi,0xff000000
	or esi,[WNDCOLORS + MOS_WNDCOLORS.grab]
.grabcolorok:
	test dword [edi + WND.flags],WND_DEFAULT_FRAMECOLOR
	mov edi,[edi + WND.framecolor]
	jz short .framecolorok
	and edi,0xff000000
	or edi,[WNDCOLORS + MOS_WNDCOLORS.frame]
.framecolorok:

	; define window
	mov eax,MOS_SC_DEFINEWINDOW
	int 0x40
	pop edi

	;
	; caption
	;
	cmp dword [edi + WND.caption],0			; is there a caption ?
	je short .captionok				; nope -> do nothing
	mov ebx,MOS_DWORD(8,8)				; ebx = position
	mov edx,[edi + WND.caption]			; edx -> string	
	push edi					; get string length
	mov edi,edx
	mov ecx,-1
	xor al,al					; scan for zero byte
	repne scasb
	sub edi,edx
	dec edi
	mov esi,edi
	pop edi						; esi = string length
	mov ecx,[edi + WND.captioncolor]
	test dword [edi + WND.flags],WND_DEFAULT_CAPTIONCOLOR
	jz short .captioncolorok
	mov ecx,[WNDCOLORS + MOS_WNDCOLORS.grabText]
.captioncolorok:
	mov eax,MOS_SC_WRITETEXT
	int 0x40
.captionok:

	popad
	popfd
	leave
	ret
%undef WNDCOLORS


;***********************************************************
; draw a bunch of buttons
;
; input:	edi = pointer to an array of BUTTON structs
;		ecx = # of buttons to draw
; output:	nothing
; destroys:	nothing
; notes:	you must call begin redraw/end redraw yourself
;***********************************************************

drawButtons:

%define WNDCOLORS ebp-MOS_WNDCOLORS_size

	or ecx,ecx
	jnz short .ok
	ret
.ok:
	enter MOS_WNDCOLORS_size,0
	pushfd
	pushad
	
	; get default window colors
	push ecx
	mov ebx,3
	lea ecx,[WNDCOLORS]
	mov edx,MOS_WNDCOLORS_size
	mov eax,MOS_SC_WINDOWPROPERTIES
	int 0x40
	pop ecx
	
.drawall:
	push ecx
	mov ebx,[edi + BUTTON.xposandsize]
	mov ecx,[edi + BUTTON.yposandsize]
	mov edx,[edi + BUTTON.id]
	mov esi,[edi + BUTTON.color]	
	cmp esi,BUTTON_COLOR_GRAB			; use a default color ?
	jne .ok1
	mov esi,[WNDCOLORS + MOS_WNDCOLORS.grabButton]
.ok1:
	cmp esi,BUTTON_COLOR_WORK
	jne .ok2
	mov esi,[WNDCOLORS + MOS_WNDCOLORS.workButton]
.ok2:	
	mov eax,MOS_SC_DEFINEBUTTON
	int 0x40
	add edi,BUTTON_size
	pop ecx
	loop .drawall
	
	popad
	popfd
	leave
	ret
%undef WNDCOLORS


;***********************************************************
; draw a bunch of labels
;
; input:	edi = pointer to an array of LABEL structs
;		ecx = # of labels to draw
; output:	nothing
; destroys:	nothing
;***********************************************************

drawLabels:

%define WNDCOLORS ebp-MOS_WNDCOLORS_size

	or ecx,ecx
	jnz short .ok
	ret
.ok:
	enter MOS_WNDCOLORS_size,0
	pushfd
	pushad
	
	; get default window colors
	push ecx
	mov ebx,3
	lea ecx,[WNDCOLORS]
	mov edx,MOS_WNDCOLORS_size
	mov eax,MOS_SC_WINDOWPROPERTIES
	int 0x40
	pop ecx
	
.drawall:
	push ecx
	
	cmp dword [edi + LABEL.caption],0
	jne short .notnull
	jmp .next
.notnull:
	
	; get caption length
	push edi
	mov edi,[edi + LABEL.caption]
	mov ecx,-1
	xor al,al
	repne scasb				; search for zero byte
	mov esi,edi
	pop edi
	sub esi,[edi + LABEL.caption]
	dec esi					; esi = string length
	
	; clear background, if necessary
	cmp dword [edi + LABEL.bgcolor],LABEL_BGCOLOR_TRANSPARENT
	je .clearok
	mov ebx,[edi + LABEL.position]		; ebx = xstart/width
	mov eax,esi				; width = stringlength * 6
	mov edx,6
	mul edx
	mov bx,ax
	mov ecx,[edi + LABEL.position]		; ecx = ystart/height
	shl ecx,16
	mov cx,8	
	mov edx,[edi + LABEL.bgcolor]
	cmp edx,LABEL_BGCOLOR_WORK
	jne short .bgcolorok
	mov edx,[WNDCOLORS + MOS_WNDCOLORS.work]
.bgcolorok:
	mov eax,MOS_SC_DRAWBAR
	int 0x40
.clearok:	

	; draw label	
	mov ebx,[edi + LABEL.position]		; ebx = label position
	mov edx,[edi + LABEL.caption]		; edx -> caption
	mov ecx,[edi + LABEL.color]		; ecx = color
	cmp ecx,LABEL_COLOR_GRABBUTTON
	jne short .ok1
	mov ecx,[WNDCOLORS + MOS_WNDCOLORS.grabButtonText]
.ok1:
	cmp ecx,LABEL_COLOR_GRAB
	jne short .ok2
	mov ecx,[WNDCOLORS + MOS_WNDCOLORS.grabText]
.ok2:
	cmp ecx,LABEL_COLOR_WORKBUTTON
	jne short .ok3
	mov ecx,[WNDCOLORS + MOS_WNDCOLORS.workButtonText]
.ok3:
	cmp ecx,LABEL_COLOR_WORK
	jne short .ok4
	mov ecx,[WNDCOLORS + MOS_WNDCOLORS.workText]
.ok4:
	mov eax,MOS_SC_WRITETEXT
	int 0x40

.next:
	add edi,LABEL_size			; next label
	pop ecx
	dec ecx
	jz .done
	jmp .drawall
.done:

	popad
	popfd
	leave
	ret
%undef WNDCOLORS


%endif