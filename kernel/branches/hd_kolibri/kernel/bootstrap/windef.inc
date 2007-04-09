; SET PRELIMINARY WINDOW STACK AND POSITIONS

        mov   esi,boot_windefs
        call  boot_log
        call  setwindowdefaults
