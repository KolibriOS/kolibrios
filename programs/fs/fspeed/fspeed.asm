;*****************************************************************************
; File Speed - for Kolibri OS
; Copyright (c) 2014, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;        * Redistributions of source code must retain the above copyright
;          notice, this list of conditions and the following disclaimer.
;        * Redistributions in binary form must reproduce the above copyright
;          notice, this list of conditions and the following disclaimer in the
;          documentation and/or other materials provided with the distribution.
;        * Neither the name of the <organization> nor the
;          names of its contributors may be used to endorse or promote products
;          derived from this software without specific prior written permission.
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
;*****************************************************************************
;-----------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'
	dd 0x01
	dd START
	dd IM_END
	dd I_END
	dd STACK_TOP
	dd 0x0
	dd cur_dir_path
;-----------------------------------------------------------------------------
include	'lang.inc'
include	'../../macros.inc'
define __DEBUG__ 1
define __DEBUG_LEVEL__ 1
include	'../../debug-fdo.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../develop/libraries/box_lib/load_lib.mac'
	@use_library
;-----------------------------------------------------------------------------
struct  RESULT_SLOT
        text            dd ?
        read_speed      dd ?
        write_speed     dd ?
        chunk_size      dd ?
ends
;-----------------------------------------------------------------------------
START:
        DEBUGF 1,'FSPEED: start of programm\n'
;-----------------------------------------------------------------------------	
	mcall	68,11
	test	eax,eax
	jz	exit	
;-----------------------------------------------------------------------------	
load_libraries l_libs_start,end_l_libs
;if return code =-1 then exit, else nornary work
	inc	eax
	test	eax,eax
	jz	exit
;-----------------------------------------------------------------------------
;OpenDialog	initialisation
	push	dword OpenDialog_data
	call	[OpenDialog_Init]

	push	check1
	call	[init_checkbox]

	mcall	40,0x27
;-----------------------------------------------------------------------------
red:
	call	draw_window
;-----------------------------------------------------------------------------
still:
	mcall	10
	cmp	eax,1
	je	red

	cmp	eax,2
	je	key

	cmp	eax,3
	je	button

	push	dword check1
	call	[check_box_mouse]

	jmp	still
;-----------------------------------------------------------------------------
key:
	mcall	2
	jmp	still
;-----------------------------------------------------------------------------
button:
	mcall	17

	cmp	ah,2
	je	select_file

	cmp	ah,3
	je	testing

	cmp	ah,1
	jne	still
;--------------------------------------
exit:
	mcall	-1
;-----------------------------------------------------------------------------
select_file:
; invoke OpenDialog
	mov	[OpenDialog_data.type],dword 0
	push	dword OpenDialog_data
	call	[OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	jne	still
; prepare for PathShow
	push	dword PathShow_data
	call	[PathShow_prepare]

	call	draw_PathShow
	jmp	still
;-----------------------------------------------------------------------------
draw_PathShow:
	mcall	13,<5,400-20>,<5,15>,0xffffff
; draw for PathShow
	push	dword PathShow_data
	call	[PathShow_draw]
	ret
;-----------------------------------------------------------------------------
draw_window:
	mcall	48,3,app_colours,4*10	; get current colors

	mcall	12,1
	xor	esi,esi
	xor	ebp,ebp
	mov	edx,[w_work]	; color of work area RRGGBB,8->color
	or	edx,0x34000000
	mcall	0,<100,400>,<100,300>,,,title

	call	draw_PathShow
	mov	eax,[w_work_text]
	or	eax,0x80000000
	mov	[check1.text_color],eax
	push	dword check1
	call	[check_box_draw]
	
	mcall	8,<5,80>,<25,15>,2,[w_work_button]
	mcall	,<400-65,50>,,3
	mov	ecx,[w_work_button_text]
	or	ecx,0x80000000
	mcall	4,<5+10,25+4>,,s_text
	mcall	,<400-65+10,25+4>,,r_text
	mov	ecx,[w_work_text]
	or	ecx,0x80000000
	mcall	,<10,47>,,check_box_warning_text
	mcall	,<10,65>,,result_table_text
	
	mov	edx,ecx
	and	edx,0xffffff
	mcall	38,<5,400-15>,<59,59>
; draw result table	
	mov	ebx,10 shl 16+77
	mov	ebp,result_table
	mov	ecx,18
;--------------------------------------
@@:
	push	ecx
	mov	ecx,[w_work_text]
	or	ecx,0x80000000	
	mcall	4,,,[ebp+RESULT_SLOT.text]
	push	ebx
	mov	edx,ebx
	add	edx,(11*6) shl 16
	mov	ebx,0x800a0000
	mcall	47,,[ebp+RESULT_SLOT.read_speed],,[w_work_text]
	add	edx,(16*6) shl 16
	mcall	,,[ebp+RESULT_SLOT.write_speed]
	pop	ebx
	add	ebx,6+5
	add	ebp,sizeof.RESULT_SLOT
	pop	ecx
	dec	ecx
	jnz	@b

	mcall	12,2
	ret
;-----------------------------------------------------------------------------;-----------------------------------------------------------------------------
testing:
	mcall	70,fileinfo
	test	eax,eax
	jz	@f
	
        DEBUGF 1,'FSPEED: file not found %s\n',fname
	jmp	still
;--------------------------------------
@@:
        DEBUGF 1,'FSPEED: target file %s\n',fname
	mov	ebp,result_table
	mov	ecx,18
;--------------------------------------
@@:
	push	ecx
	call	read_chunk

	pusha
	call	draw_window
	popa

	call	write_chunk

	pusha
	call	draw_window
	popa

	pop	ecx
	add	ebp,sizeof.RESULT_SLOT

	dec	ecx
	jnz	@b

	jmp	still
;-----------------------------------------------------------------------------
read_chunk:
	mov	eax,[file_info+32] ; file size
	cmp	[ebp+RESULT_SLOT.chunk_size],eax
	jb	@f
	
	xor	eax,eax ; small file size for current chunk size
	mov	[ebp+RESULT_SLOT.read_speed],eax
	ret
;--------------------------------------
@@:
	mcall	68,12,[ebp+RESULT_SLOT.chunk_size]
	mov	[fileread.return],eax
	xor	eax,eax
	mov	[fileread.offset],eax ; zero current offset
	mcall	26,9 ; get start time
	add	eax,1600 ; 16 sec for iterations
	mov	esi,eax
	mov	ecx,1
	mov	eax,[ebp+RESULT_SLOT.chunk_size]
	mov	[fileread.size],eax
;--------------------------------------
.loop:
	mcall	70,fileread
	
	mcall	26,9 ; check current time
	cmp	esi,eax
	jbe	.end
; correct offset	
	mov	edx,[ebp+RESULT_SLOT.chunk_size]
	add	[fileread.offset],edx ; current offset
; check offset and file size	
	mov	edx,[file_info+32] ; file size
	sub	edx,[ebp+RESULT_SLOT.chunk_size]
	cmp	[fileread.offset],edx
	jbe	@f	
	
	xor	edx,edx
	mov	[fileread.offset],edx ; zero current offset
;--------------------------------------
@@:
	inc	ecx
	jmp	.loop
;--------------------------------------
.end:	
	mov	eax,[ebp+RESULT_SLOT.chunk_size]
	xor	edx,edx
	mul	ecx
	shr	eax,10+4 ;div 1024 ; div 16
	shl	edx,18
	add	eax,edx
	mov	[ebp+RESULT_SLOT.read_speed],eax ; speed KB/s	
        DEBUGF 1,'FSPEED: read chunk size: %s iterations: %d speed: %d KB/s\n',\
		[ebp+RESULT_SLOT.text],ecx,eax
	mcall	68,13,[fileread.return]
	ret
;-----------------------------------------------------------------------------
write_chunk:
	test	[check1.flags],dword 10b
	jz	.exit

	mov	eax,[file_info+32] ; file size
	cmp	[ebp+RESULT_SLOT.chunk_size],eax
	jb	@f
;--------------------------------------
.exit:
	xor	eax,eax ; small file size for current chunk size
	mov	[ebp+RESULT_SLOT.write_speed],eax
	ret
;--------------------------------------
@@:
	mcall	68,12,[ebp+RESULT_SLOT.chunk_size]
	mov	[filewrite.data],eax
	xor	eax,eax
	mov	[filewrite.offset],eax ; zero current offset
	mcall	26,9 ; get start time
	add	eax,1600 ; 16 sec for iterations
	mov	esi,eax
	mov	ecx,1
	mov	eax,[ebp+RESULT_SLOT.chunk_size]
	mov	[filewrite.size],eax
;--------------------------------------
.loop:
	mcall	70,filewrite
	
	mcall	26,9 ; check current time
	cmp	esi,eax
	jbe	.end
; correct offset	
	mov	edx,[ebp+RESULT_SLOT.chunk_size]
	add	[filewrite.offset],edx ; current offset
; check offset and file size	
	mov	edx,[file_info+32] ; file size
	sub	edx,[ebp+RESULT_SLOT.chunk_size]
	cmp	[filewrite.offset],edx
	jbe	@f	
	
	xor	edx,edx
	mov	[filewrite.offset],edx ; zero current offset
;--------------------------------------
@@:
	inc	ecx
	jmp	.loop
;--------------------------------------
.end:	
	mov	eax,[ebp+RESULT_SLOT.chunk_size]
	xor	edx,edx
	mul	ecx
	shr	eax,10+4 ;div 1024 ; div 16
	shl	edx,18
	add	eax,edx
	mov	[ebp+RESULT_SLOT.write_speed],eax ; speed KB/s	
        DEBUGF 1,'FSPEED: write chunk size: %s iterations: %d speed: %d KB/s\n',\
		[ebp+RESULT_SLOT.text],ecx,eax
	mcall	68,13,[filewrite.data]
	ret
;-----------------------------------------------------------------------------
include 'idata.inc'
;-----------------------------------------------------------------------------
IM_END:
;-----------------------------------------------------------------------------
include 'udata.inc'
;-----------------------------------------------------------------------------
I_END:
;-----------------------------------------------------------------------------
