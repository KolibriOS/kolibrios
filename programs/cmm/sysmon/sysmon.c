/*
 * System Monitor
 * version 1.0
 * Author: Leency
*/

#define MEMSIZE 4096*30

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/io.h"
#include "../lib/gui.h"
#include "../lib/fs.h"
#include "../lib/list_box.h"

#include "../lib/obj/libio.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/box_lib.h"

#include "../lib/patterns/select_list.h"
#include "../lib/patterns/restart_process.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define WIN_PAD 20   //Window padding
#define WIN_CONTENT_X WIN_PAD
#define WIN_CONTENT_Y TAB_HEIGHT+WIN_PAD+20
#define WIN_CONTENT_W 400
#define WIN_CONTENT_H 465
#define ICONGAP 26
proc_info Form;

#ifdef LANG_RUS
#define T_CPU_AND_RAM   "Процессор и ОЗУ"
#define T_DRIVES     "Диски"
#define T_PROCESSES "Процессы"
#define T_APP_TITLE "System Monitor"
#else
#define T_CPU_AND_RAM   "CPU & RAM"
#define T_DRIVES     "Drives"
#define T_PROCESSES "Processes"
#define T_APP_TITLE "System Monitor"
#endif

enum {
	TAB_GENERAL=20,
	TAB_DRIVES,
	TAB_PROCESSES
};

_tabs tabs = { TAB_GENERAL, 4, 10, WIN_CONTENT_W+WIN_PAD+WIN_PAD-4-4, TAB_HEIGHT };

//===================================================//
//                                                   //
//                 GENERAL  EVENTS                   //
//                                                   //
//===================================================//

int Sysmon__DefineAndDrawWindow()
{
	dword butx;
	sc.get();
	DefineAndDrawWindow(screen.width - WIN_CONTENT_H - 200, 100, WIN_CONTENT_W + WIN_PAD + WIN_PAD +9, 
		WIN_CONTENT_H + TAB_HEIGHT + skin_height + 4, 0x34, sc.work, T_APP_TITLE,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return false;
	//if (Form.width  < 300) { MoveSize(OLD,OLD,300,OLD); break; }
	//if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); break; }
	tabs.draw_wrapper();
	butx = tabs.draw_button(tabs.x+TAB_PADDING, TAB_GENERAL, T_CPU_AND_RAM);	
	butx = tabs.draw_button(strlen(T_CPU_AND_RAM)*8+TAB_PADDING+butx, TAB_DRIVES, T_DRIVES);	
	       tabs.draw_button(strlen(T_DRIVES)*8+TAB_PADDING+butx, TAB_PROCESSES, T_PROCESSES);
	return true;
}

int Sysmon__ButtonEvent()
{
	int bid = GetButtonID();
	if (1==bid) ExitProcess();
	if (TAB_GENERAL==bid) {
		tabs.active_tab = TAB_GENERAL;
		CPUnRAM__Main();
	}
	if (TAB_PROCESSES==bid) {
		tabs.active_tab = TAB_PROCESSES;
		Processes__Main();
	}
	if (TAB_DRIVES==bid) {
		tabs.active_tab = TAB_DRIVES;
		Drives__Main();
	}
	return bid;
}

void Sysmon__KeyEvent()
{
	GetKeys();
	if (key_scancode == SCAN_CODE_ESC) ExitProcess();
}

#include "cpu_ram.h"
#include "drives.h"
#include "process.h"

void main()
{
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);
	load_dll(boxlib, #box_lib_init,0);
	CPUnRAM__Main();
}