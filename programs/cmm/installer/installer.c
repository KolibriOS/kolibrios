#define MEMSIZE 0xA1000
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\figures.h"
#include "..\lib\encoding.h"
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
#include "..\lib\copyf.h"

#include "add_appl_dir.c";

#define LOGOW 16
#define LOGOH 16
#define WIN_W 240
#define WIN_H 140

unsigned char logo[LOGOW*LOGOH*3]= FROM "img\logo.raw";
char iclock[3]={1,2};

proc_info Form;
system_colors sc;

#ifdef LANG_RUS
	?define T_WTITILE "Установка Kolibri N9"
	?define T_END "Установка KolibriN успешно завершена."
#else
	?define T_WTITILE "Kolibri N9 Setup"
	?define T_END "KolibriN install complete."
#endif

void main()
{
	byte id, started=false;
	mem_Init();
   
	loop() switch(WaitEvent())
	{						   
			case evButton:
					if (GetButtonID() == 01) ExitProcess();
					break;
					
			case evReDraw: 
					sc.get();
					DefineAndDrawWindow(GetScreenWidth()-WIN_W/2,GetScreenHeight()-WIN_H/2-30, WIN_W+9, WIN_H+GetSkinHeight()+4,
					0x34,0xFFFfff,T_WTITILE);
					GetProcessInfo(#Form, SelfInfo);
					if (Form.status_window>2) break;

					_PutImage(Form.cwidth-LOGOW/2, Form.height-LOGOH/2, LOGOW,LOGOH, LOGOW*LOGOH*3*iclock[0]+ #logo); //iclock[0]><iclock[1]; 
					WriteTextB(-strlen(T_WTITILE)*6+Form.cwidth/2, Form.cheight - 35, 0x90, 0, T_WTITILE);

					if (!started)
					{
						started = true;
						if (GetFreeRAM()/1024<15) notify("Too less free ram. May cause problems");
						Install();
					}
	}
}


void Install()
{
	int i;
	dword temp;
	proc_info Process;


	for (i=0; i<1000; i++;)
	{
		GetProcessInfo(#Process, i);
		if (strcmp(#Process.name, "@ICON")==0) KillProcess(Process.ID);
	}
	SetAddApplDir("kolibrios", abspath("kolibrios")+1);
	RunProgram("/sys/REFRSCRN", NULL);
	copyf(abspath("sys"), "/rd/1");
	RunProgram("/sys/launcher", NULL);
	SetSystemSkin("/kolibrios/skins/latte.skn");
	notify(T_END);
	ExitProcess();
}

void copyf_Draw_Progress(dword filename) { return; }


stop: