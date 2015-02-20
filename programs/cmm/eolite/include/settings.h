//Leency 2008-2015

#define EDITOR_PATH     "/sys/tinypad"

#ifdef LANG_RUS
	?define EDIT_FILE_ASSOCIATIONS "Редактировать ассоциации файлов"
	?define TITLE_SETT "Настройки"
	?define SET_1 "Выводить названия класса устройств"
	?define SET_2 "Показывать имена файлов не меняя регистр"
	?define SET_3 "Высота строки в списке"
	?define CANCEL_T "Отмена"
	?define APPLY_T "Применить"
#else
	?define EDIT_FILE_ASSOCIATIONS "Edit file associations"
	?define TITLE_SETT "Settings"
	?define SET_1 "Show device class name"
	?define SET_2 "Show real file names without changing case"
	?define SET_3 "List line height"
	?define CANCEL_T "Cancel"
	?define APPLY_T "Apply"
#endif

int	mouse_ddd;
char lineh_s[30]="18\0";
edit_box LineHeight_ed = {52,10,70,0xffffff,0x94AECE,0xffc90E,0xffffff,2,4,#lineh_s,#mouse_ddd, 1000000000000000b,2,2};
checkbox2 ShowDeviceName_chb = {10*65536+15, 10*65536+15, 5, 0xffffff, 0x9098B0, 0x80000000, SET_1, CH_FLAG_MIDDLE, 0};
checkbox2 RealFileNamesCase_chb = {10*65536+15, 30*65536+15, 5, 0xffffff, 0x9098B0, 0x80000000, SET_2, CH_FLAG_MIDDLE, 0};

void settings_dialog()
{   
	byte id;
	unsigned int key;
	proc_info settings_form;
	dword eolite_ini_path = abspath("Eolite.ini");

	if (active_about) ExitProcess();
	active_about=1;

	ShowDeviceName_chb.size_of_str = strlen(SET_1) * 6;
	RealFileNamesCase_chb.size_of_str = strlen(SET_2) * 6;

	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (id==10)
				{
					if ( asm test ShowDeviceName_chb.flags, 2) ini_set_int stdcall (eolite_ini_path, "Config", "ShowDeviceName", 1);
					ELSE  ini_set_int stdcall (eolite_ini_path, "Config", "ShowDeviceName", 0);
					if ( asm test RealFileNamesCase_chb.flags, 2) ini_set_int stdcall (eolite_ini_path, "Config", "RealFileNamesCase", 1);
					ELSE ini_set_int stdcall (eolite_ini_path, "Config", "RealFileNamesCase", 0);
					if (LineHeight_ed.size) ini_set_int stdcall (eolite_ini_path, "Config", "LineHeight", atoi(#lineh_s));
					active_about=0;
					action_buf = 300;
					ExitProcess();
				}					
				if (id==1) || (id==11) 
				{
					active_about=0;
					ExitProcess();
				}
				if (id==5)
				{
					RunProgram("tinypad", "/sys/settings/assoc.ini");
				}
				break;
				
		case evKey:
				key = GetKey();
				if (key==27)
				{
					active_about=0;
					action_buf = 300;
					ExitProcess();
				}
				EAX=key<<8;
				edit_box_key stdcall(#LineHeight_ed);
				break;
				
		case evMouse:
				check_box_mouse stdcall (#ShowDeviceName_chb);
				check_box_mouse stdcall (#RealFileNamesCase_chb);
				edit_box_mouse stdcall (#LineHeight_ed);
				break;
			
		case evReDraw:
				DefineAndDrawWindow(Form.left + 100, 150, 300, 200+GetSkinHeight(),0x34,sc.work,TITLE_SETT);
				GetProcessInfo(#settings_form, SelfInfo);
				
				if (show_dev_name) ShowDeviceName_chb.flags = 110b;
				ELSE  ShowDeviceName_chb.flags = 100b;
				
				if (real_files_names_case) RealFileNamesCase_chb.flags = 110b;
				ELSE RealFileNamesCase_chb.flags = 100b;
				
				key = itoa(files.line_h);
				strcpy(#lineh_s, key);
				
				check_box_draw stdcall (#ShowDeviceName_chb);
				check_box_draw stdcall (#RealFileNamesCase_chb);
				edit_box_draw stdcall (#LineHeight_ed);
				DrawRectangle(LineHeight_ed.left-1, LineHeight_ed.top-1, LineHeight_ed.width+2, 16, sc.work_graph);
				WriteText(10, 57, 0x80, 0x000000, SET_3);
				DrawFlatButton(9, 100, strlen(EDIT_FILE_ASSOCIATIONS)+4*6, 22, 5, 0xE4DFE1, EDIT_FILE_ASSOCIATIONS);
				DrawFlatButton(128, settings_form.cheight - 34, 70, 22, 10, 0xE4DFE1, APPLY_T);
				DrawFlatButton(208, settings_form.cheight - 34, 70, 22, 11, 0xE4DFE1, CANCEL_T);
	}
}


void GetIni()
{
	dword eolite_ini_path = abspath("Eolite.ini"); 
	ini_get_color stdcall (eolite_ini_path, "Config", "SelectionColor", 0x94AECE);
	edit2.shift_color = EAX;
	col_selec = EAX;
	ini_get_int stdcall (eolite_ini_path, "Config", "LineHeight", 18);
	files.line_h = EAX;
	ini_get_int stdcall (eolite_ini_path, "Config", "ShowDeviceName", 1);
	show_dev_name = EAX;
	ini_get_int stdcall (eolite_ini_path, "Config", "RealFileNamesCase", 0);
	real_files_names_case = EAX;
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
	/*
	sc.get();
	for (i=0; i<=14; i++) col_palette[i] = sc.work;
	toolbar_pal[0]= goto_about_pal[0] = sc.work = sc.work;
	col_lpanel = sc.work_graph;
	for (i=0; i<=99; i++) blue_hl_pal[i] = sc.work_graph;
	*/
}