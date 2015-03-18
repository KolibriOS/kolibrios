//Leency 2008-2015

#define EDITOR_PATH     "/sys/tinypad"

#ifdef LANG_RUS
	?define EDIT_FILE_ASSOCIATIONS "Редактировать ассоциации файлов"
	?define TITLE_SETT "Настройки"
	?define SHOW_DEVICE_CLASS "Выводить названия класса устройств"
	?define SHOW_REAL_NAMES "Показывать имена файлов не меняя регистр"
	?define LIST_LINE_HEIGHT "Высота строки в списке"
	?define NOTIFY_COPY_END "Уведомлять о завершении копирования"
	?define CANCEL_T "Отмена"
	?define APPLY_T "Применить"
#else
	?define EDIT_FILE_ASSOCIATIONS "Edit file associations"
	?define TITLE_SETT "Settings"
	?define SHOW_DEVICE_CLASS "Show device class name"
	?define SHOW_REAL_NAMES "Show real file names without changing case"
	?define LIST_LINE_HEIGHT "List line height"
	?define NOTIFY_COPY_END "Notify when copying finished"
	?define CANCEL_T "Cancel"
	?define APPLY_T "Apply"
#endif

char confir_section = "Config";


void settings_dialog()
{   
	byte id;
	unsigned int key;
	proc_info settings_form;
	
	if (active_settings) ExitProcess();
	active_settings=1;

	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (id==10)
				{
					SaveIniSettings();
					active_settings=0;
					action_buf = 300;
					ExitProcess();
				}					
				if (id==1) || (id==11) 
				{
					active_settings=0;
					ExitProcess();
				}
				if (id==5)
				{
					RunProgram("tinypad", "/sys/settings/assoc.ini");
					break;
				}
				if (id==20) show_dev_name ^= 1;
				if (id==21) real_files_names_case ^= 1;
				if (id==22) info_after_copy ^= 1;
				if (id==25) files.line_h++;
				if (id==26) && (files.line_h>8) files.line_h--;
				DrawSettingsCheckBoxes();
				break;
				
		case evKey:
				key = GetKey();
				if (key==27)
				{
					active_settings = 0;
					action_buf = 300;
					ExitProcess();
				}
				break;
			
		case evReDraw:
				DefineAndDrawWindow(Form.left + 100, 150, 300, 210+GetSkinHeight(),0x34,sc.work,TITLE_SETT);
				GetProcessInfo(#settings_form, SelfInfo);
				DrawSettingsCheckBoxes();
				DrawFlatButton(9, 116, strlen(EDIT_FILE_ASSOCIATIONS)+4*6, 22, 5, 0xE4DFE1, EDIT_FILE_ASSOCIATIONS);
				DrawFlatButton(128, settings_form.cheight - 34, 70, 22, 10, 0xE4DFE1, APPLY_T);
				DrawFlatButton(208, settings_form.cheight - 34, 70, 22, 11, 0xE4DFE1, CANCEL_T);
	}
}

void DrawSettingsCheckBoxes()
{
	CheckBox2(10, 11, 20, SHOW_DEVICE_CLASS,  show_dev_name);
	CheckBox2(10, 33, 21, SHOW_REAL_NAMES,  real_files_names_case);
	CheckBox2(10, 55, 22, NOTIFY_COPY_END,  info_after_copy);
	MoreLessBox(10, 82, 18, 25, 26, sc.work_graph, 0xD2D3D3, 0x000000, files.line_h, LIST_LINE_HEIGHT);
}


void LoadIniSettings()
{
	ini_get_color stdcall (eolite_ini_path, #confir_section, "SelectionColor", 0x94AECE);
	edit2.shift_color = EAX;
	col_selec = EAX;
	ini_get_int stdcall (eolite_ini_path, #confir_section, "LineHeight", 18);
	files.line_h = EAX;
	ini_get_int stdcall (eolite_ini_path, #confir_section, "ShowDeviceName", 1);
	show_dev_name = EAX;
	ini_get_int stdcall (eolite_ini_path, #confir_section, "RealFileNamesCase", 0);
	real_files_names_case = EAX;
	ini_get_int stdcall (eolite_ini_path, #confir_section, "InfoAfterCopy", 0);
	info_after_copy = EAX;
}

void SaveIniSettings()
{
	ini_set_int stdcall (eolite_ini_path, #confir_section, "ShowDeviceName", show_dev_name);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "RealFileNamesCase", real_files_names_case);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "InfoAfterCopy", info_after_copy);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "LineHeight", files.line_h);
}



void Write_Error(int error_number)
{
	char error_message[500];
	dword ii;
	if (files.current>=0) Line_ReDraw(0xFF0000, files.current);
	pause(5);
	strcpy(#error_message, "\"Eolite\n");
	ii = get_error(error_number);
	strcat(#error_message, ii);
	strcat(#error_message, "\" -tE");
	notify(#error_message);
}


void SetAppColors()
{
	sc.work = 0xE4DFE1;
	sc.work_text = 0;
	sc.work_graph  = 0x9098B0; //A0A0B8; //0x819FC5;
	sc.work_button_text = 0x000000;
	col_padding = 0xC8C9C9;
	//col_selec   = 0x94AECE;
	col_lpanel  = 0x00699C;
}


void CheckBox2(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, sc.work_graph, sc.work_text, value);
}