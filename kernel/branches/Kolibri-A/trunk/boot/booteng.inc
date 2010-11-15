;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;======================================================================
;
;                           BOOT DATA
;
;======================================================================

$Revision$


d80x25_bottom:
	  db	   186,' KolibriOS is based on MenuetOS and comes with ABSOLUTELY '
	  db	   'NO WARRANTY         ',186
	  db	   186,' See file COPYING for details                             '
	  db	   '                    ',186
	  line_full_bottom
d80x25_bottom_num = 3

msg_apm   db	  " APM x.x ", 0
vervesa   db	  "Version of Vesa: Vesa x.x",13,10,0
;novesa    db      "Display: EGA/CGA",13,10,0
s_vesa	  db	  "Version of VESA: "
  .ver	  db	  "?.?",13,10,0

;gr_mode   db      "Select a videomode: ",13,10,0

;vrrmprint db      "Apply VRR? (picture frequency greater than 60Hz"
;          db      " only for transfers:",13,10
;          db      186," 1024*768->800*600 and 800*600->640*480) [1-yes,2-no]:",0


;ask_bd     db      "Add disks visible by BIOS emulated in V86-mode? [1-yes, 2-no]: ",0

;bdev      db      "Load ramdisk from [1-floppy; 2-C:\kolibri.img (FAT32);"
;          db      13,10,186,"                    "
;          db      "3-use preloaded ram-image from kernel restart;"
;          db      13,10,186,"                    "
;          db      "4-create blank image]: ",0
;probetext db      13,10,13,10,186," Use standart graphics mode? [1-yes, "
;          db      "2-probe bios (Vesa 3.0)]: ",0
;prnotfnd  db      "Fatal - Videomode not found.",0
;btns      db      "Fatal - Can't determine color depth.",0
;badsect   db      13,10,186," Fatal - Bad sector. Replace floppy.",0
;memmovefailed db  13,10,186," Fatal - Int 0x15 move failed.",0
okt	  db	  " ... OK"
linef	  db	  13,10,0
diskload  db	  "Loading diskette: 00 %",8,8,8,8,0
pros	  db	  "00"
backspace2 db	  8,8,0
boot_dev  db	  0  ; 0=floppy, 1=hd
;start_msg db      "Press [abcd] to change settings, press [Enter] to continue booting",13,10,0
;time_msg  db      " or wait "
;time_str  db      " 5 seconds"
;          db      " before automatical continuation",13,10,0
;current_cfg_msg db "cfgmanager.printcfg reached",13,10,0
;curvideo_msg db   " [a] Videomode: ",0

loading_msg db	  "Loading KolibriOS...",0
;save_quest db     "Remember current settings? [y/n]: ",0
loader_block_error db "Invalid boot data",0
