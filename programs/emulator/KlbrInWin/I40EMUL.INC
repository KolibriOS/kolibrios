; Cb-n#%li.-# @l$i Lkbnbe
convert_color:
	bswap	eax
	shr	eax, 8
	ret

draw_window_base:
	mov	al, byte [ebp+tls.color_main+3]
	and	eax, 0xF
	dec	eax
	js	.type1
	jz	.nodraw
	dec	eax
	jz	.type2
; window with skin
	push	1
	call	draw_border
	call	draw_caption_skinned
	movzx	eax, [ebp+tls.y_size]
	cmp	eax, 21+5
	jle	@f
	test    byte [ebp+tls.color_main+3], 40h
	jnz     @f
	push    1
	push    [ebp+tls.color_main]
	movzx   ecx, [ebp+tls.y_size]
	sub     ecx, 5
	push    ecx
	movzx   ecx, [ebp+tls.x_size]
	sub     ecx, 5
	push    ecx
	push    [_skinh]
	push    5
	call    rectangle_gradient
@@:
; close button
	mov	edx, 40000001h	; id=1, no draw
	xor	ebx, ebx
	cmp	[skin_btn_close.left], ebx
	jge	@f
	mov	ebx, dword [ebp+tls.x_size]
@@:
	add	ebx, [skin_btn_close.left]
	shl	ebx, 16
	mov	bx, word [skin_btn_close.width]
	dec	ebx
	mov	ecx, [skin_btn_close.top]
	shl	ecx, 16
	mov	cx, word [skin_btn_close.height]
	dec	ecx
	call	add_button
; minimize button
	mov	edx, 4000FFFFh	; id=65535, no draw
	xor	ebx, ebx
	cmp	[skin_btn_minimize.left], ebx
	jge	@f
	mov	ebx, dword [ebp+tls.x_size]
@@:
	add	ebx, [skin_btn_minimize.left]
	shl	ebx, 16
	mov	bx, word [skin_btn_minimize.width]
	dec	ebx
	mov	ecx, [skin_btn_minimize.top]
	shl	ecx, 16
	mov	cx, word [skin_btn_minimize.height]
	dec	ecx
	call	add_button
	jmp	.nodraw
.type1:
; border
	mov	eax, [ebp+tls.color_border]
	call	create_select_pen
	push	eax
	xor	esi, esi
	call	rect_wnd
	call	select_delete
; caption
        call    draw_caption_type1
; work area
        test    byte [ebp+tls.color_main+3], 40h
        jnz     .nodraw
	push	1
	push	[ebp+tls.color_main]
	movzx	eax, [ebp+tls.y_size]
	dec	eax
	push	eax
	movzx	eax, [ebp+tls.x_size]
	dec	eax
	push	eax
	push	21
	push	1
	call	rectangle_gradient
	jmp	.nodraw
.type2:
; border
	push	eax
	call	draw_border
; caption
        call    draw_caption_type2
; work area
        test    byte [ebp+tls.color_main+3], 40h
        jnz     .nodraw
	push	1
	push	[ebp+tls.color_main]
	movzx	eax, [ebp+tls.y_size]
	sub	eax, 5
	push	eax
	movzx	eax, [ebp+tls.x_size]
	sub	eax, 5
	push	eax
	push	20
	push	5
	call	rectangle_gradient
.nodraw:
; caption string
        call    draw_caption_string
; draw buttons
	pushad
	mov	esi, [ebp+tls.buttons]
@@:
	test	esi, esi
	jz	@f
	push	[esi+button_desc.next]
	mov	ebx, dword [esi+button_desc.xsize]
	mov	ecx, dword [esi+button_desc.ysize]
	test	byte [esi+button_desc.id+3], 0x40
	mov	esi, [esi+button_desc.color]
	jnz	.sk
	call	draw_button
.sk:
	pop	esi
	jmp	@b
@@:
	popad
	ret

draw_caption_type1:
	push	040404h
	push	[ebp+tls.color_capt]
	movzx	eax, [ebp+tls.y_size]
	dec	eax
	cmp	eax, 21
	jb	@f
	mov	eax, 21
@@:
	push	eax
	movzx	eax, [ebp+tls.x_size]
	dec	eax
	push	eax
	push	1
	push	1
	call	rectangle_gradient
        ret

draw_caption_type2:
	mov	eax, [ebp+tls.color_capt]
	mov	ecx, 0x040404
	test	eax, 0x40000000
	jz	@f
	or	ecx, 0x80000000
	xor	eax, 0xC0000000
@@:
	push	ecx
	push	eax
	movzx	eax, [ebp+tls.y_size]
	sub	eax, 4
	cmp	eax, 20
	jb	@f
	mov	eax, 20
@@:	push	eax
	movzx	eax, [ebp+tls.x_size]
	sub	eax, 4
	push	eax
	push	4
	push	4
	call	rectangle_gradient
	ret

draw_caption_skinned:
	xor	ebx, ebx
	mov	eax, [left_bmp]
	cmp	[ebp+tls.bActive], bl
	jnz	@f
	mov	eax, [left1_bmp]
@@:
	xor	ecx, ecx
	call	putbmp
	movzx	esi, [ebp+tls.x_size]
	mov	ecx, [eax+4]
	sub	esi, ecx
	mov	eax, [oper_bmp]
	cmp	[ebp+tls.bActive], bl
	jnz	@f
	mov	eax, [oper1_bmp]
@@:
	sub	esi, [eax+4]
	mov	eax, [base_bmp]
	cmp	[ebp+tls.bActive], bl
	jnz	@f
	mov	eax, [base1_bmp]
@@:
	cmp	esi, [eax+4]
	jle	.nobase
@@:
	call	putbmp
	add	ecx, [eax+4]
	sub	esi, [eax+4]
	jg	@b
.nobase:
	movzx	ecx, [ebp+tls.x_size]
	mov	eax, [oper_bmp]
	cmp	[ebp+tls.bActive], bl
	jnz	@f
	mov	eax, [oper1_bmp]
@@:
	sub	ecx, [eax+4]

putbmp:
; in: eax=bmpinfo,ecx=y*65536+x,edi=hDC
	push	eax ecx
	lea	edx, [eax+40]
	push	ebx		; fuColorUse = DIB_RGB_COLORS
	push	eax		; lpbmi
	push	edx		; lpBits
	push	dword [eax+8]	; cScanLines
	push	ebx		; uStartScan
	push	ebx		; YSrc
	push	ebx		; XSrc
	push	dword [eax+8]	; dwHeight
	push	dword [eax+4]	; dwWidth
	movzx	eax, cx
	shr	ecx, 10h
	push	ecx		; YDest
	push	eax		; XDest
	push	edi		; hDC
	call	[SetDIBitsToDevice]
	pop	ecx eax
	ret

draw_caption_string:
        test    byte [ebp+tls.color_main+3], 0x10
        jnz     @f
.ret:
.nodraw:
        ret
@@:
        mov     esi, [ebp+tls.caption]
        test    esi, esi
        jz      .ret
        add     esi, [base]
        mov     al, byte [ebp+tls.color_main+3]
        and     eax, 0xF
        dec     eax
        js      .type1
        jz      .nodraw
        dec     eax
        jz      .type2
; caption for skinned windows
; determine maximum caption length
        mov     ax, [ebp+tls.x_size]
        sub     ax, [margins+2]
        sub     ax, [margins]
        js      .ret
        cwd
        push    6
        pop     ebx
        div     bx
        mov     ecx, eax
; determine coordinates
        mov     ebx, dword [margins]
        mov     bx, word [_skinh]
        sub     bx, [margins+6]
        sub     bx, [margins+4]
        sar     bx, 1
        adc     bx, 0
        add     bx, [margins+6]
        add     bx, -3
.common:
        mov     edx, esi
@@:
        lodsb
        test    al, al
        loopnz  @b
        jnz     @f
        dec     esi
@@:     sub     esi, edx
        mov     ecx, dword [common_colors+16]
        jmp     i40_writetext_l2
.type1:
.type2:
; caption for windows without skin
        movzx   eax, [ebp+tls.x_size]
        sub     eax, 16
        js      .ret
        cdq
        push    6
        pop     ebx
        div     ebx
        mov     ecx, eax
        mov     ebx, 0x00080007
        jmp     .common

i40_draw_window:
	mov	[ebp+tls.color_main], edx
	mov	[ebp+tls.color_capt], esi
	mov	[ebp+tls.color_border], edi
	mov	[ebp+tls.curdraw], 0
	cmp	[ebp+tls.showwnd], 0
	jnz	.wasshown
; real size is 1 pixel more
	inc	ebx
	mov	dword [ebp+tls.x_size], ebx
	inc	ecx
	mov	dword [ebp+tls.y_size], ecx
	test    edx, 0x10000000
	jz      @f
	mov     [ebp+tls.caption], edi
@@:
	push	0
	movzx	eax, cx
	push	eax
	movzx	eax, bx
	push	eax
	shr	ecx, 10h
	push	ecx
	shr	ebx, 10h
	push	ebx
	push	[ebp+tls.hWnd]
	call	[MoveWindow]
	push	5	; SW_SHOW
	push	[ebp+tls.hWnd]
	call	[ShowWindow]
	mov	[ebp+tls.showwnd], 1
	cmp	[ebp+tls.lpShapeData], 0
	jz	@f
	call	set_window_shape
@@:
.wasshown:
; define client box
        test    byte [ebp+tls.color_main+3], 0x20
        jnz     .client_relative
        and     [ebp+tls.client_left], 0
        and     [ebp+tls.client_top], 0
        movzx   eax, [ebp+tls.x_size]
        mov     [ebp+tls.client_width], eax
        movzx   eax, [ebp+tls.y_size]
        mov     [ebp+tls.client_height], eax
        jmp     .client_set
.client_relative:
        mov     eax, [_skinh]
        mov     [window_topleft+8*3+4], eax
	mov	[window_topleft+8*4+4], eax
        mov     al, byte [ebp+tls.color_main+3]
        and     eax, 0xF
        mov     edx, [eax*8+window_topleft]
        mov     [ebp+tls.client_left], edx
        neg     edx
        movzx   ecx, [ebp+tls.x_size]
        lea     ecx, [ecx+edx*2]
        mov     [ebp+tls.client_width], ecx
        mov     eax, [eax*8+window_topleft+4]
        mov     [ebp+tls.client_top], eax
        movzx   ecx, [ebp+tls.y_size]
        sub     ecx, eax
        add     ecx, edx
        mov     [ebp+tls.client_height], ecx
.client_set:
	push	0
	push	[ebp+tls.hWnd]
	call	[ValidateRect]
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, edi
	call	draw_window_base
	push	edi
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	ret

i40_put_pixel:
	test	edx, 1000000h
	jnz	.negate
	mov	eax, edx
	call	convert_color
	push	eax
	add     ebx, [ebp+tls.client_left]
	add     ecx, [ebp+tls.client_top]
	push	ecx
	push	ebx
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, edi
.1:
	push	edi
	call	[SetPixel]
	push	edi
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	ret
.negate:
	push	ecx
	push	ecx
	push	ebx
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, edi
	push	edi
	call	[GetPixel]
	pop	ecx
	not	eax
	push	eax
	push	ecx
	push	ebx
	jmp	.1

i40_getkey:
	movzx	ecx, [ebp+tls.keybuflen]
	jecxz	.empty
	lea	esi, [ebp+tls.keybuffer]
	mov	edi, esi
	lodsb
	shl	eax, 8
	mov	[esp+20h], ax
	dec	ecx
	mov	[ebp+tls.keybuflen], cl
	rep	movsb
	ret
.empty:
	mov	byte [esp+20h], 1
	ret

i40_get_sys_time:
	sub	esp, 10h
	push	esp
	call	[GetLocalTime]
	movzx	eax, word [esp+12]
	aam
	shl	ax, 4
	shr	ah, 4
	shl	eax, 16-4
	mov	ax, [esp+10]
	aam
	shl	ax, 4
	shr	ah, 4
	shr	ax, 4
	mov	bl, al
	mov	ax, [esp+8]
	aam
	shl	ax, 4
	shr	ah, 4
	shr	ax, 4
	mov	ah, bl
	add	esp, 10h
	mov	[esp+20h], eax
	ret

i40_writetext:
	add	edx, [base]
i40_writetext_l1:
        add     bx, word [ebp+tls.client_top]
        ror     ebx, 16
        add     bx, word [ebp+tls.client_left]
        ror     ebx, 16
i40_writetext_l2:
        push    edi
	push	ecx
	push	edx
	push	[ebp+tls.hWnd]
	call	[GetDC]
	mov	ecx, esi
	pop	esi
	pop	edi
	push	eax
; ecx=length, esi=pointer, ebx=x*65536+y, edi=font&color
.loop:
	xor	eax, eax
	lodsb
	test    edi, edi
	js      .test_asciiz
	dec     ecx
	js      .done
	jmp     @f
.test_asciiz:
        test    eax, eax
        jz      .done
@@:
	push	esi
	push	ecx
	mov	esi, [char_mt]
	lea	esi, [esi + eax*8]
	add	esi, eax
	mov     ecx, 6
	test	edi, 10000000h
	jz	@f
	sub	esi, [char_mt]
	add	esi, [char2_mt]
	add	esi, eax
	lodsb
	mov     ecx, eax
@@:
	mov	edx, 9
.intloop:
	lodsb
	push	ebx
	push    ecx
.intintloop:
        shr     al, 1
        pushad
        jnc     .nopix
	mov	eax, edi
@@:
	call	convert_color
	push	eax
	movzx	eax, bx
	push	eax
	shr	ebx, 16
	push	ebx
	push	dword [esp+3Ch]	; hDC
	call	[SetPixel]
	jmp     @f
.nopix:
        mov     eax, [esp+34h]
        test    edi, 0x40000000
        jnz     @b
@@:
	popad
	add	ebx, 10000h
	loop    .intintloop
	pop     ecx
	pop	ebx
	inc	ebx
	dec	edx
	jnz	.intloop
	sub	ebx, 9
   	shl	ecx, 16
	add	ebx, ecx
	pop	ecx
	pop	esi
	jmp	.loop
.done:
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	pop     edi
	ret

i40_delay:
	imul	ebx, ebx, 10
	push	ebx
	call	[Sleep]
	ret

ramdisk2win32:
	push	edi
	add	ebx, [base]
	mov	esi, ramdisk_path
	push	esi
	call	[lstrlenA]
	xchg	eax, ecx
	rep	movsb
	push	edi
	push	edi
	mov	ecx, 8
@@:
	mov	al, [ebx]
	inc	ebx
	cmp	al, '.'
	jz	@f
	stosb
	loop	@b
@@:
	dec	edi
	cmp	byte [edi], ' '
	jz	@b
	inc	edi
	mov	al, '.'
	stosb
	cmp	[ebx], al
	jnz	@f
	inc	ebx
@@:
	mov	ecx, 3
@@:
	mov	al, [ebx]
	inc	ebx
	stosb
	loop	@b
@@:
	dec	edi
	cmp	byte [edi], ' '
	jz	@b
	inc	edi
	xor	eax, eax
	stosb
	call	[OemToCharA]
	xor	eax, eax
	pop	edi
	ret

i40_read_floppy_file:
	sub	esp, 512
	mov	edi, esp
	push	esi
	call	ramdisk2win32
	pop	ebx
	push	eax
	push	eax
	push	3
	push	eax
	push	1
	push	80000000h
	push	edi
	call	[CreateFileA]
	add	esp, 512
	inc	eax
	jz	.err_ret
	dec	eax
	xchg	eax, esi
	push	0
	push	esi
	call	[GetFileSize]
	xchg	eax, edi
	push	eax
	mov	ecx, esp
	push	0
	push	ecx
	push	edi
	add	ebx, [base]
	push	ebx
	push	esi
	call	[ReadFile]
	pop	eax
	push	esi
	call	[CloseHandle]
	mov	[esp+20h], edi
	ret
.err_ret:
	or	dword [esp+20h], -1
	ret

pad_bmp:
	pushad
	movzx	eax, cx
	shr	ecx, 10h
	lea	ecx, [ecx+ecx*2]
	add	ecx, 3
	and	ecx, not 3
	mul	ecx
	push	eax
	call	malloc
	mov	[esp+1Ch], eax
	popad
	push	eax
	mov	edi, eax
	push	ecx
	mov	esi, ebx
.extloop:
	push	ecx
	shr	ecx, 10h
	lea	ecx, [ecx+ecx*2]
	mov     eax, ecx
	shr     ecx, 2
	rep	movsd
	mov     ecx, eax
	and     ecx, 3
	rep     movsb
	mov	al, 0
@@:	test	edi, 3
	jz	@f
	stosb
	jmp	@b
@@:	pop	ecx
	dec	cx
	jnz	.extloop
	pop	ecx
	pop	edi
	jmp	pad_cont

i40_putimage:
	add	ebx, [base]
; Windows requires that all scanlines are DWORD-padded
	mov	edi, ebx
	test	ecx, 30000h
	jnz	pad_bmp
pad_cont:
	xor	esi, esi
; BITMAPINFO
	push	esi	; biClrImportant
	push	esi	; biClrUsed
	push	esi	; biYPelsPerMeter
	push	esi	; biXPelsPerMeter
	push	esi	; biSizeImage
	push	esi	; biCompression
	push	180001h	; biPlanes, biBitCount
	movzx	eax, cx
	neg	eax
	push	eax	; biHeight
	neg	eax
	shr	ecx, 10h
	push	ecx	; biWidth
	push	40	; biSize
	push	ebx
	lea	ebx, [esp+4]
; SetDIBitsToDevice
	push	esi	; fuColorUse = DIB_RGB_COLORS
	push	ebx	; lpbmi
	push	edi	; lpvBits
	push	eax	; cScanLines
	dec	eax
	push	eax	; uStartScan
	push	eax	; YSrc
	inc	eax
	push	esi	; XSrc
	push	eax	; dwHeight
	push	ecx	; dwWidth
	movzx	ecx, dx
	add     ecx, [ebp+tls.client_top]
	push	ecx	; YDest
	shr	edx, 10h
	add     edx, [ebp+tls.client_left]
	push	edx	; XDest
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, ebx
	push	ebx	; hdc
	call	[SetDIBitsToDevice]
	xchg	eax, ebx
	pop	ebx
	add	esp, 40
	push	eax
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	cmp	edi, ebx
	jz	@f
	push	edi
	call	free
@@:
	ret

draw_button:
	push	ebx
	push	ecx
	push	ebp
	mov	ebp, esp
; word [ebp+4]=y_size, word [ebp+6]=y_start, word [ebp+8]=x_size, word [ebp+10]=x_start
; button body
	push	esi
	cmp	[buttontype], 0
	jz	.flatbtn
	or	esi, 80000000h
	push	esi
	mov	edx, 2
.l2:
	cmp	byte [esp+edx], 0xEB
	jbe	@f
	mov	byte [esp+edx], 0xEB
@@:	add	byte [esp+edx], 0x14
	dec	edx
	jns	.l2
	pop	esi
	mov	eax, 010101h
	cmp	cx, 20
	ja	@f
	mov	eax, 020202h
@@:
	sub	esi, eax
.flatbtn:
	push	eax
	push	esi
	movzx	eax, word [ebp+6]
	add	ax, cx
	push	eax
	movzx	eax, word [ebp+10]
	add	ax, bx
	push	eax
	shr	ecx, 16
	push	ecx
	shr	ebx, 16
	push	ebx
	call	rectangle_gradient
; button frames
	pop	eax
	push	eax
	push	eax
	xor	ecx, ecx
.l1:
	cmp	byte [esp+ecx], 0xDF
	jbe	@f
	mov	byte [esp+ecx], 0xDF
@@:
	inc	ecx
	cmp	ecx, 3
	jb	.l1
	pop	eax
	add	eax, 202020h
	call	create_select_pen
	push	eax
	push	0
	movzx	eax, word [ebp+6]
	add	ax, [ebp+4]
	push	eax
	push	ebx
	push	edi
	call	[MoveToEx]
	movzx	eax, word [ebp+6]
	push	eax
	push	ebx
	push	edi
	call	[LineTo]
	movzx	eax, word [ebp+6]
	push	eax
	movzx	eax, word [ebp+8]
	add	ax, bx
	push	eax
	push	edi
	call	[LineTo]
	call	select_delete
	cmp	byte [ebp-4], 20h
	jae	@f
	mov	byte [ebp-4], 20h
@@:
	cmp	byte [ebp-3], 20h
	jae	@f
	mov	byte [ebp-3], 20h
@@:
	cmp	byte [ebp-2], 20h
	jae	@f
	mov	byte [ebp-2], 20h
@@:
	mov	eax, [ebp-4]
	sub	eax, 202020h
	call	create_select_pen
	push	eax
	movzx	eax, word [ebp+4]
	add	ax, [ebp+6]
	push	eax
	movzx	eax, word [ebp+8]
	add	ax, bx
	push	eax
	push	edi
	call	[LineTo]
	movzx	eax, word [ebp+4]
	add	ax, [ebp+6]
	push	eax
	push	ebx
	push	edi
	call	[LineTo]
	call	select_delete
	pop	eax
	pop	ebp
	pop	ecx
	pop	ebx
	ret

add_button:
	push	esi
	lea	esi, [ebp+tls.buttons]
@@:
	mov	eax, [esi]
	test	eax, eax
	jz	.end
	xchg	esi, eax
	jmp	@b
.end:
	push	ecx
	push	edx
	push	button_desc.size
	call	malloc
	mov	[esi], eax
	mov	[eax+button_desc.next], 0
	pop	[eax+button_desc.id]
	mov	dword [eax+button_desc.xsize], ebx
	pop	dword [eax+button_desc.ysize]
	pop	[eax+button_desc.color]
	ret

i40_define_button:
	test	edx, 0x80000000
	jnz	.delete
	rol     ebx, 16
	add     bx, word [ebp+tls.client_left]
	rol     ebx, 16
	rol     ecx, 16
	add     cx, word [ebp+tls.client_top]
	rol     ecx, 16
	test	edx, 0x40000000
	jnz	.nodraw
	push	ecx
	push	edx
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, edi
	pop	edx
	pop	ecx
	pushad
	call	draw_button
	push	edi
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	popad
.nodraw:
	call	add_button
.ret:
	ret
.delete:
	and	edx, not 0x80000000
	lea	esi, [ebp+tls.buttons]
@@:
	mov	eax, [esi]
	test	eax, eax
	jz	@f
; The kernel checks only low word of button ID!
	cmp	word [eax+button_desc.id], dx
	jz	.found
	xchg	eax, esi
	jmp	@b
@@:
	ret
.found:
; if active button is deleting, there is no more active button
	cmp	eax, [ebp+tls.active_button]
	jnz	@f
	and	[ebp+tls.active_button], 0
@@:
	push	[eax+button_desc.next]
	push	eax
	call	free
	pop	[esi+button_desc.next]
	ret

invalid_slot_msg db 'Emulated program has requested information on non-existent slot'
        db      ' and will be terminated',0
invalid_slot:
	call	release_shared
	push	0
	push	0
	push	invalid_slot_msg
	push	0
	call	[MessageBoxA]
	jmp	i40_terminate

get_os_process_info:
	pushad
	mov	ebx, 5
	call	i40_sys_service
	popad
	stosd		; .cpu_usage
	xor	eax, eax
	inc	eax
	stosw		; .window_stack_position
	stosw		; .window_stack_value
	dec	eax
	stosw		; .not_used1
	mov	eax, 'OS/I'
	stosd
	mov	eax, 'DLE '
	stosd
	mov	eax, '   '
	stosd
	xor	eax, eax
	stosd		; .memory_start
	mov	eax, 0x01000000-1
	stosd		; .used_memory
	xor	eax, eax
	inc	eax
	stosd		; .PID
	dec	eax
	stosd
	stosd
	stosd
	stosd
	stosw		; .slot_state
	scasw
	stosd
	stosd
	stosd
	stosd
	stosb
	jmp	got_process_info

i40_get_process_info:
	cmp	ecx, -1
	jnz	@f
	mov	ecx, [ebp+tls.cur_slot]
	inc	ecx
@@:
	add	ebx, [base]
	mov	edx, [shared_data]
;	cmp	ecx, [edx+shared_data_struc.alloc_threads]	; too many programs will fail
	cmp	ecx, 256
	ja	invalid_slot
	dec	ecx
	call	acquire_shared
	mov	eax, ecx
	call	get_slot_ptr
	mov	esi, edi
	mov	edi, ebx
	jecxz	get_os_process_info
	xor	eax, eax
	stosd		; .cpu_usage
	mov	eax, [edx+shared_data_struc.alloc_threads]
	stosw		; .window_stack_position
	stosw		; .window_stack_value
	xor	eax, eax
	stosw		; .not_used1
	push	esi
	add	esi, 28
	movsd
	movsd
	movsd
	pop	esi
	stosd		; .memory_start
	mov	eax, [esi+24]
	stosd		; .used_memory
	mov	eax, [esi]
	stosd		; .PID
	push    ecx
	xor	eax, eax
	push	eax
	push	eax
	push	eax
	push	eax
	push	esp
	push	dword [esi+20]
	call	[GetWindowRect]
	pop	eax
	stosd
	pop	eax
	stosd
	pop	eax
	sub	eax, [edi-8]
	sub	eax, 1
	adc	eax, 0
	stosd
	pop	eax
	sub	eax, [edi-8]
	sub	eax, 1
	adc	eax, 0
	stosd
	pop     ecx
	and	word [edi], 0	; .slot_state
	cmp	dword [esi], 0
	jnz	@f
	mov	byte [edi], 9
@@:
        scasd
; client area coordinates
        push    esi
; for my window, return true coordinates from tls
        cmp     ecx, [ebp+tls.cur_slot]
        jnz     .notmy
        lea     esi, [ebp+tls.client_left]
        movsd
        movsd
        lodsd
        dec     eax
        stosd
        lodsd
        dec     eax
        stosd
        jmp     @f
.notmy:
; for other windows, return copy of window coordinates
        add     esi, 0x22
        movsd
        movsd
        movsd
        movsd
@@:
        pop     esi
; window state
        sub     esp, 40
        push    44
        push    esp
        push    dword [esi+20]
        call    [GetWindowPlacement]
        test    eax, eax
        jnz     @f
        add     esp, 44
        jmp     .normal
@@:
        mov     eax, [esp+8]    ; showCmd
        add     esp, 44
        cmp     eax, 3
        jz      .maximized
        cmp     eax, 2
        jz      .minimized
        test    eax, eax
        jz      .minimized
        push    dword [esi+20]
        call    [IsWindowVisible]
        test    eax, eax
        jz      .minimized
.normal:
        xor     eax, eax
        jmp     @f
.minimized:
        mov     al, 2
        jmp     @f
.maximized:
        mov     al, 1
@@:
        stosb
got_process_info:
	mov	eax, [shared_data]
	mov	eax, [eax+shared_data_struc.alloc_threads]
	mov	dword [esp+20h], eax	; number of processes
	call	release_shared
	ret

test_button_mouse:
	cmp	al, 86h
	jnz	@f
	mov	al, 6
	test	[ebp+tls.message_mask], 4
	jz	@f
	mov	[ebp+tls.translated_msg_code], al	; mouse
	mov	al, 3	; button
@@:	ret

event_test_redraw:
; if function 0 has not been called, then redraw message is present
;        cmp     [ebp+tls.showwnd], 0
;        jnz     @f
	cmp	[ebp+tls.curdraw], 0
	jz	@f
        test    [ebp+tls.message_mask], 1
        jz      @f
        pop     eax
        mov     dword [esp+20h], 1
@@:
        ret

i40_wait_event:
        call    event_test_redraw
	sub	esp, 20h
	xor	eax, eax
	xchg	al, [ebp+tls.translated_msg_code]
	test	eax, eax
	jnz	@f
	mov	al, 2
	test	byte [ebp+tls.message_mask], al
	jz	.nokey
	cmp	[ebp+tls.keybuflen], ah
	jnz	@f
.nokey:
	inc	eax
	test	[ebp+tls.message_mask], 4
	jz	.nobut
	cmp	[ebp+tls.butbuflen], ah
	jnz	@f
.nobut:
	mov	ebx, esp
	push	0
	push	0
	push	0
	push	ebx
	call	[GetMessageA]
	test	eax, eax
	jz	i40_terminate
	push	ebx
	call	[TranslateMessage]
	push	ebx
	call	[DispatchMessageA]
	add	esp, 20h
	jmp	i40_wait_event
@@:
	add	esp, 20h
	cmp	al, 6
	jnz	@f
	test	[ebp+tls.message_mask], 20h
	jz	i40_wait_event
@@:
	call	test_button_mouse
	mov	[esp+20h], eax
	ret

i40_check_event:
        call    event_test_redraw
	sub	esp, 20h
	xor	eax, eax
	xchg	al, [ebp+tls.translated_msg_code]
	or	eax, eax
	jnz	.event
	mov	al, 2
	test	byte [ebp+tls.message_mask], al
	jz	.nokey
	cmp	[ebp+tls.keybuflen], ah
	jnz	.event
.nokey:
	inc	eax
	test	[ebp+tls.message_mask], 4
	jz	.nobut
	cmp	[ebp+tls.butbuflen], ah
	jnz	.event
.nobut:
@@:
	mov	ebx, esp
	push	1
	push	0
	push	0
	push	0
	push	ebx
	call	[PeekMessageA]
	test	eax, eax
	jz	.noevent
	cmp	dword [ebx+4], 0x12	; WM_QUIT
	jz	i40_terminate
	push	ebx
	call	[TranslateMessage]
	push	ebx
	call	[DispatchMessageA]
	xor	eax, eax
	cmp	[ebp+tls.curdraw], al
	jnz	.redraw
	xchg	al, [ebp+tls.translated_msg_code]
	or	eax, eax
	jz	@b
.event:
	add	esp, 20h
	cmp	al, 6
	jnz	@f
	test	[ebp+tls.message_mask], 20h
	jz	i40_check_event
@@:
	call	test_button_mouse
	mov	[esp+20h], eax
	ret
.redraw:
	inc	eax
	add	esp, 20h
	test	[ebp+tls.message_mask], eax
	jz	i40_check_event
	mov	[esp+20h], eax
	ret
.noevent:
	add	esp, 20h
	and	dword [esp+20h], 0
	ret

i40_redraw_status:
	dec	ebx
	jz	.fn1
	dec	ebx
	jnz	not_supported_i40_fn
	ret
.fn1:
; delete all defined buttons
	xor	ebx, ebx
	mov	[ebp+tls.active_button], ebx	; no more active buttons
	xchg	ebx, [ebp+tls.buttons]
@@:
	test	ebx, ebx
	jz	.done
	push	ebx
	mov	ebx, [ebx]
	call	free
	jmp	@b
.done:
	ret

i40_drawrect:
	push	ecx
	push	edx
	push	[ebp+tls.hWnd]
	call	[GetDC]
	pop	edx
	pop	ecx
	xchg	eax, edi
	push	1
	push	edx
	movzx	eax, cx
	shr	ecx, 16
	add     ecx, [ebp+tls.client_top]
	add	eax, ecx
	push	eax
	movzx	eax, bx
	shr	ebx, 16
	add     ebx, [ebp+tls.client_left]
	add	eax, ebx
	push	eax
	push	ecx
	push	ebx
	call	rectangle_gradient
	push	edi
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	ret

get_screen_size:
	call	[GetDesktopWindow]
	push	eax
	push	eax
	call	[GetDC]
	xchg	eax, esi
	push	8	; HORZRES
	push	esi
	call	[GetDeviceCaps]
	dec	eax
	xchg	ebx, eax
	shl	ebx, 16
	push	10	; VERTRES
	push	esi
	call	[GetDeviceCaps]
	dec	eax
	or	ebx, eax
	pop	eax
	push	esi
	push	eax
	call	[ReleaseDC]
	ret

i40_get_screen_size:
	call	get_screen_size
	mov	[esp+20h], ebx
	ret

i40_set_background:
	pushad
	push	1
	call	init_background
	popad
	mov	eax, [bgr_section]
	test	eax, eax
	jnz	@f
	ret
@@:
	dec	ebx
	jz	.setsize
	dec	ebx
	jz	.setpixel
	dec	ebx
	jz	.redraw
	dec	ebx
	jz	.method
	dec	ebx
	jz	.setblock
	dec	ebx
	jz	.map
	dec	ebx
	jnz	not_supported_i40_fn
.unmap:
	push	ecx
	mov	esi, ecx
	add	esi, [base]
	lea	edi, [eax+10h]
	mov	ecx, [eax]
	imul	ecx, [eax+4]
	lea	ecx, [ecx*3]
	mov	edx, ecx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	mov	byte [eax+12], 1
	mov	byte [eax+13], 0
	pop	ecx
	jmp	i40_sys_services.free_heap
.map:
	mov	cl, 1
	xchg	cl, [eax+13]
	test	cl, cl
	jz	@f
	push	eax ecx edx
	push	100
	call	[Sleep]
	pop	edx ecx eax
	jmp	.map
@@:
	lea	esi, [eax+10h]
	mov	ecx, [eax]
	imul	ecx, [eax+4]
	lea	ecx, [ecx*3]
	pushad
	call	i40_sys_services.allocate_heap
	popad
	mov	[esp+20h], eax
	test	eax, eax
	jnz	@f
	mov	byte [esi-10h+13], 0
	ret
@@:
	mov	edi, eax
	add	edi, [base]
	mov	edx, ecx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	ret
.setblock:
	xchg	esi, ecx
	add	esi, [base]
	lea	edi, [eax+edx+10h]
	rep	movsb
	mov	byte [eax+12], 1
	ret
.setsize:
	push	ecx
@@:
	mov	cl, 1
	xchg	cl, [eax+13]
	test	cl, cl
	jz	@f
	push	eax ecx edx
	push	100
	call	[Sleep]
	pop	edx ecx eax
	jmp	@b
@@:
	pop	ecx
	mov	[eax], ecx
	mov	[eax+4], edx
	mov	byte [eax+12], 1
	mov	byte [eax+13], 0
	ret
.method:
	mov	[eax+8], ecx
	mov	byte [eax+12], 1
	ret
.setpixel:
	and	edx, 0x00FFFFFF
	and	dword [eax+ecx+10h], 0xFF000000
	or	[eax+ecx+10h], edx
	mov	byte [eax+12], 1
@@:	ret
.redraw:
	cmp	byte [eax+12], 0
	jz	@b
.redraw_force:
	mov	byte [eax+12], 0
	mov	ebx, 2
	mov	eax, [shared_data]
	cmp	[eax+shared_data_struc.vk], 0
	jnz	.settmp
	mov	eax, [SetBgrQuestion]
	dec	eax
	js	.ask
	jz	.permanent
	dec	eax
	jz	.settmp
; don't set
	ret
.ask:
	push	123h
	push	aConfirm
	push	BgrQuestionText
	push	0
	call	[MessageBoxA]
	cmp	al, 6
	jz	.permanent
	cmp	al, 7
	jz	.settmp
	ret
.permanent:
	inc	ebx
.settmp:
	push	eax
	push	esp
	push	3	; KEY_QUERY_VALUE | KEY_SET_VALUE
	push	0
	push	bgrkeyname
	push	80000001h
	call	[RegOpenKeyExA]
	test	eax, eax
	pop	esi
	jnz	.nopos
; save old reg values
	push	'0'
	push	'0'
	push	2
	mov	eax, esp
	push	eax	; lpcbData
	add	eax, 4
	push	eax	; lpData
	push	0	; lpType
	push	0	; lpReserved
	push	bgrstylevalue	; lpValueName
	push	esi	; hKey
	call	[RegQueryValueExA]
	mov	eax, esp
	mov	dword [eax], 2
	push	eax	; lpcbData
	add	eax, 8
	push	eax	; lpData
	push	0	; lpType
	push	0	; lpReserved
	push	bgrtilevalue	; lpValueName
	push	esi	; hKey
	call	[RegQueryValueExA]
	pop	eax
	mov	ecx, '0'
	mov	eax, [bgr_section]
	cmp	byte [eax+8], 1
	jz	@f
	mov	cl, '2'
@@:
	push	ecx
	mov	eax, esp
	push	2
	push	eax
	push	1
	push	0
	push	bgrstylevalue
	push	esi
	call	[RegSetValueExA]
	pop	ecx
	cmp	cl, '0'
	jnz	.stretch
	push	'1'	; tile
	jmp	@f
.stretch:
	push	'0'
@@:
	mov	eax, esp
	push	2
	push	eax
	push	1
	push	0
	push	bgrtilevalue
	push	esi
	call	[RegSetValueExA]
	pop	ecx
	push	esi
	call	[RegCloseKey]
.nopos:
	mov	edi, win32_path
	push	startcurdir
	push	edi
	call	[lstrcpyA]
	push	bgrfilename
	test	bl, 1
	jnz	@f
	mov	dword [esp], bgrtempfilename
@@:
	push	edi
	call	[lstrcatA]
	push	0
	push	80h
	push	2
	push	0
	push	0
	push	40000000h
	push	edi
	call	[CreateFileA]
	inc	eax
	jnz	@f
	push	10h
	push	0
	push	BgrFileErrorMsg
	push	0
	call	[MessageBoxA]
	pop	ecx
	pop	ecx
	ret
@@:
	dec	eax
	xchg	eax, esi
; bmp header
	mov	ecx, [bgr_section]
	mov	eax, [ecx+4]
	mov	[bgr_bmp_header+16h], eax
	mov	eax, [ecx]
	mov	[bgr_bmp_header+12h], eax
	add	eax, eax
	add	eax, [ecx]
	add	eax, 3
	and	al, not 3
	mul	dword [ecx+4]
; eax=size of image
	mov	[bgr_bmp_header+22h], eax
	add	eax, 36h
	mov	[bgr_bmp_header+2], eax
	push	eax
	mov	eax, esp
	push	0
	push	eax
	push	36h
	push	bgr_bmp_header
	push	esi
	call	[WriteFile]
	pop	eax
; bmp data
	mov	eax, [bgr_section]
	mov	ecx, [eax+4]
	lea	edi, [eax+10h]
	mov	eax, [eax]
	mul	ecx
	imul	eax, 3
	add	edi, eax
.putline:
	push	ecx
	mov	ecx, [bgr_section]
	mov	eax, [ecx]
	add	eax, eax
	add	eax, [ecx]
	sub	edi, eax
	push	eax
	push	eax
	mov	ecx, esp
	push	0
	push	ecx
	push	eax
	push	edi
	push	esi
	call	[WriteFile]
	pop	eax
	pop	ecx
	and	ecx, 3
	jz	.nopad
.padloop:
	push	ecx
	push	0
	push	eax
	mov	eax, esp
	push	0
	push	eax
	add	eax, 4
	push	1
	push	eax
	push	esi
	call	[WriteFile]
	pop	eax
	pop	eax
	pop	ecx
	loop	.padloop
.nopad:
	pop	ecx
	loop	.putline
	push	esi
	call	[CloseHandle]
	mov	eax, [shared_data]
	mov	[eax+shared_data_struc.dwNewBgrTime], -1
	;or	[dwNewBgrTime], -1
	push	ebx
	push	win32_path
	push	0
	push	14h
	call	[SystemParametersInfoA]
	cmp	bl, 2
	jnz	@f
	push	eax
	push	esp
	push	500
	push	0
	push	0
	push	0
	push	0x1A
	push	0xFFFF
	call	[SendMessageTimeoutA]
	pop	eax
; restore key values
	push	eax
	push	esp
	push	2	; KEY_SET_VALUE
	push	0
	push	bgrkeyname
	push	80000001h
	call	[RegOpenKeyExA]
	test	eax, eax
	pop	esi
	jnz	@f
	mov	eax, esp
	push	2
	push	eax
	push	1
	push	0
	push	bgrstylevalue
	push	esi
	call	[RegSetValueExA]
	lea	eax, [esp+4]
	push	2
	push	eax
	push	1
	push	0
	push	bgrtilevalue
	push	esi
	call	[RegSetValueExA]
	push	esi
	call	[RegCloseKey]
@@:
	mov	esi, [shared_data]
	call	[GetTickCount]
	add	eax, 3000
	;mov	[dwNewBgrTime], eax
	mov	[esi+shared_data_struc.dwNewBgrTime], eax
	pop	ecx
	pop	ecx
	ret

i40_getbutton:
	movzx	ecx, [ebp+tls.butbuflen]
	jecxz	.empty
	lea	edi, [ebp+tls.butbuffer]
	mov	eax, [edi]
	and	al, not 1
	mov	[esp+20h], eax
	lea	esi, [edi+4]
	dec	ecx
	mov	[ebp+tls.butbuflen], cl
	rep	movsd
	ret
.empty:
	mov	byte [esp+20h], 1
	ret

i40_sys_service:
	cmp	ebx, 1
	jz	.19
	cmp	ebx, 2
	jz	.kill
	cmp	ebx, 18
	jnz	.not2
.kill:
	call	acquire_shared
	xchg	eax, ecx
	cmp	eax, 1
	jbe	release_shared		; invalid PID/slot - ignore
	mov	esi, [shared_data]
	mov	edi, esi
	mov	ecx, [esi]
	add	esi, shared_data_struc.threads
	cmp	ebx, 2
	jz	.slot
@@:
	cmp	[esi], eax
	jz	.found
	add	esi, 64
	loop	@b
	jmp	release_shared		; no such PID - ignore
.slot:
	dec	eax
	cmp	eax, ecx
	jae	release_shared
	shl	eax, 6
	add	esi, eax
	cmp	dword [esi], 0
	jz	release_shared
.found:
; terminate self?
	call	get_cur_slot_ptr
	cmp	esi, edi
	jnz	@f
	call	release_shared
	jmp	i40_terminate
@@:
; get thread and process handles
	mov	eax, [esi+shared_data_struc.win32_hThread-shared_data_struc.threads]
	call	server_convert
	mov	edi, eax
	call	release_shared
; target thread is suspended? if so, do not wait
	push	edi
	call	[SuspendThread]
	test	eax, eax
	jnz	.suspended
	push	edi
	call	[ResumeThread]
; send WM_CLOSE to thread window
	push	0
	push	0
	push	0x10		; WM_CLOSE
	push	[esi+shared_data_struc.hWnd-shared_data_struc.threads]
	call	[PostMessageA]
; wait no more than 1 second
	push	1000
	push	edi
	call	[WaitForSingleObject]
	cmp	eax, 258	; WAIT_TIMEOUT
	jnz	.ret_close
; let target thread to terminate itself
	push	edi
	call	[SuspendThread]
.suspended:
	mov	ebx, [esi+shared_data_struc.win32_stack-shared_data_struc.threads]
	sub	esp, 0xB2*4
	push	1000Fh
	mov	esi, esp
	push	esp
	push	edi
	call	[GetThreadContext]
	mov	ax, ss
	cmp	[esi+0xC8], ax
	jz	@f
	mov	[esi+0xC8], ss
	mov	[esi+0xC4], ebx
@@:
	mov	[esi+0xBC], cs
	mov	[esi+0x98], ds
	mov	[esi+0x94], es
	mov	[esi+0xB8], dword i40_terminate
	push	esi
	push	edi
	call	[SetThreadContext]
	add	esp, 0xB3*4
@@:
	push	edi
	call	[ResumeThread]
	cmp	eax, 1
	ja	@b
.ret_close:
	push	edi
	call	[CloseHandle]
	ret
.not2:
	cmp	ebx, 3
	jnz	.not3
	cmp	ecx, 2
	jb	.noactivate_ret
	call	acquire_shared
	mov	eax, [shared_data]
	cmp	ecx, [eax+shared_data_struc.alloc_threads]
	ja	.noactivate_release
	lea	eax, [ecx-1]
	call	get_slot_ptr
	cmp	dword [edi], 0
	jz	.noactivate_release
	mov	esi, [edi+shared_data_struc.hWnd-shared_data_struc.threads]
	call	release_shared
.restore_given_window:
	sub	esp, 40
	push	44
	push	esp
	push	esi
	call	[GetWindowPlacement]
	mov	eax, [esp+8]
	add	esp, 44
	mov	ecx, 5	; SW_SHOW
	cmp	al, 2	; SW_SHOWMINIMIZED
	jnz	@f
	mov	cl, 9	; SW_RESTORE
@@:
	push	ecx
	push	esi
	call	[ShowWindow]
;	push	esi
;	call	[BringWindowToTop]
	push	esi
	call	[SetForegroundWindow]
	ret
.noactivate_release:
	call	release_shared
.noactivate_ret:
	ret
.not3:
	cmp	ebx, 4
	jnz	.not4
; get idle time - current implementation on NT simply returns cpuspeed
	cmp	[bIs9x], 0
	jz	.5
idletime_via_ring0 = 1
if ~idletime_via_ring0
	mov	eax, [shared_data]
	cmp	[eax+shared_data_struc.b9xPerfInited], 0
	jnz	.perfinited
	call	acquire_shared
	cmp	[eax+shared_data_struc.b9xPerfInited], 0
	jnz	@f
	push	eax
	push	esp	; phkResult
	push	1	; samDesired = KEY_QUERY_VALUE
	push	0	; ulOptions
	push	perfstart	; lpSubKey
	push	80000006h	; hKey = HKEY_DYN_DATA
	call	[RegOpenKeyExA]
	pop	esi
	test	eax, eax
	jnz	.perfinitfail
	push	eax
	mov	eax, esp
	push	4
	push	esp	; lpcbData
	push	eax	; lpData
	push	0	; lpType
	push	0	; lpReserved
	push	perfval	; lpValueName
	push	esi	; hKey
	call	[RegQueryValueExA]
	pop	ecx
	pop	ecx
	push	eax
	push	esi
	call	[RegCloseKey]
	pop	eax
	test	eax, eax
	jnz	.perfinitfail
	push	200
	call	[Sleep]		; give chance to collect data
@@:
	mov	eax, [shared_data]
	mov	[eax+shared_data_struc.b9xPerfInited], 1
	call	release_shared
.perfinited:
	push	100
	pop	ebx
	push	eax
	push	esp	; phkResult
	push	1	; samDesired = KEY_QUERY_VALUE
	push	0	; ulOptions
	push	perfget	; lpSubKey
	push	80000006h	; hKey = HKEY_DYN_DATA
	call	[RegOpenKeyExA]
	pop	esi
	test	eax, eax
	jnz	@f
	push	ebx
	mov	eax, esp
	push	4
	push	esp	; lpcbData
	push	eax	; lpData
	push	0	; lpType
	push	0	; lpReserved
	push	perfval	; lpValueName
	push	esi	; hKey
	call	[RegQueryValueExA]
	pop	ecx
	pop	ebx
	push	esi
	call	[RegCloseKey]
@@:
	pushad
	call	.5
	popad
	push	100
	pop	ecx
	sub	ebx, ecx
	neg	ebx
	mul	ebx
	div	ecx
	mov	[esp+20h], eax
	ret
.perfinitfail:
	call	release_shared
	push	40h
	push	0
	push	aPerfInitFailed
	push	0
	call	[MessageBoxA]
	jmp	.5
else
	mov	eax, [shared_data]
	cmp	[eax+shared_data_struc.b9xPerfInited], 0
	jnz	@f
.idlecount_init:
	div	edx
@@:
	mov	ebx, [eax+shared_data_struc.idlecount]
	pushad
	call	.5
	popad
	mul	ebx
	mov	ecx, 1000
	div	ecx
	mov	[esp+20h], eax
	ret
end if
.not4:
	cmp	ebx, 5
	jnz	.not5
.5:
	mov	eax, [shared_data]
	mov	ebx, [eax+shared_data_struc.cpuspeed]
	test	ebx, ebx
	jnz	@f
; determine cpu speed
; 1) check for value in registry
	push	eax
	push	esp
	push	1	; KEY_QUERY_VALUE
	push	0
	push	keycpu
	push	80000002h
	call	[RegOpenKeyExA]
	test	eax, eax
	pop	eax
	jnz	.nokey
	xchg	eax, esi
	push	eax
	push	4
	push	esp
	lea	eax, [esp+8]
	push	eax
	push	0
	push	0
	push	keymhz
	push	esi
	call	[RegQueryValueExA]
	push	eax
	push	esi
	call	[RegCloseKey]
	pop	eax
	test	eax, eax
	pop	eax
	pop	eax
	jnz	.nokey
	imul	eax, 1000000
	jmp	.speed_found
.nokey:
; 2) sleep for 1/4 sec and read TSC
	rdtsc
	push	eax edx
	push	250
	call	[Sleep]
	rdtsc
	pop	ecx ebx
	sub	eax, ebx
	sbb	edx, ecx
	shl	eax, 2
.speed_found:
	xchg	ebx, eax
	mov	eax, [shared_data]
	mov	[eax+shared_data_struc.cpuspeed], ebx
@@:
	mov	[esp+20h], ebx
	ret
.not5:
	cmp	ebx, 7
	jnz	.not7
	mov	eax, [shared_data]
	mov	eax, [eax+shared_data_struc.active_process]
	mov	[esp+20h], eax
	ret
.not7:
	cmp	ebx, 8
	jnz	.not8
	dec	ecx
	jnz	@f
	mov	eax, [shared_data]
	movzx	eax, [eax+shared_data_struc.sound_flag]
	mov	[esp+20h], eax
	ret
@@:
	dec	ecx
	jnz	not_supported_i40_fn
	mov	eax, [shared_data]
lock	xor	[eax+shared_data_struc.sound_flag], 1
	ret
.not8:
	cmp	ebx, 9
	jnz	.not9
.19:
	mov	eax, [shared_data]
	cmp	[eax+shared_data_struc.vk], 0
	jz	not_supported_i40_fn
.server_terminate:
	div	edx
	ret
.not9:
	cmp	ebx, 10
;	jnz	.not10
;	call	minimize_window
;	ret
	jz	minimize_window
.not10:
	cmp	ebx, 11
	jnz	.not11
	dec	ecx
	jnz	not_supported_i40_fn
	xor	eax, eax
	mov	edi, edx
	add	edi, [base]
; floppy is not supported yet => return "no floppy installed"
	stosb
; IDEx configuration
; CD is not supported yet. On IDEx there may be only HD (or nothing)
	xor	edx, edx
	mov	esi, hd_partitions_num
	push	esi
.idex:
	lodsd
	shl     edx, 2
	test	eax, eax
	jz	@f
	inc	edx
@@:
	cmp	esi, hd_partitions_num+4*4
	jc	.idex
	mov	[edi], dl
	inc	edi
; number of partitions
	pop	esi
	mov	cl, 4
@@:
	lodsd
	stosb
	loop	@b
; reserved bytes
        and     dword [edi], 0
	ret
.not11:
	cmp	ebx, 12
	jnz	.not12
.return0:
	and	dword [esp+20h], 0
	ret
.not12:
	cmp	ebx, 13
	jnz	.not13
	mov	edi, ecx
	add	edi, [base]
	mov	esi, version_inf
	mov	ecx, version_end - version_inf
	rep	movsb
	ret
.not13:
        cmp     ebx, 14
        jz      .return0
	cmp	ebx, 15
	jnz	.not15
	call	get_screen_size
	movzx	eax, bx
	inc	eax
	shr	eax, 1
	push	eax
	shr	ebx, 16
	inc	ebx
	shr	ebx, 1
	push	ebx
	call	[SetCursorPos]
	ret
.not15:
        cmp     ebx, 16
        jnz     .not16
        sub     esp, 20h
        push    esp
        call    [GlobalMemoryStatus]
        mov     eax, [esp+12]
        add     esp, 20h
        shr     eax, 10
        mov     [esp+20h], eax
        ret
.not16:
        cmp     ebx, 17
        jnz     .not17
        sub     esp, 20h
        push    esp
        call    [GlobalMemoryStatus]
        mov     eax, [esp+8]
        add     esp, 20h
        shr     eax, 10
        mov     [esp+20h], eax
        ret
.not17:
        cmp     ebx, 19
        jnz     .not19
        jecxz   .19_0
        dec     ecx
        jz      .19_1
        dec     ecx
        jz      .19_2
        dec     ecx
        jz      .19_3
        dec     ecx
        jnz     not_supported_i40_fn
        movzx   eax, dx
        push    eax
        shr     edx, 16
        push    edx
        call    [SetCursorPos]
        ret
.19_0:
        mov     eax, [shared_data]
        movzx   eax, [eax+shared_data_struc.mouse_speed_factor]
        mov     [esp+20h], eax
        ret
.19_1:
        mov     eax, [shared_data]
        mov     [eax+shared_data_struc.mouse_speed_factor], dx
        ret
.19_2:
        mov     eax, [shared_data]
        mov     eax, [eax+shared_data_struc.mouse_delay]
        mov     [esp+20h], eax
        ret
.19_3:
        mov     eax, [shared_data]
        mov     [eax+shared_data_struc.mouse_delay], edx
        ret
.not19:
	cmp	ebx, 21
	jnz	.not21
	xchg	eax, ecx
	call	get_slot_num
	mov	[esp+20h], edx
	ret
.not21:
	cmp	ebx, 22
	jnz	not_supported_i40_fn
	cmp	ecx, 4
	jae	not_supported_i40_fn
	shr	ecx, 1
	jnc	@f
	xchg	eax, edx
	call	get_slot_num
@@:
	dec	edx
	jz	.22err
	mov	esi, [shared_data]
	cmp	edx, [esi]
	jae	.22err
	shl	edx, 6
	mov	esi, [esi+edx+shared_data_struc.hWnd]
	and	dword [esp+20h], 0
	dec	ecx
	js	minimize_given_window
	jmp	.restore_given_window
.22err:
	or	dword [esp+20h], -1
	ret

get_slot_num:
	push	ecx
	mov	esi, [shared_data]
	mov	ecx, [esi]
	add	esi, shared_data_struc.threads
	xor	edx, edx
@@:
	inc	edx
	cmp	[esi], eax
	jz	@f
	add	esi, 64
	loop	@b
	xor	edx, edx
@@:
	pop	ecx
	ret

minimize_window:
	mov	esi, [ebp+tls.hWnd]
minimize_given_window:
	mov	[ebp+tls.butbuflen], 0
	xor	eax, eax	; SW_HIDE
	mov	ecx, [shared_data]
	cmp	[ecx+shared_data_struc.vk], 0
	jnz	@f
	mov	al, 6		; SW_MINIMIZE
@@:
	push	eax
	push	esi
	call	[ShowWindow]
	ret

server_convert:
	div	edx
	ret

i40_sys_setup:
	mov	eax, [shared_data]
	and	dword [esp+20h], 0
	dec	ebx
	jnz	.nomidi
	cmp	ecx, 100h
	jb	.nomidi
	cmp	ecx, 0xFFFF
	ja	.nomidi
	mov	[eax+shared_data_struc.midi_base], cx
	ret
.nomidi:
	dec	ebx
	jnz	.not2
	mov	edi, keymap
	dec	ecx
	jz	.settable
	mov	edi, keymap_shift
	dec	ecx
	jz	.settable
	mov	edi, keymap_alt
	dec	ecx
	jnz	.nosettable
.settable:
	add	edx, [base]
	mov	esi, edx
	mov	ecx, 80h/4
	rep	movsd
	ret
.nosettable:
	cmp	ecx, 9-3
	jnz	.nosetkeyboard
	mov	[eax+shared_data_struc.keyboard], dx
	ret
.nosetkeyboard:
	inc	dword [esp+20h]
	ret
.not2:
	dec	ebx
	jnz	.not3
	mov	[eax+shared_data_struc.cd_base], cl
	ret
.not3:
	dec	ebx
	jnz	.not4
	cmp	ecx, 0x100
	jb	.not4
	cmp	ecx, 0xFFFF
	ja	.not4
	mov	word [eax+shared_data_struc.sb16], cx
	ret
.not4:
	dec	ebx
	jnz	.not5
	mov	[eax+shared_data_struc.syslang], ecx
	ret
.not5:
	dec	ebx
	jnz	.not6
	cmp	ecx, 0x100
	jb	.not6
	mov	[eax+shared_data_struc.wss], ecx
	ret
.not6:
	dec	ebx
	jnz	.not7
	mov	[eax+shared_data_struc.hd_base], cl
	ret
.not7:
	dec	ebx
	jnz	.not8
	mov	[eax+shared_data_struc.fat32part], ecx
	ret
.not8:
	dec	ebx
	dec	ebx
	jnz	.not10
	mov	[eax+shared_data_struc.sound_dma], ecx
	ret
.not10:
	dec	ebx
	jnz	.not11
	and	ecx, 1
	mov	[eax+shared_data_struc.lba_read_enabled], ecx
	ret
.not11:
	dec	ebx
	jnz	.not12
	and	ecx, 1
	jz	.okcheck11
	cmp	[eax+shared_data_struc.bAllowReadPCI], 0
	jnz	.okcheck11
; be silent, because in loading process (of VirtualKolibri) SETUP will try this anyway
;	push	40h
;	push	0
;	push	aPciDisabled
;	push	0
;	call	[MessageBoxA]
	ret
.okcheck11:
	mov	[eax+shared_data_struc.pci_access_enabled], ecx
	ret
.not12:
	mov	byte [esp+20h], 21	; restore eax
	jmp	not_supported_i40_fn

i40_wait_event_timeout:
;        call    event_test_redraw
;	imul	ebx, ebx, 10
	sub	esp, 20h
	mov	esi, esp
	jmp	.M
.L:
	push	2
	pop	eax
	test	byte [ebp+tls.message_mask], al
	jz	.nokey
	cmp	[ebp+tls.keybuflen], ah
	jnz	.event
.nokey:
	inc	eax
	test	[ebp+tls.message_mask], 4
	jz	.nobut
	cmp	[ebp+tls.butbuflen], ah
	jnz	.event
.nobut:
	push	1	; PM_REMOVE
	push	0
	push	0
	push	0
	push	esi
	call	[PeekMessageA]
	test	eax, eax
	jz	.notfound
	cmp	dword [esi+4], 0x12	; WM_QUIT
	jz	i40_terminate
	push	esi
	call	[TranslateMessage]
	push	esi
	call	[DispatchMessageA]
.M:
	xor	eax, eax
	cmp	[ebp+tls.curdraw], al
	jz	@f
	test	[ebp+tls.message_mask], 1
	jz	@f
	inc	eax
	add	esp, 20h
	mov	dword [esp+20h], eax
	ret
@@:
	xchg	al, [ebp+tls.translated_msg_code]
	test	eax, eax
	jz	.L
	cmp	al, 6
	jnz	.event
	test	[ebp+tls.message_mask], 20h
	jz	.L
.event:
	add	esp, 20h
	call	test_button_mouse
	mov	[esp+20h], eax
	ret
.notfound:
	push	10
	call	[Sleep]
	dec	ebx
	jnz	.L
.timeout:
	add	esp, 20h
	and	dword [esp+20h], 0
	ret

i40_getsetup:
	mov	eax, [shared_data]
	dec	ebx
	jnz	.not_midi_base
	movzx	eax, [eax+shared_data_struc.midi_base]
	mov	[esp+20h], eax
	ret
.not_midi_base:
	dec	ebx
	jnz	.not_keyboard
	mov	esi, keymap
	dec	ecx
	jz	.getmap
	mov	esi, keymap_shift
	dec	ecx
	jz	.getmap
	mov	esi, keymap_alt
	dec	ecx
	jnz	.nobase
.getmap:
	mov	ecx, 128/4
	mov	edi, edx
	add	edi, [base]
	rep	movsd
	ret
.nobase:
	cmp	ecx, 9-3
	jnz	not_supported_i40_fn
	movzx	eax, [eax+shared_data_struc.keyboard]
	mov	[esp+20h], eax
	ret
.not_keyboard:
	dec	ebx
	jnz	.not_cd_base
	movzx	eax, [eax+shared_data_struc.cd_base]
	mov	[esp+20h], eax
	ret
.not_cd_base:
	dec	ebx
	jnz	.not_sb16
	mov	eax, [eax+shared_data_struc.sb16]
	mov	[esp+20h], eax
	ret
.not_sb16:
	dec	ebx
	jnz	.not_lang
	mov	eax, [eax+shared_data_struc.syslang]
	mov	[esp+20h], eax
	ret
.not_lang:
	dec	ebx
	jnz	.not_wss
	mov	eax, [eax+shared_data_struc.wss]
	mov	[esp+20h], eax
	ret
.not_wss:
	dec	ebx
	jnz	.not_hdbase
	movzx	eax, [eax+shared_data_struc.hd_base]
	mov	[esp+20h], eax
	ret
.not_hdbase:
	dec	ebx
	jnz	.not_fat32part
	mov	eax, [eax+shared_data_struc.fat32part]
	mov	[esp+20h], eax
	ret
.not_fat32part:
	dec	ebx
	jnz	.not_timer_ticks
	call	[GetTickCount]
	push	10
	pop	ecx
	xor	edx, edx
	div	ecx
	mov	[esp+20h], eax
	ret
.not_timer_ticks:
	dec	ebx
	jnz	.not_sound_dma
	mov	eax, [eax+shared_data_struc.sound_dma]
	mov	[esp+20h], eax
	ret
.not_sound_dma:
	dec	ebx
	jnz	.not_lba_enabled
	mov	eax, [eax+shared_data_struc.lba_read_enabled]
	mov	[esp+20h], eax
	ret
.not_lba_enabled:
	dec	ebx
	jnz	.not_pci_enabled
	mov	eax, [eax+shared_data_struc.pci_access_enabled]
	mov	[esp+20h], eax
	ret
.not_pci_enabled:
	mov	dword [esp+20h], 1	; this is kernel rule, not my
	ret

i40_get_sys_date:
	sub	esp, 10h
	push	esp
	call	[GetLocalTime]
	movzx	eax, word [esp+6]
	aam
	shl	ax, 4
	shr	ah, 4
	shl	eax, 16-4
	mov	ax, [esp+2]
	aam
	shl	ax, 4
	shr	ah, 4
	shr	ax, 4
	mov	bl, al
	mov	ax, [esp]
	sub	ax, 2000
	aam
	shl	ax, 4
	shr	ah, 4
	shr	ax, 4
	mov	ah, bl
	add	esp, 10h
	mov	[esp+20h], eax
	ret

i40_current_folder:
	add	ecx, [base]
	dec	ebx
	jz	.set
	dec	ebx
	jnz	not_supported_i40_fn
	mov	edi, ecx
	mov	esi, [ebp+tls.cur_dir]
	push	edx
	push	esi
	call	[lstrlenA]
	pop	ecx
	inc	eax
	cmp	ecx, eax
	jb	@f
	mov	ecx, eax
@@:
	rep	movsb
	mov	byte [edi-1], 0
	mov	[esp+20h], eax
	ret
.set:
	mov	esi, ecx
	cmp	byte [ecx], '/'
	jnz	.relative
	push	[ebp+tls.cur_dir]
	call	free
	push	esi
	call	[lstrlenA]
	inc	eax	; (for terminating zero)
	inc	eax	; '/sys' -> '/rd/1' requires one additional byte
	push	eax
	push	eax
	call	malloc
	mov	[ebp+tls.cur_dir], eax
	mov	edi, eax
	pop	ecx
	mov	eax, [esi]
	or	eax, '    '
	cmp	eax, '/sys'
	jnz	@f
	mov	eax, '/rd/'
	stosd
	mov	al, '1'
	stosb
	sub	ecx, 4
	add	esi, 4
@@:
	rep	movsb
	ret
.relative:
	push	[ebp+tls.cur_dir]
	call	[lstrlenA]
	push	eax
	push	esi
	call	[lstrlenA]
	mov	edi, eax
	inc	eax
	add	[esp], eax
	push	[ebp+tls.cur_dir]
	push	0
	push	[hHeap]
	call	[HeapReAlloc]
	mov	[ebp+tls.cur_dir], eax
	add	edi, eax
.l1:
	cmp	word [esi], '.'
	jz	.ret
	cmp	word [esi], './'
	jnz	@f
	inc	esi
	inc	esi
	jmp	.l1
@@:
	cmp	word [esi], '..'
	jnz	.copy
	cmp	byte [esi+2], 0
	jz	@f
	cmp	byte [esi+2], '/'
	jnz	.copy
@@:
	dec	edi
	cmp	edi, [ebp+tls.cur_dir]
	jbe	@f
	cmp	byte [edi], '/'
	jnz	@b
@@:
	inc	esi
	inc	esi
	cmp	byte [esi], 0
	jz	.ret
	inc	esi
	jmp	.l1
.copy:
	lodsb
	stosb
	test	al, al
	jnz	.copy
	ret
.ret:
	mov	byte [edi], 0
	ret

i40_delete_ramdisk_file:
	sub	esp, 512
	mov	edi, esp
	call	ramdisk2win32
	push	edi
	call	[DeleteFileA]
	add	esp, 512
	ret

i40_write_ramdisk_file:
	test	esi, esi
	jnz	not_supported_i40_fn
	sub	esp, 512
	mov	edi, esp
	push	ecx edx
	call	ramdisk2win32
	push	0
	push	80h
	push	2
	push	0
	push	0
	push	40000000h
	push	edi
	call	[CreateFileA]
	inc	eax
	jz	.ret
	dec	eax
	xchg	eax, esi
	pop	edx ecx
	add	ecx, [base]
	push	eax
	mov	eax, esp
	push	0
	push	eax
	push	edx
	push	ecx
	push	esi
	call	[WriteFile]
	pop	eax
	push	esi
	call	[CloseHandle]
.ret:
	add	esp, 512
	and	dword [esp+20h], 0
	ret

i40_screen_getpixel:
	push	0
	call	[GetDC]
	xchg	eax, esi
	push	8
	push	esi
	call	[GetDeviceCaps]
	xchg	eax, ebx
	xor	edx, edx
	div	ebx
	push	eax
	push	edx
	push	esi
	call	[GetPixel]
	push	eax
	push	esi
	push	0
	call	[ReleaseDC]
	pop	eax
	call	convert_color
	mov	[esp+20h], eax
	ret

i40_screen_getarea:
	add	ebx, [base]
	push	ecx ebx edx
	mov	edi, ecx
	movsx	ebx, cx
	sar	edi, 16
	cmp	ebx, 0
	jle	.nodata
	cmp	edi, 0
	jg	.okdata
.nodata:
	pop	edx ebx ecx
	ret
.okdata:
	xor	eax, eax
	push	eax
	call	[GetDC]
	push	eax
	push	eax
	call	[CreateCompatibleDC]
	xchg	eax, esi
	push	ebx
	push	edi
	push	dword [esp+8]
	call	[CreateCompatibleBitmap]
	push	eax
	push	esi
	call	[SelectObject]
	push	eax
	movzx	eax, word [esp+8]
	movzx	ecx, word [esp+8+2]
	push	0xCC0020
	push	eax
	push	ecx
	push	dword [esp+16]
	push	ebx
	push	edi
	push	0
	push	0
	push	esi
	call	[BitBlt]
	push	esi
	call	[SelectObject]
	push	ebp
	xchg	eax, ebp
; esi=hDC, ebp=hBitmap, ebx=height, edi=width
	xor	eax, eax
	push	eax	; biClrImportant
	push	eax	; biClrUsed
	push	eax	; biYPelsPerMeter
	push	eax	; biXPelsPerMeter
	push	eax	; biSizeImage
	push	eax	; biCompression
	push	180001h	; biBitCount, biPlanes
	push	ebx	; biHeight
	neg	dword [esp]
	push	edi	; biWidth
	push	40	; biSize
	mov	ecx, esp
	push	eax	; uUsage
	push	ecx	; lpbi
	mov	eax, [ecx+52]
	test	edi, 3
	jz	.width_aligned_1
	lea	eax, [3*edi+3]
	and	al, not 3
	mul	ebx
	call	malloc_big
.width_aligned_1:
	push	eax	; lpvBits
	push	ebx	; cScanLines
	push	0	; uStartScan
	push	ebp	; hbmp
	push	esi	; hdc
	xchg	eax, edi
	call	[GetDIBits]
	add	esp, 40
	push	ebp
	call	[DeleteObject]
	pop	ebp
	push	esi
	call	[DeleteDC]
	push	0
	call	[ReleaseDC]
	pop	edx eax edx
	cmp	eax, edi
	jz	.ret
; edi -> bits from Windows [dword-aligned scanlines], eax -> bits for Kolibri [no spaces],
	shr	edx, 16
; ebx = height, edx = width
	push	8000h
	push	0
	push	edi
	xchg	eax, edi
	lea	edx, [edx*3]
	xchg	eax, esi
@@:
	mov	ecx, edx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	add	esi, 3
	and	esi, not 3
	sub	ebx, 1
	jnz	@b
	call	[VirtualFree]
.ret:
	ret

i40_read_mouse_pos:
	dec	ebx
	js	.get_coord
	jz	.get_coord
	dec	ebx
	jnz	.ifcursor
	cmp	[bIs9x], 0
	jnz	read_9x_mouse_buttons
	xor	ebx, ebx
	push	6	; VK_XBUTTON2
	call	.aks
	push	5	; VK_XBUTTON1
	call	.aks
	push	4	; VK_MBUTTON
	call	.aks
	push	2	; VK_RBUTTON
	call	.aks
	push	1	; VK_LBUTTON
	call	.aks
	mov	[esp+20h], ebx
	ret
.get_coord:
	push	eax
	push	eax
	push	esp
	call	[GetCursorPos]
	pop	eax	; x
	pop	ecx	; y
	test	ebx, ebx
	js	@f
	sub     ax, [ebp+tls.x_start]
	sub	ax, word [ebp+tls.client_left]
	sub     cx, [ebp+tls.y_start]
	sub	cx, word [ebp+tls.client_top]
@@:
	shl	eax, 16
	add	eax, ecx
	mov	[esp+20h], eax
	ret
.ifcursor:
        dec     ebx
        dec     ebx
        jz      .load_cursor
        dec     ebx
        jz      .set_cursor
        dec     ebx
        jnz     .not_cursor
.delete_cursor:
        jecxz   .delete_invalid
        call    acquire_shared
        mov     eax, ecx
        mov     ecx, num_cursors
        mov     edi, [shared_data]
        add     edi, shared_data_struc.cursors
@@:
        add     edi, 8
        cmp     [edi-8], eax
        loopnz  @b
        jnz     .delete_invalid_release
        mov     ecx, [ebp+tls.cur_slot]
        cmp     [edi-4], ecx
        jnz     .delete_invalid_release
        and     dword [edi-8], 0
        and     dword [edi-4], 0
        call    release_shared
        push    eax
        call    [DestroyCursor]
; the current kernel implementation returns garbage !
        mov     dword [esp+20h], 0xBAADF00D     ; is it garbage? :-)
        ret
.delete_invalid_release:
        call    release_shared
.delete_invalid:
        push    10h
        push    0
        push    aInvalidCursor
        push    0
        call    [MessageBoxA]
; the current kernel implementation returns 6 !?
        mov     dword [esp+20h], 6
        ret
.set_cursor:
; set_cursor for invalid handle reverts to standard arrow
        call    acquire_shared
        mov     eax, ecx
        jecxz   .setarrow
        mov     ecx, num_cursors
        mov     edi, [shared_data]
        add     edi, shared_data_struc.cursors
@@:
        add     edi, 8
        cmp     [edi-8], eax
        loopnz  @b
        jz      @f
.setarrow:
        mov     eax, [hArrow]
@@:
; N.B. I don't check process field, because the kernel allows to set
;       cursors, loaded by another process
        call    release_shared
        mov     [ebp+tls.hCursor], eax
        push    eax
        call    [SetCursor]
        mov     [esp+20h], eax
        ret
.load_cursor:
        test    dx, dx
        jz      .fromfile
        dec     dx
        jz      .frommem
        dec     dx
        jz      .indirect
        jmp     not_supported_i40_fn
.fromfile:
        mov     esi, ecx
        add     esi, [base]
        sub     esp, 204h
        mov     edi, esp
        call    i40_file_system_lfn.parse
        test    eax, eax
        jz      @f
        add     esp, 204h
        and     dword [esp+20h], 0
        ret
@@:
        push    10h     ; LR_LOADFROMFILE
        push    32
        push    32
        push    2       ; IMAGE_CURSOR
        push    edi
        push    0
        call    [LoadImageA]
        add     esp, 204h
        test    eax, eax
        jnz     .addcursor
        push    10h
        push    0
        push    aCursorFailed
        jmp     .mberr
.addcursor:
        call    acquire_shared
        mov     ecx, num_cursors
        mov     edi, [shared_data]
        add     edi, shared_data_struc.cursors
@@:
        add     edi, 8
        cmp     dword [edi-8], 0
        loopnz  @b
        jz      @f
        call    release_shared
        push    10h
        push    0
        push    aCursorLimitExceeded
        jmp     .mberr
@@:
        mov     [edi-8], eax
        mov     edx, [ebp+tls.cur_slot]
        mov     [edi-4], edx
        call    release_shared
        mov     [esp+20h], eax
        ret
.frommem:
        add     ecx, [base]
        cmp     dword [ecx], 0x020000
        jz      @f
        push    10h
        push    0
        push    aInvCursorData
.mberr:
        push    0
        call    [MessageBoxA]
.reterr:
        and     dword [esp+20h], 0
        ret
@@:
        cmp     word [ecx+4], 1
        jz      @f
        push    10h
        push    0
        push    aOnlyOneCursor
        jmp     .mberr
@@:
        cmp     word [ecx+6], 0x2020
        jz      @f
        push    10h
        push    0
        push    aInvCursorDim
        jmp     .mberr
@@:
        mov     eax, [ecx+0Ah]
        add     ecx, [ecx+12h]
        sub     ecx, 4
        push    dword [ecx]
        mov     [ecx], eax
        push    ecx
        push    0
        push    32
        push    32
        push    0x00030000
        push    0
        push    0x2E8+4
        push    ecx
        call    [CreateIconFromResourceEx]
        pop     ecx
        pop     dword [ecx]
        test    eax, eax
        jnz     .addcursor
.gen_failed:
        push    10h
        push    0
        push    aCursorFailed
        jmp     .mberr
.indirect:
        mov     ebx, edx
        shr     ebx, 16         ; bl = y, bh = x
        mov     esi, ecx
        add     esi, [base]
        push    32*32*4
	call	malloc
        mov     edi, eax
        mov     ecx, 32*32
        push    esi edi
@@:
        lodsd
        and     eax, 0xFFFFFF
        stosd
        loop    @b
        pop     edi esi
        push    edi     ; lpvBits
        push    32      ; cBitsPerPel
        push    1       ; cPlanes
        push    32      ; nHeight
        push    32      ; nWidth
        call    [CreateBitmap]
        test    eax, eax
        jnz     @f
.free_edi:
        push    edi
	call	free
        jmp     .gen_failed
@@:
        push    eax     ; ICONINFO.hbmColor
        mov     ecx, 32*4
        push    esi edi
.1:
        push    ecx
        mov     cl, 8
        xor     edx, edx
@@:
        lodsd
        shr     eax, 24
        setz    al
        lea     edx, [edx*2+eax]
        loop    @b
        mov     al, dl
        stosb
        pop     ecx
        loop    .1
        pop     edi esi
        push    edi     ; lpvBits
        push    1       ; cBitsPerPel
        push    1       ; cPlanes
        push    32      ; nHeight
        push    32      ; nWidth
        call    [CreateBitmap]
        test    eax, eax
        jnz     @f
        call    [DeleteObject]
        jmp     .free_edi
@@:
        push    eax     ; ICONINFO.hbmMask
        movzx   eax, bl
        push    eax     ; ICONINFO.yHotSpot
        movzx   eax, bh
        push    eax     ; ICONINFO.xHotSpot
        push    0       ; ICONINFO.fIcon
        push    esp
        call    [CreateIconIndirect]
        mov     ebx, eax
        add     esp, 12
        call    [DeleteObject]
        call    [DeleteObject]
        test    ebx, ebx
        jz      .free_edi
        push    edi
	call	free
        mov     eax, ebx
        jmp     .addcursor

.not_cursor:
	dec	ebx
	jnz	not_supported_i40_fn
	mov	eax, [ebp+tls.scroll]
	cdq
	mov	ecx, 120	; WHEEL_DELTA
	idiv	ecx
	mov	[ebp+tls.scroll], edx
	movzx	eax, ax
	mov	[esp+20h], eax
	ret

.aks:
	push	dword [esp+4]
	call	[GetAsyncKeyState]
	cmp	ah, 80h
	cmc
	adc	ebx, ebx
	ret	4

read_9x_mouse_buttons:
	call	[jmp_temp_int33]
	movzx	eax, bl
	mov	[esp+20h], eax
	ret

use16
temp_code:
	mov	eax, esp
	mov	ss, [temp_ss]
	mov	sp, temp_stack_size
	push	eax

	mov	ax, 3
	int	33h

	pop	eax
	push	ds
	pop	ss
	mov	esp, eax
	db	66h
	retf

temp_code_int1A:
	mov	esi, esp
	mov	ss, [temp_ss]
	mov	sp, temp_stack_size
	push	esi

	int	1Ah

	pop	esi
	push	ds
	pop	ss
	mov	esp, esi
	db	66h
	retf

temp_code_size = $ - temp_code
use32

i40_draw_line:
        push    edx
	mov	esi, ecx
	mov	eax, edx
	call	convert_color
	push	eax
	push	eax
	push	1
	push	0
	call	[CreatePen]
	push	eax
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, edi
	push	edi
	call	[SelectObject]
	push	eax
	push    0
	test    byte [esp+12+3], 0x01
	jz      @f
	push    6       ; R2_NOT
	push    edi
	call    [SetROP2]
	mov     [esp], eax
@@:
	push	0
	mov	eax, esi
	shr	eax, 16
	add     eax, [ebp+tls.client_top]
	push	eax
	mov	eax, ebx
	shr	eax, 16
	add     eax, [ebp+tls.client_left]
	push	eax
	push	edi
	call	[MoveToEx]
	movzx	esi, si
	add     esi, [ebp+tls.client_top]
	push	esi
	movzx	ebx, bx
	add     ebx, [ebp+tls.client_left]
	push	ebx
	push	edi
	call	[LineTo]
	pop     eax
	test    byte [esp+8+3], 0x01
	jz      @f
	push    eax
	push    edi
	call    [SetROP2]
	push    esi
	push    ebx
	push    edi
	call    [GetPixel]
	xor     eax, 0xFFFFFF
	mov     [esp+4], eax
@@:
	pop	eax
	pop	ecx
	push	eax
	push	ecx
	push	esi
	push	ebx
	push	edi
	call	[SetPixel]
	push	edi
	call	[SelectObject]
	push	eax
	call	[DeleteObject]
	push	edi
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	pop     edx
	ret

i40_get_background:
	pushad
	push	0
	call	init_background
	popad
	mov	eax, [bgr_section]
	dec	ebx
	jz	.1
	dec	ebx
	jz	.2
	dec	ebx
	dec	ebx
	jnz	not_supported_i40_fn
	test	eax, eax
	mov	ecx, 2
	jz	@f
	mov	ecx, [eax+8]
@@:
	mov	[esp+20h], ecx
	ret
.1:
	test	eax, eax
	jz	@f
	mov	ecx, [eax]
	shl	ecx, 16
	mov	cx, [eax+4]
	jmp	@b
@@:
	call	get_screen_size
	add	ebx, 10001h
@@:
	mov	[esp+20h], ebx
.ret:
	ret
.2:
;	cmp	ecx, 0x160000-16
;	jae	.ret
	xor	ebx, ebx
	test	eax, eax
	jz	@b
	mov	ebx, [eax+ecx+10h]
	and	ebx, 0x00FFFFFF
	jmp	@b

i40_set_event_mask:
	test	ebx, not 1F7h
	jnz	not_supported_i40_fn
	mov	[ebp+tls.message_mask], ebx
	ret

i40_reserve_free_ports:
	cmp	ebx, 1
	ja	not_supported_i40_fn
	cmp	edx, 0xFFFF
	ja	.inv_range
	cmp	ecx, edx
	jbe	.range_ok
.inv_range:
	push	30h
	push	aWarning
	push	PortsRangeErr
	push	0
	call	[MessageBoxA]
	mov	dword [esp+20h], 1
	ret
.range_ok:
	call	acquire_shared
	mov	esi, [shared_data]
	mov	eax, ecx
	test	ebx, ebx
	jnz	.free
; reserve ports
@@:
	mov	edi, PortsNotEnabledErr
	bt	dword [esi+shared_data_struc.DisabledPorts], eax
	jc	.err_release
	mov	edi, PortsUsedErr
	bt	dword [esi+shared_data_struc.UsedIoMap], eax
	jnc	.err_release
	inc	eax
	cmp	eax, edx
	jbe	@b
	mov	eax, ecx
@@:
	btr	dword [esi+shared_data_struc.UsedIoMap], eax
	inc	eax
	cmp	eax, edx
	jbe	@b
	jmp	.update_iomap
.free:
	mov	edi, PortsNotUsedErr
	bt	dword [esi+shared_data_struc.UsedIoMap], eax
	jc	.err_release
	inc	eax
	cmp	eax, edx
	jbe	.free
	mov	eax, ecx
@@:
	bts	dword [esi+shared_data_struc.UsedIoMap], eax
	inc	eax
	cmp	eax, edx
	jbe	@b
.update_iomap:
	cmp	[bIs9x], 0
	jz	.nt
	call	release_shared
	and	dword [esp+20h], 0
	ret
.nt:
	mov	eax, [shared_data]
	push	0
	push	0
	add	eax, shared_data_struc.UsedIoMap
	push	2000h
	push	eax
	push	0x22203C
	call	send_driver_request
	mov	edi, DrvOpenErr
	jnz	@f
.err_release:
	call	release_shared
	push	30h
	push	aWarning
	push	edi
	push	0
	call	[MessageBoxA]
	mov	dword [esp+20h], 1
	ret
@@:
	call	release_shared
;	push	0
;	call	[Sleep]		; force task switch
	and	dword [esp+20h], 0
	ret

i40_display_number:
	cmp	bl, 1
	ja	not_supported_i40_fn
	push	ebx esi
	and	ebx, not 0xC0000000
	xor	esi, esi
	cmp	bl, 1
	jb	.noptr
	mov	eax, [base]
	test	byte [esp+7], 0x40
	jz	@f
	mov	esi, [eax+ecx+4]
@@:
	mov	ecx, [eax+ecx]
.noptr:
	mov	eax, 10
	cmp	bh, 1
	jb	@f
	mov	eax, 16
	jz	@f
	mov	eax, 2
@@:
	shr	ebx, 16
	cmp	ebx, 64
	jbe	@f
	pop	esi ebx
	jmp	not_supported_i40_fn
@@:
	push    edi
	mov	edi, esp
	sub	esp, 64
	dec	edi
	std
	push	edx
	push	ebx
	xchg	eax, ecx
	test	ebx, ebx
	jz	.done
.digit:
	xor	edx, edx
	test	esi, esi
	jz	.dig0
	push	eax
	mov	eax, esi
	div	ecx
	pop	esi
	xchg	eax, esi
.dig0:
	div	ecx
	xchg	eax, edx
	cmp	al, 10
	sbb	al, 69h
	das
	stosb
	xchg	eax, edx
	dec	ebx
	jnz	.digit
.done:
	cld
	mov	edx, edi
	inc	edx
	pop	esi
	pop	ebx
	lea	edi, [esp+40h]
	test	byte [esp+48h+3], 80h
	jz	.okleadzero
@@:
	cmp	byte [edx], '0'
	jnz	.okleadzero
	inc	edx
	dec	esi
	cmp	edx, edi
	jb	@b
	dec	edx
	inc	esi
	mov	byte [edx], '0'
.okleadzero:
	mov	ecx, [edi+4]
	and	ecx, not 0x80000000
	mov     edi, [edi]
	call	i40_writetext_l1
	add	esp, 4Ch
	ret

i40_display_settings:
	dec	ebx
	jnz	@f
set_button_style:
	div	edx
	ret
@@:
	dec	ebx
	jnz	@f
	add	ecx, [base]
set_wnd_colors:
	div	edx
	ret
@@:
	dec	ebx
	jnz	@f
	add	ecx, [base]
get_wnd_colors:
	div	edx
	ret
@@:
	dec	ebx
	jnz	@f
	mov	eax, [_skinh]
	mov	[esp+20h], eax
	ret
@@:
	dec	ebx
	jnz	@f
	call	acquire_shared
	mov	eax, [shared_data]
	mov	ecx, [eax+shared_data_struc.workarea_right]
	mov	[esp+20h], ecx
	mov	ecx, [eax+shared_data_struc.workarea_left]
	mov	[esp+22h], cx
	mov	ecx, [eax+shared_data_struc.workarea_bottom]
	mov	[esp+14h], ecx
	mov	ecx, [eax+shared_data_struc.workarea_top]
	mov	[esp+16h], cx
	call	release_shared
	ret
@@:
	dec	ebx
	jnz	@f
	call	acquire_shared
	mov	eax, [shared_data]
	mov	bx, cx
	mov	[eax+shared_data_struc.workarea_right], ebx
	shr	ecx, 10h
	mov	[eax+shared_data_struc.workarea_left], ecx
	mov	bx, dx
	mov	[eax+shared_data_struc.workarea_bottom], ebx
	shr	edx, 10h
	mov	[eax+shared_data_struc.workarea_top], edx
	call	release_shared
	ret
@@:
	dec	ebx
	jnz	not_supported_i40_fn
	mov	eax, dword [margins]
	mov	[esp+20h], eax
	mov	eax, dword [margins+4]
	mov	[esp+14h], eax
	ret

i40_set_window_shape:
	test	ebx, ebx
	jnz	@f
	mov	[ebp+tls.lpShapeData], ecx
	jmp	.common
@@:	dec	ebx
	jnz	not_supported_i40_fn
	inc	ebx
	shl	ebx, cl
	mov	[ebp+tls.scale], ebx
.common:
	cmp	[ebp+tls.showwnd], 0
	jz	.done
	call	set_window_shape
.done:	ret

set_window_shape:
	mov	esi, [ebp+tls.lpShapeData]
	add	esi, [base]
	movzx	ebx, [ebp+tls.y_size]
	mov	eax, ebx
	inc	eax
	inc	eax
	shl	eax, 4
	push	eax
	call	malloc
	push	eax
	xchg	eax, edi
	push	20h
	pop	eax
	stosd		; RGNDATAHEADER.dwSize
	push	1
	pop	eax
	stosd		; .iType
	xor	eax, eax
	stosd		; .nCount
	stosd		; .nRgnSize
	stosd		; RGNDATAHEADER.rcBound.left
	stosd		; .top
	movzx	eax, [ebp+tls.x_size]
	stosd		; .right
	movzx	eax, [ebp+tls.y_size]
	stosd		; .bottom
	xor	edx, edx	; edx=number of rects, ebp=allocated number of rects
	xor	ecx, ecx
.yloop:
	push	ecx
	xor	ecx, ecx
.1:
	lodsb
	cmp	al, 0
	jnz	.2
	add	ecx, [ebp+tls.scale]
	cmp	cx, [ebp+tls.x_size]
	jae	.3
	jmp	.1
.2:
	inc	edx
	cmp	edx, ebx
	ja	.realloc_rects
.realloc_cont:
	mov	[edi], ecx
	pop	eax
	mov	[edi+4], eax
	push	eax
	add	eax, [ebp+tls.scale]
	mov	[edi+12], eax
	add	ecx, [ebp+tls.scale]
.4:
	lodsb
	cmp	al, 0
	jz	.5
	add	ecx, [ebp+tls.scale]
	cmp	cx, [ebp+tls.x_size]
	jb	.4
.5:
	mov	[edi+8], ecx
	add	edi, 10h
	add	ecx, [ebp+tls.scale]
	cmp	cx, [ebp+tls.x_size]
	jb	.1
.3:
	pop	ecx
	add	ecx, [ebp+tls.scale]
	cmp	cx, [ebp+tls.y_size]
	jb	.yloop
	pop	edi
	mov	[edi+8], edx
	push	edi
	inc	edx
	inc	edx
	shl	edx, 4
	push	edx
	push	0
	call	[ExtCreateRegion]
	push	1
	push	eax
	push	[ebp+tls.hWnd]
	call	[SetWindowRgn]
	push	edi
	call	free
	ret

.realloc_rects:
	push	ecx
	push	edx
	add	ebx, ebx	; multiply size by 2
	mov	eax, ebx
	inc	eax
	inc	eax
	shl	eax, 4
	push	eax
	push	dword [esp+10h]
	push	0
	push	[hHeap]
	call	[HeapReAlloc]
	sub	edi, [esp+0xC]
	add	edi, eax
	mov	[esp+0xC], eax
	pop	edx
	pop	ecx
	jmp	.realloc_cont

i40_create_thread:
	dec	ebx
	jnz	not_supported_i40_fn
	push	ecx
	push	edx
	push	16
	call	malloc
	xchg	eax, ebx
	pop	dword [ebx+4]	; esp
	pop	dword [ebx]	; eip
	push	0
	push	0
	push	0
	push	0
	call	[CreateEventA]
	mov	[ebx+8], eax
	mov	eax, [ebp+tls.cur_dir]
	mov	[ebx+12], eax
	push	eax
	push	esp
	push	0
	push	ebx
	push	KolibriThreadProc
	push	10000h
	push	0
	call	[CreateThread]
	pop	ecx
	test	eax, eax
	jnz	@f
	or	dword [esp+20h], -1
	ret
@@:
	push	eax
	call	[CloseHandle]
	push	-1
	push	dword [ebx+8]
	call	[WaitForSingleObject]
	push	dword [ebx+8]
	call	[CloseHandle]
	mov	esi, [ebx]
	push	ebx
	call	free
; esi = slot; get PID
	call	acquire_shared
	xchg	eax, esi
	call	get_slot_ptr
	mov	eax, [edi]
	mov	dword [esp+20h], eax
	push	edi
	call	get_cur_slot_ptr
	lea	esi, [edi+24]
	pop	edi
	add	edi, 24
	movsd
	movsd
	movsd
	movsd
	call	release_shared
	ret

KolibriThreadProc:
	mov	ebp, [tls_index]
	mov	eax, [fs:2Ch]
	mov	ebp, [eax+ebp*4]
	mov	ebx, [esp+4]
	mov	ecx, [ebx]
	mov	edx, [ebx+4]
	mov	[ebp+tls._cs], cs
	mov	[ebp+tls._ds], ds
	mov	[ebp+tls._fs], fs
	mov	[ebp+tls._esp], edx
	mov	[ebp+tls._eip], ecx
	mov	esi, [ebx+12]
	push	esi
	call	[lstrlenA]
	inc	eax
	push	eax
	push	eax
	call	malloc
	pop	ecx
	mov	[ebp+tls.cur_dir], eax
	mov	edi, eax
	rep	movsb
lock	inc	[NumThreads]
	call	acquire_shared
server_new_thread:
	div	edx
	call	release_shared
	mov	[ebp+tls.cur_slot], ecx
	mov	[ebx], ecx
	push	dword [ebx+8]
	call	[SetEvent]
	xor	ebx, ebx
	push	ebx
	push	400000h
	push	ebx
	push	ebx
	mov	eax, 80000000h
	push	eax
	push	eax
	push	eax
	push	eax
	push	eax
	push	[process_name]
	push	classname
	push	ebx
	call	[CreateWindowExA]
	mov	[ebp+tls.hWnd], eax
	mov	[ebp+tls.bActive], 1
	mov	[ebp+tls.bFirstMouseMove], 1
	call	acquire_shared
	call	get_cur_slot_ptr
	mov	[edi+shared_data_struc.hWnd-shared_data_struc.threads], eax
	call	release_shared
	xor	eax, eax
	xor	ecx, ecx
	xor	edx, edx
	xor	esi, esi
	xor	edi, edi
	push	202h
	pushad
	and	dword [esp+8], 0
	jmp	i40_done

; initialize_winsock:
	; push	ecx edx
	; cmp	[WinSockDLL], -1
	; jz	.failed_ret
	; cmp	[WinSockDLL], 0
	; jnz	.ok
	; push	winsock_name
	; call	[LoadLibraryA]
	; mov	[WinSockDLL], eax
	; test	eax, eax
	; jz	.failed
	; push	esi
	; mov	esi, winsock_imports
; .import:
	; lodsd
	; test	eax, eax
	; jz	.importok
	; push	eax
	; push	[WinSockDLL]
	; call	[GetProcAddress]
	; mov	[esi-4], eax
	; test	eax, eax
	; jnz	.import
	; pop	esi
	; jmp	.failed
; .importok:
	; sub	esp, 190h
	; push	esp
	; push	101h
	; call	[WSAStartup]
	; add	esp, 190h
	; test	eax, eax
	; jz	.ok
; .failed:
	; or	[WinSockDLL], -1
	; push	10h
	; push	0
	; push	aWinsockInitErr
	; push	0
	; call	[MessageBoxA]
; .failed_ret:
	; stc
; .ok:
	; pop	edx ecx
	; ret

; i40_socket:
	; call	initialize_winsock
	; jnc	@f
	; or	dword [esp+20h], -1
	; ret
; @@:
	; cmp	ebx, 9
	; jnz	not_supported_i40_fn
; .is_localport_unused:
	; mov	esi, ecx
; ; destroy old tcp socket
	; mov	eax, [ebp+tls.cursocket]
	; test	eax, eax
	; jz	@f
	; dec	eax
	; push	eax
	; call	[closesocket]
; @@:
	; xor	ebx, ebx
	; mov	[ebp+tls.curport], bx
; ; create new tcp socket
	; push	ebx	; protocol - unspecified
	; push	1	; SOCK_STREAM aka TCP
	; push	2	; AF_INET
	; call	[socket]
	; inc	eax
	; mov	[ebp+tls.cursocket], eax
	; jnz	@f
	; push	10h
	; push	ebx
	; push	aSocketErr
	; push	ebx
	; call	[MessageBoxA]
	; mov	dword [esp+20h], ebx
	; ret
; @@:
	; push	ebx
	; push	ebx	; sockaddr_in.sin_zero
	; push	0x0100007F	; sockaddr_in.sin_addr = 127.0.0.1
	; shl	esi, 16
	; inc	esi
	; inc	esi	; sockaddr_in.sin_family = 2 = AF_INET
	; push	esi	; sockaddr_in.sin_port
	; mov	ecx, esp
	; push	10h	; namelen
	; push	ecx	; name
	; dec	eax
	; push	eax	; s
	; call	[bind]

i40_sound_interface:
	cmp	ebx, 55
	jnz	not_supported_i40_fn
	mov	eax, [shared_data]
	cmp	[eax+shared_data_struc.sound_flag], 0
	jz	@f
	ret
@@:
; prepare wave-block
	push	800h
	call	malloc
	push	eax
	lea	ebx, [eax+800h]
	xchg	eax, edi
	push	esi
	mov	esi, wave_block_begin
	mov	ecx, wbb_size/4
	rep	movsd
	stosd
	pop	esi
	add	esi, [base]
.mainloop:
	lodsb
	test	al, al
	jz	.done
	cmp	al, 81h
	jae	.note
	movzx	edx, al
	xor	eax, eax
	lodsw
	jmp	.doit
.note:
	sub	al, 81h
	movzx	edx, al
	lodsb
	cmp	al, 0xFF
	jz	.pause
	mov	cl, al
	and	eax, 0xF
	movzx	eax, word [kontrOctave+eax+eax]
	shr	cl, 4
	shr	eax, cl
.doit:
; eax=divider (freq=1193180/divider), edx=time (in 1/100 sec)
	mov	ecx, edx
	mul	[wave_r]
	div	[_1193180]
	push	eax
	mov	eax, ecx
	mul	[wave_r]
	div	[_100]
	mov	ecx, eax
	pop	edx
.doit2:
; ecx=number of bytes required for this note
	lea	eax, [edi+ecx]
	cmp	eax, ebx
	ja	.realloc
;	mov	al, 0xFF
;	cmp	edx, ecx
;	jbe	@f
;	mov	al, 0	; this is for pause
;@@:
	mov	al, 80h
; if edx is zero, make it 1
	cmp	edx, 1
	adc	edx, 0
.writeloop:
	push	ecx
	cmp	ecx, edx
	jb	@f
	mov	ecx, edx
@@:	rep	stosb
	pop	ecx
	xor	al, [sound_vol]
	sub	ecx, edx
	ja	.writeloop
	jmp	.mainloop
.pause:
	mov	eax, edx
	mul	[wave_r]
	div	[_100]
	mov	ecx, eax
	mov	edx, eax
	inc	edx
	jmp	.doit2
.realloc:
	pop	eax
	sub	ebx, eax
	add	ebx, ebx	; ebx=new size
	sub	edi, eax	; edi=delta
	push	ecx edx
	push	ebx
	push	eax
	push	0
	push	[hHeap]
	call	[HeapReAlloc]
	pop	edx ecx
	push	eax
	add	ebx, eax
	add	edi, eax
	jmp	.doit2
.done:
	pop	ebx
	sub	edi, ebx
	sub	edi, 8
	mov	[ebx+4], edi
	sub	edi, 24h
	mov	[ebx+28h], edi
; because we use asynchronous call of PlaySoundA, we can not free used memory,
; but we can free previous wave-block
	push	0
	push	0
	push	0
	call	[PlaySoundA]	; force previous sound terminate
	push	5	; SND_ASYNC|SND_MEMORY
	push	0
	push	ebx
	call	[PlaySoundA]
	xchg	ebx, [ebp+tls.prev_snd_block]
	test	ebx, ebx
	jz	@f
	push	ebx
	call	free
@@:
	and	dword [esp+20h], 0
	ret

i40_file_system:
	add	ebx, [base]
	lea	esi, [ebx+0x14]
	cmp     dword [ebx], 0
	jnz     not_supported_i40_fn
emul_read_file:
; read 0 bytes - OK
	cmp	dword [ebx+8], 0
	jnz	@f
	and	dword [esp+20h], 0
	ret
@@:
; check for root dir
	cmp	byte [esi], 0
	jz	.root
	cmp	byte [esi+1], 0
	jnz	.noroot
.root:
	mov	esi, dir0
	mov	edi, [ebx+12]
	add	edi, [base]
	mov	ecx, 12
	push	ecx
	rep	movsb
	add	edi, 32-12
	pop	ecx
	rep	movsb
	and	dword [esp+20h], 0	; eax=0: read ok
	mov	dword [esp+14h], 64	; ebx=size
	ret
.noroot:
	sub	esp, 200h
	call	convert_path
	jc	emul_path_err
	dec	eax
	jz	.ramdisk
	sub	al, 3
	jae	.harddisk
	add	esp, 200h
	jmp	not_supported_i40_fn
.ramdisk:
	lodsb
	cmp	al, 0
	jz	.give_dir1
; must be {/RD | /RAMDISK}{/1 | /FIRST}/...
	lodsw
	cmp	ax, '1\'
	jz	.ramdisk_ok
	cmp	ax, '1'
	jz	.ramdisk_readdir
	cmp	ax, 'FI'
	jnz	emul_path_err
	lodsd
	cmp	eax, 'RST\'
	jz	.ramdisk_ok
	cmp	eax, 'RST'
	jnz	emul_path_err
.ramdisk_readdir:
	add	esp, 200h
	mov	esi, ramdisk_path
	push	0	; read /rd/1
	call	read_directory
	mov	[esp+20h], eax
	mov	[esp+14h], ebx
	ret
.ramdisk_ok:
; now esi points to filename, relative from ramdisk
	sub	esp, 200h
	mov	edi, esp
	push	ramdisk_path
.doit1:
	push	edi
	call	[lstrcpyA]
	cmp	byte [esi], 0
	setz	al
	push	eax
	push	esi
	push	edi
	call	[lstrcatA]
	pop	eax
	lea	esi, [esp+200h]
	xchg	esi, edi
	mov	ecx, 200h/4
	rep	movsd
	add	esp, 200h
	mov	esi, esp
; now esi contains full Win32 name of requested file
	push	eax
	push	esi
	call	[GetFileAttributesA]
	inc	eax
	pop	ecx
	jz	emul_filenotfound
	dec	eax
	test	al, 10h		; FILE_ATTRIBUTE_DIRECTORY
	jnz	.read_dir
	push	0
	push	0
	push	3
	push	0
	push	1
	push	80000000h
	push	esi
	call	[CreateFileA]
	inc	eax
	jz	emul_filenotfound
	dec	eax
	xchg	eax, esi
	mov	eax, [ebx+4]
	xor	edx, edx
	shld	edx, eax, 9
	shl	eax, 9
	push	edx
	mov	edi, esp
	push	0
	push	edi
	push	eax
	push	esi
	call	[SetFilePointer]
	push	0
	push	edi
	mov	eax, [ebx+8]
	shl	eax, 9
; check limits
	mov	ecx, [limit]
	inc	ecx
	sub	ecx, [ebx+0xC]
	cmp	eax, ecx
	jbe	@f
	mov	eax, ecx
@@:	push	eax
	mov	eax, [ebx+0xC]
	add	eax, [base]
	push	eax
	push	esi
	call	[ReadFile]
	pop	eax
	push	0
	push	esi
	call	[GetFileSize]
	add	esp, 200h
	mov	[esp+14h], eax
	push	esi
	call	[CloseHandle]
	and	dword [esp+20h], 0
	ret
.read_dir:
	inc	ecx
	push	ecx	; 2 for exclude '..', 1 for all
	call	read_directory
	add	esp, 200h
	mov	[esp+20h], eax
	mov	[esp+14h], ebx
	ret
.harddisk:
	movzx	edx, al
	lodsb
	test	al, al
	jnz	@f
	add	esp, 200h
	mov	dword [esp+20h], 10	; access denied
	ret
@@:
	xor	ecx, ecx	; partition
@@:
	lodsb
	cmp	al, '\'
	jz	@f
	cmp	al, 0
	jz	@f
	sub	al, '0'
	cmp	al, 9
	ja	emul_path_err
	imul	ecx, 10
	add	cl, al
	jmp	@b
@@:
	test	al, al
	jnz	@f
	dec	esi
@@:
	test	ecx, ecx
	jz	emul_path_err
	cmp	ecx, [hd_partitions_num+edx*4]
	jbe	@f
	add	esp, 200h
	mov	dword [esp+20h], 4	; partition not defined
	ret
@@:
	sub	esp, 200h
	mov	edi, esp
	mov	eax, [hd_partitions_array+edx*4]
	dec	ecx
	shl	ecx, 9
	add	eax, ecx
	push	eax
	jmp	.doit1
.give_dir1:
	add	esp, 200h
	mov	esi, dir1
	mov	edi, [ebx+12]
	add	edi, [base]
	mov	ecx, 12
	push	ecx
	rep	movsb
	and	dword [esp+20h], 0	; eax=0: read ok
	mov	dword [esp+14h], 32	; ebx=size
	ret

emul_path_err:
	push	0
	push	aWarning
	push	aPathIsInvalid
	push	0
	call	[MessageBoxA]
emul_filenotfound:
	add	esp, 200h
	mov	dword [esp+20h], 5	; file not found
	cmp	dword [ebx], 10h
	jnz	@f
	neg	dword [esp+20h]
@@:	ret

aWarning db 'Warning',0
aPathIsInvalid db 'Requested path is invalid',0

convert_path:
	lea	edi, [esp+4]
	push	ebx
	lea	ebx, [edi + 511]
.1:
	lodsb
	cmp	al, '/'
	jz	.1
	dec	esi
	mov	ecx, 11
	sub	ebx, ecx
	cmp	edi, ebx
	ja	.5
	mov	edx, edi
	mov	al, ' '
	add	ebx, ecx
	rep	stosb
	mov	edi, edx
.2:
	lodsb
	cmp	al, '/'
	jz	.3
	cmp	al, '.'
	jz	.4
	cmp	al, 0
	jz	.3
	cmp	edi, ebx
	jae	.5
	cmp	al, 'a'
	jb	@f
	cmp	al, 'z'
	ja	@f
	sub	al, ' '
@@:	stosb
	jmp	.2
.4:
	lea	edi, [edx+8]
	jmp	.2
.3:
	lea	edi, [edx+7]
	mov	ecx, 8
	std
	mov	al, ' '
	repz	scasb
	cld
	jz	.5
	mov	ecx, [edx+8]
	mov	[edi+3], ecx
	mov	byte [edi+2], '.'
	add	edi, 5
	mov	ecx, 3
	std
	repz	scasb
	cld
	jz	@f
	inc	edi
	inc	edi
@@:
	mov	al, '\'
	stosb
	cmp	byte [esi-1], 0
	jnz	.1
	mov	byte [edi-1], 0
	mov	esi, path_begin
	xor	eax, eax
	xor	ecx, ecx
.6:
	add	esi, ecx
	lodsb
	test	al, al
	jz	.5
	xchg	eax, ecx
	lodsb
	xchg	ecx, eax
	lea	edi, [esp+8]
	repz	cmpsb
	jnz	.6
	cmp	byte [edi], 0
	jz	.7
	cmp	byte [edi], '\'
	jnz	.6
.7:
	mov	esi, edi
	pop	ebx
	push	eax
	push	esi
	push	esi
	call	[OemToCharA]
	pop	eax
; convert /hd to /hdx
	cmp	al, 3
	jnz	@f
	mov	eax, [shared_data]
	movzx	eax, [eax+shared_data_struc.hd_base]
	add	eax, 3
@@:
	clc
	ret
.5:
	stc
	pop	ebx
	ret

read_directory:
; in: ebx->fileinfo block, esi->mask
;     byte [esp+4] = 0 for /rd/1, 1 for normal call, 2 for exclude '..'
; out: eax,ebx according to kernel fn58 read /rd/1

; note that all returned values have been taken from kernel code

	mov	ecx, [ebx+8]	; number of blocks
	mov	edi, [ebx+12]	; output memory
	add	edi, [base]
	mov	ebx, [ebx+4]	; start block
; /rd/1 = 14 clusters
	cmp	byte [esp+4], 0
	jnz	.ok2
	cmp	ebx, 14		; 14 clusters = root dir
	jae	.err1
	cmp	ecx, 14
	jbe	.ok1
.err1:
	mov	eax, 5		; out of range
	or	ebx, -1
	ret	4
.ok1:
	lea	edx, [ebx+ecx]
	cmp	edx, 14
	pushfd
	jbe	@f
	sub	edx, 14
	sub	ecx, edx
	jz	.doret2
@@:
	popfd
.ok2:
	pushfd
	shl	ebx, 9
	shl	ecx, 9
	xor	edx, edx
	sub	esp, 200h
	pushad
	lea	edi, [esp+20h]
	push	esi
	call	[lstrlenA]
	mov	ecx, eax
	rep	movsb
	mov	al, '\'
	cmp	[edi-1], al
	jz	@f
	stosb
@@:
	mov	eax, '*.*'
	stosd
	popad
	push	0
	sub	esp, 400h
.loop:
	mov	esi, esp
	push	ecx
	lea	eax, [esi+404h]
	call	get_next_file
; if no file => zero record
	test	eax, eax
	jnz	.still
	push	edi
	mov	edi, esi
	mov	ecx, 0x20/4
	rep	stosd
	pop	edi
	mov	al, 0x20
	add	[esi+400h], eax
.still:
	pop	ecx
	cmp	byte [esp+60Ch], 1
	jz	@f
	cmp	word [esi], '..'
	jz	.loop
@@:
	push	ecx
	cmp	ebx, eax
	jae	.skip
	add	esi, ebx
	mov	ecx, eax
	sub	ecx, ebx
	cmp	ecx, [esp]
	jb	@f
	mov	ecx, [esp]
@@:
	sub	[esp], ecx
	rep	movsb
.skip:
	sub	ebx, eax
	jae	@f
	xor	ebx, ebx
@@:
	pop	ecx
	test	ecx, ecx
	jnz	.loop
	call	done_find_file
.doret:
	add	esp, 400h
	pop	ecx
	add	esp, 200h
.doret2:
	xor	ebx, ebx
	xor	eax, eax
	cmp	byte [esp+8], 0
	jz	.check_rd1
	popfd
;	cmp	edx, -1
;	jz	.eof
	cmp	ecx, 0x200
	jae	.eof
	jmp	.ret
.check_rd1:
	popfd
	jb	.ret
.eof:
	mov	al, 6		; EOF
.ret:	ret	4

get_next_file:
; in: eax=mask, edx=hFindData, edx=0 or edx=-1, esi->buffer
; out: buffer filled, eax=length
	cmp	edx, -1
	jnz	@f
	xor	eax, eax
	ret
@@:
	sub	esp, 0x140	; sizeof(WIN32_FIND_DATA)
	pushad
	lea	ecx, [esp+0x20]
	push	ecx
	test	edx, edx
	jnz	.inited
	push	eax
	call	[FindFirstFileA]
	mov	edx, eax
	mov	[esp+0x14], edx
	inc	eax
	jmp	.cmn
.inited:
	push	edx
	call	[FindNextFileA]
	mov	edx, [esp+0x14]
.cmn:
	test	eax, eax
	jnz	.ok
	cmp	edx, -1
	jz	@f
	push	edx
	call	[FindClose]
@@:
	popad
	add	esp, 0x140
	xor	eax, eax
	or	edx, -1
	ret
.ok:
	popad
; Long File Names are not supported yet
; file name
	push	edi
	lea	edi, [esp+4+0x130]	; WIN32_FIND_DATA.cAlternateFileName
	cmp	byte [edi], 0
	jnz	.shortname
	push	esi
	lea	esi, [esp+8+0x2C]	; WIN32_FIND_DATA.cFileName
	push	edi
.l2s:
	lodsb
	cmp	al, 'a'
	jb	@f
	cmp	al, 'z'
	ja	@f
	sub	al, 0x20
@@:	stosb
	cmp	al, 0
	jnz	.l2s
	pop	edi esi
.shortname:
	pushad
	push	edi
	push	edi
	call	[CharToOemA]
	popad
; file name
	mov	ecx, 10
@@:
	mov	byte [esi+ecx], ' '
	dec	ecx
	jns	@b
	inc	ecx
@@:
	cmp	byte [edi], '.'
	jnz	@f
	inc	edi
	mov	byte [esi+ecx], '.'
	inc	ecx
	jmp	@b
@@:
	mov	al, [edi]
	inc	edi
	cmp	al, 0
	jz	.namedone
	cmp	al, '.'
	jz	.dot
	mov	[esi+ecx], al
	inc	ecx
	jmp	@b
.dot:
	mov	cl, 8
	jmp	@b
.namedone:
	pop	edi
; attributes
	mov	eax, [esp]	; WIN32_FIND_DATA.dwFileAttributes
	and	al, 0x3F
	mov	[esi+11], al
	and	word [esi+12], 0
; creation file and date
	pushad
	lea	eax, [esi+14]
	push	eax
	inc	eax
	inc	eax
	push	eax
	lea	eax, [esp+8+20h+4]
	push	eax
	call	[FileTimeToDosDateTime]
; last access date
	lea	eax, [esp+1Ch]
	push	eax
	lea	eax, [esi+18]
	push	eax
	lea	eax, [esp+8+20h+0xC]
	push	eax
	call	[FileTimeToDosDateTime]
; high word of cluster
	and	word [esi+20], 0
; last write file and date
	lea	eax, [esi+22]
	push	eax
	inc	eax
	inc	eax
	push	eax
	lea	eax, [esp+8+20h+0x14]
	push	eax
	call	[FileTimeToDosDateTime]
; low word of cluster
	mov	word [esi+26], 0xBAD	; random value
; file size
	popad
	mov	eax, [esp+0x20]
	mov	[esi+28], eax
; return
	add	esp, 0x140
	mov	eax, 0x20
	ret

done_find_file:
	test	edx, edx
	jz	.ret
	cmp	edx, -1
	jz	.ret
	push	edx
	push	edx
	call	[FindClose]
	pop	edx
.ret:	ret

notify_run_prg:
	test	esi, esi
	jz	@f
	add	esi, [base]
@@:
	call	acquire_shared
	push	eax esi edi
	mov	esi, [ebp+tls.cur_dir]
	mov	edi, process_curdir
@@:
	lodsb
	stosb
	test	al, al
	jz	@f
	cmp	edi, process_curdir+4096
	jb	@b
	mov	byte [edi-1], 0
@@:
	pop	edi esi eax
server_run_prg:
	div	edx
	call	release_shared
	push	ecx
	push	-1
	push	ecx
	call	[WaitForSingleObject]
	call	[CloseHandle]
server_get_run_result:
	div	edx
	jecxz	.ret
	push	eax ecx edx
	push	24
	call	malloc
	mov	[eax+16], esi
	mov	[eax+20], edi
	mov	esi, [ebp + tls.debuggees]
	mov	[eax], esi
	mov	[ebp + tls.debuggees], eax
	pop	dword [eax+12]
	pop	dword [eax+8]
	pop	dword [eax+4]
	mov	eax, [eax+4]
.ret:
	ret

i40_ipc:
	dec	ebx
	jnz	@f
; set IPC memory
	call	acquire_shared
	call	get_cur_slot_ptr
	add	ecx, [base]
	mov	[edi+4], ecx
	mov	[edi+8], edx
	call	release_shared
	and	dword [esp+20h], 0
	ret
@@:
	dec	ebx
	jnz	not_supported_i40_fn
	add	edx, [base]
	call	acquire_shared
server_send_ipc:
	div	edx
	call	release_shared
	test	eax, eax
	jnz	@f
	push	0
	push	0
	push	400h		; WM_USER
	push	ecx
	call	[PostMessageA]
	xor	eax, eax
@@:
	mov	[esp+20h], eax
	ret

i40_direct_scr_access:
	cmp	[ColorDepth], 0
	jnz	@f
	push	10h
	push	0
	push	DSADisabled
	push	0
	call	[MessageBoxA]
	jmp	i40_terminate
@@:
	dec	ebx
	jz	.1
	dec	ebx
	jz	.2
	dec	ebx
	jz	.3
	jmp	not_supported_i40_fn
.1:
	call	get_screen_size
	add	ebx, 10001h
	mov	[esp+20h], ebx
	ret
.2:
	mov	eax, [ColorDepth]
	mov	[esp+20h], eax
	ret
.3:
	call	get_screen_size
	shr	ebx, 16
	lea	eax, [ebx+1]
	mul	[ColorDepth]
	shr	eax, 3
	add	eax, 3
	and	eax, not 3
	mov	[esp+20h], eax
	ret

i40_pci:
	mov	eax, [shared_data]
	cmp	[eax+shared_data_struc.pci_access_enabled], 0
	jnz	@f
.reterr:
	or	dword [esp+20h], -1	; PCI access disabled
	ret
@@:
	cmp	bl, 2
	ja	.noinit
	cmp	[eax+shared_data_struc.pci_data_init], 0
	jnz	@f
	push	eax ebx
	call	pci_data_init
	pop	ebx eax
@@:
	test	bl, bl
	jnz	.not0
	xor	ecx, ecx
	cmp	[eax+shared_data_struc.pci_data_init], 1
	jnz	.unsupp0	; emulate "unsupported" behavior
	mov	ch, [eax+shared_data_struc.pci_bios_mj]
	mov	cl, [eax+shared_data_struc.pci_bios_mn]
.unsupp0:
	mov	[esp+20h], ecx
	ret
.not0:
	dec	bl
	mov	ecx, ebx
	jnz	.not1
	cmp	[eax+shared_data_struc.pci_data_init], 1
	jnz	.unsupp1
	mov	cl, [eax+shared_data_struc.pci_bios_lb]
.unsupp1:
.retecx:
	mov	[esp+20h], ecx
	ret
.not1:
	test	[eax+shared_data_struc.pci_bios_pc], 1
	jnz	@b
	test	[eax+shared_data_struc.pci_bios_pc], 2
	jz	@b
	inc	ecx
	jmp	@b
.noinit:
	sub	bl, 4
	cmp	bl, 2
	ja	not_supported_i40_fn
	cmp	[bIs9x], 0
	jz	.read_nt
	mov	ah, 0xB1
	mov	al, bl
	add	al, 8
	mov	bl, ch
	movzx	di, cl
	mov	ecx, ebx
	call	[jmp_temp_int1A]
	jc	.reterr
	jmp	.retecx
.read_nt:
	push	cx
	push	bx
	mov	eax, esp
	push	ecx
	mov	ecx, esp
	push	4
	push	ecx
	push	4
	push	eax
	push	0x222004
	call	send_driver_request
	pop	ecx edx
	test	eax, eax
	jnz	.retecx
	push	30h
	push	aWarning
	push	DrvOpenErr
	push	0
	call	[MessageBoxA]
	jmp	.reterr

pci_data_init:
	call	acquire_shared
	cmp	[bIs9x], 0
	jz	.nt
; On 9x systems, just do "int 1Ah" from 16-bit code
	mov	ax, 0xB101
	call	[jmp_temp_int1A]
	test	ah, ah
	jnz	.err
	cmp	edx, 'PCI '
	jnz	.err
	mov	edx, [shared_data]
	mov	[edx+shared_data_struc.pci_bios_mj], bh
	mov	[edx+shared_data_struc.pci_bios_mn], bl
	mov	[edx+shared_data_struc.pci_bios_lb], cl
	mov	[edx+shared_data_struc.pci_bios_pc], al
	mov	[edx+shared_data_struc.pci_data_init], 1
	jmp	.ok
.nt:
; On NT systems, there is 'HKLM\HARDWARE\DESCRIPTION\System\MultifunctionAdapter' key,
;   with some subkeys '0','1',..., which hold information on periphery.
; Each PCI bus has its own subkey with 'Identifier'="PCI".
; For first PCI bus 'Configuration Data' holds 4 additional bytes = info on PCI BIOS.
.loop:
	push	eax
	push	esp	; phkResult
	push	1	; samDesired = KEY_QUERY_VALUE
	push	0	; ulOptions
	push	keymfa	; lpSubKey
	push	80000002h	; hKey = HKEY_LOCAL_MACHINE
	call	[RegOpenKeyExA]
	pop	esi	; key handle
	test	eax, eax
	jnz	.err	; subkeys are done, and we do not find required
	push	eax
	mov	eax, esp
	push	4
	push	esp	; lpcbData
	push	eax	; lpData
	push	0	; lpType
	push	0	; lpReserved
	push	aIdentifier	; lpValueName
	push	esi	; hKey
	call	[RegQueryValueExA]
	pop	ecx
	pop	edx
	test	eax, eax
	jnz	.cont
	cmp	ecx, 4
	jnz	.cont
	cmp	edx, 'PCI'
	jnz	.cont
	sub	esp, 24h
	mov	eax, esp
	push	24h
	push	esp
	push	eax
	push	0
	push	0
	push	aConfigurationData
	push	esi
	call	[RegQueryValueExA]
	pop	ecx
	test	eax, eax
	jnz	.err3
	cmp	ecx, 24h
	jnz	.err3
	mov	eax, [shared_data]
	add	esp, 20h
	pop	dword [eax+shared_data_struc.pci_bios_mj]
	dec	[eax+shared_data_struc.pci_bios_lb]
	js	.err2
	mov	[eax+shared_data_struc.pci_data_init], 1
	push	esi
	call	[RegCloseKey]
.ok:
	call	release_shared
	ret
.cont:
	push	esi
	call	[RegCloseKey]
	inc	[idxmfa]
	cmp	[idxmfa], '9'
	jbe	.loop
	jmp	.err
.err3:
	add	esp, 24h
.err2:
	push	esi
	call	[RegCloseKey]
.err:
	mov	eax, [shared_data]
	mov	[eax+shared_data_struc.pci_data_init], 2
	call	release_shared
	push	40h
	push	0
	push	aCannotGetPci
	push	0
	call	[MessageBoxA]
	ret

i40_debug_board:
	dec	ebx
	jz	.write
	dec	ebx
	jnz	not_supported_i40_fn
.read:
	call	acquire_shared
	mov	eax, [shared_data]
	mov	ecx, [eax+shared_data_struc.msg_board_count]
	test	ecx, ecx
	jnz	@f
	mov	[esp+20h], ecx	; zero eax
	mov	[esp+14h], ecx	; zero ebx
	jmp	.ret
@@:
	dec	[eax+shared_data_struc.msg_board_count]
	lea	edi, [eax+shared_data_struc.msg_board_data]
	lea	esi, [edi+1]
	movzx	edx, byte [edi]
	mov	[esp+20h], edx
	mov	dword [esp+14h], 1
	dec	ecx
	rep	movsb
.ret:
	jmp	release_shared
.write:
	call	acquire_shared
	mov	eax, [shared_data]
	mov	edx, [eax+shared_data_struc.msg_board_count]
	mov	[eax+shared_data_struc.msg_board_data+edx], cl
	inc	edx
	and	edx, 511
	mov	[eax+shared_data_struc.msg_board_count], edx
	jmp	.ret

i40_resize_app_memory:
	dec	ebx
	jnz	not_supported_i40_fn
	cmp     [heap_status], 0
	jz      @f
	push    30h
	push    aWarning
	push    aInvFn64Call
	push    0
	call    [MessageBoxA]
	mov     dword [esp+20h], 1
	ret
@@:
	cmp	[NumThreads], 1
	jnz	not_supported_i40_fn
	push	ecx
	push	40h	; PAGE_EXECUTE_READWRITE
	push	1000h	; MEM_COMMIT
	push	ecx
	push	0
	call	[VirtualAlloc]
	pop	ecx
	test	eax, eax
	jnz	.ok
	mov	dword [esp+0x20], 1
	ret
.ok:
	mov	edi, eax
	mov	esi, [base]
	push	ecx
	cmp	ecx, [limit]
	jbe	@f
	mov	ecx, [limit]
	inc	ecx
@@:
	push	eax
	rep	movsb
	call	acquire_shared
	call	get_cur_slot_ptr
	mov	ecx, [edi+4]
	jecxz	@f
	sub	ecx, [base]
	add	ecx, [esp]
	mov	[edi+4], ecx
@@:
	call	release_shared
	push	8000h	; MEM_RELEASE
	push	0
	push	[base]
	call	[VirtualFree]
	pop	eax
	pop	ecx
	mov	[base], eax
	dec	ecx
	mov	[limit], ecx
	mov     [fn9limit], ecx
	call	get_cur_slot_ptr
	mov	[edi+24], ecx
	mov	esi, selector_data
	mov	[esi+2], ax
	shr	eax, 10h
	mov	[esi+4], al
	mov	[esi+7], ah
	shr	ecx, 0Ch
	mov	[esi], cx
	shr	ecx, 10h
	or	cl, 11000000b
	mov	[esi+6], cl
	mov	byte [esi+5], 11110010b
	lea	edi, [esi+8]
	movsd
	movsd
	mov	byte [esi+5], 11111010b
	cmp	[bIs9x], 0
	jnz	.9x
	push	dword [esi-4]
	push	dword [esi-8]
	push	17h
	push	dword [esi+4]
	push	dword [esi]
	push	0Fh
	call	[NtSetLdtEntries]
	mov	esi, ldterr
	test	eax, eax
	js	fail
.d:	and	dword [esp+20h], 0
	ret
.9x:
	mov	eax, sl0p
	call	CallRing0
	jmp	.d

convert_2bpp:
	mov	esi, edi
	add	esi, [base]
	movzx	eax, word [esp]
	pushad
	add	ecx, 7
	shr	ecx, 1
	and	ecx, not 3
	mul	ecx
	push	eax
	call	malloc
	mov	[esp+1Ch], eax
	popad
	push	eax edx esi
	mov	edi, eax
	movzx	eax, word [esp+12]
	mov	esi, ebx
.extloop:
	push	eax
	push	ecx
.innloop:
	lodsb
	mov	dl, al
	push    eax
	shr     al, 6
	shr     dl, 4
	shl     al, 4
	and     dl, 3
	or      al, dl
	stosb
	pop     eax
	mov     dl, al
	shl     al, 2
	and     dl, 3
	and     al, 0x30
	or      al, dl
	stosb
	sub     ecx, 4
	ja      .innloop
	test	edi, 3
	jz	@f
	stosw
@@:
	pop	ecx
	pop	eax
	add	esi, [esp+16+0Ch]
	dec	eax
	jnz	.extloop
	pop	esi edx edi
	jmp	pad_cont8

pad_bmp8:
	movzx	eax, word [esp]
	pushad
	add	ecx, 3
	and	ecx, not 3
	mul	ecx
	push	eax
	call	malloc
	mov	[esp+1Ch], eax
	popad
	push	eax esi
	mov	edi, eax
	movzx	eax, word [esp+8]
	mov	esi, ebx
.extloop:
	push	eax
	push	ecx
	mov     eax, ecx
	shr     ecx, 2
	rep	movsd
	mov     ecx, eax
	and     ecx, 3
	rep     movsb
	mov	ecx, eax
	neg	ecx
	and	ecx, 3
	mov	al, 0
	rep	stosb
	pop	ecx
	pop	eax
        add     esi, [esp+4*3+0Ch]
	dec	eax
	jnz	.extloop
	pop	esi edi
	jmp	pad_cont8

i40_putimage_palette:
	add	ebx, [base]
	push	ecx
	shr	ecx, 16
	cmp	esi, 1
	jz	.1
	cmp	esi, 4
	jz	.4
	cmp	esi, 8
	jz	.8
	cmp	esi, 15
	jz	.15
	cmp	esi, 16
	jz	.16
	cmp	esi, 32
	jz	.32
	cmp	esi, 2
	jz	convert_2bpp
	cmp	esi, 24
	jz	.24
	pop	ecx
	jmp	not_supported_i40_fn
.24:
	lea	ecx, [ecx*3]
	jmp	@f
.1:
	add	ecx, 7
	shr	ecx, 3
	jmp	@f
.2:
	add	ecx, 3
	shr	ecx, 2
	jmp	@f
.4:
	inc	ecx
	shr	ecx, 1
	jmp	@f
.32:
	add	ecx, ecx
.15:
.16:
	add	ecx, ecx
.8:
@@:
	add     edi, [base]
	mov     esi, edi
; Windows requires that all scanlines are DWORD-padded
	mov	edi, ebx
	test	cl, 3
	jnz	pad_bmp8
	cmp     dword [esp+10h], 0
	jnz     pad_bmp8
pad_cont8:
	pop	ecx
	mov	eax, [esp+8]
; palette
        sub     esp, 256*4
	push	ecx edi
	lea	edi, [esp+8]
	cmp	al, 8
	jnz	@f
        mov     ecx, 256
        rep     movsd
@@:
	cmp	al, 4
	jnz	@f
	mov	ecx, 16
	rep	movsd
@@:
	cmp	al, 2
	jnz	@f
	movsd
	movsd
	movsd
	movsd
	add	eax, eax
@@:
	cmp	al, 1
	jnz	@f
	movsd
	movsd
@@:
	cmp	al, 16
	jnz	@f
	mov	dword [edi+0], 0xF800
	mov	dword [edi+4], 0x07E0
	mov	dword [edi+8], 0x001F
@@:
        pop     edi ecx
        xor     esi, esi
; BITMAPINFO
	push	esi	; biClrImportant
	push	esi	; biClrUsed
	push	esi	; biYPelsPerMeter
	push	esi	; biXPelsPerMeter
	push	esi	; biSizeImage
	cmp	al, 15
	jnz	.no15
	push	esi	; biCompression
	push	100001h	; biPlanes, biBitCount
	jmp	@f
.no15:
	cmp	al, 16
	jnz	.no16
	push	3	; biCompression
	push	100001h	; biPlanes, biBitCount
	jmp	@f
.no16:
	push	esi	; biCompression
;	push	80001h	; biPlanes, biBitCount
	shl	eax, 16
	inc	eax
	push	eax	; biPlanes, biBitCount
@@:
	movzx	eax, cx
	neg	eax
	push	eax	; biHeight
	neg	eax
	shr	ecx, 10h
	push	ecx	; biWidth
	push	40	; biSize
	push	ebx
	lea	ebx, [esp+4]
; SetDIBitsToDevice
	push	esi	; fuColorUse = DIB_RGB_COLORS
	push	ebx	; lpbmi
	push	edi	; lpvBits
	push	eax	; cScanLines
	dec	eax
	push	eax	; uStartScan
	push	eax	; YSrc
	inc	eax
	push	esi	; XSrc
	push	eax	; dwHeight
	push	ecx	; dwWidth
	movzx	ecx, dx
	add     ecx, [ebp+tls.client_top]
	push	ecx	; YDest
	shr	edx, 10h
	add     edx, [ebp+tls.client_left]
	push	edx	; XDest
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, ebx
	push	ebx	; hdc
	call	[SetDIBitsToDevice]
	xchg	eax, ebx
	pop	ebx
	add	esp, 40+256*4
	push	eax
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	cmp	edi, ebx
	jz	@f
	push	edi
	call	free
@@:
	ret

i40_process_def:
	dec	ebx
	jz	.setmode
	dec	ebx
	jz	.getmode
	dec	ebx
	jz	.get_control
	dec	ebx
	jz	.add_hotkey
	dec	ebx
	jz	.del_hotkey
	jmp	not_supported_i40_fn
.setmode:
	mov	[ebp+tls.usescancode], cl
	ret
.getmode:
	movzx	eax, [ebp+tls.usescancode]
	mov	[esp+20h], eax
	ret
.get_control:
	mov	esi, .vkeycodes
	mov	edi, 1
.gcloop:
	xor	eax, eax
	lodsb
	push	eax
	call	[GetAsyncKeyState]
	test	ax, ax
	jns	@f
	or	ebx, edi
@@:
	add	edi, edi
	cmp	esi, .vkeycodes_end
	jb	.gcloop
	mov	[esp+20h], ebx
	ret
.add_hotkey:
.del_hotkey:
	and	dword [esp+20h], 0
	ret
.vkeycodes:
	db	0xA0	; VK_LSHIFT
	db	0xA1	; VK_RSHIFT
	db	0xA2	; VK_LCONTROL
	db	0xA3	; VK_RCONTROL
	db	0xA4	; VK_LMENU
	db	0xA5	; VK_RMENU
	db	0x14	; VK_CAPITAL
	db	0x90	; VK_NUMLOCK
	db	0x91	; VK_SCROLL
.vkeycodes_end:

i40_move_resize:
	cmp	ebx, -1
	jnz	@f
	movzx	ebx, [ebp+tls.x_start]
@@:	cmp	ecx, -1
	jnz	@f
	movzx	ecx, [ebp+tls.y_start]
@@:	cmp	edx, -1
	jnz	@f
	movzx	edx, [ebp+tls.x_size]
	dec	edx
@@:	cmp	esi, -1
	jnz	@f
	movzx	esi, [ebp+tls.y_size]
	dec	esi
@@:
	inc	edx
	mov	[ebp+tls.x_start], bx
	inc	esi
	mov	[ebp+tls.y_start], cx
	mov	[ebp+tls.x_size], dx
	mov	[ebp+tls.y_size], si
	push	1
	push	esi
	push	edx
	push	ecx
	push	ebx
	push	[ebp+tls.hWnd]
	call	[MoveWindow]
	push	0
	push	0
	push	[ebp+tls.hWnd]
	call	[InvalidateRect]
	ret

i40_sys_services:
        cmp     ebx, 3
        jnz     .not3
	mov	esi, [shared_data]
	cmp     [esi+shared_data_struc.bAllowReadMSR], 0
	mov     edi, aReadMSRDisabled
	jz      .err
	cmp	[bIs9x], 0
	jz	.nt
	mov     ecx, edx
	mov	eax, rdmsrp
	call	CallRing0
	test	ebx, ebx
	jnz	.nomsr
	mov     [esp+20h], eax
	mov     [esp+14h], edx
	ret
.nt:
	push	0
	push	edx
	push	edx
	mov	eax, esp
	push	9
	push	eax
	push	4
	push	eax
	push	0x222000
	call	send_driver_request
	pop	ecx
	pop	edx
	pop	esi
	mov	edi, DrvOpenErr
	test	eax, eax
	jz	.err
	test	esi, esi
	jnz	.nomsr
	mov     [esp+20h], ecx
	mov     [esp+14h], edx
	ret
.err:
	push	30h
	push	aWarning
	push	edi
	push	0
	call	[MessageBoxA]
	and     dword [esp+20h], 0
	and     dword [esp+14h], 0
	ret
.nomsr:
        push    10h
        push    0
        push    aNoMsr
        push    0
        call    [MessageBoxA]
        jmp     i40_terminate
.not3:
        cmp     ebx, 11
        jnz     .not11
.initialize_heap:
; initialize heap
        cmp     [heap_status], 0
        jz      .create_heap
@@:
        cmp     [heap_status], 1
        jnz     @f
        push    5
        call    [Sleep]
        jmp     @b
@@:
.heap_ret_size:
        mov     eax, 0x5FC00000-0x1000
        sub     eax, [heap_start]
        mov     [esp+20h], eax
        ret
.create_heap:
        cmp     [NumThreads], 1
        jnz     not_supported_i40_fn
        mov     [heap_status], 1
        xor     ebx, ebx
; reserve needed big region
        mov     esi, 0x40000000
@@:
        push    40h     ; PAGE_EXECUTE_READWRITE
        push    2000h   ; MEM_RESERVE
        push    esi
        push    ebx
        call    [VirtualAlloc]
        test    eax, eax
        jnz     @f
        shr     esi, 1
        cmp     esi, 0x01000000
        jae     @b
.nomem_fatal:
	xor	ebx, ebx
        mov     esi, memerr
        jmp     fail
@@:
        mov     [heap_region_size], esi
        push    eax
        mov     ecx, [limit]
        inc     ecx
        push    40h     ; PAGE_EXECUTE_READWRITE
        push    1000h   ; MEM_COMMIT
        push    ecx
        push    eax
        call    [VirtualAlloc]
        test    eax, eax
        jz      .nomem_fatal
        mov     esi, [base]
        pop     edi
        push    edi
        mov     edi, eax
        mov     ecx, [limit]
        inc     ecx
        rep     movsb
        push    8000h   ; MEM_RELEASE
        push    ebx
        push    [base]
        call    [VirtualFree]
        pop     eax
        mov     [base], eax
        mov     ecx, [heap_region_size]
        dec     ecx
        mov     [limit], ecx
        mov     esi, selector_data
        mov     [esi+2], ax
        shr     eax, 10h
        mov     [esi+4], al
        mov     [esi+7], ah
        shr     ecx, 0Ch
        mov     [esi], cx
        shr     ecx, 10h
        or      cl, 11000000b
        mov     [esi+6], cl
        mov     byte [esi+5], 11110010b
        lea     edi, [esi+8]
        movsd
        movsd
        mov     byte [esi+5], 11111010b
        cmp     [bIs9x], bl
        jnz     .9x
        push    dword [esi-4]
        push    dword [esi-8]
        push    17h
        push    dword [esi+4]
        push    dword [esi]
        push    0Fh
        call    [NtSetLdtEntries]
        mov     esi, ldterr
        test    eax, eax
        js      fail
        jmp     .heap_created
.9x:
        mov     eax, sl0p
        call    CallRing0
        xor	ebx, ebx
.heap_created:
        mov     eax, [fn9limit]
        or      eax, 0xFFF
        inc     eax
        mov     [heap_start], eax
        mov     eax, [heap_region_size]
        sub     eax, [heap_start]
        shr     eax, 10
	call	malloc_big
        mov     [heap_control_block], eax
        test    eax, eax
        jz      .nomem_fatal
        mov     ecx, [heap_region_size]
        sub     ecx, [heap_start]
        or      ecx, 4
        mov     [eax], ecx
        push    heap_critical_sec
        call    [InitializeCriticalSection]
        mov     [heap_status], 2
        jmp     .heap_ret_size
.not11:
        cmp     ebx, 12
        jnz     .not12
; allocate memory block in heap
.allocate_heap:
        cmp     [heap_status], 0
        jnz     @f
        push    30h
        push    aWarning
        push    aHeapNotInited
        push    0
        call    [MessageBoxA]
        and     dword [esp+20h], 0
        ret
@@:
        lea     edi, [ecx+0x1FFF]
        and     edi, not 0xFFF
@@:
        cmp     [heap_status], 1
        jnz     @f
        push    5
        call    [Sleep]
        jmp     @b
@@:
        push    heap_critical_sec
        call    [EnterCriticalSection]
        xor     esi, esi
        mov     ecx, [heap_region_size]
        sub     ecx, [heap_start]
        mov     edx, [heap_control_block]
.l_0:
        cmp     esi, ecx
        jae     .m_exit
        mov     ebx, esi
        shr     ebx, 12
        mov     eax, [edx+ebx*4]
        test    al, 4
        jz      .test_used
        and     eax, not 0xFFF
        cmp     eax, edi
        jb      .m_next
        jz      @f
        push    esi
        add     esi, edi
        sub     eax, edi
        or      al, 4
        shr     esi, 12
        mov     [edx+esi*4], eax
        pop     esi
        mov     eax, edi
@@:
        or      al, 8
        mov     [edx+ebx*4], eax
        mov     eax, [heap_start]
        lea     eax, [eax+esi+0x1000]
        push    eax
        sub     edi, 0x1000
; now do real allocate at eax with size edi
        push    40h     ; PAGE_EXECUTE_READWRITE
        push    1000h   ; MEM_COMMIT
        push    edi
        add     eax, [base]
        push    eax
        call    [VirtualAlloc]
        test    eax, eax
        jz      .nomem_fatal
        jmp     .allocated
.m_next:
        add     esi, eax
        jmp     .l_0
.test_used:
        test    al, 8
        jnz     @f
.fail_internal:
        xor     ebx, ebx
        mov     esi, aInternalError
        jmp     fail
@@:
        and     eax, not 0xFFF
        jmp     .m_next
.m_exit:
        push    0
.allocated:
        push    heap_critical_sec
        call    [LeaveCriticalSection]
        cmp     dword [esp], 0
        jnz     @f
        push    30h
        push    aWarning
        push    aMallocFailed
        push    0
        call    [MessageBoxA]
@@:
        pop     eax
        mov     [esp+20h], eax
        ret
.not12:
        cmp     ebx, 13
        jnz     .not13
; free memory block in heap
.free_heap:
        cmp     [heap_status], 0
        jnz     @f
        push    30h
        push    aWarning
        push    aHeapNotInited
        push    0
        call    [MessageBoxA]
        and     dword [esp+20h], 0
        ret
@@:
        test    ecx, ecx
        jnz     @f
; free(NULL) is OK
        mov     dword [esp+20h], 1
        ret
@@:
        mov     esi, ecx
        cmp     [heap_status], 1
        jnz     @f
        push    5
        call    [Sleep]
        jmp     @b
@@:
        cmp     esi, [heap_region_size]
        jae     .inv_exit
        sub     esi, [heap_start]
        jb      .inv_exit
        cmp     esi, 0x1000
        jae     @f
.inv_exit:
        mov     dword [esp+20h], 1
.inv_exit_cmn:
        push    30h
        push    aWarning
        push    aFreeInvalid
        push    0
        call    [MessageBoxA]
        ret
.inv_exit_realloc:
        and     dword [esp+20h], 0
        jmp     .inv_exit_cmn
@@:
        push    heap_critical_sec
        call    [EnterCriticalSection]
        mov     eax, esi
        shr     esi, 12
        dec     esi
        mov     edx, [heap_control_block]
	test	byte [edx+esi*4], 10h
	jnz	.inv_exit_realloc
        test    byte [edx+esi*4], 8
        jz      .inv_wrn
        test    eax, 0xFFF
        jz      @f
.inv_wrn:
        push    edx
        push    30h
        push    aWarning
        push    aFreeInvalid
        push    0
        call    [MessageBoxA]
        pop     edx
@@:
        mov     eax, [edx+esi*4]
        test    al, 8
        jz      .freed
        push    edx
        and     eax, not 0xFFF
        push    0x4000          ; MEM_DECOMMIT
        sub     eax, 0x1000
        push    eax
        lea     eax, [esi+1]
        shl     eax, 12
        add     eax, [heap_start]
        add     eax, [base]
        push    eax
        call    [VirtualFree]
        pop     edx
        mov     eax, [edx+esi*4]
        and     eax, not 0xFFF
        push    eax
        or      al, 4
        mov     [edx+esi*4], al
        shr     eax, 12
        add     eax, esi
        test    byte [edx+eax*4], 4
        jz      @f
        xor     ecx, ecx
        xchg    ecx, [edx+eax*4]
        and     ecx, not 0xFFF
        add     [edx+esi*4], ecx
        add     [esp], ecx
@@:
        pop     eax
        test    esi, esi
        jz      .freed
        xor     edi, edi
.findprev:
        cmp     edi, esi
        jz      .foundprev
        mov     ecx, edi
        mov     ebx, [edx+ecx*4]
        shr     ebx, 12
        add     edi, ebx
        jmp     .findprev
.foundprev:
        test    byte [edx+ecx*4], 4
        jz      .freed
        and     dword [edx+esi*4], 0
        add     [edx+ecx*4], eax
.freed:
        push    heap_critical_sec
        call    [LeaveCriticalSection]
        mov     dword [esp+20h], 1
        ret
.not13:
        cmp     ebx, 16
        jnz     .not16
        add     ecx, [base]
        mov     esi, ecx
        push    esi
        push    aSound
        call    [lstrcmpA]
        test    eax, eax
        jnz     @f
        and     dword [esp+20h], 0
        ret
@@:
        push    esi
        push    aInfinity
        call    [lstrcmpA]
        test    eax, eax
        jnz     @f
        and     dword [esp+20h], 0
        ret
@@:
        push    10h
        push    0
        push    aUnknownDriver
        push    0
        call    [MessageBoxA]
        and     dword [esp+20h], 0
        ret
.not16:
        cmp     ebx, 19
        jnz     .not19
; load DLL
        sub     esp, 40
        mov     eax, esp
        sub     eax, [base]
        push    0
        push    0
        push    eax
        push    0
        push    0
        push    0
        push    5
        mov     ebx, esp
        mov     [ebx+21], ecx
        pushad
        call    i40_file_system_lfn.noadd
        popad
        test    eax, eax
        jz      @f
.err1:
        add     esp, 28+40
        and     dword [esp+20h], 0
        ret
@@:
        pop     eax
        push    0
        mov     ebx, esp
        cmp     dword [ebx+28+36], 0
        jnz     .err1
        mov     eax, [ebx+28+32]
        mov     [ebx+12], eax
	call	malloc_big
        test    eax, eax
        jnz     @f
.nomem_dll:
        push    10h
        push    memerr
        push    aCannotLoadDll
        push    0
        call    [MessageBoxA]
        jmp     .err1
@@:
        sub     eax, [base]
        mov     [ebx+16], eax
        push    ebx
        pushad
        call    i40_file_system_lfn.noadd
        popad
        pop     ebx
        test    eax, eax
        jnz     .err1
        mov     eax, [ebx+16]
        add     eax, [base]
        mov     [ebx+16], eax
        cmp     dword [eax], 'KPCK'
        jnz     .notpacked
	mov	eax, [eax+4]
	call	malloc_big
        test    eax, eax
        jnz     @f
        push    dword [ebx+16]
        call    free_big
        jmp     .nomem_dll
@@:
        push    eax
        push    eax
        push    dword [ebx+16]
        call    unpack
        push    dword [ebx+16]
	call	free_big
        pop     eax
        mov     [ebx+16], eax
.notpacked:
        add     esp, 28+40
; eax contains pointer to loaded file
        mov     ebx, eax
; 1. Calculate image size & allocate memory for image
        movzx   ecx, word [eax+2]
        lea     edx, [eax+20+16]
        xor     esi, esi
@@:
        add     esi, [edx]
	add	esi, 0xF
	and	esi, not 0xF
        add     edx, 40
        loop    @b
        pushad
        call    .initialize_heap
        mov     ecx, [esp+4]
        call    .allocate_heap
        popad
        test    eax, eax
        jnz     @f
; no memory in user space, user has already been notified
        and     dword [esp+20h], 0
        ret
@@:
; eax = base addr in user space
; ebx = pointer to loaded DLL data
; 2. Copy image data
        mov     edi, eax
        add     edi, [base]
        movzx   ecx, word [ebx+2]
        lea     edx, [ebx+20+16]
@@:
        mov     [edx+12-16], edi
        mov     esi, [edx+20-16]
        test    esi, esi
        jnz     .copy
        add     edi, [edx+16-16]
        jmp     .next
.copy:
        add     esi, ebx
        push    ecx
        mov     ecx, [edx+16-16]
        rep     movsb
        pop     ecx
.next:
        add     edi, 0xF
        and     edi, not 0xF
        add     edx, 40
        loop    @b
; 3. Fixup COFF symbols
; symbols table = ebx + [ebx+8]
; strings ptr = (symbols table)+[ebx+12]*18
        mov     ecx, [ebx+12]
        mov     edx, ebx
        add     edx, [ebx+8]
        lea     esi, [ecx+ecx*8]
	lea	esi, [edx+esi*2]
.fix_sym:
        movsx   edi, word [edx+12]
        test    edi, edi
        jnz     .internal
; current implementation does not support exports
; this is kernel limitation, not my
        and     dword [edx+8], 0
        jmp     .next_sym
.internal:
	js	.next_sym
        dec     edi
        shl     edi, 3
        lea     edi, [edi+edi*4]
        mov     edi, [ebx+20+edi+12]
        sub     edi, [base]
        add     [edx+8], edi
.next_sym:
        add     edx, 18
        loop    .fix_sym
; 4. Fixup COFF relocations
        movzx   ecx, word [ebx+2]
        lea     esi, [ebx+20]
.fix_sec_reloc:
        mov     edi, [esi+24]
        add     edi, ebx
        push    ecx
        movzx   ecx, word [esi+32]
        jecxz   .next_sec_reloc
.fix_reloc:
        mov     edx, [edi+4]
        lea     edx, [edx+edx*8]
        lea     edx, [ebx+edx*2]
        add     edx, [ebx+8]
        mov     edx, [edx+8]
        mov     eax, [edi]
        add     eax, [esi+12]
        cmp     word [edi+8], 6
        jz      .dir32
        cmp     word [edi+8], 20
        jz      .rel32
        pushad
        push    30h
        push    aWarning
        push    aUnknownReloc
        push    0
        call    [MessageBoxA]
        popad
        jmp     .next_reloc
.rel32:
        sub     edx, eax
        sub     edx, 4
.dir32:
        add     [eax], edx
.next_reloc:
        add     edi, 10
        loop    .fix_reloc
.next_sec_reloc:
        add     esi, 40
        pop     ecx
        loop    .fix_sec_reloc
; 5. Lookup 'EXPORTS' symbol
        mov     eax, ebx
        add     eax, [ebx+8]
        mov     ecx, [ebx+12]
.find_exp:
        cmp     dword [eax], 'EXPO'
        jnz     @f
        cmp     dword [eax+4], 'RTS'
        jz      .found_exp
@@:
        add     eax, 18
        loop    .find_exp
; 5b. Lookup '_EXPORTS' symbol
	mov	eax, ebx
	add	eax, [ebx+8]
	mov	ecx, [ebx+12]
.find__exp:
	cmp	dword [eax], '_EXP'
	jnz	@f
	cmp	dword [eax+4], 'ORTS'
	jz	.found_exp
@@:
	add	eax, 18
	loop	.find__exp
        push    30h
        push    aWarning
        push    aExportsNotFound
        push    0
        call    [MessageBoxA]
        xor     eax, eax
        jmp     @f
.found_exp:
        mov     eax, [eax+8]
@@:
        mov     [esp+20h], eax
        push    ebx
        call    free_big
        ret
.not19:
        cmp     ebx, 20
        jnz     .not20
; reallocate memory block in heap
        cmp     [heap_status], 0
        jnz     @f
        push    30h
        push    aWarning
        push    aHeapNotInited
        push    0
        call    [MessageBoxA]
        and     dword [esp+20h], 0
        ret
@@:
        cmp     [heap_status], 1
        jnz     @f
        push    ecx edx
        push    5
        call    [Sleep]
        pop     edx ecx
        jmp     @b
@@:
; realloc(NULL,sz) = malloc(sz)
        test    edx, edx
        jz      .allocate_heap
        cmp     edx, [heap_region_size]
        jae     .inv_exit_realloc
        sub     edx, [heap_start]
        jb      .inv_exit_realloc
        cmp     edx, 0x1000
        jb      .inv_exit_realloc
        push    ecx edx
        push    heap_critical_sec
        call    [EnterCriticalSection]
        pop     edx ecx
        test    edx, 0xFFF
        jnz     @f
        shr     edx, 12
        dec     edx
        mov     esi, [heap_control_block]
        mov     eax, [esi+edx*4]
	test	al, 10h
	jnz	@f
        test    al, 8
        jnz     .realloc_valid
@@:
        push    heap_critical_sec
        call    [LeaveCriticalSection]
        jmp     .inv_exit_realloc
.realloc_valid:
        add     ecx, 0x1FFF
        and     ecx, not 0xFFF
        and     eax, not 0xFFF
        cmp     eax, ecx
        jb      .realloc_inc
        jz      .realloc_done
        sub     eax, ecx
        push    eax ecx edx
        push    0x4000  ; MEM_DECOMMIT
        push    eax
        shl     edx, 12
        add     edx, ecx
        add     edx, [heap_start]
        add     edx, [base]
        push    edx
        call    [VirtualFree]
        test    eax, eax
        jz      $
        pop     edx ecx
        mov     eax, ecx
        cmp     eax, 0x1000
        jz      .realloc_freeall
        or      al, 8
        mov     [esi+edx*4], eax
        pop     eax
        shr     ecx, 12
        add     edx, ecx
        mov     edi, [heap_region_size]
        sub     edi, [heap_start]
        shr     edi, 12
        push    edx
        mov     edx, eax
        shr     edx, 12
        add     edx, [esp]
@@:
        cmp     edx, edi
        jae     @f
        mov     ecx, [esi+edx*4]
        test    cl, 4
        jz      @f
        and     dword [esi+edx*4], 0
        and     ecx, not 0xFFF
        add     eax, ecx
        shr     ecx, 12
        add     edx, ecx
        jmp     @b
@@:
        pop     edx
        or      al, 4
        mov     [esi+edx*4], eax
.realloc_done:
        push    heap_critical_sec
        call    [LeaveCriticalSection]
        mov     eax, [esp+18h]
        mov     [esp+20h], eax
        ret
.realloc_freeall:
        pop     eax
        xor     ecx, ecx
        xor     eax, eax
@@:
        cmp     ecx, edx
        jae     @f
        mov     eax, [esi+ecx*4]
        shr     eax, 12
        add     ecx, eax
        jmp     @b
@@:
        test    eax, eax
        jz      @f
        sub     ecx, eax
        test    byte [esi+ecx*4], 4
        jz      @f
        xor     eax, eax
        xchg    eax, [esi+edx*4]
        and     eax, not 0xFFF
        add     [esi+ecx*4], eax
        mov     edx, ecx
@@:
        mov     eax, [esi+edx*4]
        shr     eax, 12
        add     eax, edx
        mov     edi, [heap_region_size]
        sub     edi, [heap_start]
        shr     edi, 12
        cmp     eax, edi
        jae     @f
        test    byte [esi+eax*4], 4
        jz      @f
        xor     ecx, ecx
        xchg    ecx, [esi+eax*4]
        and     ecx, not 0xFFF
        add     [esi+edx*4], ecx
@@:
        mov     byte [esi+edx*4], 4
        push    heap_critical_sec
        call    [LeaveCriticalSection]
        and     dword [esp+20h], 0
        ret
.realloc_inc:
        mov     edi, [heap_region_size]
        sub     edi, [heap_start]
        shr     edi, 12
        push    eax
        shr     eax, 12
        add     eax, edx
        cmp     eax, edi
        jae     .realloc_realloc
        mov     eax, [esi+eax*4]
        test    al, 4
        jz      .realloc_realloc
        and     eax, not 0xFFF
        add     eax, [esp]
        sub     eax, ecx
        jb      .realloc_realloc
; reallocate in place
        jz      @f
        push    ecx edx
        shr     ecx, 12
        add     edx, ecx
        or      al, 4
        mov     [esi+edx*4], eax
        pop     edx ecx
@@:
        pop     eax
        sub     ecx, eax
        add     [esi+edx*4], ecx
        mov     edi, edx
        inc     edi
        shl     edi, 12
        add     edi, [heap_start]
        mov     [esp+20h], edi
        push    40h     ; PAGE_EXECUTE_READWRITE
        push    1000h   ; MEM_COMMIT
        push    ecx
        shl     edx, 12
        add     edx, eax
        add     edx, [heap_start]
        add     edx, [base]
        push    edx
        call    [VirtualAlloc]
        test    eax, eax
        jz      .nomem_fatal
        push    heap_critical_sec
        call    [LeaveCriticalSection]
        ret
.realloc_realloc:
        pop     esi
        push    ecx
        push    heap_critical_sec
        call    [LeaveCriticalSection]
        pop     ecx
        pushad
        call    .allocate_heap
        popad
        mov     [esp+20h], eax
        mov     edi, eax
        test    edi, edi
        jnz     @f
        ret
@@:
        lea     ecx, [esi-0x1000]
        mov     esi, [esp+18h]
        push    esi
        add     esi, [base]
        add     edi, [base]
        shr     ecx, 2
        rep     movsd
        pop     ecx
        pushad
        call    .free_heap
        popad
        ret
.not20:
	cmp	ebx, 22
	jnz	.not22
; open/create named memory area
	test	esi, not 0xD
	jnz	not_supported_i40_fn
	add	ecx, [base]
	mov	edi, ecx
..server_create_shmem:
	div	edx
	test	eax, eax
	jz	@f
.shmem.err:
	and	dword [esp+20h], 0
	mov	[esp+18h], eax
	ret
@@:
	pushad
	call	.allocate_heap
	popad
	test	eax, eax
	jnz	@f
	mov	ecx, edi
	call	..server_destroy_shmem
	mov	al, 30
	jmp	.shmem.err
@@:
	mov	[esp+20h], eax
	mov	[esp+18h], edx
	mov	ecx, eax
	sub	ecx, [heap_start]
	shr	ecx, 12
	mov	edx, [heap_control_block]
	or	byte [edx+(ecx-1)*4], 10h
	add	eax, [base]
..server_notify_shmem:
	div	edx
	push	eax
	push	esp
	push	1	; PAGE_NOACCESS
	push	ecx
	push	eax
	call	[VirtualProtect]
	pop	eax
	ret
.not22:
	cmp	ebx, 23
	jnz	not_supported_i40_fn
	add	ecx, [base]
	call	..server_destroy_shmem
	jecxz	@f
	sub	ecx, [base]
	mov	eax, ecx
	sub	eax, [heap_start]
	shr	eax, 12
	mov	edx, [heap_control_block]
	and	byte [edx+(eax-1)*4], not 10h
	pushad
	call	.free_heap
	popad
	jmp	.ret22
@@:
	push	30h
	push	aWarning
	push	aCannotDestroyShMem
	push	0
	call	[MessageBoxA]
.ret22:
	mov	dword [esp+20h], 0x87654320
	ret
..server_destroy_shmem:
	div	edx
	ret

include 'unpacker.inc'

align 4
debug_services_table:
	dd	i40_debug_services.set_event_data
	dd	i40_debug_services.getcontext
	dd	i40_debug_services.setcontext
	dd	not_supported_i40_fn	;i40_debug_services.detach
	dd	i40_debug_services.suspend
	dd	i40_debug_services.resume
	dd	i40_debug_services.read_process_memory
	dd	i40_debug_services.write_process_memory
	dd	i40_debug_services.terminate
;	dd	i40_debug_services.set_drx
.size = ($ - debug_services_table)/4

i40_debug_services:
	cmp	ebx, debug_services_table.size
	jae	not_supported_i40_fn
	jmp	dword [debug_services_table+ebx*4]
.set_event_data:
	call	get_cur_slot_ptr
; atomic operation, no synchronization required
	mov	[edi+shared_data_struc.debugger_mem-shared_data_struc.threads], ecx
	ret
.m1pop2:
	pop	eax
.m1pop:
	pop	eax
.m1:
	or	dword [esp+20h], -1
.ret:
	ret
.getcontext:
	call	find_debuggee
	test	ebx, ebx
	jz	.ret
	cmp	edx, 40
	jnz	.ret
	push	dword [ebx+12]
	call	suspend_safe
	sub	esp, 0xB2*4
	push	1000Fh
	mov	edi, esp
	push	edi
	push	dword [ebx+12]
	call	[GetThreadContext]
	add	esi, [base]
	mov	ax, cs
	cmp	word [edi+0xBC], ax
	jz	.context_win
	mov	eax, [edi+0xB8]
	mov	[esi], eax
	mov	eax, [edi+0xC0]
	mov	[esi+4], eax
	mov	eax, [edi+0xB0]
	mov	[esi+8], eax
	mov	eax, [edi+0xAC]
	mov	[esi+12], eax
	mov	eax, [edi+0xA8]
	mov	[esi+16], eax
	mov	eax, [edi+0xA4]
	mov	[esi+20], eax
	mov	eax, [edi+0xC4]
	mov	[esi+24], eax
	mov	eax, [edi+0xB4]
	mov	[esi+28], eax
	mov	eax, [edi+0xA0]
	mov	[esi+32], eax
	mov	eax, [edi+0x9C]
	mov	[esi+36], eax
	jmp	.gotcontext
.context_win:
	mov	eax, [ebx+16]
	add	eax, tls._esp
	sub	esp, 2Ch
	mov	ecx, esp
	push	0
	push	esp
	push	8
	push	ecx
	push	eax
	push	dword [ebx+8]
	call	[ReadProcessMemory]
	pop	eax
	pop	dword [esi+24]
	pop	dword [esi]
	mov	ecx, esp
	push	0
	push	esp
	push	24h
	push	ecx
	push	dword [ebx+20]
	push	dword [ebx+8]
	call	[ReadProcessMemory]
	pop	eax
	pop	dword [esi+36]
	pop	dword [esi+32]
	pop	dword [esi+28]
	pop	eax
	pop	dword [esi+20]
	pop	dword [esi+16]
	pop	dword [esi+12]
	pop	dword [esi+8]
	pop	dword [esi+4]
.gotcontext:
	add	esp, 0xB3*4
@@:
	push	dword [ebx+12]
	call	[ResumeThread]
	ret
.resume:
	call	find_debuggee
	test	ebx, ebx
	jz	.ret
	jmp	@b
.suspend:
	call	find_debuggee
	test	ebx, ebx
	jz	.ret
	push	dword [ebx+12]
	call	suspend_safe
	ret
.setcontext:
	call	find_debuggee
	test	ebx, ebx
	jz	.ret
	cmp	edx, 40
	jnz	.ret
	push	dword [ebx+12]
	call	suspend_safe
	sub	esp, 0xB2*4
	push	1000Fh
	mov	edi, esp
	push	edi
	push	dword [ebx+12]
	call	[GetThreadContext]
	add	esi, [base]
	mov	ax, cs
	cmp	word [edi+0xBC], ax
	jz	.context_win_set
	lodsd
	mov	[edi+0xB8], eax
	lodsd
	mov	[edi+0xC0], eax
	lodsd
	mov	[edi+0xB0], eax
	lodsd
	mov	[edi+0xAC], eax
	lodsd
	mov	[edi+0xA8], eax
	lodsd
	mov	[edi+0xA4], eax
	lodsd
	mov	[edi+0xC4], eax
	lodsd
	mov	[edi+0xB4], eax
	lodsd
	mov	[edi+0xA0], eax
	lodsd
	mov	[edi+0x9C], eax
	push	edi
	push	dword [ebx+12]
	call	[SetThreadContext]
	jmp	.setcontextdone
.context_win_set:
	mov	edx, [ebx+16]
	add	edx, tls._esp
	lodsd
	push	eax
	push	dword [esi-4+24]
	mov	ecx, esp
	push	0
	push	esp
	push	8
	push	ecx
	push	eax
	push	dword [ebx+8]
	call	[WriteProcessMemory]
	pop	eax
	pop	eax
	pop	eax
	push	9
	pop	ecx
@@:
	lodsd
	push	eax
	loop	@b
	mov	ecx, esp
	push	0
	push	esp
	push	24h
	push	ecx
	push	dword [ebx+20]
	push	dword [ebx+8]
	call	[WriteProcessMemory]
	add	esp, 4+24h
.setcontextdone:
	add	esp, 0xB3*4
	push	dword [ebx+12]
	call	[ResumeThread]
	ret
.write_process_memory:
	push	[WriteProcessMemory]
	jmp	@f
.read_process_memory:
	push	[ReadProcessMemory]
@@:
	call	find_debuggee
	test	ebx, ebx
	jz	.m1pop
; get base and limit of target process
	push	eax
	push	eax
	mov	eax, esp
	push	edx
	push	0
	push	esp
	push	8
	push	eax
	push	base
	push	dword [ebx+8]
	call	[ReadProcessMemory]
	pop	eax
	pop	edx
	pop	ecx	; ecx = [base] for target process
	cmp	eax, 8
	jnz	.m1pop2
	pop	eax	; eax = [limit] for target process
	push	edx
	inc	eax
	add	edx, esi
	sub	edx, eax
	jc	@f
	sub	dword [esp], edx
	jnc	@f
	and	dword [esp], 0
@@:
	add	esi, ecx
	pop	edx
	pop	ecx
	xor	eax, eax
	test	edx, edx
	jz	@f
	push	-1
	push	esp
	add	edi, [base]
	push	edx
	push	edi
	push	esi
	push	dword [ebx+8]
	call	ecx
	pop	eax
@@:
	mov	[esp+20h], eax
	ret
.terminate:
	call	find_debuggee
	test	ebx, ebx
	jnz	i40_sys_service.kill
	ret
.set_drx:
	sub	esp, 0xB2*4
	push	1001Fh
	call	find_debuggee
	test	ebx, ebx
	jz	.ret1
	mov	edi, esp
	push	ecx edx
	push	edi
	push	dword [ebx+12]
	call	[GetThreadContext]
	pop	edx ecx
	test	dh, dh
	js	.clear
; set hardware breakpoint, dl=index (0..3), dh=(length*4)+(condition), esi=address
	cmp	esi, 0x80000000
	jae	.ret1
	cmp	dl, 4
	jae	.ret1
	test	dh, 0xF0
	jnz	.ret1
	mov	al, dh
	and	al, 3
	cmp	al, 2
	jz	.ret1
	movzx	eax, dh
	shr	al, 2
	cmp	al, 2
	jz	.ret1
	test	esi, eax
	jnz	.ret1
	push	edx
	push	0
	mov	eax, esp
	push	eax
	push	esp
	push	4
	push	eax
	push	base
	push	dword [ebx+8]
	call	[ReadProcessMemory]
	pop	eax
	pop	eax
	pop	edx
	add	esi, eax
	or	byte [edi+18h+1], 3	; set GE and LE flags
	movzx	eax, dh
	movzx	ecx, dl
	add	ecx, ecx
	bts	dword [edi+18h], ecx	; set L<i> flag
	mov	[edi+4+ecx*2], esi	; set DR<i>
	shl	eax, cl
	mov	edx, 0xF
	shl	edx, cl
	not	edx
	and	[edi+18h+2], dx
	or	[edi+18h+2], ax		; set R/W and LEN fields
	jmp	.set_and_ret
.clear:
	movzx	ecx, dl
	add	ecx, ecx
	btr	dword [edi+18h], ecx	; clear L<i> flag
.set_and_ret:
	push	edi
	push	dword [ebx+12]
	call	[SetThreadContext]
	add	esp, 0xB3*4
	and	dword [esp+20h], 0
	ret
.ret1:
	add	esp, 0xB3*4
	push	30h
	push	aWarning
	push	aInvalidDataForDR
	push	0
	call	[MessageBoxA]
	mov	dword [esp+20h], 1
	ret

find_debuggee:
	lea	ebx, [ebp+tls.debuggees]
@@:
	mov	ebx, [ebx]
	test	ebx, ebx
	jz	@f
	cmp	dword [ebx+4], ecx
	jnz	@b
@@:
	ret

suspend_safe:
	xchg	ebx, [esp+4]
.redo:
	push	ebx
	call	[SuspendThread]
	sub	esp, 0xB2*4
	push	1000Fh
	push	esp
	push	ebx
	call	[GetThreadContext]
	mov	ax, cs
	cmp	word [context+0xBC], ax
	jnz	.ok
	cmp	dword [context+0xB8], i40_nt
	jz	.wait
	cmp	dword [context+0xB8], i40_9x
	jb	.test2
	cmp	dword [context+0xB8], safe_to_suspend
	ja	.test2
.wait:
	add	esp, 0xB3*4
	push	ebx
	call	[ResumeThread]
	push	50
	call	[Sleep]
	jmp	.redo
.test2:
	cmp	dword [context+0xB8], i40_done
	jb	.ok
	cmp	dword [context+0xB8], not_supported_i40_fn
	jb	.wait
.ok:
	add	esp, 0xB3*4
	xchg	ebx, [esp+4]
	ret	4

rootdirs:
        db      2,'rd'
        dd      fs_OnRamdisk
        dd      fs_NextRamdisk
        db      7,'ramdisk'
        dd      fs_OnRamdisk
        dd      fs_NextRamdisk
;        db      2,'fd'
;        dd      fs_OnFloppy
;        dd      fs_NextFloppy
;        db      10,'floppydisk'
;        dd      fs_OnFloppy
;        dd      fs_NextFloppy
        db      3,'hd0'
        dd      fs_OnHd0
        dd      fs_NextHd0
        db      3,'hd1'
        dd      fs_OnHd1
        dd      fs_NextHd1
        db      3,'hd2'
        dd      fs_OnHd2
        dd      fs_NextHd2
        db      3,'hd3'
        dd      fs_OnHd3
        dd      fs_NextHd3
        db      0

virtual_root_query:
        dd      fs_HasRamdisk
        db      'rd',0
;        dd      fs_HasFloppy
;        db      'fd',0
        dd      fs_HasHd0
        db      'hd0',0
        dd      fs_HasHd1
        db      'hd1',0
        dd      fs_HasHd2
        db      'hd2',0
        dd      fs_HasHd3
        db      'hd3',0
        dd      0

i40_file_system_lfn:
        add     ebx, [base]
.noadd:
        push    0
; parse file name
        lea     esi, [ebx+20]
        lodsb
        test    al, al
        jnz     @f
        mov     esi, [esi]
        add     esi, [base]
        lodsb
        jmp     @f
.parse:
        push    edi
        lodsb
@@:
	push	0
        cmp     al, '/'
        jz      @f
	dec	esi
	mov	dword [esp], esi
	mov	esi, [ebp+tls.cur_dir]
	inc	esi
	jmp	@f
.notfound:
	pop	eax
        pop     eax
        test    eax, eax
        mov     eax, 5                  ; file not found
        jnz     .notfound_retparse
        mov     [esp+20h], eax
.notfound_retparse:
        ret
@@:
	mov	eax, [esi]
	or	eax, '   '
	cmp	eax, 'sys'
	jz	.sys1
	cmp	eax, 'sys/'
	jnz	@f
	inc	esi
.sys1:
	mov	eax, ramdisk_path
	add	esi, 3
	jmp	fs_common
@@:
        cmp     byte [esi], 0
        jz      .rootdir
        mov     edi, rootdirs-8
        xor     ecx, ecx
        push    esi
.scan1:
        pop     esi
        add     edi, ecx
        scasd
        scasd
        mov     cl, byte [edi]
        jecxz   .notfound
        inc     edi
        push    esi
@@:
        lodsb
        or      al, 20h
        scasb
        loopz   @b
        jnz     .scan1
        lodsb
        cmp     al, '/'
        jz      .found1
        test    al, al
        jnz     .scan1
        pop     eax
; directory /xxx
.maindir:
	pop	eax
        pop     eax
        test    eax, eax
        jz      @f
.parse_access_denied:
        mov     eax, 10         ; access denied
        ret
@@:
        cmp     dword [ebx], 1
        jnz     .access_denied
        xor     eax, eax
        push    dword [ebx+12]
        mov     edx, [ebx+16]
        add     edx, [base]
        push    dword [ebx+4]   ; first block
        mov     ebx, [ebx+8]    ; flags
        and     ebx, 1
        mov     esi, [edi+4]
; [esp]=first block, [esp+4]=number of blocks, ebx=flags, edx=return area, esi='Next' handler
        mov     edi, edx
        mov     ecx, 32/4
        rep     stosd
        mov     byte [edx], 1   ; version
.maindir_loop:
        call    esi
        jc      .maindir_done
        inc     dword [edx+8]
        dec     dword [esp]
        jns     .maindir_loop
        dec     dword [esp+4]
        js      .maindir_loop
        inc     dword [edx+4]
        mov     dword [edi], 0x10       ; attributes: folder
        mov     dword [edi+4], ebx      ; name type
        push    eax
        xor     eax, eax
        add     edi, 8
        mov     ecx, 40/4-2
        rep     stosd
        pop     eax
        push    eax edx
; convert number in eax to decimal ANSI/UNICODE string
        push    edi
        push    -'0'
        mov     cl, 10
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     al, '0'
        stosb
        pushf
        test    bl, 1
        jz      .ansi_name
        xor     eax, eax
        stosb
.ansi_name:
        popf
        jnz     @b
        pop     edi
        add     edi, 520
        test    bl, 1
        jnz     @f
        sub     edi, 520-264
@@:
        pop     edx eax
        jmp     .maindir_loop
.maindir_done:
        mov     ebx, [edx+4]
        xor     eax, eax
        pop     ecx ecx
        dec     ecx
        js      @f
        mov     al, 6
@@:
        mov     [esp+20h], eax
        mov     [esp+14h], ebx
        ret
; directory /
.rootdir:
	pop	eax
        pop     eax
        test    eax, eax
        jnz     .parse_access_denied
        cmp     dword [ebx], 1  ; read folder?
        jz      .readroot
.access_denied:
        mov     dword [esp+20h], 10      ; access denied
        ret

.readroot:
; virtual root folder - special handler
        mov     esi, virtual_root_query
        push    dword [ebx+12]
        mov     edx, [ebx+16]
        add     edx, [base]
        push    dword [ebx+4]
        mov     ebx, [ebx+8]
        xor     eax, eax
; eax=0, [esp]=first block, [esp+4]=number of blocks, ebx=flags, edx=return area
        mov     edi, edx
        mov     ecx, 32/4
        rep     stosd
        mov     byte [edx], 1   ; version
.readroot_loop:
        cmp     dword [esi], eax
        jz      .readroot_done
        call    dword [esi]
        add     esi, 4
        test    eax, eax
        jnz     @f
.readroot_next:
        or      ecx, -1
        xchg    esi, edi
        repnz   scasb
        xchg    esi, edi
        jmp     .readroot_loop
@@:
        xor     eax, eax
        inc     dword [edx+8]
        dec     dword [esp]
        jns     .readroot_next
        dec     dword [esp+4]
        js      .readroot_next
        inc     dword [edx+4]
        mov     dword [edi], 0x10       ; attributes: folder
        mov     dword [edi+4], ebx      ; name type
        add     edi, 8
        push    40/4-2
        pop     ecx
        rep     stosd
        push    edi
@@:
        lodsb
        stosb
        test    bl, 1
        jz     .ansi_name2
        mov     byte [edi], 0
        inc     edi
.ansi_name2:
        test    eax, eax
        jnz     @b
        pop     edi
        add     edi, 520
        test    bl, 1
        jnz     @f
        sub     edi, 520-264
@@:
        jmp     .readroot_loop
.readroot_done:
        mov     ebx, [edx+4]
        xor     eax, eax
        pop     ecx ecx
        dec     ecx
        js      @f
        mov     al, 6
@@:
        mov     [esp+20h], eax
        mov     [esp+14h], ebx
        ret

.found1:
        pop     eax
        cmp     byte [esi], 0
        jz      .maindir
; read partition number
        xor     ecx, ecx
        xor     eax, eax
@@:
        lodsb
        cmp     al, '/'
        jz      .done1
        test    al, al
        jz      .done1
        sub     al, '0'
        cmp     al, 9
        ja      .notfound
        imul    ecx, 10
        add     ecx, eax
        jmp     @b
.done1:
        test    ecx, ecx
        jz      .notfound
        test    al, al
        jnz     @f
        dec     esi
@@:
; now [edi] contains handler address, ecx - partition number,
; esi points to ASCIIZ string - rest of name
        jmp     dword [edi]

fs_NextRamdisk:
        test    eax, eax
        stc
        jnz     @f
        mov     al, 1
        clc
@@:     ret

fs_NextHd0:
        push    0
        jmp     fs_NextHd
fs_NextHd1:
        push    1
        jmp     fs_NextHd
fs_NextHd2:
        push    2
        jmp     fs_NextHd
fs_NextHd3:
        push    3
fs_NextHd:
        pop     ecx
        mov     ecx, [hd_partitions_num+ecx*4]
        cmp     eax, ecx
        jae     .no
        inc     eax
        clc
        ret
.no:
        stc
        ret

fs_HasRamdisk:
        mov     al, 1
        ret

fs_HasHd0:
        cmp     [hd_partitions_num], 0
        setnz   al
        ret
fs_HasHd1:
        cmp     [hd_partitions_num+4], 0
        setnz   al
        ret
fs_HasHd2:
        cmp     [hd_partitions_num+8], 0
        setnz   al
        ret
fs_HasHd3:
        cmp     [hd_partitions_num+12], 0
        setnz   al
        ret

fs_OnRamdisk:
        cmp     ecx, 1
        jnz     i40_file_system_lfn.notfound
        mov     eax, ramdisk_path
        jmp     fs_common

fs_OnHd0:
        push    0
        jmp     fs_OnHd
fs_OnHd1:
        push    1
        jmp     fs_OnHd
fs_OnHd2:
        push    2
        jmp     fs_OnHd
fs_OnHd3:
        push    3
fs_OnHd:
        pop     eax
        cmp     ecx, [hd_partitions_num+eax*4]
        jbe     @f
        pop     eax eax
        test    eax, eax
        mov     eax, 5
.hderr:
        jnz     .ret
        mov     dword [esp+20h], eax
.ret:
        ret
@@:
	dec	ecx
        mov     eax, [hd_partitions_array+eax*4]
        shl     ecx, 9
        add     eax, ecx
        cmp     byte [eax+511], 0
        jz      fs_common
; readonly disk - allow only subfunctions 0,1,5,7
        cmp     dword [ebx], 0
        jz      fs_common
        cmp     dword [ebx], 1
        jz      fs_common
        cmp     dword [ebx], 5
        jz      fs_common
        cmp     dword [ebx], 7
        jz      fs_common
; access denied!
        pop     eax eax
        test    eax, eax
        mov     eax, 10
        jmp     .hderr
fs_common:
	pop	ecx
        pop     edi
        test    edi, edi
        jnz     @f
        sub     esp, 204h
        mov     edi, esp
@@:
	push	ecx
        push    eax
        push    edi
        call    [lstrcpyA]
        push    edi
        call    [lstrlenA]
	pop	ecx
        push    edi
        add     edi, eax
        sub     eax, 204h
        neg     eax
	push	ecx
        push    eax
        push    esi
        push    edi
        call    [lstrcpynA]
	pop	ecx
	jecxz	.slash_pass
	cmp	byte [ecx], 0
	jz	.slash_pass
	mov	esi, ecx
	push	edi
	call	[lstrlenA]
	add	edi, eax
	mov	eax, edi
	sub	eax, [esp]
	sub	eax, 204h
	neg	eax
	push	eax
	push	esi
	push	edi
	call	[lstrcpynA]
.slash_pass:
        mov     al, [edi]
        test    al, al
        jz      .slash_pass_done
        cmp     al, '\'
        jz      .invalid
        cmp     al, '/'
        jnz     @f
        mov     al, '\'
@@:
        stosb
        jmp     .slash_pass
.slash_pass_done:
; delete trailing slashes, if present
        dec     edi
        cmp     byte [edi], '\'
        jz      .slash_pass_done
        inc     edi
        stosb
; but do not delete one slash in X:\
        pop     edi
        cmp     byte [edi+1], ':'
        jnz     @f
        cmp     byte [edi+2], 0
        jnz     @f
        mov     word [edi+2], '\'
@@:
        push    edi
        push    edi
        call    [OemToCharA]
; now edi -> Win32 path
        cmp     edi, esp
        jz      fs_do
        xor     eax, eax
        ret
.invalid:
        pop     edi
        cmp     edi, esp
        jnz     @f
        add     esp, 204h
        mov     dword [esp+20h], 5      ; file not found
        ret
@@:
        push    5
        pop     eax
        ret
fs_do:
        cmp     dword [ebx], 10
        jb      @f
fs_unsupported:
        add     esp, 204h
        jmp     not_supported_i40_fn
@@:
        push    edi
        call    [GetFileAttributesA]
        mov     ecx, dword [ebx]
        jmp     dword [i40_fs_subfunctions + ecx*4]

i40_fs_subfunctions:
        dd      fs_read
        dd      fs_read_folder
        dd      fs_rewrite
        dd      fs_write
        dd      fs_setsize
        dd      fs_getattr
        dd      fs_setattr
        dd      fs_execute
        dd      fs_delete
        dd      fs_makedir

fs_read:
        inc     eax
        jnz     @f
        add     esp, 204h
        mov     dword [esp+20h], 5      ; file not found
        or      dword [esp+14h], -1
        ret
@@:
        dec     eax
        test    al, 10h         ; FILE_ATTRIBUTE_DIRECTORY
        jz      @f
        add     esp, 204h
.access_denied:
        mov     dword [esp+20h], 10     ; access denied
        or      dword [esp+14h], -1
        ret
@@:
        push    0
        push    0
        push    3
        push    0
        push    1
        push    80000000h
        push    edi
        call    [CreateFileA]
        add     esp, 204h
        inc     eax
        jz      .access_denied
        dec     eax
        xchg    eax, esi
        push    dword [ebx+8]
        mov     eax, esp
        push    0
        push    eax
        push    dword [ebx+4]
        push    esi
        call    [SetFilePointer]
        inc     eax
        jnz     @f
        call    [GetLastError]
        test    eax, eax
        jz      @f
        pop     eax
        mov     dword [esp+20h], 6      ; EOF
        and     dword [esp+14h], 0      ; no bytes read
        push    esi
        call    [CloseHandle]
        ret
@@:
        mov     eax, esp
        push    0
        push    eax
;        mov     eax, [limit]
;        inc     eax
;        sub     eax, [ebx+16]
;        cmp     eax, [ebx+12]
;        jb      @f
;        mov     eax, [ebx+12]
;@@:
;        push    eax
        push    dword [ebx+12]
        mov     eax, [ebx+16]
        add     eax, [base]
        push    eax
        push    esi
        call    [ReadFile]
        pop     ecx
        xor     eax, eax
        cmp     ecx, [ebx+12]
        jz      @f
        mov     al, 6   ; EOF
@@:
        mov     [esp+20h], eax
        mov     [esp+14h], ecx  ; bytes read
        push    esi
        call    [CloseHandle]
        ret

fs_read_folder:
        inc     eax
        jnz     @f
        add     esp, 204h
        mov     dword [esp+20h], 5      ; file not found
        or      dword [esp+14h], -1
        ret
@@:
        push    edi
        call    [lstrlenA]
        cmp     eax, 202h
        jb      @f
        add     esp, 204h
        mov     dword [esp+20h], 5
        or      dword [esp+14h], -1
        ret
@@:
        cmp     byte [edi+eax-1], '/'
        jz      @f
        cmp     byte [edi+eax-1], '\'
        jz      @f
        mov     byte [edi+eax], '/'
        inc     eax
@@:
        mov     word [edi+eax], '*'
        cmp     byte [esi], 0
        mov     esi, 2
        jz      @f
        xor     esi, esi
@@:
        sub     esp, 140h       ; sizeof(WIN32_FIND_DATAA)
        push    esp
        push    edi
        call    [FindFirstFileA]
        inc     eax
        jnz     @f
        add     esp, 140h+204h
        mov     dword [esp+20h], 10     ; access denied
        or      dword [esp+14h], -1
        ret
@@:
        dec     eax
        push    dword [ebx+8]   ; flags
        push    dword [ebx+4]   ; 1st block
        push    dword [ebx+12]  ; number of blocks
        mov     edi, [ebx+16]   ; buffer
        add     edi, [base]
        push    edi
        xchg    eax, ebx
        mov     ecx, 32/4
        xor     eax, eax
        rep     stosd
        mov     byte [edi-32], 1        ; version
.loop:
        test    esi, 2
        jz      .noroot
        cmp     word [esp+16+44], '.'
        jz      .cont
        cmp     word [esp+16+44], '..'
        jnz     .noroot
        cmp     byte [esp+16+46], 0
        jz      .cont
.noroot:
        pop     eax
        push    eax
        inc     dword [eax+8]   ; new file found
        dec     dword [esp+8]
        jns     .cont
        dec     dword [esp+4]
        js      .cont
        inc     dword [eax+4]   ; new file copied
        mov     eax, [esp+16]   ; attributes
        and     eax, 3Fh
        stosd
        mov     al, [esp+12]    ; name type
        and     eax, 1
        stosd
        lea     eax, [esp+16+4]
        call    get_bdfe_time
        lea     eax, [esp+16+12]
        call    get_bdfe_time
        lea     eax, [esp+16+20]
        call    get_bdfe_time
        mov     eax, [esp+16+32]
        stosd
        mov     eax, [esp+16+28]
        stosd
        lea     eax, [esp+16+44]
        test    byte [esp+12], 1
        jz      .name_ansi
        push    260
        push    edi
        push    -1
        push    eax
        push    0
        push    0
        call    [MultiByteToWideChar]
        add     edi, 260*2
        jmp     .cont
.name_ansi:
        push    edi
        push    eax
        call    [CharToOemA]
        add     edi, 264
.cont:
        lea     eax, [esp+16]
        push    eax
        push    ebx
        call    [FindNextFileA]
        test    eax, eax
        jnz     .loop
        push    ebx
        call    [FindClose]
        pop     edx
        mov     ebx, [edx+4]
        xor     eax, eax
        pop     ecx
        dec     ecx
        js      @f
        mov     al, 6
@@:
        add     esp, 140h+204h+8
        mov     [esp+20h], eax
        mov     [esp+14h], ebx
        ret

get_bdfe_time:
        sub     esp, 10h
        push    esp
        push    eax
        call    [FileTimeToSystemTime]
        mov     ax, [esp+8]
        shl     eax, 16
        mov     ah, [esp+10]
        mov     al, [esp+12]
        stosd
        mov     ax, [esp+0]
        shl     eax, 16
        mov     ah, [esp+2]
        mov     al, [esp+6]
        stosd
        add     esp, 10h
        ret
from_bdfe_time:
        sub     esp, 10h
        xor     eax, eax
        lodsb
        mov     [esp+12], ax
        lodsb
        mov     [esp+10], ax
        lodsw
        mov     [esp+8], ax
        mov     ah, 0
        lodsb
        mov     [esp+6], ax
        lodsb
        mov     [esp+2], ax
        lodsw
        mov     [esp+0], ax
        mov     eax, esp
        push    edi
        push    eax
        call    [SystemTimeToFileTime]
        add     esp, 10h
        add     edi, 8
        ret

fs_rewrite:
        push    0
        push    80h
        push    2
        push    0
        push    0
        push    40000000h
        push    edi
        call    [CreateFileA]
        add     esp, 204h
        inc     eax
        jnz     @f
        mov     dword [esp+20h], 10
        and     dword [esp+14h], 0
        ret
@@:
        dec     eax
        xchg    eax, esi
        push    eax
        mov     eax, esp
        push    0
        push    eax
        push    dword [ebx+12]
        mov     eax, [ebx+16]
        add     eax, [base]
        push    eax
        push    esi
        call    [WriteFile]
        pop     ebx
        push    esi
        call    [CloseHandle]
        and     dword [esp+20h], 0
        mov     [esp+14h], ebx
        ret

fs_write:
        inc     eax
        jnz     @f
        add     esp, 204h
        mov     dword [esp+20h], 5      ; file not found
        or      dword [esp+14h], -1
        ret
@@:
        dec     eax
        test    al, 10h         ; FILE_ATTRIBUTE_DIRECTORY
        jz      @f
        add     esp, 204h
.access_denied:
        mov     dword [esp+20h], 10     ; access denied
        or      dword [esp+14h], -1
        ret
@@:
        push    0
        push    0
        push    3
        push    0
        push    1
        push    40000000h
        push    edi
        call    [CreateFileA]
        add     esp, 204h
        inc     eax
        jz      .access_denied
        dec     eax
        xchg    eax, esi
        push    dword [ebx+8]
        mov     eax, esp
        push    0
        push    eax
        push    dword [ebx+4]
        push    esi
        call    [SetFilePointer]
        inc     eax
        jnz     @f
        call    [GetLastError]
        test    eax, eax
        jz      @f
        pop     eax
        mov     dword [esp+20h], 6      ; EOF
        and     dword [esp+14h], 0      ; no bytes read
        push    esi
        call    [CloseHandle]
        ret
@@:
        mov     eax, esp
        push    0
        push    eax
        mov     eax, [limit]
        inc     eax
        sub     eax, [ebx+16]
        cmp     eax, [ebx+12]
        jb      @f
        mov     eax, [ebx+12]
@@:
        push    eax
        mov     eax, [ebx+16]
        add     eax, [base]
        push    eax
        push    esi
        call    [WriteFile]
        pop     ecx
        xor     eax, eax
        cmp     ecx, [ebx+12]
        jz      @f
        mov     al, 6   ; EOF
@@:
        mov     [esp+20h], eax
        mov     [esp+14h], ecx  ; bytes read
        push    esi
        call    [CloseHandle]
        ret

fs_setsize:
        inc     eax
        jnz     @f
        add     esp, 204h
        mov     dword [esp+20h], 5      ; file not found
        or      dword [esp+14h], -1
        ret
@@:
        dec     eax
        test    al, 10h         ; FILE_ATTRIBUTE_DIRECTORY
        jz      @f
        add     esp, 204h
.access_denied:
        mov     dword [esp+20h], 10     ; access denied
        or      dword [esp+14h], -1
        ret
@@:
        push    0
        push    0
        push    3
        push    0
        push    1
        push    40000000h
        push    edi
        call    [CreateFileA]
        add     esp, 204h
        inc     eax
        jz      .access_denied
        dec     eax
        xchg    eax, esi
        push    dword [ebx+8]
        mov     eax, esp
        push    0
        push    eax
        push    dword [ebx+4]
        push    esi
        call    [SetFilePointer]
        inc     eax
        jnz     @f
        call    [GetLastError]
        test    eax, eax
        jz      @f
        pop     eax
        mov     dword [esp+20h], 6      ; EOF
        mov     dword [esp+14h], 1234567h       ; destroy ebx
        push    esi
        call    [CloseHandle]
        ret
@@:
        pop     ecx
        push    esi
        call    [SetEndOfFile]
        and     dword [esp+20h], 0
        mov     dword [esp+14h], 234567h        ; destroy ebx
        push    esi
        call    [CloseHandle]
        ret

fs_getattr:
        inc     eax
        jnz     @f
.notfound:
        add     esp, 204h
        mov     dword [esp+20h], 5      ; file not found
.ret_destroy_ebx:
        mov     dword [esp+14h], 12345678h
        ret
@@:
; Win32 does not allow to read attributes of root directories through FindFirstFile.
; Moreover, on FAT systems the root directory simply does not contain attributes.
; The current kernel implementation in Kolibri returns error eax = 2.
; Let it be.
        cmp     word [edi+1], ':\'
        jnz     @f
        cmp     byte [edi+3], 0
        jnz     @f
; The program requests root directory. Return eax=2.
; N.B. All previous words are related to the case when Kolibri-disk is mapped to
;      real disk, /hdi/j = N:\. Otherwise, it is possible to get real attributes
;      of root directory, and emulator will do so (in another branch).
        add     esp, 204h
        mov     dword [esp+20h], 2
        jmp     .ret_destroy_ebx
@@:
        sub     esp, 140h       ; sizeof(WIN32_FIND_DATAA)
        push    esp
        push    edi
        call    [FindFirstFileA]
        inc     eax
        jnz     @f
        add     esp, 140h+204h
        mov     dword [esp+20h], 10     ; access denied
        mov     dword [esp+14h], 12345678h
        ret
@@:
        dec     eax
        push    eax
        call    [FindClose]
        mov     edi, [ebx+16]
        add     edi, [base]
        pop     eax             ; attributes
        and     eax, 3Fh
        stosd
        xor     eax, eax
        stosd
        mov     eax, esp
        call    get_bdfe_time
        lea     eax, [esp+12-4]
        call    get_bdfe_time
        lea     eax, [esp+20-4]
        call    get_bdfe_time
        mov     eax, [esp+32-4]
        stosd
        mov     eax, [esp+28-4]
        stosd
        add     esp, 140h-4+204h
        and     dword [esp+20h], 0
        mov     dword [esp+14h], 12345678h
        ret

fs_setattr:
        inc     eax
        jz      fs_getattr.notfound
        mov     esi, [ebx+16]
        add     esi, [base]
        lodsd
        push    eax     ; remember attributes
        push    80h     ; FILE_ATTRIBUTE_NORMAL
        push    edi
        call    [SetFileAttributesA]
        push    0
        push    0
        push    3       ; OPEN_EXISTING
        push    0
        push    3       ; FILE_SHARE_READ | FILE_SHARE_WRITE
        push    0x40000000      ; GENERIC_WRITE
        push    edi
        call    [CreateFileA]
        mov     ebx, eax
        inc     eax
        jnz     @f
        push    edi
        call    [SetFileAttributesA]
        add     esp, 204h
        mov     dword [esp+20h], 10
        ret
@@:
        lodsd
        sub     esp, 8*3
        mov     edi, esp
        call    from_bdfe_time
        call    from_bdfe_time
        call    from_bdfe_time
        lea     eax, [esp+16]
        push    eax
        sub     eax, 8
        push    eax
        sub     eax, 8
        push    eax
        push    ebx
        call    [SetFileTime]
        add     esp, 8*3
        push    ebx
        call    [CloseHandle]
        push    edi
        call    [SetFileAttributesA]
        add     esp, 204h
        and     dword [esp+20h], 0
        ret

fs_execute:
        inc     eax
        jnz     @f
        add     esp, 204h
        mov     dword [esp+20h], -5     ; file not found
        or      dword [esp+14h], -1
        ret
@@:
	mov	eax, [ebx+4]
        mov     esi, [ebx+8]
        call    notify_run_prg
        add     esp, 204h
        mov     [esp+20h], eax
        mov     dword [esp+14h], 23456789h
        ret

fs_delete:
        inc     eax
        jnz     @f
        add     esp, 204h
        mov     dword [esp+20h], 5      ; file not found
.ret:
        mov     dword [esp+14h], 0x12345678
        ret
@@:
        dec     eax
        test    al, 10h
        jnz     .directory
        push    edi
        call    [DeleteFileA]
@@:
        add     esp, 204h
        and     dword [esp+20h], 0
        test    eax, eax
        jnz     .ret
        mov     byte [esp+20h], 10      ; access denied
        jmp     .ret
.directory:
        push    edi
        call    [RemoveDirectoryA]
        jmp     @b

fs_makedir:
        push    0
        push    edi
        call    [CreateDirectoryA]
        add     esp, 204h
        and     dword [esp+20h], 0
        test    eax, eax
        jnz     @f
        call	[GetLastError]
        cmp	eax, 183	; ERROR_ALREADY_EXISTS
        jz	@f
        mov     byte [esp+20h], 10
@@:
        ret

i40_window_settings:
        cmp     ebx, 1
        jnz     not_supported_i40_fn
        mov     [ebp+tls.caption], ecx
        or      byte [ebp+tls.color_main+3], 10h
        push    [ebp+tls.hWnd]
        call    [GetDC]
        push    eax
        push    [ebp+tls.hWnd]
        mov     edi, eax
; redraw caption
	mov	al, byte [ebp+tls.color_main+3]
	and	eax, 0xF
	dec	eax
	js	.type1
	jz	.nodraw
	dec	eax
	jz	.type2
	call    draw_caption_skinned
.str:
	call    draw_caption_string
.nodraw:
        call    [ReleaseDC]
        ret
.type1:
        call    draw_caption_type1
        jmp     .str
.type2:
        call    draw_caption_type2
        jmp     .str
