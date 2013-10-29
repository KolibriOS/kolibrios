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
          latin1   '║ KolibriOS on IGASUGUSE GARANTIITA. Vaata faili COPYING info saamiseks. Kui   ║'
          latin1   '║ leiate vigu, anna neist palun teada aadressil: http://board.kolibrios.org    ║'
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   latin1  " APM x.x ", 0
novesa    latin1  "Ekraan: EGA/CGA",13,10,0
s_vesa    latin1  "Vesa versioon: "
  .ver    db      "?.?",13,10,0

gr_mode   latin1  "Vali video resolutsioon: ",13,10,0

ask_bd    latin1  "Lisa V86 reziimis BIOSle nähtavad kettad? [1-jah, 2-ei]: ",0

if defined extended_primary_loader
bdev      latin1  "Paigalda mäluketas [1-diskett; 2-kolibri.img]: ",0
else
bdev      latin1  "Paigalda mäluketas [1-diskett; 2-C:\kolibri.img (FAT32);"
          latin1  13,10,"║                    "
          latin1  "3-kasuta eellaaditud mäluketast kerneli restardist;"
          latin1  13,10,"║                    "
          latin1  "4-loo tühi pilt]: ",0
end if

prnotfnd  latin1  "Fataalne - Video resolutsiooni ei leitud.",0

not386    latin1  "Fataalne - CPU 386+ on vajalik.",0
fatalsel  latin1  "Fataalne - Riistvara ei toeta graafilist resolutsiooni.",0
pres_key  latin1  "Vajutage suvalist klahvi, et valida uus videomode.",0
badsect   latin1  13,10,"║ Fataalne - Vigane sektor. Asenda diskett.",0
memmovefailed latin1  13,10,"║ Fataalne - Int 0x15 liigutamine ebaõnnestus.",0
okt       latin1  " ... OK"
linef     latin1  13,10,0
diskload  latin1  "Loen disketti: 00 %",8,8,8,8,0
pros      latin1  "00"
backspace2 latin1 8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg latin1  "Vajuta [abcde] seadete muutmiseks, vajuta [Enter] laadimise jätkamiseks",13,10,0
time_msg  latin1  " või oota "
time_str  latin1  " 5 sekundit"
          latin1  " automaatseks jätkamiseks",13,10,0
current_cfg_msg latin1 "Praegused seaded:",13,10,0
curvideo_msg latin1   " [a] Video resolutsioon: ",0

mode0     latin1  "320x200, EGA/CGA 256 värvi",0
mode9     latin1  "640x480, VGA 16 värvi",0

usebd_msg latin1  " [b] Lisa BIOSle nähtavad kettad:",0
on_msg    latin1  " sees",13,10,0
off_msg   latin1  " väljas",13,10,0

debug_mode_msg  latin1 " [c] Dubleeri silumisinfo ekraanile:",0
ask_debug       latin1 "Dubleeri silumisinfo ekraanile? [1-jah, 2-ei]: ",0

launcher_msg    latin1 " [d] Käivita LAUNCHER pärast kerneli laadimist:",0
ask_launcher    latin1 "Käivita esimese programm (LAUNCHER) peale kerneli laadimist? [1-jah, 2-ei]: ",0

preboot_device_msg latin1 " [e] Disketi kujutis: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1      latin1  "reaalne diskett",13,10,0
pdm2      latin1  "kolibri.img",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3,pdm4,0
pdm1      latin1  "reaalne diskett",13,10,0
pdm2      latin1  "C:\kolibri.img (FAT32)",13,10,0
pdm3      latin1  "kasuta juba laaditud kujutist",13,10,0
pdm4      latin1  "loo tühi pilt",13,10,0
end if

loading_msg latin1 "Laadin KolibriOS...",0

if ~ defined extended_primary_loader
save_quest latin1 "Jäta meelde praegused seaded? [y/n]: ",0
loader_block_error latin1 "Alglaaduri andmed vigased, ei saa jätkata. Peatatud.",0
end if

_st latin1 '║                   ┌───────────────────────────────┬─┐',13,10,0
_r1 latin1 '║                   │  320x200  EGA/CGA 256 colors  │ │',13,10,0
_r2 latin1 '║                   │  640x480  VGA 16 colors       │ │',13,10,0
_rs latin1 '║                   │  ????x????@??  SVGA VESA      │ │',13,10,0
_bt latin1 '║                   └───────────────────────────────┴─┘',13,10,0

remark1 latin1  "Vaikimisi väärtused on kasutatavad enamikes arvutites, kuid mitte kõigis.",0
remark2 latin1  "Kui süsteem ei käivitu, proovige lülitada kirje [b] välja. Kui see läheb",0
remark3 latin1  "kinni pärast käivitamist, võimaldama valik [c], keelake [d] ja teha foto.",0
remarks dw      remark1, remark2, remark3
num_remarks = 3