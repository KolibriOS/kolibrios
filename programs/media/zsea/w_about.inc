;---------------------------------------------------------------------
  kabout:
	test dword [status],4
	jnz	 still
	or	 dword [status],4

	call	get_memory_for_tread_stack
	mov	[thread_stack_3],eax
	mcall	51,1,thread3	;,thread_stack_3
	jmp	still
;---------------------------------------------------------------------
thread3:			  ; start of bgrd thread
    mcall 9, procinfo_threads, -1
    mov   eax,[ebx+30]
    mov   [PID3],eax
	mcall 40, 0x7
.red:
     call .draw_window

.still:

    mov  eax,10 		; wait here for event
    mcall

    cmp  eax,1			; redraw request ?
    je	 .red
    cmp  eax,2			; key in buffer ?
    je	 .close   ;.key
    cmp  eax,3			; button in buffer ?
    je	 .button

    jmp  .still

;  .key:
;    mcall
;    cmp  ah,27
;    je	 .close
;    jmp  .still


.button:			 ; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
	je   .close
	cmp  ah,2
    jne  .still   ;noclose3
.close:
;    btr dword [status],3
;    bts dword [status],2
	xor  dword [status],4
	xor   eax,eax
	mov   [PID3],eax
	mcall	68,13,[thread_stack_3]
	mov  eax,-1 	; close this program
	mcall
;  .noclose:
;     jmp  still3




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


.draw_window:


    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall

				   ; DRAW WINDOW
    xor  eax,eax		   ; function 0 : define and draw window
    mov  ebx,100*65536+300	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+240	   ; [y start] *65536 + [y size]
    mov  edx,0x03eeeeee 	   ; color of work area RRGGBB,8->color gl
    mcall

    mov  ebx,120*65536+40
    mov  ecx,210*65536+20
    mov  edx,2
    mov  esi,0xdddddd
    mcall 8
	
	pusha
    mcall 4,<20,30>,0x80000000,Authors_text.1
	mcall  ,<30,45>, ,Authors_text.2

	mcall  ,<30,60>, ,Authors_text.3
	mcall  ,<60,70>, ,Authors_text.4
	mcall  ,<60,80>, ,Authors_text.5
	mcall  ,<60,90>, ,Authors_text.6
	mcall  ,<60,100>, ,Authors_text.7

	mcall  ,<30,115>, ,Authors_text.8
	mcall  ,<60,125>, ,Authors_text.9
	mcall  ,<60,135>, ,Authors_text.10

	mcall  ,<30,150>, ,Authors_text.11
	mcall  ,<60,160>, ,Authors_text.12

	mcall  ,<30,175>, ,Authors_text.13
	mcall  ,<60,185>, ,Authors_text.14
	popa
	
    add  ebx,15 shl 16
	shr  ecx,16
	mov  bx,cx
	add  ebx,6
	
	mov  ecx,0
    mov  edx, ok_btn
    mov  esi, ok_btn.size ;2
    mcall 4
	
				   ; WINDOW LABEL
    mcall 71,1, labelt3
;    mcall 47,0x80000,[PID3],<200, 5>,0xffffff
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret
;---------------------------------------------------------------------