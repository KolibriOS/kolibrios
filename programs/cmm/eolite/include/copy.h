//Leency 2008-2014

#ifdef LANG_RUS
	?define INFO_AFTER_COPY "Копирование завершено"
#elif LANG_EST
	?define INFO_AFTER_COPY "Copy finished"
#else
	?define INFO_AFTER_COPY "Copy finished"
#endif

byte copy_to[4096];
byte cut_active=0;

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

void copyf_Draw_Progress(dword filename) {
	DrawRectangle(0,0,WIN_COPY_W-5, 15,sc.work);
	WriteText(5,8, 0x80, sc.work_text, T_PASTE_WINDOW_TEXT);
	DrawBar(5, 26, WIN_COPY_W-10, 10, sc.work);
	WriteText(5,26, 0x80, sc.work_text, filename);
}

void Paste()
{
	char copy_rezult;
	byte copy_from[4096];
	int j;
    int cnt = 0;
	dword buf;
	
	buf = clipboard.GetSlotData(clipboard.GetSlotCount()-1);
    if (DSDWORD[buf+4] != 3) return;
	cnt = ESINT[buf+8];
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

void CopyExit()
{
	action_buf = COPY_PASTE_END;
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}