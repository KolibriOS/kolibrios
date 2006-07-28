write_score:

    mov  eax,13
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2-3)*65536+95	   ;clear box to write new score
    mov  ecx,20*65536+((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-20)
    mov  edx,0x00000000
    int  40h

    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2)*65536+25	 ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,scoretext
    mov  esi,linestext-scoretext
    mov  eax,4
    int  0x40

    mov  eax,[score]
    call number_to_str


    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+35)*65536+25	    ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,number_str
    mov  esi,[size_of_number_str]
    mov  eax,4
    int  0x40

    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2)*65536+35	 ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,linestext
    mov  esi,speedtext-linestext
    mov  eax,4
    int  0x40

    mov  eax,[lines]
    call number_to_str


    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+35)*65536+35	    ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,number_str
    mov  esi,[size_of_number_str]
    mov  eax,4
    int  0x40

    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2)*65536+45	 ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,speedtext
    mov  esi,leveltext-speedtext
    mov  eax,4
    int  0x40

    mov  eax,[speed]
    call number_to_str


    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+35)*65536+45	    ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,number_str
    mov  esi,[size_of_number_str]
    mov  eax,4
    int  0x40

    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2)*65536+55	 ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,leveltext
    mov  esi,startgame-leveltext
    mov  eax,4
    int  0x40

    mov  eax,[level]
    call number_to_str


    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+35)*65536+55	    ; draw info text with function 4
    mov  ecx,0xffff00		   ; color
    mov  edx,number_str
    mov  esi,[size_of_number_str]
    mov  eax,4
    int  0x40

    call draw_next_block

ret


number_to_str:

    mov edi,end_number_str-1
    mov ecx,9;size_of_number_str
    mov ebx,10
    cld
  new_digit:
    xor edx,edx
    div ebx
    add dl,'0'
    mov [edi],dl
    dec edi
    loop new_digit

ret