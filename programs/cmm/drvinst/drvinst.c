#define MEMSIZE 4096*20

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libio_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/patterns/select_list.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

proc_info Form;

#define WINDOW_TITLE "Driver Installer"
#define T_CAUTION_TITLE "CAUTION"
#define T_CAUTION_PARAGRAPH "Installing additional drivers can be harmful to the stability of the operation system and potentionally can harm hardware."
#define T_ASSEPT_RISK "I assept the risk"
#define T_README "Readme"
#define T_INSTALL "Install"

#define BUTTON_ID_ASSEPT_RISK 10
#define BUTTON_ID_README 11
#define BUTTON_ID_INSTALL 12

//WINDOW STEPS
#define WINDOW_STEP_INTRO 1;
#define WINDOW_STEP_DRIVER_LIST 2;
char window_step = WINDOW_STEP_INTRO;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void GetIniData()
{
	ini_enum_sections stdcall ("/sys/drvinf.ini", #process_sections);
}

byte process_sections(dword sec_name, f_name)
{
	ini_enum_keys stdcall (f_name, sec_name, #process_keys);
	return true;
}

byte process_keys(dword key_value, key_name, sec_name, f_name)
{
	debugln(key_value);
	return true;
}

void main()
{
	int id;
	load_dll(libio,  #libio_init,1);
	load_dll(libini, #lib_init,1);
	load_dll(boxlib, #box_lib_init,0);
	//GetIniData();
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evMouse:
			if (!CheckActiveProcess(Form.ID)) return;
			SelectList_ProcessMouse();
			break;

		case evButton:
			id=GetButtonID();               
			if (id==1) ExitProcess();
			if (id==BUTTON_ID_ASSEPT_RISK) Event_AsseptRisk();
			if (id==BUTTON_ID_README) Event_ShowReadme();
			if (id==BUTTON_ID_INSTALL) Event_RunInstall();
			break;
	  
		case evKey:
			GetKeys();
			if (select_list.ProcessKey(key_scancode)) SelectList_LineChanged();
			break;
		 
		case evReDraw:
			Event_DrawWindow();
	}
} 


void draw_intro_window()
{
	incn y;
	y.n = Form.cheight/2 - 80;
	WriteTextB(30+2,y.n+2,0x81,MixColors(system.color.work, 0xB92234,220),T_CAUTION_TITLE);
	WriteTextB(30,y.n,0x81,0xB92234,T_CAUTION_TITLE);
	y.n = DrawTextViewArea(30, y.inc(30), Form.cwidth-60, Form.cheight-140, 
		T_CAUTION_PARAGRAPH, -1, system.color.work_text);
	DrawStandartCaptButton(30, y.inc(10), BUTTON_ID_ASSEPT_RISK, T_ASSEPT_RISK);
}


void draw_driver_list_window()
{
	int PADDING = 12;
	int right_frame_x = Form.cwidth/2 + PADDING + calc(PADDING/2);
	//LEFT FRAME
	select_list.count = 4;
	SelectList_Init(PADDING, 
		PADDING, 
		Form.cwidth/2 - PADDING - scroll1.size_x, 
		Form.cheight - PADDING - PADDING, 
		false);
	SelectList_Draw();
	//RIGHT FRAME
	WriteTextB(right_frame_x, PADDING+3, 0x81, system.color.work_text, "ATI KMS");
	WriteText(right_frame_x, PADDING+23, 0x80, MixColors(system.color.work, system.color.work_text,120), "ver 4.4");
	DrawTextViewArea(right_frame_x-2, PADDING+53, Form.cwidth - right_frame_x - PADDING, Form.cheight-100, 
		T_CAUTION_PARAGRAPH, -1, system.color.work_text);
	right_frame_x += DrawStandartCaptButton(right_frame_x, Form.cheight-40, BUTTON_ID_README, T_README);
	DrawStandartCaptButton(right_frame_x, Form.cheight-40, BUTTON_ID_INSTALL, T_INSTALL);
}

void SelectList_DrawLine(dword i)
{
	int yyy, list_last;

	yyy = i*select_list.item_h+select_list.y;
	
	if (select_list.cur_y-select_list.first==i)
	{
		DrawBar(select_list.x, yyy, select_list.w, select_list.item_h, system.color.work_button);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,system.color.work_button_text, "Hello");
	}
	else
	{
		DrawBar(select_list.x,yyy,select_list.w, select_list.item_h, 0xFFFfff);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,0, "Hello");
	}
}

void SelectList_LineChanged()
{
	SelectList_Draw();
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void Event_DrawWindow() 
{
	system.color.get();
	DefineAndDrawWindow(215, 100, 600, 400, 0x33, system.color.work, WINDOW_TITLE);
	GetProcessInfo(#Form, SelfInfo);
	if (window_step == WINDOW_STEP_INTRO) draw_intro_window();
	if (window_step == WINDOW_STEP_DRIVER_LIST) draw_driver_list_window();
	return;
}

void Event_AsseptRisk()
{
	window_step = WINDOW_STEP_DRIVER_LIST;
	Event_DrawWindow();
}

void Event_ShowReadme()
{
	return;
}

void Event_RunInstall()
{
	return;
}