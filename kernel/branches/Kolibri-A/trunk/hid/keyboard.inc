;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;// mike.dld [

VKEY_LSHIFT   = 0000000000000001b
VKEY_RSHIFT   = 0000000000000010b
VKEY_LCONTROL = 0000000000000100b
VKEY_RCONTROL = 0000000000001000b
VKEY_LALT     = 0000000000010000b
VKEY_RALT     = 0000000000100000b
VKEY_CAPSLOCK = 0000000001000000b
VKEY_NUMLOCK  = 0000000010000000b
VKEY_SCRLOCK  = 0000000100000000b

VKEY_SHIFT    = 0000000000000011b
VKEY_CONTROL  = 0000000000001100b
VKEY_ALT      = 0000000000110000b

uglobal
  align 4
  kb_state	dd 0
  ext_code	db 0

  keyboard_mode db 0
  keyboard_data db 0

  altmouseb	db 0
  ctrl_alt_del	db 0

  kb_lights	db 0

align 4
	hotkey_scancodes	rd	256	; we have 256 scancodes
	hotkey_list		rd	256*4	; max 256 defined hotkeys
	hotkey_buffer		rd	120*2	; buffer for 120 hotkeys
endg

iglobal
hotkey_tests	dd	hotkey_test0
		dd	hotkey_test1
		dd	hotkey_test2
		dd	hotkey_test3
		dd	hotkey_test4
hotkey_tests_num = 5
endg

hotkey_test0:
	test	al, al
	setz	al
	ret
hotkey_test1:
	test	al, al
	setnp	al
	ret
hotkey_test2:
	cmp	al, 3
	setz	al
	ret
hotkey_test3:
	cmp	al, 1
	setz	al
	ret
hotkey_test4:
	cmp	al, 2
	setz	al
	ret

hotkey_do_test:
	push	eax
	mov	edx, [kb_state]
	shr	edx, cl
	add	cl, cl
	mov	eax, [eax+4]
	shr	eax, cl
	and	eax, 15
	cmp	al, hotkey_tests_num
	jae	.fail
	xchg	eax, edx
	and	al, 3
	call	[hotkey_tests + edx*4]
	cmp	al, 1
	pop	eax
	ret
.fail:
	stc
	pop	eax
	ret

align 4
irq1:
;       save_ring3_context
;       mov   ax, os_data
;       mov   ds, ax
;       mov   es, ax

       movzx eax,word[TASK_COUNT] ; top window process
       movzx eax,word[WIN_POS+eax*2]
       shl   eax,8
       mov   al,[SLOT_BASE+eax+APPDATA.keyboard_mode]
       mov   [keyboard_mode],al

       in    al,0x60
       mov   [keyboard_data],al

; ch = scancode
; cl = ext_code
; bh = 0 - normal key
; bh = 1 - modifier (Shift/Ctrl/Alt)
; bh = 2 - extended code

       mov   ch,al
       cmp   al,0xE0
       je    @f
       cmp   al,0xE1
       jne   .normal_code
   @@:
	mov	bh, 2
	mov	[ext_code], al
	jmp	.writekey
   .normal_code:
	mov	cl, 0
	xchg	cl, [ext_code]
       and   al,0x7F
	mov	bh, 1
   @@: cmp   al,0x2A
       jne   @f
       cmp   cl,0xE0
       je    .writekey
       mov   eax,VKEY_LSHIFT
       jmp   .modifier
   @@: cmp   al,0x36
       jne   @f
       cmp   cl,0xE0
       je    .writekey
       mov   eax,VKEY_RSHIFT
       jmp   .modifier
   @@: cmp   al,0x38
       jne   @f
	mov	eax, VKEY_LALT
	test	cl, cl
	jz	.modifier
	mov	al, VKEY_RALT
	jmp	.modifier
   @@: cmp   al,0x1D
       jne   @f
	mov	eax, VKEY_LCONTROL
	test	cl, cl
	jz	.modifier
	mov	al, VKEY_RCONTROL
	cmp	cl, 0xE0
	jz	.modifier
	mov	[ext_code], cl
	jmp	.writekey
   @@: cmp   al,0x3A
       jne   @f
       mov   bl,4
       mov   eax,VKEY_CAPSLOCK
       jmp   .no_key.xor
   @@: cmp   al,0x45
       jne   @f
	test	cl, cl
	jnz	.writekey
       mov   bl,2
       mov   eax,VKEY_NUMLOCK
       jmp   .no_key.xor
   @@: cmp   al,0x46
       jne   @f
       mov   bl,1
       mov   eax,VKEY_SCRLOCK
       jmp   .no_key.xor
   @@:
       xor   ebx, ebx
       test  ch,ch
       js    .writekey
       movzx eax,ch		 ; plain key
       mov   bl,[keymap+eax]
       mov   edx,[kb_state]
       test  dl,VKEY_CONTROL	 ; ctrl alt del
       jz    .noctrlaltdel
       test  dl,VKEY_ALT
       jz    .noctrlaltdel
       cmp   ch,53h
       jne   .noctrlaltdel
       mov   [ctrl_alt_del],1
   .noctrlaltdel:
       test  dl,VKEY_CONTROL	 ; ctrl on ?
       jz    @f
       sub   bl,0x60
   @@: test  dl,VKEY_SHIFT	 ; shift on ?
       jz    @f
       mov   bl,[keymap_shift+eax]
   @@: test  dl,VKEY_ALT	 ; alt on ?
       jz    @f
       mov   bl,[keymap_alt+eax]
   @@:
	jmp	.writekey
.modifier:
	test	ch, ch
	js	.modifier.up
	or	[kb_state], eax
	jmp	.writekey
.modifier.up:
	not	eax
	and	[kb_state], eax
	jmp	.writekey
.no_key.xor:
	mov	bh, 0
	test	ch, ch
	js	.writekey
	xor	[kb_state], eax
	xor	[kb_lights], bl
	call	set_lights

.writekey:
; test for system hotkeys
	movzx	eax, ch
	cmp	bh, 1
	ja	.nohotkey
	jb	@f
	xor	eax, eax
@@:
	mov	eax, [hotkey_scancodes + eax*4]
.hotkey_loop:
	test	eax, eax
	jz	.nohotkey
	mov	cl, 0
	call	hotkey_do_test
	jc	.hotkey_cont
	mov	cl, 2
	call	hotkey_do_test
	jc	.hotkey_cont
	mov	cl, 4
	call	hotkey_do_test
	jnc	.hotkey_found
.hotkey_cont:
	mov	eax, [eax]
	jmp	.hotkey_loop
.hotkey_found:
	mov	eax, [eax+8]
; put key in buffer for process in slot eax
	mov	edi, hotkey_buffer
@@:
	cmp	dword [edi], 0
	jz	.found_free
	add	edi, 8
	cmp	edi, hotkey_buffer+120*8
	jb	@b
; no free space - replace first entry
	mov	edi, hotkey_buffer
.found_free:
	mov	[edi], eax
	movzx	eax, ch
	cmp	bh, 1
	jnz	@f
	xor	eax, eax
@@:
	mov	[edi+4], ax
	mov	eax, [kb_state]
	mov	[edi+6], ax
	jmp	.exit.irq1
.nohotkey:
       cmp   [keyboard_mode],0 ; return from keymap
       jne   .scancode
	test	bh, bh
	jnz	.exit.irq1
	test	bl, bl
	jz	.exit.irq1

;.........................Part1 Start.......Code by Rus, optimize by Ghost...................................
	test  [kb_state], VKEY_NUMLOCK
	jz     .dowrite
        cmp     cl, 0xE0
        jz      .dowrite

	cmp	ch, 55
	jnz	@f
	mov	bl, 0x2A	;*
	jmp	.dowrite
      @@:
	cmp	ch, 71
	jb	.dowrite
	cmp	ch, 83
	ja	.dowrite
	;push    eax
	movzx	eax, ch
	mov	bl, [numlock_map + eax - 71]
	;pop     eax

;.........................Part1 End.................................................

	jmp	.dowrite
.scancode:
	mov	bl, ch
.dowrite:
       movzx eax,byte[KEY_COUNT]
       cmp   al,120
       jae   .exit.irq1
       inc   eax
       mov   [KEY_COUNT],al
       mov   [KEY_COUNT+eax],bl

   .exit.irq1:
       mov   [check_idle_semaphore],5

;       mov   al,0x20                  ; ready for next irq
;       out   0x20,al

;       restore_ring3_context
;       iret
	ret

set_lights:
       mov   al,0xED
       call  kb_write
       mov   al,[kb_lights]
       call  kb_write
       ret

;// mike.dld ]
;..........................Part2 Start.......Code by Rus.......................................
numlock_map:
    db	 0x37	;Num 7
    db	 0x38	;Num 8
    db	 0x39	;Num 9
    db	 0x2D	;Num -
    db	 0x34	;Num 4
    db	 0x35	;Num 5
    db	 0x36	;Num 6
    db	 0x2B	;Num +
    db	 0x31	;Num 1
    db	 0x32	;Num 2
    db	 0x33	;Num 3
    db	 0x30	;Num 0
    db	 0x2E	;Num .
;..........................Part2 End................................................