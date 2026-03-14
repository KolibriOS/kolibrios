;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem
        dd      mem
        dd      driver_name, 0  ; NAME W/O EXT, NOT PATH. SEE f68.16

include '../../debug.inc'
		
start:
        mov     eax, 68
        mov     ebx, 16
        mov     ecx, driver_name
        int     0x40

        cmp     eax, 0
        jne     ok
nok:
        print   'E: LoadDrv: Error loading driver'
                print   'I: Usage: loaddrv <driver_name>'
		print   'I: Driver must be stored as /sys/drivers/<driver_name>.sys'
		print   'I: <driver_name> must be w/o extension and it is case-sensitive'
        mov     eax, -1
        int     0x40
ok:
        print   'I: LoadDrv: Driver loaded well'
        mov     eax, -1
        int     0x40

i_end:

        driver_name  rb 1024

mem: