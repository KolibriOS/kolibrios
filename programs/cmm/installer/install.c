#define MEMSIZE 4096*15
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\copyf.h"

#include "..\lib\obj\libini.h"

#include "..\lib\patterns\restart_process.h"

void main()
{
	//#include "..\lib\added_sysdir.c";
	//SetAdditionalSystemDirectory("kolibrios", abspath("install/kolibrios")+1);
	
	load_dll(libini, #lib_init,1);
	ini_set_int stdcall ("/sys/settings/taskbar.ini", "Flags", "Attachment", 0);
	copyf(abspath("settings"), "/sys/settings");

	RestartProcessByName("/sys/@icon", MULTIPLE);
	RestartProcessByName("/sys/@taskbar", SINGLE);
	RestartProcessByName("/sys/@docky", SINGLE);

	RunProgram("/sys/media/kiv", "\\S__/kolibrios/res/Wallpapers/Free yourself.jpg");

	ExitProcess();
}

void Operation_Draw_Progress(dword filename) { debug("copying: "); debugln(filename); }

stop: