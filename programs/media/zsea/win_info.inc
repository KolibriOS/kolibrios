;---------------------------------------------------------------------
  kinfo:
	test dword [status],2
	jnz      still
	or	dword [status],2

	call	get_memory_for_tread_stack
	mov	[thread_stack_2],eax
	mcall	51,1,thread2	;, thread_stack_2
	jmp	still
;---------------------------------------------------------------------
thread2:			    ; start of info thread
	mcall 9, procinfo_threads, -1
	mov   eax,[ebx+30]
	mov   [PID2],eax
	mcall 40, 0x7
	call load_directory.copy_dir_path
.red:
	call .draw_window

.still:

	mov  eax,10	     ; wait here for event
	mcall

	cmp  eax,1			; redraw request ?
	je	.red
	cmp  eax,2			; key in buffer ?
	je	.close
	cmp  eax,3			; button in buffer ?
	je	.button

	jmp  .still

.button:			   ; button
	mov  eax,17	     ; get id
	mcall

	cmp  ah,1			 ; button id=1 ?
	jne  .still  ;.noclose
.close:
;	btr dword [status],1
;	bts dword [status],2
	xor	dword [status],2
	xor	eax,eax
	mov	[PID2],eax
	mcall	68,13,[thread_stack_2]
	mov	eax,-1	     ; close this program
	mcall
;.noclose:
;	jmp  .still




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


.draw_window:


	mov  eax,12		 ; function 12:tell os about windowdraw
	mov  ebx,1			   ; 1, start of draw
	mcall

					; DRAW WINDOW
	xor  eax,eax		; function 0 : define and draw window
	mov  ebx,100*65536+330     ; [x start] *65536 + [x size]
	mov  ecx,100*65536+120  ;290	; [y start] *65536 + [y size]
	mov  edx,0x03eeeeee	; color of work area RRGGBB,8->color gl
	mcall

					; WINDOW LABEL
	mcall 71,1, labelt2

;	mov  ecx,0x80000000	; font 1 & color ( 0xF0RRGGBB )
;	mov  ebx,120*65536+30
;	mov  edx,file_name  ;string
;	mov  esi,1  ;43
	mcall 4,<120,30>,0x80000000,file_name
	mov  edx,fitext
	mov  esi,14
	mov  ebx,10*65536+30
	mov  ecx,0x10000000
@@:
	mcall
	add  ebx,10
	add  edx,esi
	cmp  ebx,10*65536+80
	jbe  @r
	mov  eax,47
	mov  edx,120*65536+40
	mov  esi,ecx
	mov  ecx, [img_size]
	mov  ebx,0x800a0000
	mcall
	add  edx,10
	mov  ecx,[img_width_2]
	mcall
	add  edx,10
	mov  ecx,[img_high_2]
	mcall
	add  edx,10
	mov  ecx,[img_resolution_2]
	mcall
	add  edx,10
	mov  ecx,[Total_Repeat_N]
	mcall

;	add  edx,10
;	mov	ecx,[communication_area]
;	mov ecx,[ecx+4096]
;	mcall

;	add  edx,10
;	mov	ecx,[communication_area]
;	mov ecx,[ecx+4096+4]
;	mcall
	
;	add  edx,10+10
;	mov  ecx,[zoom_auto_factor]
;	mcall

;	add  edx,10+10
;	mov  ecx,[scroll_bar_data_vertical.max_area] ;[img_resolution]
;	mcall
;	add  edx,10
;	mov  ecx,[scroll_bar_data_vertical.cur_area]  ;[return_code]
;	mcall
;	add  edx,10
;	mov  ecx,[scroll_bar_data_vertical.position]
;	mcall
;	add  edx,10
;	xor  ecx,ecx
;	mov  cx,[scroll_bar_data_vertical.size_y]
;	mcall

;	add  edx,10+10
;	mcall 47,0x80000,[OpenDialog_Init]

;	add  edx,10+10
;	mcall 47,0x80000,[OpenDialog_Start]

;	add  edx,10+10
;	mcall 47,0x80000,[OpenDialog_data.com_area]

;	add  edx,10+10
;	mcall 47,0x80000,I_END

;	add  edx,10+10
;	mov  ecx,[scroll_bar_data_horizontal.max_area]  ;[img_resolution]
;	mcall
;	add  edx,10
;	mov  ecx,[scroll_bar_data_horizontal.cur_area]  ;[return_code]
;	mcall
;	add  edx,10
;	mov  ecx,[scroll_bar_data_horizontal.position]
;	mcall
;	add  edx,10
;	xor  ecx,ecx
;	mov  cx,[scroll_bar_data_horizontal.size_x]
;	mcall

;	add  edx,10
;	mov  ecx,[raw_pointer] ;[deflate_unpack]
;	mcall

;	add  edx,10+10
;	mov  ebp,test1  ;raw_pointer_2
;	mov  ecx,[ebp]
;	mcall
	
;	add  edx,10+10
;	mov  ebp,size_before_open_dir ;lib_init   ;test1  ;raw_pointer_2
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[ebp]
;	mcall
	
;	add  edx,10
;	add  ebp,4
;	mov  ecx,[version_lib_ini]
;;	mov  ecx,[ecx]
;	mcall

;	add  edx,10
;	movzx  ecx,word  [image_draw_size.x]
;	mcall
;	add  edx,10
;	movzx  ecx,word [image_draw_size.y]
;	mcall
;	add  edx,10+10
;	movzx  ecx,word [wa_width]
;	mcall
;	add  edx,10
;	movzx  ecx,word [wa_high]
;	mcall
;	add  edx,10
;	movzx  ecx,word [wa_width_1]
;	mcall
;	add  edx,10
;	movzx  ecx,word [wa_width_2]
;	mcall
;	add  edx,10
;	movzx  ecx,word [wa_high_1]
;	mcall
;	add  edx,10
;	movzx  ecx,word [wa_high_2]
;	mcall

;	add  edx,10+10
;	mov  ecx,[window_width]
;	mcall
;	add  edx,10
;	mov  ecx,[window_high]
;	mcall
	
;	add  edx,10
;	xor  ecx,ecx
;	mov  cl,[Disposal_Method]
;	mcall
	
;	add  edx,10
;	mov  ecx,[soi]
;	mcall
;	mcall 47,0x80000,[PID2],<200, 5>,0xffffff
;	mcall 47,0x800a0000,[error_locate],<140,5>,0x10ffffff
	mov  eax,12		 ; function 12:tell os about windowdraw
	mov  ebx,2			   ; 2, end of draw
	mcall

	ret
;---------------------------------------------------------------------