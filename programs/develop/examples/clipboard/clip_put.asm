;*****************************************************************************
; Write the data to the clipboard
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
	je	.write_button

	cmp	ah,3
	je	.delete_button
	
	cmp	ah,1
	jne	still
.exit:
	mcall	-1
;--------------------------------------	
.write_button:
	mcall	54,2,buffer_data.end-buffer_data,buffer_data
	mov	[operation_result],eax
	call	redraw_operation_result
	jmp	still
;--------------------------------------	
.delete_button:
	mcall	54,3
	mov	[operation_result],eax
	call	redraw_operation_result
	jmp	still
;---------------------------------------------------------------------
draw_window:
	mcall	12,1
	xor	esi,esi
	mcall	0,<0,350>,<0,100>,0x13FFFFFF,,title
	
	mcall	8,<20,130>,<40,20>,2,0xCCCCCC
	mcall	,<160,155>,,3
	mcall	4,<25,47>,0x90000000,write_button_text
	mcall	,<165,47>,,delete_button_text
	mcall	,<25,77>,,operation_result_text
	call	redraw_operation_result
	
	mcall	12,2
	ret
;---------------------------------------------------------------------
redraw_operation_result:
	mcall	13,<200,100>,<77,8>,0xFFFFFF
	mcall	47,0x80080100,[operation_result],<200,77>,0x10000000
	ret
;---------------------------------------------------------------------
title:
	db 'Write the data to the clipboard',0
write_button_text:
	db 'Write to clipboard',0
delete_button_text:
	db 'Delete from clipboard',0
operation_result_text:
	db 'Result of the operation:',0
;---------------------------------------------------------------------
buffer_data:
	dd buffer_data.end - buffer_data
	dd 0	; type 'text'
	dd 866	; text encoding
	db 'Test message to the clipboard'
.end:
;---------------------------------------------------------------------
IM_END:
operation_result:
	rd 1
;--------------------------------------		
	rb 1024
stacktop:
	rb 4
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------