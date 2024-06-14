
sz htext,'TINYPAD ',APP_VERSION

; Language support for locales: ru_RU (CP866), en_US, it_IT, et_EE, es_ES.

menubar_res main_menu,\
  ru_RU,'Файл'	,mm.File    ,onshow.file   ,\
  ru_RU,'Правка'	,mm.Edit    ,onshow.edit   ,\
  ru_RU,'Поиск'	,mm.Search  ,onshow.search ,\
  ru_RU,'Запуск'	,mm.Run     ,onshow.run    ,\
  ru_RU,'Кодировка',mm.Encoding,onshow.recode ,\
  ru_RU,'Опции'	,mm.Options ,onshow.options,\
\
  en_US,'File'	,mm.File    ,onshow.file   ,\
  en_US,'Edit'	,mm.Edit    ,onshow.edit   ,\
  en_US,'Search'	,mm.Search  ,onshow.search ,\
  en_US,'Run'	,mm.Run     ,onshow.run    ,\
  en_US,'Encoding'	,mm.Encoding,onshow.recode ,\
  en_US,'Options'	,mm.Options ,onshow.options,\
\
  it_IT,'File'	,mm.File    ,onshow.file   ,\
  it_IT,'Modificare'	,mm.Edit    ,onshow.edit   ,\
  it_IT,'Ricerca'	,mm.Search  ,onshow.search ,\
  it_IT,'Correre'	,mm.Run     ,onshow.run    ,\
  it_IT,'Codifica'	,mm.Encoding,onshow.recode ,\
  it_IT,'Opzioni'	,mm.Options ,onshow.options,\
\
  et_EE,'Fail'	,mm.File    ,onshow.file   ,\
  et_EE,'Muuda'	,mm.Edit    ,onshow.edit   ,\
  et_EE,'Otsi'	,mm.Search  ,onshow.search ,\
  et_EE,'Kфivita'	,mm.Run     ,onshow.run    ,\
  et_EE,'Kodeering',mm.Encoding,onshow.recode ,\
  et_EE,'Seaded'	,mm.Options ,onshow.options,\
\
  es_ES,'Archivo'	,mm.File    ,onshow.file   ,\
  es_ES,'Editar'	,mm.Edit    ,onshow.edit   ,\
  es_ES,'Buscar'	,mm.Search  ,onshow.search ,\
  es_ES,'Ejecutar'	,mm.Run     ,onshow.run    ,\
  es_ES,'Codificaciєn',mm.Encoding,onshow.recode,\
  es_ES,'Opciones'	,mm.Options ,onshow.options

popup_res mm.File,\
  ru_RU,'Новый'	 ,'Ctrl+N'	,New	,\
  ru_RU,'Открыть...','Ctrl+O'	,Open	,\
  ru_RU,'Сохранить' ,'Ctrl+S'	,Save	,\
  ru_RU,'Сохранить как...','Ctrl+Shift+S',SaveAs,\
  ru_RU,'-','',,\
  ru_RU,'Закрыть'	 ,'Ctrl+F4'	,Close	,\
  ru_RU,'-','',,\
  ru_RU,'Выход'	 ,'Alt+X'	,Exit	,\
\
  en_US,'New'	 ,'Ctrl+N'	,New	,\
  en_US,'Open...'	 ,'Ctrl+O'	,Open	,\
  en_US,'Save'	 ,'Ctrl+S'	,Save	,\
  en_US,'Save as...','Ctrl+Shift+S',SaveAs	,\
  en_US,'-','',,\
  en_US,'Close'	 ,'Ctrl+F4'	,Close	,\
  en_US,'-','',,\
  en_US,'Exit'	 ,'Alt+X'	,Exit	,\
\
  it_IT,'Nuovo'	 ,'Ctrl+N'	,New	,\
  it_IT,'Aperto...'	 ,'Ctrl+O'	,Open	,\
  it_IT,'Salva'	 ,'Ctrl+S'	,Save	,\
  it_IT,'Salva come...','Ctrl+Shift+S',SaveAs	,\
  it_IT,'-','',,\
  it_IT,'Chiudi'	 ,'Ctrl+F4'	,Close	,\
  it_IT,'-','',,\
  it_IT,'Uscita'	 ,'Alt+X'	,Exit	,\
\
  et_EE,'Uus'	,'Ctrl+N'	,New	,\
  et_EE,'Ava...'	,'Ctrl+O'	,Open	,\
  et_EE,'Salvesta'	,'Ctrl+S'	,Save	,\
  et_EE,'Salvesta nimega...','Ctrl+Shift+S',SaveAs,\
  et_EE,'-','',,\
  et_EE,'Sulge'	,'Ctrl+F4'	,Close	,\
  et_EE,'-','',,\
  et_EE,'Vфlju'	,'Alt+X'	,Exit	,\
\
  es_ES,'Nuevo'	,'Ctrl+N' ,New	,\
  es_ES,'Abrir...'	,'Ctrl+O' ,Open	,\
  es_ES,'Guardar'	,'Ctrl+S' ,Save	,\
  es_ES,'Guardar como...','Ctrl+Shift+S',SaveAs,\
  es_ES,'-','',,\
  es_ES,'Cerrar'	,'Ctrl+F4',Close,\
  es_ES,'-','',,\
  es_ES,'Salir'	,'Alt+X'  ,Exit

popup_res mm.Edit,\
  ru_RU,'Вырезать'		,'Ctrl+X',Cut	,\
  ru_RU,'Копировать'	,'Ctrl+C',Copy	,\
  ru_RU,'Вставить'		,'Ctrl+V',Insert,\
  ru_RU,'Удалить'		,''	 ,Delete,\
  ru_RU,'-','',,\
  ru_RU,'Выделить всё'	,'Ctrl+A',SelAll,\
\
  en_US,'Cut'	 ,'Ctrl+X',Cut   ,\
  en_US,'Copy'	 ,'Ctrl+C',Copy  ,\
  en_US,'Paste'	 ,'Ctrl+V',Insert,\
  en_US,'Delete'	 ,''	  ,Delete,\
  en_US,'-','',,\
  en_US,'Select all','Ctrl+A',SelAll,\
\
  it_IT,'Taglio'	 ,'Ctrl+X',Cut   ,\
  it_IT,'Copia'	 ,'Ctrl+C',Copy  ,\
  it_IT,'Incolla'	 ,'Ctrl+V',Insert,\
  it_IT,'Elimina'	 ,'' ,Delete,\
  it_IT,'-','',,\
  it_IT,'Seleziona tutto','Ctrl+A',SelAll,\
\
  et_EE,'Lїika'	,'Ctrl+X',Cut	,\
  et_EE,'Kopeeri'	,'Ctrl+C',Copy	,\
  et_EE,'Kleebi'	,'Ctrl+V',Insert,\
  et_EE,'Kustuta'	,''	 ,Delete,\
  et_EE,'-','',,\
  et_EE,'Vali kїik','Ctrl+A',SelAll,\
\
  es_ES,'Cortar'	,'Ctrl+X',Cut	,\
  es_ES,'Copiar'	,'Ctrl+C',Copy	,\
  es_ES,'Pegar'	,'Ctrl+V',Insert,\
  es_ES,'Eliminar'	,''	 ,Delete,\
  es_ES,'-','',,\
  es_ES,'Seleccionar todo','Ctrl+A',SelAll

popup_res mm.Search,\
  ru_RU,'Перейти...' ,'Ctrl+G',Position,\
  ru_RU,'-','',,\
  ru_RU,'Найти...'   ,'Ctrl+F',Find    ,\
  ru_RU,'Найти далее','F3'    ,FindNext,\
  ru_RU,'Заменить...','Ctrl+H',Replace ,\
\
  en_US,'Position...','Ctrl+G',Position,\
  en_US,'-','',,\
  en_US,'Find...'    ,'Ctrl+F',Find    ,\
  en_US,'Find next'  ,'F3'    ,FindNext,\
  en_US,'Replace...' ,'Ctrl+H',Replace ,\
\
  it_IT,'Posizione...','Ctrl+G',Position,\
  it_IT,'-','',,\
  it_IT,'Trova...'    ,'Ctrl+F',Find    ,\
  it_IT,'Trova il prossimo'  ,'F3'    ,FindNext,\
  it_IT,'Sostituire...' ,'Ctrl+H',Replace ,\
\
  et_EE,'Positsioon...','Ctrl+G',Position,\
  et_EE,'-','',,\
  et_EE,'Leia...'      ,'Ctrl+F',Find    ,\
  et_EE,'Leia jфrgmine','F3'    ,FindNext,\
  et_EE,'Asenda...'    ,'Ctrl+H',Replace ,\
\
  es_ES,'Posiciєn...'  ,'Ctrl+G',Position,\
  es_ES,'-','',,\
  es_ES,'Buscar...'    ,'Ctrl+F',Find    ,\
  es_ES,'Buscar siguiente','F3' ,FindNext,\
  es_ES,'Reemplazar...','Ctrl+H',Replace

popup_res mm.Run,\
  ru_RU,'Запустить'	,'F9'	  ,Run     ,\
  ru_RU,'Запустить в дебаггере','F10',Debug   ,\
  ru_RU,'Компилировать'	,'Ctrl+F9',Compile ,\
  ru_RU,'-','',,\
  ru_RU,'Доска отладки'	,''	  ,DbgBoard,\
  ru_RU,'Системные функции',''	  ,SysFuncs,\
\
  en_US,'Run'		,'F9'	  ,Run     ,\
  en_US,'Run in debugger'	,'F10'	  ,Debug   ,\
  en_US,'Compile'		,'Ctrl+F9',Compile ,\
  en_US,'-','',,\
  en_US,'Debug board'	,''	  ,DbgBoard,\
  en_US,'System functions'	,''	  ,SysFuncs,\
\
  it_IT,'Correre'		,'F9'	  ,Run     ,\
  it_IT,'Esegui nel debugger','F10',Debug   ,\
  it_IT,'Compilare','Ctrl+F9',Compile ,\
  it_IT,'-','',,\
  it_IT,'Scheda di debug'	,''	  ,DbgBoard,\
  it_IT,'Funzioni di sistema',''	  ,SysFuncs,\
\
  et_EE,'Kфivita'		,'F9'	  ,Run     ,\
  et_EE,'Kфivita aastal siluri','F10',Debug   ,\
  et_EE,'Kompileeri'	,'Ctrl+F9',Compile ,\
  et_EE,'-','',,\
  et_EE,'Silumis paneel'	,''	  ,DbgBoard,\
  et_EE,'S№steemi funktsioonid',''	  ,SysFuncs,\
\
  es_ES,'Ejecutar'		,'F9'	  ,Run     ,\
  es_ES,'Ejecutar en el depurador','F10',Debug,\
  es_ES,'Compilar'		,'Ctrl+F9',Compile ,\
  es_ES,'-','',,\
  es_ES,'Depuraciєn'	,''	  ,DbgBoard,\
  es_ES,'Funciones del sistema'   ,'',SysFuncs

popup_res mm.Encoding,\
  @!,<'CP866  ',0x1A,' CP1251'>,'',CP866.CP1251,\
  @!,<'CP1251 ',0x1A,' CP866' >,'',CP1251.CP866,\
  @!,<'-'>,'',,\
  @!,<'CP866  ',0x1A,' KOI8-R'>,'',CP866.KOI8R ,\
  @!,<'KOI8-R ',0x1A,' CP866' >,'',KOI8R.CP866 ,\
  @!,<'-'>,'',,\
  @!,<'CP1251 ',0x1A,' KOI8-R'>,'',CP1251.KOI8R,\
  @!,<'KOI8-R ',0x1A,' CP1251'>,'',KOI8R.CP1251

popup_res mm.Options,\
  ru_RU,'Увеличить'	  ,'Ctrl +',ZoomIn	,\
  ru_RU,'Уменьшить'	  ,'Ctrl -',ZoomOut	,\
  ru_RU,'-','',,\
  ru_RU,'Номера строк'		,'',LineNumbers	,\
  ru_RU,'Безопасное выделение'	,'',SecureSel	,\
  ru_RU,'Автоматические скобки'	,'',AutoBrackets,\
  ru_RU,'Автоматический отступ'	,'',AutoIndents	,\
  ru_RU,'Оптимальное сохранение'	,'',OptimalFill	,\
  ru_RU,'-','',,\
  ru_RU,'Внешний вид...'		,'',Appearance	,\
\
  en_US,'Zoom in'		  ,'Ctrl +',ZoomIn	,\
  en_US,'Zoom out'		  ,'Ctrl -',ZoomOut	,\
  en_US,'-','',,\
  en_US,'Line numbers'		,'',LineNumbers	,\
  en_US,'Secure selection'		,'',SecureSel	,\
  en_US,'Automatic brackets'	,'',AutoBrackets,\
  en_US,'Automatic indents'	,'',AutoIndents	,\
  en_US,'Optimal fill on saving'	,'',OptimalFill	,\
  en_US,'-','',,\
  en_US,'Appearance...'		,'',Appearance	,\
\
  it_IT,'Ingrandire'		  ,'Ctrl +',ZoomIn	,\
  it_IT,'Rimpicciolisci'		  ,'Ctrl -',ZoomOut	,\
  it_IT,'-','',,\
  it_IT,'Numeri di riga'		,'',LineNumbers	,\
  it_IT,'Selezione sicura'		,'',SecureSel	,\
  it_IT,'Staffe automatiche'	,'',AutoBrackets,\
  it_IT,'Rientri automatici'	,'',AutoIndents	,\
  it_IT,'Combina spazi'	,'',OptimalFill	,\
  it_IT,'-','',,\
  it_IT,'Aspetto...'		,'',Appearance	,\
\
  et_EE,'Zoom in'		  ,'Ctrl +',ZoomIn	,\
  et_EE,'Zoom out'		  ,'Ctrl -',ZoomOut	,\
  et_EE,'-','',,\
  et_EE,'Rea numbrid'		,'',LineNumbers	,\
  et_EE,'Turvaline valimine'	,'',SecureSel	,\
  et_EE,'Automaatsed sulud'	,'',AutoBrackets,\
  et_EE,'Automaatsed taandread'	,'',AutoIndents	,\
  et_EE,'Optimaalne tфitmine salvestamisel','',OptimalFill,\
  et_EE,'-','',,\
  et_EE,'Vфlimus...'		,'',Appearance	,\
\
  es_ES,'Zoom in'		  ,'Ctrl +',ZoomIn	,\
  es_ES,'Zoom out'		  ,'Ctrl -',ZoomOut	,\
  es_ES,'-','',,\
  es_ES,'N·meros en lэneas'	,'',LineNumbers	,\
  es_ES,'Selecciєn segura'		,'',SecureSel	,\
  es_ES,'Parщntesis automсticos'	,'',AutoBrackets,\
  es_ES,'Identaciєn automсtica'	,'',AutoIndents	,\
  es_ES,'Llenado єptimo al guardar','',OptimalFill	,\
  es_ES,'-','',,\
  es_ES,'Apariencia...'		,'',Appearance

lsz s_defname,\
  ru_RU,<'Безымянный',0>,\
  en_US,<'Untitled',0>,\
  it_IT,<'Senza titolo',0>,\
  et_EE,<'Nimetu',0>,\
  es_ES,<'Sin tэtulo',0>

lsz s_modified,\
  ru_RU,'Изменено',\
  en_US,'Modified',\
  it_IT,'Modificato',\
  et_EE,'Muudetud',\
  es_ES,'Modificado'

lsz s_2filename,\
  ru_RU,'Имя файла:',\
  en_US,'Filename:',\
  en_US,'Nome del file:',\
  et_EE,'Faili nimi:',\
  es_ES,'Nombre de archivo:'

lsz s_2open,\
  ru_RU,'Открыть',\
  en_US,'Open',\
  it_IT,'Aperto',\
  et_EE,'Ava',\
  es_ES,'Abrir'

lsz s_2save,\
  ru_RU,'Сохранить',\
  en_US,'Save',\
  it_IT,'Salva',\
  et_EE,'Salvesta',\
  es_ES,'Guardar'

lsz s_2save_no,\
  ru_RU,'Нет',\
  en_US,'No',\
  it_IT,'No',\
  et_EE,'Ei',\
  es_ES,'No'

lsz s_2find,\
  ru_RU,'Найти',\
  en_US,'Find',\
  it_IT,'Trova',\
  et_EE,'Leia',\
  es_ES,'Buscar'
db ':'

lsz s_2replace,\
  ru_RU,'Заменить',\
  en_US,'Replace',\
  it_IT,'Sostituire',\
  et_EE,'Asenda',\
  es_ES,'Reemplazar'
db ':'

lsz s_2cancel,\
  ru_RU,'Отмена',\
  en_US,'Cancel',\
  it_IT,'Annulla',\
  et_EE,'T№hista',\
  es_ES,'Cancelar'

lsz s_color,\
  ru_RU,'Цвет: 0x',\
  en_US,'Color: 0x',\
  it_IT,'Colore: 0x',\
  et_EE,'Vфrv: 0x',\
  es_ES,'Color: 0x'

lsz s_tabpos,\
  ru_RU,'Расположение закладок',\
  en_US,'Tabs placement',\
  it_IT,'Posizionamento delle schede',\
  et_EE,'Saki asetus',\
  es_ES,'Lugar de tabulador'

lsz s_enter_filename,\
  ru_RU,<'Введите имя файла',0>,\
  en_US,<'Enter filename',0>,\
  it_IT,<'Inserisci il nome del file',0>,\
  et_EE,<'Sisesta faili nimi',0>,\
  es_ES,<'Ingresa el nombre del archivo',0>

lsz s_ask_save,\
  ru_RU,<'Желаете сохранить?',0>,\
  en_US,<'Would you like to save?',0>,\
  it_IT,<'Vorresti salvare?',0>,\
  et_EE,<'Kas soovite salvestada?',0>,\
  es_ES,<'┐Quieres guardar?',0>

lsz s_enter_text_to_find,\
  ru_RU,<'Введите текст для поиска',0>,\
  en_US,<'Enter text to find',0>,\
  it_IT,<'Inserisci il testo per trovare',0>,\
  et_EE,<'Sisesta otsitav tekst',0>,\
  es_ES,<'Ingresa el texto a buscar',0>

lsz s_enter_text_to_replace,\
  ru_RU,<'Введите текст для замены',0>,\
  en_US,<'Enter text to replace',0>,\
  it_IT,<'Inserisci il testo da sostituire',0>,\
  et_EE,<'Sisesta asendatav tekst',0>,\
  es_ES,<'Ingresa el texto a reemplazar',0>

lsz s_text_not_found,\
  ru_RU,<'Достигнут конец файла, текст не найден',0>,\
  en_US,<'Reached end of file, text not found',0>,\
  it_IT,<'Raggiunta la fine del file, testo non trovato',0>,\
  et_EE,<'Faili lїpp, teksti ei leitud',0>,\
  es_ES,<'Se llegє al final del archivo, texto no encontrado',0>

lszc s_fs_error,b,\
  ru_RU,<'Операция завершена успешно (0)',0>,\
  ru_RU,<'',0>,\
  ru_RU,<'Функция не поддерживается для данной файловой системы (2)',0>,\
  ru_RU,<'Неизвестная файловая система (3)',0>,\
  ru_RU,<'',0>,\
  ru_RU,<'Невозможно открыть файл (5)',0>,\
  ru_RU,<'Операция завершена успешно (6)',0>,\
  ru_RU,<'Адрес находится за границами памяти программы (7)',0>,\
  ru_RU,<'На диске нет свободного места (8)',0>,\
  ru_RU,<'Таблица FAT уничтожена (9)',0>,\
  ru_RU,<'Доступ запрещён (10)',0>,\
  ru_RU,<'Ошибка устройства (11)',0>,\
\
  en_US,<'Operation executed successfully (0)',0>,\
  en_US,<'',0>,\
  en_US,<'Function is not supported for the given filesystem (2)',0>,\
  en_US,<'Unknown filesystem (3)',0>,\
  en_US,<'',0>,\
  en_US,<'Unable to open file (5)',0>,\
  en_US,<'Operation executed successfully (6)',0>,\
  en_US,<'Pointer lies outside of application memory (7)',0>,\
  en_US,<'Disk is full (8)',0>,\
  en_US,<'FAT table is destroyed (9)',0>,\
  en_US,<'Access denied (10)',0>,\
  en_US,<'Device error (11)',0>,\
\
  it_IT,<'Operazione eseguita correttamente (0)',0>,\
  it_IT,<'',0>,\
  it_IT,<'La funzione non e supportata per il file system specificato (2)', 0>, \
  it_IT,<'Unknown filesystem (3)',0>,\
  it_IT,<'',0>,\
  it_IT,<'Impossibile aprire il file (5)',0>,\
  it_IT,<'Operazione eseguita correttamente (6)',0>,\
  it_IT,<'Il puntatore si trova al di fuori della memoria dell',96,'applicazione (7)', 0>, \
  it_IT,<'Il disco e pieno (8)',0>,\
  it_IT,<'La tabella FAT e distrutta (9)',0>,\
  it_IT,<'Accesso negato (10)',0>,\
  it_IT,<'Errore dispositivo (11)',0>,\
\
  et_EE,<'Operatsioon kфivitati edukalt (0)',0>,\
  et_EE,<'',0>,\
  et_EE,<'Failis№steem ei toeta funktsiooni (2)',0>,\
  et_EE,<'Tundmatu failis№steem (3)',0>,\
  et_EE,<'',0>,\
  et_EE,<'Faili ei suudetud avada (5)',0>,\
  et_EE,<'Operatsioon kфivitatud edukalt (6)',0>,\
  et_EE,<'Viit on vфljaspool programmi mфlu (7)',0>,\
  et_EE,<'Ketas tфis (8)',0>,\
  et_EE,<'FAT tabel hфvitatud (9)',0>,\
  et_EE,<'Juurdepффs keelatud (10)',0>,\
  et_EE,<'Seadme viga (11)',0>,\
\
  es_ES,<'Operaciєn ejecutada correctamente (0)',0>,\
  es_ES,<'',0>,\
  es_ES,<'La funciєn no estс soportada por el sistema de archivos dado (2)',0>,\
  es_ES,<'Sistema de archivos desconocido (3)',0>,\
  es_ES,<'',0>,\
  es_ES,<'No es posible abrir el archivo (5)',0>,\
  es_ES,<'Operaciєn ejecutada correctamente (6)',0>,\
  es_ES,<'El puntero estс fuera de la memoria de la aplicaciєn (7)',0>,\
  es_ES,<'El disco estс lleno (8)',0>,\
  es_ES,<'La tabla FAT estс destruida (9)',0>,\
  es_ES,<'Acceso denegado (10)',0>,\
  es_ES,<'Error de dispositivo (11)',0>

lszc s_appearance,b,\
  ru_RU,'Текст',\
  ru_RU,'Фон',\
  ru_RU,'Текст выделения',\
  ru_RU,'Фон выделения',\
  ru_RU,'Символы',\
  ru_RU,'Числа',\
  ru_RU,'Строки',\
  ru_RU,'Комментарии',\
  ru_RU,'Изменёные строки',\
  ru_RU,'Сохранённые строки',\
\
  en_US,'Text',\
  en_US,'Background',\
  en_US,'Selection text',\
  en_US,'Selection background',\
  en_US,'Symbols',\
  en_US,'Numbers',\
  en_US,'Strings',\
  en_US,'Comments',\
  en_US,'Modified lines',\
  en_US,'Saved lines',\
\
  it_IT,'Testo',\
  it_IT,'Sfondo',\
  it_IT,'Testo di selezione',\
  it_IT,'Sfondo di selezione',\
  it_IT,'Simboli',\
  it_IT,'Numeri',\
  it_IT,'Stringhe',\
  it_IT,'Commenti',\
  it_IT,'Linee modificate',\
  it_IT,'Linee salvate',\
\
  et_EE,'Tekst',\
  et_EE,'Taust',\
  et_EE,'Valitud tekst',\
  et_EE,'Valitud taust',\
  et_EE,'S№mbolid',\
  et_EE,'Numbrid',\
  et_EE,'Stringid',\
  et_EE,'Kommentaarid',\
  et_EE,'Muudetud read',\
  et_EE,'Salvestatud read',\
\
  es_ES,'Texto',\
  es_ES,'Fondo',\
  es_ES,'Texto de selecciєn',\
  es_ES,'Fondo de selecciєn',\
  es_ES,'Sэmbolos',\
  es_ES,'N·meros',\
  es_ES,'Cadenas',\
  es_ES,'Comentarios',\
  es_ES,'Lэneas modificadas',\
  es_ES,'Lэneas guardadas'

lsz sysfuncs_filename,\
  ru_RU,<'SYSFUNCR.TXT',0>,\
  en_US,<'SYSFUNCS.TXT',0>,\
  it_IT,<'SYSFUNCS.TXT',0>,\
  et_EE,<'SYSFUNCET.TXT',0>,\
  es_ES,<'SYSFUNCS.TXT',0>

sz s_example,'/sys/example.asm'
sz s_still  ,'still'
