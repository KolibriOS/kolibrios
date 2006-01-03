help_thread_start:
	call	draw_help_wnd

  still_hw:
	cmp	[main_closed],1
	je	exit_hw
	mcall	10
	cmp	eax,1
	je	help_thread_start
	cmp	eax,2
	je	key_hw
	cmp	eax,3
	je	button_hw
	jmp	still_hw

  key_hw:
	mcall	;2
	cmp	ah, 27
	jne	still_hw

  button_hw:
	mcall	17
  exit_hw:
	mcall	-1

func draw_help_wnd
	mcall	12,1

	mcall	48,3,sc,sizeof.system_colors

	mov	ecx,[skinh]
	add	ecx,200*65536+(5+10*2+16*10)
	mov	edx,[sc.work]
	or	edx,0x03000000
	mcall	0,<200,10+10*2+help_text.maxl*6>

	mov	ebx,[skinh]
	shr	ebx,1
	adc	ebx,1+0x000A0000-4
	mcall	4,,[sc.grab_text],help_title,help_title.size

	mov	eax,4
	mov	ebx,[skinh]
	add	ebx,0x000F000A
	xor	ecx,ecx
	mov	edx,help_text
    @@: inc	edx
	movzx	esi,byte[edx-1]
	mcall
	add	ebx,10
	add	edx,esi
	cmp	byte[edx],0
	jne	@b

	mcall	12,2
	ret
endf