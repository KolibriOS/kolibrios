#ifdef LANG_RUS
	?define WINDOW_TITLE_PROPERTIES "Свойства"
	?define BTN_CLOSE "Закрыть"
	?define PR_T_NAME "Имя:"
	?define PR_T_DEST "Расположение:"
	?define PR_T_SIZE "Размер:"
	?define SET_3 "Создан:"
	?define SET_4 "Открыт:"
	?define SET_5 "Изменен:"
	?define SET_6 "Файлов: "
	?define SET_7 " Папок: "
	?define PR_T_CONTAINS "Содержит: "
	?define FLAGS " Аттрибуты "
	?define PR_T_HIDDEN "Скрытый"
	?define PR_T_SYSTEM "Системный"
	?define PR_T_ONLY_READ "Только чтение"
#else
	?define WINDOW_TITLE_PROPERTIES "Properties"
	?define BTN_CLOSE "Close"
	?define PR_T_NAME "Name:"
	?define PR_T_DEST "Destination:"
	?define PR_T_SIZE "Size:"
	?define SET_3 "Created:"
	?define SET_4 "Opened:"
	?define SET_5 "Modified:"
	?define SET_6 "Files: "
	?define SET_7 " Folders: "
	?define PR_T_CONTAINS "Contains: "
	?define FLAGS " Attributes "
	?define PR_T_HIDDEN "Hidden"
	?define PR_T_SYSTEM "System"
	?define PR_T_ONLY_READ "Read-only"
#endif

dword mouse_ddd2;
char path_to_file[4096]="\0";
char file_name2[4096]="\0";
edit_box file_name_ed = {195,50,25,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#file_name2,#mouse_ddd2, 1000000000000000b,2,2};
edit_box path_to_file_ed = {145,100,46,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#path_to_file,#mouse_ddd2, 1000000000000000b,2,2};
frame flags_frame = { 0, 280, 10, 83, 151, 0x000111, 0xFFFfff, 1, FLAGS, 0, 0, 6, 0x000111, 0xCCCccc };

int file_count, dir_count, size_dir;
char folder_info[200];
BDVK file_info_general;
BDVK file_info_dirsize;


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
				GetFileInfo(#cur_file, #file_info_dirsize);
				size_dir = size_dir + file_info_dirsize.sizelo;
				file_count++;
			}
		}
	}
}

void GetSizeMoreFiles(dword way)
{
	int all_file_count, all_dir_count, all_size;
	char cur_file[4096];
	dword selected_offset2;
	
	all_file_count = 0;
	all_dir_count = 0; 
	all_size = 0;
	
	for (i=0; i<files.count; i++) 
    {
        selected_offset2 = file_mas[i]*304 + buf+32 + 7;
        if (ESBYTE[selected_offset2]) {
            strcpy(#cur_file, way);
            strcat(#cur_file, file_mas[i]*304+buf+72);

			GetFileInfo(#cur_file, #file_info_general);
			if ( file_info_general.isfolder )
			{
				GetSizeDir(#cur_file);
				all_file_count = all_file_count + file_count;
				all_dir_count = all_dir_count + dir_count +1;
				all_size = all_size + size_dir;
			}
			else
			{
				all_file_count++;
				all_size = all_size + file_info_general.sizelo;
			}
        }
	}  
	file_count = all_file_count;
	dir_count = all_dir_count;
	size_dir = all_size;
}

void properties_dialog()
{
	byte id;
	byte key;
	dword file_name_off;
	dword element_size;
	dword selected_offset2;
	char element_size_label[32];
	proc_info settings_form;
	
	strcpy(#folder_info, "\0");
	file_count = 0;
	dir_count = 0;	
	size_dir = 0;
			
	if (selected_count) GetSizeMoreFiles(#path);
	else
	{
		GetFileInfo(#file_path, #file_info_general);
		strcpy(#file_name2, #file_name);
		file_name_ed.size = strlen(#file_name2);	
		if (itdir) GetSizeDir(#file_path);
	}
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
				DefineAndDrawWindow(Form.left + 150,150,270,285+GetSkinHeight(),0x34,sc.work,WINDOW_TITLE_PROPERTIES);
				GetProcessInfo(#settings_form, SelfInfo);
				DrawFlatButton(settings_form.cwidth - 70 - 13, settings_form.cheight - 34, 70, 22, 10, 0xE4DFE1, BTN_CLOSE);
				DrawBar(10, 10, 32, 32, 0xFFFfff);
				
				WriteText(10, 50, 0x80, 0x000000, PR_T_DEST);
				edit_box_draw stdcall (#path_to_file_ed);

				WriteText(10, 65, 0x80, 0x000000, PR_T_SIZE);
				
				if (selected_count)
				{
					Put_icon('', 18, 20, 0xFFFfff, 0);
					strcpy(#folder_info, SET_6);
					strcat(#folder_info, itoa(file_count));
					strcat(#folder_info, SET_7);
					strcat(#folder_info, itoa(dir_count));
					WriteText(50, 23, 0x80, 0x000000, #folder_info);
					EAX = ConvertSize(size_dir);
					strcpy(#element_size_label, EAX);
					strcat(#element_size_label, " (");
					strcat(#element_size_label, itoa(size_dir));
					strcat(#element_size_label, " b)");
					WriteText(100, 65, 0x80, 0x000000, #element_size_label);
				}
				else
				{
					if ( file_info_general.isfolder ) 
						Put_icon("<DIR>", 18, 20, 0xFFFfff, 0);
					else 
						Put_icon(#file_name2+strrchr(#file_name2,'.'), 18, 20, 0xFFFfff, 0);

					WriteText(50, 13, 0x80, 0x000000, PR_T_NAME);				
					edit_box_draw stdcall (#file_name_ed);
					
					if (!itdir)
					{
						element_size = file_info_general.sizelo;
					}
					else
					{
						WriteText(10, 80, 0x80, 0x000000, PR_T_CONTAINS);				
						strcpy(#folder_info, SET_6);
						strcat(#folder_info, itoa(file_count));
						strcat(#folder_info, SET_7);
						strcat(#folder_info, itoa(dir_count));
						WriteText(100, 80, 0x80, 0x000000, #folder_info);
						element_size = size_dir;
					}
	
					WriteText(10,  95, 0x80, 0x000000, SET_3);
					WriteText(10, 110, 0x80, 0x000000, SET_4);
					WriteText(10, 125, 0x80, 0x000000, SET_5);
					DrawDate(100,  95, 0, #file_info_general.datecreate);
					DrawDate(100, 110, 0, #file_info_general.datelastaccess);
					DrawDate(100, 125, 0, #file_info_general.datelastedit);
	
					EAX = ConvertSize(element_size);
					strcpy(#element_size_label, EAX);
					strcat(#element_size_label, " (");
					strcat(#element_size_label, itoa(element_size));
					strcat(#element_size_label, " b)");
					WriteText(100, 65, 0x80, 0x000000, #element_size_label);
	
					flags_frame.size_x = - flags_frame.start_x * 2 + settings_form.cwidth - 2;
					flags_frame.font_color = sc.work_text;
					flags_frame.font_backgr_color = sc.work;
					flags_frame.ext_col = sc.work_graph;
					frame_draw stdcall (#flags_frame);
	
					DrawPropertiesCheckBoxes();
				}
	}
}

void DrawPropertiesCheckBoxes()
{
	CheckBox2(22, flags_frame.start_y + 14, 20, PR_T_ONLY_READ,  file_info_general.readonly);
	CheckBox2(22, flags_frame.start_y + 36, 21, PR_T_HIDDEN,  file_info_general.hidden);
	CheckBox2(22, flags_frame.start_y + 58, 22, PR_T_SYSTEM,  file_info_general.system);
}