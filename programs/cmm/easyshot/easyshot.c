#define MEMSIZE 1024 * 50
#include "../lib/kolibri.h" 
#include "../lib/strings.h" 
#include "../lib/mem.h" 
#include "../lib/gui.h" 

#include "../lib/obj/libimg.h"
#include "../lib/obj/box_lib.h"

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

/* === TRANSLATIONS === */

#define T_WTITLE "EasyShot v0.76"

#ifdef LANG_RUS
	?define T_TAKE_SCREENSHOT "  Сделать скриншот"
#else
	?define T_TAKE_SCREENSHOT "  Take a screenshot"
#endif

/* === DATA === */	

proc_info Form;

dword screenshot;
int screenshot_length;

enum {
	BTN_MAKE_SCREENSHOT=10,
	BTN_SETTINGS
};

#define PD 18 //padding


char save_path[4096] = "/tmp0/1";
dword mouse_dd1;
edit_box edit_box_path = {270,10,70,0xffffff,0x94AECE,0xFFFfff,0xffffff,
	0x10000000,sizeof(save_path),#save_path,#mouse_dd1, 0b};

more_less_box delay = { 1, 0, 64, "Delay in seconds" };
checkbox minimise = { "Minimize window", true };


/* === CODE === */

void main()
{	
	char id;

	load_dll(libio,  #libio_init,  1);
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);

	Libimg_LoadImage(#skin, "/sys/icons16.png");
	screenshot_length = screen.width * screen.height * 3;
	screenshot = malloc(screenshot_length);

	loop() switch(WaitEvent())
	{
	case evButton:
		id = GetButtonID();
		if (id == CLOSE_BTN) ExitProcess();
		if (id == BTN_MAKE_SCREENSHOT) EventTakeScreenshot();
		if (id == BTN_SETTINGS) CreateThread(#SettingsWindow,#settings_stak+4092);
		break;

	case evKey:
		GetKeys();
		if (SCAN_CODE_ENTER == key_scancode) EventTakeScreenshot();
		break;
     
	case evReDraw:
		system.color.get();
		DefineAndDrawWindow(screen.width/4, screen.height-100/3, 270, 
			skin_height + 27+PD+PD, 0x34, system.color.work, T_WTITLE,0);
		GetProcessInfo(#Form, SelfInfo);
		if (Form.status_window>2) break;
		DrawMainContent();
	}
}

void DrawMainContent()
{
	int take_scr_btn_width;
	take_scr_btn_width = DrawIconButton(PD, PD, BTN_MAKE_SCREENSHOT, T_TAKE_SCREENSHOT, 44);
	DrawIconButton(PD+take_scr_btn_width, PD, BTN_SETTINGS, " ", 10);	
}

void EventTakeScreenshot() {
	if (minimise.checked) MinimizeWindow(); 
	pause(delay.value*100);
	CopyScreen(screenshot, 0, 0, screen.width, screen.height);
	ActivateWindow(GetProcessSlot(Form.ID));
	if (!minimise.checked) DrawMainContent();
	EventSaveImageFile();
}

void EventSaveImageFile()
{
	int i=0;
	char save_file_name[4096];
	do {
		i++;
		sprintf(#save_file_name, "%s/screen_%i.png", #save_path, i);
	} while (file_exists(#save_file_name));
	save_image(screenshot, screen.width, screen.height, #save_file_name);
}


void SettingsWindow()
{
	int id;
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);	
	loop() switch(WaitEvent())
	{
	case evMouse:
		//edit_box_mouse stdcall (#address_box);
		break;

	case evKey:
		GetKeys();
		if (SCAN_CODE_ESC == key_scancode) ExitProcess();
		break;

	case evButton:
		id = GetButtonID();
		if (CLOSE_BTN == id) ExitProcess();
		delay.click(id);
		minimise.click(id);
		break;

	case evReDraw:
		DefineAndDrawWindow(Form.left+100, Form.top-40, 330, 170, 0x34, system.color.work, "Settings",0);
		_DRAW_CONTENT:
		minimise.draw(15, 10);
		delay.draw(15, 40);
		//DrawEditBox(#edit_box_path);
	}
}

int DrawIconButton(dword x, y, id, text, icon)
{
	int btwidth;
	system.color.work_button = 0xFFFfff;
	system.color.work_button_text = 0;
	btwidth = DrawStandartCaptButton(x, y, id, text);
	img_draw stdcall(skin.image, x+12, y+5, 16, 16, 0, icon*16);
	system.color.get();
	return btwidth;
}

stop:

char settings_stak[4096];