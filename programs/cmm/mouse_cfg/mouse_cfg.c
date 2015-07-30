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

#include "kos_mouse_functions.h"

#ifdef LANG_RUS
	?define WINDOW_TITLE "Проверка и настройка параметров мыши"
	?define CHECK_MOUSE_1 "Нажмите на этой области"
	?define CHECK_MOUSE_2 "для проверки кнопок мыши"
	?define POINTER_SPEED "Скорость указателя мыши"
	?define POINTER_DELAY "Задержка указателя мыши"
	?define MOUSE_EMULATION "Эмуляция управления указателем через клавиатуру"
	?define MADMOUSE "Скрозные для курсора стороны экрана"
	//?define MADMOUSE_DESCRIPTION "'When cursor reaches screen side switch it to inverce side' -I"
#else
	?define WINDOW_TITLE "Mouse testing and configuration"
	?define CHECK_MOUSE_1 "Click on this area to"
	?define CHECK_MOUSE_2 "check your mouse buttons"
	?define POINTER_SPEED "Mouse pointer speed"
	?define POINTER_DELAY "Mouse pointer delay"
	?define MOUSE_EMULATION "Enable mouse emulation using keyboard NumPad"
	?define MADMOUSE "Through screen sides for pointer"
	//?define MADMOUSE_DESCRIPTION "'When cursor reaches screen side switch it to inverce side' -I"
#endif

frame mouse_frame = { 0, 000, 14, 130, 14, 0x000111, 0xFFFfff, 0, 0, 0, 0, 6, 0x000111, 0xCCCccc };


unsigned char panels_img_data[] = FROM "mouse_image.raw";
raw_image panels_img = { 59, 101, #panels_img_data };

system_colors sc;
proc_info Form;


struct mouse_cfg1 {
	char pointer_speed, 
	pointer_delay,
	emulation, 
	madmouse, 
	button_clicked;
} mouse_cfg;


void main() {
	char id, old_button_clicked;
	mouse m;

	mem_Init();
	load_dll(boxlib, #box_lib_init,0);

	LoadCfg();

	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evMouse:
				m.get();
				if (m.y <= mouse_frame.start_y) || (m.y >= mouse_frame.start_y + mouse_frame.size_y) 
				|| (m.x >= mouse_frame.start_x + mouse_frame.size_x) || (m.x <= mouse_frame.start_x) break;
				old_button_clicked = mouse_cfg.button_clicked;
				if (m.lkm) mouse_cfg.button_clicked=1;
				else if (m.pkm) mouse_cfg.button_clicked=2;
				else if (m.mkm) mouse_cfg.button_clicked=3;
				else mouse_cfg.button_clicked=0;
				if (mouse_cfg.button_clicked != old_button_clicked) DrawMouseImage();
				break;

		case evButton: 
				id=GetButtonID();
				if (id==1)
				{
					ExitProcess();
				}
				if (id==99) 
				{
					mouse_cfg.button_clicked=0;
					DrawMouseImage();
					break;
				}
				if (id==100)
				{
					if (mouse_cfg.emulation==true) KillProcessByName("mousemul", SINGLE);
					else RunProgram("/sys/mousemul", 0);
					mouse_cfg.emulation ^= 1;
				}
				if (id==101) 
				{
					if (mouse_cfg.madmouse==true) KillProcessByName("madmouse", SINGLE);
					else RunProgram("/sys/madmouse", 0);
					mouse_cfg.madmouse ^= 1;
				}
				if (id==120) 
				{
					mouse_cfg.pointer_speed++;
					SetMouseSpeed(mouse_cfg.pointer_speed);
				}
				if (id==121) && (mouse_cfg.pointer_speed>0)
				{
					mouse_cfg.pointer_speed--;
					SetMouseSpeed(mouse_cfg.pointer_speed);
				}
				if (id==122)
				{
					mouse_cfg.pointer_delay++;
					SetMouseDelay(mouse_cfg.pointer_delay);
				}
				if (id==123) && (mouse_cfg.pointer_delay>0)
				{
					mouse_cfg.pointer_delay--;
					SetMouseDelay(mouse_cfg.pointer_delay);
				}
				DrawWindowContent();
				break;
				
		case evKey:
				if (GetKey()==27) ExitProcess();
				break;
			
		case evReDraw:
				sc.get();
				DefineAndDrawWindow(430, 150, 360, 280+GetSkinHeight(),0x34,sc.work,WINDOW_TITLE);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) break;
				mouse_frame.size_x = - mouse_frame.start_x * 2 + Form.cwidth;
				mouse_frame.font_color = sc.work_text;
				mouse_frame.font_backgr_color = sc.work;
				mouse_frame.ext_col = sc.work_graph;
				DrawWindowContent();
	}
}


void DrawWindowContent() {
	char pos_x = 22;

	DefineButton(mouse_frame.start_x, mouse_frame.start_y, mouse_frame.size_x, mouse_frame.size_y, 99+BT_NOFRAME, 0xF0F2F3); //needed to handle mouse_up and refresh mouse image
	frame_draw stdcall (#mouse_frame);
	DrawMouseImage();
	WriteTextB(pos_x + 110, mouse_frame.start_y + 25, 0x90, 0x2C343C, CHECK_MOUSE_1);
	WriteTextB(pos_x + 110, mouse_frame.start_y + 45, 0x90, 0x2C343C, CHECK_MOUSE_2);

	PanelCfg_MoreLessBox(pos_x, mouse_frame.start_y + 142, 120, 121, mouse_cfg.pointer_speed, POINTER_SPEED);
	PanelCfg_MoreLessBox(pos_x, mouse_frame.start_y + 170, 122, 123, mouse_cfg.pointer_delay, POINTER_DELAY);

	PanelCfg_CheckBox(pos_x, mouse_frame.start_y + 202, 100, MOUSE_EMULATION, mouse_cfg.emulation);
	PanelCfg_CheckBox(pos_x, mouse_frame.start_y + 226, 101, MADMOUSE, mouse_cfg.madmouse);
}


void PanelCfg_CheckBox(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, sc.work_graph, sc.work_text, value);
}


void PanelCfg_MoreLessBox(dword x, y, id_more, id_less; byte value; dword text) {
	MoreLessBox(x, y, 18, id_more, id_less, #sc, value, text);
}

void DrawMouseImage() {
	_PutImage(mouse_frame.start_x+30, mouse_frame.start_y + 15,  panels_img.w, panels_img.h, mouse_cfg.button_clicked * panels_img.w * panels_img.h * 3 + panels_img.data);
}

void LoadCfg() {
	mouse_cfg.pointer_delay = GetMouseDelay();
	mouse_cfg.pointer_speed = GetMouseSpeed();
	mouse_cfg.madmouse = CheckProcessExists("MADMOUSE");
	mouse_cfg.emulation = CheckProcessExists("MOUSEMUL");
}



stop: