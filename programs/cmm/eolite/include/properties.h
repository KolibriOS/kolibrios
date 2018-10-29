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

char path_to_file[4096];
char file_name2[4096];
edit_box file_name_ed = {230,59,32,0xffffff,0x94AECE,0xFFFfff,0xffffff,0x10000000,sizeof(file_name2)-2,#file_name2,NULL, 0b,2,2};
edit_box path_to_file_ed = {160,120,79,0xffffff,0x94AECE,0xFFFfff,0xffffff,2,sizeof(path_to_file)-2,#path_to_file,NULL, 0b,2,2};

BDVK file_info_general;
BDVK file_info_dirsize;

bool quest_active;

_dir_size more_files_count;

checkbox ch_read_only = { PR_T_ONLY_READ, NULL };
checkbox ch_hidden = { PR_T_HIDDEN, NULL };
checkbox ch_system = { PR_T_SYSTEM, NULL };

void SetPropertiesFile(dword cur_file, bdvk_pointer)
{
	GetFileInfo(cur_file, bdvk_pointer);
	ESI = bdvk_pointer;
	ESI.BDVK.readonly = ch_read_only.checked;
	ESI.BDVK.hidden = ch_hidden.checked;
	ESI.BDVK.system = ch_system.checked;
	SetFileInfo(cur_file, bdvk_pointer);
}

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
			SetPropertiesFile(cur_file, #file_info_dirsize);
		}
		free(cur_file);
	}
}

void SetProperties(byte prop)
{
	dword cur_file;
	dword i;

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
					SetPropertiesDir(cur_file);
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
			SetPropertiesFile(#file_path, #file_info_general);
			if (prop==2) SetPropertiesDir(#file_path);
		}
		quest_active = 0;
		DrawPropertiesWindow();
	}
	else
	{
		SetPropertiesFile(#file_path, #file_info_general);
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
	DrawStandartCaptButton(62,138,301,T_YES);
	DrawStandartCaptButton(155,138,302,T_NO);
}

void GetSizeMoreFiles(dword way)
{
	char cur_file[4096];
	dword i;
	
	for (i=0; i<files.count; i++) 
	{
		if (getElementSelectedFlag(i) == true) 
		{
			sprintf(#cur_file,"%s/%s",way,file_mas[i]*304+buf+72);
			if (TestBit(ESDWORD[file_mas[i]*304+buf+32], 4) )
			{
				more_files_count.calculate_loop(#cur_file);
				more_files_count.folders++;
			}
			else
			{
				GetFileInfo(#cur_file, #file_info_dirsize);
				more_files_count.bytes += file_info_dirsize.sizelo;
				more_files_count.files++;
			}
		}
	}  
}

void properties_dialog()
{
	int id;
	
	if (selected_count)
	{
		more_files_count.get(NULL);
		GetSizeMoreFiles(#path);
		ch_read_only.checked = 0;
		ch_hidden.checked = 0;
		ch_system.checked = 0;
	}
	else
	{
		GetFileInfo(#file_path, #file_info_general);
		strcpy(#file_name2, #file_name);
		EditBox_UpdateText(#file_name_ed, 0);
		if(itdir) dir_size.get(#file_path);
		ch_read_only.checked = file_info_general.readonly;
		ch_hidden.checked = file_info_general.hidden;
		ch_system.checked = file_info_general.system;
	}
	strcpy(#path_to_file, #path);
	path_to_file_ed.size = strlen(#path_to_file);
	
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				ch_read_only.click(id);
				ch_hidden.click(id);
				ch_system.click(id);
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
	proc_info settings_form;
	char element_size_label[32];
	char folder_info[200];
	dword ext1;
	dword element_size;
	incn y;
	char temp_path[sizeof(file_path)];
	DefineAndDrawWindow(Form.left + 150,150,315,360+skin_height,0x34,system.color.work,WINDOW_TITLE_PROPERTIES,0);
	GetProcessInfo(#settings_form, SelfInfo);

	DrawStandartCaptButton(settings_form.cwidth - 96, settings_form.cheight-34, 10, BTN_CLOSE);
	DrawStandartCaptButton(settings_form.cwidth -208, settings_form.cheight-34, 11, BTN_APPLY);
	
	WriteText(10, 78, 0x90, system.color.work_text, PR_T_DEST);
	edit_box_draw stdcall (#path_to_file_ed);

	WriteText(10, 97, 0x90, system.color.work_text, PR_T_SIZE);
	
	if (selected_count)
	{
		PropertiesDrawIcon(NULL, "<lot>");
		sprintf(#folder_info,"%s%d%s%d",SET_6,more_files_count.files,SET_7,more_files_count.folders);
		WriteText(file_name_ed.left+4, 30, 0x90, system.color.work_text, #folder_info);
		sprintf(#element_size_label,"%s (%d %s)",ConvertSize64(more_files_count.bytes, NULL),more_files_count.bytes,SET_BYTE_LANG);
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
			sprintf(#folder_info,"%s%d%s%d",SET_6,dir_size.files,SET_7,dir_size.folders);
			WriteText(120, 116, 0x90, system.color.work_text, #folder_info);
			element_size = dir_size.bytes;
		}
		WriteTextLines(10,  136, 0x90, system.color.work_text, CREATED_OPENED_MODIFIED, 20);
		DrawDate(120,  136, system.color.work_text, #file_info_general.datecreate);
		DrawDate(120, 156, system.color.work_text, #file_info_general.datelastaccess);
		DrawDate(120, 176, system.color.work_text, #file_info_general.datelastedit);

		sprintf(#element_size_label,"%s (%d %s)",ConvertSize64(element_size, NULL),element_size,SET_BYTE_LANG);
		WriteText(120, 99, 0x90, system.color.work_text, #element_size_label);
	}
	DrawFrame(10, 212, -10*2 + settings_form.cwidth - 2, 92, FLAGS);
	y.n = 212; //212 => attributes_frame.y
	ch_read_only.draw(24, y.inc(18));
	ch_hidden.draw(24, y.inc(24));
	ch_system.draw(24, y.inc(24));
}

void PropertiesDrawIcon(dword file_path, extension)
{
	#define ICON_PADDING 11
	DrawBar(20-ICON_PADDING, 30-ICON_PADDING-1, ICON_PADDING*2+16, ICON_PADDING*2+16, 0xFFFfff);
	DrawIconByExtension(file_path, extension, -icon_size/2+28, -icon_size/2+38, 0xFFFfff);
}
