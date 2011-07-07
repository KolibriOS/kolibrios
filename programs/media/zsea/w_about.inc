;---------------------------------------------------------------------
kabout:
	test	dword [status],4
	jnz	still
	or	dword [status],4

	call	get_memory_for_thread_stack
	mov	[thread_stack_3],eax
	mcall	51,1,thread3	;thread_stack_3
	jmp	still
;---------------------------------------------------------------------
thread3:	  ; start of bgrd thread
	mcall	9, procinfo_threads, -1
	mov	eax,[ebx+30]
	mov	[PID3],eax
	mcall	40,0x7
;------------------------------
.red:
	call	.draw_window
;------------------------------
.still:
	mcall	10 	; wait here for event
	cmp	eax,1	; redraw request ?
	je	.red

	cmp	eax,2	; key in buffer ?
	je	.close   ;.key

	cmp	eax,3	; button in buffer ?
	je	.button

	jmp	.still
;---------------------------------------------------------------------
;.key:
;	mcall
;	cmp	ah,27
;	je	.close
;	jmp	.still
;---------------------------------------------------------------------
.button:			 ; button
	mcall	17 		; get id
	cmp	ah,1			; button id=1 ?
	je	.close
	
	cmp	ah,2
	jne	.still   ;noclose3
    
.close:
	xor	dword [status],4
	xor	eax,eax
	mov	[PID3],eax
	mcall	68,13,[thread_stack_3]
	mcall	-1 	; close this program
;---------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
.draw_window:
	mcall	12,1
	xor	eax,eax		   ; function 0 : define and draw window
	xor	esi,esi
	mcall	,<100,300>,<100,240>,0x13eeeeee,,labelt3
	mcall	8,<120,40>,<210,20>,2,0xdddddd
	add	ebx,15 shl 16
	shr	ecx,16
	mov	bx,cx
	add	ebx,6
	xor	ecx,ecx
	mcall	4,,,ok_btn,ok_btn.size
	mcall	,<20,30>,0x80000000,Authors_text.1
	mcall	,<30,45>, ,Authors_text.2
	mcall	,<30,60>, ,Authors_text.3
	mcall	,<60,70>, ,Authors_text.4
	mcall	,<60,80>, ,Authors_text.5
	mcall	,<60,90>, ,Authors_text.6
	mcall	,<60,100>, ,Authors_text.7
	mcall	,<30,115>, ,Authors_text.8
	mcall	,<60,125>, ,Authors_text.9
	mcall	,<60,135>, ,Authors_text.10
	mcall	,<30,150>, ,Authors_text.11
	mcall	,<60,160>, ,Authors_text.12
	mcall	,<30,175>, ,Authors_text.13
	mcall	,<60,185>, ,Authors_text.14
;	mcall	47,0x80000,[PID3],<200, 5>,0xffffff
	mcall	12,2
	ret
;---------------------------------------------------------------------
