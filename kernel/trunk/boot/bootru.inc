;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;=================================================================
;
;                           BOOT DATA
;
;=================================================================

$Revision$


d80x25_bottom:
          db       186,' KolibriOS основана на MenuetOS и НЕ ПРЕДОСТАВЛЯЕТ НИ'
          db       'КАКИХ ГАРAНТИЙ.          ',186
          db       186,' Подробнее смотрите в файле COPYING.TXT              '
          db       '                         ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ", 0
novesa    db      "Видеокарта: EGA/CGA",13,10,0
s_vesa    db      "Версия VESA: "
  .ver    db      "?.?",13,10,0

gr_mode   db      "Выберите видеорежим: ",13,10,0

ask_bd     db      "Добавить диски, видимые через BIOS в режиме V86? [1-да, 2-нет]: ",0

if defined extended_primary_loader
bdev      db      "Загрузить образ из [1-дискета; 2-kolibri.img из папки загрузки]: ",0
else
bdev      db      "Загрузить образ из [1-дискета; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-использовать уже загруженный образ;"
          db      13,10,186,"                    "
          db      "4-создать чистый образ]: ",0
end if

prnotfnd  db      "Ошибка - Видеорежим не найден.",0

not386    db      "Ошибка - Требуется процессор 386+.",0
fatalsel  db      "Ошибка - Выбранный видеорежим не поддерживается.",0
pres_key  db      "Нажимите любую клавишу, для перехода в выбор режимов.",0
badsect   db      13,10,186," Ошибка - Дискета повреждена. Попробуйте другую.",0
memmovefailed db  13,10,186," Ошибка - Int 0x15 move failed.",0
okt       db      " ... OK"
linef     db      13,10,0
diskload  db      "Загрузка дискеты: 00 %",8,8,8,8,0
pros      db      "00"
backspace2 db     8,8,0
boot_dev  db      0
start_msg db      "Нажмите [abcd] для изменения настроек, [Enter] для продолжения загрузки",13,10,0
time_msg  db      " или подождите "
time_str  db      " 5 секунд "
          db      " до автоматического продолжения",13,10,0
current_cfg_msg db "Текущие настройки:",13,10,0
curvideo_msg db   " [a] Видеорежим: ",0

mode0     db      "320x200, EGA/CGA 256 цветов",13,10,0
mode9     db      "640x480, VGA 16 цветов",13,10,0  

usebd_msg db      " [b] Добавить диски, видимые через BIOS:",0
on_msg    db      " вкл",13,10,0
off_msg   db      " выкл",13,10,0

preboot_device_msg db " [c] Образ дискеты: ",0

if defined extended_primary_loader
preboot_device_msgs dw 0,pdm1,pdm2,0
pdm1      db      "настоящая дискета",13,10,0
pdm2      db      "kolibri.img из папки загрузки",13,10,0
else
preboot_device_msgs dw 0,pdm1,pdm2,pdm3,pdm4
pdm1      db      "настоящая дискета",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "использовать уже загруженный образ",13,10,0
pdm4      db      "создать чистый образ",13,10,0
end if

loading_msg db    "Идёт загрузка KolibriOS...",0

if ~ defined extended_primary_loader ; saving not supported in this case
save_quest db     "Запомнить текущие настройки? [y/n]: ",0
loader_block_error db "Ошибка в данных начального загрузчика, продолжение невозможно.",0
end if

_st db 186,'                   ┌───────────────────────────────┬─┐  ',13,10,0
_r1 db 186,'                   │  320x200  EGA/CGA 256 цветов  │ │  ',13,10,0
_r2 db 186,'                   │  640x480  VGA 16 цветов       │ │  ',13,10,0
_rs db 186,'                   │  ????x????@??  SVGA VESA      │ │  ',13,10,0
_bt db 186,'                   └───────────────────────────────┴─┘  ',13,10,0

remark1 db      "Значения по умолчанию выбраны для удобства большинства, но не всех.",0
remark2 db      "Если у Вас не грузится система, попробуйте отключить пункт [b].",0
remarks dw      remark1, remark2
num_remarks = 2
