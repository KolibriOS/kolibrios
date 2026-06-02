#define MEMSIZE 1024*60

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
#define CONX 28 //content X pos
#define CONY 25 //content Y pos

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
?define WINDOW_TITLE_TEXT "Внимание!"
?define CONTENT_HEADER_TEXT "KolibriOS работает в режиме дискеты"
?define DESCRIPTION_TEXT "Многие приложения сейчас недоступны.

Причина в том, что не смонтирован дополнительный системный
каталог. Но вы можете найти и задать его вручную. Его название
'kolibrios'. Вы также можете загрузить latest-dirstr.7z через
WebView и найти его внутри."
?define MANUALLY_BUTTON_TEXT "Указать папку /kolibrios/..."
?define OPEN_ANYWAY_BUTTON_TEXT "Запустить APP+ (некоторые программы будут недоступны)"
#else
?define WINDOW_TITLE_TEXT "Warning!"
?define CONTENT_HEADER_TEXT "KolibriOS is running in floppy mode"
?define DESCRIPTION_TEXT "A lot of apps aren't available.

The reason is that additional system directory is not mounted.
But you can find and set it manually. Its name is 'kolibrios'.
You can also download latest-dirstr.7z via WebView and find
it inside."
?define MANUALLY_BUTTON_TEXT "Choose /kolibrios/ folder..."
?define OPEN_ANYWAY_BUTTON_TEXT "Open APP+ anyway (some programs won't be available)"
#endif

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
	draw_icon_16w(CONX+2, CONY-3, 5);
	WriteTextB(CONX+27+2,CONY+2,0x81,MixColors(sc.work, 0xB92234,220),CONTENT_HEADER_TEXT);
	WriteTextB(CONX+27,CONY,0x81,0xB92234,CONTENT_HEADER_TEXT);
	WriteTextLines(CONX,CONY+42,0x90,sc.work_text,DESCRIPTION_TEXT,20);
	DrawButtons();
}

void DrawButtons()
{
	DrawStandartCaptButton(CONX, WINH-CONY-58, BTN_MANUAL_SEARCH, MANUALLY_BUTTON_TEXT);
	DrawStandartCaptButton(CONX, WINH-CONY-22, BTN_OPEN_ANYWAY, OPEN_ANYWAY_BUTTON_TEXT);
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
		notify("'App+ cannot start because\n/sys/syspanel does not exist' -E");
	}
}

void EventButton(dword id)
{
	if (id==CLOSE_BTN) ExitProcess();
	else if (id==BTN_MANUAL_SEARCH) EventManualSearch();
	else if (id==BTN_OPEN_ANYWAY) {
		EventOpenApp();
		ExitProcess();
	}
}

stop:
