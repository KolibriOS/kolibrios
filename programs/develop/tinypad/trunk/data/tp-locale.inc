
sz htext,'TINYPAD ',APP_VERSION

; Language support for locales: ru_RU (CP866), en_US, it_IT, et_EE, es_ES.

menubar_res main_menu,\
  ru_RU,'����'	,mm.File    ,onshow.file   ,\
  ru_RU,'�ࠢ��'	,mm.Edit    ,onshow.edit   ,\
  ru_RU,'����'	,mm.Search  ,onshow.search ,\
  ru_RU,'�����'	,mm.Run     ,onshow.run    ,\
  ru_RU,'����஢��',mm.Encoding,onshow.recode ,\
  ru_RU,'��樨'	,mm.Options ,onshow.options,\
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
  et_EE,'K�ivita'	,mm.Run     ,onshow.run    ,\
  et_EE,'Kodeering',mm.Encoding,onshow.recode ,\
  et_EE,'Seaded'	,mm.Options ,onshow.options,\
\
  es_ES,'Archivo'	,mm.File    ,onshow.file   ,\
  es_ES,'Editar'	,mm.Edit    ,onshow.edit   ,\
  es_ES,'Buscar'	,mm.Search  ,onshow.search ,\
  es_ES,'Ejecutar'	,mm.Run     ,onshow.run    ,\
  es_ES,'Codificaci�n',mm.Encoding,onshow.recode,\
  es_ES,'Opciones'	,mm.Options ,onshow.options

popup_res mm.File,\
  ru_RU,'����'	 ,'Ctrl+N'	,New	,\
  ru_RU,'������...','Ctrl+O'	,Open	,\
  ru_RU,'���࠭���' ,'Ctrl+S'	,Save	,\
  ru_RU,'���࠭��� ���...','Ctrl+Shift+S',SaveAs,\
  ru_RU,'-','',,\
  ru_RU,'�������'	 ,'Ctrl+F4'	,Close	,\
  ru_RU,'-','',,\
  ru_RU,'��室'	 ,'Alt+X'	,Exit	,\
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
  et_EE,'V�lju'	,'Alt+X'	,Exit	,\
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
  ru_RU,'��१���'		,'Ctrl+X',Cut	,\
  ru_RU,'����஢���'	,'Ctrl+C',Copy	,\
  ru_RU,'��⠢���'		,'Ctrl+V',Insert,\
  ru_RU,'�������'		,''	 ,Delete,\
  ru_RU,'-','',,\
  ru_RU,'�뤥���� ���'	,'Ctrl+A',SelAll,\
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
  et_EE,'L�ika'	,'Ctrl+X',Cut	,\
  et_EE,'Kopeeri'	,'Ctrl+C',Copy	,\
  et_EE,'Kleebi'	,'Ctrl+V',Insert,\
  et_EE,'Kustuta'	,''	 ,Delete,\
  et_EE,'-','',,\
  et_EE,'Vali k�ik','Ctrl+A',SelAll,\
\
  es_ES,'Cortar'	,'Ctrl+X',Cut	,\
  es_ES,'Copiar'	,'Ctrl+C',Copy	,\
  es_ES,'Pegar'	,'Ctrl+V',Insert,\
  es_ES,'Eliminar'	,''	 ,Delete,\
  es_ES,'-','',,\
  es_ES,'Seleccionar todo','Ctrl+A',SelAll

popup_res mm.Search,\
  ru_RU,'��३�...' ,'Ctrl+G',Position,\
  ru_RU,'-','',,\
  ru_RU,'����...'   ,'Ctrl+F',Find    ,\
  ru_RU,'���� �����','F3'    ,FindNext,\
  ru_RU,'��������...','Ctrl+H',Replace ,\
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
  et_EE,'Leia j�rgmine','F3'    ,FindNext,\
  et_EE,'Asenda...'    ,'Ctrl+H',Replace ,\
\
  es_ES,'Posici�n...'  ,'Ctrl+G',Position,\
  es_ES,'-','',,\
  es_ES,'Buscar...'    ,'Ctrl+F',Find    ,\
  es_ES,'Buscar siguiente','F3' ,FindNext,\
  es_ES,'Reemplazar...','Ctrl+H',Replace

popup_res mm.Run,\
  ru_RU,'��������'	,'F9'	  ,Run     ,\
  ru_RU,'�������� � ��������','F10',Debug   ,\
  ru_RU,'�������஢���'	,'Ctrl+F9',Compile ,\
  ru_RU,'-','',,\
  ru_RU,'��᪠ �⫠���'	,''	  ,DbgBoard,\
  ru_RU,'���⥬�� �㭪樨',''	  ,SysFuncs,\
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
  et_EE,'K�ivita'		,'F9'	  ,Run     ,\
  et_EE,'K�ivita aastal siluri','F10',Debug   ,\
  et_EE,'Kompileeri'	,'Ctrl+F9',Compile ,\
  et_EE,'-','',,\
  et_EE,'Silumis paneel'	,''	  ,DbgBoard,\
  et_EE,'S�steemi funktsioonid',''	  ,SysFuncs,\
\
  es_ES,'Ejecutar'		,'F9'	  ,Run     ,\
  es_ES,'Ejecutar en el depurador','F10',Debug,\
  es_ES,'Compilar'		,'Ctrl+F9',Compile ,\
  es_ES,'-','',,\
  es_ES,'Depuraci�n'	,''	  ,DbgBoard,\
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
  ru_RU,'��������'	  ,'Ctrl +',ZoomIn	,\
  ru_RU,'��������'	  ,'Ctrl -',ZoomOut	,\
  ru_RU,'-','',,\
  ru_RU,'����� ��ப'		,'',LineNumbers	,\
  ru_RU,'������᭮� �뤥�����'	,'',SecureSel	,\
  ru_RU,'��⮬���᪨� ᪮���'	,'',AutoBrackets,\
  ru_RU,'��⮬���᪨� �����'	,'',AutoIndents	,\
  ru_RU,'��⨬��쭮� ��࠭����'	,'',OptimalFill	,\
  ru_RU,'-','',,\
  ru_RU,'���譨� ���...'		,'',Appearance	,\
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
  et_EE,'Optimaalne t�itmine salvestamisel','',OptimalFill,\
  et_EE,'-','',,\
  et_EE,'V�limus...'		,'',Appearance	,\
\
  es_ES,'Zoom in'		  ,'Ctrl +',ZoomIn	,\
  es_ES,'Zoom out'		  ,'Ctrl -',ZoomOut	,\
  es_ES,'-','',,\
  es_ES,'N�meros en l�neas'	,'',LineNumbers	,\
  es_ES,'Selecci�n segura'		,'',SecureSel	,\
  es_ES,'Par�ntesis autom�ticos'	,'',AutoBrackets,\
  es_ES,'Identaci�n autom�tica'	,'',AutoIndents	,\
  es_ES,'Llenado �ptimo al guardar','',OptimalFill	,\
  es_ES,'-','',,\
  es_ES,'Apariencia...'		,'',Appearance

lsz s_defname,\
  ru_RU,<'����ﭭ�',0>,\
  en_US,<'Untitled',0>,\
  it_IT,<'Senza titolo',0>,\
  et_EE,<'Nimetu',0>,\
  es_ES,<'Sin t�tulo',0>

lsz s_modified,\
  ru_RU,'��������',\
  en_US,'Modified',\
  it_IT,'Modificato',\
  et_EE,'Muudetud',\
  es_ES,'Modificado'

lsz s_2filename,\
  ru_RU,'��� 䠩��:',\
  en_US,'Filename:',\
  en_US,'Nome del file:',\
  et_EE,'Faili nimi:',\
  es_ES,'Nombre de archivo:'

lsz s_2open,\
  ru_RU,'������',\
  en_US,'Open',\
  it_IT,'Aperto',\
  et_EE,'Ava',\
  es_ES,'Abrir'

lsz s_2save,\
  ru_RU,'���࠭���',\
  en_US,'Save',\
  it_IT,'Salva',\
  et_EE,'Salvesta',\
  es_ES,'Guardar'

lsz s_2save_no,\
  ru_RU,'���',\
  en_US,'No',\
  it_IT,'No',\
  et_EE,'Ei',\
  es_ES,'No'

lsz s_2find,\
  ru_RU,'����',\
  en_US,'Find',\
  it_IT,'Trova',\
  et_EE,'Leia',\
  es_ES,'Buscar'
db ':'

lsz s_2replace,\
  ru_RU,'��������',\
  en_US,'Replace',\
  it_IT,'Sostituire',\
  et_EE,'Asenda',\
  es_ES,'Reemplazar'
db ':'

lsz s_2cancel,\
  ru_RU,'�⬥��',\
  en_US,'Cancel',\
  it_IT,'Annulla',\
  et_EE,'T�hista',\
  es_ES,'Cancelar'

lsz s_color,\
  ru_RU,'����: 0x',\
  en_US,'Color: 0x',\
  it_IT,'Colore: 0x',\
  et_EE,'V�rv: 0x',\
  es_ES,'Color: 0x'

lsz s_tabpos,\
  ru_RU,'��ᯮ������� ��������',\
  en_US,'Tabs placement',\
  it_IT,'Posizionamento delle schede',\
  et_EE,'Saki asetus',\
  es_ES,'Lugar de tabulador'

lsz s_enter_filename,\
  ru_RU,<'������ ��� 䠩��',0>,\
  en_US,<'Enter filename',0>,\
  it_IT,<'Inserisci il nome del file',0>,\
  et_EE,<'Sisesta faili nimi',0>,\
  es_ES,<'Ingresa el nombre del archivo',0>

lsz s_ask_save,\
  ru_RU,<'������ ��࠭���?',0>,\
  en_US,<'Would you like to save?',0>,\
  it_IT,<'Vorresti salvare?',0>,\
  et_EE,<'Kas soovite salvestada?',0>,\
  es_ES,<'�Quieres guardar?',0>

lsz s_enter_text_to_find,\
  ru_RU,<'������ ⥪�� ��� ���᪠',0>,\
  en_US,<'Enter text to find',0>,\
  it_IT,<'Inserisci il testo per trovare',0>,\
  et_EE,<'Sisesta otsitav tekst',0>,\
  es_ES,<'Ingresa el texto a buscar',0>

lsz s_enter_text_to_replace,\
  ru_RU,<'������ ⥪�� ��� ������',0>,\
  en_US,<'Enter text to replace',0>,\
  it_IT,<'Inserisci il testo da sostituire',0>,\
  et_EE,<'Sisesta asendatav tekst',0>,\
  es_ES,<'Ingresa el texto a reemplazar',0>

lsz s_text_not_found,\
  ru_RU,<'���⨣��� ����� 䠩��, ⥪�� �� ������',0>,\
  en_US,<'Reached end of file, text not found',0>,\
  it_IT,<'Raggiunta la fine del file, testo non trovato',0>,\
  et_EE,<'Faili l�pp, teksti ei leitud',0>,\
  es_ES,<'Se lleg� al final del archivo, texto no encontrado',0>

lszc s_fs_error,b,\
  ru_RU,<'������ �����襭� �ᯥ譮 (0)',0>,\
  ru_RU,<'',0>,\
  ru_RU,<'�㭪�� �� �����ন������ ��� ������ 䠩����� ��⥬� (2)',0>,\
  ru_RU,<'�������⭠� 䠩����� ��⥬� (3)',0>,\
  ru_RU,<'',0>,\
  ru_RU,<'���������� ������ 䠩� (5)',0>,\
  ru_RU,<'������ �����襭� �ᯥ譮 (6)',0>,\
  ru_RU,<'���� ��室���� �� �࠭�栬� ����� �ணࠬ�� (7)',0>,\
  ru_RU,<'�� ��᪥ ��� ᢮������� ���� (8)',0>,\
  ru_RU,<'������ FAT 㭨�⮦��� (9)',0>,\
  ru_RU,<'����� ������ (10)',0>,\
  ru_RU,<'�訡�� ���ன�⢠ (11)',0>,\
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
  et_EE,<'Operatsioon k�ivitati edukalt (0)',0>,\
  et_EE,<'',0>,\
  et_EE,<'Failis�steem ei toeta funktsiooni (2)',0>,\
  et_EE,<'Tundmatu failis�steem (3)',0>,\
  et_EE,<'',0>,\
  et_EE,<'Faili ei suudetud avada (5)',0>,\
  et_EE,<'Operatsioon k�ivitatud edukalt (6)',0>,\
  et_EE,<'Viit on v�ljaspool programmi m�lu (7)',0>,\
  et_EE,<'Ketas t�is (8)',0>,\
  et_EE,<'FAT tabel h�vitatud (9)',0>,\
  et_EE,<'Juurdep��s keelatud (10)',0>,\
  et_EE,<'Seadme viga (11)',0>,\
\
  es_ES,<'Operaci�n ejecutada correctamente (0)',0>,\
  es_ES,<'',0>,\
  es_ES,<'La funci�n no est� soportada por el sistema de archivos dado (2)',0>,\
  es_ES,<'Sistema de archivos desconocido (3)',0>,\
  es_ES,<'',0>,\
  es_ES,<'No es posible abrir el archivo (5)',0>,\
  es_ES,<'Operaci�n ejecutada correctamente (6)',0>,\
  es_ES,<'El puntero est� fuera de la memoria de la aplicaci�n (7)',0>,\
  es_ES,<'El disco est� lleno (8)',0>,\
  es_ES,<'La tabla FAT est� destruida (9)',0>,\
  es_ES,<'Acceso denegado (10)',0>,\
  es_ES,<'Error de dispositivo (11)',0>

lszc s_appearance,b,\
  ru_RU,'�����',\
  ru_RU,'���',\
  ru_RU,'����� �뤥�����',\
  ru_RU,'��� �뤥�����',\
  ru_RU,'�������',\
  ru_RU,'��᫠',\
  ru_RU,'��ப�',\
  ru_RU,'�������ਨ',\
  ru_RU,'������� ��ப�',\
  ru_RU,'���࠭�� ��ப�',\
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
  et_EE,'S�mbolid',\
  et_EE,'Numbrid',\
  et_EE,'Stringid',\
  et_EE,'Kommentaarid',\
  et_EE,'Muudetud read',\
  et_EE,'Salvestatud read',\
\
  es_ES,'Texto',\
  es_ES,'Fondo',\
  es_ES,'Texto de selecci�n',\
  es_ES,'Fondo de selecci�n',\
  es_ES,'S�mbolos',\
  es_ES,'N�meros',\
  es_ES,'Cadenas',\
  es_ES,'Comentarios',\
  es_ES,'L�neas modificadas',\
  es_ES,'L�neas guardadas'

lsz sysfuncs_filename,\
  ru_RU,<'SYSFUNCR.TXT',0>,\
  en_US,<'SYSFUNCS.TXT',0>,\
  it_IT,<'SYSFUNCS.TXT',0>,\
  et_EE,<'SYSFUNCET.TXT',0>,\
  es_ES,<'SYSFUNCS.TXT',0>

sz s_example,'/sys/example.asm'
sz s_still  ,'still'
