draw_window:


    mov  eax,12 								  ; function 12:tell os about windowdraw
    mov  ebx,1									  ; 1, start of draw
    int  0x40

										  ; DRAW WINDOW
    mov  eax,0									  ; function 0 : define and draw window
    mov  ebx,320*65536+(LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98
    mov  ecx,25*65536+ (LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION+4
    mov  edx,0x03000000 							  ; color of work area RRGGBB
    mov  esi,0x006688ee 							  ; color of grab bar  RRGGBB,8->col
    mov  edi,0x007799ff 							  ; color of frames    RRGGBB
    int  0x40

										  ; WINDOW LABEL
    mov  eax,4									  ; function 4 : write text to window
    mov  ebx,8*65536+5								  ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff 							  ; color of text RRGGBB
    mov  edx,labelt								  ; pointer to text beginning
    mov  esi,scoretext-labelt							  ; text length
    int  0x40

    cmp  byte[status],'0'
    jne  status1
    call draw_logo
    call draw_menu

   status1:
    cmp  byte[status],'1'
    jne  status2

    call draw_game

   status2:
    cmp  byte[status],'2'
    jne  status3

    call draw_game

    mov  eax,4									  ; function 4 : write text to window
    mov  ebx,80*65536+170							 ; [x start] *65536 + [y start]
    mov  ecx,0x10ff0000 							  ; color of text RRGGBB
    mov  edx,paused								  ; pointer to text beginning
    mov  esi,txt_end-paused							  ; text length
    int  0x40

   status3:
    cmp  byte[status],'3'
    jne  status4

    call draw_logo
    mov  edx,history
    call show_text

   status4:
    cmp  byte[status],'4'
    jne  status5

    call draw_logo
    mov  edx,instructions
    call show_text

   status5:

    mov  eax,12 								  ; function 12:tell os about windowdraw
    mov  ebx,2									  ; 2, end of draw
    int  0x40

ret

draw_game:

    call draw_table
    movzx edx,byte [current_block_color]
    call draw_block

    call write_score

    mov  eax,38
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2-4) shl 16 + ((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2-4)
    mov  ecx,20 shl 16 + ((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION+2-4)
    mov  edx,0x00ffffff
    int  0x40

ret

draw_logo:

    mov ebx,2
    mov ecx,2
    mov edx,[color_table+1*4]
    mov edi,tetris_t
    call draw_title_block
    mov ebx,5
    mov ecx,3
    mov edx,[color_table+2*4]
    mov edi,tetris_e
    call draw_title_block
    mov ebx,8
    mov ecx,2
    mov edx,[color_table+3*4]
    mov edi,tetris_t
    call draw_title_block
    mov ebx,11
    mov ecx,3
    mov edx,[color_table+4*4]
    mov edi,tetris_r
    call draw_title_block
    mov ebx,13
    mov ecx,2
    mov edx,[color_table+5*4]
    mov edi,tetris_i
    call draw_title_block
    mov ebx,15
    mov ecx,3
    mov edx,[color_table+6*4]
    mov edi,tetris_s
    call draw_title_block
    mov ebx,20
    mov ecx,2
    mov edx,[color_table+7*4]
    mov edi,tetris_II
    call draw_title_block

ret

show_text:
    mov  ebx,6*65536+120	   ; draw info text with function 4
    mov  ecx,0xff0000
    mov  esi,34
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,34
    cmp  [edx],byte 'x'
    jne  newline
ret


