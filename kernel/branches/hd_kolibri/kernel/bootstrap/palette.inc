
; PALETTE FOR 320x200 and 640x480 16 col

        cmp   [SCR_MODE],word 0x12
        jne   no_pal_vga
        mov   esi,boot_pal_vga
        call  boot_log
        call  paletteVGA
      no_pal_vga:

        cmp   [SCR_MODE],word 0x13
        jne   no_pal_ega
        mov   esi,boot_pal_ega
        call  boot_log
        call  palette320x200
      no_pal_ega:
