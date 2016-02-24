
#ifdef LANG_RUS
	?define EDIT_FILE_ASSOCIATIONS "Редактировать ассоциации файлов"
	?define TITLE_SETT "Настройки"
	?define SHOW_DEVICE_CLASS "Выводить названия класса устройств"
	?define SHOW_REAL_NAMES "Показывать имена файлов не меняя регистр"
	?define FONT_SIZE_LABEL "Размер шрифта"
	?define USE_TWO_PANELS "Две панели"
	?define smooth_FONT "Использовать сглаженный шрифт"
	?define LIST_LINE_HEIGHT "Высота строки в списке"
	?define NOTIFY_COPY_END "Уведомлять о завершении копирования"
	?define SAVE_PATH_AS_DEFAULT "Сделать текущий путь домашним каталогом"
#else
	?define EDIT_FILE_ASSOCIATIONS "Edit file associations"
	?define TITLE_SETT "Settings"
	?define SHOW_DEVICE_CLASS "Show device class name"
	?define SHOW_REAL_NAMES "Show file names in original case"
	?define FONT_SIZE_LABEL "Font size"
	?define USE_TWO_PANELS "Two panels"
	?define smooth_FONT "Use smooth font"
	?define LIST_LINE_HEIGHT "List line height"
	?define NOTIFY_COPY_END "Notify when copying finished"
	?define SAVE_PATH_AS_DEFAULT "Save current path as home folder"
#endif

char config_section[] = "Config";
int WinX, WinY, WinW, WinH;

void settings_dialog()
{   
	byte id;
	active_settings=1;
	loop(){
		switch(WaitEvent())
		{
			case evButton: 
				id=GetButtonID();
				if (id==1) { ExitSettings(); break; }
				else if (id==5)
				{
					RunProgram("tinypad", "/sys/settings/assoc.ini");
					break;
				}
				else if (id==6)
				{
					ini_set_str stdcall (eolite_ini_path, #config_section, "DefaultPath", #path,strlen(#path));
					break;
				}
				else if (id==20) show_dev_name ^= 1;
				else if (id==21) { action_buf=109; real_files_names_case ^= 1; }
				else if (id==22) info_after_copy ^= 1;
				else if (id==24) two_panels ^= true;
				else if (id==32) show_breadcrumb ^= true;
				else if (id==25) { files.item_h++; files_active.item_h = files_inactive.item_h = files.item_h; }
				else if (id==26) && (files.item_h>18) files.item_h--;
				else if (id==30) { label.size.pt++; IF(!label.changeSIZE()) label.size.pt--; BigFontsChange(); }
				else if (id==31) { label.size.pt--; IF(!label.changeSIZE()) label.size.pt++; BigFontsChange(); }
				EventRedrawWindow(Form.left,Form.top);
				//DrawSettingsCheckBoxes();
			break;
					
			case evKey:
				GetKeys();
				if (key_scancode==SCAN_CODE_ESC) ExitSettings();
				break;
				
			case evReDraw:
				DefineAndDrawWindow(Form.left + Form.width/2-10, Form.top + Form.height/2 - 75, 370, 282+GetSkinHeight(),0x34,system.color.work,TITLE_SETT);
				DrawSettingsCheckBoxes();
		}
	}
}

void ExitSettings()
{
	active_settings = 0;
	settings_window = 0;
	cmd_free = 4;
	ExitProcess();
}

void DrawSettingsCheckBoxes()
{
	incn y;
	y.n = 0;
	CheckBox(10, y.inc(13), 20, SHOW_DEVICE_CLASS,  show_dev_name);
	CheckBox(10, y.inc(24), 21, SHOW_REAL_NAMES,  real_files_names_case);
	CheckBox(10, y.inc(24), 22, NOTIFY_COPY_END,  info_after_copy);
	CheckBox(10, y.inc(24), 24, USE_TWO_PANELS,  two_panels);
	CheckBox(10, y.inc(24), 32, "Show breadcrumbs",  show_breadcrumb);
	MoreLessBox(10, y.inc(30), 25, 26, files.item_h, LIST_LINE_HEIGHT);
	if (label.font) MoreLessBox(10, y.inc(30), 30, 31, label.size.pt, FONT_SIZE_LABEL);
	DrawFlatButton(9, y.inc(35), strlen(SAVE_PATH_AS_DEFAULT)+3*8, 24, 6, SAVE_PATH_AS_DEFAULT);
	DrawFlatButton(9, y.inc(35), strlen(EDIT_FILE_ASSOCIATIONS)+3*8, 24, 5, EDIT_FILE_ASSOCIATIONS);
}


void LoadIniSettings()
{
	files.SetFont(6, 9, 10000000b);
	FileShow.font_size_x = files.font_w;
	FileShow.font_number = 0;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "ShowDeviceName",    1); show_dev_name = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "RealFileNamesCase", 0); real_files_names_case = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "InfoAfterCopy",     0); info_after_copy = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "FontSize",         12); label.size.pt = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "TwoPanels",         0); two_panels = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "LineHeight",       19); files.item_h = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "WinX", 200); WinX = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "WinY", 50); WinY = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "WinW", 550); WinW = EAX;
	ini_get_int stdcall   (eolite_ini_path, #config_section, "WinH", 506); WinH = EAX;
	ini_get_str stdcall   (eolite_ini_path, #config_section, "DefaultPath", #path,4096,"/rd/1/");


	ini_get_str stdcall ("/sys/SETTINGS/SYSTEM.INI", "system", "font file",#temp,4096,DEFAULT_FONT);
	label.init(#temp);
	ini_get_str stdcall ("/sys/SETTINGS/SYSTEM.INI", "system", "font smoothing",#temp,4096,"on");
	if(!strcmp(#temp,"off")) label.smooth = false; else label.smooth = true;
}


void SaveIniSettings()
{
	ini_set_int stdcall (eolite_ini_path, #config_section, "ShowDeviceName", show_dev_name);
	ini_set_int stdcall (eolite_ini_path, #config_section, "RealFileNamesCase", real_files_names_case);
	ini_set_int stdcall (eolite_ini_path, #config_section, "InfoAfterCopy", info_after_copy);
	ini_set_int stdcall (eolite_ini_path, #config_section, "FontSize", label.size.pt);
	ini_set_int stdcall (eolite_ini_path, #config_section, "TwoPanels", two_panels);
	ini_set_int stdcall (eolite_ini_path, #config_section, "LineHeight", files.item_h);
	ini_set_int stdcall (eolite_ini_path, #config_section, "WinX", Form.left);
	ini_set_int stdcall (eolite_ini_path, #config_section, "WinY", Form.top);
	ini_set_int stdcall (eolite_ini_path, #config_section, "WinW", Form.width);
	ini_set_int stdcall (eolite_ini_path, #config_section, "WinH", Form.height);
}



void Write_Error(int error_number)
{
	char error_message[500];
	dword ii;
	if (files.cur_y>=0) Line_ReDraw(0xFF0000, files.cur_y);
	pause(5);
	sprintf(#error_message,"\"%s\n%s\" -%s","Eolite",get_error(error_number),"tE");
	notify(#error_message);	
}


void SetAppColors()
{
	system.color.get();
	//system.color.work = 0xE4DFE1;
	//system.color.work_text = 0;
	//system.color.work_graph  = 0x7E87A3; //A0A0B8;
	//system.color.work_button = 0x7E87A3;
	//system.color.work_button_text = 0x000000
	col_work    = 0xE4DFE1;
	col_padding = 0xC8C9C9;
	col_selec   = 0x94AECE;
	col_lpanel  = 0x00699C;
	col_graph   = 0x7E87A3;
}


void BigFontsChange()
{
	files.item_h = label.size.pt + 4;
	if (files.item_h<18) files.item_h = 18;
	files_active.item_h = files_inactive.item_h = files.item_h;
}
