;**************************************************************
; 2016, 0CodErr
;       Added border styles(raised, sunken, etched, ridged).
;       Added possibility to fill frame background.
;**************************************************************
; Frame Macro for Kolibri OS
; Copyright (c) 2013, Marat Zakiyanov aka Mario79, aka Mario
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
macro frame_start
{
	pusha
}
;*****************************************************************************
macro frame_exit
{
popa         
ret 4
}
;*****************************************************************************
fr equ [esp + 36]
frame: 
fr_type              equ [eax + FR_STYLE]         ; dword 
fr_size_x            equ [eax + FR_WIDTH]         ;  word 
fr_start_x           equ [eax + FR_LEFT]          ;  word 
fr_size_y            equ [eax + FR_HEIGHT]        ;  word 
fr_start_y           equ [eax + FR_TOP]           ;  word 
fr_ext_fr_col        equ [eax + FR_OUTER_COLOR]   ; dword 
fr_int_fr_col        equ [eax + FR_INNER_COLOR]   ; dword 
fr_flags             equ [eax + FR_FLAGS]         ; dword 
fr_text_pointer      equ [eax + FR_TEXT]          ; dword 
fr_text_position     equ [eax + FR_TEXT_POSITION] ; dword 
fr_font_number       equ [eax + FR_FONT]          ; dword
fr_font_size_y       equ [eax + FR_FONT_HEIGHT]   ; dword
fr_font_color        equ [eax + FR_FORE_COLOR]    ; dword
fr_font_backgr_color equ [eax + FR_BACK_COLOR]    ; dword
;*****************************************************************************
;*****************************************************************************
; draw event
;*****************************************************************************
;*****************************************************************************
align 16
.draw:
frame_start
        mov    eax, fr
        mov    edx, fr_ext_fr_col
        mov    edi, fr_int_fr_col
        mov    esi, edx
        mov    ebp, edi
        mov    eax, fr_flags
        and    eax, 1110b
.raised:        
        cmp    eax, FR_RAISED     
        je     .border_style_selected
.sunken:        
        cmp    eax, FR_SUNKEN
        jne    .etched
        xchg   edx, edi
        xchg   esi, ebp
        jmp    .border_style_selected
.etched:        
        cmp    eax, FR_ETCHED
        jne    .ridged
        xchg   edx, edi
        jmp    .border_style_selected
.ridged:        
        cmp    eax, FR_RIDGED
        jne    .double
        xchg   esi, ebp
        jmp    .border_style_selected
.double:    
        cmp    eax, FR_DOUBLE
        jne    .border_style_selected
        mov    edi, edx
        mov    esi, ebp
.border_style_selected:                
; Outer Top Line
        mov    eax, fr
        mov    bx, fr_start_x
        mov    cx, fr_start_y
        shl    ebx, 16
        shl    ecx, 16
        mov    bx, fr_size_x
        add    bx, fr_start_x
        sub    ebx, 1
        mov    cx, fr_start_y
        mov    eax, 38
        int    64
; Outer Left Line
        mov    eax, fr
        mov    bx, fr_start_x
        add    cx, fr_size_y        
        sub    ecx, 1
        mov    eax, 38
        int    64                
; Inner Top Line
        mov    eax, fr
        mov    bx, fr_start_x
        mov    cx, fr_start_y
        add    ebx, 1
        add    ecx, 1
        shl    ebx, 16
        shl    ecx, 16
        mov    bx, fr_size_x
        mov    cx, fr_start_y
        add    bx, fr_start_x        
        sub    ebx, 2
        add    ecx, 1
        mov    edx, esi
        mov    eax, 38
        int    64
; Inner Left Line
        mov    eax, fr
        mov    bx, fr_start_x
        add    cx, fr_size_y        
        add    ebx, 1
        sub    ecx, 3
        mov    edx, esi
        mov    eax, 38
        int    64    
; Outer Bottom Line    
        mov    eax, fr    
        mov    bx, fr_size_x
        mov    cx, fr_size_y        
        add    bx, fr_start_x
        add    cx, fr_start_y   
        sub    ebx, 1 
        sub    ecx, 1
        shl    ebx, 16
        shl    ecx, 16
        mov    bx, fr_start_x
        mov    cx, fr_size_y
        add    cx, fr_start_y
        sub    ecx, 1
        mov    edx, edi
        mov    eax, 38
        int    64
; Outer Right Line
        mov    eax, fr
        add    bx, fr_size_x        
        sub    ebx, 1
        mov    cx, fr_start_y
        mov    edx, edi
        mov    eax, 38
        int    64           
; Inner Bottom Line
        mov    eax, fr
        mov    bx, fr_size_x
        mov    cx, fr_size_y        
        add    bx, fr_start_x
        add    cx, fr_start_y   
        sub    ebx, 2
        sub    ecx, 2
        shl    ebx, 16
        shl    ecx, 16
        mov    bx, fr_start_x
        mov    cx, fr_size_y
        add    cx, fr_start_y
        add    ebx, 1
        sub    ecx, 2
        mov    edx, ebp
        mov    eax, 38
        int    64
; Inner Right Line
        mov    eax, fr
        mov    cx, fr_start_y        
        add    bx, fr_size_x
        sub    ebx, 3
        add    ecx, 1
        mov    edx, ebp
        mov    eax, 38
        int    64        
;----------------------------------------------------------------------
        mov    eax, fr        
        test   dword fr_flags, FR_FILLED
        je     .fill_exit
        
        mov    bx, fr_start_x 
        mov    cx, fr_start_y
        add    ebx, 2
        add    ecx, 2
        shl    ebx, 16
        shl    ecx, 16
        mov    bx, fr_size_x
        mov    cx, fr_size_y
        sub    ebx, 4
        sub    ecx, 4
        mov    edx, fr_font_backgr_color
        mov    eax, 13
        int    64               
.fill_exit:  
;----------------------------------------------------------------------
        mov    eax, fr
        test    dword fr_flags, FR_CAPTION
	je	.exit

	mov	ecx,0xC0000000
	mov	eax,fr_font_number
	and	eax,11b
	shl	eax,28
	add	ecx,eax
        mov    eax, fr
	mov	eax,fr_font_color
	and	eax,0xffffff
	add	ecx,eax

	mov    eax, fr

	mov	eax,fr_font_backgr_color
	and	eax,0xffffff

	xor	esi,esi

        mov    eax, fr
	mov	bx,fr_start_x
	add	bx,10
	shl	ebx,16
	mov	bx,fr_font_size_y
	shr	bx,1

	not	bx
	add	bx,fr_start_y

	test	fr_font_size_y,word 1b
	jz	@f
	
	inc	bx
;--------------------------------------
align 4
@@:
	cmp	fr_text_position,dword 0
	je	.draw_1
	
	add	bx,fr_size_y
;--------------------------------------
align 4
.draw_1:
	mov     edx,fr_text_pointer
	mov     edi,fr_font_backgr_color
	mcall	SF_DRAW_TEXT
;----------------------------------------------------------------------
align 4
.exit:
frame_exit
;*****************************************************************************
