mouse:


    mov  eax,37 		      ; get mouse position                        ; EXPERIMENTAL !!!
    mov  ebx,1			      ; window relative                           ;
    int  0x40			      ;                                           ;
    shr  eax,16 		      ; we only need to know the x position       ;
    sub  eax,X_LOCATION 							  ;
										  ;
    cmp  eax,ADOBE_SIZE*(LEN_X-2)     ; check to see if mouse is in field         ;
    jg	 no_mouse		      ;                                           ;
    cmp  eax,0			      ;                                           ;
    jl	 no_mouse		      ;                                           ;
										  ;
    mov  ebx,0									  ;
										  ;
   mouseloop:									  ;
    cmp  eax,ADOBE_SIZE 							  ;
    jl	 yes_mouse								  ;
    sub  eax,ADOBE_SIZE 							  ;
    inc  ebx									  ;
    jmp  mouseloop								  ;
										  ;
   yes_mouse:									  ;
    cmp  ebx,[current_block_x]
    jg	 go_right
    jl	 go_left
;    mov  dword[current_block_x],ebx                                               ;

    mov  eax,37 		      ; get mouse position                        ;
    mov  ebx,2			      ; buttons pressed (1=left, 2=right, 3=both) ;
    int  0x40									  ;
										  ;
    cmp  eax,0									  ;
    jne  yes_mouse_button							  ;
    mov  byte[lastmousebutton],0						  ;
    jmp  no_mouse								  ;
   yes_mouse_button:								  ;
										  ;
    cmp  eax,2									  ;
    jne  no_left_mouse								  ;
    cmp  byte[status],'1'							  ;
    jne  no_left_mouse								  ;
    mov  byte[lastmousebutton],2						  ;
    inc  dword [current_block_y]						  ;
    call check_crash								  ;
    jne  block_crash								  ;
    jmp  no_mouse								  ;
										  ;
   no_left_mouse:								  ;
										  ;
										  ;
    cmp  eax,1									  ;
    jne  no_right_mouse 							  ;
    cmp  byte[status],'1'							  ;
    jne  no_right_mouse 							  ;
    cmp  byte[lastmousebutton],1						  ;
    mov  byte[lastmousebutton],1						  ;
    je	 no_right_mouse 							  ;
    call rotate_block								  ;
    call check_crash								  ;
    jne  block_crash								  ;
    call draw_block								  ;
    jmp  scendi 								  ;
										  ;
   no_right_mouse:								  ;
										  ;
										  ;
    cmp  eax,3									  ;
    jne  no_mouse								  ;
    cmp  byte[lastmousebutton],3						  ;
    mov  byte[lastmousebutton],3						  ;
    je	 no_mouse								  ;
    cmp  byte[status],'2'							  ;
    je	 unpause								  ;
    mov  byte[status],'2'							  ;
    call draw_window								  ;
    jmp  attesa 								  ;
;   unpause:                                                                       ;
;    mov  byte[status],'1'                                                         ;
;    call draw_window                                                              ;
;    jmp  still                                                                    ;
										  ;
   ;                                                                              ;
   no_mouse:									  ;

ret