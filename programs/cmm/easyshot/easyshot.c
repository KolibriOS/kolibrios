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

#define T_WTITLE "EasyShot v0.7"

#ifdef LANG_RUS
	?define T_TAKE_SCREENSHOT "  Сделать скриншот"
	?define T_SAVE "  Сохранить"
	?define T_PREVIEW "Предпросмотр"
#else
	?define T_TAKE_SCREENSHOT "  Take a screenshot"
	?define T_SAVE "  Save"
	?define T_PREVIEW "Preview"
#endif

/* === DATA === */

proc_info Form;

dword screenshot,
      preview;

int screenshot_length,
    preview_length;

enum {
	BTN_MAKE_SCREENSHOT=10,
	BTN_SAVE,
	BTN_SETTINGS
};

#define PD 18 //padding
#define TOOLBAR_H 20+PD

block pw;

struct _settings {
	bool minimise;
	int delay;
	char save_path[4096];
} settings = { true, 1, "/tmp0/1" };

/* === CODE === */

void main()
{	
	char id;

	load_dll(libio,  #libio_init,  1);
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);

	Libimg_LoadImage(#skin, "/sys/icons16.png");

	screenshot_length = screen.width * screen.height * 3;
	pw.set_size(PD, TOOLBAR_H+PD, screen.width/2, screen.height/2);
	preview_length = screenshot_length / 2;	

	screenshot  = malloc(screenshot_length);
	preview = malloc(screenshot_length/2);

	loop() switch(WaitEvent())
	{
	case evButton:
		id = GetButtonID();
		if (id == CLOSE_BTN) ExitProcess();
		if (id == BTN_MAKE_SCREENSHOT) EventTakeScreenshot();
		if (id == BTN_SAVE) EventSaveImageFile();
		if (id == BTN_SETTINGS) EventShowSettings();
		break;

	case evKey:
		GetKeys();
		if (SCAN_CODE_KEY_S == key_scancode) EventSaveImageFile();
		if (SCAN_CODE_ENTER == key_scancode) EventTakeScreenshot();
		break;
     
	case evReDraw:
		DefineAndDrawWindow(screen.width/4, screen.height/4, pw.w + 9 +PD+PD, 
			pw.h + skin_height + TOOLBAR_H + 4 +PD+PD, 0x74, 0, T_WTITLE,0);
		GetProcessInfo(#Form, SelfInfo);
		if (Form.status_window>2) break;
		system.color.get();
		DrawMainContent();
	}
}

void DrawMainContent()
{
	int take_scr_btn_width;
	DrawBar(0, 0, Form.cwidth, TOOLBAR_H, system.color.work);
	DrawWideRectangle(0, TOOLBAR_H, pw.w+PD+PD, pw.h+PD+PD, PD-1, system.color.work);
	DrawRectangle(pw.x-1, pw.y-1, pw.w+1, pw.h+1, system.color.work_graph);
	take_scr_btn_width = DrawIconButton(pw.x, pw.y-42, BTN_MAKE_SCREENSHOT, T_TAKE_SCREENSHOT, 44);
	if (ESDWORD[preview]==0) {
		DrawBar(pw.x, pw.y, pw.w, pw.h, 0xEEEeee);
		WriteText(Form.cwidth-calc(strlen(T_PREVIEW)*8)/2, pw.h/2+pw.y, 0x90, 0x777777, T_PREVIEW);
	}
	else {
		_PutImage(pw.x, pw.y, pw.w, pw.h, preview);
		DrawIconButton(take_scr_btn_width + pw.x, pw.y-42, BTN_SAVE, T_SAVE, 5);
	}
	DrawIconButton(Form.cwidth-40-PD, pw.y-42, BTN_SETTINGS, " ", 10);	
}

void EventTakeScreenshot() {
	if (settings.minimise) MinimizeWindow(); 
	pause(settings.delay*100);
	CopyScreen(screenshot, 0, 0, screen.width, screen.height);
	ZoomImageTo50percent();
	ActivateWindow(GetProcessSlot(Form.ID));
	if (!settings.minimise) DrawMainContent();
}

void EventSaveImageFile()
{
	int i=0;
	char save_file_name[4096];
	do {
		i++;
		sprintf(#save_file_name, "%s/screen_%i.png", #settings.save_path, i);
	} while (file_exists(#save_file_name));
	save_image(screenshot, screen.width, screen.height, #save_file_name);
}

void EventShowSettings()
{
	CreateThread(#SettingsWindow,#settings_stak+4092);	
}

char path_tmp[4096];
dword mouse_dd1;
edit_box edit_box_path = {270,10,70,0xffffff,0x94AECE,0xFFFfff,0xffffff,0x10000000,sizeof(path_tmp),#path_tmp,#mouse_dd1, 0b};

void SettingsWindow()
{
	#define x 15
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
		if (10 == id) settings.delay++;
		if (11 == id) && (settings.delay>0) settings.delay--;
		if (12 == id) settings.minimise ^= 1;
		goto _DRAW_CONTENT;
		break;

	case evReDraw:
		DefineAndDrawWindow(Form.left+100, Form.top-40, 330, 170, 0x34, system.color.work, "Settings",0);
		_DRAW_CONTENT:
		CheckBox(x, 10, 12, "Minimize window", settings.minimise);
		MoreLessBox(x, 40, 10, 11, settings.delay, "Delay in seconds");
		//DrawEditBox(#edit_box_path);
	}
}

inline byte calc_rgb(dword B, item_h)
{
	return calc(ESBYTE[B+3] + ESBYTE[B] + ESBYTE[B-3]
		+ ESBYTE[B-item_h] + ESBYTE[B+item_h] / 5);
}

void ZoomImageTo50percent() {
	dword point_x = 0;
	dword item_h = screen.width * 3;
	dword small = preview;
	dword big = screenshot;

	while( (small <= preview + preview_length) && (big <= screenshot + screenshot_length ) ) {
		
		if (big <= screenshot + item_h) || (big >= screenshot + screenshot_length - item_h)
		{
			ESBYTE[small]   = ESBYTE[big];
			ESBYTE[small+1] = ESBYTE[big+1];
			ESBYTE[small+2] = ESBYTE[big+2];
		}
		else
		{
			ESBYTE[small]   = calc_rgb(big, item_h);
			ESBYTE[small+1] = calc_rgb(big+1, item_h);
			ESBYTE[small+2] = calc_rgb(big+2, item_h);
		}
	
		small+=3;
		big+=6;

		point_x+=2;
		if (point_x >= screen.width) 
		{
			big += item_h;
			point_x = 0;
		}
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