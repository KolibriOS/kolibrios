if lang eq ru
  label2:
      db   'Выключить       Ядро',0
  label3:
      db   'Перезапуск     Отмена',0
  label4:
      db   'Сохранить настройки (Ctrl-S)',0

else if lang eq ge
  label2:
      db   ' Beenden         Kernel',0
  label3:
      db   '  Neustart      Abbrechen',0
  label4:
      db   'Save your settings (Ctrl-S)',0

else if lang eq it
  label2:
      db   '   Spegni         Kernel',0
  label3:
      db   '   Riavvio        Annulla',0
  label4:
      db   'Salva impostazioni (Ctrl-S)',0

else
  label2:
      db   ' Power off        Kernel',0
  label3:
      db   '   Restart         Cancel',0
  label4:
      db   'Save your settings (Ctrl-S)',0

end if

  label5:
      db   '(End)         (Home)',0
  label6:
      db   '(Enter)         (Esc)',0

rdsave:
        dd  7,0,hide,0,0
        db  '/sys/rdsave',0
;---------------------------------------------------------------------
;Lib_DATA
;Всегда соблюдать последовательность в имени.
system_dir_Boxlib       db '/sys/lib/box_lib.obj',0
system_dir_libini       db '/sys/lib/libini.obj',0
;---------------------------------------------------------------------
head_f_i:
head_f_l        db 'System error',0

err_message_found_lib1  db 'box_lib.obj - Not found!',0
err_message_found_lib2  db 'libini.obj - Not found!',0

err_message_import1     db 'box_lib.obj - Wrong import!',0
err_message_import2     db 'libini.obj - Wrong import!',0
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, cur_dir_path, library_path, system_dir_Boxlib, \
err_message_found_lib1, head_f_l, Box_lib_import, err_message_import1, head_f_i

library02  l_libs system_dir_libini+9, cur_dir_path, library_path, system_dir_libini, \
err_message_found_lib2, head_f_l, libini_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------
align 4
Box_lib_import:
init_checkbox2    dd      aInit_checkbox2
check_box_draw2   dd      aCheck_box_draw2
check_box_mouse2  dd      aCheck_box_mouse2
;version_ch2       dd      aVersion_ch2
                  dd 0
                  dd 0
aInit_checkbox2   db 'init_checkbox2',0
aCheck_box_draw2  db 'check_box_draw2',0
aCheck_box_mouse2 db 'check_box_mouse2',0
;aVersion_ch2      db 'version_ch2',0
;---------------------------------------------------------------------
align 4
libini_import:
init_lib     dd a_init
ini_get_int  dd aini_get_int
ini_set_int  dd aini_set_int
             dd 0
             dd 0
a_init       db 'lib_init',0
aini_get_int db 'ini_get_int',0
aini_set_int db 'ini_set_int',0
;---------------------------------------------------------------------

check1 check_box2 20 shl 16 +8,93 shl 16 +8,6,0xffffff,0,0,label4

hide        db 'h',0
asettings   db 'settings',0
aautosave   db 'autosave',0
ini_file db  '/sys/settings/rdsave.ini',0


If fade=1
color1 dd 0x019098d0
color2 dd 0xFFFFFF
color3 dd 0xe4dfe1

color4 dd 0x990022
color5 dd 0xaa7700
color6 dd 0x777777
color7 dd 0x007700

color8 dd 0x90FFFFFF

color21 dd 0x01505890
color22 dd 0xbfbfbf
color23 dd 0xa49fa1

color24 dd 0x590011
color25 dd 0x593700
color26 dd 0x373737
color27 dd 0x003700

color28 dd 0x90bfbfbf
else
color1 dd 0x019098d0
color2 dd 0xFFFFFF
color3 dd 0xe4dfe1

color4 dd 0x990022
color5 dd 0xaa7700
color6 dd 0x777777
color7 dd 0x007700

color8 dd 0x90FFFFFF

if lang eq ru
  label7:
      db   'Сохранение образа...',0
else
  label7:
      db   'Saving RAM-drive...',0
end if

end if
