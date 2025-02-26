//Copyright 2021 by Leency

#ifdef LANG_RUS
	#define DL_WINDOW_HEADER "Менеджер загрузок"
	#define T_DOWNLOAD "Скачать"
	#define T_CANCEL "Отмена"
	#define T_OPEN_DIR "Показать в папке"
	#define T_RUN "Открыть файл"
	#define T_NEW "Новая загрузка"
	#define T_SAVE_TO "Сохранить в:"
	#define T_AUTOCLOSE "Закрыть окно по завершении загрузки"

	#define T_STATUS_READY       "Готов к загрузке ^_^                                   "
	#define T_STATUS_DOWNLOADING "Идет загрузка файла... %i.%i MБ получено (%i KБ/с)   "
	#define T_STATUS_COMPLETE    "Загрузка успешно завершена.                            "
	#define T_STATUS_DL_P1 "Идет загрузка файла... "
	#define T_STATUS_DL_P2 " MБ получено ("
	#define T_STATUS_DL_P3 " KБ/с)  "

	#define FILE_SAVED_AS "'Менеджер загрузок\nФайл сохранен как %s' -Dt"
	#define FILE_NOT_SAVED "'Менеджер загрузок\nОшибка! Файл не может быть сохранен как\n%s' -Et"
	#define T_ERROR_STARTING_DOWNLOAD "'Невозможно начать скачивание.\nПроверьте введенный путь и соединение с Интернетом.' -E"
	char accept_language[]= "Accept-Language: ru\n";
#else
	#define DL_WINDOW_HEADER "Download Manager"
	#define T_DOWNLOAD "Download"
	#define T_CANCEL "Cancel"
	#define T_OPEN_DIR " Show in folder "
	#define T_RUN "Open file"
	#define T_NEW "New download"
	#define T_SAVE_TO "Download to:"
	#define T_AUTOCLOSE "Close this window when download completes"

	#define T_STATUS_READY       "Ready to download ^_^                          "
	#define T_STATUS_DOWNLOADING "Downloading... %i.%i MB received (%i KB/s)   "
	#define T_STATUS_COMPLETE    "Download completed succesfully.                "
	#define T_STATUS_DL_P1 "Downloading... "
	#define T_STATUS_DL_P2 " MB received ("
	#define T_STATUS_DL_P3 " KB/s)    "

	#define FILE_SAVED_AS "'Download manager\nFile saved as %s' -Dt"
	#define FILE_NOT_SAVED "'Download manager\nError! Can\96t save file as %s' -Et"
	#define T_ERROR_STARTING_DOWNLOAD "'Error while starting download process.\nCheck entered path and Internet connection.' -E"
	char accept_language[]= "Accept-Language: en\n";
#endif

#define WIN_W 526
#define WIN_H 195

#define GAPX 15
#define BUT_W 148

#define DEFAULT_SAVE_DIR "/tmp0/1/Downloads"

char dl_shared[] = "DL";

#define URL_SPEED_TEST "http://speedtest.tele2.net/100MB.zip"

enum { 
	BTN_EXIT=1,
	BTN_START,
	BTN_STOP,
	BTN_DIR, 
	BTN_RUN, 
	BTN_NEW
};

#define PB_COL_ERROR 0xF55353
#define PB_COL_PROGRESS 0x297FFD
#define PB_COL_COMPLETE 0x74DA00