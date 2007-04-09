 ; TIMER SET TO 1/100 S

        mov   esi,boot_timer
        call  boot_log
        mov   al,0x34              ; set to 100Hz
        out   0x43,al
        mov   al,0x9b              ; lsb    1193180 / 1193
        out   0x40,al
        mov   al,0x2e              ; msb
        out   0x40,al