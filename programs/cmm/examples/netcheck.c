/*
 * Network testing utility
 * Leency 2018
*/

#define MEMSIZE 4096*10

#include "../lib/fs.h"
#include "../lib/gui.h"
#include "../lib/events.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

char download_file[] = "-d http://kolibri-n.org/files/KolibriN9.zip";

EVENTS button;
EVENTS key;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	loop() switch(WaitEvent())
	{
		case evButton:
			button.press( GetButtonID() );
			break;
	  
		case evKey:
			GetKeys();
			key.press( key_scancode );
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void draw_window()
{
	system.color.get();
	DefineAndDrawWindow(215, 100, 350, 300, 0x34, system.color.work, "Network testing utility",0);
	button.init(10);
	key.init(10);

	WriteText(10,10, 0x90, system.color.work_text, "Download via:");
	AddEvent(10, 30, #EventRunAsmDownloader, SCAN_CODE_F1, "ASM Downloader [F1]");
	AddEvent(10, 60, #EventRunCmmDownloader, SCAN_CODE_F2, "C-- Downloader [F2]");
	AddEvent(10, 120, #EventRunNetStat, SCAN_CODE_F3, "NetStat [F3]");
	AddEvent(10, 150, #EventRunNetCfg, SCAN_CODE_F4, "NetCfg [F4]");
	AddEvent(10, 180, #EventRunBoard, SCAN_CODE_F5, "Board [F5]");
	button.add_n(1, #ExitProcess);
	key.add_n(SCAN_CODE_ESC, #ExitProcess);
}

void AddEvent(dword bx, by, event, hotkey, text)
{
	DrawStandartCaptButton(bx, by, button.add(event), text);
	key.add_n(hotkey, event);
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventRunAsmDownloader()
{
	RunProgram("/sys/network/downloader", #download_file+3);
}

void EventRunCmmDownloader()
{
	RunProgram("/sys/network/webview", #download_file);
}

void EventRunNetStat()
{
	RunProgram("/sys/network/netstat", NULL);
}

void EventRunNetCfg()
{
	RunProgram("/sys/network/netcfg", NULL);
}

void EventRunBoard()
{
	RunProgram("/sys/develop/board", NULL);
}

