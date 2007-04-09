; SET PORTS FOR IRQ HANDLERS

        mov  esi,boot_setrports
        call boot_log
        call setirqreadports