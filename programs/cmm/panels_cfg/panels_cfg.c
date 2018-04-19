#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define MEMSIZE 0x23E80
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\fs.h"
#include "..\lib\dll.h"
#include "..\lib\gui.h"

#include "..\lib\obj\libio.h"
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

char taskbar_ini_path[] = "/sys/settings/taskbar.ini";
_ini taskbar_flags_ini = { #taskbar_ini_path, "Flags" };
_ini taskbar_vars_ini = { #taskbar_ini_path, "Variables" };

_ini docky_ini = { "/sys/settings/docky.ini", "@" };

unsigned char panels_img_data[] = FROM "panels_image.raw";
raw_image panels_img = { 37, 27, #panels_img_data };

proc_info Form;

word dkFsize;
byte dkLocation, dkAshow;

byte tbAttachment, tbSoftenUp, tbSoftenDown, tbMinLeftButton, tbMinRightButton,
tbMenuButton, tbRunApplButton, tbClnDeskButton, tbClock, tbCpuUsage, tbChangeLang;

enum {
	TASKBAR,
	DOCKY,
	ALL
};

more_less_box tbPanelHeight  = { NULL, 6, 99, PANEL_HEIGHT };
more_less_box tbSoftenHeight = { NULL, 0, 99, SOFTEN_HEIGHT };
more_less_box tbButtonOffset = { NULL, 0, 99, BUTTON_OFFSET };

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
					if (id==100) tbAttachment ^= 1;
					if (id==105) tbSoftenUp ^= 1;
					if (id==106) tbSoftenDown ^= 1;
					if (id==107) tbMinLeftButton ^= 1;
					if (id==108) tbMinRightButton ^= 1;
					if (id==109) tbRunApplButton ^= 1;
					if (id==110) tbClnDeskButton ^= 1;
					if (id==111) tbClock ^= 1;
					if (id==112) tbCpuUsage ^= 1;
					if (id==113) tbChangeLang ^= 1;
					if (id==114) tbMenuButton ^= 1;
					tbPanelHeight.click(id);
					tbSoftenHeight.click(id);
					tbButtonOffset.click(id);
					DrawWindowContent(TASKBAR);
					SaveCfg(TASKBAR);
					RestartProcess(TASKBAR);
				}
				//docky buttons			
				if (id>=200)
				{
					if (id==200)
					{
						dkLocation++;
						if (dkLocation>3) dkLocation=1;
					}
					if (id==201) dkFsize ^= 1;
					if (id==202) dkAshow ^= 1;
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
				DefineAndDrawWindow(130, 150, 465, 398 + skin_height, 0x34, system.color.work, WINDOW_TITLE, 0);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) break;
				DrawWindowContent(ALL);
	}
}

void DrawWindowContent(byte panel_type)
{
	#define PD 10
	dword frame_y;
	word win_center_x = Form.cwidth / 2 + 20;
	incn y;

	if (panel_type==ALL) || (panel_type==TASKBAR)
	{
		frame_y = 15;
		y.n = frame_y;
		DefineButton(22, y.inc(18), panels_img.w-1, 27-1, 100 + BT_HIDE, 0);
		_PutImage(22, y.n, 37, 27, tbAttachment * 37 * 27 * 3 + panels_img.data);
		WriteText(68, y.inc(7), 0x90, system.color.work_text, CHANGE_POS);
		CheckBox(22, y.inc(35), 105, SOFTEN_UP, tbSoftenUp);
		CheckBox(win_center_x, y.n, 111, CLOCK, tbClock);
		CheckBox(22, y.inc(24), 106, SOFTEN_DOWN, tbSoftenDown);
		CheckBox(win_center_x, y.n, 112, CPU_USAGE, tbCpuUsage);
		CheckBox(22, y.inc(24), 107, MIN_LEFT_BUTTON, tbMinLeftButton);
		CheckBox(win_center_x, y.n, 113, CHANGE_LANG, tbChangeLang);
		CheckBox(22, y.inc(24), 108, MIN_RIGHT_BUTTON, tbMinRightButton);
		CheckBox(win_center_x, y.n, 114, MENU_BUTTON, tbMenuButton);	
		tbPanelHeight.draw(22, y.inc(28));
		tbSoftenHeight.draw(22, y.inc(32));
		tbButtonOffset.draw(22, y.inc(32));
		DrawFrame(PD, frame_y, Form.cwidth-PD-PD, y.inc(32)-frame_y, TASK_FRAME_T);
	}
	if (panel_type==ALL) || (panel_type==DOCKY)
	{
		frame_y = calc(y.inc(20));
		DefineButton(22, y.inc(18), panels_img.w-1, 27-1, 200 + BT_HIDE, 0);
		_PutImage(22, y.n,  37, 27, dkLocation + 1 * 37 * 27 * 3 + panels_img.data);
		WriteText(68, y.inc(7), 0x90, system.color.work_text, CHANGE_POS);
		CheckBox(22, y.inc(35), 201, FSIZE,  dkFsize);
		CheckBox(win_center_x, y.n, 202, ASHOW, dkAshow);
		DrawFrame(PD, frame_y, Form.cwidth-PD-PD, Form.cheight-frame_y-PD, DOCK_FRAME_T);
	}
}

void LoadCfg()
{ 
	tbAttachment     = taskbar_flags_ini.GetInt("Attachment", 1);    
	tbSoftenUp       = taskbar_flags_ini.GetInt("SoftenUp", 1);      
	tbSoftenDown     = taskbar_flags_ini.GetInt("SoftenDown", 1);    
	tbMinLeftButton  = taskbar_flags_ini.GetInt("MinLeftButton", 1); 
	tbMinRightButton = taskbar_flags_ini.GetInt("MinRightButton", 1);
	tbClock          = taskbar_flags_ini.GetInt("Clock", 1);         
	tbCpuUsage       = taskbar_flags_ini.GetInt("CpuUsage", 1);      
	tbChangeLang     = taskbar_flags_ini.GetInt("ChangeLang", 1);    
	tbMenuButton     = taskbar_flags_ini.GetInt("MenuButton", 1);    
	tbPanelHeight.value  = taskbar_vars_ini.GetInt("PanelHeight", 28);
	tbSoftenHeight.value = taskbar_vars_ini.GetInt("SoftenHeight", 4);   
	tbButtonOffset.value = taskbar_vars_ini.GetInt("ButtonTopOffset", 3);
	tbButtonOffset.value = taskbar_vars_ini.GetInt("ButtonBotOffset", 3);

	dkLocation = docky_ini.GetInt("location", 0);
	dkFsize = docky_ini.GetInt("fsize", 0);   
	dkAshow = docky_ini.GetInt("ashow", 0);   
}

void SaveCfg(byte panel_type)
{
	if (panel_type==TASKBAR) {
		taskbar_flags_ini.SetInt("Attachment", tbAttachment);
		taskbar_flags_ini.SetInt("SoftenUp", tbSoftenUp);
		taskbar_flags_ini.SetInt("SoftenDown", tbSoftenDown);
		taskbar_flags_ini.SetInt("MinLeftButton", tbMinLeftButton);
		taskbar_flags_ini.SetInt("MinRightButton", tbMinRightButton);
		taskbar_flags_ini.SetInt("RunApplButton", tbRunApplButton);
		taskbar_flags_ini.SetInt("ClnDeskButton", tbClnDeskButton);
		taskbar_flags_ini.SetInt("Clock", tbClock);
		taskbar_flags_ini.SetInt("CpuUsage", tbCpuUsage);
		taskbar_flags_ini.SetInt("ChangeLang", tbChangeLang);
		taskbar_flags_ini.SetInt("MenuButton", tbMenuButton);
		taskbar_vars_ini.SetInt("PanelHeight", tbPanelHeight.value);
		taskbar_vars_ini.SetInt("SoftenHeight", tbSoftenHeight.value);
		taskbar_vars_ini.SetInt("ButtonTopOffset", tbButtonOffset.value);
		taskbar_vars_ini.SetInt("ButtonBottOffset", tbButtonOffset.value);
	}
	if (panel_type==DOCKY) {
		docky_ini.SetInt("location", dkLocation);
		docky_ini.SetInt("fsize", dkFsize);
		docky_ini.SetInt("ashow", dkAshow);
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



stop: