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


WND_CENTER			equ	(1 shl 0)
WND_DEFAULT_WORKCOLOR		equ	(1 shl 1)
WND_DEFAULT_GRABCOLOR		equ	(1 shl 2)
WND_DEFAULT_FRAMECOLOR		equ	(1 shl 3)
WND_DEFAULT_CAPTIONCOLOR	equ	(1 shl 4)
WND_DEFAULT_COLORS		equ	(WND_DEFAULT_WORKCOLOR | WND_DEFAULT_GRABCOLOR | WND_DEFAULT_FRAMECOLOR | WND_DEFAULT_CAPTIONCOLOR)


struct WND
	xposandsize	rd	1	; x position and size (like syscall)
	yposandsize	rd	1	; y position and size (like syscall)
	workcolor	rd	1	; work area color (like syscall)
	grabcolor	rd	1	; grab bar color (like syscall)
	framecolor	rd	1	; frame color (like syscall)
	caption	rd	1	; pointer to caption (zero terminated)
					; can be zero, if no caption is desired.
	captioncolor	rd	1	; caption color
	flags		rd	1	; combination of WND_xxx flags, or zero.
ends

USE_SYSTEM_COLORS		equ 0 ;0 or 1
BUTTON_COLOR_WORK		equ	0x505050


struct BUTTON
	xposandsize	rd	1	; x position and size (like syscall)
	yposandsize	rd	1	; y position and size (like syscall)
	id		rd	1	; button id
	color		rd	1	; button color. can be a real color
					; or one of the BUTTON_COLOR_xxx constants
ends


LABEL_COLOR_WORKBUTTON	equ	0xffffff	; use work button text default color
LABEL_BGCOLOR_TRANSPARENT equ	0x01000000	; transparent


struct LABEL
	position	rd	1	; position, x in upper word, y in lower word
	caption	rd	1	; pointer to caption (zero terminated)
					; if this is field is zero, the label will
					; not be drawn.
	color		rd	1	; text color, or a LABEL_COLOR_xxx constant
	bgcolor	rd	1	; background color, or a LABEL_BGCOLOR_xxx constant
ends


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
	pushfd
	pushad

	; get default window colors
	mcall SF_STYLE_SETTINGS,3,sc,sizeof.system_colors

	;
	; window position
	;
	test dword [edi + WND.flags],WND_CENTER		; center window ?
	jnz .center
	mov ebx,[edi + WND.xposandsize]			; nope -> just load dimensions
	mov ecx,[edi + WND.yposandsize]
	jmp .positionok
.center:						; so let's center this window...
	mcall SF_GET_SCREEN_SIZE	; get screen dimensions
	mov ebx,eax					; xpos = (screenx-width)/2
	shr ebx,16
	sub bx,word[edi + WND.xposandsize]
	jns .xok
	xor ebx,ebx
.xok:
	shl ebx,15					; / 2, move result to hi-word
	mov bx,word[edi + WND.xposandsize]
	movzx ecx,ax					; same for ypos
	sub cx,word[edi + WND.yposandsize]
	jns .yok
	xor ecx,ecx
.yok:
	shl ecx,15
	mov cx,word[edi + WND.yposandsize]
.positionok:						; ebx/ecx contain dimensions

	; define window
	mov edx,[edi + WND.workcolor]
	mov edi,windowtitle
	mcall SF_CREATE_WINDOW

	popad
	popfd
	ret


;***********************************************************
; draw a bunch of buttons
;
; input:	edi = pointer to an array of BUTTON structs
;		ecx = # of buttons to draw
; output:	nothing
; destroys:	nothing
; notes:	you must call begin redraw/end redraw yourself
;***********************************************************
align 4
drawButtons:
	or ecx,ecx
	jnz .ok
	ret
.ok:
	pushfd
	pushad
	
	; get default window colors
	if USE_SYSTEM_COLORS eq 1
	push ecx
	mcall SF_STYLE_SETTINGS,3,sc,sizeof.system_colors
	pop ecx
	end if
align 4
.drawall:
	push ecx
	mov ebx,[edi + BUTTON.xposandsize]
	mov ecx,[edi + BUTTON.yposandsize]
	mov edx,[edi + BUTTON.id]
	if USE_SYSTEM_COLORS eq 1
	mov esi,[sc.work_button] ; use a system color
	else
	mov esi,[edi + BUTTON.color] ; use a default color
	end if
	mcall SF_DEFINE_BUTTON
	add edi,sizeof.BUTTON
	pop ecx
	loop .drawall
	
	popad
	popfd
	ret


;***********************************************************
; draw a bunch of labels
;
; input:	edi = pointer to an array of LABEL structs
;		ecx = # of labels to draw
; output:	nothing
; destroys:	nothing
;***********************************************************
align 4
drawLabels:
	or ecx,ecx
	jnz .ok
	ret
.ok:
	pushfd
	pushad
	
	; get default window colors
	if USE_SYSTEM_COLORS eq 1
	push ecx
	mcall SF_STYLE_SETTINGS,3,sc,sizeof.system_colors
	pop ecx
	end if
	
.drawall:
	push ecx
	
	cmp dword [edi + LABEL.caption],0
	jne .notnull
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
	if USE_SYSTEM_COLORS eq 1
	mov edx,[sc.work]
	else
	mov edx,[edi + LABEL.bgcolor]
	end if	
	mcall SF_DRAW_RECT
.clearok:	

	; draw label	
	mov ebx,[edi + LABEL.position]		; ebx = label position
	mov edx,[edi + LABEL.caption]		; edx -> caption

	if USE_SYSTEM_COLORS eq 1
	mov ecx,[sc.work_button_text]
	else
	mov ecx,[edi + LABEL.color]		; ecx = color
	end if
	mcall SF_DRAW_TEXT

.next:
	add edi,sizeof.LABEL			; next label
	pop ecx
	dec ecx
	jz .done
	jmp .drawall
.done:

	popad
	popfd
	ret

