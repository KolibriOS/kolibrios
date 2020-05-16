char version[]="WebView 2.5b";

#ifdef LANG_RUS
char page_not_found[] = FROM "html\\page_not_found_ru.htm""\0";
char homepage[] = FROM "html\\homepage_ru.htm""\0";
char help[] = FROM "html\\help_ru.htm""\0";
char accept_language[]= "Accept-Language: ru\n";
char rmb_menu[] = 
"Посмотреть исходник|Ctrl+U
Редактировать исходник";
char main_menu[] = 
"Открыть файл|Ctrl+O
Новое окно|Ctrl+N
-
История|Ctrl+H
Менеджер загрузок|Ctrl+J
Очистить кэш
Обновить браузер";
char link_menu[] =
"Открыть в новой вкладке
Открыть в новом окне
-
Копировать ссылку
Скачать содержимое ссылки";
char loading_text[] = "Загрузка...";

char update_param[] = "-download_and_exit http://builds.kolibrios.org/rus/data/programs/cmm/browser/WebView.com";
char update_download_error[] = "'WebView\nОшибка при получении обновлений!' -tE";
char update_ok[] = "'WebView\nБраузер был успешно обновлен!' -tO";
char update_is_current[] = "'WebView\nВы уже используете последнюю версию.' -tI";
char update_can_not_copy[] = "'WebView\nНе могу переместить новую версию из папки Downloads на Ramdisk. Возможно, не достаточно места.' -tE";
char clear_cache_ok[] = "'WebView\nКэш очищен.' -tI";
#else
char page_not_found[] = FROM "html\\page_not_found_en.htm""\0";
char homepage[] = FROM "html\\homepage_en.htm""\0";
char help[] = FROM "html\\help_en.htm""\0";
char accept_language[]= "Accept-Language: en\n";
char rmb_menu[] =
"View source|Ctrl+U
Edit source";
char main_menu[] = 
"Open local file|Ctrl+O
New window|Ctrl+N
-
History|Ctrl+H
Download Manager|Ctrl+J
Clear cache
Update browser";
char link_menu[] =
"Open in new tab
Open in new window
-
Copy link
Download link contents";
char loading_text[] = "Loading...";
char update_param[] = "-download_and_exit http://builds.kolibrios.org/eng/data/programs/cmm/browser/WebView.com";
char update_download_error[] = "'WebView\nError receiving an up to date information!' -tE";
char update_ok[] = "'WebView\nThe browser has been updated!' -tO";
char update_is_current[] = "'WebView\nThe browser is up to date.' -tI";
char update_can_not_copy[] = "'WebView\nError copying a new version from Downloads folder!\nProbably too litle space on Ramdisk.' -tE";
char clear_cache_ok[] = "'WebView\nThe cache has been cleared.' -tI";
#endif

#define URL_SERVICE_HISTORY "WebView:history"
#define URL_SERVICE_HOMEPAGE "WebView:home"
#define URL_SERVICE_HELP "WebView:help"

char webview_shared[] = "WEBVIEW";
