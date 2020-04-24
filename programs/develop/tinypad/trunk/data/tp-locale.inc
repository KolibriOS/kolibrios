
sz htext,'TINYPAD ',APP_VERSION

menubar_res main_menu,\
  ru,'Файл'	,mm.File    ,onshow.file   ,\
  ru,'Правка'	,mm.Edit    ,onshow.edit   ,\
  ru,'Поиск'	,mm.Search  ,onshow.search ,\
  ru,'Запуск'	,mm.Run     ,onshow.run    ,\
  ru,'Кодировка',mm.Encoding,onshow.recode ,\
  ru,'Опции'	,mm.Options ,onshow.options,\
\
  en,'File'	,mm.File    ,onshow.file   ,\
  en,'Edit'	,mm.Edit    ,onshow.edit   ,\
  en,'Search'	,mm.Search  ,onshow.search ,\
  en,'Run'	,mm.Run     ,onshow.run    ,\
  en,'Encoding'	,mm.Encoding,onshow.recode ,\
  en,'Options'	,mm.Options ,onshow.options,\
\
  it,'Fail'	,mm.File    ,onshow.file   ,\
  it,'Muuda'	,mm.Edit    ,onshow.edit   ,\
  it,'Otsi'	,mm.Search  ,onshow.search ,\
  it,'Kфivita'	,mm.Run     ,onshow.run    ,\
  it,'Kodeering',mm.Encoding,onshow.recode ,\
  it,'Seaded'	,mm.Options ,onshow.options,\
\
  sp,'Archivo'	,mm.File    ,onshow.file   ,\
  sp,'Editar'	,mm.Edit    ,onshow.edit   ,\
  sp,'Buscar'	,mm.Search  ,onshow.search ,\
  sp,'Ejecutar'	,mm.Run     ,onshow.run    ,\
  sp,'Codificaciєn',mm.Encoding,onshow.recode,\
  sp,'Opciones'	,mm.Options ,onshow.options

popup_res mm.File,\
  ru,'Новый'	 ,'Ctrl+N'	,New	,\
  ru,'Открыть...','Ctrl+O'	,Open	,\
  ru,'Сохранить' ,'Ctrl+S'	,Save	,\
  ru,'Сохранить как...','Ctrl+Shift+S',SaveAs,\
  ru,'-','',,\
  ru,'Закрыть'	 ,'Ctrl+F4'	,Close	,\
  ru,'-','',,\
  ru,'Выход'	 ,'Alt+X'	,Exit	,\
\
  en,'New'	 ,'Ctrl+N'	,New	,\
  en,'Open...'	 ,'Ctrl+O'	,Open	,\
  en,'Save'	 ,'Ctrl+S'	,Save	,\
  en,'Save as...','Ctrl+Shift+S',SaveAs	,\
  en,'-','',,\
  en,'Close'	 ,'Ctrl+F4'	,Close	,\
  en,'-','',,\
  en,'Exit'	 ,'Alt+X'	,Exit	,\
\
  it,'Uus'	,'Ctrl+N'	,New	,\
  it,'Ava...'	,'Ctrl+O'	,Open	,\
  it,'Salvesta'	,'Ctrl+S'	,Save	,\
  it,'Salvesta nimega...','Ctrl+Shift+S',SaveAs,\
  it,'-','',,\
  it,'Sulge'	,'Ctrl+F4'	,Close	,\
  it,'-','',,\
  it,'Vфlju'	,'Alt+X'	,Exit	,\
\
  sp,'Nuevo'	,'Ctrl+N' ,New	,\
  sp,'Abrir...'	,'Ctrl+O' ,Open	,\
  sp,'Guardar'	,'Ctrl+S' ,Save	,\
  sp,'Guardar como...','Ctrl+Shift+S',SaveAs,\
  sp,'-','',,\
  sp,'Cerrar'	,'Ctrl+F4',Close,\
  sp,'-','',,\
  sp,'Salir'	,'Alt+X'  ,Exit

popup_res mm.Edit,\
  ru,'Вырезать'		,'Ctrl+X',Cut	,\
  ru,'Копировать'	,'Ctrl+C',Copy	,\
  ru,'Вставить'		,'Ctrl+V',Insert,\
  ru,'Удалить'		,''	 ,Delete,\
  ru,'-','',,\
  ru,'Выделить всё'	,'Ctrl+A',SelAll,\
\
  en,'Cut'	 ,'Ctrl+X',Cut   ,\
  en,'Copy'	 ,'Ctrl+C',Copy  ,\
  en,'Paste'	 ,'Ctrl+V',Insert,\
  en,'Delete'	 ,''	  ,Delete,\
  en,'-','',,\
  en,'Select all','Ctrl+A',SelAll,\
\
  it,'Lїika'	,'Ctrl+X',Cut	,\
  it,'Kopeeri'	,'Ctrl+C',Copy	,\
  it,'Kleebi'	,'Ctrl+V',Insert,\
  it,'Kustuta'	,''	 ,Delete,\
  it,'-','',,\
  it,'Vali kїik','Ctrl+A',SelAll,\
\
  sp,'Cortar'	,'Ctrl+X',Cut	,\
  sp,'Copiar'	,'Ctrl+C',Copy	,\
  sp,'Pegar'	,'Ctrl+V',Insert,\
  sp,'Eliminar'	,''	 ,Delete,\
  sp,'-','',,\
  sp,'Seleccionar todo','Ctrl+A',SelAll

popup_res mm.Search,\
  ru,'Перейти...' ,'Ctrl+G',Position,\
  ru,'-','',,\
  ru,'Найти...'   ,'Ctrl+F',Find    ,\
  ru,'Найти далее','F3'    ,FindNext,\
  ru,'Заменить...','Ctrl+H',Replace ,\
\
  en,'Position...','Ctrl+G',Position,\
  en,'-','',,\
  en,'Find...'    ,'Ctrl+F',Find    ,\
  en,'Find next'  ,'F3'    ,FindNext,\
  en,'Replace...' ,'Ctrl+H',Replace ,\
\
  it,'Positsioon...','Ctrl+G',Position,\
  it,'-','',,\
  it,'Leia...'      ,'Ctrl+F',Find    ,\
  it,'Leia jфrgmine','F3'    ,FindNext,\
  it,'Asenda...'    ,'Ctrl+H',Replace ,\
\
  sp,'Posiciєn...'  ,'Ctrl+G',Position,\
  sp,'-','',,\
  sp,'Buscar...'    ,'Ctrl+F',Find    ,\
  sp,'Buscar siguiente','F3' ,FindNext,\
  sp,'Reemplazar...','Ctrl+H',Replace

popup_res mm.Run,\
  ru,'Запустить'	,'F9'	  ,Run     ,\
  ru,'Запустить в дебаггере','F10',Debug   ,\
  ru,'Компилировать'	,'Ctrl+F9',Compile ,\
  ru,'-','',,\
  ru,'Доска отладки'	,''	  ,DbgBoard,\
  ru,'Системные функции',''	  ,SysFuncs,\
\
  en,'Run'		,'F9'	  ,Run     ,\
  en,'Run in debugger'	,'F10'	  ,Debug   ,\
  en,'Compile'		,'Ctrl+F9',Compile ,\
  en,'-','',,\
  en,'Debug board'	,''	  ,DbgBoard,\
  en,'System functions'	,''	  ,SysFuncs,\
\
  it,'Kфivita'		,'F9'	  ,Run     ,\
  it,'Kфivita aastal siluri','F10',Debug   ,\
  it,'Kompileeri'	,'Ctrl+F9',Compile ,\
  it,'-','',,\
  it,'Silumis paneel'	,''	  ,DbgBoard,\
  it,'S№steemi funktsioonid',''	  ,SysFuncs,\
\
  sp,'Ejecutar'		,'F9'	  ,Run     ,\
  sp,'Ejecutar en el depurador','F10',Debug,\
  sp,'Compilar'		,'Ctrl+F9',Compile ,\
  sp,'-','',,\
  sp,'Depuraciєn'	,''	  ,DbgBoard,\
  sp,'Funciones del sistema'   ,'',SysFuncs

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
  ru,'Увеличить'	  ,'Ctrl +',ZoomIn	,\
  ru,'Уменьшить'	  ,'Ctrl -',ZoomOut	,\
  ru,'-','',,\
  ru,'Номера строк'		,'',LineNumbers	,\
  ru,'Безопасное выделение'	,'',SecureSel	,\
  ru,'Автоматические скобки'	,'',AutoBrackets,\
  ru,'Автоматический отступ'	,'',AutoIndents	,\
  ru,'Оптимальное сохранение'	,'',OptimalFill	,\
  ru,'-','',,\
  ru,'Внешний вид...'		,'',Appearance	,\
\
  en,'Zoom in'		  ,'Ctrl +',ZoomIn	,\
  en,'Zoom out'		  ,'Ctrl -',ZoomOut	,\
  en,'-','',,\
  en,'Line numbers'		,'',LineNumbers	,\
  en,'Secure selection'		,'',SecureSel	,\
  en,'Automatic brackets'	,'',AutoBrackets,\
  en,'Automatic indents'	,'',AutoIndents	,\
  en,'Optimal fill on saving'	,'',OptimalFill	,\
  en,'-','',,\
  en,'Appearance...'		,'',Appearance	,\
\
  it,'Zoom in'		  ,'Ctrl +',ZoomIn	,\
  it,'Zoom out'		  ,'Ctrl -',ZoomOut	,\
  it,'-','',,\
  it,'Rea numbrid'		,'',LineNumbers	,\
  it,'Turvaline valimine'	,'',SecureSel	,\
  it,'Automaatsed sulud'	,'',AutoBrackets,\
  it,'Automaatsed taandread'	,'',AutoIndents	,\
  it,'Optimaalne tфitmine salvestamisel','',OptimalFill,\
  it,'-','',,\
  it,'Vфlimus...'		,'',Appearance	,\
\
  sp,'Zoom in'		  ,'Ctrl +',ZoomIn	,\
  sp,'Zoom out'		  ,'Ctrl -',ZoomOut	,\
  sp,'-','',,\
  sp,'N·meros en lэneas'	,'',LineNumbers	,\
  sp,'Selecciєn segura'		,'',SecureSel	,\
  sp,'Parщntesis automсticos'	,'',AutoBrackets,\
  sp,'Identaciєn automсtica'	,'',AutoIndents	,\
  sp,'Llenado єptimo al guardar','',OptimalFill	,\
  sp,'-','',,\
  sp,'Apariencia...'		,'',Appearance

lsz s_defname,\
  en,<'Untitled',0>,\
  ru,<'Безымянный',0>,\
  it,<'Nimetu',0>,\
  sp,<'Sin tэtulo',0>

lsz s_modified,\
  ru,'Изменено',\
  en,'Modified',\
  it,'Muudetud',\
  sp,'Modificado'

lsz s_2filename,\
  ru,'Имя файла:',\
  en,'Filename:',\
  it,'Faili nimi:',\
  sp,'Nombre de archivo:'
lsz s_2open,\
  ru,'Открыть',\
  en,'Open',\
  it,'Ava',\
  sp,'Abrir'
lsz s_2save,\
  ru,'Сохранить',\
  en,'Save',\
  it,'Salvesta',\
  sp,'Guardar'
lsz s_2save_no,\
  ru,'Нет',\
  en,'No',\
  it,'Ei',\
  sp,'No'
lsz s_2find,\
  ru,'Найти',\
  en,'Find',\
  it,'Leia',\
  sp,'Buscar'
db ':'
lsz s_2replace,\
  ru,'Заменить',\
  en,'Replace',\
  it,'Asenda',\
  sp,'Reemplazar'
db ':'
lsz s_2cancel,\
  ru,'Отмена',\
  en,'Cancel',\
  it,'T№hista',\
  sp,'Cancelar'
lsz s_color,\
  ru,'Цвет: 0x',\
  en,'Color: 0x',\
  it,'Vфrv: 0x',\
  sp,'Color: 0x'
lsz s_tabpos,\
  ru,'Расположение закладок',\
  en,'Tabs placement',\
  it,'Saki asetus',\
  sp,'Lugar de tabulador'

lsz s_enter_filename,\
  ru,<'Введите имя файла',0>,\
  en,<'Enter filename',0>,\
  it,<'Sisesta faili nimi',0>,\
  sp,<'Ingresa el nombre del archivo',0>

lsz s_ask_save,\
  ru,<'Желаете сохранить?',0>,\
  en,<'Would you like to save?',0>,\
  it,<'Kas soovite salvestada?',0>,\	; Now correct
  sp,<'┐Quieres guardar?',0>

lsz s_enter_text_to_find,\
  ru,<'Введите текст для поиска',0>,\
  en,<'Enter text to find',0>,\
  it,<'Sisesta otsitav tekst',0>,\
  sp,<'Ingresa el texto a buscar',0>

lsz s_enter_text_to_replace,\
  ru,<'Введите текст для замены',0>,\
  en,<'Enter text to replace',0>,\
  it,<'Sisesta asendatav tekst',0>,\
  sp,<'Ingresa el texto a reemplazar',0>

lsz s_text_not_found,\
  ru,<'Достигнут конец файла, текст не найден',0>,\
  en,<'Reached end of file, text not found',0>,\
  it,<'Faili lїpp, teksti ei leitud',0>,\
  sp,<'Se llegє al final del archivo, texto no encontrado',0>

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
  en,<'Device error (11)',0>,\
\
  it,<'Operatsioon kфivitati edukalt (0)',0>,\
  it,<'',0>,\
  it,<'Failis№steem ei toeta funktsiooni (2)',0>,\
  it,<'Tundmatu failis№steem (3)',0>,\
  it,<'',0>,\
  it,<'Faili ei suudetud avada (5)',0>,\
  it,<'Operatsioon kфivitatud edukalt (6)',0>,\
  it,<'Viit on vфljaspool programmi mфlu (7)',0>,\
  it,<'Ketas tфis (8)',0>,\
  it,<'FAT tabel hфvitatud (9)',0>,\
  it,<'Juurdepффs keelatud (10)',0>,\
  it,<'Seadme viga (11)',0>,\
\
  sp,<'Operaciєn ejecutada correctamente (0)',0>,\
  sp,<'',0>,\
  sp,<'La funciєn no estс soportada por el sistema de archivos dado (2)',0>,\
  sp,<'Sistema de archivos desconocido (3)',0>,\
  sp,<'',0>,\
  sp,<'No es posible abrir el archivo (5)',0>,\
  sp,<'Operaciєn ejecutada correctamente (6)',0>,\
  sp,<'El puntero estс fuera de la memoria de la aplicaciєn (7)',0>,\
  sp,<'El disco estс lleno (8)',0>,\
  sp,<'La tabla FAT estс destruida (9)',0>,\
  sp,<'Acceso denegado (10)',0>,\
  sp,<'Error de dispositivo (11)',0>

lszc s_appearance,b,\
  ru,'Текст',\
  ru,'Фон',\
  ru,'Текст выделения',\
  ru,'Фон выделения',\
  ru,'Символы',\
  ru,'Числа',\
  ru,'Строки',\
  ru,'Комментарии',\
  ru,'Изменёные строки',\
  ru,'Сохранённые строки',\
\
  en,'Text',\
  en,'Background',\
  en,'Selection text',\
  en,'Selection background',\
  en,'Symbols',\
  en,'Numbers',\
  en,'Strings',\
  en,'Comments',\
  en,'Modified lines',\
  en,'Saved lines',\
\
  it,'Tekst',\
  it,'Taust',\
  it,'Valitud tekst',\
  it,'Valitud taust',\
  it,'S№mbolid',\
  it,'Numbrid',\
  it,'Stringid',\
  it,'Kommentaarid',\
  it,'Muudetud read',\
  it,'Salvestatud read',\
\
  sp,'Texto',\
  sp,'Fondo',\
  sp,'Texto de selecciєn',\
  sp,'Fondo de selecciєn',\
  sp,'Sэmbolos',\
  sp,'N·meros',\
  sp,'Cadenas',\
  sp,'Comentarios',\
  sp,'Lэneas modificadas',\
  sp,'Lэneas guardadas'

lsz sysfuncs_filename,\
  ru,<'SYSFUNCR.TXT',0>,\
  en,<'SYSFUNCS.TXT',0>,\
  it,<'SYSFUNCET.TXT',0>,\
  sp,<'SYSFUNCS.TXT',0>

sz s_example,'/sys/example.asm'
sz s_still  ,'still'
