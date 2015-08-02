#define MEMSIZE 0xA1000
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\copyf.h"

#include "..\lib\patterns\restart_process.h"

#include "add_appl_dir.c";

?define T_END "\'Установка KolibriN успешно завершена.\' -O"
?define T_LESS_RAM "Мало свободной оперативной памяти. Могут возникнуть проблемы"

void main()
{

	SetAddApplDir("kolibrios", abspath("kolibrios")+1);
	RunProgram("/sys/media/kiv", "\\S__/kolibrios/res/Wallpapers/In the wind there is longing.png");
	copyf(abspath("tmp"), "/tmp0/1");
	copyf(abspath("sys"), "/sys");
	RestartProcessByName("@icon", MULTIPLE);
	RestartProcessByName("@taskbar", SINGLE);
	RestartProcessByName("@docky", SINGLE);
	notify(T_END);
	ExitProcess();
}

void copyf_Draw_Progress(dword filename) { return; }


stop: