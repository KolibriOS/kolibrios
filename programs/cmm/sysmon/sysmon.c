/*
 * System Monitor
 * version 0.87
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
#define WIN_CONTENT_Y TAB_HEIGHT+WIN_PAD
#define WIN_CONTENT_W 400
#define WIN_CONTENT_H 465
proc_info Form;

#include "general.h"
#include "process.h"

#define T_GENERAL "General"
#define T_PROCESSES "Processes"

enum {
	TAB_GENERAL=20,
	TAB_PROCESSES
};
_tabs tabs = { TAB_GENERAL, 4, 0, WIN_CONTENT_W+WIN_PAD+WIN_PAD-4-4, TAB_HEIGHT };

void main()
{
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);
	load_dll(boxlib, #box_lib_init,0);
	General__Main();
}

int Sysmon__DefineAndDrawWindow()
{
	system.color.get();
	DefineAndDrawWindow(screen.width - WIN_CONTENT_H - 200, 100, WIN_CONTENT_W + WIN_PAD + WIN_PAD +9, 
		WIN_CONTENT_H + TAB_HEIGHT + skin_height + 4, 0x34, system.color.work, "System Monitor",0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return false;
	//if (Form.width  < 300) { MoveSize(OLD,OLD,300,OLD); break; }
	//if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); break; }
	tabs.draw_wrapper();
	tabs.draw_button(tabs.x+TAB_PADDING, TAB_GENERAL, T_GENERAL);	
	tabs.draw_button(strlen(T_GENERAL)*8+tabs.x+TAB_PADDING+TAB_PADDING, TAB_PROCESSES, T_PROCESSES);
	return true;
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void Sysmon__ButtonEvent(dword id)
{
	if (1==id) ExitProcess();
	if (TAB_GENERAL==id) {
		tabs.active_tab = TAB_GENERAL;
		General__Main();
	}
	if (TAB_PROCESSES==id) {
		tabs.active_tab = TAB_PROCESSES;
		Processes__Main();
	}
}


stop: