#ifdef LANG_RUS
//===================================================//
//                                                   //
//                     CYRILLIC                      //
//                                                   //
//===================================================//
char buildin_page_error[] = FROM "res/page_not_found_ru.htm";
char buildin_page_home[]  = FROM "res/homepage_ru.htm";
char buildin_page_help[]  = FROM "res/help_ru.htm";
char accept_language[]= "Accept-Language: ru\n";
char rmb_menu[] = 
"Back|Ctrl+<
Forward|Ctrl+>
-
��ᬮ���� ��室���|Ctrl+U
������஢��� ��室���|F2";
char main_menu[] = 
"������ 䠩�...|Ctrl+O 
����� ����|Ctrl+N 
-
�����|Ctrl+H 
�������� ����㧮�  |Ctrl+J 
������ ���|Ctrl+F5
�������� ��㧥�";
char link_menu[] =
"������ � ����� �������
������ � ����� ����
-
����஢��� ��뫪�
������ ᮤ�ন��� ��뫪�";
char loading_text[] = "����㧪�...";

char update_param[] = "-e http://builds.kolibrios.org/ru_RU/data/programs/cmm/browser/WebView.com";
char update_download_error[] = "'WebView\n�訡�� �� ����祭�� ����������!' -tE";
char update_ok[] = "'WebView\n��㧥� �� �ᯥ譮 ��������!' -tO";
char update_is_current[] = "'WebView\n�� 㦥 �ᯮ���� ��᫥���� �����.' -tI";
char update_can_not_copy[] = "'WebView\n�� ���� ��६����� ����� ����� �� ����� Downloads �� Ramdisk. ��������, �� �����筮 ����.' -tE";
char clear_cache_ok[] = "'WebView\n��� ��饭.' -tI";
#define T_RENDERING "�����ਭ� ��࠭���..."
#define T_DONE_IN_SEC "��⮢�: %i ᥪ (%i ����ᥪ)"
#else
//===================================================//
//                                                   //
//                      ENGLISH                      //
//                                                   //
//===================================================//
char buildin_page_error[] = FROM "res/page_not_found_en.htm";
char buildin_page_home[]  = FROM "res/homepage_en.htm";
char buildin_page_help[]  = FROM "res/help_en.htm";
char accept_language[]= "Accept-Language: en\n";
char rmb_menu[] =
"Back|Ctrl+<
Forward|Ctrl+>
-
View source|Ctrl+U
Edit source|F2";
char main_menu[] = 
"Open local file...|Ctrl+O 
New window|Ctrl+N 
-
History|Ctrl+H 
Download Manager  |Ctrl+J 
Clear cache|Ctrl+F5
Update browser";
char link_menu[] =
"Open in new tab
Open in new window
-
Copy link
Download link contents";
char loading_text[] = "Loading...";
char update_param[] = "-e http://builds.kolibrios.org/en_US/data/programs/cmm/browser/WebView.com";
char update_download_error[] = "'WebView\nError receiving update information!' -tE";
char update_ok[] = "'WebView\nThe browser has been updated!' -tO";
char update_is_current[] = "'WebView\nThe browser is up to date.' -tI";
char update_can_not_copy[] = "'WebView\nError copying a new version from Downloads folder!\nProbably too litle space on Ramdisk.' -tE";
char clear_cache_ok[] = "'WebView\nThe cache has been cleared.' -tI";
#define T_RENDERING "Rendering..."
#define T_DONE_IN_SEC "Done in %i sec (%i milisec)"
#endif

//===================================================//
//                                                   //
//                      GLOBAL                       //
//                                                   //
//===================================================//

char buildin_page_test[]  = FROM "res/test.htm";

#define URL_SERVICE_HISTORY  "WebView:history"
#define URL_SERVICE_HOMEPAGE "WebView:home"
#define URL_SERVICE_TEST     "WebView:test"

char webview_shared[] = "WEBVIEW";

enum { 
	NEW_TAB=600,
	ENCODINGS=700,
	BACK_BUTTON=800, FORWARD_BUTTON,  VIEW_SOURCE,    EDIT_SOURCE,
	REFRESH_BUTTON, GOTOURL_BUTTON,   CHANGE_ENCODING,
	SANDWICH_BUTTON, 
	OPEN_FILE,      NEW_WINDOW,      VIEW_HISTORY,     DOWNLOAD_MANAGER, 
	CLEAR_CACHE,    UPDATE_BROWSER,  IN_NEW_TAB,       IN_NEW_WINDOW,    
	COPY_LINK_URL,  DOWNLOAD_LINK_CT, TAB_ID, 
	TAB_CLOSE_ID = 900
};

char editbox_icons[] = FROM "res/editbox_icons.raw";

#define WIN_W 850
#define WIN_H 920

#define DEFAULT_URL URL_SERVICE_HOMEPAGE

char version[]="WebView 3.91";