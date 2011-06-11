;---------------------------------------------------------------------
kfile:
	test	dword [status],1
	jnz	still
	or	dword [status],1

	call	load_directory.copy_dir_path

;	call	get_filter_data
	
	
	cmp [OpenDialog_path],dword 'NoKe'
	je	@f
	mov [OpenDialog_data.start_path],dword OpenDialog_path
	jmp	.start_OpenDialog
@@:
	mov	ebx,open_dialog_name
	mov	esi,path
	mov	edi,library_path
	call	copy_file_path
	
.start_OpenDialog:
	
;start_OpenDialog	OpenDialog_data
	push    dword OpenDialog_data
	call    [OpenDialog_Start]

	call	clear_control_key_flag

	cmp	[OpenDialog_data.status],2
	je	thread1_start
	cmp	[OpenDialog_data.status],1
	jne	@f
	xor	dword [status],1
	jmp	kopen_1
@@:
	xor	dword [status],1
	jmp	still
	
thread1_start:

	call	get_memory_for_tread_stack
	mov	[thread_stack],eax
	mcall	51,1,thread1	;, thread_stack
	jmp	still
;---------------------------------------------------------------------
get_filter_data:
	mov	edi,Filter+4
	xor	eax,eax
	mov	ecx,10
	cld
@@:
	mov	esi,10
	sub	esi,ecx
	lea     esi,[esi+esi*2] ; x 3
	shl	esi,3  ; x 8
	add	esi,dword Convert_plugin_0.Assoc
	mov	esi,[esi]
	add	esi,4

	test	esi,esi
	jz	@f
	call	.start
	dec	ecx
	jnz	@r
@@:
	mov	[edi],byte 0
	mov	eax,Filter
	sub	edi,eax
	mov	[eax],edi

	ret
.start:
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@r
	cmp	[esi],ah
	jne	@r
	ret
;---------------------------------------------------------------------
thread1:			; start of thread1
    mcall 9, procinfo_threads, -1
    mov   eax,[ebx+30]
    mov   [PID1],eax
	mcall 40, 0x27

   or  ecx,-1		; get information about me
   call getappinfo

    mov  esi,string
@@:
    cld
    lodsb
    test al,al
    jne  @r
    sub  esi,string
    mov  eax,esi
    dec  eax
    mov edi, edit1
    mov [edi+48], eax  ;ed_size
    mov [edi+52], eax  ;ed_pos
.red:
    call .draw_window

.still:

    mcall 10		; wait here for event

    cmp  eax,1			; redraw request ?
    je	 .red
    cmp  eax,2			; key in buffer ?
    je	 .key
    cmp  eax,3			; button in buffer ?
    je	 .button

	push    dword name_editboxes
	call    [edit_box_mouse]
;	mouse_edit_box name_editboxes
;    mouse_menubar menu_data_2
    jmp  .still

.key: 			; key
    mcall 2
    cmp ah,13
    je	.close_with_open_file
    cmp ah,27
    je	.close
	
	push    dword name_editboxes
	call    [edit_box_key]
;    key_edit_box name_editboxes
    jmp  .still

.close_with_open_file:
	mov  [open_file_flag],byte 1
	jmp  .close
	
.button:			; button
    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; button id=1 ?
    jne  .still
.close:
;    bts  dword [status],2
;    btr  dword [status],0
	xor  dword [status],1
	xor   eax,eax
    mov   [PID1],eax
	mcall	68,13,[thread_stack]
    mov  eax,-1 		; close this program
    mcall

    jmp  .still

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
    mov  ecx,100*65536+80	   ; [y start] *65536 + [y size]
    mov  edx,0x03eeeeee 	   ; color of work area RRGGBB,8->color gl
    mcall

				   ; WINDOW LABEL
				   
    mcall 71,1, labelt1

	push    dword name_editboxes
	call    [edit_box_draw]
;	draw_edit_box name_editboxes
;    mcall 47,0x80000,[PID1],<200, 5>,0xffffff	
;    mcall 71,1, proverka
;    mcall 47,0x80001,0,<300, 5>,0xffffff
;	draw_menu_bar menu_data_2
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall

    ret
	

;---------------------------------------------------------------------