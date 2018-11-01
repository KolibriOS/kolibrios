#define MEMSIZE 4096*20
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\io.h"
#include "..\lib\gui.h"
#include "..\lib\obj\proc_lib.h"
#include "..\lib\patterns\simple_open_dialog.h"
#include "..\lib\patterns\restart_process.h"
#include "..\lib\added_sysdir.c"

#ifndef AUTOBUILD
#include "lang.h--"
#endif

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

proc_info Form;
#define CONX 30 //content X pos

char default_dir[] = "/rd/1";
od_filter filter2 = {"",0};

dword scr = FROM "scr.raw_8bit";
dword scr_pal[] = {0xFFFFFF,0xBBDDFF,0x4166B5,0xE0E4E6,0xAFBEDD,0xC4D4E8,0x52ACDD,0x000000,
0xE9DAB2,0xC99811,0xFDF9D4,0xF8B93C,0xFDEEBE,0xFBEBA6,0xDFAF4F,0xF3D57C};

#define BTN_MANUAL_SEARCH 10
#define BTN_OPEN_ANYWAY 11

#define APP_PLUS_INI_PATH "/sys/settings/app_plus.ini"

//===================================================//
//                                                   //
//                   TRANSLATIONS                    //
//                                                   //
//===================================================//

#ifdef LANG_RUS
?define WINDOW_TITLE_TEXT "Внимание! Это важно."
?define CONTENT_HEADER_TEXT "ПАПКА /KOLIBRIOS/ НЕ НАЙДЕНА"
?define DESCRIPTION_TEXT "Попробуйте найти ее самостоятельно.
Содержимое икомой папки показано на 
картинке справа. В случае неверно 
выбранной папки требуется выполнить 
перезагрузку ПК и попробовать снова."
?define MANUALLY_BUTTON_TEXT "Найти /kolibrios/..."
?define OPEN_ANYWAY_BUTTON_TEXT "Запустить APP+ (некоторые программы будут недоступны)"
#else
?define WINDOW_TITLE_TEXT "Warning! It's important."
?define CONTENT_HEADER_TEXT "/KOLIBRIOS/ IS NOT MOUNTED"
?define DESCRIPTION_TEXT "Try to find it manually. It should look
like image on the right.
Note: this action can be done only once 
per 1 session of the OS running. If you 
will choose the wrong folder then you 
need to reboot system to try again."
?define MANUALLY_BUTTON_TEXT "Choose /kolibrios/ folder..."
?define OPEN_ANYWAY_BUTTON_TEXT "Open APP+ anyway (some apps will be unavailable)"
#endif

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	WaitAutosearch();
	CheckKosMounted();

	o_dialog.type = 2;
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	active_button_id = BTN_MANUAL_SEARCH;

	loop() switch(WaitEvent())
	{	
		case evButton:
			EventButton(GetButtonID());
			break;
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ENTER) {
				EventButton(active_button_id);
			}
			else if (key_scancode == SCAN_CODE_TAB) {
				active_button_id = active_button_id-10^1 + 10;
				DrawButtons();
			}
			break;
		 
		case evReDraw:
			draw_window();
	}
}

void draw_window()
{
	incn y;
	y.n=0;
	system.color.get();
	DefineAndDrawWindow(screen.width-570/2, 100, 570, 300+skin_height, 0x34, system.color.work, WINDOW_TITLE_TEXT,0);
	GetProcessInfo(#Form, SelfInfo);
	WriteTextB(CONX+2,y.inc(20)+2,0x81,MixColors(system.color.work, 0xB92234,220),CONTENT_HEADER_TEXT);
	WriteTextB(CONX,y.n,0x81,0xB92234,CONTENT_HEADER_TEXT);
	
	PutPaletteImage(#scr,144,171,Form.cwidth-180,y.n,8,#scr_pal);
	DrawRectangle(Form.cwidth-180-1,y.n-1, 144+1,171+1, system.color.work_graph);

	WriteTextLines(CONX,y.inc(50),0x90,system.color.work_text,DESCRIPTION_TEXT,20);

	DrawButtons();	
}

void DrawButtons()
{
	DrawStandartCaptButton(CONX, Form.cheight-80, BTN_MANUAL_SEARCH, MANUALLY_BUTTON_TEXT);
	DrawStandartCaptButton(CONX, Form.cheight-42, BTN_OPEN_ANYWAY, OPEN_ANYWAY_BUTTON_TEXT);
}

void CheckKosMounted()
{
	if (dir_exists("/kolibrios")) 
	{
		if (file_exists(APP_PLUS_INI_PATH))	EventOpenApp();
		ExitProcess();
	}
}

void WaitAutosearch()
{
	while (CheckProcessExists("SEARCHAP")) pause(2);
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventManualSearch()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) SetAdditionalSystemDirectory("kolibrios", #openfile_path+1);
	pause(3);
	CheckKosMounted();	
}

void EventOpenApp()
{
	RunProgram("syspanel", APP_PLUS_INI_PATH);
}

void EventButton(dword id)
{
	if (id==CLOSE_BTN) ExitProcess();
	else if (id==BTN_MANUAL_SEARCH) EventManualSearch();
	else if (id==BTN_OPEN_ANYWAY) { EventOpenApp();	 ExitProcess(); }
}


stop:
