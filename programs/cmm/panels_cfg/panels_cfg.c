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
#define PIMG_W 37
#define PIMG_H 27 //27*5

proc_info Form;

enum { BTN_TB_ATTACHEMENT=100, BTN_DOCKY_ATTACHEMENT=200 };
enum { TASKBAR, DOCKY };

more_less_box tbPanelHeight  = { NULL, 6, 99, PANEL_HEIGHT };
more_less_box tbSoftenHeight = { NULL, 0, 99, SOFTEN_HEIGHT };
more_less_box tbButtonOffset = { NULL, 0, 99, BUTTON_OFFSET };

int tbAttachment;
checkbox tbSoftenUp = { SOFTEN_UP };
checkbox tbClock = { CLOCK };
checkbox tbSoftenDown = { SOFTEN_DOWN };
checkbox tbCpuUsage = { CPU_USAGE };
checkbox tbMinLeftButton = { MIN_LEFT_BUTTON };
checkbox tbChangeLang = { CHANGE_LANG };
checkbox tbMinRightButton = { MIN_RIGHT_BUTTON };
checkbox tbMenuButton = { MENU_BUTTON };
checkbox dkFsize = { FSIZE };
checkbox dkAshow = { ASHOW };

int dkLocation;
checkbox tbRunApplButton;
checkbox tbClnDeskButton;


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
				if (1==id) ExitProcess();
				//taskbar buttons
				if (BTN_TB_ATTACHEMENT == id) {
					tbAttachment ^= 1;
					DrawWindowContent(); 
					SaveSettingsAndRestartProcess(TASKBAR);
				}

				if (tbSoftenUp.click(id))      || (tbSoftenDown.click(id))
				|| (tbMinLeftButton.click(id)) || (tbMinRightButton.click(id))
				|| (tbRunApplButton.click(id)) || (tbClnDeskButton.click(id)) {
					SaveSettingsAndRestartProcess(TASKBAR); 
				}

				if (tbClock.click(id))       || (tbCpuUsage.click(id))
				|| (tbChangeLang.click(id))  || (tbMenuButton.click(id))
				|| (tbPanelHeight.click(id)) || (tbSoftenHeight.click(id)) 
				|| (tbButtonOffset.click(id)) {
					SaveSettingsAndRestartProcess(TASKBAR);
				}
				//docky buttons
				if (id==200) {
					dkLocation++; 
					if (dkLocation>3) dkLocation=1; 
					DrawWindowContent(); 
					SaveSettingsAndRestartProcess(DOCKY);
				}
				if (dkFsize.click(id)) SaveSettingsAndRestartProcess(DOCKY);
				if (dkAshow.click(id)) SaveSettingsAndRestartProcess(DOCKY);
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
				DrawWindowContent();
	}
}

void DrawPanelsImage(dword y, n)
{
	_PutImage(22, y, PIMG_W, PIMG_H, n * PIMG_W * PIMG_H * 3 + #panels_img_data);
}

void DrawWindowContent()
{
	#define PD 10
	dword frame_y;
	word win_center_x = Form.cwidth / 2 + 20;
	incn y;

	frame_y = 15;
	y.n = frame_y;
	DefineButton(22, y.inc(18), PIMG_W-1, PIMG_H-1, 100 + BT_HIDE, 0);
	DrawPanelsImage(y.n, tbAttachment);
	WriteText(68, y.inc(7), 0x90, system.color.work_text, CHANGE_POS);
	tbSoftenUp.draw(22, y.inc(35));
	tbClock.draw(win_center_x, y.n);
	tbSoftenDown.draw(22, y.inc(24));
	tbCpuUsage.draw(win_center_x, y.n);
	tbMinLeftButton.draw(22, y.inc(24));
	tbChangeLang.draw(win_center_x, y.n);
	tbMinRightButton.draw(22, y.inc(24));
	tbMenuButton.draw(win_center_x, y.n);
	tbPanelHeight.draw(22, y.inc(28));
	tbSoftenHeight.draw(22, y.inc(32));
	tbButtonOffset.draw(22, y.inc(32));
	DrawFrame(PD, frame_y, Form.cwidth-PD-PD, y.inc(32)-frame_y, TASK_FRAME_T);
	//DOCKY
	frame_y = calc(y.inc(20));
	DefineButton(22, y.inc(18), PIMG_W-1, PIMG_H-1, 200 + BT_HIDE, 0);
	DrawPanelsImage(y.n, dkLocation+1);
	WriteText(68, y.inc(7), 0x90, system.color.work_text, CHANGE_POS);
	dkFsize.draw(22, y.inc(35)); 
	dkAshow.draw(win_center_x, y.n);
	DrawFrame(PD, frame_y, Form.cwidth-PD-PD, Form.cheight-frame_y-PD, DOCK_FRAME_T);
}

void LoadCfg()
{ 
	tbAttachment = taskbar_flags_ini.GetInt("Attachment", 1);    
	tbSoftenUp.checked       = taskbar_flags_ini.GetInt("SoftenUp", 1);      
	tbSoftenDown.checked     = taskbar_flags_ini.GetInt("SoftenDown", 1);    
	tbMinLeftButton.checked  = taskbar_flags_ini.GetInt("MinLeftButton", 1); 
	tbMinRightButton.checked = taskbar_flags_ini.GetInt("MinRightButton", 1);
	tbClock.checked          = taskbar_flags_ini.GetInt("Clock", 1);         
	tbCpuUsage.checked       = taskbar_flags_ini.GetInt("CpuUsage", 1);      
	tbChangeLang.checked     = taskbar_flags_ini.GetInt("ChangeLang", 1);    
	tbMenuButton.checked     = taskbar_flags_ini.GetInt("MenuButton", 1);    
	tbPanelHeight.value  = taskbar_vars_ini.GetInt("PanelHeight", 28);
	tbSoftenHeight.value = taskbar_vars_ini.GetInt("SoftenHeight", 4);   
	tbButtonOffset.value = taskbar_vars_ini.GetInt("ButtonTopOffset", 3);
	tbButtonOffset.value = taskbar_vars_ini.GetInt("ButtonBotOffset", 3);

	dkLocation = docky_ini.GetInt("location", 0);
	dkFsize.checked = docky_ini.GetInt("fsize", 0);   
	dkAshow.checked = docky_ini.GetInt("ashow", 0);   
}

void SaveSettingsAndRestartProcess(byte panel_type)
{
	if (panel_type==TASKBAR) {
		taskbar_flags_ini.SetInt("Attachment", tbAttachment);
		taskbar_flags_ini.SetInt("SoftenUp", tbSoftenUp.checked);
		taskbar_flags_ini.SetInt("SoftenDown", tbSoftenDown.checked);
		taskbar_flags_ini.SetInt("MinLeftButton", tbMinLeftButton.checked);
		taskbar_flags_ini.SetInt("MinRightButton", tbMinRightButton.checked);
		taskbar_flags_ini.SetInt("RunApplButton", tbRunApplButton.checked);
		taskbar_flags_ini.SetInt("ClnDeskButton", tbClnDeskButton.checked);
		taskbar_flags_ini.SetInt("Clock", tbClock.checked);
		taskbar_flags_ini.SetInt("CpuUsage", tbCpuUsage.checked);
		taskbar_flags_ini.SetInt("ChangeLang", tbChangeLang.checked);
		taskbar_flags_ini.SetInt("MenuButton", tbMenuButton.checked);
		taskbar_vars_ini.SetInt("PanelHeight", tbPanelHeight.value);
		taskbar_vars_ini.SetInt("SoftenHeight", tbSoftenHeight.value);
		taskbar_vars_ini.SetInt("ButtonTopOffset", tbButtonOffset.value);
		taskbar_vars_ini.SetInt("ButtonBottOffset", tbButtonOffset.value);
	}
	if (panel_type==DOCKY) {
		docky_ini.SetInt("location", dkLocation);
		docky_ini.SetInt("fsize", dkFsize.checked);
		docky_ini.SetInt("ashow", dkAshow.checked);
	}
	RestartProcess(panel_type);
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