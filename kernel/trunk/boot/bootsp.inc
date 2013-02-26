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

; Para modificar ‚ste archivo es necesario abrirlo con codificaci¢n CP850

$Revision: 2455 $


d80x25_bottom:
          db       186,' KolibriOS est  basado en MenuetOS y viene ABSOLUTAMENTE  '
          db       'SIN GARANT¡A        ',186
          db       186,' Lee el archivo COPYING por m s detalles                  '
          db       '                    ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ", 0
novesa    db      "Monitor: EGA/CGA",13,10,0
s_vesa    db      "Versi¢n de VESA: "
  .ver    db      "?.?",13,10,0

gr_mode   db      "Selecciona un modo de video: ",13,10,0

ask_bd     db      "¨Agregar discos visibles por el BIOS emulados en modo V86? [1-si, 2-no]: ",0

if defined extended_primary_loader
bdev      db      "Cargar ramdisk desde [1-disquete; 2-kolibri.img]: ",0
else
bdev      db      "Cargar ramdisk desde [1-disquete; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-usar imagen precargada en el reinicio del kernel;"
          db      13,10,186,"                    "
          db      "4-crear imagen vac¡a]: ",0
end if

prnotfnd  db      "Fatal - Modo de video no encontrado.",0

not386    db      "Fatal - CPU 386+ requerido.",0
fatalsel  db      "Fatal - Modo de gr ficos no soportado por hardware.",0
pres_key  db      "Presiona una tecla para seleccionar otro modo de video.",0
badsect   db      13,10,186," Fatal - Sector mal. Reemplaze el disquete.",0
memmovefailed db  13,10,186," Fatal - Int 0x15 move failed.",0
okt       db      " ... BIEN"
linef     db      13,10,0
diskload  db      "Cargando disquete: 00 %",8,8,8,8,0
pros      db      "00"
backspace2 db     8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg db      "Presiona [abcd] para cambiar la configuraci¢n, [Enter] para continuar",13,10,0
time_msg  db      " o espera "
time_str  db      " 5 segundos"
          db      " para que inicie autom ticamente",13,10,0
current_cfg_msg db "Configuraci¢n actual:",13,10,0
curvideo_msg db   " [a] Modo de video: ",0

mode0     db      "320x200, EGA/CGA 256 colores",13,10,0
mode9     db      "640x480, VGA 16 colores",13,10,0

usebd_msg db      " [b] Agregar discos visibles por el BIOS:",0
on_msg    db      " activado",13,10,0
off_msg   db      " desactivado",13,10,0

preboot_device_msg db " [c] Imagen de disquete: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1      db      "disquete real",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3
pdm1      db      "disquete real",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "usar imagen ya cargada",13,10,0
pdm4      db      "crear imagen vac¡a",13,10,0
end if

loading_msg db    "Cargando KolibriOS...",0

if ~ defined extended_primary_loader
save_quest db     "¨Recordar configuraci¢n actual? [s/n]: ",0
loader_block_error db "Bootloader inv lido, no puedo continuar. Detenido.",0
end if

_st db 186,'                   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄ¿',13,10,0
_r1 db 186,'                   ³  320x200 EGA/CGA 256 colores  ³ ³',13,10,0
_r2 db 186,'                   ³  640x480 VGA 16 colores       ³ ³',13,10,0
_rs db 186,'                   ³  ????x????@??  SVGA VESA      ³ ³',13,10,0
_bt db 186,'                   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÙ',13,10,0

remark1 db      "Los valores por defecto puede que no funcionen en algunas configuraciones.",0
remark2 db      "Si el sistema no inicia, prueba deshabilitar la opci¢n [b].",0
remarks dw      remark1, remark2
num_remarks = 2
