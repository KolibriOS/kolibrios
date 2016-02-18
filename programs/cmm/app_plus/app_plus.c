#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define MEMSIZE 4096*20
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\io.h"
#include "..\lib\gui.h"
#include "..\lib\obj\proc_lib.h"
#include "..\lib\patterns\simple_open_dialog.h"
#include "..\lib\added_sysdir.c"

char default_dir[] = "/rd/1";
od_filter filter2 = {"",0};

proc_info Form;

dword scr = FROM "scr.raw_8bit";
dword scr_pal[] = {0xFFFFFF,0xBBDDFF,0x4166B5,0xE0E4E6,0xAFBEDD,0xC4D4E8,0x52ACDD,0x000000,
0xE9DAB2,0xC99811,0xFDF9D4,0xF8B93C,0xFDEEBE,0xFBEBA6,0xDFAF4F,0xF3D57C};



void CheckKosMounted()
{
	if (isdir("/kolibrios/")) 
	{
		io.run("syspanel", "/kolibrios/settings/app_plus.ini");
		ExitProcess();
	}
}

void RunAutosearch()
{
	dword searchap_run_id;
	searchap_run_id = io.run("/sys/searchap",0);
	while (GetProcessSlot(searchap_run_id)) pause(10);
}

void main()
{
	word id;

	CheckKosMounted();
	RunAutosearch();
	CheckKosMounted();

	o_dialog.type = 2;
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);

	loop() switch(WaitEvent())
	{	
		case evButton:
			id=GetButtonID();               
			if (id==1) ExitProcess();
			if (id==10)
			{
				OpenDialog_start stdcall (#o_dialog);
				if (o_dialog.status) SetAdditionalSystemDirectory("kolibrios", #openfile_path);
				pause(3);
				CheckKosMounted();
			}
			break;
		 
		case evReDraw:
			draw_window();
	}
}

#define WINDOW_TITLE_TEXT "Error"
#define CONTENT_HEADER_TEXT "/KOLIBRIOS/ NOT MOUNTED"
#define DESCRIPTION_TEXT "Try to find it manually. It should look
like image on the right.
Note: this action can be done only once 
per 1 session of the OS running. If you 
will choose the wrong folder then you 
need to reboot system to try again."
#define MANUALLY_BUTTON_TEXT "Choose /kolibrios/ folder..."


void draw_window()
{
	incn y;
	dword x=30;
	y.n=0;
	system.color.get();
	DefineAndDrawWindow(screen.width-570/2, 100, 570, 280+skin_height, 0x34, system.color.work, WINDOW_TITLE_TEXT);
	GetProcessInfo(#Form, SelfInfo);
	WriteTextB(x+2,y.inc(20)+2,0x81,MixColors(system.color.work, 0xB92234,220),CONTENT_HEADER_TEXT);
	WriteTextB(x,y.n,0x81,0xB92234,CONTENT_HEADER_TEXT);
	
	WriteTextLines(x,y.inc(50),0x90,system.color.work_text,DESCRIPTION_TEXT,20);
	
	PutPaletteImage(#scr,144,171,Form.cwidth-180,y.n,8,#scr_pal);
	DrawRectangle(Form.cwidth-180-1,y.n-1, 144+1,171+1, system.color.work_graph);
	
	DrawCaptButton(x,Form.cheight-66,300,30,10,system.color.work_button,system.color.work_button_text,MANUALLY_BUTTON_TEXT);
}


stop:
