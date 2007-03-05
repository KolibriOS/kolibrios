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
space_msg:	line_space
verstr:
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
	repeat 78 - ($-verstr)
		db	' '
	end repeat
	db 32,186
		line_half
d80x25_top_num = 4
d80x25_bottom:
          db       186,' Kolibri OS основана на Menuet OS и не предоставляет '
          db       'никаких гарaнтий.        ',186
          db       186,' Подробнее смотрите файл GNU.TXT                     '
          db       '                         ',186
          line_full_bottom
d80x25_bottom_num = 3

novesa    db      "Видеокарта: EGA/CGA",13,10,0
vervesa   db      "Версия VESA:  Vesa x.x",13,10,0
vervesa_off=19
msg_apm	  db      " APM x.x ", 0
gr_mode   db      186," Vesa 2.0+ 16 M LFB:  [1] 640x480, [2] 800x600, "
          db      "[3] 1024x768, [4] 1280x1024",13,10
          db      186," Vesa 1.2  16 M Bnk:  [5] 640x480, [6] 800x600, "
          db      "[7] 1024x768, [8] 1280x1024",13,10
          db      186," EGA/CGA   256 Цветов:  [9] 320x200, "
          db      "VGA 16 Цветов: [0]  640x480",13,10
          db      186," Выберите видеорежим: ",0
bt24      db      "Глубина цвета: 24",13,10,0
bt32      db      "Глубина цвета: 32",13,10,0
vrrmprint db      "Использовать VRR? (частота кадров выше 60 Гц"
          db      " только для переходов:",13,10
          db      186," 1024*768>800*600 и 800*600>640*480) [1-да, 2-нет]: ",0
;askmouse  db      "Мышь:" ; 186, " "
;          db      " [1] PS/2 (USB), [2] Com1, [3] Com2."
;          db      " Выберите порт [1-3]: ",0
;no_com1   db      13,10,186," No COM1 mouse",0
;no_com2   db      13,10,186," No COM2 mouse",0
ask_dma   db      "Использовать DMA для записи на HDD? [1-да/2-нет]: ",0
;gr_direct db      186," Использовать линейный видеобуфер? "
;          db      "[1-да/2-нет]: ",0
;mem_model db      13,10,186," Объ+м памяти [1-16 Mb / 2-32 Mb / "
;          db      "3-64Mb / 4-128 Mb / 5-256 Mb]: ",0
;bootlog   db      13,10,186," Просмотреть журнал загрузки? [1-нет/2-да]: ",0
bdev      db      "Загрузить образ из [1-дискета; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-использовать уже загруженный образ]: ",0
probetext db      13,10,13,10,186," Стандартный видеорежим? [1-да, "
          db      "2-проверить другие (Vesa 3.0)]: ",0
;memokz256 db      13,10,186," RAM 256 Mb",0
;memokz128 db      13,10,186," RAM 128 Mb",0
;memokz64  db      13,10,186," RAM 64 Mb",0
;memokz32  db      13,10,186," RAM 32 Mb",0
;memokz16  db      13,10,186," RAM 16 Mb",0
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
modes_msg dw mode4,mode1,mode2,mode3
modevesa20 db     " с LFB",0
modevesa12 db     ", VESA 1.2 Bnk",0
mode9     db      "320x200, EGA/CGA 256 цветов",0
mode10    db      "640x480, VGA 16 цветов",0
probeno_msg db    " (стандартный видеорежим)",0
probeok_msg db    " (проверить нестандартные режимы)",0
dma_msg   db      " [b] Использование DMA для записи на HDD:",0
on_msg    db      " вкл",13,10,0
off_msg   db      " выкл",13,10,0
vrrm_msg  db      " [c] Использование VRR:",0
preboot_device_msg db " [d] Образ дискеты: ",0
preboot_device_msgs dw 0,pdm1,pdm2,pdm3
pdm1      db      "настоящая дискета",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "использовать уже загруженный образ",13,10,0
loading_msg db    "Идёт загрузка KolibriOS...",0
save_quest db     "Запомнить текущие настройки? [y/n]: ",0
loader_block_error db "Ошибка в данных начального загрузчика, продолжение невозможно.",0
