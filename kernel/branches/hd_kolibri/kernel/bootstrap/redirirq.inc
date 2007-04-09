; REDIRECT ALL IRQ'S TO INT'S 0x20-0x2f

        mov   esi,boot_irqs
        call  boot_log
        call  rerouteirqs

        mov    esi,boot_tss
        call   boot_log

        mov    esi,boot_devices
        call   boot_log
        call   detect_devices

