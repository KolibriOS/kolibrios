#include "../lib/obj/libini.h"

_ini ini = { "/sys/settings/app.ini", "Notes" };

void LoadIniSettings()
{
	load_dll(libini, #lib_init,1);
	Form.left     = ini.GetInt("WinX", 150); 
	Form.top      = ini.GetInt("WinY", 50); 
	//Form.width    = ini.GetInt("WinW", 640); 
	//Form.height   = ini.GetInt("WinH", 560); 
}

void SaveIniSettings()
{
	ini.SetInt("WinX", Form.left);
	ini.SetInt("WinY", Form.top);
	//ini.SetInt("WinW", Form.width);
	//ini.SetInt("WinH", Form.height);
}