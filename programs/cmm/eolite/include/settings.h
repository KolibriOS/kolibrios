//Leency 2008-2015

#define EDITOR_PATH     "/sys/tinypad"

#ifdef LANG_RUS
	?define EDIT_FILE_ASSOCIATIONS "Редактировать ассоциации файлов"
	?define TITLE_SETT "Настройки"
	?define SHOW_DEVICE_CLASS "Выводить названия класса устройств"
	?define SHOW_REAL_NAMES "Показывать имена файлов не меняя регистр"
	?define USE_BIG_FONTS "Большой шрифт (только английские символы!)"
	?define LIST_LINE_HEIGHT "Высота строки в списке"
	?define NOTIFY_COPY_END "Уведомлять о завершении копирования"
	?define CANCEL_T "Отмена"
	?define APPLY_T "Применить"
	?define T_DOUBLE_CLICK "Время двойного клика (в сотых)"
#else
	?define EDIT_FILE_ASSOCIATIONS "Edit file associations"
	?define TITLE_SETT "Settings"
	?define SHOW_DEVICE_CLASS "Show device class name"
	?define SHOW_REAL_NAMES "Show real file names without changing case"
	?define USE_BIG_FONTS "Use big fonts (English characters only!)"
	?define LIST_LINE_HEIGHT "List line height"
	?define NOTIFY_COPY_END "Notify when copying finished"
	?define CANCEL_T "Cancel"
	?define APPLY_T "Apply"
	?define T_DOUBLE_CLICK "Double click time (in hundredths)"
#endif

char confir_section = "Config";


void settings_dialog()
{   
	byte id;
	proc_info settings_form;
	
	dword save_show_dev_name,save_real_files_names_case, save_info_after_copy, save_use_big_fonts, save_files_h, save_DBLTime;
	
	if (active_settings){
		EXIT_SETTING: 
		
		show_dev_name         = save_show_dev_name;
		real_files_names_case = save_real_files_names_case;
		info_after_copy       = save_info_after_copy;
		use_big_fonts         = save_use_big_fonts;
		files.line_h          = save_files_h;
		MOUSE_TIME            = save_DBLTime;
		
		cmd_free = 4;
		
		ExitProcess();
	}
	active_settings=1;
	
	save_show_dev_name         = show_dev_name;
	save_real_files_names_case = real_files_names_case;
	save_info_after_copy       = info_after_copy;
	save_use_big_fonts         = use_big_fonts;
	save_files_h               = files.line_h;
	save_DBLTime               = MOUSE_TIME;
	
	loop(){
		switch(WaitEvent())
		{
			case evButton: 
				id=GetButtonID();
				if (id==10)
				{
					SaveIniSettings();
					active_settings=0;
					action_buf = 300;
					cmd_free = 4;
					ExitProcess();
				}					
				else if (id==1) || (id==11) 
				{
					active_settings=0;
					goto EXIT_SETTING;
				}
				else if (id==5)
				{
					RunProgram("tinypad", "/sys/settings/assoc.ini");
					break;
				}
				else if (id==20) show_dev_name ^= 1;
				else if (id==21) real_files_names_case ^= 1;
				else if (id==22) info_after_copy ^= 1;
				else if (id==23) { use_big_fonts ^= 1; BigFontsChange(); }
				else if (id==25) files.line_h++;
				else if (id==26) && (files.line_h>14) files.line_h--;
				else if (id==27) MOUSE_TIME++;
				else if (id==28) && (MOUSE_TIME>29) MOUSE_TIME--;
				EventRedrawWindow(Form.left,Form.top);
				DrawSettingsCheckBoxes();
			break;
					
			case evKey:
				GetKeys();
				if (key_scancode==SCAN_CODE_ESC)
				{
					active_settings = 0;
					action_buf = 300;
					goto EXIT_SETTING;
				}
				break;
				
			case evReDraw:
				DefineAndDrawWindow(Form.left + Form.width/2, Form.top + Form.height/2 - 75, 300, 234+GetSkinHeight(),0x34,system.color.work,TITLE_SETT);
				GetProcessInfo(#settings_form, SelfInfo);
				DrawSettingsCheckBoxes();
				DrawFlatButton(9, 166, strlen(EDIT_FILE_ASSOCIATIONS)+4*6, 22, 5, 0xE4DFE1, EDIT_FILE_ASSOCIATIONS);
				DrawFlatButton(128, settings_form.cheight - 30, 70, 22, 10, 0xE4DFE1, APPLY_T);
				DrawFlatButton(208, settings_form.cheight - 30, 70, 22, 11, 0xE4DFE1, CANCEL_T);
		}
	}
}

void DrawSettingsCheckBoxes()
{
	CheckBox2(10, 11, 20, SHOW_DEVICE_CLASS,  show_dev_name);
	CheckBox2(10, 33, 21, SHOW_REAL_NAMES,  real_files_names_case);
	CheckBox2(10, 55, 22, NOTIFY_COPY_END,  info_after_copy);
	CheckBox2(10, 77, 23, USE_BIG_FONTS,  use_big_fonts); 
	MoreLessBox(10, 104, 18, 25, 26, #system.color, files.line_h, LIST_LINE_HEIGHT);
	MoreLessBox(10, 134, 18, 27, 28, #system.color, MOUSE_TIME, T_DOUBLE_CLICK);
}


void LoadIniSettings()
{
	ini_get_color stdcall (eolite_ini_path, #confir_section, "SelectionColor",   0x94AECE); col_selec = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "ShowDeviceName",    1); show_dev_name = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "RealFileNamesCase", 0); real_files_names_case = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "InfoAfterCopy",     0); info_after_copy = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "UseBigFonts",       0); use_big_fonts = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "LineHeight",       18); files.line_h = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "TimeDoubleClick",  50); MOUSE_TIME = EAX;
	BigFontsChange();
}


void SaveIniSettings()
{
	ini_set_int stdcall (eolite_ini_path, #confir_section, "ShowDeviceName", show_dev_name);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "RealFileNamesCase", real_files_names_case);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "InfoAfterCopy", info_after_copy);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "UseBigFonts", use_big_fonts);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "LineHeight", files.line_h);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "TimeDoubleClick", MOUSE_TIME);
}



void Write_Error(int error_number)
{
	char error_message[500];
	dword ii;
	if (files.current>=0) Line_ReDraw(0xFF0000, files.current);
	pause(5);
	sprintf(#error_message,"\"%s\n%s\" -%s","Eolite",get_error(error_number),"tE");
	notify(#error_message);	
}


void SetAppColors()
{
	system.color.work = 0xE4DFE1;
	system.color.work_text = 0;
	system.color.work_graph  = 0x9098B0; //A0A0B8; //0x819FC5;
	system.color.work_button = 0xD2D3D3;
	system.color.work_button_text = 0x000000;
	col_padding = 0xC8C9C9;
	//col_selec   = 0x94AECE;
	col_lpanel  = 0x00699C;
}


void BigFontsChange()
{
	if (use_big_fonts) 
	{
		font_type = 10110000b;
		font_h = 14;
		FileShow.font_size_x = 8;
		FileShow.font_number = 3;
	}
	else
	{
		font_type=10000000b;
		font_h = 6;
		FileShow.font_size_x = 6;
		FileShow.font_number = 0;
	} 
}


void CheckBox2(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, system.color.work_graph, system.color.work_text, value);
}