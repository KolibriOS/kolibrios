#define MEMSIZE 0xA1000
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\io.h"
#include "..\lib\copyf.h"

#include "..\lib\patterns\restart_process.h"

#include "add_appl_dir.c";

?define T_END "\'Установка KolibriN успешно завершена.\' -O"
?define T_LESS_RAM "Мало свободной оперативной памяти. Могут возникнуть проблемы"

void main()
{

	SetAddApplDir("kolibrios", abspath("install/kolibrios")+1);
	io.run("/sys/media/kiv", "\\S__/kolibrios/res/Wallpapers/In the wind there is longing.png");
	io.del("/sys/docpack");
	copyf(abspath("install/sys"), "/sys");
	RestartProcessByName("@icon", MULTIPLE);
	RestartProcessByName("@taskbar", SINGLE);
	RestartProcessByName("@docky", SINGLE);
	notify(T_END);
	io.run("/sys/tmpdisk", "a0s10");
	pause(50);
	copyf(abspath("install/tmp"), "/tmp0/1");
	ExitProcess();
}

void Operation_Draw_Progress(dword filename) { return; }

stop: