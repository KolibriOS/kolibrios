#ifdef LANG_RUS
	char t_skins[] =       "   Стиль окон";
	char t_wallpapers[] =  "   Обои";
	char t_screensaver[] =  "   Скринсейвер";
	?define WINDOW_HEADER "Настройки оформления"
	?define T_SELECT_FOLDER "Выбрать папку"
	?define T_PICTURE_MODE " Положение картинки "
	?define T_CHECKBOX_STRETCH "Растянуть"
	?define T_CHECKBOX_TILED "Замостить"
	?define T_CHECKBOX_AUTO "Автоматически"
	?define T_UPDATE_DOCK "Обновлять Dock-панель"
	?define T_NO_FILES "'Поддерживаемые файлы не найдены' -E"
	?define T_UI_PREVIEW " Пример компонентов "
	?define T_SS_TIMEOUT "Интервал в минутах: %i "
	?define T_SS_PREVIEW "Просмотр"
	?define T_SS_SET "Установить"
	?define T_NO_SS "[Выключен]"
	?define T_DEFAULT "[По умолчанию]"
#else
	char t_skins[] =       "   Skins";
	char t_wallpapers[] =  "   Wallpapers";
	char t_screensaver[] =  "   Screensaver";
	?define WINDOW_HEADER "Appearance"
	?define T_SELECT_FOLDER "Select folder"
	?define T_PICTURE_MODE " Picture Mode "
	?define T_CHECKBOX_STRETCH "Stretch"
	?define T_CHECKBOX_TILED "Tiled"
	?define T_CHECKBOX_AUTO "Auto"
	?define T_UPDATE_DOCK "Update Dock"
	?define T_NO_FILES "'No supported files were found' -E"
	?define T_UI_PREVIEW " Components Preview "
	?define T_SS_TIMEOUT "Wait in minutes: %i "
	?define T_SS_PREVIEW "View"
	?define T_SS_SET "Set"
	?define T_NO_SS "[Disable]"
	?define T_DEFAULT "[Default]"
#endif

#define WIN_W 560
#define WIN_H 445
#define LIST_W 260
#define PANEL_H 50
#define LP 6 //LIST_PADDING
#define SL_VISIBLE WIN_H - PANEL_H - LP / SELECT_LIST_ITEMH

#define RIGHTx LP + LIST_W + TAB_P + 30
#define RIGHTy PANEL_H
#define RIGHTw WIN_W - RIGHTx - LP - TAB_P
#define RIGHTh 215

enum {
	TAB_SKINS, 
	TAB_WALLPAPERS, 
	TAB_SCREENSAVERS
};

enum { 
	BASE_TAB_BUTTON_ID=3, 
	BTN_SELECT_WALLP_FOLDER=10,
	BTN_TEST_SCREENSAVER,
	BTN_SET_SCREENSAVER
};

_ini ini = { "/sys/settings/system.ini" };

char default_dir[] = "/sys";
od_filter filter2 = { 8, "TXT\0\0" };

_tabs tabs = { -sizeof(t_skins)-sizeof(t_wallpapers)-sizeof(t_screensaver)
	-3*8+WIN_W - TAB_P / 2, LP, NULL, BASE_TAB_BUTTON_ID };

scroll_bar ss_timeout = { RIGHTw-19,RIGHTx,15,RIGHTy+25,0,3,89,10,0,0xFFFfff,
	0xBBBbbb,0xeeeeee};





void sort_by_name(int a, b) // for the first call: a = 0, b = sizeof(mas) - 1
{
	int j;
	int isn = a;
	if (a >= b) return;
	for (j = a; j <= b; j++) {
		if (strcmpi(io.dir.position(ESDWORD[j*4+fmas]), io.dir.position(ESDWORD[b*4+fmas]))<=0) { 
			ESDWORD[isn*4+fmas] >< ESDWORD[j*4+fmas]; 
			isn++;
		}
	}
	sort_by_name(a, isn-2);
	sort_by_name(isn, b);
}

dword get_real_kolibrios_path()
{
	char real_kolibrios_path[256];
	if (!dir_exists("/kolibrios")) return 0;
	SetCurDir("/kolibrios");
	GetCurDir(#real_kolibrios_path, sizeof(real_kolibrios_path));
	return #real_kolibrios_path;
}

void SelectList_LineChanged() 
{
	EventApply();
}
