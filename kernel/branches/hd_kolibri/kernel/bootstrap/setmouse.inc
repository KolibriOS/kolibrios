; SET MOUSE

        mov   esi,boot_setmouse
        call  boot_log
        call  setmouse

        mov  [pci_access_enabled],1
