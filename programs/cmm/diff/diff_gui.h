//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/patterns/simple_open_dialog.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

//proc_info Form;
#define WIN_W 450

char default_dir[] = "/sys";
od_filter filter2 = {0,0};

char src_path[4096];
char dst_path[4096];
edit_box src_box = {WIN_W-36-DOT_W,18,30,0xffffff,0x94AECE,0xFFFfff,
	0xffffff,0x10000000,sizeof(src_path)-2,#src_path,0, ed_focus};
edit_box dst_box = {WIN_W-36-DOT_W,18,85,0xffffff,0x94AECE,0xFFFfff,
	0xffffff,0x10000000,sizeof(dst_path)-2,#dst_path,0, 0b};

enum {
	BID_EXIT_PRC=1,
	BID_SRC_OPEN,
	BID_DST_OPEN,
	BID_GO
};

#define T_FIRST "First file:"
#define T_SECOND "Second file:"
#define T_GO  " Compare "


#define READY 0
int state=READY;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void gui()
{
	#define NO_DLL_INIT
	load_dll(boxlib, #box_lib_init,0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);

	loop() switch(@WaitEvent())
	{
		case evMouse:
			if (READY == state) {
				edit_box_mouse stdcall (#src_box);
				edit_box_mouse stdcall (#dst_box);				
			}
			break;

		case evButton:
			@GetButtonID();
			if (EAX == BID_EXIT_PRC) ExitProcess();
			if (EAX == BID_SRC_OPEN) EventOpenDialogFirst();
			if (EAX == BID_DST_OPEN) EventOpenDialogSecond();
			if (EAX == BID_GO) EventGo();
			break;
	  
		case evKey:
			@GetKey();
			edit_box_key stdcall (#src_box);
			edit_box_key stdcall (#dst_box);
			EAX >>= 16;
			if (AL == SCAN_CODE_ESC) ExitProcess();
			if (AL == SCAN_CODE_TAB) EventTabClick();
			if (AL == SCAN_CODE_ENTER) EventGo();
			if (AL == SCAN_CODE_INS) EventInsert();
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void draw_window()
{
	sc.get();
	DefineAndDrawWindow(215, 100, WIN_W+9, 170 + GetSkinHeight(), 0x34, sc.work, #window_title,0);
	//GetProcessInfo(#Form, SelfInfo);
	if (READY==state) {
		DrawFileBox(#src_box, T_FIRST, BID_SRC_OPEN);
		DrawFileBox(#dst_box, T_SECOND, BID_DST_OPEN);
		DrawCaptButton(dst_box.left - 2, dst_box.top + 40, 120, 25, BID_GO, sc.button, sc.button_text, T_GO);
	}
}

void UpdateEditBoxes(dword flag1, flag2)
{
	EditBox_UpdateText(#src_box, flag1);
	EditBox_UpdateText(#dst_box, flag2);
	edit_box_draw stdcall (#src_box);
	edit_box_draw stdcall (#dst_box);
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventGo()
{
	char run_param[4096];
	wsprintf(#run_param, "\"%s\" \"%s\"", #src_path, #dst_path);
	RunProgram(I_Path, #run_param);
}

void EventTabClick()
{
	if ( src_box.flags & ed_focus ) {
		UpdateEditBoxes(0, ed_focus);
	} else {
		UpdateEditBoxes(ed_focus, 0);
	} 
}

void EventOpenDialogFirst()
{
	o_dialog.type = 0; //0-file, 1-save, 2-select folder
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		strcpy(#src_path, #openfile_path);
		UpdateEditBoxes(ed_focus, 0);
	}
}

void EventOpenDialogSecond()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		strcpy(#dst_path, #openfile_path);
		UpdateEditBoxes(0, ed_focus);
	}
}

void EventInsert()
{
	if ( src_box.flags & ed_focus ) EventOpenDialogFirst();
	if ( dst_box.flags & ed_focus ) EventOpenDialogSecond();
}
