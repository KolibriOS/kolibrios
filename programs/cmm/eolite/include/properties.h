#ifdef LANG_RUS
	?define WINDOW_TITLE_PROPERTIES "Свойства"
	?define BTN_CLOSE "Закрыть"
	?define BTN_APPLY "Применить"
	?define QUEST_1 "Применить ко всем вложенным"
	?define QUEST_2 "файлам и папкам"
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
#else // Apply to all subfolders
	?define WINDOW_TITLE_PROPERTIES "Properties"
	?define BTN_CLOSE "Close"
	?define BTN_APPLY "Apply"
	?define QUEST_1 "Apply to all subfolders"
	?define QUEST_2 "files and Folders"
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
frame flags_frame = { 0, 280, 10, 83, 151, 0x000111, 0xFFFfff, 1, FLAGS, 0, 0, 6, 0x000111, 0xFFFFFF };

int file_count, dir_count, size_dir;
char folder_info[200];
dword element_size;
char element_size_label[32];
BDVK file_info_general;
BDVK file_info_dirsize;

proc_info settings_form;
byte quest_active, atr_readonly, atr_hidden, atr_system;

void SetPropertiesDir(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	if (isdir(way))
	{
		cur_file = malloc(4096);
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			strcpy(cur_file, way);
			chrcat(cur_file, '/');
			strcat(cur_file, filename);
			if ( TestBit(ESDWORD[filename-40], 4) )
			{
				SetPropertiesDir(cur_file);
			}
			GetFileInfo(cur_file, #file_info_dirsize);
			file_info_dirsize.readonly = atr_readonly;
			file_info_dirsize.hidden = atr_hidden;
			file_info_dirsize.system = atr_system;
			SetFileInfo(cur_file, #file_info_dirsize);
		}
		free(cur_file);
	}
}

void SetProperties(byte prop)
{
	dword cur_file;
	dword selected_offset2;

	if (prop==1) || (prop==2)
	{
		if (selected_count)
		{
			cur_file = malloc(4096);
			for (i=0; i<files.count; i++) 
			{
				selected_offset2 = file_mas[i]*304 + buf+32 + 7;
				if (ESBYTE[selected_offset2])
				{
					strcpy(cur_file, #path);
					strcat(cur_file, file_mas[i]*304+buf+72);
					GetFileInfo(cur_file, #file_info_general);
					file_info_general.readonly = atr_readonly;
					file_info_general.hidden = atr_hidden;
					file_info_general.system = atr_system;
					SetFileInfo(cur_file, #file_info_general);
					if (prop==2)
					{
						if (isdir(cur_file))
						{
							SetPropertiesDir(cur_file);
						}
					}
				}
			}
			free(cur_file);
		}
		else
		{
			GetFileInfo(#file_path, #file_info_general);
			file_info_general.readonly = atr_readonly;
			file_info_general.hidden = atr_hidden;
			file_info_general.system = atr_system;
			SetFileInfo(#file_path, #file_info_general);
			if (prop==2) SetPropertiesDir(#file_path);
		}
		quest_active = 0;
		DrawPropertiesWindow();
	}
	else
	{
		GetFileInfo(#file_path, #file_info_general);
		file_info_general.readonly = atr_readonly;
		file_info_general.hidden = atr_hidden;
		file_info_general.system = atr_system;
		SetFileInfo(#file_path, #file_info_general);
	}
	cmd_free=3;
	_not_draw = true;
    Open_Dir(#path,WITH_REDRAW);
    _not_draw = false;
    EventRedrawWindow(Form.left,Form.top);
	ExitProcess();
}

void Quest()
{
	DrawPopup(30,80,200,90,1,system.color.work, system.color.work_graph);
	WriteText(50, 100, 0x80, 0x000000, QUEST_1);
	WriteText(80, 115, 0x80, 0x000000, QUEST_2);
	DrawFlatButton(52,138,70,20,301,0xFFB6B5,T_YES);
	DrawFlatButton(145,138,70,20,302,0xC6DFC6,T_NO);
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
	char cur_file[4096];
	dword selected_offset2;
	
	for (i=0; i<files.count; i++) 
    {
        selected_offset2 = file_mas[i]*304 + buf+32 + 7;
        if (ESBYTE[selected_offset2]) {
			sprintf(#cur_file,"%s%s",way,file_mas[i]*304+buf+72);
			if (TestBit(ESDWORD[file_mas[i]*304+buf+32], 4) )
			{
				debugln(#cur_file);
				GetSizeDir(#cur_file);
				dir_count++;
			}
			else
			{
				GetFileInfo(#cur_file, #file_info_dirsize);
				size_dir += file_info_dirsize.sizelo;
				file_count++;
			}
        }
	}  
}

void properties_dialog()
{
	byte id;
	dword file_name_off;
	dword selected_offset2;
	
	DSBYTE[#folder_info]=0;
	file_count = 0;
	dir_count = 0;	
	size_dir = 0;
			
	if (selected_count)
	{
		GetSizeMoreFiles(#path);
		debugi(size_dir);
		atr_readonly = 0;
		atr_hidden = 0;
		atr_system = 0;
	}
	else
	{
		GetFileInfo(#file_path, #file_info_general);
		strcpy(#file_name2, #file_name);
		file_name_ed.size = strlen(#file_name2);   
		if(itdir) GetSizeDir(#file_path);
		atr_readonly = file_info_general.readonly;
		atr_hidden = file_info_general.hidden;
		atr_system = file_info_general.system;
	}
	strcpy(#path_to_file, #path);
	path_to_file_ed.size = strlen(#path_to_file);
	
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (quest_active)
				{
					IF (id==301) SetProperties(2);
					IF (id==302) SetProperties(1);
					break;
				}
				if (id==1) || (id==10)
				{
					cmd_free=3;
					ExitProcess();
				}
				IF (id==11) 
				{
					if (selected_count) || (itdir)
					{
						quest_active = 1;
						Quest();
					}
					else 
					{
						SetProperties(0);
					}
					break;
				}
				if (id==20) atr_readonly ^= 1;
				if (id==21) atr_hidden ^= 1;
				if (id==22) atr_system ^= 1;
				DrawPropertiesCheckBoxes();
				break;
				
		case evMouse:
				edit_box_mouse stdcall (#file_name_ed);
				edit_box_mouse stdcall (#path_to_file_ed);
				break;
			
		case evKey:
				GetKeys();
				
				if (quest_active)
				{
					IF (key_scancode==SCAN_CODE_ENTER) SetProperties(2);
					IF (key_scancode==SCAN_CODE_ESC) SetProperties(1);
					break;
				}
				if (key_scancode==SCAN_CODE_ESC)
				{
					cmd_free=3;
					ExitProcess();
				}
				if (key_scancode==SCAN_CODE_ENTER)
				{
					if (selected_count) || (itdir)
					{
						quest_active = 1;
						Quest();
					}
					else 
					{
						SetProperties(0);
					}
					break;
				}
				EAX = key_ascii << 8;
				edit_box_key stdcall(#file_name_ed);
				edit_box_key stdcall(#path_to_file_ed);
				break;
				
		case evReDraw:
				DrawPropertiesWindow();
	}
}

void DrawPropertiesWindow()
{
	DefineAndDrawWindow(Form.left + 150,150,270,285+GetSkinHeight(),0x34,0xFFFFFF,WINDOW_TITLE_PROPERTIES);
	GetProcessInfo(#settings_form, SelfInfo);
	DrawFlatButton(settings_form.cwidth - 70 - 13, settings_form.cheight - 34, 70, 22, 10, 0xE4DFE1, BTN_CLOSE);
	DrawFlatButton(settings_form.cwidth - 150 - 13, settings_form.cheight - 34, 70, 22, 11, 0xE4DFE1, BTN_APPLY);
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
	}
	flags_frame.size_x = - flags_frame.start_x * 2 + settings_form.cwidth - 2;
	flags_frame.font_color = system.color.work_text;
	flags_frame.ext_col = system.color.work_graph;
	frame_draw stdcall (#flags_frame);
	DrawPropertiesCheckBoxes();
}

void DrawPropertiesCheckBoxes()
{
	CheckBox2(22, flags_frame.start_y + 14, 20, PR_T_ONLY_READ, atr_readonly);
	CheckBox2(22, flags_frame.start_y + 36, 21, PR_T_HIDDEN, atr_hidden);
	CheckBox2(22, flags_frame.start_y + 58, 22, PR_T_SYSTEM, atr_system);
}