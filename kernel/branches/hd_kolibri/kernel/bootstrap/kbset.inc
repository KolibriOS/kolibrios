; SET KEYBOARD PARAMETERS
        mov   al, 0xf6         ; reset keyboard, scan enabled
        call  kb_write

        ; wait until 8042 is ready
        xor ecx,ecx
      @@:
        in     al,64h
        and    al,00000010b
        loopnz @b

       ; mov   al, 0xED       ; svetodiody - only for testing!
       ; call  kb_write
       ; call  kb_read
       ; mov   al, 111b
       ; call  kb_write
       ; call  kb_read

        mov   al, 0xF3       ; set repeat rate & delay
        call  kb_write
;        call  kb_read
        mov   al, 0 ; 30 250 ;00100010b ; 24 500  ;00100100b  ; 20 500
        call  kb_write
;        call  kb_read
     ;// mike.dld [
        call  set_lights
     ;// mike.dld ]

