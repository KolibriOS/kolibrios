#include "../lib/gui.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/patterns/simple_open_dialog.h"

char default_dir[] = "/rd/1";
od_filter filter2 = {0,0};

int mouse_dd1, mouse_dd2;
char src_box_text[4096];
char dst_box_text[4096];
edit_box src_box = {340,20,35,0xffffff,0x94AECE,0xFFFfff,0xffffff,0x10000000,sizeof(src_box_text),#src_box_text,#mouse_dd1, 10b};
edit_box dst_box = {340,20,95,0xffffff,0x94AECE,0xFFFfff,0xffffff,0x10000000,sizeof(dst_box_text),#dst_box_text,#mouse_dd2, 0b};

#define BID_EXIT_PRC 01
#define BID_SRC_OPEN 10
#define BID_DST_OPEN 11
#define BID_COMPARE  12

proc_info Form;

void gui()
{
	word btn;
	char run_param[4096];
	load_dll(boxlib, #box_lib_init,0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	SetEventMask(0x27);

	debugln("GUI");

	loop() switch(WaitEvent())
	{
		case evMouse:
			edit_box_mouse stdcall (#src_box);
			edit_box_mouse stdcall (#dst_box);
			break;

		case evButton:
			btn = GetButtonID();
			switch (btn) 
			{
			case BID_EXIT_PRC:
				ExitProcess();
			case BID_SRC_OPEN:
				OpenDialog_start stdcall (#o_dialog);
				if (o_dialog.status) {
					strcpy(#src_box_text, #openfile_path);
					src_box.size = src_box.pos 
					= src_box.shift = src_box.shift_old = strlen(#src_box_text);
				}
				break;
			case BID_DST_OPEN:
				OpenDialog_start stdcall (#o_dialog);
				if (o_dialog.status) {
					strcpy(#dst_box_text, #openfile_path);
					dst_box.size = dst_box.pos 
					= dst_box.shift = dst_box.shift_old = strlen(#dst_box_text);
				}
				break;
			case BID_COMPARE:
				sprintf(#run_param, "\"%s\" \"%s\"", #src_box_text, #dst_box_text);
				io.run(I_Path, #run_param);
				break;
			}
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			if (key_scancode == SCAN_CODE_TAB) {
				if ( src_box.flags & 10b ) {
					src_box.flags -= 10b;
					dst_box.flags += 10b;
				} else {
					src_box.flags += 10b;
					dst_box.flags -= 10b;					
				} 		
				edit_box_draw stdcall (#src_box);
				edit_box_draw stdcall (#dst_box);
			}
			EAX = key_editbox;
			edit_box_key stdcall (#src_box);
			EAX = key_editbox;
			edit_box_key stdcall (#dst_box);
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void draw_window()
{
	system.color.get();
	DefineAndDrawWindow(215, 100, 450, 195 + skin_height, 0x34, system.color.work, #window_title,0);
	GetProcessInfo(#Form, SelfInfo);

	WriteText(src_box.left-2, src_box.top-21, 0x90, system.color.work_text, "First file:");
	WriteText(dst_box.left-2, dst_box.top-21, 0x90, system.color.work_text, "Second file:");
	DrawEditBox(#src_box);
	DrawEditBox(#dst_box);
	DrawStandartCaptButton(src_box.left + src_box.width + 15, src_box.top-3, BID_SRC_OPEN, "...");
	DrawStandartCaptButton(dst_box.left + dst_box.width + 15, dst_box.top-3, BID_DST_OPEN, "...");
	DrawStandartCaptButton(dst_box.left - 2, dst_box.top + 40, BID_COMPARE, "Compare");
}