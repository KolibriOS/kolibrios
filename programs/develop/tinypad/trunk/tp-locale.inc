
sz htext,'TINYPAD ',APP_VERSION

menubar_res main_menu,\
  ru,'Файл'	,mm.File    ,onshow.file   ,\
  ru,'Правка'	,mm.Edit    ,onshow.edit   ,\
  ru,'Поиск'	,mm.Search  ,onshow.search ,\
  ru,'Запуск'	,mm.Run     ,onshow.run    ,\
  ru,'Кодировка',mm.Encoding,onshow.recode ,\
  ru,'Опции'	,mm.Options ,onshow.options,\
\
  en,'File'    ,mm.File    ,onshow.file   ,\
  en,'Edit'    ,mm.Edit    ,onshow.edit   ,\
  en,'Search'  ,mm.Search  ,onshow.search ,\
  en,'Run'     ,mm.Run	   ,onshow.run	  ,\
  en,'Encoding',mm.Encoding,onshow.recode ,\
  en,'Options' ,mm.Options ,onshow.options

popup_res mm.File,\
  ru,'Новый'	       ,'Ctrl+N'      ,New   ,\
  ru,'Открыть...'      ,'Ctrl+O'      ,Open  ,\
  ru,'Сохранить'       ,'Ctrl+S'      ,Save  ,\
  ru,'Сохранить как...','Ctrl+Shift+S',SaveAs,\
  ru,'-'	       ,''	      ,      ,\
  ru,'Закрыть'	       ,'Ctrl+F4'     ,Close ,\
  ru,'-'	       ,''	      ,      ,\
  ru,'Выход'	       ,'Alt+X'       ,Exit  ,\
\
  en,'New'	 ,'Ctrl+N'	,New   ,\
  en,'Open...'	 ,'Ctrl+O'	,Open  ,\
  en,'Save'	 ,'Ctrl+S'	,Save  ,\
  en,'Save as...','Ctrl+Shift+S',SaveAs,\
  en,'-'	 ,''		,      ,\
  en,'Close'	 ,'Ctrl+F4'	,Close ,\
  en,'-'	 ,''		,      ,\
  en,'Exit'	 ,'Alt+X'	,Exit

popup_res mm.Edit,\
  ru,'Вырезать'    ,'Ctrl+X',Cut   ,\
  ru,'Копировать'  ,'Ctrl+C',Copy  ,\
  ru,'Вставить'    ,'Ctrl+V',Insert,\
  ru,'Удалить'	   ,''	    ,Delete,\
  ru,'-'	   ,''	    ,	   ,\
  ru,'Выделить всё','Ctrl+A',SelAll,\
\
  en,'Cut'	 ,'Ctrl+X',Cut	 ,\
  en,'Copy'	 ,'Ctrl+C',Copy  ,\
  en,'Paste'	 ,'Ctrl+V',Insert,\
  en,'Delete'	 ,''	  ,Delete,\
  en,'-'	 ,''	  ,	 ,\
  en,'Select all','Ctrl+A',SelAll

popup_res mm.Search,\
  ru,'Перейти...' ,'Ctrl+G',Position,\
  ru,'-'	  ,''	   ,	    ,\
  ru,'Найти...'   ,'Ctrl+F',Find    ,\
  ru,'Найти далее','F3'    ,FindNext,\
  ru,'Заменить...','Ctrl+H',Replace ,\
\
  en,'Position...','Ctrl+G',Position,\
  en,'-'	  ,''	   ,	    ,\
  en,'Find...'	  ,'Ctrl+F',Find    ,\
  en,'Find next'  ,'F3'    ,FindNext,\
  en,'Replace...' ,'Ctrl+H',Replace

popup_res mm.Run,\
  ru,'Запустить'	,'F9'	  ,Run	   ,\
  ru,'Компилировать'	,'Ctrl+F9',Compile ,\
  ru,'-'		,''	  ,	   ,\
  ru,'Доска отладки'	,''	  ,DbgBoard,\
  ru,'Системные функции',''	  ,SysFuncs,\
\
  en,'Run'		,'F9'	  ,Run	   ,\
  en,'Compile'		,'Ctrl+F9',Compile ,\
  en,'-'		,''	  ,	   ,\
  en,'Debug board'	,''	  ,DbgBoard,\
  en,'System functions' ,''	  ,SysFuncs

popup_res mm.Encoding,\
  @!,<'CP866  ',0x1A,' CP1251'>,'',CP866.CP1251,\
  @!,<'CP1251 ',0x1A,' CP866' >,'',CP1251.CP866,\
  @!,<'-'		      >,'',	       ,\
  @!,<'CP866  ',0x1A,' KOI8-R'>,'',CP866.KOI8R ,\
  @!,<'KOI8-R ',0x1A,' CP866' >,'',KOI8R.CP866 ,\
  @!,<'-'		      >,'',	       ,\
  @!,<'CP1251 ',0x1A,' KOI8-R'>,'',CP1251.KOI8R,\
  @!,<'KOI8-R ',0x1A,' CP1251'>,'',KOI8R.CP1251

popup_res mm.Options,\
  ru,'Внешний вид...'	     ,'',Appearance  ,\
  ru,'-'		     ,'',	     ,\
  ru,'Безопасное выделение'  ,'',SecureSel   ,\
  ru,'Автоматические скобки' ,'',AutoBrackets,\
  ru,'Автоматический отступ' ,'',AutoIndents ,\
  ru,'Умная табуляция'	     ,'',SmartTabs   ,\
  ru,'Оптимальное сохранение','',OptimalFill ,\
  ru,'-'		     ,'',	     ,\
  ru,'Номера строк'	     ,'',LineNumbers ,\
\
  en,'Appearance...'	     ,'',Appearance  ,\
  en,'-'		     ,'',	     ,\
  en,'Secure selection'      ,'',SecureSel   ,\
  en,'Automatic brackets'    ,'',AutoBrackets,\
  en,'Automatic indents'     ,'',AutoIndents ,\
  en,'Smart tabulation'      ,'',SmartTabs   ,\
  en,'Optimal fill on saving','',OptimalFill ,\
  en,'-'		     ,'',	     ,\
  en,'Line numbers'	     ,'',LineNumbers

lsz s_modified,\
  ru,'Изменено',\
  en,'Modified'

lsz s_2filename,\
  ru,'Имя файла:',\
  en,'Filename:'
lsz s_2open,\
  ru,'Открыть',\
  en,'Open'
lsz s_2save,\
  ru,'Сохранить',\
  en,'Save'
lsz s_2find,\
  ru,'Найти',\
  en,'Find'
db ':'
lsz s_2replace,\
  ru,'Заменить',\
  en,'Replace'
db ':'
lsz s_2cancel,\
  ru,'Отмена',\
  en,'Cancel'

lsz s_enter_filename,\
  ru,<'Введите имя файла',0>,\
  en,<'Enter filename',0>

lsz s_enter_text_to_find,\
  ru,<'Введите текст для поиска',0>,\
  en,<'Enter text to find',0>

lsz s_enter_text_to_replace,\
  ru,<'Введите текст для замены',0>,\
  en,<'Enter text to replace',0>

lsz s_text_not_found,\
  ru,<'Достигнут конец файла, текст не найден',0>,\
  en,<'Reached end of file, text not found',0>

lszc s_fs_error,b,\
  ru,<'Операция завершена успешно (0)',0>,\
  ru,<'',0>,\
  ru,<'Функция не поддерживается для данной файловой системы (2)',0>,\
  ru,<'Неизвестная файловая система (3)',0>,\
  ru,<'',0>,\
  ru,<'Невозможно открыть файл (5)',0>,\
  ru,<'Операция завершена успешно (6)',0>,\
  ru,<'Адрес находится за границами памяти программы (7)',0>,\
  ru,<'На диске нет свободного места (8)',0>,\
  ru,<'Таблица FAT уничтожена (9)',0>,\
  ru,<'Доступ запрещён (10)',0>,\
  ru,<'Ошибка устройства (11)',0>,\
\
  en,<'Operation executed successfully (0)',0>,\
  en,<'',0>,\
  en,<'Function is not supported for the given filesystem (2)',0>,\
  en,<'Unknown filesystem (3)',0>,\
  en,<'',0>,\
  en,<'Unable to open file (5)',0>,\
  en,<'Operation executed successfully (6)',0>,\
  en,<'Pointer lies outside of application memory (7)',0>,\
  en,<'Disk is full (8)',0>,\
  en,<'FAT table is destroyed (9)',0>,\
  en,<'Access denied (10)',0>,\
  en,<'Device error (11)',0>

lsz sysfuncs_filename,\
  ru,<'SYSFUNCR.TXT',0>,\
  en,<'SYSFUNCS.TXT',0>

sz s_example,'EXAMPLE.ASM'
sz s_still  ,'still'
