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

; Para modificar éste archivo es necesario abrirlo con codificación CP850

$Revision: 2455 $


d80x25_bottom:
          cp850    '║ KolibriOS y viene ABSOLUTAMENTE SIN GARANTíA.            '
          cp850    '                    ║'
          cp850    '║ Lee el archivo COPYING por más detalles                  '
          cp850    '                    ║'
          line_full_bottom
d80x25_bottom_num = 3

msg_apm:  cp850   " APM x.x ", 0
novesa:   cp850   "Monitor: EGA/CGA",13,10,0
s_vesa:   cp850   "Versión de VESA: "
  .ver    db      "?.?",13,10,0

gr_mode:  cp850   "Selecciona un modo de video: ",13,10,0

ask_bd:   cp850   "¿Agregar discos visibles por el BIOS emulados en modo V86? [1-si, 2-no]: ",0

if defined extended_primary_loader
bdev:     cp850   "Cargar unidad ram desde [1-disquete; 2-kolibri.img]: ",0
else
bdev:     cp850   "Cargar unidad ram desde [1-disquete; 2-C:\kolibri.img (FAT32);"
          cp850   13,10,"║                    "
          cp850   "3-usar imagen precargada en el reinicio del núcleo;"
          cp850   13,10,"║                    "
          cp850   "4-crear imagen vacía]: ",0
end if

prnotfnd: cp850   "Fatal - Modo de video no encontrado.",0

not386:   cp850   "Fatal - CPU 386+ requerido.",0
fatalsel: cp850   "Fatal - Modo de gráficos no soportado por hardware.",0
pres_key: cp850   "Presiona una tecla para seleccionar otro modo de video.",0
badsect:  cp850   13,10,"║ Fatal - Sector mal. Reemplaze el disquete.",0
memmovefailed:cp850 13,10,"║ Fatal - Int 0x15 move failed.",0
okt:      cp850   " ... BIEN"
linef:    cp850   13,10,0
diskload: cp850   "Cargando disquete: 00 %",8,8,8,8,0
pros:     cp850   "00"
backspace2:cp850  8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg:cp850   "Presiona [abcde] para cambiar la configuración, [Enter] para continuar",13,10,0
time_msg: cp850   " o espera "
time_str: cp850   " 5 segundos"
          cp850   " para que inicie automáticamente",13,10,0
current_cfg_msg:cp850 "Configuración actual:",13,10,0
curvideo_msg:cp850 " [a] Modo de video: ",0

mode0:    cp850   "320x200, EGA/CGA 256 colores",13,10,0
mode9:    cp850   "640x480, VGA 16 colores",13,10,0

usebd_msg:cp850   " [b] Agregar discos visibles por el BIOS:",0
on_msg:   cp850   " activado",13,10,0
off_msg:  cp850   " desactivado",13,10,0

debug_mode_msg: cp850 " [c] Duplicar depurar salida a la pantalla:",0
ask_debug:      cp850 "¿Duplicar depurar la salida a la pantalla? [1-si, 2-no]: ",0

launcher_msg:   cp850 " [d] Iniciar LAUNCHER después de cargar kernel:",0
ask_launcher:   cp850 "¿Inicie la primera aplicación después de cargar el kernel? [1-si, 2-no]: ",0

preboot_device_msg:cp850 " [e] Imagen de disquete: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1:     cp850   "disquete real",13,10,0
pdm2:     cp850   "C:\kolibri.img (FAT32)",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3
pdm1:     cp850   "disquete real",13,10,0
pdm2:     cp850   "C:\kolibri.img (FAT32)",13,10,0
pdm3:     cp850   "usar imagen ya cargada",13,10,0
pdm4:     cp850   "crear imagen vacía",13,10,0
end if

loading_msg:cp850 "Cargando KolibriOS...",0

if ~ defined extended_primary_loader
save_quest:cp850  "¿Recordar configuración actual? [s/n]: ",0
loader_block_error:cp850 "Bootloader inválido, no puedo continuar. Detenido.",0
end if

_st:cp850 '║                   ┌───────────────────────────────┬─┐',13,10,0
_r1:cp850 '║                   │  320x200 EGA/CGA 256 colores  │ │',13,10,0
_r2:cp850 '║                   │  640x480 VGA 16 colores       │ │',13,10,0
_rs:cp850 '║                   │  ????x????@??  SVGA VESA      │ │',13,10,0
_bt:cp850 '║                   └───────────────────────────────┴─┘',13,10,0

remark1:cp850   "Los valores por defecto puede que no funcionen en algunas configuraciones.",0
remark2:cp850   "Si el sistema no inicia, prueba deshabilitar la opción [b].",0
remarks dw      remark1, remark2
num_remarks = 2
