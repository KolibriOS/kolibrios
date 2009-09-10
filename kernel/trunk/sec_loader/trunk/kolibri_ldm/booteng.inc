;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                     ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved.        ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa             ;;
;; Distributed under terms of the GNU General Public License           ;;
;;                                                                     ;;
;;  booteng.asm the module for Secondary Loader                        ;;
;;                                                                     ;;
;;  KolibriOS 16-bit loader module,                                    ;;
;;                        based on bootcode for KolibriOS              ;;
;;                                                                     ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;======================================================================
;
;                           BOOT DATA
;
;======================================================================

d80x25_bottom:
          db       186,' KolibriOS is based on MenuetOS and comes with ABSOLUTELY '
          db       'NO WARRANTY         ',186
          db       186,' See file COPYING for details                             '
          db       '                    ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ", 0
vervesa   db      "Version of Vesa: Vesa x.x",13,10,0
novesa    db      "Display: EGA/CGA",13,10,0
s_vesa    db      "Version of VESA: "
  .ver    db      "?.?",13,10,0

gr_mode   db      "Select a videomode: ",13,10,0

vrrmprint db      "Apply VRR? (picture frequency greater than 60Hz"
          db      " only for transfers:",13,10
          db      186," 1024*768->800*600 and 800*600->640*480) [1-yes,2-no]:",0


ask_bd     db      "Add disks visible by BIOS emulated in V86-mode? [1-yes, 2-no]: ",0

bdev      db      "Load ramdisk from [1-floppy; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-use preloaded ram-image from kernel restart;"
          db      13,10,186,"                    "
          db      "4-create blank image]: ",0
probetext db      13,10,13,10,186," Use standart graphics mode? [1-yes, "
          db      "2-probe bios (Vesa 3.0)]: ",0
prnotfnd  db      "Fatal - Videomode not found.",0
not386    db      "Fatal - CPU 386+ required.",0
btns      db      "Fatal - Can't determine color depth.",0
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
start_msg db      "Press [abc] to change settings, press [Enter] to continue booting",13,10,0
time_msg  db      " or wait "
time_str  db      " 5 seconds"
          db      " before automatical continuation",13,10,0
current_cfg_msg db "Current settings:",13,10,0
curvideo_msg db   " [a] Videomode: ",0

mode0     db      "320x200, EGA/CGA 256 colors",13,10,0
mode9     db      "640x480, VGA 16 colors",13,10,0

usebd_msg db      " [b] Add disks visible by BIOS:",0
on_msg    db      " on",13,10,0
off_msg   db      " off",13,10,0
;readonly_msg db   " only for reading",13,10,0
vrrm_msg  db      " [c] Use VRR:",0
preboot_device_msg db " [d] Floppy image: ",0
preboot_device_msgs dw 0,pdm1,pdm2,pdm3
pdm1      db      "real floppy",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "use already loaded image",13,10,0
pdm4      db      "create blank image",13,10,0
loading_msg db    "Loading KolibriOS...",0
save_quest db     "Remember current settings? [y/n]: ",0
loader_block_error db "Bootloader data invalid, I cannot continue. Stopped.",0

_st db 186,'                   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄ¿',13,10,0
_r1 db 186,'                   ³  320x200  EGA/CGA 256 colors  ³ ³',13,10,0
_r2 db 186,'                   ³  640x480  VGA 16 colors       ³ ³',13,10,0
_rs db 186,'                   ³  ????x????@??  SVGA VESA      ³ ³',13,10,0
_bt db 186,'                   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÙ',13,10,0

remark1 db      "Default values were selected to match most of configurations, but not all.",0
remark2 db      "If you have LCD-monitor, disable VRR in the item [c] - you do not need it.",0
remark3 db      "If the system does not boot, try to disable the item [b].",0
remarks dw      remark1, remark2, remark3
num_remarks = 3
