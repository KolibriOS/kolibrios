
byte copy_to[4096];
byte copy_from[4096];
byte cut_active=0;

progress_bar copy_bar = {0,16,49,50,20,0,0,1,0xFFFFFF,0x00FF00,0x000000};

enum {NOCUT, CUT, COPY_PASTE_END};

Clipboard clipboard;

void Copy(dword pcth, char cut)
{
    dword selected_offset2;
    byte copy_t[4096];
    dword buff_data;
    int ind = 0; 
    if (selected_count)
	{
        buff_data = malloc(selected_count*4096+10);
        ESDWORD[buff_data] = selected_count*4096+10;
        ESDWORD[buff_data+4] = 3;
        ESINT[buff_data+8] = selected_count;
        for (i=0; i<files.count; i++) 
        {
            selected_offset2 = file_mas[i]*304 + buf+32 + 7;
            if (ESBYTE[selected_offset2]) {
                strcpy(#copy_t, #path);
                strcat(#copy_t, file_mas[i]*304+buf+72);                         
                strlcpy(ind*4096+buff_data+10, #copy_t, 4096);;
                ind++;
            }
        }
		clipboard.SetSlotData(selected_count*4096+10, buff_data);
	}
	else
	{
		buff_data = malloc(4106);
        ESDWORD[buff_data] = 4106;
        ESDWORD[buff_data+4] = 3;
        ESINT[buff_data+8] = 1;
        strlcpy(buff_data+10, #file_path, 4096);;
		clipboard.SetSlotData(4106, buff_data);
	}
	cut_active = cut;
	free(buff_data);
}


void Paste() {
	copy_stak = malloc(20000);
	CreateThread(#PasteThread,copy_stak+20000-4);
}

BDVK file_info_count;
int file_count_copy;

void DirFileCount(dword way)
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
				file_count_copy++;
				DirFileCount(cur_file);
			}
			else
			{
				file_count_copy++;
			}
		}
		free(cur_file);
	}
}

void PasteThread()
{
	char copy_rezult;
	int j;
	int cnt = 0;
	dword buf;
	file_count_copy = 0;
	copy_bar.value = 0; 
	
	buf = clipboard.GetSlotData(clipboard.GetSlotCount()-1);
	if (DSDWORD[buf+4] != 3) return;
	cnt = ESINT[buf+8];
	for (j = 0; j < cnt; j++) {
		strlcpy(#copy_from, j*4096+buf+10, 4096);
		GetFileInfo(#copy_from, #file_info_count);
		if ( file_info_count.isfolder ) DirFileCount(#copy_from);
		else file_count_copy++;
	}
	copy_bar.max = file_count_copy;
	DisplayCopyfForm();
	for (j = 0; j < cnt; j++) {
		strlcpy(#copy_from, j*4096+buf+10, 4096);
		if (!copy_from) CopyExit();
		strcpy(#copy_to, #path);
		strcat(#copy_to, #copy_from+strrchr(#copy_from,'/'));
		if (!strcmp(#copy_from,#copy_to))
		{
			strcpy(#copy_to, #path);
			strcat(#copy_to, "new_");
			strcat(#copy_to, #copy_from+strrchr(#copy_from,'/'));
		}
		if (strstr(#copy_to, #copy_from))
		{
			notify("Copy directory into itself is a bad idea...");
			CopyExit();
		}

		if (copy_rezult = copyf(#copy_from,#copy_to))
		{
			Write_Error(copy_rezult);
		}
		else if (cut_active)
		{
			strcpy(#file_path, #copy_from);
			Del_File(true);
			
		}
	}
	if (cut_active)
	{
		cut_active=false;
	}
	if (info_after_copy) notify(INFO_AFTER_COPY);
	CopyExit();
}

#define WIN_COPY_W 345
#define WIN_COPY_H 110
proc_info Copy_Form;

void DisplayCopyfForm()
{
	  switch(CheckEvent())
	  {
		 case evButton:
			notify(T_CANCEL_PASTE);
			CopyExit();
			break;
		 
		case evReDraw:
			DefineAndDrawWindow(Form.left+Form.width-200,Form.top+90,WIN_COPY_W,GetSkinHeight()+WIN_COPY_H,0x34,0xFFFFFF,T_PASTE_WINDOW_TITLE);
			GetProcessInfo(#Copy_Form, SelfInfo);
			WriteText(45, 11, 0x80, system.color.work_text, T_PASTE_WINDOW_TEXT);
			DrawFlatButton(Copy_Form.cwidth - 96, Copy_Form.cheight - 32, 80, 22, 10, system.color.work_button, T_PASTE_WINDOW_BUTTON);
			DrawBar(8, 10, 32, 32, 0xFFFfff);
			break;
	  }
}

void CopyExit() {
	action_buf = COPY_PASTE_END;
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}


void copyf_Draw_Progress(dword copying_filename) {
	if (Copy_Form.cwidth==0)
	{
		copy_bar.value++;
		return;
	}
	copy_bar.width = Copy_Form.cwidth-32;
	DisplayCopyfForm();
	Put_icon(copying_filename+strrchr(copying_filename,'.'), 16, 19, 0xFFFfff, 0);
	DrawBar(45, 29, Copy_Form.cwidth-45, 10, 0xFFFFFF);
	WriteText(45, 29, 0x80, 0x000000, copying_filename);
	progressbar_draw stdcall (#copy_bar);
	progressbar_progress stdcall (#copy_bar);
	//copy_bar.value++;
	//pause(10);
}