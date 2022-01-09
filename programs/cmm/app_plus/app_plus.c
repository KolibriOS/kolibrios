#define MEMSIZE 1024*40
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\gui.h"
#include "..\lib\obj\proc_lib.h"
#include "..\lib\patterns\simple_open_dialog.h"
#include "..\lib\patterns\restart_process.h"
#include "added_sysdir.c"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define WINW 570
#define WINH 300
#define CONX 30 //content X pos
#define SCRX WINW-180
#define SCRY 20

char default_dir[] = "/sys";
od_filter filter2;

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
Содержимое искомой папки показано на 
картинке справа. В случае неверно 
выбранной папки требуется выполнить 
перезагрузку ПК и попробовать снова."
?define MANUALLY_BUTTON_TEXT "Указать папку /kolibrios/..."
?define OPEN_ANYWAY_BUTTON_TEXT "Запустить APP+ (некоторые программы будут недоступны)"
#else
?define WINDOW_TITLE_TEXT "Warning! It's important."
?define CONTENT_HEADER_TEXT "/KOLIBRIOS/ IS NOT MOUNTED"
?define DESCRIPTION_TEXT "Try to find it manually. It should look
like image on the right.
Note: this action can be done only once 
per 1 session of the OS running. If you 
will choose the wrong folder then you 
need to reboot a system to try again."
?define MANUALLY_BUTTON_TEXT "Choose /kolibrios/ folder..."
?define OPEN_ANYWAY_BUTTON_TEXT "Open APP+ anyway (some programs won't be available)  "
#endif

char kolibrios_dirs[] = "..\0     3D     \0demos  \0develop\0drivers\0emul   \0games  \0grafx2";

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	if (param) {
		SetAdditionalSystemDirectory("kolibrios", #param+1);
		ExitProcess();
	}

	WaitAutosearch();
	CheckKosMounted();

	o_dialog.type = 2;
	#define NO_DLL_INIT
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	active_button_id = BTN_MANUAL_SEARCH;

	loop() switch(@WaitEvent())
	{	
		case evButton:
			EventButton(GetButtonID());
			break;
		case evKey:
			@GetKeyScancode();
			if (AL == SCAN_CODE_ENTER) {
				EventButton(active_button_id);
			} else if (AL == SCAN_CODE_TAB) {
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
	sc.get();
	DefineAndDrawWindow(screen.w-WINW/2, 100, WINW, WINH+skin_h, 0x34, sc.work, WINDOW_TITLE_TEXT,0);
	WriteTextB(CONX+2,SCRY+2,0x81,MixColors(sc.work, 0xB92234,220),CONTENT_HEADER_TEXT);
	WriteTextB(CONX,SCRY,0x81,0xB92234,CONTENT_HEADER_TEXT);
	draw_screen();
	WriteTextLines(CONX,SCRY+50,0x90,sc.work_text,DESCRIPTION_TEXT,20);
	DrawButtons();	
}

void draw_screen()
{
	char i;
	int icon_n=1;
	DrawRectangle(WINW-180-1,SCRY-1, 145+1,170+1, sc.line);
	DrawBar(SCRX,SCRY,145,170,0xFFFfff);
	DrawBar(SCRX+25,SCRY+5,144-25,20,0xBBDDFF);
	for (i=0; i<8; i++) {
		draw_icon_16(SCRX+5, i*20+SCRY+5, icon_n);
		WriteText(SCRX+27, i*20+SCRY+11, 0x80, 0, i*8 + #kolibrios_dirs);
		icon_n = 0;
	}
}

void DrawButtons()
{
	DrawStandartCaptButton(CONX, WINH-80, BTN_MANUAL_SEARCH, MANUALLY_BUTTON_TEXT);
	DrawStandartCaptButton(CONX, WINH-42, BTN_OPEN_ANYWAY, OPEN_ANYWAY_BUTTON_TEXT);
	//DrawCaptButton(CONX, WINH-80, 300, 25, BTN_MANUAL_SEARCH, sc.button, sc.button_text, MANUALLY_BUTTON_TEXT);
	//DrawCaptButton(CONX, WINH-42, 500, 25, BTN_OPEN_ANYWAY, sc.button, sc.button_text, OPEN_ANYWAY_BUTTON_TEXT);
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
	if (RunProgram("/sys/syspanel", APP_PLUS_INI_PATH) < 0) {
		notify("'App+ can not be started because\n/sys/syspanel does not exists' -E");
	}
}

void EventButton(dword id)
{
	if (id==CLOSE_BTN) ExitProcess();
	else if (id==BTN_MANUAL_SEARCH) EventManualSearch();
	else if (id==BTN_OPEN_ANYWAY) { EventOpenApp(); ExitProcess(); }
}


stop:
