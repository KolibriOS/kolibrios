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

$Revision: 4135 $


d80x25_bottom:
           db       186,' KolibriOS wird ohne jegliche Garantie vertrieben. Details stehen in der      ',186
           db       186,' Datei COPYING. Bitte melden Sie Fehler bei: http://board.kolibrios.org       ',186           
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ", 0
novesa    db      "Anzeige: EGA/CGA ",13,10,0
s_vesa    db      "Vesa-Version: "
  .ver    db      "?.?",13,10,0

gr_mode   db      "Wahlen Sie einen videomode: ",13,10,0

ask_bd     db      "Add-Festplatten sichtbar BIOS in V86-Modus emuliert? [1-ja, 2 nein]: ",0

if defined extended_primary_loader
bdev      db      "Lade die Ramdisk von [1-Diskette; 2-kolibri.img]: ",0
else
bdev      db      "Lade die Ramdisk von [1-Diskette; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-benutze ein bereits geladenes Kernel image;"
          db      13,10,186,"                    "
          db      "4-create blank image]: ",0
end if

prnotfnd  db      "Fatal - Videomodus nicht gefunden.",0

not386    db      "Fatal - CPU 386+ benoetigt.",0
fatalsel  db      "Fatal - Grafikmodus nicht unterstuetzt.",0
pres_key  db      "Drucken Sie eine beliebige Taste, um eine neue videomode wahlen.",0
badsect   db      13,10,186," Fatal - Sektorfehler, Andere Diskette neutzen.",0
memmovefailed db  13,10,186," Fatal - Int 0x15 Fehler.",0
okt       db      " ... OK"
linef     db      13,10,0
diskload  db      "Lade Diskette: 00 %",8,8,8,8,0
pros      db      "00"
backspace2 db     8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg db      "Druecke [abcde], um die Einstellungen zu aendern, druecke [Enter] zum starten",13,10,0
time_msg  db      " oder warte "
time_str  db      " 5 Sekunden"
          db      " bis zum automatischen Start",13,10,0
current_cfg_msg db "Aktuelle Einstellungen:",13,10,0
curvideo_msg db   " [a] Videomodus: ",0

mode0     db      "320x200, EGA/CGA 256 colors",13,10,0
mode9     db      "640x480, VGA 16 colors",13,10,0

usebd_msg db      " [b] Add-Festplatten sichtbar durch das BIOS:",0
on_msg    db      " an",13,10,0
off_msg   db      " aus",13,10,0

debug_mode_msg db " [c] Duplizieren debuggen Ausgabe auf dem Bildschirm:",0
ask_debug      db "Duplizieren debuggen Ausgabe auf dem Bildschirm? [1-ja, 2 nein]: ",0

launcher_msg   db " [d] Start LAUNCHER nach Kernel geladen wird:",0
ask_launcher   db "Starten erste Anwendung nach Kernel geladen wird? [1-ja, 2 nein]: ",0

preboot_device_msg db " [e] Diskettenimage: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1      db      "Echte Diskette",13,10,0
pdm2      db      "kolibri.img",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3,pdm4,0
pdm1      db      "Echte Diskette",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "Nutze bereits geladenes Image",13,10,0
pdm4      db      "create blank image",13,10,0
end if

loading_msg db    "Lade KolibriOS...",0

if ~ defined extended_primary_loader
save_quest db     "Aktuelle Einstellungen speichern? [y/n]: ",0
loader_block_error db "Bootloader Daten ungueltig, Kann nicht fortfahren. Angehalten.",0
end if

_st latin1 '║                   ┌───────────────────────────────┬─┐',13,10,0
_r1 latin1 '║                   │  320x200  EGA/CGA 256 colors  │ │',13,10,0
_r2 latin1 '║                   │  640x480  VGA 16 colors       │ │',13,10,0
_rs latin1 '║                   │  ????x????@??  SVGA VESA      │ │',13,10,0
_bt latin1 '║                   └───────────────────────────────┴─┘',13,10,0

remark1 db      "Die Standardwerte sind fur die meisten gewahlt, aber nicht fur jedermann.",0
remark2 db      "Wenn das System nicht bootet, das Option [b] deaktivieren versuchen. Wenn es",0
remark3 db      "nach dem Booten hangen bleibt, aktivieren Sie Option [c], deaktivieren [d]",0
remark4 db      "und machen Fotos.",0
remarks dw      remark1, remark2, remark3, remark4
num_remarks = 4