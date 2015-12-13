
byte copy_to[4096];
byte copy_from[4096];
byte cut_active=0;

enum {NOCUT, CUT};

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
	
	if (cut_active)  operation_flag = MOVE_FLAG;
	else  operation_flag = COPY_FLAG;
	
	DisplayOperationForm();
	for (j = 0; j < cnt; j++) {
		strlcpy(#copy_from, j*4096+buf+10, 4096);
		if (!copy_from) DialogExit();
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
			DialogExit();
		}

		if (copy_rezult = copyf(#copy_from,#copy_to))
		{
			Write_Error(copy_rezult);
		}
		else if (cut_active)
		{
			//strcpy(#file_path, #copy_from);
			Del_File2(#copy_from, 0);
			
		}
	}
	if (cut_active)
	{
		cut_active=false;
	}
	if (info_after_copy) notify(INFO_AFTER_COPY);
	DialogExit();
}