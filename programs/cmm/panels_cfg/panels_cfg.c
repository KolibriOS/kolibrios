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
#include "..\lib\obj\libini.h"
#include "..\lib\obj\box_lib.h"

#include "..\lib\patterns\restart_process.h"

#ifdef LANG_RUS
	?define WINDOW_TITLE "Настройки панели задач и Дока"
    ?define TASK_FRAME_T " Панель задач "
	?define DOCK_FRAME_T " Док "
	?define MIN_LEFT_BUTTON "Кнопка скрытия слева"
	?define MIN_RIGHT_BUTTON "Кнопка скрытия справа"
	?define SOFTEN_UP   "Сглаживание сверху"
	?define SOFTEN_DOWN "Сглаживание снизу"
	?define CLOCK    "Часы"
	?define CPU_USAGE "Загрузка ЦП"
	?define CHANGE_LANG "Язык ввода"
	?define MENU_BUTTON "Кнопка меню"
	?define PANEL_HEIGHT "Высота панели"
	?define SOFTEN_HEIGHT "Высота сглаживания"
	?define BUTTON_OFFSET "Поле вокруг кнопок"
	?define FSIZE "Режим панели"
	?define ASHOW "Не скрывать"
	?define CHANGE_POS "Нажмите на изображение для смены позиции"
#else
	?define WINDOW_TITLE "Taskbar and Docky configuration"
    ?define TASK_FRAME_T " Taskbar "
	?define DOCK_FRAME_T " Docky "
	?define MIN_LEFT_BUTTON "Min Left Button"
	?define MIN_RIGHT_BUTTON "Min Right Button"
	?define SOFTEN_UP   "Soften Up"
	?define SOFTEN_DOWN "Soften Down"
	?define CLOCK    "Clock"
	?define CPU_USAGE "Cpu Usage"
	?define CHANGE_LANG "Change Language"
	?define MENU_BUTTON "Menu Button"
	?define PANEL_HEIGHT "Panel Height"
	?define SOFTEN_HEIGHT "Soften Height"
	?define BUTTON_OFFSET "Button Offset"
	?define FSIZE "Full width"
	?define ASHOW "Always show"
	?define CHANGE_POS "Click on image to change position"
#endif


frame taskbar_frame = { 0, 000, 10, 188, 14, 0x000111, 0xFFFfff, 1, TASK_FRAME_T, 0, 0, 6, 0x000111, 0xCCCccc };
frame docky_frame = { 0, 000, 10, 73, 217, 0x000111, 0xFFFfff, 1, DOCK_FRAME_T, 0, 0, 6, 0x000111, 0xCCCccc };

char taskbar_ini_path[] = "/sys/settings/taskbar.ini";
char taskbar_c_flags[] = "Flags";
char taskbar_c_variables[] = "Variables";
char docky_ini_path[] = "/sys/settings/docky.ini";

unsigned char panels_img_data[] = FROM "panels_image.raw";
raw_image panels_img = { 37, 27, #panels_img_data };

proc_info Form;

struct docky_cfg {
	word fsize;
	byte location, ashow;
} docky_cfg;

struct taskbar_cfg {
	byte Attachment;
	byte PanelHeight, SoftenHeight, ButtonOffset;
	byte SoftenUp, SoftenDown, MinLeftButton, MinRightButton, MenuButton,
	     RunApplButton, ClnDeskButton, Clock, CpuUsage, ChangeLang;
} taskbar_cfg;

enum {
	TASKBAR,
	DOCKY,
	ALL
};


void main()
{
	dword id, key;

	load_dll(libini, #lib_init,1);
	load_dll(boxlib, #box_lib_init,0);

	LoadCfg();

	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (id==1) ExitProcess();
				//taskbar buttons
				if (id>=100) && (id<200)
				{
					if (id==100) taskbar_cfg.Attachment ^= 1;
					if (id==105) taskbar_cfg.SoftenUp ^= 1;
					if (id==106) taskbar_cfg.SoftenDown ^= 1;
					if (id==107) taskbar_cfg.MinLeftButton ^= 1;
					if (id==108) taskbar_cfg.MinRightButton ^= 1;
					if (id==109) taskbar_cfg.RunApplButton ^= 1;
					if (id==110) taskbar_cfg.ClnDeskButton ^= 1;
					if (id==111) taskbar_cfg.Clock ^= 1;
					if (id==112) taskbar_cfg.CpuUsage ^= 1;
					if (id==113) taskbar_cfg.ChangeLang ^= 1;
					if (id==114) taskbar_cfg.MenuButton ^= 1;
					if (id==120) taskbar_cfg.PanelHeight++;
					if (id==121) && (taskbar_cfg.PanelHeight>6) taskbar_cfg.PanelHeight--;
					if (id==122) taskbar_cfg.SoftenHeight++;
					if (id==123) && (taskbar_cfg.SoftenHeight>0) taskbar_cfg.SoftenHeight--;
					if (id==124) taskbar_cfg.ButtonOffset++;
					if (id==125) && (taskbar_cfg.ButtonOffset>0) taskbar_cfg.ButtonOffset--;
					DrawWindowContent(TASKBAR);
					SaveCfg(TASKBAR);
					RestartProcess(TASKBAR);
				}
				//docky buttons			
				if (id>=200)
				{
					if (id==200)
					{
						     if (docky_cfg.location==1) docky_cfg.location = 2;
						else if (docky_cfg.location==2) docky_cfg.location = 3;
						else if (docky_cfg.location==3) docky_cfg.location = 1;
					}
					if (id==201) docky_cfg.fsize ^= 1;
					if (id==202) docky_cfg.ashow ^= 1;
					DrawWindowContent(DOCKY);
					SaveCfg(DOCKY);
					RestartProcess(DOCKY);
				}
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				break;
			
		case evReDraw:
				system.color.get();
				DefineAndDrawWindow(130, 150, 400, 300+GetSkinHeight(),0x34,system.color.work,WINDOW_TITLE);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) break;
				taskbar_frame.size_x = docky_frame.size_x = - taskbar_frame.start_x * 2 + Form.cwidth;
				taskbar_frame.font_color = docky_frame.font_color = system.color.work_text;
				taskbar_frame.font_backgr_color = docky_frame.font_backgr_color = system.color.work;
				taskbar_frame.ext_col = docky_frame.ext_col = system.color.work_graph;
				DrawWindowContent(ALL);
	}
}


void DrawWindowContent(byte panel_type)
{
	word win_center_x;

	if (panel_type==ALL)
	{
		frame_draw stdcall (#taskbar_frame);
		frame_draw stdcall (#docky_frame);
	}
	if (panel_type==ALL) || (panel_type==TASKBAR)
	{
		DefineButton(22, taskbar_frame.start_y + 12, panels_img.w-1, 27-1, 100 + BT_HIDE, 0);
		_PutImage(22, taskbar_frame.start_y + 12,  37, 27, taskbar_cfg.Attachment * 37 * 27 * 3 + panels_img.data);
		WriteText(68, taskbar_frame.start_y + 20, 0x80, system.color.work_text, CHANGE_POS);
		PanelCfg_CheckBox(22, taskbar_frame.start_y +  48, 105, SOFTEN_UP, taskbar_cfg.SoftenUp);
		PanelCfg_CheckBox(22, taskbar_frame.start_y +  68, 106, SOFTEN_DOWN, taskbar_cfg.SoftenDown);
		PanelCfg_CheckBox(22, taskbar_frame.start_y +  88, 107, MIN_LEFT_BUTTON, taskbar_cfg.MinLeftButton);
		PanelCfg_CheckBox(22, taskbar_frame.start_y + 108, 108, MIN_RIGHT_BUTTON, taskbar_cfg.MinRightButton);
		win_center_x = Form.cwidth / 2;
		PanelCfg_CheckBox(win_center_x, taskbar_frame.start_y +  48, 111, CLOCK, taskbar_cfg.Clock);
		PanelCfg_CheckBox(win_center_x, taskbar_frame.start_y +  68, 112, CPU_USAGE, taskbar_cfg.CpuUsage);
		PanelCfg_CheckBox(win_center_x, taskbar_frame.start_y +  88, 113, CHANGE_LANG, taskbar_cfg.ChangeLang);
		PanelCfg_CheckBox(win_center_x, taskbar_frame.start_y + 108, 114, MENU_BUTTON, taskbar_cfg.MenuButton);	
		PanelCfg_MoreLessBox(22, taskbar_frame.start_y + 131, 120, 121, taskbar_cfg.PanelHeight, PANEL_HEIGHT);
		PanelCfg_MoreLessBox(win_center_x, taskbar_frame.start_y + 131, 122, 123, taskbar_cfg.SoftenHeight, SOFTEN_HEIGHT);
		PanelCfg_MoreLessBox(22, taskbar_frame.start_y + 159, 124, 125, taskbar_cfg.ButtonOffset, BUTTON_OFFSET);
	}
	if (panel_type==ALL) || (panel_type==DOCKY)
	{
		DefineButton(22, docky_frame.start_y + 12, panels_img.w-1, 27-1, 200 + BT_HIDE, 0);
		_PutImage(22, docky_frame.start_y + 12,  37, 27, docky_cfg.location + 1 * 37 * 27 * 3 + panels_img.data);
		WriteText(68, docky_frame.start_y + 20, 0x80, system.color.work_text, CHANGE_POS);
		PanelCfg_CheckBox(22, docky_frame.start_y + 48, 201, FSIZE,  docky_cfg.fsize);
		PanelCfg_CheckBox(win_center_x, docky_frame.start_y + 48, 202, ASHOW, docky_cfg.ashow);
	}
}

void LoadCfg()
{ 
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "Attachment", 1);     taskbar_cfg.Attachment = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "SoftenUp", 1);       taskbar_cfg.SoftenUp = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "SoftenDown", 1);     taskbar_cfg.SoftenDown = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "MinLeftButton", 1);  taskbar_cfg.MinLeftButton = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "MinRightButton", 1); taskbar_cfg.MinRightButton = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "Clock", 1);          taskbar_cfg.Clock = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "CpuUsage", 1);       taskbar_cfg.CpuUsage = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "ChangeLang", 1);     taskbar_cfg.ChangeLang = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "MenuButton", 1);     taskbar_cfg.MenuButton = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "PanelHeight", 18);    taskbar_cfg.PanelHeight = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "SoftenHeight", 4);    taskbar_cfg.SoftenHeight = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "ButtonTopOffset", 3); taskbar_cfg.ButtonOffset = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "ButtonBotOffset", 3); taskbar_cfg.ButtonOffset = EAX;

	ini_get_int stdcall (#docky_ini_path, "@", "location", 0);  docky_cfg.location = EAX;
	ini_get_int stdcall (#docky_ini_path, "@", "fsize", 0);     docky_cfg.fsize = EAX;
	ini_get_int stdcall (#docky_ini_path, "@", "ashow", 0);     docky_cfg.ashow = EAX;
}

void SaveCfg(byte panel_type)
{
	if (panel_type==TASKBAR) {
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "Attachment", taskbar_cfg.Attachment);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "SoftenUp", taskbar_cfg.SoftenUp);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "SoftenDown", taskbar_cfg.SoftenDown);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "MinLeftButton", taskbar_cfg.MinLeftButton);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "MinRightButton", taskbar_cfg.MinRightButton);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "RunApplButton", taskbar_cfg.RunApplButton);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "ClnDeskButton", taskbar_cfg.ClnDeskButton);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "Clock", taskbar_cfg.Clock);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "CpuUsage", taskbar_cfg.CpuUsage);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "ChangeLang", taskbar_cfg.ChangeLang);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "MenuButton", taskbar_cfg.MenuButton);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "PanelHeight", taskbar_cfg.PanelHeight);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "SoftenHeight", taskbar_cfg.SoftenHeight);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "ButtonTopOffset", taskbar_cfg.ButtonOffset);
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_variables, "ButtonBottOffset", taskbar_cfg.ButtonOffset);
	}
	if (panel_type==DOCKY) {
		ini_set_int stdcall (#taskbar_ini_path, #taskbar_c_flags, "Attachment", taskbar_cfg.Attachment);
		ini_set_int stdcall (#docky_ini_path, "@", "location", docky_cfg.location);
		ini_set_int stdcall (#docky_ini_path, "@", "fsize", docky_cfg.fsize);
		ini_set_int stdcall (#docky_ini_path, "@", "ashow", docky_cfg.ashow);
	}
}

void RestartProcess(byte panel_type)
{
	dword proc_name1;
	if (panel_type == TASKBAR)
	{
		RestartProcessByName("@taskbar", SINGLE);
		pause(50);
	}
	else
	{
		RestartProcessByName("@docky", SINGLE);
		pause(120);
	}
	GetProcessInfo(#Form, SelfInfo);
	ActivateWindow(GetProcessSlot(Form.ID));
}



void PanelCfg_CheckBox(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, system.color.work_graph, system.color.work_text, value);
}

void PanelCfg_MoreLessBox(dword x, y, id_more, id_less; byte value; dword text) {
	MoreLessBox(x, y, 18, id_more, id_less, #system.color, value, text);
}


stop: