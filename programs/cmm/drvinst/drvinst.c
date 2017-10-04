#define MEMSIZE 4096*20

#ifndef AUTOBUILD
#include "lang.h--"
#endif

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libio.h"
#include "../lib/obj/libini.h"
#include "../lib/collection.h"
#include "../lib/io.h"
#include "../lib/patterns/select_list.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

proc_info Form;

#ifdef LANG_RUS
	#define WINDOW_TITLE "Установщик драйверов"
	#define T_CAUTION_TITLE "ПРЕДУПРЕЖДЕНИЕ"
	#define T_CAUTION_PARAGRAPH "Установка дополнительных драйверов может нанести вред стабильности операционной системы и потенциально привести к порче оборудования."
	#define T_ASSEPT_RISK "Я принимаю риск"
	#define T_README "Readme"
	#define T_INSTALL "Установить"
#else
	#define WINDOW_TITLE "Driver Installer"
	#define T_CAUTION_TITLE "CAUTION"
	#define T_CAUTION_PARAGRAPH "Installing additional drivers can be harmful to the stability of the operation system and potentionally can harm hardware."
	#define T_ASSEPT_RISK "I accept the risk"
	#define T_README "Readme"
	#define T_INSTALL "Install"
#endif

#define BUTTON_ID_ASSEPT_RISK 10
#define BUTTON_ID_README 11
#define BUTTON_ID_INSTALL 12

//WINDOW STEPS
#define WINDOW_STEP_INTRO 1;
#define WINDOW_STEP_DRIVER_LIST 2;
char window_step = WINDOW_STEP_INTRO;

collection ini_sections;

char drvinf_path[4096] = "/kolibrios/drivers/drvinf.ini";
char cur_version[64];
char cur_description[1024];
char cur_readme_path[4096];
char cur_install_path[4096];

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void GetIniData()
{
	select_list.count = 0;
	ini_enum_sections stdcall (#drvinf_path, #process_sections);
}

byte process_sections(dword sec_name, f_name)
{
	select_list.count++;
	ini_sections.add(sec_name);
	return true;
}

void main()
{
	load_dll(libio,  #libio_init,1);
	load_dll(libini, #lib_init,1);
	load_dll(boxlib, #box_lib_init,0);
	GetIniData();
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
		case evMouse:
			SelectList_ProcessMouse();
			break;

		case evButton:
			Event_ProcessButtonId(GetButtonID());
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ENTER) Event_ProcessButtonId(active_button_id);
			if (window_step == WINDOW_STEP_DRIVER_LIST) 
			{
				if (select_list.ProcessKey(key_scancode)) SelectList_LineChanged();
				if (key_scancode == SCAN_CODE_TAB) 
				{
					ActiveButtonSwitch(11, 12);
					Draw_DriverListWindow();
				}
			} 
			break;
		 
		case evReDraw:
			Event_DrawWindow();
	}
} 


void Draw_IntroWindow()
{
	incn y;
	y.n = Form.cheight/2 - 80;
	WriteTextB(30+2,y.n+2,0x81,MixColors(system.color.work, 0xB92234,220),T_CAUTION_TITLE);
	WriteTextB(30,y.n,0x81,0xB92234,T_CAUTION_TITLE);
	y.n = DrawTextViewArea(30, y.inc(30), Form.cwidth-60, Form.cheight-140, 
		T_CAUTION_PARAGRAPH, -1, system.color.work_text);
	active_button_id = BUTTON_ID_ASSEPT_RISK;
	DrawStandartCaptButton(30, y.inc(10), BUTTON_ID_ASSEPT_RISK, T_ASSEPT_RISK);
}


void Draw_DriverListWindow()
{
	int PADDING = 12;
	int right_frame_x = Form.cwidth*46/100;
	int readme_w = 0;
	//LEFT FRAME
	SelectList_Init(PADDING, 
		PADDING, 
		right_frame_x - PADDING - PADDING - 8 - scroll1.size_x, 
		Form.cheight - PADDING - PADDING, 
		false);
	SelectList_Draw();
	SelectList_DrawBorder();
	//RIGHT FRAME
	GetCurrentSectionData();
	DrawBar(right_frame_x, PADDING+3, Form.cwidth - right_frame_x - PADDING, 80, system.color.work);
	WriteTextB(right_frame_x, PADDING+3, 0x81, system.color.work_text, ini_sections.get(select_list.cur_y));
	WriteText(right_frame_x, PADDING+23, 0x80, system.color.work_text, #cur_version);
	if(cur_readme_path[0]) readme_w = DrawStandartCaptButton(right_frame_x, PADDING+45, BUTTON_ID_README, T_README);
	DrawStandartCaptButton(right_frame_x + readme_w, PADDING+45, BUTTON_ID_INSTALL, T_INSTALL);
	DrawTextViewArea(right_frame_x-2, PADDING+83, Form.cwidth - right_frame_x - PADDING, Form.cheight-PADDING-PADDING, 
		#cur_description, system.color.work, system.color.work_text);
}

void SelectList_DrawLine(dword i)
{
	int yyy, list_last;

	yyy = i*select_list.item_h+select_list.y;
	
	if (select_list.cur_y-select_list.first==i)
	{
		DrawBar(select_list.x, yyy, select_list.w, select_list.item_h, system.color.work_button);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,system.color.work_button_text, ini_sections.get(i));
	}
	else
	{
		DrawBar(select_list.x,yyy,select_list.w, select_list.item_h, 0xFFFfff);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,0, ini_sections.get(i));
	}
}

void SelectList_LineChanged()
{
	Draw_DriverListWindow();
}


void GetCurrentSectionData()
{
	dword section_name = ini_sections.get(select_list.cur_y);
	dword description_name;
	if (GetSystemLanguage() == SYS_LANG_RUS) description_name = "description_ru"; else description_name = "description_en";
	ini_get_str stdcall (#drvinf_path, section_name, "ver", #cur_version, sizeof(cur_version), 0);
	ini_get_str stdcall (#drvinf_path, section_name, description_name, #cur_description, sizeof(cur_description), 0);
	ini_get_str stdcall (#drvinf_path, section_name, "readme", #cur_readme_path, sizeof(cur_readme_path), 0);
	ini_get_str stdcall (#drvinf_path, section_name, "install", #cur_install_path, sizeof(cur_install_path), 0);
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void Event_ProcessButtonId(int id)
{
	if (id==1) ExitProcess();
	if (id==BUTTON_ID_ASSEPT_RISK) Event_AsseptRisk();
	if (id==BUTTON_ID_README) Event_ShowReadme();
	if (id==BUTTON_ID_INSTALL) Event_RunInstall();
}

void Event_DrawWindow() 
{
	system.color.get();
	DefineAndDrawWindow(215, 100, 600, 400, 0x33, system.color.work, WINDOW_TITLE,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	if (Form.width  < 450) { MoveSize(OLD,OLD,450,OLD); return; }
	if (Form.height < 250) { MoveSize(OLD,OLD,OLD,250); return; }
	if (window_step == WINDOW_STEP_INTRO) Draw_IntroWindow();
	if (window_step == WINDOW_STEP_DRIVER_LIST) Draw_DriverListWindow();
	return;
}

void Event_AsseptRisk()
{
	window_step = WINDOW_STEP_DRIVER_LIST;
	active_button_id = BUTTON_ID_INSTALL;
	Event_DrawWindow();
}

void Event_ShowReadme()
{
	io.run("/sys/txtread", #cur_readme_path);
}

void Event_RunInstall()
{
	int result;
	result = io.run(#cur_install_path, NULL);
	if (result) notify("'Driver installation started.\nPlease, open BOARD to check status.'I");
}