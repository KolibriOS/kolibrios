char ini_path[4096];
char config_section[] = "Config";
int encoding;

void LoadIniSettings()
{
	kfont.size.pt = 14;
	encoding = CH_CP866;
	Form.left = 150;
	Form.top = 50;
	Form.width = 640;
	Form.height = 560;
	/*
	strcpy(#ini_path, "/sys/settings/treader.ini");
	ini_get_int stdcall   (#ini_path, #config_section, "FontSize",  14); kfont.size.pt = EAX;
	ini_get_int stdcall   (#ini_path, #config_section, "Encoding",  CH_CP866); encoding = EAX;
	ini_get_int stdcall   (#ini_path, #config_section, "WinX", 150); Form.left = EAX;
	ini_get_int stdcall   (#ini_path, #config_section, "WinY", 50); Form.top = EAX;
	ini_get_int stdcall   (#ini_path, #config_section, "WinW", 640); Form.width = EAX;
	ini_get_int stdcall   (#ini_path, #config_section, "WinH", 560); Form.height = EAX;
	*/
}

void SaveIniSettings()
{
	/*
	ini_set_int stdcall (#ini_path, #config_section, "FontSize", kfont.size.pt);
	ini_set_int stdcall (#ini_path, #config_section, "Encoding", encoding);
	ini_set_int stdcall (#ini_path, #config_section, "WinX", Form.left);
	ini_set_int stdcall (#ini_path, #config_section, "WinY", Form.top);
	ini_set_int stdcall (#ini_path, #config_section, "WinW", Form.width);
	ini_set_int stdcall (#ini_path, #config_section, "WinH", Form.height);
	*/
}
