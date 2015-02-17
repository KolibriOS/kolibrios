//Leency 2008-2013

#define EDITOR_PATH     "/sys/tinypad"
#define BROWSER_PATH    "/sys/htmlv"
#define BROWSER_LINK    "http://kolibri-n.org/index.php"

#ifdef LANG_RUS
	?define TITLE_SETT "Настройки"
	?define SET_1 "Показывать имена устройств"
	?define SET_2 "Реальные имена"
	?define SET_3 "Высота выделения"
	?define CANCEL_T "Отмена"
	?define APPLY_T "Применить"
#elif LANG_EST
	?define TITLE_SETT "Programmis Eolite"
#else
	?define TITLE_SETT "About Eolite"

#endif

int	mouse_ddd;
char lineh_s[30]="18\0";
edit_box LineHeight_ed = {50,10,70,0xffffff,0x94AECE,0x000000,0x000000,2,4,#lineh_s,#mouse_ddd, 1000000000000010b,2,2};
checkbox2 ShowDeviceName_chb = {10*65536+15, 10*65536+15, 5, 0xffffff, 0x000000, 0x80000000, SET_1, 110b};
checkbox2 RealFileNamesCase_chb = {10*65536+15, 30*65536+15, 5, 0xffffff, 0x000000, 0x80000000, SET_2, 100b};

void settings_dialog()
{   
	byte id;
	unsigned int key;
	dword eolite_ini_path = abspath("Eolite.ini");
	IF (active_about) ExitProcess();
	active_about=1;
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (id==10)
				{
					IF ( asm test ShowDeviceName_chb.flags, 2) ini_set_int stdcall (eolite_ini_path, "Config", "ShowDeviceName", 1);
					ELSE  ini_set_int stdcall (eolite_ini_path, "Config", "ShowDeviceName", 0);
					IF ( asm test RealFileNamesCase_chb.flags, 2) ini_set_int stdcall (eolite_ini_path, "Config", "RealFileNamesCase", 1);
					ELSE ini_set_int stdcall (eolite_ini_path, "Config", "RealFileNamesCase", 0);
					IF (LineHeight_ed.size) ini_set_int stdcall (eolite_ini_path, "Config", "LineHeight", atoi(#lineh_s));
					active_about=0;
					ExitProcess();
				}					
				IF (id==1) || (id==11) 
				{
					active_about=0;
					ExitProcess();
				}
				IF (id==23) RunProgram(BROWSER_PATH, BROWSER_LINK);
				break;
				
		case evKey:
				key = GetKey();
				IF (key==27)
				{
					active_about=0;
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
				DefineAndDrawWindow(600,150,281,228+GetSkinHeight(),0x34,sc.work,TITLE_SETT);
				
				IF (show_dev_name) ShowDeviceName_chb.flags = 110b;
				ELSE  ShowDeviceName_chb.flags = 100b;
				
				IF (real_files_names_case) RealFileNamesCase_chb.flags = 110b;
				ELSE RealFileNamesCase_chb.flags = 100b;
				
				key = itoa(files.line_h);
				strcpy(#lineh_s, key);
				
				check_box_draw stdcall (#ShowDeviceName_chb);
				check_box_draw stdcall (#RealFileNamesCase_chb);
				edit_box_draw stdcall (#LineHeight_ed);
				WriteText(10, 55, 0x80, 0x000000, SET_3);
				DrawFlatButton(115,190,70,22,10,0xE4DFE1, APPLY_T);
				DrawFlatButton(195,190,70,22,11,0xE4DFE1, CANCEL_T);
	}
}