;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; // Alver 22.06.2008 // {
align 4
dtext_asciiz_esi:			   ; for skins title out
    push eax
    xor eax, eax
    inc eax
    jmp    dtext.1
; } \\ Alver \\

align 4
dtext:	; Text String Output (rw by Johnny_B[john@kolibrios.org])
	; ebx x & y
	; ecx style ( 0xX0000000 ) & color ( 0x00RRGGBB )
	; X = ABnnb:
	; nn = font
	; A = 0 <=> output esi characters; otherwise output ASCIIZ string
	; B = 1 <=> fill background with color eax
	; edx start of text
	; edi 1 force

; // Alver 22.06.2008 // {
       push eax
	xor	eax, eax
.1:
; } \\ Alver \\
	pushad
	call	[_display.disable_mouse]

	movsx	eax, bx 	; eax=y
	sar	ebx, 16 	; ebx=x
	xchg	eax, ebx	; eax=x, ebx=y
	cmp	esi, 255
	jb	.loop
	mov	esi, 255
.loop:
	test	ecx, ecx
	js	.test_asciiz
	dec	esi
	js	.end
	jmp	@f
.test_asciiz:
	cmp	byte [edx], 0
	jz	.end
; // Alver 22.06.2008 // {
	cmp byte [esp+28], 1	;  was the entry point = dtext.1 ?
	jne   @f
	dec	esi
	js	  .end
; } \\ Alver \\
@@:
	inc	edx
	pushad			; esp -= 64 !
	movzx	edx, byte [edx-1]
	test	ecx, 0x10000000
	jnz	.font2
	mov	esi, 9
	lea	ebp, [FONT_I+8*edx+edx]
.symloop1:
	mov	dl, byte [ebp]
	or	dl, 1 shl 6
.pixloop1:
	shr	dl, 1
	jz	.pixloop1end
	jnc	.nopix
	call	[putpixel]
	jmp	.pixloop1cont
.nopix:
	test	ecx, 0x40000000
	jz	.pixloop1cont
	push	ecx
	mov	ecx, [esp+4+20h+20h]	; original eax?
	call	[putpixel]
	pop	ecx
.pixloop1cont:
	inc	eax
	jmp	.pixloop1
.pixloop1end:
	sub	eax, 6
	inc	ebx
	inc	ebp
	dec	esi
	jnz	.symloop1
	popad
	add	eax, 6
	jmp	.loop
.font2:
	add	edx, edx
	lea	ebp, [FONT_II+4*edx+edx+1]
	push	9
	movzx	esi, byte [ebp-1]
.symloop2:
	mov	dl, byte [ebp]
	push	esi
.pixloop2:
	shr	dl, 1
	jnc	.nopix2
	call	[putpixel]
	jmp	.pixloop2cont
.nopix2:
	test	ecx, 0x40000000
	jz	.pixloop2cont
	push	ecx
	mov	ecx, [esp+12+20h+20h]
	call	[putpixel]
	pop	ecx
.pixloop2cont:
	inc	eax
	dec	esi
	jnz	.pixloop2
	pop	esi
	sub	eax, esi
	inc	ebx
	inc	ebp
	dec	dword [esp]
	jnz	.symloop2
	pop	eax
	add	dword [esp+28], esi
	popad
	jmp	.loop
.end:
	popad
	pop  eax		; << // Alver 22.06.2008 // <<
	ret

diff10 "old font  code size",dtext,$

;=====================================================
; NEW FONTS
;_____________________________________________________
;

align 8
sdsh_data:

diff16 "sdsh_data      : ",0,$

include 'fonts/sd_data.asm'

sdsh_code:

; eax   =       if (bit[31])
;                               then lower 31 bits points to the user-selected font
;                               else ax = number of preloaded system font
; ebx   =       x<<16 + y
; ecx   =       0x00RRGGBB
; edx   =       if (bits[31:8] == 0)
;                               then dl = single char to write
;                               else edx= pointer to asciiz-string

diff16 "draw_char      : ",0,$

draw_char:
; font test
; bl=y; bh=x
; dl=ascii
	movzx	eax, bh
	movzx	ebx, bl
	movzx	ecx, dl
	xor	edi, edi
	mov	ebp, edi


sdsh_draw_char:
; eax = left side X
; ebx = bottom Y
; edi = color 0x0RRGGBB
; ebp = font@
; ecx = ascii, zero-extended to 32bits
	push	ecx
	push	edx
	push	ebp
	shl	ebp, 4
	add	ebp, sdsh_data.info
;        call   ch_checkscreen
;        jb             .exit
;        call   ch_checkwin
;        jc             .exit
	mov	edx, [ebp + 4]		    ; chartable addr
	mov	cx, word[edx + ecx*2]	    ; tick info
	mov	edx, ecx
	and	ecx, 7			    ; cl = number of ticks
	jz	.blank
	shr	edx, 4			    ; offset in the chartable
	add	edx, sdsh_data.chars	    ; edx -> the char's ticklist
.next_tick:
	call	parse_tick
	add	edx, 2
	dec	cl
	jnz	.next_tick
.blank:
.exit:
	pop	ebp
	pop	edx
	pop	ecx
	ret


parse_tick:
diff16 "parse_tick     : ",0,$
; eax = left side X
; ebx = bottom Y
; edx ->tickinfo
; edi = color 0x0RRGGBB
; ebp = font's header
	pushad
	mov	dx, word[edx]
	movzx	ecx, dl
	test	dl, 0xE0	  
	jz	.lntick.short   ; ticks #0..31
	cmp	dl, 0xE0
	jae	.lntick.long    ; ticks #224..255
	cmp	dl, 0xD8
	jae	.cstick         ; ticks #216..223
.gptick:
	push	edx
	cmp	dl, 38
	jb	.sptick             ; special ticks #32-37
                                    ; general ticks #38..215 
	mov	cl, dh
	shr	cl, 3		    ; number of origin
	mov	edx, [ebp+8]	    ; orig. table
	mov	ch, byte[edx+ecx]   ; orig. coords
	pop	edx
	mov	cl, dh
	and	cl, 7		    ; three rotation bits
	movzx	edx, dl 	    ; dl = gptick#
	cmp	dl, 80
	jae	.gptick.2
	sub	dl, 38

.gptick.1:
	;  1-byte tickgroups .v1-v4:
	lea	esi, [sdsh_data.v1+edx]
	inc	dh                ; dh = 1 vertex
	sub	dl, 2			; .v1 : 2 ticks only (#38, 39)
	jae	.gptick.1v2v4
	mov	dl, dh
	mov	dh, ch
	jmp	.draw

.gptick.1v2v4:
	; 1-byte tickgroups .v2-v4 : 8, 16 and 16 ticks
	inc	dh               ; dh = 2 vertice
        add     dl, 8
	shr	dl, 4
	jmp	.gptick.done

.gptick.2:
	; 2-byte tickgroups .v5-v8: 8 ticks each
	cmp	dl, 112
	jae	.gptick.3
	sub	dl, 80
	lea	esi, [sdsh_data.v5 + edx*2]
	mov	dh, 5
	shr	dl, 3
	jmp	.gptick.done

.gptick.3:
	; 3-byte tickgroups .v9-12: 4 ticks each
	cmp	dl, 128
	jae	.gptick.4
	sub	dl, 112
	lea	esi, [sdsh_data.v9 + edx*2 + edx]
	mov	dh, 9
	shr	dl, 2
	jmp	.gptick.done

.gptick.4:
	; 4-byte tickgroups .v13-16: 4 ticks each
	cmp	dl, 128                 ; fix this when needed
	jae	.exit                   ; not implemented yet!
	sub	dl, 128
	lea	esi, [sdsh_data.v13 + edx*4]
	mov	dh, 13
	shr	dl, 2

.gptick.done:
	add	dl, dh
	mov	dh, ch			; dh = orig.XY;     dl = numvert
	jmp	.draw			; cl = rotation;   esi-> tickfield

.cstick:
	and	cl, 4
	shr	cl, 2			; only one rotational bit
	and	dl, 3
	jz	.cstick.0
	dec	dl
	jnz	@f
	mov	dl, 6
	jz	.cstick.1		; 8-pix square outline
@@:	dec	dl
	jz	.cstick.2
.cstick.3:
	mov	esi, sdsh_data.cs3	; 12pix-ring
	mov	dl, 10
	jmp	.draw
.cstick.2:
	mov	dl, 7			; 9-pix filled square
.cstick.1:
	mov	esi, sdsh_data.cs2	; the square tickfield
	jmp	.draw
.cstick.0:
	mov	esi, sdsh_data.cs0	; 4pix-square
	mov	dl, 2
	jmp	.draw

.ritick:
	test	dl, 1
	jnz	.ritick.1
.ritick.0:
	movzx	ecx, dh 	    ; y
	and	cl, 0x0F
	sub	ebx, ecx
	mov	cl, dh
	shr	cl, 4		    ; x
	add	eax, ecx
	call	ch_putpixel
	jmp	.exit
.ritick.1:
	mov	esi, sdsh_data.ri1  ; 8pix-ring
	mov	dl, 6
	jmp	.draw

.lntick.short:
	test	dl, 0x06	    ; ticks #0, 1 are reserved for
	jz	.ritick 	    ;             rotational invariants
	and	dl, 0x07	    ; dl = line length
	jmp	.lntick.both
.lntick.long:
	and	dl, 0x07
	add	dl, 8
.lntick.both:
	sub	dl, 2		    ; num_vertice = num_points - 2
	and	cl, 0x18	    ; two (lower) rotation bits
	shr	cl, 3
	mov	esi, sdsh_data.blank

.draw:
; cl = rot; dl = numvert; dh = orig.xy
	push	ecx
	movzx	ecx, dh 	    ; y
	and	cl, 0x0F
	sub	ebx, ecx
	mov	cl, dh
	shr	cl, 4		    ; x
	add	eax, ecx
	pop	ecx
	call	draw_tick
.gptick.5:
.sptick:
.exit:
	popad
	ret


diff16 "draw_tick      : ",0,$

draw_tick:
; eax = x-origin
; ebx = y-origin
; edi = 0x0RRGGBB
;  cl = direction (0..7)
;  dl = number of vertice  (dl)
; esi -> tick bitfield

	pushad
	call	ch_putpixel			; the point of origin
	and	ecx, 7
	lea	ebp, [.move000+ ecx*4]
	call	ebp		      ; basic vector
	call	ch_putpixel
	and	edx, 15
	jz	.done		       ; no vertex (2pix line)
	mov	esi, dword [esi]       ; 15 vertice will easily fit to 1dw
	xchg	esi, edx
.move_and_draw:
	mov	ch, dl
	and	ch, 3
	jz	.moved
	dec	ch
	jz	.1
	dec	ch
	jz	.2
.3:
	dec	cl
.1:
	dec	cl
	jmp	.wipe_it
.2:
	inc	cl
.wipe_it:
	and	ecx, 7
.moved:
diff16 "moved          : ",0,$
	lea	ebp, [.move000+ ecx*4]
	call	ebp		    ; go new way
	call	ch_putpixel
	shr	edx, 2
	dec	esi
	jnz	.move_and_draw
diff16 "done           : ",0,$
.done:
	popad
	ret

; WARNING! The order matters! ------
align 4
diff16 "move000        : ",0,$
.move000:
	inc		eax
	ret
align 4
.move001:
	inc		eax
	dec		ebx
	ret
align 4
.move010:
	dec		ebx
	ret
align 4
.move011:
	dec		eax
	dec		ebx
	ret
align 4
.move100:
	dec		eax
	ret
align 4
.move101:
	dec		eax
	inc		ebx
	ret
align 4
.move110:
	inc		ebx
	ret

align 4
diff10 "move111-move000 (must be 28!)  : ",.move000,$
.move111:
	inc		eax
	inc		ebx
	ret

;) ---------------

align 4
diff16 "checkscreen    : ",0,$
ch_checkscreen:
     cmp	[Screen_Max_X], eax
     jb 	.getout
     cmp	[Screen_Max_Y], ebx
.getout:
     ret

align 4
diff16 "checkwin       : ",0,$
ch_checkwin:
; eax = x coordinate
; ebx = y coordinate
; ebp -> font info
;!destroys ch, edx!
	push	eax
	push	ebx
	mov	ch,  byte[CURRENT_TASK]
	mov	al,  byte [ebp] 	    ; char X-width
	mov	edx, [_display.width]	    ; screen X-size
	imul	edx, ebx
	add	edx, [_WinMapAddress]
	add	edx, eax
	cmp	ch,  byte [edx]
	jne	.fail
	movzx	eax, byte [ebp]
	cmp	ch,  byte [edx+eax]
	jne	.fail
	movzx	ebx, byte [ebp+1]
	imul	ebx, [_display.width]
	cmp	ch,  byte [edx+ebx]
	jne	.fail
	add	edx, eax
	cmp	ch,  byte [edx+ebx]
	clc
	je     .done
.fail:
	stc				    ; CF means the charbox is invisible
.done:
	pop	ebx
	pop	eax
	ret

align 4
diff16 "ch_putpix      : ",0,$
; ch_putpixel:
; eax = x coordinate
; ebx = y coordinate
; edi = 0x0RRGGBB
     push   edx
	 mov	edx, ebx
     imul	edx, [BytesPerScanLine]
     lea	edx, [edx+eax*4]
     mov   [LFB_BASE+edx], edi
     pop    edx
     ret
;)
ch_putpixel:
; test version!!! ===== to be removed!!
; eax = x coordinate
; ebx = y coordinate
; edi = 0x0RRGGBB
     push   edx
	 mov	edx, ebx
	shl	edx, 1
     imul	edx, [BytesPerScanLine]
     lea	edx, [edx+eax*8]
	add	edx, LFB_BASE
     mov   [edx], edi
	mov	[edx+4], edi
	sub	edx, [BytesPerScanLine]
	mov	[edx], edi
	mov	[edx+4], edi
     pop    edx
     ret


diff10 "new font  code size",sdsh_code,$
