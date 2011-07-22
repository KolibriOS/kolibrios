;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


flm               db 0
preboot_lfb       db 0
preboot_bootlog   db 0
boot_drive        db 0
bx_from_load:     dw 'r1'  ; структура для хранения параметров- откуда гашрузились, берется ниже из bx ; {SPraid}[13.03.2007]
                           ; a,b,c,d - винчестеры, r - рам диск
                           ; # диска... символ, а не байт. '1', а не 1

align 4
old_ints_h:
    dw      0x400
    dd      0
    dw      0

if ~ defined extended_primary_loader ; restart from memory is not supported in extended primary loader cfg
kernel_restart_bootblock:
    db    1    ; version
    dw    1    ; floppy image is in memory
    dd    0    ; cannot save parameters
end if

; table for move to extended memory (int 15h, ah=87h)
align 8
movedesc:
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

        db      0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0
        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0

        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

fwmovedesc:
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0
        db      0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0

        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

if defined extended_primary_loader
; look in PrimaryLoader.txt for the description
bootdevice      dw      0                       ; ax from primary loader
bootfs          dw      0                       ; bx from primary loader
bootcallback    dd      0                       ; ds:si from primary loader
; data for configuration file loading, look in PrimaryLoader.txt
config_file_struct:
        dw      0, 4000h ; load to 4000:0000
        dw      16       ; read no more than 16*4K = 64K
        db      'config.ini',0
; data for configuration file parsing
macro config_variable string,parser
{
local len
len     dw      0
        db      string
store word $ - len - 2 at len
        dw      parser
}
config_file_variables:
        config_variable 'timeout', parse_timeout
        config_variable 'resolution', parse_resolution
        config_variable 'vbemode', parse_vbemode
        config_variable 'vrr', parse_vrr
        config_variable 'biosdisks', parse_biosdisks
        config_variable 'imgfrom', parse_imgfrom
        dw      0
; data for image file loading, look in PrimaryLoader.txt
image_file_struct:
        dw      0, 4000h ; load to 4000:0000
        dw      16       ; read no more than 16*4K = 64K
        db      'kolibri.img',0
end if
