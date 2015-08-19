
#ifdef LANG_RUS
	?define EDIT_FILE_ASSOCIATIONS "Редактировать ассоциации файлов"
	?define TITLE_SETT "Настройки"
	?define SHOW_DEVICE_CLASS "Выводить названия класса устройств"
	?define SHOW_REAL_NAMES "Показывать имена файлов не меняя регистр"
	?define FONT_SIZE_LABEL "Размер шрифта"
	?define USE_TWO_PANELS "Две панели"
	?define LIST_LINE_HEIGHT "Высота строки в списке"
	?define NOTIFY_COPY_END "Уведомлять о завершении копирования"
	?define T_DOUBLE_CLICK "Время двойного клика (в сотых)"
#else
	?define EDIT_FILE_ASSOCIATIONS "Edit file associations"
	?define TITLE_SETT "Settings"
	?define SHOW_DEVICE_CLASS "Show device class name"
	?define SHOW_REAL_NAMES "Show real file names without changing case"
	?define FONT_SIZE_LABEL "Font size"
	?define USE_TWO_PANELS "Two panels"
	?define LIST_LINE_HEIGHT "List line height"
	?define NOTIFY_COPY_END "Notify when copying finished"
	?define T_DOUBLE_CLICK "Double click time (in hundredths)"
#endif

char confir_section = "Config";


void settings_dialog()
{   
	byte id;
	
	loop(){
		switch(WaitEvent())
		{
			case evButton: 
				id=GetButtonID();
				if (id==1) ExitSettings();
				else if (id==5)
				{
					RunProgram("tinypad", "/sys/settings/assoc.ini");
					break;
				}
				else if (id==20) show_dev_name ^= 1;
				else if (id==21) { action_buf=109; real_files_names_case ^= 1; }
				else if (id==22) info_after_copy ^= 1;
				else if (id==24) two_panels ^= 1;
				else if (id==25) files.line_h++;
				else if (id==26) && (files.line_h>18) files.line_h--;
				else if (id==27) MOUSE_TIME++;
				else if (id==28) && (MOUSE_TIME>29) MOUSE_TIME--;
				else if (id==30) && (font_size<22) { font_size++; BigFontsChange(); }
				else if (id==31) && (font_size>9) { font_size--; BigFontsChange(); }
				SaveIniSettings();
				EventRedrawWindow(Form.left,Form.top);
				DrawSettingsCheckBoxes();
			break;
					
			case evKey:
				GetKeys();
				if (key_scancode==SCAN_CODE_ESC) ExitSettings();
				break;
				
			case evReDraw:
				DefineAndDrawWindow(Form.left + Form.width/2-10, Form.top + Form.height/2 - 75, 300, 226+GetSkinHeight(),0x34,system.color.work,TITLE_SETT);
				DrawSettingsCheckBoxes();
				DrawFlatButton(9, 186, strlen(EDIT_FILE_ASSOCIATIONS)+4*6, 22, 5, 0xE4DFE1, EDIT_FILE_ASSOCIATIONS);
		}
	}
}

void ExitSettings()
{
	settings_window = 0;
	cmd_free = 4;
	SaveIniSettings();
	ExitProcess();
}

void DrawSettingsCheckBoxes()
{
	CheckBox2(10, 11, 20, SHOW_DEVICE_CLASS,  show_dev_name);
	CheckBox2(10, 33, 21, SHOW_REAL_NAMES,  real_files_names_case);
	CheckBox2(10, 55, 22, NOTIFY_COPY_END,  info_after_copy);
	CheckBox2(10, 77, 24, USE_TWO_PANELS,  two_panels); 
	MoreLessBox(10, 103, 18, 27, 28, #system.color, MOUSE_TIME, T_DOUBLE_CLICK);
	MoreLessBox(10, 130, 18, 25, 26, #system.color, files.line_h, LIST_LINE_HEIGHT);
	if (font.data) MoreLessBox(10, 157, 18, 30, 31, #system.color, font_size, FONT_SIZE_LABEL);
}


void LoadIniSettings()
{
	ini_get_color stdcall (eolite_ini_path, #confir_section, "SelectionColor",   0x94AECE); col_selec = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "ShowDeviceName",    1); show_dev_name = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "RealFileNamesCase", 0); real_files_names_case = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "InfoAfterCopy",     0); info_after_copy = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "FontSize",          9); font_size = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "TwoPanels",         0); two_panels = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "LineHeight",       18); files.line_h = EAX;
	ini_get_int stdcall   (eolite_ini_path, #confir_section, "TimeDoubleClick",  50); MOUSE_TIME = EAX;
	BigFontsChange();
	font.no_bg_copy = true;
	font.load("/sys/font/Tahoma.kf");
}


void SaveIniSettings()
{
	ini_set_int stdcall (eolite_ini_path, #confir_section, "ShowDeviceName", show_dev_name);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "RealFileNamesCase", real_files_names_case);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "InfoAfterCopy", info_after_copy);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "FontSize", font_size);
	ini_set_int stdcall (eolite_ini_path, #confir_section, "TwoPanels", two_panels);
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
	files.line_h = font.height + 4;
	if (files.line_h<18) files.line_h = 18;
	files_active.line_h = files_inactive.line_h = files.line_h;

	files.SetFont(6, 6, 10000000b);
	FileShow.font_size_x = files.font_w;
	FileShow.font_number = 0;
}


void CheckBox2(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, system.color.work_graph, system.color.work_text, value);
}