#define MEMSIZE 4096*15
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\io.h"
#include "..\lib\copyf.h"

#include "..\lib\patterns\restart_process.h"
//#include "..\lib\added_sysdir.c";

void main()
{
	//SetAdditionalSystemDirectory("kolibrios", abspath("install/kolibrios")+1);
	io.run("/sys/tmpdisk", "a0");
	pause(50);

	io.del("/sys/docpack");

	copyf(abspath("rd"), "/sys");
	copyf(abspath("kos"), "/kolibrios");
	copyf(abspath("tmp"), "/tmp0/1");

	KillProcessByName("@icon", MULTIPLE);
	KillProcessByName("@taskbar", SINGLE);
	KillProcessByName("@docky", SINGLE);

	RunProgram("/sys/@icon", NULL);
	RunProgram("/sys/@taskbar", NULL);
	RunProgram("/sys/@docky", NULL);

	io.run("/sys/media/kiv", "\\S__/kolibrios/res/Wallpapers/deink.png");
	//notify("'KolibriNext\nInstall complete'-tO");
	ExitProcess();
}

void Operation_Draw_Progress(dword filename) { debug("copying: "); debugln(filename); }

stop: