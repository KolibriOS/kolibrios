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

char config_section[] = "Config";

void LoadIniConfig()
{
	ini_get_int stdcall (#pixie_ini_path, #config_section, "window_mode", WINDOW_MODE_NORMAL); window_mode = EAX;
	ini_get_int stdcall (#pixie_ini_path, #config_section, "win_x_normal", 100); win_x_normal = EAX;
	ini_get_int stdcall (#pixie_ini_path, #config_section, "win_y_normal", 90); win_y_normal = EAX;
	ini_get_int stdcall (#pixie_ini_path, #config_section, "win_x_small", -1); win_x_small = EAX;
	ini_get_int stdcall (#pixie_ini_path, #config_section, "win_y_small", -1); win_y_small = EAX;
	ini_get_str stdcall (#pixie_ini_path, #config_section, "last_folder", #work_folder, sizeof(work_folder), 0);

	Libimg_LoadImage(#skin, abspath("skin.png"));
	skin.w = 322;
	theme.color_top_panel_text = 0x5C5146;
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

	if (win_x_small==-1) win_x_small = 2000;
	if (win_y_small==-1) win_y_small = GetClientHeight() - WIN_H_SMALL + 1;

}

void SaveIniConfig()
{
	if (window_mode == WINDOW_MODE_NORMAL)
	{
		win_x_normal = Form.left;
		win_y_normal = Form.top;
	}
	if (window_mode == WINDOW_MODE_SMALL)
	{
		win_x_small = Form.left;
		win_y_small = Form.top;
	}
	ini_set_int stdcall (#pixie_ini_path, #config_section, "window_mode", window_mode);
	ini_set_int stdcall (#pixie_ini_path, #config_section, "win_x_normal", win_x_normal);
	ini_set_int stdcall (#pixie_ini_path, #config_section, "win_y_normal", win_y_normal);
	ini_set_int stdcall (#pixie_ini_path, #config_section, "win_x_small", win_x_small);
	ini_set_int stdcall (#pixie_ini_path, #config_section, "win_y_small", win_y_small);
	ini_set_str stdcall (#pixie_ini_path, #config_section, "last_folder", #work_folder, strlen(#work_folder));
}

