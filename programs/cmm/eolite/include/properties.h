#ifdef LANG_RUS
	?define WINDOW_TITLE_PROPERTIES "Свойства"
	?define BTN_CLOSE "Закрыть"
	?define BTN_APPLY "Применить"
	?define QUEST_1 "Применить ко всем вложенным"
	?define QUEST_2 "файлам и папкам?"
	?define PR_T_NAME "Имя:"
	?define PR_T_DEST "Расположение:"
	?define PR_T_SIZE "Размер:"
	?define CREATED_OPENED_MODIFIED "Создан:\nОткрыт:\nИзменен:"
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
	?define QUEST_2 "files and Folders?"
	?define PR_T_NAME "Name:"
	?define PR_T_DEST "Destination:"
	?define PR_T_SIZE "Size:"
	?define CREATED_OPENED_MODIFIED "Created:\nOpened:\nModified:"
	?define SET_6 "Files: "
	?define SET_7 " Folders: "
	?define PR_T_CONTAINS "Contains: "
	?define FLAGS " Attributes "
	?define PR_T_HIDDEN "Hidden"
	?define PR_T_SYSTEM "System"
	?define PR_T_ONLY_READ "Read-only"
	?define SET_BYTE_LANG "byte"
#endif

dword mouse_2;
char path_to_file[4096];
char file_name2[4096];
edit_box file_name_ed = {230,59,32,0xffffff,0x94AECE,0xFFFfff,0xffffff,0x10000000,sizeof(file_name2),#file_name2,#mouse_2, 1000000000000000b,2,2};
edit_box path_to_file_ed = {160,120,79,0xffffff,0x94AECE,0xFFFfff,0xffffff,2,sizeof(path_to_file),#path_to_file,#mouse_2, 1000000000000000b,2,2};

int file_count, dir_count, size_dir;
char folder_info[200];
dword element_size;
char element_size_label[32];
BDVK file_info_general;
BDVK file_info_dirsize;

proc_info settings_form;
bool quest_active, atr_readonly, atr_hidden, atr_system;

void SetPropertiesDir(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	if (dir_exists(way))
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

	if (prop==1) || (prop==2)
	{
		if (selected_count)
		{
			cur_file = malloc(4096);
			for (i=0; i<files.count; i++) 
			{
				if (getElementSelectedFlag(i) == true) 
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
						if (dir_exists(cur_file))
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

void ShowConfirmQuestionPopin()
{
	quest_active = 1;
	DrawPopup(15,80,250,90,1,system.color.work, system.color.work_graph);
	WriteText(35, 102, 0x90, 0x000000, QUEST_1);
	WriteText(65, 117, 0x90, 0x000000, QUEST_2);
	DrawFlatButton(62,138,301,T_YES);
	DrawFlatButton(155,138,302,T_NO);
}

void GetSizeDir(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	if (dir_exists(way))
	{
		cur_file = malloc(4096);
		// In the process of recursive descent, memory must be allocated dynamically, 
		// because the static memory -> was a bug !!! But unfortunately pass away to sacrifice speed.
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
	
	for (i=0; i<files.count; i++) 
	{
		if (getElementSelectedFlag(i) == true) 
		{
			sprintf(#cur_file,"%s/%s",way,file_mas[i]*304+buf+72);
			if (TestBit(ESDWORD[file_mas[i]*304+buf+32], 4) )
			{
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
	
	DSBYTE[#folder_info]=0;
	file_count = 0;
	dir_count = 0;	
	size_dir = 0;
			
	if (selected_count)
	{
		GetSizeMoreFiles(#path);
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
	
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				if (quest_active)
				{
					IF (id==301) SetProperties(2);
					IF (id==302) SetProperties(1);
					quest_active=false;
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
						ShowConfirmQuestionPopin();
					}
					else 
					{
						SetProperties(0);
					}
					break;
				}
				if (id==20)
				{
					atr_readonly ^= 1;
					DrawPropertiesCheckBoxes();
				}
				if (id==21)
				{
					atr_hidden ^= 1;
					DrawPropertiesCheckBoxes();
				}
				if (id==22)
				{
					atr_system ^= 1;
					DrawPropertiesCheckBoxes();
				}
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
					quest_active=false;
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
						ShowConfirmQuestionPopin();
					}
					else 
					{
						SetProperties(0);
					}
					break;
				}
				edit_box_key stdcall(#file_name_ed);
				edit_box_key stdcall(#path_to_file_ed);
				break;
				
		case evReDraw:
				DrawPropertiesWindow();
	}
}

void DrawPropertiesWindow()
{
	dword ext1;
	char temp_path[sizeof(file_path)];
	DefineAndDrawWindow(Form.left + 150,150,315,360+skin_height,0x34,system.color.work,WINDOW_TITLE_PROPERTIES,0);
	GetProcessInfo(#settings_form, SelfInfo);

	DrawFlatButton(settings_form.cwidth - 96, settings_form.cheight-34, 10, BTN_CLOSE);
	DrawFlatButton(settings_form.cwidth -208, settings_form.cheight-34, 11, BTN_APPLY);
	
	WriteText(10, 78, 0x90, system.color.work_text, PR_T_DEST);
	edit_box_draw stdcall (#path_to_file_ed);

	WriteText(10, 97, 0x90, system.color.work_text, PR_T_SIZE);
	
	if (selected_count)
	{
		PropertiesDrawIcon(NULL, "<lot>");
		sprintf(#folder_info,"%s%d%s%d",SET_6,file_count,SET_7,dir_count);
		WriteText(file_name_ed.left+4, 30, 0x90, system.color.work_text, #folder_info);
		sprintf(#element_size_label,"%s (%d %s)",ConvertSize64(size_dir, NULL),size_dir,SET_BYTE_LANG);
		WriteText(120, 97, 0x90, system.color.work_text, #element_size_label);
	}
	else
	{
		if ( file_info_general.isfolder )
				PropertiesDrawIcon(NULL, "<DIR>");
		else {
			sprintf(#temp_path,"%s/%s",#path,#file_name2);
			ext1 = strrchr(#file_name2,'.');
			if (ext1) ext1 += #file_name2;
			PropertiesDrawIcon(#temp_path, ext1);
		}
		WriteText(file_name_ed.left, file_name_ed.top-15, 0x80, system.color.work_text, PR_T_NAME);
		DrawEditBox(#file_name_ed);
		
		if (!itdir) element_size = file_info_general.sizelo;
		else
		{
			WriteText(10,116, 0x90, system.color.work_text, PR_T_CONTAINS);                              
			sprintf(#folder_info,"%s%d%s%d",SET_6,file_count,SET_7,dir_count);
			WriteText(120, 116, 0x90, system.color.work_text, #folder_info);
			element_size = size_dir;
		}
		WriteTextLines(10,  136, 0x90, system.color.work_text, CREATED_OPENED_MODIFIED, 20);
		DrawDate(120,  136, system.color.work_text, #file_info_general.datecreate);
		DrawDate(120, 156, system.color.work_text, #file_info_general.datelastaccess);
		DrawDate(120, 176, system.color.work_text, #file_info_general.datelastedit);

		sprintf(#element_size_label,"%s (%d %s)",ConvertSize64(element_size, NULL),element_size,SET_BYTE_LANG);
		WriteText(120, 99, 0x90, system.color.work_text, #element_size_label);
	}
	DrawFrame(10, 212, -10*2 + settings_form.cwidth - 2, 92, FLAGS);
	DrawPropertiesCheckBoxes();
}

void PropertiesDrawIcon(dword file_path, extension)
{
	#define ICON_PADDING 11
	DrawBar(20-ICON_PADDING, 30-ICON_PADDING-1, ICON_PADDING*2+16, ICON_PADDING*2+16, 0xFFFfff);
	DrawIconByExtension(file_path, extension, 20, 30, system.color.work_light);
}

void DrawPropertiesCheckBoxes()
{
	incn y;
	y.n = 212; //212 => attributes_frame.y
	CheckBox(24, y.inc(18), 20, PR_T_ONLY_READ, atr_readonly);
	CheckBox(24, y.inc(24), 21, PR_T_HIDDEN, atr_hidden);
	CheckBox(24, y.inc(24), 22, PR_T_SYSTEM, atr_system);
}
