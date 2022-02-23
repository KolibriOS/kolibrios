
char path_to_file[PATHLEN];
char file_name2[PATHLEN];
edit_box file_name_ed = {230,59,32,0xffffff,0x94AECE,0xFFFfff,0xffffff,0x10000000,sizeof(file_name2)-2,#file_name2,NULL, 0b,2,2};
edit_box path_to_file_ed = {160,120,79,0xffffff,0x94AECE,0xFFFfff,0xffffff,2,sizeof(path_to_file)-2,#path_to_file,NULL, 0b,2,2};

BDVK file_info_general;
BDVK file_info_dirsize;

bool apply_question_active;

DIR_SIZE more_files_count;
DIR_SIZE dir_size;

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
		cur_file = malloc(PATHLEN);
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			sprintf(cur_file, "%s/%s", way, filename);
			if ( ESDWORD[filename-40] & ATR_FOLDER )
			{
				SetPropertiesDir(cur_file);
			}
			SetPropertiesFile(cur_file, #file_info_dirsize);
		}
		free(cur_file);
	}
}

#define SET_PROPERTIES_SINGLE_FILE 0
#define SET_PROPERTIES_NO_SUBFOLDER 1
#define SET_PROPERTIES_ALL_SUBFOLDER 2
void SetProperties(int mode)
{
	char pcur_file[4096];
	dword i;

	apply_question_active = false;

	if (SET_PROPERTIES_SINGLE_FILE == mode) {
		SetPropertiesFile(#file_path, #file_info_general);
	}

	if (SET_PROPERTIES_ALL_SUBFOLDER == mode)
	|| (SET_PROPERTIES_NO_SUBFOLDER == mode)
	{
		if (getSelectedCount())
		{
			for (i=0; i<files.count; i++) 
			{
				if (getElementSelectedFlag(i) == true) 
				{
					sprintf(#pcur_file,"%s/%s",path,items.get(i)*304+buf+72);
					SetPropertiesFile(#pcur_file, #file_info_general);
					if (SET_PROPERTIES_ALL_SUBFOLDER == mode) {
						if (dir_exists(#pcur_file)) SetPropertiesDir(#pcur_file);
					}
				}
			}
		}
		else
		{
			SetPropertiesFile(#file_path, #file_info_general);
			if (SET_PROPERTIES_ALL_SUBFOLDER == mode) SetPropertiesDir(#file_path);
		}
	}

	cmd_free=3;
	OpenDir(ONLY_OPEN);
	ExitProcess();
}

void ShowConfirmQuestionPopin()
{
	apply_question_active = true;
	DrawPopup(15,80,250,90,1,sc.work, sc.line);
	WriteText(35, 102, 0x90, 0x000000, QUEST_1);
	WriteText(65, 117, 0x90, 0x000000, QUEST_2);
	DrawStandartCaptButton(62,138,B_SETINGS_APPLY_SUBFOLDER,T_YES);
	DrawStandartCaptButton(155,138,B_SETINGS_APPLY_NO_SUBFOLDER,T_NO);
}

void GetSizeMoreFiles(dword way)
{
	char cur_file[4096];
	dword i;
	
	for (i=0; i<files.count; i++) 
	{
		if (getElementSelectedFlag(i) == true) 
		{
			sprintf(#cur_file,"%s/%s",way,items.get(i)*304+buf+72);
			if (ESDWORD[items.get(i)*304+buf+32] & ATR_FOLDER )
			{
				more_files_count.calculate_loop(#cur_file);
				more_files_count.folders++;
			}
			else
			{
				GetFileInfo(#cur_file, #file_info_dirsize);
				more_files_count.sizelo += file_info_dirsize.sizelo;
				more_files_count.sizehi += file_info_dirsize.sizehi;
				more_files_count.files++;
			}
		}
	}  
}

void properties_dialog()
{
	int id;
	
	if (getSelectedCount())
	{
		more_files_count.get(NULL);
		GetSizeMoreFiles(path);
		ch_read_only.checked = 0;
		ch_hidden.checked = 0;
		ch_system.checked = 0;
	}
	else
	{
		GetFileInfo(#file_path, #file_info_general);
		edit_box_set_text stdcall (#file_name_ed, #file_name);
		if(itdir) dir_size.get(#file_path);
		ch_read_only.checked = file_info_general.readonly;
		ch_hidden.checked = file_info_general.hidden;
		ch_system.checked = file_info_general.system;
	}
	edit_box_set_text stdcall (#path_to_file_ed, path);
	
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				ch_read_only.click(id);
				ch_hidden.click(id);
				ch_system.click(id);
				if (apply_question_active)
				{
					IF (id==B_SETINGS_APPLY_SUBFOLDER) 
						SetProperties(SET_PROPERTIES_ALL_SUBFOLDER);
					IF (id==B_SETINGS_APPLY_NO_SUBFOLDER) 
						SetProperties(SET_PROPERTIES_NO_SUBFOLDER);
					break;
				}
				if (id==1) || (id==B_CLOSE)
				{
					cmd_free=3;
					ExitProcess();
				}
				if (id==B_APPLY) goto _APPLY_PROPERTIES;
				break;
				
		case evMouse:
				edit_box_mouse stdcall (#file_name_ed);
				edit_box_mouse stdcall (#path_to_file_ed);
				break;
			
		case evKey:
				GetKeys();

				if (apply_question_active)
				{
					IF (key_scancode==SCAN_CODE_ENTER) 
						SetProperties(SET_PROPERTIES_ALL_SUBFOLDER);
					IF (key_scancode==SCAN_CODE_ESC) 
						SetProperties(SET_PROPERTIES_NO_SUBFOLDER);
					break;
				}

				switch(key_scancode)
				{
					case SCAN_CODE_ESC:
						cmd_free=3;
						ExitProcess();
						break;

					case SCAN_CODE_ENTER:
						_APPLY_PROPERTIES:
						if (getSelectedCount()) || (itdir) {
							ShowConfirmQuestionPopin();
						} else {
							SetProperties(SET_PROPERTIES_SINGLE_FILE);
						}
						break;

					case SCAN_CODE_KEY_A:
					case SCAN_CODE_KEY_C:
						if (key_modifier & KEY_LCTRL) || (key_modifier & KEY_RCTRL) { 
							edit_box_key_c stdcall(#file_name_ed,key_editbox);
							edit_box_key_c stdcall(#path_to_file_ed,key_editbox);
						}
				}
				break;
				
		case evReDraw:
				DrawPropertiesWindow();
	}
}

void DrawPropertiesWindow()
{
	proc_info pform;
	dword ext1;
	incn y;
	char temp_path[PATHLEN];
	bool show_date = false;

	dword p_t_formated_size;
	dword p_q_size_bytes;
	dword t_contains_files_and_folders[200];

	if (chrnum(path, '/')>1) || (streq(path, "/kolibrios")) || (streq(path, "/sys")) show_date = true;
	if (getSelectedCount()) show_date = false;

	DefineAndDrawWindow(Form.left + 150,150,315,show_date*60+342+skin_h,0x34,sc.work,WINDOW_TITLE_PROPERTIES,0);
	GetProcessInfo(#pform, SelfInfo);

	DrawStandartCaptButton(pform.cwidth - 96, pform.cheight-34, B_CLOSE, T_CLOSE);
	DrawStandartCaptButton(pform.cwidth -208, pform.cheight-34, B_APPLY, T_APPLY);
	
	if (getSelectedCount())
	{
		PropertiesDrawIcon(NULL, "<lot>");
		WriteText(file_name_ed.left+4, 30, 0x90, sc.work_text, T_BULK_SELECTION);
		p_t_formated_size = ConvertSize64(more_files_count.sizelo, more_files_count.sizehi);
		p_q_size_bytes = #more_files_count.sizelo;
		sprintf(#t_contains_files_and_folders,T_FILES_FOLDERS,more_files_count.files,more_files_count.folders);
	} else {
		WriteText(file_name_ed.left, file_name_ed.top-15, 0x80, sc.work_text, T_NAME);
		DrawEditBox(#file_name_ed);
		if (itdir) {
			PropertiesDrawIcon(NULL, "<DIR>");
			p_t_formated_size = ConvertSize64(dir_size.sizelo, dir_size.sizehi);
			p_q_size_bytes = #dir_size.sizelo;
			sprintf(#t_contains_files_and_folders,T_FILES_FOLDERS,dir_size.files,dir_size.folders);
		} else {
			sprintf(#temp_path,"%s/%s",path,#file_name2);
			if (ext1 = strrchr(#file_name2,'.')) ext1 += #file_name2;
			PropertiesDrawIcon(#temp_path, ext1);
			p_t_formated_size = ConvertSize64(file_info_general.sizelo, file_info_general.sizehi);
			p_q_size_bytes = #file_info_general.sizelo;
			sprintf(#t_contains_files_and_folders,T_DATA);
		}	
	}
	WriteTextLines(10, y.set(78), 0x90, sc.work_text, T_PATH_SIZE, 20);
	edit_box_draw stdcall (#path_to_file_ed);

	WriteText(120, y.inc(20), 0x90, sc.work_text, p_t_formated_size);
	WriteNumber(120, y.inc(20), 0x90, sc.work_text, 0xc0140001, p_q_size_bytes);
	WriteText(120, y.inc(20), 0x90, sc.work_text, #t_contains_files_and_folders);

	if (show_date) {
		WriteTextLines(10, y.inc(32), 0x90, sc.work_text, CREATED_OPENED_MODIFIED, 20);
		DrawDateTime(120, y.n, sc.work_text, #file_info_general.datecreate, #file_info_general.timecreate);
		DrawDateTime(120, y.inc(20), sc.work_text, #file_info_general.datelastaccess, #file_info_general.timelastaccess);
		DrawDateTime(120, y.inc(20), sc.work_text, #file_info_general.datelastedit, #file_info_general.timelastedit);			
	}

	DrawFrame(10, y.set(pform.cheight - 143), -10*2 + pform.cwidth - 2, 92, FLAGS);
	ch_read_only.draw(24, y.inc(18));
	ch_hidden.draw(24, y.inc(24));
	ch_system.draw(24, y.inc(24));
	if (apply_question_active) ShowConfirmQuestionPopin();
}

void PropertiesDrawIcon(dword file_path, extension)
{
	int icon_n = ini_icons.get(file_path, extension, 32);
	draw_icon_32(12, 22, sc.work, icon_n);
}
