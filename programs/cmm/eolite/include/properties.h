#ifdef LANG_RUS
	?define PROP_TITLE "Свойства"
	?define BTN_CLOSE "Закрыть"
	?define SET_1 "Расположение"
	?define SET_2 "Размер"
	?define SET_3 "Создан"
	?define SET_4 "Открыт"
	?define SET_5 "Изменен"
	?define SET_6 "Файлов: "
	?define SET_7 " Папок: "
	?define SET_8 "Содержит "
	?define HIDDEN_T "Скрытый"
	?define SYSTEM_T "Системный"
	?define ONLY_READ_T "Только чтение"
#elif LANG_EST
	?define PROP_TITLE "Свойства"
	?define BTN_CLOSE "Закрыть"
	?define SET_1 "Расположение"
	?define SET_2 "Размер"
	?define SET_3 "Создан"
	?define SET_4 "Открыт"
	?define SET_5 "Изменен"
	?define SET_6 "Файлов: "
	?define SET_7 " Папок: "
	?define SET_8 "Содержит "
	?define HIDDEN_T "Скрытый"
	?define SYSTEM_T "Системный"
	?define ONLY_READ_T "Только чтение"
#else
	?define PROP_TITLE "Свойства"
	?define BTN_CLOSE "Закрыть"
	?define SET_1 "Расположение"
	?define SET_2 "Размер"
	?define SET_3 "Создан"
	?define SET_4 "Открыт"
	?define SET_5 "Изменен"
	?define SET_6 "Файлов: "
	?define SET_7 " Папок: "
	?define SET_8 "Содержит "
	?define HIDDEN_T "Скрытый"
	?define SYSTEM_T "Системный"
	?define ONLY_READ_T "Только чтение"
#endif

dword mouse_ddd2;
char path_to_file[4096]="\0";
char file_name2[4096]="\0";
edit_box file_name_ed = {100,30,5,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#file_name2,#mouse_ddd2, 1000000000000000b,2,2};
edit_box path_to_file_ed = {100,100,27,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#path_to_file,#mouse_ddd2, 1000000000000000b,2,2};
checkbox2 HIDDEN_chb = {10*65536+15, 115*65536+15, 5, 0xffffff, 0x9098B0, 0x80000000, HIDDEN_T, CH_FLAG_MIDDLE, 0};
checkbox2 SYSTEM_chb = {90*65536+15, 115*65536+15, 5, 0xffffff, 0x9098B0, 0x80000000, SYSTEM_T, CH_FLAG_MIDDLE, 0};
checkbox2 ONLY_READ_chb = {180*65536+15, 115*65536+15, 5, 0xffffff, 0x9098B0, 0x80000000, ONLY_READ_T, CH_FLAG_MIDDLE, 0};

int file_count, dir_count, size_dir;
char folder_info[200];
BDVK file_info2;

void GetSizeDir(dword way)
{
	dword dirbuf, fcount, i, filename;
	char cur_file[4096];
	if (isdir(way))
	{
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			strcpy(#cur_file, way);
			chrcat(#cur_file, '/');
			strcat(#cur_file, filename);
			if ( TestBit(ESDWORD[filename-40], 4) )
			{
				dir_count++;
				GetSizeDir(#cur_file);
			}
			else
			{
				GetFileInfo(#cur_file, #file_info2);
				size_dir = size_dir + file_info2.sizelo;
				file_count++;
			}
		}
	}
}

void properties_dialog()
{
	byte id;
	unsigned int key;
	dword file_name_off;
	BDVK file_info;
	proc_info settings_form;
	
	strcpy(#folder_info, "\0");
	file_count = 0;
	dir_count = 0;	
	size_dir = 0;
	GetFileInfo(#file_path, #file_info);
	strcpy(#file_name2, #file_name);
	file_name_ed.size = strlen(#file_name2);
	strcpy(#path_to_file, #path);
	path_to_file_ed.size = strlen(#path_to_file);
	
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				IF (id==1) || (id==10) ExitProcess();
				break;
				
		case evMouse:
				edit_box_mouse stdcall (#file_name_ed);
				edit_box_mouse stdcall (#path_to_file_ed);
				break;
			
		case evKey:
				key = GetKey();
				IF (key==27) ExitProcess();
				EAX=key<<8;
				edit_box_key stdcall(#file_name_ed);
				edit_box_key stdcall(#path_to_file_ed);
				break;
				
		case evReDraw:
				DefineAndDrawWindow(Form.left + 150,150,300,228+GetSkinHeight(),0x34,sc.work,PROP_TITLE);
				GetProcessInfo(#settings_form, SelfInfo);
				DrawFlatButton(208, settings_form.cheight - 34, 70, 22, 10, 0xE4DFE1, BTN_CLOSE);
				if (! TestBit(file_info.attr, 4) ) Put_icon(#file_name2+strrchr(#file_name2,'.'), 10, 5, sc.work, 0);
				else Put_icon("<DIR>", 10, 5, sc.work, 0);
				WriteText(10, 30, 0x80, 0x000000, SET_1);
				edit_box_draw stdcall (#file_name_ed);
				edit_box_draw stdcall (#path_to_file_ed);
				WriteText(10, 47, 0x80, 0x000000, SET_2);
				/*WriteText(10, 63, 0x80, 0x000000, SET_3);
				if (!itdir)
				{
					WriteText(10, 78, 0x80, 0x000000, SET_4);
					WriteText(10, 93, 0x80, 0x000000, SET_5);					
				}*/
				if (TestBit(file_info.attr, 0)) ONLY_READ_chb.flags = 110b;
				if (TestBit(file_info.attr, 1)) HIDDEN_chb.flags = 110b;
				if (TestBit(file_info.attr, 2)) SYSTEM_chb.flags = 110b;
				if (!itdir)
				{
					WriteText(100, 47, 0x80, 0x000000, ConvertSize(file_info.sizelo));
				}
				else
				{
					WriteText(10, 63, 0x80, 0x000000, SET_8);
					GetSizeDir(#file_path);
					strcat(#folder_info, SET_6);
					strcat(#folder_info, itoa(file_count));
					strcat(#folder_info, SET_7);
					strcat(#folder_info, itoa(dir_count));
					WriteText(100, 63, 0x80, 0x000000, #folder_info);
					WriteText(100, 47, 0x80, 0x000000, ConvertSize(size_dir));
				}
				check_box_draw stdcall (#HIDDEN_chb);
				check_box_draw stdcall (#SYSTEM_chb);
				check_box_draw stdcall (#ONLY_READ_chb);
	}
}