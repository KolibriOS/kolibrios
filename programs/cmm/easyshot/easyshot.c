#define MEMSIZE 1024 * 50
#include "../lib/kolibri.h" 
#include "../lib/strings.h" 
#include "../lib/mem.h" 
#include "../lib/gui.h" 

#include "../lib/obj/libimg.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

/* === TRANSLATIONS === */

#define T_WTITLE "EasyShot v1.1"

#ifdef LANG_RUS
	?define T_TAKE_SCREENSHOT "  Сделать скриншот"
	?define T_SETTINGS "Настройки"
	?define T_EDITBOX_FRAME " Путь сохранения скриншота "
	?define T_MINIMIZE "Свернуть окно при снимке"
	?define T_CONTINUOUS_SHOOTING "Continuous shooting"
	?define T_DELAY "Задержка в секундах"
	?define T_NO_DIR "'Папка не существует!' -E"
	?define T_SET_PATH "Задать"
#else
	?define T_TAKE_SCREENSHOT "  Take a screenshot"
	?define T_SETTINGS "Settings"
	?define T_EDITBOX_FRAME " Save path "
	?define T_MINIMIZE "Minimize window"
	?define T_CONTINUOUS_SHOOTING "Continuous shooting"
	?define T_DELAY "Delay in seconds"
	?define T_NO_DIR "'Directory does not exists!' -E"
	?define T_SET_PATH "Set"
#endif

/* === DATA === */	

proc_info Form;
proc_info Settings;

dword screenshot;
int screenshot_length;

enum {
	BTN_MAKE_SCREENSHOT=10,
	BTN_SETTINGS
};

#define PD 18 //padding

char save_path[4096];
char save_path_stable[4096];
char open_dir[4096];

edit_box edit_save = {250,25,100,0xffffff,0x94AECE,0xFFFfff,0xffffff,
	0x10000000,sizeof(save_path)-2,#save_path,0, 0b};

more_less_box delay = { 1, 0, 64, T_DELAY };
checkbox minimize = { T_MINIMIZE, true };
checkbox continuous_shooting = { T_CONTINUOUS_SHOOTING, true };


opendialog open_folder_dialog = 
{
  2, //0-file, 2-save, 3-select folder
  #Settings,
  #communication_area_name,
  0,
  0, //dword opendir_path,
  #open_dir, //dword dir_default_path,
  #open_dialog_path,
  #DrawSettingsWindow,
  0,
  #open_dir, //dword openfile_path,
  0, //dword filename_area,
  0, //dword filter_area,
  420,
  NULL,
  320,
  NULL
};

/* === CODE === */

void main()
{	
	int id;

	load_dll(libio,  #libio_init,  1);
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#open_folder_dialog);

	system.color.get();
	Libimg_LoadImage(#skin, "/sys/icons16.png");
	Libimg_ReplaceColor(skin.image, skin.w, skin.h, 0xffFFFfff, system.color.work_button);
	Libimg_ReplaceColor(skin.image, skin.w, skin.h, 0xffCACBD6, MixColors(system.color.work_button, 0, 200));
	screenshot_length = screen.width * screen.height * 3;
	screenshot = malloc(screenshot_length);

	strcpy(#save_path_stable, "/tmp0/1");
	strcpy(#save_path, #save_path_stable);
	edit_save.size = strlen(#save_path);

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
		DefineAndDrawWindow(screen.width/4, screen.height-100/3, 270, 
			skin_height + 27+PD+PD, 0x34, system.color.work, T_WTITLE,0);
		GetProcessInfo(#Form, SelfInfo);
		DrawMainContent();
	}
}

void DrawMainContent()
{
	int take_scr_btn_width;
	take_scr_btn_width = DrawIconButton(PD, PD, BTN_MAKE_SCREENSHOT, T_TAKE_SCREENSHOT, 45);
	DrawIconButton(PD+take_scr_btn_width, PD, BTN_SETTINGS, " ", 10);	
}

void EventTakeScreenshot() {
	if (minimize.checked) MinimizeWindow(); 
	pause(delay.value*100);
	CopyScreen(screenshot, 0, 0, screen.width, screen.height);
	ActivateWindow(GetProcessSlot(Form.ID));
	if (!minimize.checked) DrawMainContent();
	EventSaveImageFile();
}

void EventSaveImageFile()
{
	int i=0;
	char save_file_name[4096];
	do {
		i++;
		sprintf(#save_file_name, "%s/screen_%i.png", #save_path_stable, i);
	} while (file_exists(#save_file_name));
	save_image(screenshot, screen.width, screen.height, #save_file_name);
}


void SettingsWindow()
{
	#define BTN_OD 10
	#define BTN_SET 11
	int id, butw;
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
	case evMouse:
		edit_box_mouse stdcall (#edit_save);
		break;

	case evKey:
		GetKeys();
		if (SCAN_CODE_ESC == key_scancode) ExitProcess();
		EAX= key_ascii << 8;
		edit_box_key stdcall (#edit_save);	
		break;

	case evButton:
		id = GetButtonID();
		if (CLOSE_BTN == id) ExitProcess();
		if (BTN_OD == id) {
			OpenDialog_start stdcall (#open_folder_dialog);
			if (open_folder_dialog.status) {
				strcpy(#save_path, open_folder_dialog.opendir_path);
				edit_save.size = edit_save.pos = edit_save.shift 
					= edit_save.shift_old = strlen(#save_path);
			}
		}
		if (BTN_SET == id) {
			if (save_path[0]) && (dir_exists(#save_path)) {
				strcpy(#save_path_stable, #save_path);
				strrtrim(#save_path_stable);
				if (save_path_stable[strlen(#save_path_stable)-1]=='/')
				    save_path_stable[strlen(#save_path_stable)-1]=NULL; //no "/" at the end
			}
			else notify(T_NO_DIR);

		}
		delay.click(id);
		minimize.click(id);
		break;

	case evReDraw:
		DrawSettingsWindow();
	}
}

void DrawSettingsWindow()
{
	DefineAndDrawWindow(Form.left+100, Form.top-40, 400, 230, 0x34, system.color.work, T_SETTINGS, 0);
	GetProcessInfo(#Settings, SelfInfo);
	minimize.draw(15, 15);
	delay.draw(15, 45);
	DrawFrame(15, 85, 360, 95, T_EDITBOX_FRAME);
		DrawEditBoxPos(32, 110, #edit_save);
		DrawStandartCaptButton(edit_save.left + edit_save.width + 15, edit_save.top-3, BTN_OD, "...");
		DrawStandartCaptButton(edit_save.left, edit_save.top+32, BTN_SET, T_SET_PATH);	
}

int DrawIconButton(dword x, y, id, text, icon)
{
	int btwidth;
	btwidth = DrawStandartCaptButton(x, y, id, text);
	img_draw stdcall(skin.image, x+12, y+5, 16, 16, 0, icon*16);
	return btwidth;
}

stop:

char settings_stak[4096];