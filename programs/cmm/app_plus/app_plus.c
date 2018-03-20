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
#include "..\lib\patterns\restart_process.h"
#include "..\lib\added_sysdir.c"


char default_dir[] = "/rd/1";
od_filter filter2 = {"",0};

proc_info Form;

dword scr = FROM "scr.raw_8bit";
dword scr_pal[] = {0xFFFFFF,0xBBDDFF,0x4166B5,0xE0E4E6,0xAFBEDD,0xC4D4E8,0x52ACDD,0x000000,
0xE9DAB2,0xC99811,0xFDF9D4,0xF8B93C,0xFDEEBE,0xFBEBA6,0xDFAF4F,0xF3D57C};

#define BTN_MANUAL_SEARCH 10

#define APP_PLUS_INI_PATH "/kolibrios/settings/app_plus.ini"

#define APP_PLUS_INI_NOT_EXISTS "'APP+\n/kolibrios/settings/app_plus.ini does not exists.\nProgram terminated.' -tE"

#define WINDOW_TITLE_TEXT "Error"
#define CONTENT_HEADER_TEXT "/KOLIBRIOS/ IS NOT MOUNTED"
#define DESCRIPTION_TEXT "Try to find it manually. It should look
like image on the right.
Note: this action can be done only once 
per 1 session of the OS running. If you 
will choose the wrong folder then you 
need to reboot system to try again."
#define MANUALLY_BUTTON_TEXT "Choose /kolibrios/ folder..."


void CheckKosMounted()
{
	if (dir_exists("/kolibrios")) 
	{
		if (file_exists(APP_PLUS_INI_PATH))	
			RunProgram("syspanel", APP_PLUS_INI_PATH);
		else
			notify(APP_PLUS_INI_NOT_EXISTS);
		ExitProcess();
	}
}

void WaitAutosearch()
{
	while (CheckProcessExists("SEARCHAP")) pause(2);
}

void main()
{
	word id;

	CheckKosMounted();
	WaitAutosearch();
	CheckKosMounted();

	o_dialog.type = 2;
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	active_button_id = BTN_MANUAL_SEARCH;

	loop() switch(WaitEvent())
	{	
		case evButton:
			id=GetButtonID();               
			if (id==1) ExitProcess();
			if (id==BTN_MANUAL_SEARCH) EventManualSearch();
			break;
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ENTER) EventManualSearch();
			break;
		 
		case evReDraw:
			draw_window();
	}
}

void draw_window()
{
	incn y;
	dword x=30;
	y.n=0;
	system.color.get();
	DefineAndDrawWindow(screen.width-570/2, 100, 570, 280+skin_height, 0x34, system.color.work, WINDOW_TITLE_TEXT,0);
	GetProcessInfo(#Form, SelfInfo);
	WriteTextB(x+2,y.inc(20)+2,0x81,MixColors(system.color.work, 0xB92234,220),CONTENT_HEADER_TEXT);
	WriteTextB(x,y.n,0x81,0xB92234,CONTENT_HEADER_TEXT);
	
	WriteTextLines(x,y.inc(50),0x90,system.color.work_text,DESCRIPTION_TEXT,20);
	
	PutPaletteImage(#scr,144,171,Form.cwidth-180,y.n,8,#scr_pal);
	DrawRectangle(Form.cwidth-180-1,y.n-1, 144+1,171+1, system.color.work_graph);
	DrawStandartCaptButton(x, Form.cheight-66, BTN_MANUAL_SEARCH, MANUALLY_BUTTON_TEXT);
}

void EventManualSearch()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) SetAdditionalSystemDirectory("kolibrios", #openfile_path+1);
	pause(3);
	CheckKosMounted();	
}


stop:
