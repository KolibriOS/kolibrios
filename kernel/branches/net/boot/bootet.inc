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
          db       186,' KolibriOS pohineb MenuetOS ja kaasas IGASUGUSE GARANTI'
          db       'ITA                    ',186
          db       186,' Naha faili COPYING detailid                           '
          db       '                       ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ", 0
novesa    db      "Ekraan: EGA/CGA",13,10,0
s_vesa    db      "Vesa versioon: "
  .ver    db      "?.?",13,10,0

gr_mode   db      "Vali videomode: ",13,10,0

ask_bd     db      "Lisa kettad nahtavaks BIOS reziim V86? [1-jah, 2-no]: ",0

if defined extended_primary_loader
bdev      db      "Paigalda mäluketas [1-diskett; 2-kolibri.img]: ",0
else
bdev      db      "Paigalda mäluketas [1-diskett; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-kasuta eellaaditud mäluketast kerneli restardist;"
          db      13,10,186,"                    "
          db      "4-loo tühi pilt]: ",0
end if

prnotfnd  db      "Fataalne - Videoreziimi ei leitud.",0

not386    db      "Fataalne - CPU 386+ on vajalik.",0
fatalsel  db      "Fataalne - Graafilist reziimi riistvara ei toeta.",0
pres_key  db      "Vajutage suvalist klahvi, et valida uus videomode.",0
badsect   db      13,10,186," Fataalne - Vigane sektor. Asenda diskett.",0
memmovefailed db  13,10,186," Fataalne - Int 0x15 liigutamine ebaõnnestus.",0
okt       db      " ... OK"
linef     db      13,10,0
diskload  db      "Loen disketti: 00 %",8,8,8,8,0
pros      db      "00"
backspace2 db     8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg db      "Vajuta [abcd] seadete muutmiseks, vajuta [Enter] laadimise jätkamiseks",13,10,0
time_msg  db      " või oota "
time_str  db      " 5 sekundit"
          db      " automaatseks jätkamiseks",13,10,0
current_cfg_msg db "Praegused seaded:",13,10,0
curvideo_msg db   " [a] Videoreziim: ",0

mode0     db      "320x200, EGA/CGA 256 värvi",0
mode9     db      "640x480, VGA 16 värvi",0

usebd_msg db      " [b] Lisa kettad nahtavaks BIOS:",0
on_msg    db      " sees",13,10,0
off_msg   db      " väljas",13,10,0

preboot_device_msg db " [c] Disketi kujutis: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1      db      "reaalne diskett",13,10,0
pdm2      db      "kolibri.img",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3
pdm1      db      "reaalne diskett",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "kasuta juba laaditud kujutist",13,10,0
pdm4      db      "loo tühi pilt",13,10,0
end if

loading_msg db    "Laadin KolibriOS...",0

if ~ defined extended_primary_loader
save_quest db     "Jäta meelde praegused seaded? [y/n]: ",0
loader_block_error db "Alglaaduri andmed vigased, ei saa jätkata. Peatatud.",0
end if

_st db 186,'                   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄ¿',13,10,0
_r1 db 186,'                   ³  320x200  EGA/CGA 256 colors  ³ ³',13,10,0
_r2 db 186,'                   ³  640x480  VGA 16 colors       ³ ³',13,10,0
_rs db 186,'                   ³  ????x????@??  SVGA VESA      ³ ³',13,10,0
_bt db 186,'                   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÙ',13,10,0

remark1 db      "Vaikimisi maaratud vaartused on valitud mugavuse enamikes, kuid mitte koik.",0
remark2 db      "Kui susteem ei kaivitu, proovige lulitada kirje [b].",0
remarks dw      remark1, remark2
num_remarks = 2
