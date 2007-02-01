;====================================================================
;=====  …„‘’€ ===========================================
;====================================================================
scr_window:
.start:
.red:
	call	.draw_window
.still:
	wait_event .red,close
	jmp	.still

.draw_window:
	start_draw_window
	xor	eax,eax
	movsx	ebx,word [scr.width]
	movsx	ecx,word [scr.height]
	;xor     edx,edx
	mov	edx,0x01000000
	xor	esi,esi
	xor	edi,edi
	int	0x40

	movr	 eax,7
	mov	ebx,[scr_buf.ptr]
	mov	ecx,[scr_buf.size]
	xor	edx,edx
	int	0x40

	movr	 eax,13
	mov	ebx,19*65536+231
	mov	ecx,19*65536+10
	mov	edx,0x00FF00
	int	0x40

	mov	edi,.label
	call	draw_label

	stop_draw_window
ret
.label label 20,20,0x80000000,$+4
db 'β® Ά θ α­¨¬®ª. ‚λε®¤ - «ξ΅ ο ª« Ά¨θ .',0