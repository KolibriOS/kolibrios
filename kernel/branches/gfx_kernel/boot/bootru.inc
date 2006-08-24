;======================================================================
;
;                           BOOT DATA
;
;======================================================================

macro line_full_top {
          db       201
          times    78   db 205
          db       187
}
macro line_full_bottom {
          db       200
          times    78   db 205
          db       188
}
macro line_half {
          db      186,' '
          times    76   db 0xc4
          db      ' ',186
}
macro line_space {
          db      186
          times   78 db 32
          db      186
}

d80x25_top:
		line_full_top
verstr2:
;		line_space
; version string
	db 186,32
	repeat 78
		load a byte from version+%-1
		if a = 13
			break
		end if
		db a
	end repeat
	repeat 78 - ($-verstr2)
		db	' '
	end repeat
	db 32,186
verstr:
		line_half
space_msg:	line_space
d80x25_top_num = 3
d80x25_bottom:
          db       186,' Kolibri OS основана на Menuet OS и не предоставляет '
          db       'никаких гарaнтий.        ',186
          db       186,' Подробнее смотрите файл GNU.TXT                     '
          db       '                         ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ",0

novesa    db      "Видеокарта: EGA/CGA",13,10,0
s_vesa    db      "Версия VESA: "
  .ver    db      "?.? ("
  .mem    db      "??? Мб)",13,10,0
gr_mode   db      "Выберите видеорежим: ",13,10,0
s_bpp     db      13,10,186," Глубина цвета: "
  .bpp    dw      "??"
          db      13,10,0
vrrmprint db      "Использовать VRR? (частота кадров выше 60 Гц"
          db      " только для переходов:",13,10
          db      186," 1024*768>800*600 и 800*600>640*480) [1-да, 2-нет]: ",0
gr_acc    db      "Vesa 2.0+: Включить MTRR для ускорения графики? "
          db      "[1-да/2-нет]: ",0
bdev      db      "Загрузить образ из [1-дискета; 2-C:\menuet.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-использовать уже загруженный образ]: ",0
probetext db      13,10,13,10,186," Стандартный видеорежим? [1-да, "
          db      "2-проверить другие (Vesa 3.0)]: ",0
prnotfnd  db      "Ошибка - Видеорежим не найден.",0
;modena    db      "Ошибка - Требуется поддержка VBE 0x112+.",0
not386    db      "Ошибка - Требуется процессор 386+.",0
btns      db      "Ошибка - Не могу определить глубину цвета.",0
fatalsel  db      "Ошибка - Выбранный видеорежим не поддерживается.",0
badsect   db      13,10,186," Ошибка - Дискета повреждена. Попробуйте другую.",0
memmovefailed db  13,10,186," Ошибка - Int 0x15 move failed.",0
okt       db      " ... OK"
linef	  db      13,10,0
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
mode1     db      "640x480",0
mode2     db      "800x600",0
mode3     db      "1024x768",0
mode4     db      "1280x1024",0
modes_msg dw mode4-0x10000,mode1-0x10000,mode2-0x10000,mode3-0x10000
modevesa20 db     " с LFB",0
modevesa12 db     ", VESA 1.2 Bnk",0
mode9     db      "320x200, EGA/CGA 256 цветов",0
mode10    db      "640x480, VGA 16 цветов",0
probeno_msg db    " (стандартный видеорежим)",0
probeok_msg db    " (проверить нестандартные режимы)",0
mtrr_msg  db      " [b] Использование MTRR для ускорения графики:",0
on_msg    db      " вкл",13,10,0
off_msg   db      " выкл",13,10,0
vrrm_msg  db      " [c] Использование VRR:",0
preboot_device_msg db " [d] Образ дискеты: ",0
preboot_device_msgs dw 0,pdm1-0x10000,pdm2-0x10000,pdm3-0x10000
pdm1      db      "настоящая дискета",13,10,0
pdm2      db      "C:\menuet.img (FAT32)",13,10,0
pdm3      db      "использовать уже загруженный образ",13,10,0
loading_msg db    "Идёт загрузка KolibriOS...",0
save_quest db     "Запомнить текущие настройки? [y/n]: ",0
loader_block_error db "Ошибка в данных начального загрузчика, продолжение невозможно.",0

_oem db 'oem: ',0

db 5
s_ven_intel  db 'Intel'
db 2
s_ven_s3     db 'S3'
;db 5
;s_ven_bochs  db 'Bochs'
;db 8
;s_ven_vmware db 'V M ware'

s_mode    db      "  ????-????-?? (?)  ",0
s_mode1   db      "  0640-0480-04 (a)    0320-0200-08 (b)  ",13,10,0

;_tl db '┌────────────┬─────┬─────┬─────┬─────┬─────┬─────┐',13,10,\
;       '│ разрешение │   4 │   8 │  15 │  16 │  24 │  32 │',13,10,\
;       '├────────────┼─────┼─────┼─────┼─────┼─────┼─────┤',13,10,0
;_rs db '│ ????x????  │  -  │  -  │  -  │  -  │  -  │  -  │',13,10,0
;_bt db '└────────────┴─────┴─────┴─────┴─────┴─────┴─────┘',13,10,0

_tl db 186,'                         ┌─────┬─────┬─────┬─────┬─────┬─────┐',13,10,\
       186,'                         │   4 │   8 │  15 │  16 │  24 │  32 │',13,10,\
       186,'             ┌───────────┼─────┼─────┼─────┼─────┼─────┼─────┼─┐',13,10,0
_rs db 186,'             │ ????x???? │  ?  │  ?  │  ?  │  ?  │  ?  │  ?  │█│',13,10,0
_bt db 186,'             └───────────┴─────┴─────┴─────┴─────┴─────┴─────┴─┘',13,10,0


_sel1 db 0x1A,0
_sel2 db 0x1B,0
