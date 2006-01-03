draw_menu:

    cmp  byte[menu],'0'
    jne  menu1

    mov  eax,4
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-39)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*4)
    mov  ecx,0x10ffff00
    mov  edx,startgame
    mov  esi,instr-startgame
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-84)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*5)
    mov  ecx,0x10ff0000
    mov  edx,instr
    mov  esi,hist-instr
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-50)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*6)
    mov  ecx,0x10ff0000
    mov  edx,hist
    mov  esi,quit-hist
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-26)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*7)
    mov  ecx,0x10ff0000
    mov  edx,quit
    mov  esi,paused-quit
    int  0x40

menu1:
    cmp  byte[menu],'1'
    jne  menu2


    mov  eax,4
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-39)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*4)
    mov  ecx,0x10ff0000
    mov  edx,startgame
    mov  esi,instr-startgame
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-84)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*5)
    mov  ecx,0x10ffff00
    mov  edx,instr
    mov  esi,hist-instr
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-50)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*6)
    mov  ecx,0x10ff0000
    mov  edx,hist
    mov  esi,quit-hist
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-26)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*7)
    mov  ecx,0x10ff0000
    mov  edx,quit
    mov  esi,paused-quit
    int  0x40

menu2:
    cmp  byte[menu],'2'
    jne  menu3

    mov  eax,4
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-39)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*4)
    mov  ecx,0x10ff0000
    mov  edx,startgame
    mov  esi,instr-startgame
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-84)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*5)
    mov  ecx,0x10ff0000
    mov  edx,instr
    mov  esi,hist-instr
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-50)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*6)
    mov  ecx,0x10ffff00
    mov  edx,hist
    mov  esi,quit-hist
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-26)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*7)
    mov  ecx,0x10ff0000
    mov  edx,quit
    mov  esi,paused-quit
    int  0x40

menu3:
    cmp  byte[menu],'3'
    jne  menu4

    mov  eax,4
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-39)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*4)
    mov  ecx,0x10ff0000
    mov  edx,startgame
    mov  esi,instr-startgame
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-84)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*5)
    mov  ecx,0x10ff0000
    mov  edx,instr
    mov  esi,hist-instr
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-50)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*6)
    mov  ecx,0x10ff0000
    mov  edx,hist
    mov  esi,quit-hist
    int  0x40
    mov  ebx,((LEN_X-BORDER_LEFT-BORDER_RIGHT)*ADOBE_SIZE+X_LOCATION*2+98-26)/2*65536+(((LEN_Y-BORDER_TOP-BORDER_BOTTOM)*ADOBE_SIZE+Y_LOCATION-2)/8*7)
    mov  ecx,0x10ffff00
    mov  edx,quit
    mov  esi,paused-quit
    int  0x40

menu4:


ret

exemenu:
    cmp  byte[menu],'0'  ;start
    jne  exemenu1
    call new_game

exemenu1:
    cmp  byte[menu],'1'  ;instr
    jne  exemenu2
    mov  byte[status],'4'
    call draw_window

exemenu2:
    cmp  byte[menu],'2'  ;history
    jne  exemenu3
    mov  byte[status],'3'
    call draw_window

exemenu3:
    cmp  byte[menu],'3'  ;exit
    jne  exemenu4
    mov  eax,-1
    int  0x40

exemenu4:

ret
