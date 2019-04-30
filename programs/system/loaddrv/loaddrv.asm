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
        print   'LoadDrv: Error loading driver'
		print   'Driver must be in /sys/drivers/ folder.'
		print   'Its name must be w/o extension and it is case-sensitive'
        mov     eax, -1
        int     0x40
ok:
        print   'LoadDrv: Driver loaded well'
        mov     eax, -1
        int     0x40

i_end:

        driver_name  rb 1024

mem: