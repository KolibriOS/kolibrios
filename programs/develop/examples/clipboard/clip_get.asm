;*****************************************************************************
; Read the data from the clipboard
; Copyright (c) 2013, Marat Zakiyanov aka Mario79, aka Mario
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
;******************************************************************************
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
;---------------------------------------------------------------------
include '../../../macros.inc'
;---------------------------------------------------------------------
START:
	mcall	68,11
red:
	call	draw_window
still:
	mcall	10

	cmp	eax,1
	je	red
	cmp	eax,2
	je	key
	cmp	eax,3
	je	button

	jmp	still
;---------------------------------------------------------------------
key:
	mcall	2
	jmp	still
;---------------------------------------------------------------------
button:
	mcall	17
	cmp	ah,2
	je	.read_button
	cmp	ah,1
	jne	still
.exit:
	mcall	-1
;--------------------------------------
.read_button:
	call	draw_clipboard
	jmp	still
;---------------------------------------------------------------------
draw_window:
	mcall	12,1
	xor	esi,esi
	mcall	0,<0,600>,<0,400>,0x13FFFFFF,,title
	mcall	8,<20,150>,<40,20>,2,0xCCCCCC
	mcall	4,<25,47>,0x90000000,read_button_text
	mcall	12,2
	ret
;---------------------------------------------------------------------
draw_clipboard:
	mcall	54,0
	cmp	eax,-1
	je	.exit
	
	test	eax,eax
	jz	.exit
	
	mov	[slots_number],eax
	
	xor	eax,eax
	mov	[current_slot],eax
	
	mov	[text_coordinates],dword 10 shl 16+70
.start:
	xor	eax,eax
	mov	[current_slot_data],eax
	mcall	54,1,[current_slot]

	cmp	eax,-1
	je	.no_relevant_data
	
	cmp	eax,1
	jne	@f
	
.no_relevant_data:
	mov	edx,no_relevant_data_text
	mov	esi,no_relevant_data_text.end-no_relevant_data_text
	jmp	.print
.no_relevant_data_1:
	mov	edx,no_relevant_data_text_1
	mov	esi,no_relevant_data_text_1.end-no_relevant_data_text_1
	jmp	.print
.no_relevant_data_2:
	mov	edx,no_relevant_data_text_2
	mov	esi,no_relevant_data_text_2.end-no_relevant_data_text_2
	jmp	.print
@@:
	mov	[current_slot_data],eax
	mov	eax,[current_slot_data]
	mov	esi,[eax]
	sub	esi,12
	add	eax,4
	cmp	[eax],dword 0
	jne	.no_relevant_data_1
	add	eax,4	
	cmp	[eax],dword 866
	jne	.no_relevant_data_2
	
	add	eax,4
	mov	edx,eax

.print:
	mcall	4,[text_coordinates],0x0
	
	mov	ecx,[current_slot_data]
	test	ecx,ecx
	jz	@f
	mcall	68,13
@@:
	add	[text_coordinates],dword 15
	inc	dword [current_slot]
	mov	eax,[slots_number]
	dec	eax
	mov	[slots_number],eax
	jnz	.start
.exit:
	ret
;---------------------------------------------------------------------
title:
	db 'Read the data from the clipboard',0
read_button_text:
	db 'Read from clipboard',0
no_relevant_data_text:	
	db '<NO RELEVANT DATA>',0
.end:
no_relevant_data_text_1:	
	db '<NO TEXT>',0
.end:
no_relevant_data_text_2:	
	db '<NO 866>',0
.end:
;---------------------------------------------------------------------
IM_END:
slots_number:
	rd 1
text_coordinates:
	rd 1
current_slot:
	rd 1
current_slot_data:
	rd 1
;--------------------------------------
	rb 1024
stacktop:
	rb 4
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------