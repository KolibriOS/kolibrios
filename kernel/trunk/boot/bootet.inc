;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
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
          latin1   '║ KolibriOS kaasas IGASUGUSE GARANTIITA. Naha faili COPY'
          latin1   'ING detailid           ║'
          line_full_bottom
d80x25_bottom_num = 2

msg_apm:  latin1  " APM x.x ", 0
novesa:   latin1  "Ekraan: EGA/CGA",13,10,0
s_vesa:   latin1  "Vesa versioon: "
  .ver    db      "?.?",13,10,0

gr_mode:  latin1  "Vali videomode: ",13,10,0

ask_bd:   latin1  "Lisa kettad nahtavaks BIOS reziim V86? [1-jah, 2-no]: ",0

if defined extended_primary_loader
bdev:     latin1  "Paigalda mäluketas [1-diskett; 2-kolibri.img]: ",0
else
bdev:     latin1  "Paigalda mäluketas [1-diskett; 2-C:\kolibri.img (FAT32);"
          latin1  13,10,"║                    "
          latin1  "3-kasuta eellaaditud mäluketast kerneli restardist;"
          latin1  13,10,"║                    "
          latin1  "4-loo tühi pilt]: ",0
end if

prnotfnd: latin1  "Fataalne - Videoreziimi ei leitud.",0

not386:   latin1  "Fataalne - CPU 386+ on vajalik.",0
fatalsel: latin1  "Fataalne - Graafilist reziimi riistvara ei toeta.",0
pres_key: latin1  "Vajutage suvalist klahvi, et valida uus videomode.",0
badsect:  latin1  13,10,"║ Fataalne - Vigane sektor. Asenda diskett.",0
memmovefailed:latin1  13,10,"║ Fataalne - Int 0x15 liigutamine ebaõnnestus.",0
okt:      latin1  " ... OK"
linef:    latin1  13,10,0
diskload: latin1  "Loen disketti: 00 %",8,8,8,8,0
pros:     latin1  "00"
backspace2:latin1 8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg:latin1  "Vajuta [abc] seadete muutmiseks, vajuta [Enter] laadimise jätkamiseks",13,10,0
time_msg: latin1  " või oota "
time_str: latin1  " 5 sekundit"
          latin1  " automaatseks jätkamiseks",13,10,0
current_cfg_msg:latin1 "Praegused seaded:",13,10,0
curvideo_msg:latin1   " [a] Videoreziim: ",0

mode0:    latin1  "320x200, EGA/CGA 256 värvi",0
mode9:    latin1  "640x480, VGA 16 värvi",0

usebd_msg:latin1  " [b] Lisa kettad nahtavaks BIOS:",0
on_msg:   latin1  " sees",13,10,0
off_msg:  latin1  " väljas",13,10,0

preboot_device_msg:latin1 " [c] Disketi kujutis: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1:     latin1  "reaalne diskett",13,10,0
pdm2:     latin1  "kolibri.img",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3
pdm1:     latin1  "reaalne diskett",13,10,0
pdm2:     latin1  "C:\kolibri.img (FAT32)",13,10,0
pdm3:     latin1  "kasuta juba laaditud kujutist",13,10,0
pdm4:     latin1  "loo tühi pilt",13,10,0
end if

loading_msg:latin1 "Laadin KolibriOS...",0

if ~ defined extended_primary_loader
save_quest:latin1 "Jäta meelde praegused seaded? [y/n]: ",0
loader_block_error:latin1 "Alglaaduri andmed vigased, ei saa jätkata. Peatatud.",0
end if

_st:latin1 '║                   ┌───────────────────────────────┬─┐',13,10,0
_r1:latin1 '║                   │  320x200  EGA/CGA 256 colors  │ │',13,10,0
_r2:latin1 '║                   │  640x480  VGA 16 colors       │ │',13,10,0
_rs:latin1 '║                   │  ????x????@??  SVGA VESA      │ │',13,10,0
_bt:latin1 '║                   └───────────────────────────────┴─┘',13,10,0

remark1:latin1  "Vaikimisi maaratud vaartused on valitud mugavuse enamikes, kuid mitte koik.",0
remark2:latin1  "Kui susteem ei kaivitu, proovige lulitada kirje [b].",0
remarks dw      remark1, remark2
num_remarks = 2
