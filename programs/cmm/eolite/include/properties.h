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
	?define SET_BYTE_LANG "байт"
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
	?define SET_BYTE_LANG "byte"
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

void SetProperties(byte id)
{
	if (selected_count) return;
	else
	{
		if (id==20) file_info_general.readonly ^= true;
		if (id==21) file_info_general.hidden ^= true;
		if (id==22) file_info_general.system ^= true;
		SetFileInfo(#file_path, #file_info_general);
	}
}

void GetSizeDir(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	if (isdir(way))
	{
		cur_file = malloc(4096);
		// In the process of recursive descent, memory must be allocated dynamically, because the static memory -> was a bug !!! But unfortunately pass away to sacrifice speed.
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			sprintf(cur_file,"%s/%s",way,filename);
			
			if (TestBit(ESDWORD[filename-40], 4) )
			{
				dir_count++;
				GetSizeDir(cur_file);
			}
			else
			{
				GetFileInfo(cur_file, #file_info_dirsize);
				size_dir += file_info_dirsize.sizelo;
				file_count++;
			}
		}
		free(cur_file);
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
			sprintf(#cur_file,"%s/%s",way,file_mas[i]*304+buf+72);

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
	char element_size_label[32],tmp;
	proc_info settings_form;
	
	DSBYTE[#folder_info]=0;
	file_count = 0;
	dir_count = 0;	
	size_dir = 0;
			
	if (selected_count) GetSizeMoreFiles(#path);
	else
	{
		GetFileInfo(#file_path, #file_info_general);
		strcpy(#file_name2, #file_name);
		file_name_ed.size = strlen(#file_name2);   
		if(itdir) GetSizeDir(#file_path);
	}
	strcpy(#path_to_file, #path);
	path_to_file_ed.size = strlen(#path_to_file);
	
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (id==1) || (id==10)
				{
					cmd_free=3;
					ExitProcess();
				}
				if (id==20) SetProperties(id);
				if (id==21)
				{
					SetProperties(id);
					_not_draw = true;
					Open_Dir(#path,WITH_REDRAW);
					_not_draw = false;
					EventRedrawWindow(Form.left,Form.top);
				}
				if (id==22) SetProperties(id);
				
				DrawPropertiesCheckBoxes();
				break;
				
		case evMouse:
				edit_box_mouse stdcall (#file_name_ed);
				edit_box_mouse stdcall (#path_to_file_ed);
				break;
			
		case evKey:
				key = GetKey();
				if (key==27)
				{
					cmd_free=3;
					ExitProcess();
				}
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
					Put_icon('', 18, 19, 0xFFFfff, 0);
					sprintf(#folder_info,"%s%d%s%d",SET_6,file_count,SET_7,dir_count);
					WriteText(50, 23, 0x80, 0x000000, #folder_info);
					sprintf(#element_size_label,"%s (%d %s)",ConvertSize(size_dir),size_dir,SET_BYTE_LANG);
					WriteText(100, 65, 0x80, 0x000000, #element_size_label);
				}
				else
				{
					if ( file_info_general.isfolder )
							Put_icon("<DIR>", 18, 19, 0xFFFfff, 0);
					else
							Put_icon(#file_name2+strrchr(#file_name2,'.'), 18, 19, 0xFFFfff, 0);

					WriteText(50, 13, 0x80, 0x000000, PR_T_NAME);                          
					edit_box_draw stdcall (#file_name_ed);
					
					if (!itdir) element_size = file_info_general.sizelo;
					else
					{
						WriteText(10, 80, 0x80, 0x000000, PR_T_CONTAINS);                              
						sprintf(#folder_info,"%s%d%s%d",SET_6,file_count,SET_7,dir_count);
						WriteText(100, 80, 0x80, 0x000000, #folder_info);
						element_size = size_dir;
					}
					WriteText(10,  95, 0x80, 0x000000, SET_3);
                    WriteText(10, 110, 0x80, 0x000000, SET_4);
                    WriteText(10, 125, 0x80, 0x000000, SET_5);
					DrawDate(100,  95, 0, #file_info_general.datecreate);
                    DrawDate(100, 110, 0, #file_info_general.datelastaccess);
                    DrawDate(100, 125, 0, #file_info_general.datelastedit);

					sprintf(#element_size_label,"%s (%d %s)",ConvertSize(element_size),element_size,SET_BYTE_LANG);
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