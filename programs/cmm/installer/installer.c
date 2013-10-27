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
#define BLACK_H 40
#define TEXTX 20
#define WIN_W 300
#define WIN_H 200

unsigned char logo[LOGOW*LOGOH*3]= FROM "img\logo.raw";

proc_info Form;
system_colors sc;
char dialog;
enum {
	INSTALL,
	END
};

#ifdef LANG_RUS
	?define T_WTITILE "Установка Kolibri N9"
	?define T_END "Установка KolibriN успешно завершена."
#else
	?define T_WTITILE "Kolibri N9 Setup"
	?define T_END "KolibriN install complete."
#endif


int DefineWindow(dword wtitle, wbutton)
{
	sc.get();
	DefineAndDrawWindow(GetScreenWidth()-WIN_W/2,GetScreenHeight()-WIN_H/2-30, WIN_W+9, WIN_H+GetSkinHeight()+4,0x74,0,T_WTITILE);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return 0; //rolled_up

	DrawBar(0, 0, Form.cwidth, BLACK_H, 0);
	_PutImage(BLACK_H-LOGOW/2, BLACK_H-LOGOH/2, LOGOW,LOGOH, #logo);
	WriteTextB(BLACK_H-LOGOW + LOGOW, BLACK_H-6/2, 0x90, 0xFFFfff, wtitle);
	DrawBar(0, BLACK_H, Form.cwidth, Form.cheight-BLACK_H, 0xFFFfff);
	DrawCaptButton(Form.cwidth-107, Form.cheight-40, 90, 24, 10, sc.work_button, sc.work_button_text,wbutton);
	return 1;
}

void main()
{
	mem_Init();
	InstallationLoop(INSTALL);
}

char iclock[3]={1,2};

void InstallationLoop(int dialog_t)
{
	byte id, key, started=false;
	int free_ram;
	unsigned char free_ram_text[256];

	dialog = dialog_t;
	goto _DRAW_WIN;
   
	loop() switch(WaitEvent())
	{						   
			case evButton:
					id=GetButtonID();
					if (id == 01) ExitProcess();
					if (id == 11) RunProgram("/sys/htmlv", "http://kolibri-n.org/index.php");
					if (id == 10)
					{
						if (dialog==INSTALL) InstallationLoop(END);
						else if (dialog==END) ExitProcess();
					}
					break;
					
			case evReDraw: 
					_DRAW_WIN:
					if (dialog==INSTALL)
					{
						if !(DefineWindow("Installation Started", "Stop")) break;
						//iclock[0]><iclock[1]; 
						_PutImage(Form.cwidth-LOGOW/2, Form.height-LOGOH/2, LOGOW,LOGOH, LOGOW*LOGOH*3*iclock[0]+ #logo);

						if (!started)
						{
							started = true;
							if (GetFreeRAM()/1024<15) notify("Too less free ram. May cause problems");
							Install();
						}
					}
					if (dialog==END)
					{
						if !(DefineWindow("Installation complete", "Exit")) break;
						WriteText(TEXTX, BLACK_H*2, 0x80, 0, T_END);
						DrawLink(TEXTX, BLACK_H*2+15, 0x80, 11, "http://kolibri-n.org");
					}
	}
}


void Install()
{
	int i;
	proc_info Process;

	for (i=0; i<256; i++;)
	{
		GetProcessInfo(#Process, i);
		if (i==Form.ID) || (strcmp(#Process.name, "OS")==0) continue;
		KillProcess(i);
	}
	SetAddApplDir("kolibrios", abspath("kolibrios")+1);
	RunProgram("/sys/REFRSCRN", NULL);
	copyf(abspath("sys"), "/rd/1");
	RunProgram("/sys/launcher", NULL);
	SetSystemSkin("/kolibrios/skins/latte.skn");
	InstallationLoop(END);
	//===to tmp===
	// RunProgram("/sys/tmpdisk", "a9s100");
	// copyf(abspath("tmp"), "/tmp9/1");
}

void copyf_Draw_Progress(dword filename) { return; }


stop: