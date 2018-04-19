// Mouse Configuration Utility ver 1.5

#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define MEMSIZE 0x23E80
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\fs.h"
#include "..\lib\gui.h"
#include "..\lib\obj\libio.h"
#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\libini.h"
#include "..\lib\patterns\restart_process.h"

// Translatiions
#ifdef LANG_RUS
	?define WINDOW_TITLE "Проверка и настройка параметров мыши"
	?define CHECK_MOUSE_1 "Нажмите на этой области"
	?define CHECK_MOUSE_2 "для проверки кнопок мыши"
	?define POINTER_SPEED "Делитель скорости указателя мыши"
	?define ACCELERATION_TEXT "Чувствительность указателя мыши"
	?define DOUBLE_CLICK_TEXT "Задержка двойного клика мышью"
	?define MOUSE_EMULATION "Управление указателем мыши через клавиатуру"
	?define MADMOUSE "Сквозные для курсора стороны экрана"
#else
	?define WINDOW_TITLE "Mouse testing and configuration"
	?define CHECK_MOUSE_1 "Click on this area to"
	?define CHECK_MOUSE_2 "check your mouse buttons"
	?define POINTER_SPEED "Mouse pointer speed divider"
	?define ACCELERATION_TEXT "Mouse pointer sensitivity"
	?define DOUBLE_CLICK_TEXT "Mouse double click delay"
	?define MOUSE_EMULATION "Enable mouse emulation using keyboard NumPad"
	?define MADMOUSE "Through screen sides for pointer"
#endif

proc_info Form;

#define PD 18 //padding
block mouse_frame = { PD, PD, NULL, 130 };

more_less_box pointer_speed      = { NULL, 0, 64, POINTER_SPEED };
more_less_box acceleration       = { NULL, 0, 64, ACCELERATION_TEXT };
more_less_box double_click_delay = { NULL, 0, 999, DOUBLE_CLICK_TEXT, 8 };

unsigned char panels_img_data[] = FROM "mouse_image.raw";
raw_image panels_img = { 59, 101, #panels_img_data };

_ini ini = { "/sys/settings/system.ini", "mouse" };

struct _mouse_cfg {
	bool emulation;
	bool madmouse;
	int click_status;
} mouse_cfg;


void main() {
	char id;
	
	load_dll(libini, #lib_init,1);
	load_dll(boxlib, #box_lib_init,0);
	
	LoadCfg();

	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);	

	loop() switch(WaitEvent())
	{
		case evMouse:
				mouse.get();
				if (mouse.down) && (mouse_cfg.click_status==0) && (mouse_frame.hovered()) {
					if (mouse.key&MOUSE_LEFT) mouse_cfg.click_status = 1;
					if (mouse.key&MOUSE_RIGHT) mouse_cfg.click_status = 2;
					if (mouse.key&MOUSE_CENTER) mouse_cfg.click_status = 3;
					DrawMouseImage();
				}
				if (mouse.up) {
					mouse_cfg.click_status=0;
					DrawMouseImage();
				}
				break;

		case evButton: 
				id = GetButtonID();
				switch (id) {
					case 1:
						ExitApp();
						break;
					case 100:
						if (mouse_cfg.emulation==true) KillProcessByName("mousemul", SINGLE);
						else RunProgram("/sys/mousemul", 0);
						mouse_cfg.emulation ^= 1;
						DrawControls();
						break;
					case 101: 
						if (mouse_cfg.madmouse==true) KillProcessByName("madmouse", SINGLE);
						else RunProgram("/sys/madmouse", 0);
						mouse_cfg.madmouse ^= 1;
						DrawControls();
						break;
					default:
						if (pointer_speed.click(id)) ApplyCfg();;
						if (acceleration.click(id)) ApplyCfg();;
						if (double_click_delay.click(id)) ApplyCfg();;
				}
				break;

		case evKey:
				GetKeys();
				if (key_scancode == SCAN_CODE_ESC) ExitApp();
				break;
			
		case evReDraw:
				system.color.get();
				DefineAndDrawWindow(430, 150, 424, 310+skin_height,0x34,system.color.work,WINDOW_TITLE,0);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) break;
				mouse_frame.w = - mouse_frame.x * 2 + Form.cwidth;
				DefineButton(mouse_frame.x, mouse_frame.y, mouse_frame.w, 
					mouse_frame.h, 99+BT_NOFRAME, 0xF0F2F3); //needed to handle mouse_up and refresh mouse image
				WriteText(mouse_frame.x + 110, mouse_frame.y + 25, 0x90, 0x2C343C, CHECK_MOUSE_1);
				WriteText(mouse_frame.x + 110, mouse_frame.y + 45, 0x90, 0x2C343C, CHECK_MOUSE_2);
				DrawMouseImage();
				DrawControls();
	}
}


void DrawMouseImage() {
	_PutImage(mouse_frame.x+30, mouse_frame.y + 15,  panels_img.w, panels_img.h, 
		mouse_cfg.click_status * panels_img.w * panels_img.h * 3 + panels_img.data);
}

void DrawControls() {
	pointer_speed.draw(PD, PD+142);
	acceleration.draw(PD, PD+172);
	double_click_delay.draw(PD+202);
	CheckBox(mouse_frame.x, mouse_frame.y + 236, 100, MOUSE_EMULATION, mouse_cfg.emulation);
	CheckBox(mouse_frame.x, mouse_frame.y + 262, 101, MADMOUSE, mouse_cfg.madmouse);
}

void LoadCfg() {
	acceleration.value = ini.GetInt("acceleration", GetMouseAcceleration());
	pointer_speed.value = ini.GetInt("speed", GetMouseSpeed());
	double_click_delay.value = ini.GetInt("double_click_delay", GetMouseDoubleClickDelay());
	mouse_cfg.madmouse = CheckProcessExists("MADMOUSE");
	mouse_cfg.emulation = CheckProcessExists("MOUSEMUL");
}

void ExitApp() {
	ini.SetInt("acceleration", acceleration.value);
	ini.SetInt("speed", pointer_speed.value);
	ini.SetInt("double_click_delay", double_click_delay.value);
	ExitProcess();
}

void ApplyCfg() {
	SetMouseSpeed(pointer_speed.value);
	SetMouseAcceleration(acceleration.value);
	SetMouseDoubleClickDelay(double_click_delay.value);
}

stop: