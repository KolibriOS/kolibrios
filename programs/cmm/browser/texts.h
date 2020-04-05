char version[]="WebView 2.2";

#ifdef LANG_RUS
char page_not_found[] = FROM "html\\page_not_found_ru.htm""\0";
char homepage[] = FROM "html\\homepage_ru.htm""\0";
char help[] = FROM "html\\help_ru.htm""\0";
char accept_language[]= "Accept-Language: ru\n";
char rmb_menu[] = 
"Посмотреть исходник
Редактировать исходник";
char main_menu[] = 
"Открыть файл
Новое окно
История
Менеджер загрузок";
char link_menu[] =
"Копировать ссылку
Скачать содержимое ссылки";
char loading_text[] = "Загрузка...";
#else
char page_not_found[] = FROM "html\\page_not_found_en.htm""\0";
char homepage[] = FROM "html\\homepage_en.htm""\0";
char help[] = FROM "html\\help_en.htm""\0";
char accept_language[]= "Accept-Language: en\n";
char rmb_menu[] =
"View source
Edit source";
char main_menu[] = 
"Open local file
New window
History
Download Manager";
char link_menu[] =
"Copy link
Download link contents";
char loading_text[] = "Loading...";
#endif

#define URL_SERVICE_HISTORY "WebView:history"
#define URL_SERVICE_HOMEPAGE "WebView:home"
#define URL_SERVICE_HELP "WebView:help"

