;-------------------------------------------------------------------------
;
;    Файл конфигурации
;
;
;     SPraid
;
;-------------------------------------------------------------------------

conf_file_loaded: db 0			; флаг загрузки конфига

; загрузка ини файла в библиотеку  
proc load_conf_file
  pushad
  invoke ini.load,.fname
  mov [conf_file_loaded],byte 1
  popad
  ret
  .fname db '%sys%/sys.conf',0
endp