#include "../lib/obj/libini.h"

_ini ini = { "/sys/settings/eskin.ini", "main" };

void SaveSkinSettings(dword skin_path)
{
	char real_skin_path[4096];
	SetCurDir("/kolibrios");
	GetCurDir(#real_skin_path, sizeof(real_skin_path));
	strcat(#real_skin_path, skin_path+10);
	ini.SetString("skin", #real_skin_path, strlen(#real_skin_path));
}
