#define MEMSIZE 4096*20

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/libini.h"
#include "../lib/collection.h"
#include "../lib/io.h"
#include "../lib/patterns/select_list.h"
#include "../lib/patterns/restart_process.h"

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
	#define T_DRIVER_INSTALLARION_STARTED "'Началась установка драйвера.\nЛог установки находится в приложении BOARD.'I"
	char description_name[] = "description_ru";
#else
	#define WINDOW_TITLE "Driver Installer"
	#define T_CAUTION_TITLE "CAUTION"
	#define T_CAUTION_PARAGRAPH "Installing additional drivers can be harmful to the stability of the operation system and potentionally can harm hardware."
	#define T_ASSEPT_RISK "I accept the risk"
	#define T_README "Readme"
	#define T_INSTALL "Install"
	#define T_DRIVER_INSTALLARION_STARTED "'Driver installation started.\nInstallation log can be found in BOARD app.'I"
	char description_name[] = "description_en";
#endif

#define BUTTON_ID_ASSEPT_RISK 10
#define BUTTON_ID_README 11
#define BUTTON_ID_INSTALL 12

//WINDOW STEPS
#define WINDOW_STEP_INTRO 1;
#define WINDOW_STEP_DRIVER_LIST 2;
char window_step = WINDOW_STEP_INTRO;

collection ini_sections=0;

char drvinf_path[4096] = "/kolibrios/drivers/drvinf.ini";
char cur_version[64];
int  cur_icon;
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
	load_dll(libini, #lib_init,1);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libimg, #libimg_init,1);
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
	WriteTextB(30+2,y.n+2,0x81,MixColors(sc.work, 0xB92234,220),T_CAUTION_TITLE);
	WriteTextB(30,y.n,0x81,0xB92234,T_CAUTION_TITLE);
	y.n = DrawTextViewArea(30, y.inc(30), Form.cwidth-60, Form.cheight-140,
		T_CAUTION_PARAGRAPH, -1, sc.work_text);
	active_button_id = BUTTON_ID_ASSEPT_RISK;
	DrawStandartCaptButton(30, y.inc(10), BUTTON_ID_ASSEPT_RISK, T_ASSEPT_RISK);
}


void Draw_DriverListWindow()
{
	#define PADDING 12
	int right_frame_x = Form.cwidth*46/100;
	int readme_w = 0;
	//LEFT FRAME
	SelectList_Init(PADDING, PADDING,
		right_frame_x - PADDING - PADDING - 8 - scroll1.size_x,
		Form.cheight - PADDING - PADDING);
	SelectList_Draw();
	SelectList_DrawBorder();
	//RIGHT FRAME
	GetCurrentSectionData();
	DrawBar(right_frame_x, PADDING+3, Form.cwidth - right_frame_x - PADDING, 80, sc.work);
	draw_icon_32(right_frame_x, PADDING, sc.work, cur_icon);
	WriteTextB(right_frame_x+44, PADDING+3, 0x81, sc.work_text, ini_sections.get(select_list.cur_y));
	WriteText(right_frame_x+44, PADDING+23, 0x80, sc.work_text, #cur_version);
	if(cur_readme_path[0]) readme_w = DrawStandartCaptButton(right_frame_x, PADDING+45, BUTTON_ID_README, T_README);
	DrawStandartCaptButton(right_frame_x + readme_w, PADDING+45, BUTTON_ID_INSTALL, T_INSTALL);
	DrawTextViewArea(right_frame_x-2, PADDING+83, Form.cwidth - right_frame_x - PADDING, Form.cheight-PADDING-PADDING,
		#cur_description, sc.work, sc.work_text);
}

void SelectList_DrawLine(dword i)
{
	int yyy, list_last;

	yyy = i*select_list.item_h+select_list.y;

	if (select_list.cur_y-select_list.first==i)
	{
		DrawBar(select_list.x, yyy, select_list.w, select_list.item_h, sc.button);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,sc.button_text, ini_sections.get(i+select_list.first));
	}
	else
	{
		DrawBar(select_list.x,yyy,select_list.w, select_list.item_h, 0xFFFfff);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,0, ini_sections.get(i+select_list.first));
	}
}

void SelectList_LineChanged()
{
	Draw_DriverListWindow();
}


void GetCurrentSectionData()
{
	dword section_name = ini_sections.get(select_list.cur_y);
	ini_get_str stdcall (#drvinf_path, section_name, "ver", #cur_version, sizeof(cur_version), 0);
	ini_get_int stdcall (#drvinf_path, section_name, "icon", 38); cur_icon = EAX;
	ini_get_str stdcall (#drvinf_path, section_name, #description_name, #cur_description, sizeof(cur_description), 0);
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
	sc.get();
	DefineAndDrawWindow(215, 100, 600, 400, 0x33, sc.work, WINDOW_TITLE,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window&ROLLED_UP) return;
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
	io.run("/sys/@open", #cur_readme_path);
}

void Event_RunInstall()
{
	int result;
	result = io.run(#cur_install_path, NULL);
	if (result) notify(T_DRIVER_INSTALLARION_STARTED);
	pause(300);
	if (cur_icon == 61) {
		RestartProcessByName("/sys/@taskbar", SINGLE);
		RestartProcessByName("/sys/@docky", SINGLE);
		RestartProcessByName("/sys/@icon", MULTIPLE);
	}
}