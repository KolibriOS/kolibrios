;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  GRAPH32.INC                                                 ;;
;;                                                              ;;
;;            32bpp graph engine for Kolibri-A                  ;;
;;                                                              ;;
;;  art_zh (kolibri@jerdev.co.uk) Dec. 2010 :                   ;;
;;      - 4x2 granularity & tiled winmap structure              ;;
;;      - speed-optimized line/box graphics                     ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 1708 $


;*************************************************
; getpixel
;
; in:
; eax = x coordinate
; ebx = y coordinate
;
; ret:
; ecx = 00 RR GG BB


get_pixel:
     mov     ecx, [BytesPerScanLine]
     imul    ecx, ebx
     lea     ecx, [ecx+eax*4]		; ecx = x*4+(y*y multiplier)
     mov     ecx, [ecx+LFB_BASE]
     and     ecx, 0xffffff
     ret

;-----------------------------------------------------------------------------------
; esi : Buffer origin
; edi : Screen origin
; ebp : Map origin
; ecx : block height (pix)
; ebx : bit[24] = odd line; bh = temp; bl = current task

align 4
draw_aligned_box:
        pushad
        xor     edx, edx
.new_line:
        btr     ebx, 26
        mov     eax, [img_map_x]
        xor     ecx, ecx
        cmp	bl, byte[ebp]		        ; check the left tile first
        je      .new_tile
        bts     ebx, 26                         ; ebx[26] = 1 if edi/esi pushed
        push    edi
        push    esi
        push    [img_bitoffset]
        jmp     .seek_visible
.new_tile:
        inc     ecx			        ; visible - scan the open space
        cmp     ecx, eax      
        jz      .end_of_line
        cmp     bl, byte[ebp+ecx]  
        je      .new_tile
                                                ; overlapped? draw the last visible segment if so
        bts     ebx, 26                         ; check if edi/esi already pushed
        jc      @f
        push    edi
        push    esi
        push    [img_bitoffset]
@@:     call    [img_draw_core_fn]	        ; bpp-specific helper (see below)

.seek_visible:
        inc     ecx
        cmp     ecx, eax	
        je      .next_line
        cmp     bl, byte[ebp+ecx]
        jne     .seek_visible
.got_visible:
        sub     eax, ecx
	shl     ecx, 4
        add	edi, ecx                        ; shift the left edge
        bt      ebx, 25                         ; 1bpp?
        jc      @f 
        shr     ecx, 2
        imul    ecx, [img_bytes_per_pix] 
        jmp     .new_visible      
@@:     shr     ecx, 8                          ; 2 tiles = 1 byte 
        jnc     .new_visible
        rol     [img_bitoffset], 4
        jnc     .new_visible
        inc     ecx
.new_visible:
        add     esi, ecx
        xor     ecx, ecx
        jmp     .new_tile

.end_of_line:
        call    [img_draw_core_fn]

.next_line:  
        bt      ebx, 26
        jnc     @f
        pop     [img_bitoffset]
        pop     esi
        pop     edi                                   
@@:	inc	edx
	cmp	edx, [esp+24]                   ; stacked ecx = image height 
	je	.finish
	add	edi, [BytesPerScanLine]
        add     esi, [img_buf_line_size]
	btc	ebx, 24			        ; odd line?
	jnc	.new_line
        add     ebp, [_WinMapWidth]
	jmp     .new_line

.finish:
        popad
        ret

;--------------------------------
; ebx : bit[24] = odd line; bh = reserved; bl = current task
; ecx : column height (pix)
; edx : max tile offset: 0, 4, 8, or 12 bytes (1,2,3 or 4 pix to draw)
; ebp : map origin
; esi : buffer image origin 
; edi : LFB-origin (4byte-aligned)

align 4
draw_unaligned_edge:
        pushad
        mov     eax, [img_buf_line_size]
        mov     bh, dl                  ; store the 1st tile offset
        btr     ebx, 24                 ; check if the 1st line odd
        jnc     .new_tile           
        cmp     bl, byte[ebp]
        jne     @f
        call    [img_draw_edge_fn]      ; bpp-specific helper (see below)
@@:     
        dec     ecx
        jz      .exit
        add     edi, [BytesPerScanLine]
        add     ebp, [_WinMapWidth]
        add     esi, eax
.new_tile:
        cmp     bl, byte[ebp]
        jne     .skip_tile
        call    [img_draw_edge_fn]      
        dec     ecx
        jz      .exit
        add     edi, [BytesPerScanLine]
        add     esi, eax
        call    [img_draw_edge_fn]      
        dec     ecx
        jz      .exit
        add     edi, [BytesPerScanLine]
        add     ebp, [_WinMapWidth]
        add     esi, eax
        jmp     .new_tile
.skip_tile:
        sub     cx, 2
        jbe     .exit
        add     edi, [BytesPerScanLine]
        add     edi, [BytesPerScanLine]
        add     esi, eax
        add     esi, eax
        add     ebp, [_WinMapWidth]
        jmp     .new_tile
.exit:
        popad
        ret               

              

;-------------
; unaligned edge helpers
; esi -> left point of the image edge 
; edi -> left point of the screen edge
; bh = edx = tile offset (0, 4, 8 or 12 bytes)

align 4
draw_edge_0bpp:
        push    eax 
        mov     eax, [esi]
.putpix:
        mov     [edi+edx], eax
        sub     dl,  4
        jae     .putpix
.exit:
	movzx	edx, bh
        pop     eax
        ret
align 4
draw_edge_32bpp:
        push    eax 
.putpix:
        mov     eax, [esi+edx]
        mov     [edi+edx], eax
        sub     dl,  4
        jae     .putpix
.exit:
	movzx	edx, bh
        pop     eax
        ret
align 4
draw_edge_24bpp:
        push    eax esi
	xor	dl, dl
.putpix:
        mov     eax, [esi]
        and     eax, 0x00FFFFFF
        mov     [edi+edx], eax
        cmp     dl, bh
        je      .exit
        add     dl,  4
        add     esi, 3
        jmp     .putpix
.exit:
        pop     esi eax
        ret
align 4
draw_edge_8bpp:
        push    eax esi ebp
	xor	dl, dl
        mov     ebp, [img_palette]
.putpix:
        movzx   eax, byte[esi]
        mov     eax, [ebp+eax*4]
        mov     [edi+edx], eax
        cmp     dl, bh
        je      .exit
        add     dl,  4
        inc     esi
        jmp     .putpix
.exit:
        pop     ebp esi eax
        ret
align 4
draw_edge_1bpp:
        pushad
        movzx   edx, bh
        add     edx, edi
        mov     ebp, [img_palette]
        mov     ebx, [ebp+4]            ; forecolor
        mov     ebp, [ebp]              ; backcolor
        mov     ecx, [img_edgeoffset]    ; cl = 1 << left_edge_pix_num
        mov     eax, [esi]     
.testbit:
        test    eax, ecx
        jnz     @f
        mov     eax, ebp 
        jmp     .putpix
@@:     mov     eax, ebx
.putpix:
        mov     [edi], eax
        cmp     edi, edx
        je      .exit
        add     edi, 4
        rol     ecx, 1
        jmp     .testbit
.exit:
        popad
        ret

draw_edge_16bpp:
draw_core_16bpp:
        ret

;-------------
; aligned core helpers
; esi -> left point address (buffer) 
; edi -> left point address (screen)
; ecx =  number of tiles to draw
align 4
draw_core_0bpp:
        push    eax ecx edi 
        pushfd
;        cli
        cld
        mov     eax, [esi]
        shl     ecx, 2
        rep     stosd
        popfd
        pop     edi ecx eax
        ret
align 4
draw_core_32bpp:
        push    ecx esi edi 
        pushfd
;        cli
        cld
        shl     ecx, 2
        rep     movsd
        popfd
        pop     edi esi ecx 
        ret
align 4
draw_core_24bpp:
        push    eax ecx edx
        shl     ecx, 2                  ; ecx = numpixels
        dec     ecx
        lea     edx, [ecx*2+ecx]        ; edx = buffer byte offset
.putpix:
        mov     eax, [esi+edx]
        and     eax, 0x00FFFFFF
        mov     [edi+ecx*4], eax
        dec     ecx
        sub     edx, 3
        jnb     .putpix
        pop     edx ecx eax
        ret
align 4
draw_core_8bpp:
        pushad 
        mov     ebp, [img_palette]
.putpix:
        xor     edx, edx
        mov     eax, dword[esi]         ; block of 4 pixels
.putone:
        movzx   ebx, al
        mov     ebx, [ebp+ebx*4]
        mov     [edi+edx*4], ebx
        shr     eax, 8
        inc     dl
        cmp     dl, 4
        jnz     .putone
        add     esi, edx        ;-)
        add     edi, 16
        dec     ecx
        jnz     .putpix
.exit:
        popad
        ret
align 4
draw_core_1bpp:
        pushad   
        mov     ebp, [img_palette]      
        mov     edx, [ebp+4]            ; foreground color
        mov     ebp, [ebp]              ; background color
        mov     ebx, [img_bitoffset]
        shl     ecx, 2                  ; 1 tyle = 4 pix
.newblock:
        mov     eax, [esi]
.putpix:
        test    ebx, eax
        jz      .bkcolor
        mov     [edi], edx
        jmp     .nextpix
.bkcolor:
        mov     [edi], ebp
.nextpix:
        dec     ecx
        jz      .exit  
        rol     ebx, 1
        jc      .nextblock
        add     edi, 4
        jmp     .putpix
.nextblock:
        add     esi, 4
        jmp     .newblock
.exit:
        popad     
        ret

;-----------------------------------------
virtual at esp
 putimg:
   .image_sx	   dd ?		; X-size (pix)
   .image_sy	   dd ?		; Y-size 
   .stack_data = 2*4
end virtual

align 4
; ebx -> Buffer origin
; ecx = packed size [x|y]
; edx = packed coordinates [x|y]
; static variables required:
; [img_draw_core_fn],  [img_draw_edge_fn]
; [img_bytes_per_pix], [img_buf_line_size]
; [img_palette]  (1bpp and 8bpp only)

_putimage:
;     	call    [_display.disable_mouse]
	pushad
     	sub     esp, putimg.stack_data
     	mov     [img_buf_origin], ebx		; save pointer to image buffer
     	mov     esi, ebx		        ; pointer to image
.unpack_coords:
     	mov     eax, ecx
    	and     ecx, 0xFFFF			; Ysize
     	shr     eax, 16	                        ; Xsize
     	mov     [putimg.image_sy], ecx
     	mov     [putimg.image_sx], eax
    	mov     eax, edx
     	and     edx, 0xFFFF			; Ytop
     	shr     eax, 16			        ; Xleft
.calculate_abs_coords:
        mov     edi, [TASK_BASE]
     	mov     ebx, [edi-twdw + WDATA.box.left]
     	mov     ecx, [edi-twdw + WDATA.box.top]
        add     ebx, eax
     	add     ecx, edx
     	mov     [img_screen_x], ebx		; abs Xleft
;     	mov     [img_screen_y], ecx		; ecx = abs Ytop        ; hold it !
.check_x_size:
     	mov     ebx, [edi-twdw + WDATA.box.width] 
     	inc     ebx				; ebx = window Xsize
     	sub     ebx, eax                        ; eax = rel Xleft
     	jbe     .finish				; image is out of the window
        mov     eax, [putimg.image_sx]
     	cmp     ebx, eax		        ; real_sx = MIN(wnd_sx-image_cx, image_sx);
     	jae     @f
        mov     eax, ebx
@@:    	dec     eax
        mov     [img_pix_x], eax
.check_y_size:
     	mov     ebx, [edi-twdw + WDATA.box.height] 
     	inc     ebx				; ebx = real window y-size
     	sub     ebx, edx                        ; edx = rel Ytop
     	jbe     .finish				; image isn't visible
        mov     edx, [putimg.image_sy]
        cmp     ebx, edx
        jae     @f
        mov     edx, ebx
@@:     mov     [img_pix_y], edx

.calculate_lfb_origin:
     	mov     edi, ecx                        ; ecx = absY
     	imul    edi, [BytesPerScanLine]
     	mov     eax, [img_screen_x]             ; eax = absX
     	lea     edi, [edi+eax*4]		
     	add     edi, LFB_BASE			; edi -> Screen origin
	mov	[img_lfb_origin], edi
.calculate_map_origin:
	xor	ebx, ebx
        mov     bl,  byte [img_bytes_per_pix]
        or      bl,  bl
        jnz     @f
        mov     ecx, [img_buf_line_size]
        or      cl, cl
        je      @f
        bts     ebx, 25
@@:    	mov     bl,  byte [CURRENT_TASK]	; get process number 
    	mov     ebp, ecx                        ; ecx = absY
	shr	ebp, 1				; CF= odd line
	jnc	@f
	bts	ebx, 24				; ebx[24] = odd start line
@@:    	imul    ebp, [_WinMapWidth]	
	add	ebp, [_WinMapAddress]
     	mov     ecx, eax                        ; eax = absX
	shr	ecx, 2                 
	add     eax, [img_pix_x]
        inc     eax
        shr     eax, 2
        add     eax, ebp	
;        mov     [img_map_right], eax		; right edge tile
     	add     ebp, ecx			; left edge Map origin
        mov	ecx, [img_pix_y]
        sub     eax, ebp
        jz      .thin_bar                       ; special case: all image is 1 tile  thick
        mov     [img_map_x], eax                ; tiles in row (excluding the right one) 

; ----- at this point:
; esi = [img_buf_origin] -> buffered image
; edi = [img_lfb_origin] -> LFB image (corner point, 0RGB format) 
; ebp -> corner tile position
; ecx = [img_pix_y] =  image height
;  bl = task #
; ebx[24] = 1 if Ytop is odd
; ebx[25] = 1 if 1bpp image

.start:
        bt      ebx, 25 
        jnc     @f
        xor     eax, eax
        inc     al
	mov	[img_bitoffset], eax		; 1bpp image must be byte-aligned
	mov	[img_edgeoffset], eax		
@@:
	mov	edx, edi
	mov	dh, 0x0C
        and     dl, dh       
        jz      .go_right                       ; left edge already aligned
.left_edge:
        sub     dh, dl
        movzx   edx, dh
	call	draw_unaligned_edge
        dec     [img_map_x]
	shr	edi, 4
        inc     edi                             ; align edi to the next 16-byte tile
	shl	edi, 4
	mov	[img_lfb_origin], edi           ; core Screen origin
        shr     edx, 2
        inc     edx
        sub     [img_pix_x], edx                ; shrink image width      
        bt      ebx, 25
        jnc     @f  
        xchg    dl, cl
        mov     eax, [img_edgeoffset]           ; that's for 1bpp images only
        shl     eax, cl
        mov     [img_edgeoffset], eax
        mov     [img_bitoffset],  eax
        xchg    dl, cl
@@:     mov     eax, edx
        imul    eax, [img_bytes_per_pix]        ; 0 for 1bbp bitmaps
        add     esi, eax
        mov     [img_buf_origin], esi           ; core Buffer origin
        inc     ebp                             ; core Map origin
.go_right:
        mov     eax, [img_map_x]
        mov     edx, eax
        bt      ebx, 25                         ; 1bpp image ?
        jc      .shift_mono
        shl     eax, 2
        imul    eax, [img_bytes_per_pix]
        jmp     .get_right
.shift_mono:
        shr     eax, 1                          ; 2 tiles = 1 byte Buffer offset
        jnc     .get_right
        rol     byte [img_edgeoffset], 4        ; odd number of tiles: shift 4bits
.get_right:
        add     esi, eax                        ; rightEdge Buffer origin
        push    ebp
        add     ebp, edx                        ; rightEdge Map origin 
        mov     eax, [img_pix_x]
        shl     eax, 2                          ; 1 pix = 4 bytes
        add     eax, edi                        ; rightEdge last pix (LFB addr)
        shl     edx, 4
        add     edi, edx                        ; rightEdge Screen origin
        movzx   edx, al 
        mov     eax, [img_map_x]
        and     dl, 0x0C
        cmp     dl, 0x0C
        je      .core_block             	; rightEdge is already tile-aligned
.right_edge:
        call    draw_unaligned_edge
.core_block:
        or      eax, eax                        ; empty central core?
        jz      .finish            
        mov     ebp, [esp]
    	mov	edi, [img_lfb_origin]
	mov	esi, [img_buf_origin]
	
	call	draw_aligned_box
	
.finish:
     	add     esp, (putimg.stack_data + 4)
;	call	[_display.enable_mouse]
	popad
	ret

.thin_bar:                                      ; < a special case > :  one-tile-wide image
        mov     edx, [img_pix_x]
        shl     edx, 2                          ; edx = rightmost tile offset (0, 4, 8, or 12 bytes)
        call    draw_unaligned_edge
     	add     esp, putimg.stack_data
	popad
	ret


;align 64
;img_test_struct_32:     ; 8 x 10
;        dd      0x112233, 0x223344, 0x334455, 0x445566, 0x556677, 0x667788, 0x778899, 0x887766
;        dd      0x223344, 0x334455, 0x445566, 0x556677, 0x667788, 0x777799, 0x887766, 0x997755
;        dd      0x334455, 0x445566, 0x556677, 0x667788, 0x777799, 0x887766, 0x997755, 0xAA7744
;        dd      0x445566, 0x556677, 0x667788, 0x777799, 0x887766, 0x997755, 0xAA7744, 0xBB7733
;        dd      0x334455, 0x445566, 0x556677, 0x667788, 0x777799, 0x887766, 0x997755, 0xAA7744
;        dd      0x223344, 0x334455, 0x445566, 0x556677, 0x667788, 0x777799, 0x887766, 0x997755
;        dd      0x112233, 0x223344, 0x334455, 0x445566, 0x556677, 0x667788, 0x777799, 0x887766
;        dd      0x001122, 0x112233, 0x223344, 0x334455, 0x445566, 0x556677, 0x667788, 0x777799
;        dd      0x220000, 0x001122, 0x112233, 0x223344, 0x334455, 0x445566, 0x556677, 0x667788
;        dd      0x441100, 0x220000, 0x001122, 0x112233, 0x223344, 0x334455, 0x445566, 0x556677

;align 64
;img_test_struct_24:     ; 8 x 16
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211
;        dw      0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211, 0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB
;        dw      0xBBAA, 0xAACC, 0xCCBB, 0xBBAA, 0xAACC, 0xCCBB, 0x1100, 0x0022, 0x2211, 0x1100, 0x0022, 0x2211

;align 64
;img_test_struct_8:     ; 20 x 10
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0
;        db      0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0

;align 64
;img_test_struct_1:     ; 16 x 10
;        db      0x0F, 0xF0
;        db      0x0F, 0xF0
;        db      0x3C, 0xC3
;        db      0x3C, 0xC3
;        db      0xF0, 0x0F
;        db      0xF0, 0x0F
;        db      0x3C, 0xC3
;        db      0x3C, 0xC3
;        db      0x0F, 0xF0
;        db      0x0F, 0xF0

;align 64
;img_test_palette:     ; 6 colors
;        dd      0x00BB2233, 0xAA4466, 0x995555, 0x00339966, 0x00884455, 0x00775566, 0x00664455, 0x00553344, 0x0

;**************************************************************************************
align 4
__sys_putpixel:
        push    edx
	mov	edx, [TASK_BASE]
	add	eax, [edx-twdw+WDATA.box.left]
	add	ebx, [edx-twdw+WDATA.box.top]
        pop     edx
_putpixel:

; eax = x coordinate
; ebx = y coordinate
; ecx = ?? RR GG BB    ; 0x01000000 negation
; edi = 0x00000001 force

     	cmp   [Screen_Max_X], eax
     	jb    .exit0
     	cmp   [Screen_Max_Y], ebx
     	jb    .exit0
.check_forced:
     	test    edi,1		 ; force ?
     	jnz     .checked

.not_forced:
     	push    ebx eax
	shr	eax, 2
     	shr  	ebx, 1
     	imul    ebx, [_WinMapWidth]	  ; win_map (X size)/2
	add	ebx, eax
	mov	al, byte [CURRENT_TASK]
     	mov   	ah, byte [_WinMapAddress+ebx]
     	cmp     ah, al
     	pop     eax ebx
     	jne     .exit0
.checked:
	push	ebx
     	imul    ebx, [BytesPerScanLine]
     	lea     ebx, [ebx+eax*4]
     	bt      ecx, 24
     	jnc     .noneg
     	mov     ecx, [LFB_BASE+ebx]
	xor     ecx, 0x00FFFFFF
.noneg:
     	mov     [LFB_BASE+ebx], ecx
	pop	ebx
.exit0:
     ret



;align 4
;_put_pixel:	; left for compatibility with Vesa20_putpixel32
;; eax = x
;; ebx = y
;     imul    ebx, [BytesPerScanLine]	 ; ebx = y * y multiplier
;     lea     edi, [ebx+eax*4]  ; edi = x*4+(y*y multiplier)
;;     mov     eax, [esp+32-8+4] ; eax = color
;     mov     [LFB_BASE+edi], ecx
;     ret


; DRAWLINE

align 4
__sys_draw_line:
     call    [_display.disable_mouse]

; draw a line
; eax = HIWORD = x1
;       LOWORD = x2
; ebx = HIWORD = y1
;       LOWORD = y2
; ecx = color
; edi = force ?
	pusha

dl_x1 equ esp+20
dl_y1 equ esp+16
dl_x2 equ esp+12
dl_y2 equ esp+8
dl_dx equ esp+4
dl_dy equ esp+0

     xor     edx, edx	   ; clear edx
     xor     esi, esi	   ; unpack arguments
     xor     ebp, ebp
     mov     si, ax	   ; esi = x2
     mov     bp, bx	   ; ebp = y2
     shr     eax, 16	   ; eax = x1
     shr     ebx, 16	   ; ebx = y1
     push    eax	   ; save x1
     push    ebx	   ; save y1
     push    esi	   ; save x2
     push    ebp	   ; save y2

; checking x-axis...
     sub     esi, eax	   ; esi = x2-x1
     push    esi	   ; save y2-y1
     jl      .x2lx1	   ; is x2 less than x1 ?
     jg      .no_vline	   ; x1 > x2 ?
     mov     edx, ebp	   ; else (if x1=x2)
     call    vline
     push    edx    ; necessary to rightly restore stack frame at .exit
     jmp     .exit
.x2lx1:
     neg     esi	    ; get esi absolute value
.no_vline:
; checking y-axis...
     sub     ebp, ebx	    ; ebp = y2-y1
     push    ebp	    ; save y2-y1
     jl      .y2ly1	    ; is y2 less than y1 ?
     jg      .no_hline	    ; y1 > y2 ?
     mov     edx, [dl_x2]   ; else (if y1=y2)
     call    hline
     jmp     .exit

.y2ly1:
     neg     ebp	    ; get ebp absolute value
.no_hline:
     cmp     ebp, esi
     jle     .x_rules	    ; |y2-y1| < |x2-x1|  ?
     cmp     [dl_y2], ebx   ; make sure y1 is at the begining
     jge     .no_reverse1
     neg     dword [dl_dx]
     mov     edx, [dl_x2]
     mov     [dl_x2], eax
     mov     [dl_x1], edx
     mov     edx, [dl_y2]
     mov     [dl_y2], ebx
     mov     [dl_y1], edx
.no_reverse1:
     mov     eax, [dl_dx]
     cdq		    ; extend eax sing to edx
     shl     eax, 16	    ; using 16bit fix-point maths
     idiv    ebp	    ; eax = ((x2-x1)*65536)/(y2-y1)
     mov     edx, ebp	    ; edx = counter (number of pixels to draw)
     mov     ebp, 1 *65536  ; <<16   ; ebp = dy = 1.0
     mov     esi, eax	    ; esi = dx
     jmp     .y_rules

.x_rules:
     cmp     [dl_x2], eax    ; make sure x1 is at the begining
     jge     .no_reverse2
     neg     dword [dl_dy]
     mov     edx, [dl_x2]
     mov     [dl_x2], eax
     mov     [dl_x1], edx
     mov     edx, [dl_y2]
     mov     [dl_y2], ebx
     mov     [dl_y1], edx
.no_reverse2:
     xor     edx, edx
     mov     eax, [dl_dy]
     cdq		    ; extend eax sing to edx
     shl     eax, 16	    ; using 16bit fix-point maths
     idiv    esi	    ; eax = ((y2-y1)*65536)/(x2-x1)
     mov     edx, esi	    ; edx = counter (number of pixels to draw)
     mov     esi, 1 *65536  ;<< 16   ; esi = dx = 1.0
     mov     ebp, eax	    ; ebp = dy
.y_rules:
     mov     eax, [dl_x1]
     mov     ebx, [dl_y1]
     shl     eax, 16
     shl     ebx, 16
align 4
.draw:
     push    eax ebx
     shr     eax, 16
     shr     ebx, 16
     call    _putpixel
     pop     ebx eax
     add     ebx, ebp	     ; y = y+dy
     add     eax, esi	     ; x = x+dx
     dec     edx
     jnz     .draw
; force last drawn pixel to be at (x2,y2)
     mov     eax, [dl_x2]
     mov     ebx, [dl_y2]
     call    _putpixel
.exit:
     add     esp, 6*4
     popa
     call   [draw_pointer]	; mouse 
     ret

align 4
hline:
; ------------  draw a horizontal line -------------
; eax = x1
; edx = x2
; ebx = y
; ecx = color
; edi = force ?
     cmp     ebx, [Screen_Max_Y]
     jge     .out
     pushad

     bt      ecx, 24			; color inversion check
     rcl     edi,1			; forced graphics check

     mov     ebp, ebx
     shr     ebp, 1
     imul    ebp, [_WinMapWidth]	; ebp = screen map base
     add     ebp, [_WinMapAddress]

     cmp     edx, eax			; to make sure x2 > x1
     jge     @f
     xchg    eax, edx
@@:
     cmp     eax, [Screen_Max_X]
     jge     .exit

     mov     esi, eax
     shr     esi, 4
     add     ebp, esi			; ebp -> win_map element

     imul    ebx, [BytesPerScanLine]	; ebx -> LFB pix_line
     add     ebx, LFB_BASE

     cmp     edx, [Screen_Max_X]	; last check
     jb      @f
     mov     edx, [Screen_Max_X]

@@:  mov     esi, ecx			; store color here
     mov     cl, byte [CURRENT_TASK]	;
     mov     ch, cl
     mov     [CURRENT_TASK+2], cx
     mov     [CURRENT_TASK+1], cl	; replicate byte to dword

.newsegment:
     mov     ecx, [ebp] 		; check the line segment (16 pixels!)
     xor     ecx, [CURRENT_TASK]
; -- the line ---
     jmp     dword [hline.drawtable + edi*4]	; (C) Serge, 2010


align 4 				; internal loop
.invert_color:
     mov     esi, [ebx+eax*4]
     xor     esi, 0x00FFFFFF
align 4
.check_overlap:
     or      cl, cl
     jz      .putpixel
     jmp     .nextpixel
align 4
.invert_force:
     mov     esi, [ebx+eax*4]
     xor     esi, 0x00FFFFFF
align 4
.putpixel:
     mov     [ebx+eax*4], esi
align 4
.nextpixel:
     inc     eax
     cmp     eax, edx
     ja      .exit				; line drawn -- exit all loops
     test    al, 3
     jz     .newtile
.newpixel:
     jmp     dword [hline.drawtable + edi*4]	; the internal loop
.newtile:
     inc     ebp
     test    ebp, 3
     jz     .newsegment				; the external loop
     shr     ecx, 8
     jmp     dword [hline.drawtable + edi*4]

.exit:
     mov    eax, 0x0FF
     and    [CURRENT_TASK], eax
     popad
.out:
     ret
align 4
.drawtable:
dd	.check_overlap	; general case
dd	.invert_color
dd	.putpixel	; force to draw it
dd	.invert_force


align 4
vline:
; ---------  draw a vertical line  ------------
; eax = x
; ebx = y1
; edx = y2
; ecx = color
; edi = force ?
     cmp     eax, [Screen_Max_X]
     jge     .out
     pushad
     bt      ecx, 24			; color inversion check
     rcl     edi, 1			; forced graphics check

     cmp     edx, ebx			; to make sure y2 > y1
     jge     @f
     xchg    ebx, edx
@@:
     cmp     ebx, [Screen_Max_Y]
     jge     .exit
     mov     ebp, ebx
     shr     ebp, 1
     imul    ebp, [_WinMapWidth]
     add     ebp, [_WinMapAddress]
     mov     esi, eax
     shr     esi, 1
     shr     esi, 1
     add     ebp, esi			; ebp = screen map at (x, y1)
     push    ebx

     imul    ebx, [BytesPerScanLine]
     shl     eax, 1
     shl     eax, 1
     add     eax, ebx
     add     eax, LFB_BASE
     pop     ebx			; restore ebx = y1
     cmp     edx, [Screen_Max_Y]	; the last check
     jb     .draw
     mov     edx, [Screen_Max_Y]	; to prevent off-screen drawing

.draw:
     jmp     dword [vline.drawtable + edi*4]
align 4
.invert_color:
     mov     ecx, [eax]
     xor     ecx, 0x00FFFFFF
align 4
.check_overlap:
     movzx   esi, byte [ebp]
     cmp     esi, [CURRENT_TASK]
     je      .putpixel
     jmp     .nextpixel

align 4
.invert_force:
     mov     ecx, [eax]
     xor     ecx, 0x00FFFFFF
align 4
.putpixel:
     mov     [eax], ecx
align 4
.nextpixel:
     add     eax, [BytesPerScanLine]
     inc     ebx
     test    bl, 1
     jnz     @f
     add     ebp, [_WinMapWidth]	
@@:     
     cmp     ebx, edx
     ja     .exit
     jmp     dword [vline.drawtable + edi*4]
.exit:
     shr     edi, 1
     popad

.out:
     ret
align 4
.drawtable:
dd	.check_overlap	; general case
dd	.invert_color
dd	.putpixel	; force to draw it
dd	.invert_force


;*************************************************



align 4
; eax   xOrigin
; ebx   yOrigin
; ecx   xSize
; edx   ySize
; edi   color

_drawbar:
        pushad
        sub     esp, putimg.stack_data
	mov     [img_bytes_per_pix], 0
	mov     [img_buf_line_size], 0
	mov	[img_draw_core_fn],  draw_core_0bpp
	mov	[img_draw_edge_fn],  draw_edge_0bpp
     	mov     [putimg.image_sx], ecx
     	mov     [putimg.image_sy], edx
        mov     edx, ebx
        mov     [img_palette], edi
        mov     esi, img_palette     
        mov     [img_buf_origin], esi     

	jmp     _putimage.calculate_abs_coords
;       ret


draw_background:
        pushad
	pushfd
	cld	; increment edi here!
        mov     ebp, [_WinMapAddress]
        mov	eax, 0x00337766		; bgndcolor
	mov	bl, 1
	mov	edx, [Screen_Max_X]
	shr	edx, 1
	mov	edi, LFB_BASE
	mov	esi, [BytesPerScanLine]	
.new_row:
	xor	ecx, ecx
.fill:
	cmp	byte [ebp+ecx], bl
	jne	.next
	
	mov	[edi+esi], eax		; fill all 8 pixels of this tile
	stosd
	mov	[edi+esi], eax
	stosd
	mov	[edi+esi], eax
	stosd
	mov	[edi+esi], eax
	stosd
.next:	inc	ecx
	cmp	ecx, [_WinMapWidth]
	jb	.fill
	dec	edx
	jz	.done
	add	ebp, ecx		; += [_WinMapWidth]
	add	edi, esi 		; += [BytesPerScanLine]
	jmp	.new_row
.done:
	popfd
        popad
        ret


drawbackground_stretch:		; left for future development
	call	drawbackground
	ret
drawbackground_tiled:		; left for future development
	call	drawbackground
	ret

uglobal
align 4
bgr_cur_line	rd	1920	; maximum width of screen
bgr_next_line	rd	1920
endg


_init_background:
;	mov	edi, BgrAuxTable
;	xor	edx, edx
;.loop2:
;	mov	eax, edx
;	shl	eax, 8
;	neg	eax
;	mov	ecx, 0x200
;.loop1:
;	mov	byte [edi], ah
;	inc	edi
;	add	eax, edx
;	loop	.loop1
;	add	dl, 4
;	jnz	.loop2
        mov     byte [REDRAW_BACKGROUND], 1
;        mov     dword[BgrAuxTable], 0x00337766
	ret


diff16 "GRAPH32 code end ",0,$
diff10 "GRAPH32 code size",get_pixel,$


