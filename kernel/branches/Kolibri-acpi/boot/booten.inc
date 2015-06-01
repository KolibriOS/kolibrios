;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;======================================================================
;
;                           BOOT DATA
;
;======================================================================

$Revision: 2455 $


d80x25_bottom:
          db       186,' KolibriOS comes with ABSOLUTELY NO WARRANTY. See file COPYING for details    ',186
          db       186,' If you find any bugs, please report them at: http://board.kolibrios.org      ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ", 0
novesa    db      "Display: EGA/CGA",13,10,0
s_vesa    db      "Version of VESA: "
  .ver    db      "?.?",13,10,0

gr_mode   db      "Select a videomode: ",13,10,0

ask_bd     db      "Add disks visible by BIOS emulated in V86-mode? [1-yes, 2-no]: ",0

if defined extended_primary_loader
bdev      db      "Load ramdisk from [1-floppy; 2-kolibri.img]: ",0
else
bdev      db      "Load ramdisk from [1-floppy; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-use preloaded ram-image from kernel restart;"
          db      13,10,186,"                    "
          db      "4-create blank image]: ",0
end if

prnotfnd  db      "Fatal - Videomode not found.",0

not386    db      "Fatal - CPU 386+ required.",0
fatalsel  db      "Fatal - Graphics mode not supported by hardware.",0
pres_key  db      "Press any key to choose a new videomode.",0
badsect   db      13,10,186," Fatal - Bad sector. Replace floppy.",0
memmovefailed db  13,10,186," Fatal - Int 0x15 move failed.",0
okt       db      " ... OK"
linef     db      13,10,0
diskload  db      "Loading diskette: 00 %",8,8,8,8,0
pros      db      "00"
backspace2 db     8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg db      "Press [abcde] to change settings, press [Enter] to continue booting",13,10,0
time_msg  db      " or wait "
time_str  db      " 5 seconds"
          db      " to continue automatically",13,10,0
current_cfg_msg db "Current settings:",13,10,0
curvideo_msg db   " [a] Videomode: ",0

mode0     db      "320x200, EGA/CGA 256 colors",13,10,0
mode9     db      "640x480, VGA 16 colors",13,10,0

usebd_msg db      " [b] Add disks visible by BIOS:",0
on_msg    db      " on",13,10,0
off_msg   db      " off",13,10,0

debug_mode_msg db " [c] Duplicate debug output to the screen:",0
ask_debug      db "Duplicate debug output to the screen? [1-yes, 2-no]: ",0

launcher_msg   db " [d] Start LAUNCHER after kernel is loaded:",0
ask_launcher   db "Start first application (LAUNCHER) after kernel is loaded? [1-yes, 2-no]: ",0

preboot_device_msg db " [e] Floppy image: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1      db      "real floppy",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3,pdm4,0
pdm1      db      "real floppy",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "use already loaded image",13,10,0
pdm4      db      "create blank image",13,10,0
end if

loading_msg db    "Loading KolibriOS...",0

if ~ defined extended_primary_loader
save_quest db     "Remember current settings? [y/n]: ",0
loader_block_error db "Bootloader data invalid, I cannot continue. Stopped.",0
end if

_st latin1 '║                   ┌───────────────────────────────┬─┐',13,10,0
_r1 latin1 '║                   │  320x200  EGA/CGA 256 colors  │ │',13,10,0
_r2 latin1 '║                   │  640x480  VGA 16 colors       │ │',13,10,0
_rs latin1 '║                   │  ????x????@??  SVGA VESA      │ │',13,10,0
_bt latin1 '║                   └───────────────────────────────┴─┘',13,10,0

remark1 db      "Default values were selected to match most of configurations, but not all.",0
remark2 db      "If the system does not boot, try to disable option [b]. If the system gets",0
remark3 db      "stuck after booting, enable option [c], disable option [d] and make photo.",0
remarks dw      remark1, remark2, remark3
num_remarks = 3