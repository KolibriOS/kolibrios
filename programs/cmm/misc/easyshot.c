#define MEMSIZE 1024 * 40
#include "../lib/kolibri.h" 
#include "../lib/strings.h" 
#include "../lib/mem.h" 
#include "../lib/gui.h" 

#include "../lib/obj/libimg.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"

/* === TRANSLATIONS === */

#ifdef LANG_RUS
	?define T_TAKE_SCREENSHOT "Сделать скриншот"
	?define T_SETTINGS "Настройки"
	?define T_EDITBOX_FRAME "Путь сохранения скриншота"
	?define T_DELAY "Задержка в секундах"
	?define T_NO_DIR "'Папка не существует!' -E"
#else
	?define T_TAKE_SCREENSHOT "Take a screenshot"
	?define T_SETTINGS "Settings"
	?define T_EDITBOX_FRAME "Save path"
	?define T_DELAY "Delay in seconds"
	?define T_NO_DIR "'Directory does not exists!' -E"
#endif

/* === DATA === */	

proc_info Form;

enum {
	BTN_MAKE_SCREENSHOT=10,
	BTN_SETTINGS,
	BTN_CHOOSE_SAVING_PATH
};

#define PD 18 //padding
#define SETTINGS_Y PD+PD+30+10

char save_path[4096];

more_less_box delay = { 1, 0, SETTINGS_Y, T_DELAY };
edit_box edit_save = {260,PD,SETTINGS_Y+50,0xffffff,0x94AECE,0xFFFfff,0xffffff,
	0x10000000,sizeof(save_path)-2,#save_path,0, 0b};

bool show_settings = false;

opendialog open_folder_dialog = 
{
  2, //0-file, 2-save, 3-select folder
  #Form,
  #communication_area_name,
  0,
  0, //dword opendir_path,
  #save_path, //dword dir_default_path,
  #open_dialog_path,
  #DrawWindow,
  0,
  #save_path, //dword openfile_path,
  0, //dword filename_area,
  0, //dword filter_area,
  420,
  NULL,
  320,
  NULL
};

/* === CODE === */

void init_libraries()
{
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#open_folder_dialog);	
}

void main()
{	
	int id;

	init_libraries();

	edit_box_set_text stdcall (#edit_save, "/tmp0/1");	

	@SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(@WaitEvent())
	{
	case evMouse:
		edit_box_mouse stdcall (#edit_save);
		break;

	case evButton:
		id = @GetButtonID();
		switch(id){
			case CLOSE_BTN: @ExitProcess();
			case BTN_MAKE_SCREENSHOT: EventTakeScreenshot(); break;
			case BTN_SETTINGS: EventClickSettings(); break;
			case BTN_CHOOSE_SAVING_PATH: EventChooseSavePath(); break;
			default: delay.click(id);
		}
		break;

	case evKey:
		@GetKey();
		edit_box_key stdcall (#edit_save);
		EAX >>= 16;
		if (SCAN_CODE_ENTER == AL) EventTakeScreenshot();
		if (SCAN_CODE_ESC == AL) ExitProcess();
		break;
     
	case evReDraw:
		DrawWindow();
	}
}


void DrawWindow()
{
	int i;

	sc.get();
	DefineAndDrawWindow(screen.width-400, screen.height/3, 270, 
		skin_height + 30+PD+PD, 0x34, sc.work, "EasyShot",0);
	GetProcessInfo(#Form, SelfInfo);

	DrawCaptButton(PD, PD, 170, 28, BTN_MAKE_SCREENSHOT, 0x0090B8, 0xFFFfff, T_TAKE_SCREENSHOT);
	DefineButton(PD+170+20, PD, 35, 28, BTN_SETTINGS, sc.button);
	for (i=0; i<=2; i++) DrawBar(PD+170+30, i*5+PD+9, 15, 2, sc.button_text);
	delay.draw(PD, SETTINGS_Y);
	DrawFileBox(#edit_save, T_EDITBOX_FRAME, BTN_CHOOSE_SAVING_PATH);	
}


void EventChooseSavePath()
{
	OpenDialog_start stdcall (#open_folder_dialog);
	if (open_folder_dialog.status) {
		edit_box_set_text stdcall (#edit_save, #save_path);	
	}
}


void EventClickSettings()
{
	show_settings ^= 1;
	@MoveSize(OLD, OLD, show_settings*75 + 270, 
		show_settings*110 + skin_height + PD+PD+30);
}


void EventTakeScreenshot() 
{
	int i=0;
	char save_file_name[4096];
	static dword screenshot;

	if (!screenshot) screenshot = malloc(screen.width * screen.height * 3);

	do {
		i++;
		//sprintf(, "%s/screen_%i.png", #save_path, i);
		strcpy(#save_file_name, #save_path);
		if (save_file_name[strlen(#save_file_name)-1]!='/') chrcat(#save_file_name, '/');
		strcat(#save_file_name, "screen_");
		strcat(#save_file_name, itoa(i));
		strcat(#save_file_name, ".png");
	} while (file_exists(#save_file_name));

	if (!dir_exists(#save_path)) {
		notify(T_NO_DIR);
		return;
	}

	MinimizeWindow(); 
	pause(delay.value*100);
	CopyScreen(screenshot, 0, 0, screen.width, screen.height);
	save_image(screenshot, screen.width, screen.height, #save_file_name);
	ActivateWindow(GetProcessSlot(Form.ID));
}

