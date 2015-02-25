
#define MEMSIZE 0x23E80
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"
#include "..\lib\dll.h"
#include "..\lib\figures.h"

#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
#include "..\lib\lib.obj\libini.h"
#include "..\lib\lib.obj\box_lib.h"

#include "..\lib\patterns\libimg_load_skin.h"

#define WINDOW_TITLE "System panels configuration v0.8"

frame taskbar_frame = { 0, 100, 10, 152, 14, 0x000111, 0xFFFfff, 1, " Taskbar ", 0, 0, 6, 0x000111, 0xCCCccc };
frame docky_frame = { 0, 100, 10, 98, 183, 0x000111, 0xFFFfff, 1, " Docky ", 0, 0, 6, 0x000111, 0xCCCccc };

char taskbar_ini_path[] = "/sys/settings/taskbar.ini";
char taskbar_category[] = "Flags";
char docky_ini_path[] = "/sys/settings/docky.ini";


system_colors sc;
proc_info Form;
mouse m;
libimg_image panels_image;

struct docky_cfg {
	word fsize;
	byte location, ashow;
} docky_cfg;

struct taskbar_cfg {
	byte Attachment;
	byte PanelHeigh;
	byte SoftenUp, SoftenDown, MinLeftButton, MinRightButton, MenuButton,
	     RunApplButton, ClnDeskButton, Clock, CpuUsage, ChangeLang;
} taskbar_cfg;



void main()
{
	dword id, key;

	mem_Init();
	if (load_dll2(libio,  #libio_init,1)!=0) notify("Error: library doesn't exists - libio");
	if (load_dll2(libimg, #libimg_init,1)!=0) notify("Error: library doesn't exists - libimg");
	if (load_dll2(libini, #lib_init,1)!=0) notify("Error: library doesn't exists - libini");
	if (load_dll2(boxlib, #box_lib_init,0)!=0) notify("Eror: library doesn't exists - boxlib");

	Libimg_LoadImage(#panels_image, abspath("panels_cfg.png"));

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
					DrawWindowContent();
					SaveCfg();
					RestartProcess("@taskbar");
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
					DrawWindowContent();
					SaveCfg();
					RestartProcess("@docky");
				}
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				break;
			
		case evReDraw:
				sc.get();
				DefineAndDrawWindow(130, 150, 400, 300+GetSkinHeight(),0x34,sc.work,WINDOW_TITLE);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) break;
				taskbar_frame.size_x = docky_frame.size_x = - taskbar_frame.start_x * 2 + Form.cwidth;
				taskbar_frame.font_color = docky_frame.font_color = sc.work_text;
				taskbar_frame.font_backgr_color = docky_frame.font_backgr_color = sc.work;
				DrawWindowContent();
	}
}


void DrawWindowContent() 
{
	word win_center_x;

  frame_draw stdcall (#taskbar_frame);
	DefineButton(22, taskbar_frame.start_y + 12, panels_image.w-1, 27-1, 100 + BT_HIDE, 0);
	img_draw stdcall(panels_image.image, 22, taskbar_frame.start_y + 12, panels_image.w, 27, 0, taskbar_cfg.Attachment * 27);
	WriteText(68, taskbar_frame.start_y + 20, 0x80, 0x333222, "Click on image to change position");
	PanelCfgCheckBox(22, taskbar_frame.start_y +  48, 105, "Soften Up", taskbar_cfg.SoftenUp);
	PanelCfgCheckBox(22, taskbar_frame.start_y +  68, 106, "Soften Down", taskbar_cfg.SoftenDown);
	PanelCfgCheckBox(22, taskbar_frame.start_y +  88, 107, "Min Left Button", taskbar_cfg.MinLeftButton);
	PanelCfgCheckBox(22, taskbar_frame.start_y + 108, 108, "Min Right Button", taskbar_cfg.MinRightButton);
	PanelCfgCheckBox(22, taskbar_frame.start_y + 128, 109, "Run Application Button", taskbar_cfg.RunApplButton);
	win_center_x = Form.cwidth * 55 / 100;
	PanelCfgCheckBox(win_center_x, taskbar_frame.start_y +  48, 110, "ClnDeskButton - wtf?", taskbar_cfg.ClnDeskButton);
	PanelCfgCheckBox(win_center_x, taskbar_frame.start_y +  68, 111, "Clock", taskbar_cfg.Clock);
	PanelCfgCheckBox(win_center_x, taskbar_frame.start_y +  88, 112, "Cpu Usage", taskbar_cfg.CpuUsage);
	PanelCfgCheckBox(win_center_x, taskbar_frame.start_y + 108, 113, "Change Language", taskbar_cfg.ChangeLang);
	PanelCfgCheckBox(win_center_x, taskbar_frame.start_y + 128, 114, "Menu Button", taskbar_cfg.MenuButton);	
	//PanelCfgCheckBox(22, taskbar_frame.start_y + 64, 204, "111", taskbar_cfg.PanelHeigh);

  frame_draw stdcall (#docky_frame);
	DefineButton(22, docky_frame.start_y + 12, panels_image.w-1, 27-1, 200 + BT_HIDE, 0);
	img_draw stdcall(panels_image.image, 22, docky_frame.start_y + 12, panels_image.w, 27, 0, docky_cfg.location * 27 + 27);
	WriteText(68, docky_frame.start_y + 20, 0x80, 0x333222, "Click on image to change position");

	PanelCfgCheckBox(22, docky_frame.start_y + 48, 201, "Full width",  docky_cfg.fsize);
	PanelCfgCheckBox(22, docky_frame.start_y + 70, 202, "Always show", docky_cfg.ashow);
}

void LoadCfg()
{ 
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "Attachment", 1);     taskbar_cfg.Attachment = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "PanelHeigh", 18);    taskbar_cfg.PanelHeigh = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "SoftenUp", 1);       taskbar_cfg.SoftenUp = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "SoftenDown", 1);     taskbar_cfg.SoftenDown = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "MinLeftButton", 1);  taskbar_cfg.MinLeftButton = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "MinRightButton", 1); taskbar_cfg.MinRightButton = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "RunApplButton", 1);  taskbar_cfg.RunApplButton = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "ClnDeskButton", 1);  taskbar_cfg.ClnDeskButton = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "Clock", 1);          taskbar_cfg.Clock = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "CpuUsage", 1);       taskbar_cfg.CpuUsage = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "ChangeLang", 1);     taskbar_cfg.ChangeLang = EAX;
	ini_get_int stdcall (#taskbar_ini_path, #taskbar_category, "MenuButton", 1);     taskbar_cfg.MenuButton = EAX;

	ini_get_int stdcall (#docky_ini_path, "@", "location", 0);  docky_cfg.location = EAX;
	ini_get_int stdcall (#docky_ini_path, "@", "fsize", 0);     docky_cfg.fsize = EAX;
	ini_get_int stdcall (#docky_ini_path, "@", "ashow", 0);     docky_cfg.ashow = EAX;
}

void SaveCfg()
{
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "Attachment", taskbar_cfg.Attachment);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "PanelHeigh", taskbar_cfg.PanelHeigh);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "SoftenUp", taskbar_cfg.SoftenUp);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "SoftenDown", taskbar_cfg.SoftenDown);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "MinLeftButton", taskbar_cfg.MinLeftButton);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "MinRightButton", taskbar_cfg.MinRightButton);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "RunApplButton", taskbar_cfg.RunApplButton);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "ClnDeskButton", taskbar_cfg.ClnDeskButton);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "Clock", taskbar_cfg.Clock);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "CpuUsage", taskbar_cfg.CpuUsage);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "ChangeLang", taskbar_cfg.ChangeLang);
	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "MenuButton", taskbar_cfg.MenuButton);

	ini_set_int stdcall (#taskbar_ini_path, #taskbar_category, "Attachment", taskbar_cfg.Attachment);
	ini_set_int stdcall (#docky_ini_path, "@", "location", docky_cfg.location);
	ini_set_int stdcall (#docky_ini_path, "@", "fsize", docky_cfg.fsize);
	ini_set_int stdcall (#docky_ini_path, "@", "ashow", docky_cfg.ashow);
}

void RestartProcess(dword proc_name)
{
	int i, slot;
	proc_info Process;
	for (i=0; i<1000; i++;)
	{
		GetProcessInfo(#Process, i);
		if (strcmpi(#Process.name, proc_name)==0) { KillProcess(Process.ID); break; }
	}
	RunProgram(proc_name, "");

	pause(20);
	GetProcessInfo(#Form, SelfInfo);
	slot = GetProcessSlot(Form.ID);
	ActivateWindow(slot);
}

void PanelCfgCheckBox(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, sc.work_graph, sc.work_text, value);
}

stop: