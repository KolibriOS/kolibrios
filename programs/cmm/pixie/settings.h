void LoadIniConfig()
{
	debugln(#pixie_ini_path);
	ini_get_int stdcall (#pixie_ini_path, "Config", "current_theme", THEME_DARK);
	current_theme = EAX;
	if (current_theme == THEME_DARK) SetColorThemeDark(); else SetColorThemeLight();

	ini_get_int stdcall (#pixie_ini_path, "Config", "window_mode", WINDOW_MODE_NORMAL);
	window_mode = EAX;

	ini_get_int stdcall (#pixie_ini_path, "Config", "win_x", 100);
	win_x = EAX;

	ini_get_int stdcall (#pixie_ini_path, "Config", "win_y", 90);
	win_y = EAX;

	ini_get_str stdcall (#pixie_ini_path, "Config", "last_folder", #work_folder, sizeof(work_folder), 0);
}

void SaveIniConfig()
{
	debugln(#pixie_ini_path);
	ini_set_int stdcall (#pixie_ini_path, "Config", "current_theme", current_theme);
	ini_set_int stdcall (#pixie_ini_path, "Config", "window_mode", window_mode);
	ini_set_int stdcall (#pixie_ini_path, "Config", "win_x", Form.left);
	ini_set_int stdcall (#pixie_ini_path, "Config", "win_y", Form.top);
	ini_set_str stdcall (#pixie_ini_path, "Config", "last_folder", #work_folder, strlen(#work_folder));
}

struct struct_pixie_colors {
	dword color_top_panel_text,
		  color_list_bg,
	      color_list_text,
	      color_list_active_bg,
	      color_list_active_text,
	      color_list_active_pointer,
	      color_list_scroller,
		  color_list_border;
} theme;


void SetColorThemeDark()
{
	current_theme = THEME_DARK;
	Libimg_LoadImage(#skin, abspath("s_dark.png"));
	skin.w = 300;
	theme.color_top_panel_text = 0xFCFFBE;
	theme.color_list_bg = 0x313031;
	theme.color_list_text = 0xADAEAD;
	theme.color_list_active_bg = 0x434343;
	theme.color_list_active_text = 0xADAEAD;
	theme.color_list_active_pointer = 0xD6D6D6;
	theme.color_list_scroller = 0xBBBbbb;
	theme.color_list_border = 0x121212;
	scroll1.bckg_col = theme.color_list_bg;
	scroll1.frnt_col = theme.color_list_border;
	scroll1.line_col = theme.color_list_border;
	DrawWindow();
}


void SetColorThemeLight()
{
	current_theme = THEME_LIGHT;
	Libimg_LoadImage(#skin, abspath("s_light.png"));
	skin.w = 300;
	theme.color_top_panel_text = 0x85663F;
	theme.color_list_bg = 0xE2E2E2;
	theme.color_list_text = 0x595959;
	theme.color_list_active_bg = 0xFAF3AF;
	theme.color_list_active_text = 0x85663F;
	theme.color_list_active_pointer = 0x85663F;
	theme.color_list_scroller = 0xBBBbbb;
	theme.color_list_border = 0x736D65;
	scroll1.bckg_col = theme.color_list_bg;
	scroll1.frnt_col = theme.color_list_border;
	scroll1.line_col = theme.color_list_border;
	DrawWindow();
}