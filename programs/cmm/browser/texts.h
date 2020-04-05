char version[]="WebView 2.25";

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
Менеджер загрузок
Обновить браузер";
char link_menu[] =
"Копировать ссылку
Скачать содержимое ссылки";
char loading_text[] = "Загрузка...";

char update_param[] = "-download_and_exit http://builds.kolibrios.org/rus/data/programs/cmm/browser/WebView.com";
char update_download_error[] = "'WebView\nОшибка при получении обновлений!' -tE";
char update_ok[] = "'WebView\nБраузер был успешно обновлен!' -tO";
char update_is_current[] = "'WebView\nВы уже используете последнюю версию.' -I";
char update_can_not_copy[] = "'WebView\nНе могу переместить новую версию из папки Downloads на Ramdisk. Возможно, не достаточно места.' -E";
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
Download Manager
Update browser";
char link_menu[] =
"Copy link
Download link contents";
char loading_text[] = "Loading...";
char update_param[] = "-download_and_exit http://builds.kolibrios.org/eng/data/programs/cmm/browser/WebView.com";
char update_download_error[] = "'WebView\nError receiving an up to date information!' -tE";
char update_ok[] = "'WebView\nThe browser has been updated!' -tO";
char update_is_current[] = "'WebView\nThe browser is up to date.' -I";
char update_can_not_copy[] = "'WebView\nError copying a new version from Downloads folder!\nProbably too litle space on Ramdisk.' -E";
#endif

#define URL_SERVICE_HISTORY "WebView:history"
#define URL_SERVICE_HOMEPAGE "WebView:home"
#define URL_SERVICE_HELP "WebView:help"

