display_modechg   db    0  ; display mode change for text, yes/no (0 or 2)
                           ;
                           ; !! Important note !!
                           ;
                           ; Must be set to 2, to avoid two screenmode
                           ; changes within a very short period of time.

display_atboot    db    0  ; show boot screen messages ( 2-no )

preboot_graph     db    0 ; graph mode
preboot_gprobe    db    0 ; probe vesa3 videomodes (1-no, 2-yes)
preboot_vrrm      db    0 ; use VRR_M (1-yes, 2- no)
;;preboot_mouse     db    0 ; mouse port (1-PS2, 2-COM1, 3-COM2)
preboot_mtrr      db    0 ; mtrr acceleration (1-yes, 2-no)
preboot_device    db    0 ; boot device
                           ; (1-floppy 2-harddisk 3-kernel restart)
;;preboot_memory    db    0  ; amount of memory
                           ; (1-16Mb;2-32Mb;3-64Mb;4-128Mb;5-256Mb)
                           ;       !!!! 0 - autodetect !!!!
preboot_blogesc   db    0  ; start immediately after bootlog

hdsysimage        db    'MENUET  IMG' ; load from
image_save        db    'MENUET  IMG' ; save to
