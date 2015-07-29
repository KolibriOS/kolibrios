#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define MEMSIZE 0x23E80
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"
#include "..\lib\dll.h"
#include "..\lib\gui.h"
#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\box_lib.h"
#include "..\lib\patterns\restart_process.h"

?define WINDOW_TITLE "Mouse configuration"
?define MOUSE_FRAME_T " Mouse "
?define KEYBOARD_FRAME_T " Keyboard "
?define CHECK_MOUSE "Click to check mouse"
?define POINTER_SPEED "Mouse pointer speed"
?define POINTER_DELAY "Mouse pointer delay"
?define MOUSE_EMULATION "Enable mouse emulation using keyboard NumPad"
?define MADMOUSE "When cursor reaches screen side switch it to inverce side"

frame mouse_frame = { 0, 000, 10, 160, 14, 0x000111, 0xFFFfff, 1, MOUSE_FRAME_T, 0, 0, 6, 0x000111, 0xCCCccc };
frame keyboard_frame = { 0, 000, 10, 73, 217, 0x000111, 0xFFFfff, 1, KEYBOARD_FRAME_T, 0, 0, 6, 0x000111, 0xCCCccc };


unsigned char panels_img_data[] = FROM "panels_image.raw";
raw_image panels_img = { 37, 27, #panels_img_data };

system_colors sc;
proc_info Form;


struct mouse_cfg1 {
	byte pointer_speed, pointer_delay, emulation, madmouse;
} mouse_cfg;


void main() {
	dword id;

	mem_Init();
	load_dll(boxlib, #box_lib_init,0);

	LoadCfg();

	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (id==1) ExitProcess();

				if (id>=100) && (id<200)
				{
					if (id==100) {
						if (mouse_cfg.emulation==true) KillProcessByName("mousemul", SINGLE);
						else RunProgram("/sys/mousemul", 0);
						mouse_cfg.emulation ^= 1;
					}
					if (id==101) {
						if (mouse_cfg.madmouse==true) KillProcessByName("madmouse", SINGLE);
						else RunProgram("/sys/madmouse", 0);
						mouse_cfg.madmouse ^= 1;
					}
					if (id==120) mouse_cfg.pointer_speed++;
					if (id==121) && (mouse_cfg.pointer_speed>0) mouse_cfg.pointer_speed--;
					if (id==122) mouse_cfg.pointer_delay++;
					if (id==123) && (mouse_cfg.pointer_delay>0) mouse_cfg.pointer_delay--;
					DrawWindowContent();
				}
				break;
				
		case evKey:
				if (GetKey()==27) ExitProcess();
				break;
			
		case evReDraw:
				sc.get();
				DefineAndDrawWindow(130, 150, 430, 200+GetSkinHeight(),0x34,sc.work,WINDOW_TITLE);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) break;
				mouse_frame.size_x = keyboard_frame.size_x = - mouse_frame.start_x * 2 + Form.cwidth;
				mouse_frame.font_color = keyboard_frame.font_color = sc.work_text;
				mouse_frame.font_backgr_color = keyboard_frame.font_backgr_color = sc.work;
				mouse_frame.ext_col = keyboard_frame.ext_col = sc.work_graph;
				DrawWindowContent();
	}
}


void DrawWindowContent() {
	char pos_x = 22;

	frame_draw stdcall (#mouse_frame);

	DefineButton(pos_x, mouse_frame.start_y + 12, panels_img.w-1, 27-1, 100 + BT_HIDE, 0);
	_PutImage(pos_x, mouse_frame.start_y + 12,  37, 27, 0 * 37 * 27 * 3 + panels_img.data);
	WriteText(pos_x + 46, mouse_frame.start_y + 20, 0x80, sc.work_text, CHECK_MOUSE);

	PanelCfg_MoreLessBox(pos_x, mouse_frame.start_y + 50, 120, 121, mouse_cfg.pointer_speed, POINTER_SPEED);
	PanelCfg_MoreLessBox(pos_x, mouse_frame.start_y + 80, 122, 123, mouse_cfg.pointer_delay, POINTER_DELAY);

	PanelCfg_CheckBox(pos_x, mouse_frame.start_y +  108,  100, MOUSE_EMULATION, mouse_cfg.emulation);
	PanelCfg_CheckBox(pos_x, mouse_frame.start_y +  130, 101, MADMOUSE, mouse_cfg.madmouse);
}


void PanelCfg_CheckBox(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, sc.work_graph, sc.work_text, value);
}


void PanelCfg_MoreLessBox(dword x, y, id_more, id_less; byte value; dword text) {
	MoreLessBox(x, y, 18, id_more, id_less, #sc, value, text);
}


void LoadCfg() {
	mouse_cfg.pointer_delay = 10;
	mouse_cfg.pointer_speed = 2;
	//CheckProcessExists("MADMOUSE");
	mouse_cfg.madmouse = 0;
	//CheckProcessExists("MOUSEMUL");
	mouse_cfg.emulation = 0;
}

stop: