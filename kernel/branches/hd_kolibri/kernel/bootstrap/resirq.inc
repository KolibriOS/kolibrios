; RESERVE SYSTEM IRQ'S JA PORT'S

        mov   esi,boot_resirqports
        call  boot_log
        call  reserve_irqs_ports
