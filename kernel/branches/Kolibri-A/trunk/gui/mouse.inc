;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2010. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 1532$

include 'mousepointer.inc'

;==============================================================================
;///// public functions ///////////////////////////////////////////////////////
;==============================================================================

mouse.LEFT_BUTTON_FLAG	 = 0001b
mouse.RIGHT_BUTTON_FLAG  = 0010b
mouse.MIDDLE_BUTTON_FLAG = 0100b

mouse.BUTTONS_MASK = \
  mouse.LEFT_BUTTON_FLAG or \
  mouse.RIGHT_BUTTON_FLAG or \
  mouse.MIDDLE_BUTTON_FLAG

mouse.WINDOW_RESIZE_N_FLAG = 000001b
mouse.WINDOW_RESIZE_W_FLAG = 000010b
mouse.WINDOW_RESIZE_S_FLAG = 000100b
mouse.WINDOW_RESIZE_E_FLAG = 001000b
mouse.WINDOW_MOVE_FLAG	   = 010000b

mouse.WINDOW_RESIZE_SW_FLAG = \
  mouse.WINDOW_RESIZE_S_FLAG or \
  mouse.WINDOW_RESIZE_W_FLAG
mouse.WINDOW_RESIZE_SE_FLAG = \
  mouse.WINDOW_RESIZE_S_FLAG or \
  mouse.WINDOW_RESIZE_E_FLAG

align 4
;------------------------------------------------------------------------------
mouse_check_events: ;//////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Check if mouse buttons state or cursor position has changed and call
;? appropriate handlers
;------------------------------------------------------------------------------
	push	eax ebx

	mov	al, [BTN_DOWN]
	mov	bl, [mouse.state.buttons]
	and	al, mouse.BUTTONS_MASK
	mov	cl, al
	xchg	cl, [mouse.state.buttons]
	xor	bl, al
	push	eax ebx

	; did any mouse button changed its state?
	or	bl, bl
	jz	.check_position

	; yes it did, is that the first button of all pressed down?
	or	cl, cl
	jnz	.check_buttons_released

	; yes it is, activate window user is pointing at, if needed
	call	mouse._.activate_sys_window_under_cursor

	; NOTE: this code wouldn't be necessary if we knew window did
	;       already redraw itself after call above
	or	eax, eax
	jz	@f

	and	[mouse.state.buttons], 0
	jmp	.exit

	; is there any system button under cursor?
    @@: call	mouse._.find_sys_button_under_cursor
	or	eax, eax
	jz	.check_buttons_released

	; yes there is, activate it and exit
	mov	[mouse.active_sys_button.pbid], eax
	mov	[mouse.active_sys_button.coord], ebx
	mov	cl, [mouse.state.buttons]
	mov	[mouse.active_sys_button.buttons], cl
	call	sys_button_activate_handler
	jmp	.exit

  .check_buttons_released:
	cmp	[mouse.state.buttons], 0
	jnz	.buttons_changed

	; did we press some button earlier?
	cmp	[mouse.active_sys_button.pbid], 0
	je	.buttons_changed

	; yes we did, deactivate it
	xor	eax, eax
	xchg	eax, [mouse.active_sys_button.pbid]
	mov	ebx, [mouse.active_sys_button.coord]
	mov	cl, [mouse.active_sys_button.buttons]
	push	eax ebx
	call	sys_button_deactivate_handler
	pop	edx ecx

	; is the button under cursor the one we deactivated?
	call	mouse._.find_sys_button_under_cursor
	cmp	eax, ecx
	jne	.exit
	cmp	ebx, edx
	jne	.exit

	; yes it is, perform associated action
	mov	cl, [mouse.active_sys_button.buttons]
	call	sys_button_perform_handler
	jmp	.exit

  .buttons_changed:
	test	byte[esp], mouse.LEFT_BUTTON_FLAG
	jz	@f
	mov	eax, [esp + 4]
	call	.call_left_button_handler

    @@: test	byte[esp], mouse.RIGHT_BUTTON_FLAG
	jz	@f
	mov	eax, [esp + 4]
	call	.call_right_button_handler

    @@: test	byte[esp], mouse.MIDDLE_BUTTON_FLAG
	jz	.check_position
	mov	eax, [esp + 4]
	call	.call_middle_button_handler

  .check_position:
	movzx	eax, word[MOUSE_X]
	movzx	ebx, word[MOUSE_Y]
	cmp	eax, [mouse.state.pos.x]
	jne	.position_changed
	cmp	ebx, [mouse.state.pos.y]
	je	.exit

  .position_changed:
	xchg	eax, [mouse.state.pos.x]
	xchg	ebx, [mouse.state.pos.y]

	call	mouse._.move_handler

  .exit:
	add	esp, 8
	pop	ebx eax
	ret

  .call_left_button_handler:
	test	eax, mouse.LEFT_BUTTON_FLAG
	jnz	mouse._.left_button_press_handler
	jmp	mouse._.left_button_release_handler

  .call_right_button_handler:
	test	eax, mouse.RIGHT_BUTTON_FLAG
	jnz	mouse._.right_button_press_handler
	jmp	mouse._.right_button_release_handler

  .call_middle_button_handler:
	test	eax, mouse.MIDDLE_BUTTON_FLAG
	jnz	mouse._.middle_button_press_handler
	jmp	mouse._.middle_button_release_handler

;==============================================================================
;///// private functions //////////////////////////////////////////////////////
;==============================================================================

uglobal
  mouse.state:
    .pos     POINT
    .buttons db ?

  ; NOTE: since there's no unique and lifetime-constant button identifiers,
  ;       we're using two dwords to identify each of them:
  ;       * pbid - process slot (high 8 bits) and button id (low 24 bits) pack
  ;       * coord - left (high 16 bits) and top (low 16 bits) coordinates pack
  align 4
  mouse.active_sys_button:
    .pbid    dd ?
    .coord   dd ?
    .buttons db ?

  align 4
  mouse.active_sys_window:
    .pslot	dd ?
    .old_box	BOX
    .new_box	BOX
    .delta	POINT
    .last_ticks dd ?
    .action	db ?
endg

align 4
;------------------------------------------------------------------------------
mouse._.left_button_press_handler: ;///////////////////////////////////////////
;------------------------------------------------------------------------------
;? Called when left mouse button has been pressed down
;------------------------------------------------------------------------------
	test	[mouse.state.buttons], not mouse.LEFT_BUTTON_FLAG
	jnz	.exit

	call	mouse._.find_sys_window_under_cursor
	call	mouse._.check_sys_window_actions
	mov	[mouse.active_sys_window.action], al
	or	eax, eax
	jz	.exit

	xchg	eax, edx
	test	dl, mouse.WINDOW_MOVE_FLAG
	jz	@f

	mov	eax, [timer_ticks]
	mov	ebx, eax
	xchg	ebx, [mouse.active_sys_window.last_ticks]
	sub	eax, ebx
	cmp	eax, 50
	jg	@f

	mov	[mouse.active_sys_window.last_ticks], 0
	call	sys_window_maximize_handler
	jmp	.exit

    @@: test	[edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
	jnz	.exit
	mov	[mouse.active_sys_window.pslot], esi
	lea	eax, [edi + WDATA.box]
	mov	ebx, mouse.active_sys_window.old_box
	mov	ecx, BOX.sizeof
	call	memmove
	mov	ebx, mouse.active_sys_window.new_box
	call	memmove
	test	edx, mouse.WINDOW_MOVE_FLAG
	jz	@f

	call	.calculate_n_delta
	call	.calculate_w_delta
	jmp	.call_window_handler

    @@: test	dl, mouse.WINDOW_RESIZE_W_FLAG
	jz	@f
	call	.calculate_w_delta

    @@: test	dl, mouse.WINDOW_RESIZE_S_FLAG
	jz	@f
	call	.calculate_s_delta

    @@: test	dl, mouse.WINDOW_RESIZE_E_FLAG
	jz	.call_window_handler
	call	.calculate_e_delta

  .call_window_handler:
	mov	eax, mouse.active_sys_window.old_box
	call	sys_window_start_moving_handler

  .exit:
	ret

  .calculate_n_delta:
	mov	eax, [mouse.state.pos.y]
	sub	eax, [mouse.active_sys_window.old_box.top]
	mov	[mouse.active_sys_window.delta.y], eax
	ret

  .calculate_w_delta:
	mov	eax, [mouse.state.pos.x]
	sub	eax, [mouse.active_sys_window.old_box.left]
	mov	[mouse.active_sys_window.delta.x], eax
	ret

  .calculate_s_delta:
	mov	eax, [mouse.active_sys_window.old_box.top]
	add	eax, [mouse.active_sys_window.old_box.height]
	sub	eax, [mouse.state.pos.y]
	mov	[mouse.active_sys_window.delta.y], eax
	ret

  .calculate_e_delta:
	mov	eax, [mouse.active_sys_window.old_box.left]
	add	eax, [mouse.active_sys_window.old_box.width]
	sub	eax, [mouse.state.pos.x]
	mov	[mouse.active_sys_window.delta.x], eax
	ret

align 4
;------------------------------------------------------------------------------
mouse._.left_button_release_handler: ;/////////////////////////////////////////
;------------------------------------------------------------------------------
;? Called when left mouse button has been released
;------------------------------------------------------------------------------
	xor	esi, esi
	xchg	esi, [mouse.active_sys_window.pslot]
	or	esi, esi
	jz	.exit

	mov	eax, esi
	shl	eax, 5
	add	eax, window_data + WDATA.box
	mov	ebx, mouse.active_sys_window.old_box
	mov	ecx, BOX.sizeof
	call	memmove

	mov	eax, mouse.active_sys_window.old_box
	mov	ebx, mouse.active_sys_window.new_box
	call	sys_window_end_moving_handler

  .exit:
	ret

align 4
;------------------------------------------------------------------------------
mouse._.right_button_press_handler: ;//////////////////////////////////////////
;------------------------------------------------------------------------------
;? Called when right mouse button has been pressed down
;------------------------------------------------------------------------------
	test	[mouse.state.buttons], not mouse.RIGHT_BUTTON_FLAG
	jnz	.exit

	call	mouse._.find_sys_window_under_cursor
	call	mouse._.check_sys_window_actions
	test	al, mouse.WINDOW_MOVE_FLAG
	jz	.exit

	call	sys_window_rollup_handler

  .exit:
	ret

align 4
;------------------------------------------------------------------------------
mouse._.right_button_release_handler: ;////////////////////////////////////////
;------------------------------------------------------------------------------
;? Called when right mouse button has been released
;------------------------------------------------------------------------------
	ret

align 4
;------------------------------------------------------------------------------
mouse._.middle_button_press_handler: ;/////////////////////////////////////////
;------------------------------------------------------------------------------
;? Called when middle mouse button has been pressed down
;------------------------------------------------------------------------------
	ret

align 4
;------------------------------------------------------------------------------
mouse._.middle_button_release_handler: ;///////////////////////////////////////
;------------------------------------------------------------------------------
;? Called when middle mouse button has been released
;------------------------------------------------------------------------------
	ret

align 4
;------------------------------------------------------------------------------
mouse._.move_handler: ;////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Called when cursor has been moved
;------------------------------------------------------------------------------
;> eax = old x coord
;> ebx = old y coord
;------------------------------------------------------------------------------
	cmp	[mouse.active_sys_button.pbid], 0
	jnz	.exit

	mov	esi, [mouse.active_sys_window.pslot]
	or	esi, esi
	jz	.exit

	mov	eax, mouse.active_sys_window.new_box
	mov	ebx, mouse.active_sys_window.old_box
	mov	ecx, BOX.sizeof
	call	memmove

	mov	dl, [mouse.active_sys_window.action]
	test	dl, mouse.WINDOW_MOVE_FLAG
	jz	.check_resize_w

	mov	eax, [mouse.state.pos.x]
	sub	eax, [mouse.active_sys_window.delta.x]
	mov	[mouse.active_sys_window.new_box.left], eax
	mov	eax, [mouse.state.pos.y]
	sub	eax, [mouse.active_sys_window.delta.y]
	mov	[mouse.active_sys_window.new_box.top],  eax

	mov	eax, [mouse.active_sys_window.new_box.left]
	or	eax, eax
	jge	@f
	xor	eax, eax
	mov	[mouse.active_sys_window.new_box.left], eax
    @@: add	eax, [mouse.active_sys_window.new_box.width]
	cmp	eax, [Screen_Max_X]
	jl	@f
	sub	eax, [Screen_Max_X]
	sub	[mouse.active_sys_window.new_box.left], eax
    @@: mov	eax, [mouse.active_sys_window.new_box.top]
	or	eax, eax
	jge	@f
	xor	eax, eax
	mov	[mouse.active_sys_window.new_box.top], eax
    @@: add	eax, [mouse.active_sys_window.new_box.height]
	cmp	eax, [Screen_Max_Y]
	jle	.call_window_handler
	sub	eax, [Screen_Max_Y]
	sub	[mouse.active_sys_window.new_box.top], eax
	jmp	.call_window_handler

  .check_resize_w:
	test	dl, mouse.WINDOW_RESIZE_W_FLAG
	jz	.check_resize_s

	mov	eax, [mouse.state.pos.x]
	sub	eax, [mouse.active_sys_window.delta.x]
	mov	[mouse.active_sys_window.new_box.left], eax
	sub	eax, [mouse.active_sys_window.old_box.left]
	sub	[mouse.active_sys_window.new_box.width], eax

	mov	eax, [mouse.active_sys_window.new_box.width]
	sub	eax, 128
	jge	@f
	add	[mouse.active_sys_window.new_box.left], eax
	mov	[mouse.active_sys_window.new_box.width], 128
    @@: mov	eax, [mouse.active_sys_window.new_box.left]
	or	eax, eax
	jge	.check_resize_s
	add	[mouse.active_sys_window.new_box.width], eax
	xor	eax, eax
	mov	[mouse.active_sys_window.new_box.left], eax

  .check_resize_s:
	test	dl, mouse.WINDOW_RESIZE_S_FLAG
	jz	.check_resize_e

	mov	eax, [mouse.state.pos.y]
	add	eax, [mouse.active_sys_window.delta.y]
	sub	eax, [mouse.active_sys_window.old_box.top]
	mov	[mouse.active_sys_window.new_box.height], eax

	push	eax
	mov	edi, esi
	shl	edi, 5
	add	edi, window_data
	call	window._.get_rolledup_height
	mov	ecx, eax
	pop	eax
	mov	eax, [mouse.active_sys_window.new_box.height]
	cmp	eax, ecx
	jge	@f
	mov	eax, ecx
	mov	[mouse.active_sys_window.new_box.height], eax
    @@: add	eax, [mouse.active_sys_window.new_box.top]
	cmp	eax, [Screen_Max_Y]
	jle	.check_resize_e
	sub	eax, [Screen_Max_Y]
	neg	eax
	add	[mouse.active_sys_window.new_box.height], eax
	mov	ecx, [Screen_Max_Y]
	cmp	ecx, eax
	jge	.check_resize_e
	mov	[mouse.active_sys_window.new_box.height], ecx

  .check_resize_e:
	test	dl, mouse.WINDOW_RESIZE_E_FLAG
	jz	.call_window_handler

	mov	eax, [mouse.state.pos.x]
	add	eax, [mouse.active_sys_window.delta.x]
	sub	eax, [mouse.active_sys_window.old_box.left]
	mov	[mouse.active_sys_window.new_box.width], eax

	mov	eax, [mouse.active_sys_window.new_box.width]
	cmp	eax, 128
	jge	@f
	mov	eax, 128
	mov	[mouse.active_sys_window.new_box.width], eax
    @@: add	eax, [mouse.active_sys_window.new_box.left]
	cmp	eax, [Screen_Max_X]
	jle	.call_window_handler
	sub	eax, [Screen_Max_X]
	neg	eax
	add	[mouse.active_sys_window.new_box.width], eax
	mov	ecx, [Screen_Max_X]
	cmp	ecx, eax
	jge	.call_window_handler
	mov	[mouse.active_sys_window.new_box.width], ecx

  .call_window_handler:
	mov	eax, mouse.active_sys_window.old_box
	mov	ebx, mouse.active_sys_window.new_box

	push	esi
	mov	esi, mouse.active_sys_window.old_box
	mov	edi, mouse.active_sys_window.new_box
	mov	ecx, BOX.sizeof / 4
	repe
	cmpsd
	pop	esi
	je	.exit

	mov	[mouse.active_sys_window.last_ticks], 0
	call	sys_window_moving_handler

  .exit:
	ret

align 4
;------------------------------------------------------------------------------
mouse._.find_sys_window_under_cursor: ;////////////////////////////////////////
;------------------------------------------------------------------------------
;? Find system window object which is currently visible on screen and has
;? mouse cursor within its bounds
;------------------------------------------------------------------------------
;< esi = process slot
;< edi = pointer to WDATA struct
;------------------------------------------------------------------------------
        mov     esi, [Screen_Max_X]
        inc     esi
        imul    esi, [mouse.state.pos.y]
        add     esi, [_WinMapAddress]
        add     esi, [mouse.state.pos.x]
	movzx	esi, byte[esi]
	mov	edi, esi
	shl	edi, 5
	add	edi, window_data
	ret

align 4
;------------------------------------------------------------------------------
mouse._.activate_sys_window_under_cursor: ;////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
	; activate and redraw window under cursor (if necessary)
	call	mouse._.find_sys_window_under_cursor
	movzx	esi, word[WIN_STACK + esi * 2]
	lea	esi, [WIN_POS + esi * 2]
	jmp	waredraw

align 4
;------------------------------------------------------------------------------
mouse._.find_sys_button_under_cursor: ;////////////////////////////////////////
;------------------------------------------------------------------------------
;? Find system button object which is currently visible on screen and has
;? mouse cursor within its bounds
;------------------------------------------------------------------------------
;< eax = pack[8(process slot), 24(button id)] or 0
;< ebx = pack[16(button x coord), 16(button y coord)]
;------------------------------------------------------------------------------
	push	ecx edx esi edi

	call	mouse._.find_sys_window_under_cursor
	mov	edx, esi

	; check if any process button contains cursor
	mov	eax, [BTN_ADDR]
	mov	ecx, [eax]
	imul	esi, ecx, SYS_BUTTON.sizeof
	add	esi, eax
	inc	ecx
	add	esi, SYS_BUTTON.sizeof

  .next_button:
	dec	ecx
	jz	.not_found

	add	esi, -SYS_BUTTON.sizeof

	; does it belong to our process?
	cmp	dx, [esi + SYS_BUTTON.pslot]
	jne	.next_button

	; does it contain cursor coordinates?
	mov	eax, [mouse.state.pos.x]
	sub	eax, [edi + WDATA.box.left]
	sub	ax, [esi + SYS_BUTTON.left]
	jl	.next_button
	sub	ax, [esi + SYS_BUTTON.width]
	jge	.next_button
	mov	eax, [mouse.state.pos.y]
	sub	eax, [edi + WDATA.box.top]
	sub	ax, [esi + SYS_BUTTON.top]
	jl	.next_button
	sub	ax, [esi + SYS_BUTTON.height]
	jge	.next_button

	; okay, return it
	shl	edx, 24
	mov	eax, dword[esi + SYS_BUTTON.id_hi - 2]
	mov	ax, [esi + SYS_BUTTON.id_lo]
	and	eax, 0x0ffffff
	or	eax, edx
	mov	ebx, dword[esi + SYS_BUTTON.left - 2]
	mov	bx, [esi + SYS_BUTTON.top]
	jmp	.exit

  .not_found:
	xor	eax, eax
	xor	ebx, ebx

  .exit:
	pop	edi esi edx ecx
	ret

align 4
;------------------------------------------------------------------------------
mouse._.check_sys_window_actions: ;////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;< eax = action flags or 0
;------------------------------------------------------------------------------
	; is window movable?
	test	byte[edi + WDATA.cl_titlebar + 3], 0x01
	jnz	.no_action

	mov	eax, [mouse.state.pos.x]
	mov	ebx, [mouse.state.pos.y]
	sub	eax, [edi + WDATA.box.left]
	sub	ebx, [edi + WDATA.box.top]

	; is there a window titlebar under cursor?
	push	eax
	call	window._.get_titlebar_height
	cmp	ebx, eax
	pop	eax
	jl	.move_action

	; no there isn't, can it be resized then?
	mov	dl, [edi + WDATA.fl_wstyle]
	and	dl, 0x0f
	; NOTE: dangerous optimization, revise if window types changed;
	;       this currently implies only types 2 and 3 could be resized
	test	dl, 2
	jz	.no_action

	mov	ecx, [edi + WDATA.box.width]
	add	ecx, -window.BORDER_SIZE
	mov	edx, [edi + WDATA.box.height]
	add	edx, -window.BORDER_SIZE

	; is it rolled up?
	test	[edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
	jnz	.resize_w_or_e_action

	cmp	eax, window.BORDER_SIZE
	jl	.resize_w_action
	cmp	eax, ecx
	jg	.resize_e_action
	cmp	ebx, edx
	jle	.no_action

  .resize_s_action:
	cmp	eax, window.BORDER_SIZE + 10
	jl	.resize_sw_action
	add	ecx, -10
	cmp	eax, ecx
	jge	.resize_se_action
	mov	eax, mouse.WINDOW_RESIZE_S_FLAG
	jmp	.exit

  .resize_w_or_e_action:
	cmp	eax, window.BORDER_SIZE + 10
	jl	.resize_w_action.direct
	add	ecx, -10
	cmp	eax, ecx
	jg	.resize_e_action.direct
	jmp	.no_action

  .resize_w_action:
	add	edx, -10
	cmp	ebx, edx
	jge	.resize_sw_action
  .resize_w_action.direct:
	mov	eax, mouse.WINDOW_RESIZE_W_FLAG
	jmp	.exit

  .resize_e_action:
	add	edx, -10
	cmp	ebx, edx
	jge	.resize_se_action
  .resize_e_action.direct:
	mov	eax, mouse.WINDOW_RESIZE_E_FLAG
	jmp	.exit

  .resize_sw_action:
	mov	eax, mouse.WINDOW_RESIZE_SW_FLAG
	jmp	.exit

  .resize_se_action:
	mov	eax, mouse.WINDOW_RESIZE_SE_FLAG
	jmp	.exit

  .move_action:
	mov	eax, mouse.WINDOW_MOVE_FLAG
	jmp	.exit

  .no_action:
	xor	eax, eax

  .exit:
	ret

diff16 "mouse code end ",0,$
diff10 "mouse code size",mouse_check_events,$
