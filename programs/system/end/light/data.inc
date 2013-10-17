if lang eq ru
  label2:
      db   'Выключить          Ядро',0
  label3:
      db   'Перезапуск        Отмена',0
  label4:
      db   'Сохранить настройки (Ctrl-S)',0

else if lang eq ge
  label2:
      db   ' Beenden          Kernel',0
  label3:
      db   ' Neustart       Abbrechen',0
  label4:
      db   'Save your settings (Ctrl-S)',0

else if lang eq it
  label2:
      db   '  Spegni          Kernel',0
  label3:
      db   '  Riavvio         Annulla',0
  label4:
      db   'Salva impostazioni (Ctrl-S)',0

else
  label2:
      db   'Power off         Kernel',0
  label3:
      db   ' Restart          Cancel',0
  label4:
      db   'Save your settings (Ctrl-S)',0

end if

  label5:
      db   '(End)           (Home)',0
  label6:
      db   '(Enter)          (Esc)',0

rdsave:
        dd      7,0,0,0,0
        db      '/sys/rdsave',0
