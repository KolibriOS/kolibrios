.;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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

;=====================================================
; NEW FONTS
;_____________________________________________________
;

align 8
sdsh_data:

include 'sdsh.inc'

sdsh_code:

; eax	=	if (bit[31]) 
;				then lower 31 bits points to the user-selected font
;				else ax = number of preloaded system font 
; ebx	=	x<<16 + y
; ecx	=	0x00RRGGBB
; edx	=	if (bits[31:8] == 0)
;				then dl = single char to write 
;				else edx= pointer to asciiz-string


sdsh_draw_char:
; eax = left side X
; ebx = bottom Y
; edi = color 0x0RRGGBB
; ebp = font@ 
; ecx = ascii, zero-extended to 32bits
        push    ecx, edx, ebp
        shl     ebp, 4
        add     ebp, sdsh_data.info
        call	ch_checkscreen
        jb		.exit
        call	ch_checkwin
        jc		.exit
        xor		ch, ch
        mov     edx, [ebp + 4]              ; chartable addr
        mov		cx, word[edx + ecx*2]       ; tick info
        mov		edx, ecx
        and     ecx, 7                      ; cl = number of ticks
        jz      .blank
        shr     edx, 4                      ; offset in the chartable
        add     edx, [sdsh_data.chars]      ; edx -> the char's ticklist 
.next_tick:
        call    parse_tick
        add     edx, 4
        dec     cl
        jnz     .next_tick
.blank:
.exit:
        pop     ebp, edx, ecx
        ret


parse_tick:
; eax = left side X
; ebx = bottom Y
; edx ->tickinfo
; edi = color 0x0RRGGBB
; ebp = font's header 
        pushad    
        xor     ecx, ecx
        mov     dx, word[edx]
        mov     cl, dl
        test    dl, 0xE0            ; ticks #32..255
        jz      .lntick.short
        cmp     dl, 0xE0
        jae     .lntick.long
        cmp     dl, 0xC0
        jae     .cstick
.gptick:
        push    edx
        mov     cl, dh
        shr     cl, 3               ; orig.#
        mov     edx, [ebp+8]        ; orig. table
        mov     ch, byte[edx+ecx]   ; orig. coords
        pop     edx
        mov     cl, dh
        and     cl, 7               ; three rotation bits
        and     edx, 0xFF
        sub     dl, 32
        test    dl, (sdsh_data.v5-sdsh_data.v1)
        jae     .gptick.2
.gptick.1:
        mov     esi, sdsh_data.tick_table
        add     esi, edx            ; the tickfield
        inc     dh
        sub     dl, 2               ; .v1
        jb      .gptick.done
        inc     dh
        sub     dl, 8               ; .v2
        jb      .gptick.done
        inc     dh
        sub     dl, 8               ; .v3
        jb      .gptick.done
        inc     dh
        jmp     .gptick.done        ; .v4
.gptick.2:
        sub     dl, (sdsh_data.v5-sdsh_data.v1)
        jae     .gptick.3
        mov     esi, sdsh_data.v5
        add     esi, edx*2          ; 2-byte tickfield
        mov     dh, 5
        sub     dl, 8               ; .v5
        jb      .gptick.done
        inc     dh
        sub     dl, 8               ; .v6
        jb      .gptick.done
        inc     dh
        sub     dl, 8               ; .v7
        jb      .gptick.done
        inc     dh
        jmp     .gptick.done        ; .v8
.gptick.3:
        sub     dl, (sdsh_data.v9-sdsh_data.v5)
        jae     .gptick.4
        mov     esi, sdsh_data.v9
        lea     esi,[esi+edx*2+edx] ; 3-byte tickfield
        mov     dh, 9
        sub     dl, 4               ; .v9
        jb      .gptick.done
        inc     dh
        sub     dl, 4               ; .v10
        jb      .gptick.done
        inc     dh
        sub     dl, 4               ; .v11
        jb      .gptick.done
        inc     dh
        jmp     .gptick.done        ; .v12
.gptick.4:
        sub     dl, (sdsh_data.v13-sdsh_data.v9)
        jae     .gptick.5
        mov     esi, sdsh_data.v13
        lea     esi,[esi+edx*2+edx] ; 3-byte tickfield
        mov     dh, 13
        sub     dl, 4               ; .v13
        jb      .gptick.done
        inc     dh
        sub     dl, 4               ; .v14
        jb      .gptick.done
        inc     dh
        sub     dl, 4               ; .v15
        jb      .gptick.done
        inc     dh
        jmp     .gptick.done        ; .v16
.gptick.unknown:
        jmp     .exit
.gptick.done:
        mov     dl, dh              ; dl = numvert
        mov     dh, ch              ; dh = orig.xy
        call    draw_tick
        jmp     .exit
        
.cstick:
        and     cl, 4
        shr     cl, 2               ; only one rotational bit
        and     dl, 3
        dec     dl
        jb      .cstick.0
        mov     esi, sdsh_data.cs2  ; the square
        dec     dl
        jb      .cstick.1
        dec     dl
        jb      .cstick.2
.cstick.3:
        mov     esi, sdsh_data.cs3  ; 12pix-ring
        mov     dl, 10
        jmp     .draw
.cstick.2:
        mov     dl, 7
        jmp     .draw
.cstick.1:
        mov     dl, 6
        jmp     .draw
.cstick.3:
        mov     esi, sdsh_data.cs0  ; 4pix-square
        mov     dl, 3
        jmp     .draw

.ritick:
        test    dl, 1
        jnz     .ritick.1
.ritick.0:
        mov     cl, dh              ; y
        and     cl, 0x0F
        sub     ebx, ecx
        mov     cl, dh
        shr     cl, 4               ; x
        add     eax, ecx
        call    ch_putpixel
        jmp     .exit
.ritick.1:
        mov     esi, sdsh_data.ri1  ; 8pix-ring
        mov     dl, 4 
        xor     cl, cl
        jmp     .draw 
        
.lntick.short:
        test    dl, 0x06            ; ticks #0, 1 are reserved for 
        jz      .ritick             ;             rotational invariants     
        and     dl, 0x07            ; dl = line length
        jmp     .lntick.both
.lntick.long:
        and     dl, 0x07
        add     dl, 8
.lntick.both:
        sub     dl, 2               ; num_vertice = num_points - 2
        and     cl, 0x18            ; two rotation bits
        shr     cl, 3
        mov     esi, sdsh_data.blank 
.draw:
; cl = rot; dl = numvert; dh = orig.xy 
        push    ecx
        mov     cl, dh              ; y
        and     cl, 0x0F
        sub     ebx, ecx
        mov     cl, dh
        shr     cl, 4               ; x
        add     eax, ecx
        pop     ecx
        call    draw_tick
.gptick.5:
.exit:
        popad
        ret

                
	
draw_tick:
; eax = x-origin
; ebx = y-origin
; edi = 0x0RRGGBB 
;  cl = direction (0..7)  
;  dl = number of vertices  (dl)
; esi -> tick bitfield  

	pushad
    inc     dl                      ; -- that's to count down to 0 
	call	ch_putpixel		        ; the point of origin
	and		ecx, 7
	call	[.moves + ecx*4]        ; basic vector
	call	ch_putpixel		
    dec     dl
    jz      .done                   ; 2pix lines only
    mov     esi, dword [esi]        ; max 16 vertice bitfield limit
    xchg    esi, edx 
.move_and_draw:
    mov     ch, dl
    and     ch, 3
    jz      .moved
    btc     ch, 0
    jz      .2
.1:
    dec     cl                      ; right turns (1 and 3)
    and     cl, 7
@@: btc     ch, 1                   ; straight angle (3) ?
    jc      .1
    jmp     .moved
.2:
    inc     cl                      ; left turn (2)
    and     cl, 7
    btc     ch, 1                   ; just to zero ah
.moved:
	call	[.moves + ecx*4]        ; go new way
	call	ch_putpixel	
    shr     edx, 2	
    dec     esi
    jnz     .move_and_draw	         
.done:
	popad
	ret

.move000:
	inc		eax
	ret
.move001:
	inc		eax
	dec		ebx
	ret
.move010:
	dec		ebx
	ret
.move011:
	dec		eax
	dec		ebx
	ret
.move100:
	dec		eax
	ret
.move101:
	dec		eax
	inc		ebx
	ret
.move110:
	inc		ebx
	ret
.move111:
	inc		eax
	inc		ebx
	ret

align 4
.moves dd .move000, .move001, .move010, .move011, .move100, .move101, .move110, .move111

;)

align 4
ch_checkscreen:
     cmp	[Screen_Max_X], eax
     jb		.getout
     cmp	[Screen_Max_Y], ebx
.getout:
     ret

align 4
ch_checkwin:
; eax = x coordinate
; ebx = y coordinate
; ebp -> font info
;!destroys ch, edx! 
        push    eax, ebx
        mov     ch,  byte[CURRENT_TASK]
        mov     al,  byte [ebp]	            ; char X-width
        mov     edx, [_display.width]       ; screen X-size
        imul    edx, ebx
        add     edx, [_WinMapAddress]
        add     edx, eax
        cmp     ch,  byte [edx]
        jne     .fail
        moxzx   eax, byte [ebp]
        cmp     ch,  byte [edx+eax]
        jne     .fail
        moxzx   ebx, byte [ebp+1]
        imul    ebx, [_display.width]
        cmp     ch,  byte [edx+ebx]
        jne     .fail
        add     edx, eax
        cmp     ch,  byte [edx+ebx]
        clc
        je     .done
.fail:  
        stc                                 ; CF means the charbox is invisible
.done:
        pop     ebx, eax
        ret

align 4
ch_putpixel:
; eax = x coordinate
; ebx = y coordinate
; edi = 0x0RRGGBB    
     push   edx
	 mov	edx, ebx
     imul	edx, [BytesPerScanLine]
     lea	edx, [edx+eax*4]
     mov   [LFB_BASE+edx], edi
     pop   edx
     ret
;)

;diff16 "font  code end ",0,$
diff10 "font  code size",dtext,$
