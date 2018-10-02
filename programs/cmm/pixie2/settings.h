struct struct_pixie_colors {
	dword color_top_panel_bg,
	      color_top_panel_folder_name,
	      color_top_panel_song_name,
		  color_list_bg,
	      color_list_text,
	      color_list_active_bg,
	      color_list_active_text,
	      color_list_active_pointer,
	      color_list_scroller,
		  color_list_border;
} theme;

_ini ini;

#define WIN_W_SMALL 126
#define WIN_H_SMALL 31

void LoadIniConfig()
{
	ini.path = GetIni(#pixie_ini_path, "app.ini");
	ini.section = "Pixie";
	window_mode   = ini.GetInt("window_mode", WINDOW_MODE_NORMAL);
	win_x_normal  = ini.GetInt("win_x_normal", 100);
	win_y_normal  = ini.GetInt("win_y_normal", 90);
	win_x_small   = ini.GetInt("win_x_small", -1);
	win_y_small   = ini.GetInt("win_y_small", -1);
	ini.GetString("last_folder", #work_folder, sizeof(work_folder), NULL);

	Libimg_LoadImage(#skin, abspath("skin.png"));
	skin.w = 322;
	theme.color_top_panel_bg = 0x242424;
	theme.color_top_panel_folder_name = 0xEEEeee;
	theme.color_top_panel_song_name = 0xBEBEBE;
	theme.color_list_bg = 0x313031;
	theme.color_list_text = 0xADAEAD;
	theme.color_list_active_bg = 0x434343;
	theme.color_list_active_text = 0x17A2CC;
	theme.color_list_active_pointer = 0xD6D6D6;
	theme.color_list_scroller = 0xBBBbbb;
	theme.color_list_border = 0x121212;
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
	ini.SetInt("window_mode", window_mode);
	ini.SetInt("win_x_normal", win_x_normal);
	ini.SetInt("win_y_normal", win_y_normal);
	ini.SetInt("win_x_small", win_x_small);
	ini.SetInt("win_y_small", win_y_small);
	ini.SetString("last_folder", #work_folder, strlen(#work_folder));
}

