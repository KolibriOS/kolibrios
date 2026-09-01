SCAN_LWIN_RELEASE = 0xDB
SCAN_RWIN_RELEASE = 0xDC

struct EditBox.InplaceStr
	len 	rd 1
	p_str 	rd 1
ends

struct BClipBuf
	cbSize 		rd 1
	contentType rd 1
	encoding	rd 1
	data_ 		rb 0
ends

struct BColor
	union
		bgrf rd 1
		struct
			b 		rb 1
			g 		rb 1
			r 		rb 1
			format 	rb 1
		ends
	ends
ends

struct BPoint
	x rw 1
	y rw 1
ends

struct EditBox
	width 	dd ?
	left 	dd ?
	top 	dd ?
	union
		color 	dd ?
		bgColor dd ?
	ends
	union
		shift_color dd ?
		selectColor dd ?
	ends
	union
		focus_border_color 	dd ?
		focusBorderColor 	dd ?
	ends
	union
		blur_border_color 	dd ?
		blurBorderColor 	dd ?
	ends
	union
		text_color 	dd ?
		txColor 	BColor
	ends
	union
		max 		dd ?
		textMaxLen 	dd ?
	ends
	text 	dd ?
	union
		mouse_variable 	dd ?
		pp_mouseOwner 	dd ?
	ends
	flags 	dd ?
	union
		size 	dd ?
		textLen dd ?
	ends
	union
		pos 		dd ?
		caretPos	dd ?
	ends
	offset 	dd 0
	union
		struct
  			cl_curs_x	dw 0
  			cl_curs_y	dw 0
		ends
		cursorPos BPoint
	ends
	union
		shift 			dw 0
		selectionPos	dw ?
	ends
	union
		shift_old 		dw 0
		visibleCount	dw ?
	ends
	height 	dd 0
	union
		char_width 		dd 0
		charWidth		dd ?
	ends
ends

ES.PASSWORD 	= 1 	; режим для ввода пароля
ES.FOCUS 		= 2		; в фокусе
ES.SELECT 		= 4		; есть выбранный текст
ES.SHIFT_ON 	= 8		; при вводе нажат шифт
ES.DRAWN_ONCE 	= 16	; отрисован хоть раз (бывший ES.SELECT_BAC)
ES.LEFT_FL 		= 32	; не используется
ES.OFFSET_FL 	= 64 	; не используется
ES.INSERT_MODE 	= 128	; режим вода с перезаписью
ES.MOUSE_ON 	= 256	; удерживает мышь
ES.CTRL_ON 		= 512	; при вводе нажат ctrl
ES.ALT_ON 		= 1024	; при вводе нажат alt
ES.DISABLED 	= 2048	; отключен
ES.ALWAYS_FOCUS = 16384	; всегда в фокусе, после получения не теряет
ES.NUMERIC 		= 32768	; режим ввода только цифры

; in private methods edi always contains 'this' pointer

align 4
proc EditBox.__drawBorder
	virtual at edi
		.this EditBox
	end virtual
	mov	edx, [.this.focusBorderColor]
	test [.this.flags], ES.FOCUS
	jne	@f
		mov	edx, [.this.blurBorderColor]
	@@:

	; left:left
	mov	ebx, [.this.left]
	mov	eax, ebx
	shl	ebx, 16
	or	ebx, eax
	; top:top
	mov	ecx, [.this.top]
	mov	eax, ecx
	shl	ecx, 16
	or ecx, eax

	mov esi, [.this.height]
	inc esi
	mov ebp, [.this.width]

	add	cx, si 	; top:bottom
	mov eax, SF_DRAW_LINE
	mcall 		; left top -> left bottom

	rol ecx, 16
	add cx, si 	; bottom:bottom
	add bx, bp 	; left:right
	mcall		; left bottom -> right bottom

	rol ebx, 16
	add bx, bp 	; right:right
	sub cx, si 	; bottom:top
	mcall 		; right bottom -> right top

	rol ecx, 16
	sub cx, si 	; top:top
	sub bx, bp 	; right:left
	mcall 		; right top -> left top
	ret
endp

macro EditBox.__visibleSmbCount{
	local ..zero_width
	mov edx, [edi + EditBox.width]
	xor eax, eax
	cmp [edi + EditBox.charWidth], 0
	je ..zero_width
		lea eax, [edx - 4]
		xor edx, edx
		div [edi + EditBox.charWidth]
	..zero_width:
}

align 4
proc EditBox.__drawBg
	virtual at edi
		.this EditBox
	end virtual
	mov ebx, [.this.left]
	inc ebx
	shl ebx, 16
	mov bx, word[.this.width]
	dec bx

	mov edx, [.this.bgColor]
	mov eax, 0xCACACA
	test [.this.flags], ES.DISABLED
	cmovnz edx, eax
	
	mov ecx, [.this.top]
	inc ecx
	shl ecx, 16
	mov cx, word[.this.height]

	mcall SF_DRAW_RECT
	ret
endp

;----------------------------------------------------------
; Calculate offset to visible part of the text based on
; current caret position
;----------------------------------------------------------
align 4
proc EditBox.__calcOffset
	virtual at edi
		.this EditBox
	end virtual
	movzx eax, [.this.visibleCount]
	mov ecx, [.this.caretPos]
	mov edx, [.this.offset]
	cmp [.this.offset], ecx
	ja .offset_above
		lea edx, [edx + eax + 1] ; + 1 for normal cursor position on the edge of the left
		cmp edx, [.this.caretPos]
			ja .end_

		; check if text len is equal to caret position
		mov edx, [.this.textLen]
		cmp edx, [.this.caretPos]
		jne @f
			xor ebx, ebx
			sub edx, eax
			cmovs edx, ebx
			mov [.this.offset], edx

			jmp .end_
		@@:

		; offset increment correction
		; by default visible contents shifts by 8
		; but if visible content is lower than that,
		; we shift by the size of visible content size
		mov ebx, 8
		cmp eax, ebx
		cmova eax, ebx
		add [.this.offset], eax

		; check if we are still too far from caret
		; in that case, set offset to caretPos - visibleCount
		mov ebx, [.this.offset]
		add bx, [.this.visibleCount]
		cmp ebx, [.this.caretPos]
		ja @f
			mov ebx, ecx
			sub bx, [.this.visibleCount]
			mov [.this.offset], ebx
		@@:
		jmp .end_
	.offset_above:
	; offset decrement correction
	; by default visible contents shifts by 8
	; but if visible content is lower than that,
	; it shifts by the size of visible content size
	mov ebx, 8
	cmp eax, ebx
	cmova eax, ebx

	; check if decreased value lower than 0
	xor ebx, ebx
	sub edx, eax
	cmovb edx, ebx

	; check if offset still above caret
	mov ebx, ecx
	cmp edx, ecx
	jbe .offset_not_above_caret
		; in case it is, set offset to caretPos - 1, if caretPos is higher than 0
		test ebx, ebx
		jz @f
			dec ebx
		@@:
		mov edx, ebx
	.offset_not_above_caret:
	mov [.this.offset], edx


	.end_:
		; fix if offset shifts us from the end of visible field
		; while we near to the text end
		mov eax, [.this.offset]
		mov edx, [.this.textLen]
		movzx ebx, [.this.visibleCount]
		sub edx, ebx
		cmp ecx, edx
		jb @f
			; check if we are near the end, but end of text is above visible range
			mov ecx, eax
			add cx, [.this.visibleCount]
			cmp ecx, [.this.textLen]
			cmova eax, edx
		@@:
		mov [.this.offset], eax
		ret
endp

align 4
proc EditBox.__drawText
	virtual at edi
		.this EditBox
	end virtual

	movzx eax, [.this.visibleCount]
	mov esi, [.this.textLen]
	sub esi, [.this.offset]
	cmp eax, esi
	cmovb esi, eax

	test esi, esi
	jz .no_text
		mov eax, SF_DRAW_TEXT
		mov ebx, [.this.left]
		add ebx, 2
		shl ebx, 16
		add bx, word[.this.top]
		add bx, 3
		mov ecx, [.this.txColor.bgrf]
		test [.this.flags], ES.PASSWORD
		jnz .password_mode
			mov edx, [.this.text]
			add edx, [.this.offset]
			mcall
			ret

		.password_mode:
		local .charWidth:DWORD
		mov edx, [.this.charWidth]
		push edi
		mov	edi, esi
		mov	esi,1
		shl edx, 16
		mov [.charWidth], edx
		mov	edx, .txtPass
		@@:
			mcall
			add	ebx, [.charWidth]
		dec	edi
		jnz @b
	.no_text:
	ret

	.txtPass db "*"
endp

align 4
proc EditBox.__drawSelection
	virtual at edi
		.this EditBox
	end virtual
	test [.this.flags], ES.SELECT
	jz .no_selection
		movzx eax, [.this.visibleCount]
		mov ecx, [.this.offset]
		add eax, ecx
		; ebx - min, esi- max
		mov edx, [.this.caretPos]
		mov ebx, edx
		movzx esi, [.this.selectionPos]
		cmp esi, ebx
		jae @f
			xchg esi, ebx
		@@:
		cmp eax, esi
		cmovb esi, eax
		cmp ebx, ecx
		cmovb ebx, ecx
		sub ebx, ecx
		sub esi, ecx
		sub esi, ebx

		mov ecx, [.this.charWidth]
		imul ebx, ecx
		mov eax, esi
		mul ecx

		add ebx, [.this.left]
		shl ebx, 16
		lea ebx, [ebx + 0x10000 + eax + 1]

		mov edx, [.this.selectColor]

		mov ecx, [.this.top]
		inc ecx
		shl ecx, 16
		mov cx, word[.this.height]
		
		mcall SF_DRAW_RECT
	.no_selection:
	ret
endp

macro EditBox.__calcFontParameters{
	local ..font6x9
	mov ecx, [edi + EditBox.txColor.bgrf]
	and ecx, 17FFFFFFh
	shld edx, ecx, 8
	and edx, 0xF
	inc edx
	shr ecx, 28

	lea eax, [edx * 3]
	lea ebx, [edx * 9 + 4]
	jecxz ..font6x9
		lea eax, [edx * 4]
		lea ebx, [edx * 8 + 2]
		shl ebx, 1
	..font6x9:
	shl eax, 1
	mov [edi + EditBox.charWidth], eax
	mov [edi + EditBox.height], ebx
}

macro EditBox.__terminateText{
	local ..void_text
	mov ecx, [edi + EditBox.text]
	jecxz ..void_text
		mov eax, [edi + EditBox.textLen]
		mov word[ecx + eax], 0
	..void_text:
}

macro EditBox.__calcCursorPos{
	mov eax, [edi + EditBox.caretPos]
	sub eax, [edi + EditBox.offset]
	mov ecx, [edi + EditBox.charWidth]
	mul ecx
	add eax, [edi + EditBox.left]
	inc eax
	mov ecx, [edi + EditBox.top]
	add ecx, 2

	mov [edi + EditBox.cursorPos.x], ax
	mov [edi + EditBox.cursorPos.y], cx
}

align 4
proc EditBox.__drawCursor
	virtual at edi
		.this EditBox
	end virtual

	mov edx, [.this.txColor.bgrf]

	movzx ebx, [.this.cursorPos.x]
	movzx ecx, [.this.cursorPos.y]

	mov eax, ebx
	shl ebx, 16
	or ebx, eax

	mov eax, ecx
	shl ecx, 16
	or ecx, eax

	add cx, word[.this.height]
	sub cx, 3

	mcall SF_DRAW_LINE
	ret
endp

align 4
proc EditBox.__draw
	virtual at edi
		.this EditBox
	end virtual
	
	EditBox.__terminateText
	
	EditBox.__calcFontParameters
	EditBox.__visibleSmbCount
	mov [.this.visibleCount], ax

	call EditBox.__calcOffset

	or [.this.flags], ES.DRAWN_ONCE
	
	call EditBox.__drawBorder
	call EditBox.__drawBg


	test [.this.flags], ES.FOCUS
	jz .no_focus
		EditBox.__calcCursorPos
		call EditBox.__drawSelection
		call EditBox.__drawText
		jmp EditBox.__drawCursor

	.no_focus:
	jmp EditBox.__drawText
endp

align 4
proc edit_box_draw stdcall, .pthis
	pusha
	mov edi, [.pthis]
	
	call EditBox.__draw
	popa
	ret
endp

align 4
proc EditBox.__processMouse
	virtual at edi
		.this EditBox
	end virtual

	test [.this.flags], ES.DRAWN_ONCE
		jz .return
		
	test [.this.flags], ES.DISABLED
		jnz .return

	mcall SF_MOUSE_GET, SSF_BUTTON
	mov ecx, [.this.pp_mouseOwner]
	test eax, 1  ; left button clicked
	jnz .left_button_clicked
		and [.this.flags], not ES.MOUSE_ON
		jecxz @f
			mov dword[ecx], 0
		@@:
		.return: ret
	.left_button_clicked:

	jecxz .mouse_is_free
		cmp dword[ecx], 0
			jz .mouse_is_free

		cmp [ecx], edi
		jne .mouse_blur

	.mouse_is_free:
		;получаем координаты мыши относительно 0 т.е всей области экрана
		mcall SF_MOUSE_GET, SSF_WINDOW_POSITION
		test [.this.flags], ES.MOUSE_ON
		jz .mouse_not_captured
			; mouse wig-wag
			or [.this.flags], ES.SELECT
			test eax, eax
				js .mouse_left
			
			shr eax, 16
			sub eax, [.this.left]
				jc .mouse_left
			
			cmp [.this.width], eax
				jc .mouse_right

			xor edx, edx
			div [.this.charWidth]
			add eax, [.this.offset]
			cmp eax, [.this.textLen]
			jae @f
				mov [.this.caretPos], eax
			@@:
			jmp .after_move

			.mouse_right:
				mov eax, [.this.caretPos]
				cmp eax, [.this.textLen]
				jae @f
					inc [.this.caretPos]
				@@:
				jmp .after_move

			.mouse_left:
				cmp [.this.caretPos], 0
				je @f
					dec [.this.caretPos]
				@@:

			.after_move:
				mov eax, [.this.caretPos]
				cmp ax, [.this.selectionPos]
				jne @f
					and [.this.flags], not ES.SELECT
				@@:
				jmp EditBox.__draw
		.mouse_not_captured:
		mov ebx, [.this.top]
		cmp ax, bx
			jl .mouse_blur

		add ebx, [.this.height]
		cmp ax, bx
			jg .mouse_blur

		shr eax, 16
		movzx ebx, word[.this.left]
		cmp ax, bx
			jl .mouse_blur

		add bx, word[.this.width]
		cmp ax, bx
			jg .mouse_blur

		; перемещение позиции курсора
		xor edx, edx
		sub	eax, [.this.left]
		div [.this.charWidth]
		add eax, [.this.offset]
		cmp	eax, [.this.textLen]
		cmova eax, [.this.textLen]

		; обработка и перемещение выделения
		test [.this.flags], ES.MOUSE_ON
		jnz @f
			mov [.this.caretPos], eax
			mov [.this.selectionPos], ax
			or [.this.flags], ES.MOUSE_ON
			cmp [.this.pp_mouseOwner], 0
				je @f

			mov [ecx], edi
		@@:
		or [.this.flags], ES.FOCUS
		and [.this.flags], not ES.SELECT
		jmp EditBox.__draw

	.mouse_blur:
	test [.this.flags], ES.ALWAYS_FOCUS
	jnz @f
		and [.this.flags], not ES.FOCUS
	@@:
	and [.this.flags], not ES.SELECT
	jmp EditBox.__draw
endp

align 4
proc edit_box_mouse stdcall, .pthis
	pusha
	mov edi, [.pthis]
	call EditBox.__processMouse
	popa
	ret
endp

align 4
proc EditBox.__delSelection
	virtual at edi
		.this EditBox
	end virtual

	xor ecx, ecx
	movzx edx, [.this.selectionPos]
	mov eax, [.this.caretPos]
	cmp edx, eax
		je .return
	jb @f
		xchg edx, eax
	@@:
	mov ecx, [.this.textLen]
	mov esi, [.this.text]
	
	sub ecx, eax
	lea ebp, [edx + ecx]
	jecxz .end_selection
		mov ebx, edi
		mov edi, esi

		add esi, eax
		add edi, edx
		rep movsb
		mov edi, ebx
	.end_selection:

	mov [.this.caretPos], edx
	mov [.this.textLen], ebp
	and [.this.flags], not ES.SELECT
	mov ecx, esp
	.return: ret
endp

align 4
proc EditBox.__replaceSelectChar
	virtual at edi
		.this EditBox
	end virtual

	mov ecx, eax
	movzx edx, [.this.selectionPos]
	mov eax, [.this.caretPos]
	cmp edx, eax
		je .return
	jb @f
		xchg edx, eax
	@@:
	mov esi, [.this.text]
	mov byte[esi + edx], cl
	inc edx
	mov ecx, [.this.textLen]
	
	mov ebx, edi
	mov edi, esi

	add esi, eax
	sub ecx, eax
	lea eax, [edx + ecx]
	jecxz .selected_at_end
		add edi, edx
		cmp esi, edi
		je .one_smb_selection
			rep movsb
		.one_smb_selection:
	.selected_at_end:
	mov edi, ebx

	mov [.this.textLen], eax
	mov [.this.caretPos], edx
	and [.this.flags], not ES.SELECT
	.return: ret
endp

; edx != 0 - inserted, edx == 0 - not inserted
align 4
proc EditBox.__insertChar
	virtual at edi
		.this EditBox
	end virtual
	xor edx, edx
	mov ecx, [.this.textLen]
	cmp ecx, [.this.textMaxLen]
		je .return

	cmp ecx, [.this.caretPos]
	jne .caret_not_at_end
		mov edx, [.this.text]
		mov [edx + ecx], al
		inc [.this.textLen]
		inc [.this.caretPos]
		.return: ret
	.caret_not_at_end:

	mov esi, [.this.text]
	test [.this.flags], ES.INSERT_MODE
	jnz .insert_mode
		sub ecx, [.this.caretPos]
		mov edx, [.this.textLen]
		lea esi, [esi + edx - 1]

		mov ebx, edi
		lea edi, [esi + 1]

		std
		rep movsb
		cld

		mov byte[edi], al

		mov edi, ebx
		inc [.this.textLen]
		inc [.this.caretPos]
		ret

	.insert_mode:
		mov edx, [.this.caretPos]
		mov byte[esi + edx], al
		inc edx
		mov [.this.caretPos], edx
		ret
endp

align 4
proc EditBox.__selectionToClipboard
	virtual at edi
		.this EditBox
	end virtual

	test [.this.flags], ES.SELECT
		jz .return

	movzx edx, [.this.selectionPos]
	mov ecx, [.this.caretPos]
	cmp edx, ecx
		je .return
	jb @f
		xchg edx, ecx
	@@:
	sub ecx, edx
	add ecx, sizeof.BClipBuf
	mcall SF_SYS_MISC, SSF_MEM_ALLOC
	cmp eax, 1
		je .return
	cmp eax, -1
		je .return

	mov [eax + BClipBuf.cbSize], ecx
	mov [eax + BClipBuf.contentType], 0
	mov [eax + BClipBuf.encoding], 1
	mov esi, [.this.text]
	add esi, edx
	mov ebx, edi
	lea edi, [eax + BClipBuf.data_]
	sub ecx, sizeof.BClipBuf
	rep movsb
	mov edi, ebx

	mov edx, eax
	mov ecx, [eax + BClipBuf.cbSize]
	mcall SF_CLIPBOARD, SSF_WRITE_CB
	mcall SF_SYS_MISC, SSF_MEM_FREE, edx
	.return: ret
endp


; .p_str - const parameter, never changes
align 4
proc EditBox.__insertStr c, .p_str
	virtual at edi
		.this EditBox
	end virtual

	mov ebx, [.p_str]
	virtual at ebx
		.str EditBox.InplaceStr
	end virtual

	locals
	 	.startSel 	rd 1
	 	.endSel 	rd 1
	 	.restLen 	rd 1
	 	.newLen 	rd 1
	endl

	mov eax, [.this.caretPos]
	movzx edx, [.this.selectionPos]
	test [.this.flags], ES.SELECT
		cmovz edx, eax

	cmp edx, eax
	jb @f
		xchg edx, eax
	@@:
	mov [.startSel], edx
	mov [.endSel], eax

	mov eax, [.str.len]
	add eax, edx

	mov ecx, [.this.textLen]
	sub ecx, [.endSel]
	mov [.restLen], ecx
	add eax, ecx

	xor edx, edx
	mov ecx, [.this.textMaxLen]
	inc ecx
	div ecx

	mov [.newLen], edx
	mov ecx, edx
	sub ecx, [.startSel]
		jz .return

	; check if we have enough place for remaining part of text after selection 
	cmp ecx, [.str.len]
	jle .no_place_for_rest
		; 
		local .newCaret:DWORD
		mov esi, [.this.text]
		sub ecx, [.str.len]
		mov [.restLen], ecx

		mov eax, [.startSel]
		add eax, [.str.len]
		mov [.newCaret], eax

		push edi
		mov edi, esi
		add edi, eax
		add esi, [.endSel]
		
		cmp esi, edi
		je @f
			jb .std
				;cld
				rep movsb
				sub edi, [.restLen]
				jmp .rest_move_end
			.std:
				lea edi, [edi + ecx - 1]
				lea esi, [esi + ecx - 1]
				std
				rep movsb
				cld
				inc edi
			.rest_move_end:
		@@:
		mov ecx, [.str.len]
		sub edi, ecx
		mov esi, [.str.p_str]
		rep movsb

		pop edi
		mov eax, [.newCaret]
		mov [.this.caretPos], eax
		mov eax, [.newLen]
		mov [.this.textLen], eax
		and [.this.flags], not ES.SELECT
		.return: ret

	.no_place_for_rest:
		push edi
		mov edi, [.this.text]
		add edi, [.startSel]
		mov esi, [.str.p_str]
		rep movsb
		pop edi
		mov eax, [.newLen]
		mov [.this.caretPos], eax
		mov [.this.textLen], eax
		and [.this.flags], not ES.SELECT
		ret
endp

; const parameter, didn't change
align 4
proc BEdit.__validateStrNumeric c, .p_str
	virtual at edi
		.this EditBox
	end virtual

	mov ebx, [.p_str]
	virtual at ebx
		.str EditBox.InplaceStr
	end virtual

	xor eax, eax
	mov ecx, [.str.len]
		jecxz .return

	mov esi, [.str.p_str]
	@@:
		lodsb
		sub al, 0x30
		cmp al, 9
			ja .return
	loop @b
	xor eax, eax
	.return:
		ret
endp

align 4
proc EditBox.__processKey
	.L_WIN 	= 0x02
	.R_WIN 	= 0x04
	.SHIFT 	= 0x03
	.CTRL 	= 0x0C
	.ALT 	= 0x30

	virtual at edi
		.this EditBox
	end virtual

	test [.this.flags], ES.DRAWN_ONCE
		jz .return

	test [.this.flags], ES.FOCUS
		jz .return

	test [.this.flags], ES.MOUSE_ON or ES.DISABLED
		jnz .return

	mov esi, eax	; save key state
	;--------------------------------------
	; this code for Win-keys, works with
	; kernel SVN r.3356 or later
	mcall SF_KEYBOARD, SSF_GET_CONTROL_KEYS
	test ah, .L_WIN or .R_WIN
		jnz .return

	and [.this.flags], not (ES.ALT_ON or ES.CTRL_ON or ES.SHIFT_ON)
	test eax, .SHIFT
	jz @f
		or [.this.flags], ES.SHIFT_ON
	@@:
	test eax, .CTRL
	jz @f
		or [.this.flags], ES.CTRL_ON
	@@:
	test eax, .ALT
	jz @f
		or [.this.flags], ES.ALT_ON
	@@:

	mov eax, esi; restore key state
	ror eax, 8
	test [.this.flags], ES.CTRL_ON
	jz .skip_ctrl
		cmp ah, SCAN_CODE_V
			je .ctrl_plus_v
		cmp ah, SCAN_CODE_A
			je .ctrl_plus_a
		cmp ah, SCAN_CODE_X
			je .ctrl_plus_x
		cmp ah, SCAN_CODE_C
			je .ctrl_plus_c
		jmp .ctrl_end_case
		.ctrl_plus_v:
			mcall SF_CLIPBOARD, SSF_GET_SLOT_COUNT
			test eax, eax
				js .ctrl_end_case
				jz .ctrl_end_case
			lea ecx, [eax - 1]
			mcall SF_CLIPBOARD, SSF_READ_CB
			cmp eax, -1
				je .ctrl_end_case
			cmp eax, 1
				je .ctrl_end_case
			mov	ecx,[eax + BClipBuf.contentType]
			; check for text
			test ecx,ecx
			jz	@f
				mov ecx, eax
				mcall SF_SYS_MISC, SSF_MEM_FREE
				ret
			@@:
			mov	ecx,[eax+8]
			; check for cp866
			cmp	cl,1
			je	@f
				mov ecx, eax
				mcall SF_SYS_MISC, SSF_MEM_FREE
				ret
			@@:
			lea ecx, [eax + BClipBuf.data_]
			mov [eax + EditBox.InplaceStr.p_str], ecx
			sub [eax + EditBox.InplaceStr.len], sizeof.BClipBuf
			push eax
			test [.this.flags], ES.NUMERIC
			jz @f
				call BEdit.__validateStrNumeric
				test eax, eax
					jz @f
					
				pop ecx
				mcall SF_SYS_MISC, SSF_MEM_FREE
				ret
			@@:
			call EditBox.__insertStr
			pop ecx
			mcall SF_SYS_MISC, SSF_MEM_FREE
			jmp EditBox.__draw

		.ctrl_plus_a:
			mov ecx, [.this.textLen]
				jecxz .ctrl_end_case

			test [.this.flags], ES.SELECT
			jz @f
				cmp [.this.selectionPos], 0
					jne @f
				cmp [.this.caretPos], ecx
					je .ctrl_end_case
			@@:
			mov [.this.selectionPos], 0
			mov [.this.caretPos], ecx
			or [.this.flags], ES.SELECT
			jmp EditBox.__draw

		.ctrl_plus_x:
			call EditBox.__selectionToClipboard
			call EditBox.__delSelection
				jecxz .ctrl_end_case
			jmp EditBox.__draw

		.ctrl_plus_c:
			call EditBox.__selectionToClipboard

		.ctrl_end_case:
		ret
	.skip_ctrl:
	cmp ah, SCAN_CODE_SPACE
 	ja .scan_after_space
		cmp al, ASCII_KEY_BACK
		jne .no_backspace
			cmp [.this.textLen], 0
				je .return

			test [.this.flags], ES.SELECT
			jnz @f
				mov ecx, [.this.caretPos]
					jecxz .return
				dec ecx
				mov [.this.selectionPos], cx
				or [.this.flags], ES.SELECT
			@@:
			call EditBox.__delSelection
			jmp EditBox.__draw
		.no_backspace:
		; skip unsupported scancodes
		irp val, SCAN_CODE_TAB, SCAN_CODE_RETURN, SCAN_CODE_ESCAPE{
			cmp ah, val
				je .return
		}
		jmp EditBox.__processPrintableCharacter
	.scan_after_space:
	cmp ah, SCAN_CODE_DELETE
		ja .return
	cmp ah, SCAN_CODE_HOME
		jb .return
	cmp ax, SCAN_CODE_CLEAR shl 8 + ASCII_KEY_CLEAR
		je .return
	cmp al, ASCII_KEY_LEFT
		jb EditBox.__processPrintableCharacter
	and eax, 0x0F
	jmp dword[.unlock_numpad_filtration + eax * 4]
	.return: ret

	.unlock_numpad_filtration: 
	dd 	.key_left,\ 	; LEFT
		.return,\ 		; DOWN
		.return,\ 		; UP
		.key_right,\	; RIGHT
		.key_home,\		; HOME
		.key_end,\		; END
		.key_delete,\	; DELETE
		.return,\		; PGDN
		.return,\		; PGUP
		.key_insert		; INSERT_MODE

	.key_left:
		mov ecx, [.this.caretPos]
		jecxz @f
			dec ecx
		@@:
		jmp .commit_caret_move

	.key_right:
		mov eax, [.this.textLen]
		mov ecx, [.this.caretPos]
		cmp ecx, eax
		je @f
			inc ecx
		@@:
		jmp .commit_caret_move

	.key_home:
		mov ecx, [.this.caretPos]
		jecxz @f
			xor ecx, ecx
		@@:
		jmp .commit_caret_move

	.key_end:
		mov eax, [.this.textLen]
		cmp [.this.caretPos], eax
		je @f
			mov ecx, eax
		@@:
		; jmp .commit_caret_move

	.commit_caret_move:
		btr [.this.flags], bsf ES.SELECT
		jnc .caret_no_select
			test [.this.flags], ES.SHIFT_ON
			jz @f
				or [.this.flags], ES.SELECT
				cmp ecx, [.this.caretPos]
					je .return

				mov [.this.caretPos], ecx
				jmp EditBox.__draw
			@@:
			mov [.this.caretPos], ecx
			mov eax, [.this.caretPos]
			mov [.this.selectionPos], ax
			jmp EditBox.__draw
		.caret_no_select:
		cmp ecx, [.this.caretPos]
			je .return

		mov eax, [.this.caretPos]
		mov [.this.selectionPos], ax
		mov [.this.caretPos], ecx
		test [.this.flags], ES.SHIFT_ON
		jz @f
			cmp cx, [.this.selectionPos]
				je @f

			or [.this.flags], ES.SELECT
		@@:
		jmp EditBox.__draw

	.key_delete:
		cmp [.this.textLen], 0
			je .return

		test [.this.flags], ES.SELECT
		jnz @f
			mov ecx, [.this.caretPos]
			cmp ecx, [.this.textLen]
				je .return

			inc ecx
			mov [.this.selectionPos], cx
			or [.this.flags], ES.SELECT
		@@:
		call EditBox.__delSelection
		jmp EditBox.__draw

	.key_insert:
		xor [.this.flags], ES.INSERT_MODE
		ret

endp

align 4
proc EditBox.__processPrintableCharacter
	virtual at edi
		.this EditBox
	end virtual
	test [.this.flags], ES.NUMERIC
	jz @f
		cmp al, '0'
			jb .return
		cmp al, '9'
			ja .return
	@@:
	rol eax, 8
	movzx eax, ah
	test [.this.flags], ES.SELECT
	jz @f
		call EditBox.__replaceSelectChar
		jmp EditBox.__draw
	@@:
	call EditBox.__insertChar
	test edx, edx
		jz .return

	jmp EditBox.__draw
	.return: ret
endp

align 4
proc edit_box_key_safe stdcall, .pthis, .key
	pusha
	mov edi, [.pthis]
	mov eax, [.key]
	call EditBox.__processKey
	popa
	ret
endp

align 4
proc edit_box_key stdcall, .pthis
	pusha
	mov edi, [.pthis]
	call EditBox.__processKey
	popa
	ret
endp

align 4
proc edit_box_set_text stdcall, .pthis, .text
	pusha
	mov edx, [.pthis]
	virtual at edx
		.this EditBox
	end virtual

	mov ecx, [.this.textMaxLen]

	mov esi, [.text]
	mov edi, [.this.text]
	@@:
		lodsb
		stosb
		test al, al
	loopnz @b
	sub ecx, [.this.textMaxLen]
	not ecx ; neg ecx; sub ecx, 1
	mov [.this.caretPos], ecx
	mov [.this.textLen], ecx
	and [.this.flags], not ES.SELECT
	mov edi, edx
	call EditBox.__draw
	popa
	ret
endp
