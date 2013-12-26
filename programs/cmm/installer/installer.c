#define MEMSIZE 0xA1000
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\copyf.h"

#include "add_appl_dir.c";

#ifdef LANG_RUS
	?define T_WTITILE "Установка Kolibri N9"
	?define T_END "Установка KolibriN успешно завершена."
	?define T_LESS_RAM "Мало свободной оперативной памяти. Могут возникнуть проблемы"
#else
	?define T_WTITILE "Kolibri N9 Setup"
	?define T_END "KolibriN install complete."
	?define T_LESS_RAM "Too less free ram. May cause problems"
#endif

void main()
{
	int i;
	proc_info Process;

	mem_Init();
	if (GetFreeRAM()/1024<15) notify(T_LESS_RAM);

	for (i=0; i<1000; i++;)
	{
		GetProcessInfo(#Process, i);
		if (strcmp(#Process.name, "@ICON")==0) KillProcess(Process.ID);
	}
	SetAddApplDir("kolibrios", abspath("kolibrios")+1);
	RunProgram("/sys/REFRSCRN", NULL);
	copyf(abspath("sys"), "/rd/1");
	RunProgram("/sys/launcher", NULL);
	SetSystemSkin("/kolibrios/res/skins/OpusN.skn");
	notify(T_END);
	DeleteFile("/sys/3d/free3d04");
	DeleteFile("/sys/games/invaders");
	ExitProcess();	
}


void copyf_Draw_Progress(dword filename) { return; }


stop: