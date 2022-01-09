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
	?define T_SCREENSHOT "Скриншот"
	?define T_READY "Готов."
	?define T_RECORD "Запись"
	?define T_STOP "Остановить запись"
	?define T_SETTINGS "Настройки"
	?define T_EDITBOX_FRAME "Путь сохранения скриншота"
	?define T_DELAY "Задержка в секундах"
	?define T_NO_DIR "Папка для сохранения не существует!"
#else
	?define T_SCREENSHOT "Screenshot"
	?define T_READY "Ready."
	?define T_RECORD "Record"
	?define T_STOP "Stop recording"
	?define T_SETTINGS "Settings"
	?define T_EDITBOX_FRAME "Save path"
	?define T_DELAY "Delay in seconds"
	?define T_NO_DIR "Saving directory does not exists!"
#endif

/* === DATA === */	

proc_info Form;

enum {
	BTN_SCREENSHOT=10,
	BTN_RECORD,
	BTN_SETTINGS,
	BTN_CHOOSE_SAVING_PATH
};

bool recording=false;

#define PD 18 //padding
#define SETTINGS_Y PD+PD+60+10

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

	I_Param = T_READY;
	init_libraries();

	edit_box_set_text stdcall (#edit_save, "/tmp0/1");	

	@SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(@WaitEventTimeout(delay.value*100))
	{
	case evMouse:
		edit_box_mouse stdcall (#edit_save);
		break;

	case evButton:
		id = @GetButtonID();
		switch(id){
			case CLOSE_BTN: @ExitProcess();
			case BTN_SCREENSHOT: EventScreenshotClick(); break;
			case BTN_RECORD: EventRecordingClick(); break;
			case BTN_SETTINGS: EventSettingsClick(); break;
			case BTN_CHOOSE_SAVING_PATH: EventChooseSavePathClick(); break;
			default: delay.click(id);
		}
		break;

	case evKey:
		@GetKey();
		edit_box_key stdcall (#edit_save);
		EAX >>= 16;
		if (SCAN_CODE_ENTER == AL) EventScreenshotClick();
		else if (SCAN_CODE_ESC == AL) ExitProcess();
		break;
     
	case evReDraw:
		DrawWindow();

	default:
		if (recording) MakeScreenshot();
	}
}


void DrawWindow()
{
	unsigned i;

	sc.get();
	DefineAndDrawWindow(screen.w-400, screen.h/3, 280, 
		skin_h + 50+PD+PD, 0x34, sc.work, "EasyShot",0);
	GetProcessInfo(#Form, SelfInfo);

	if (!recording) {
		DrawCaptButton(PD, PD, 101, 28, BTN_SCREENSHOT, 
			0x0090B8, 0xFFFfff, T_SCREENSHOT);
		DrawCaptButton(PD+105, PD, 75, 28, BTN_RECORD, 
			0x0090B8, 0xFFFfff, T_RECORD);
	} else {
		DrawCaptButton(PD, PD, 96+80+4, 28, BTN_RECORD, 
			0xC75C54, 0xFFFfff, T_STOP);
	}
	DefineButton(PD+200, PD, 35, 28, BTN_SETTINGS, sc.button);
	for (i=0; i<=2; i++) DrawBar(PD+210, i*5+PD+9, 15, 2, sc.button_text);
	DrawStatusBar(I_Param);	

	delay.draw(PD, SETTINGS_Y);
	DrawFileBox(#edit_save, T_EDITBOX_FRAME, BTN_CHOOSE_SAVING_PATH);

}

void DrawStatusBar(char *s)
{
	I_Param = s; 
	WriteTextWithBg(PD, 35+PD, 0xD0, sc.work_text, I_Param, sc.light);
}

dword ScreenshotBuf()
{
	static dword screenshot;
	if (!screenshot) screenshot = malloc(screen.w * screen.h * 3);	
	return screenshot;
}

bool GetSavingPath()
{
	int i=0;
	char save_file_name[4096];
	do {
		i++;
		//sprintf(, "%s/screen_%i.png", #save_path, i);
		strcpy(#save_file_name, #save_path);
		if (save_file_name[strlen(#save_file_name)-1]!='/') chrcat(#save_file_name, '/');
		strcat(#save_file_name, "scr_");
		strcat(#save_file_name, itoa(i));
		strcat(#save_file_name, ".png");
	} while (file_exists(#save_file_name));

	if (!dir_exists(#save_path)) {
		DrawStatusBar(T_NO_DIR);
		return NULL;
	}
	return #save_file_name;
}

void MakeScreenshot() 
{
	if (I_Path = GetSavingPath()) {
		CopyScreen(ScreenshotBuf(), 0, 0, screen.w, screen.h);
		I_Param = save_image(ScreenshotBuf(), screen.w, screen.h, I_Path);
		if (!I_Param) I_Param = I_Path;
		DrawStatusBar(I_Param);
	} else {
		recording = false;
		DrawWindow();
	}
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventScreenshotClick()
{
	MinimizeWindow();
	pause(delay.value*100);
	MakeScreenshot();
	ActivateWindow(GetProcessSlot(Form.ID));
}

void EventRecordingClick()
{
	recording ^= 1;
	DrawWindow();
}

void EventSettingsClick()
{
	show_settings ^= 1;
	@MoveSize(OLD, OLD, show_settings*65 + 280, 
		show_settings*110 + skin_h + PD+PD+50);
}

void EventChooseSavePathClick()
{
	OpenDialog_start stdcall (#open_folder_dialog);
	if (open_folder_dialog.status) {
		edit_box_set_text stdcall (#edit_save, #save_path);	
	}
}