;---------------------------------------------------------------------
clear_thread:
	call	get_memory_for_tread_stack
	mov	[clear_thread_stack],eax
	mcall	51, 1, clear_thread_start	;, clear_thread_stack
	ret
;---------------------------------------------------------------------
clear_thread_start:
	mcall	5,100
@@:
	mcall	23,10
	cmp	eax,5
	je	@f
	mcall	15,3
	mcall	68,13,[clear_thread_stack]
	mcall	-1
;---------------------------------------------------------------------