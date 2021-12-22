#ifdef LANG_RUS
	?define WINDOW_HEADER "Настройки оформления"
	?define T_SELECT_FOLDER "Выбрать папку"
	?define MENU_LIST "Открыть файл   |Enter\nУдалить файл     |Del"
	?define T_PICTURE_MODE " Положение картинки "
	?define T_CHECKBOX_STRETCH "Растянуть"
	?define T_CHECKBOX_TILED "Замостить"
	?define T_UPDATE_DOCK "Обновлять Dock-панель"
	char t_skins[] =       "   Стиль окон";
	char t_wallpapers[] =  "   Обои";
	char t_screensaver[] =  "   Скринсейвер";
#else
	?define WINDOW_HEADER "Appearance"
	?define T_SELECT_FOLDER "Select folder"
	?define MENU_LIST "Open file      |Enter\nDelete file      |Del"
	?define T_PICTURE_MODE " Picture Mode "
	?define T_CHECKBOX_STRETCH "Stretch"
	?define T_CHECKBOX_TILED "Tiled"
	?define T_UPDATE_DOCK "Update Dock"
	char t_skins[] =       "   Skins";
	char t_wallpapers[] =  "   Wallpapers";
	char t_screensaver[] =  "   Screensaver";
#endif

#define WIN_W 600
#define WIN_H 400
#define LIST_W 280
#define PANEL_H 50
#define LP 6 //LIST_PADDING