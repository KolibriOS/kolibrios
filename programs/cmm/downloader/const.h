//Copyright 2020 by Leency

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

#ifdef LANG_RUS
	#define DL_WINDOW_HEADER "Менеджер загрузок"
	#define START_DOWNLOADING "Скачать"
	#define STOP_DOWNLOADING "Отмена"
	#define SHOW_IN_FOLDER "Показать в папке"
	#define OPEN_FILE_TEXT "Открыть файл"
	#define FILE_SAVED_AS "'Менеджер загрузок\nФайл сохранен как %s' -Dt"
	#define FILE_NOT_SAVED "'Менеджер загрузок\nОшибка! Файл не может быть сохранен как\n%s' -Et"
	#define KB_RECEIVED "Идет скачивание... %s получено"
	#define T_ERROR_STARTING_DOWNLOAD "'Невозможно начать скачивание.\nПожалуйста, проверьте введенный путь и соединение с Интернетом.' -E"
	#define T_AUTOCLOSE "Автозакрытие"
	char accept_language[]= "Accept-Language: ru\n";
#else
	#define DL_WINDOW_HEADER "Download Manager"
	#define START_DOWNLOADING "Download"
	#define STOP_DOWNLOADING "Cancel"
	#define SHOW_IN_FOLDER "Show in folder"
	#define OPEN_FILE_TEXT "Open file"
	#define FILE_SAVED_AS "'Download manager\nFile saved as %s' -Dt"
	#define FILE_NOT_SAVED "'Download manager\nError! Can\96t save file as %s' -Et"
	#define KB_RECEIVED "Downloading... %s received"
	#define T_ERROR_STARTING_DOWNLOAD "'Error while starting download process.\nPlease, check entered path and Internet connection.' -E"
	#define T_AUTOCLOSE "Autoclose"
	char accept_language[]= "Accept-Language: en\n";
#endif

#define GAPX 15
#define WIN_W 580
#define WIN_H 100

#define URL_SIZE 4000

char save_to[] = "/tmp0/1/Downloads";
char dl_shared[] = "DL";
