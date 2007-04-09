; PRINT AMOUNT OF MEMORY
        mov     esi, boot_memdetect
        call    boot_log

        movzx   ecx, word [boot_y]
        or      ecx, (10+29*6) shl 16 ; "Determining amount of memory"
        sub     ecx, 10
        mov     edx, 0xFFFFFF
        mov     ebx, [MEM_AMOUNT]
        shr     ebx, 20
        mov     edi, 1
        mov     eax, 0x00040000
        call    display_number_force
