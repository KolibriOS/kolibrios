;**************************************************************
; Dinamic Button Macro for KolibriOS
; Copyright (c) 2009, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;     * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;     * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
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
macro dinamic_button_exit
{
popa         
ret 4
}
;*****************************************************************************
align 16
dinamic_button:
db_type				equ [edi]
db_size_x			equ [edi+4]
db_start_x			equ [edi+6]
db_size_y			equ [edi+8]
db_start_y			equ [edi+10]
db_mouse_pos		equ [edi+12]
db_mouse_keys		equ [edi+16]
db_mouse_keys_old	equ [edi+20]
db_active_raw		equ [edi+24]
db_passive_raw		equ [edi+28]
db_click_raw		equ [edi+32]
db_resolution_raw	equ [edi+36]
db_palette_raw		equ [edi+40]
db_offset_raw		equ [edi+44]
db_select			equ [edi+48]
db_click			equ [edi+52]
;*****************************************************************************
;*****************************************************************************
; draw event
;*****************************************************************************
;*****************************************************************************
.draw:
    pusha
	mov   edi,dword [esp+36]
    call  .draw_1
dinamic_button_exit

.draw_1:
	cmp    dword db_select,1
	je     .active_1
	cmp    dword db_select,2
	je     .click_2
	mov    ebx,db_passive_raw
	jmp    .draw_2 
.active_1:
	mov    ebx,db_active_raw
	jmp   .draw_2 
.click_2:
	mov    ebx,db_click_raw
@@:
.draw_2:
    mov   cx,db_size_x
	shl   ecx,16
	mov   cx,db_size_y
	
    mov   dx,db_start_x
	shl   edx,16
	mov   dx,db_start_y

	mov   esi,db_resolution_raw

	mov   ebp,db_offset_raw
	
	push  edi
	mov   edi,db_palette_raw
	mcall SF_PUT_IMAGE_EXT
	pop   edi
	ret
;*****************************************************************************
;*****************************************************************************
; mouse event
;*****************************************************************************
;*****************************************************************************
align 4
.mouse:
	pusha
	mov   edi,dword [esp+36]
     mcall SF_MOUSE_GET,SSF_BUTTON
	 mov   ebx,db_mouse_keys
	 mov   db_mouse_keys_old,ebx
	 
     mov   db_mouse_keys,eax  
	 
     mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
     mov   db_mouse_pos,eax	
	 
     test  eax,0x80000000
     jnz   .exit_menu
     test  eax,0x8000
     jnz   .exit_menu

     mov   ebx,eax
     shr   ebx,16   ; x position
     shl   eax,16
     shr   eax,16   ; y position
	 
     mov   cx,db_start_x
     cmp   bx,cx
     jb    .exit_menu
     
     add   cx,db_size_x
     cmp   bx,cx
     ja    .exit_menu

     mov   cx,db_start_y
     cmp   ax,cx
     jb    .exit_menu
     
     add   cx,db_size_y
     cmp   ax,cx
     ja    .exit_menu
	 
	 test   db_mouse_keys,dword 1b
	 jnz   @f
	 cmp   dword db_select,1
	 je    .exit_menu_1
	 mov   db_select,dword 1
	 call  .draw_1
	 jmp   .exit_menu_1
@@:
	 mov   eax,db_mouse_keys
     cmp   eax,db_mouse_keys_old
	 je    .exit_menu_1
	 
	 mov   db_select,dword 2
	 call  .draw_1
	 mcall SF_SLEEP, 25
	 mov   db_select,dword 1
	 call  .draw_1

	 mov    db_click,dword 1
	 jmp   .exit_menu_2
	 
.exit_menu:
	 cmp   dword db_select,0
	 je    .exit_menu_1
	 mov   db_select,dword 0
	 
	 call  .draw_1

.exit_menu_1:
;	mov    db_click,dword 0
.exit_menu_2:
dinamic_button_exit
