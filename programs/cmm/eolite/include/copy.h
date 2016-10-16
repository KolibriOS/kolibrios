
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
    dword path_len = 0;
    dword buffer_len = 0;
    dword size_buf = 0;
    int ind = 0; 
    if (selected_count)
	{
        for (i=0; i<files.count; i++) 
        {
            selected_offset2 = file_mas[i]*304 + buf+32 + 7;
            if (ESBYTE[selected_offset2]) {
                strcpy(#copy_t, #path);
                strcat(#copy_t, file_mas[i]*304+buf+72);
                path_len = strlen(#copy_t);
                size_buf = size_buf + path_len + 4;
            }
        }
        
        buff_data = malloc(size_buf);
        ESDWORD[buff_data] = size_buf;
        ESDWORD[buff_data+4] = 3;
        ESINT[buff_data+8] = selected_count;    
        
        for (i=0; i<files.count; i++) 
        {
            selected_offset2 = file_mas[i]*304 + buf+32 + 7;
            if (ESBYTE[selected_offset2]) {
                strcpy(#copy_t, #path);
                strcat(#copy_t, file_mas[i]*304+buf+72);
                path_len = strlen(#copy_t);
                ESDWORD[buff_data+10+buffer_len] = path_len;
                strlcpy(buff_data+10+4+buffer_len, #copy_t, path_len);
                buffer_len = buffer_len + 4 + path_len;
                ind++;
            }
        }
		clipboard.SetSlotData(size_buf+10, buff_data);
	}
	else
	{
		path_len = strlen(#file_path)+4;
		buff_data = malloc(path_len);
        ESDWORD[buff_data] = path_len;
        ESDWORD[buff_data+4] = 3;
        ESINT[buff_data+8] = 1;
        ESDWORD[buff_data+10] = path_len;
        strlcpy(buff_data+14, #file_path, path_len);;
		clipboard.SetSlotData(path_len+10, buff_data);
	}
	cut_active = cut;
	free(buff_data);
}

void Paste() {
	copy_stak = malloc(64000);
	CreateThread(#PasteThread,copy_stak+64000-4);
}

void PasteThread()
{
	char copy_rezult;
	int j;
	int cnt = 0;
	dword buf;
	dword path_len = 0;
    dword buffer_len = 0;
    file_count_copy = 0;
	copy_bar.value = 0; 
    
    buf = clipboard.GetSlotData(clipboard.GetSlotCount()-1);
	if (DSDWORD[buf+4] != 3) return;
	cnt = ESINT[buf+8];
	for (j = 0; j < cnt; j++) {
		path_len = ESDWORD[buf+10+buffer_len];
		strlcpy(#copy_from, buf+10+buffer_len+4, path_len);
		buffer_len = buffer_len + 4 + path_len;
		GetFileInfo(#copy_from, #file_info_count);
		if ( file_info_count.isfolder ) DirFileCount(#copy_from);
		else file_count_copy++;
	}
	copy_bar.max = file_count_copy;
	
	if (cut_active)  operation_flag = MOVE_FLAG;
	else  operation_flag = COPY_FLAG;
	
	path_len = 0;
    buffer_len = 0;
    DisplayOperationForm();
	for (j = 0; j < cnt; j++) {
		path_len = ESDWORD[buf+10+buffer_len];
		strlcpy(#copy_from, buf+10+buffer_len+4, path_len);
		buffer_len = buffer_len + 4 + path_len;
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
			strcpy(#file_path, #copy_from);
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

//Another version of the function copy/paste

/*
void Copy(dword pcth, char cut)
{
    dword selected_offset2;
    byte copy_t[4096];
    dword buff_data;
    dword path_len = 0;
    dword buffer_len = 0;
    dword size_buf = 0;
    int ind = 0; 
    if (selected_count)
	{
        for (i=0; i<files.count; i++) 
        {
            selected_offset2 = file_mas[i]*304 + buf+32 + 7;
            if (ESBYTE[selected_offset2]) {
                strcpy(#copy_t, #path);
                strcat(#copy_t, file_mas[i]*304+buf+72);
                path_len = strlen(#copy_t);
                size_buf = size_buf + path_len;
            }
        }
        
        buff_data = malloc(size_buf);
        ESDWORD[buff_data] = size_buf;
        ESDWORD[buff_data+4] = 3;
        ESINT[buff_data+8] = selected_count;    
        
        for (i=0; i<files.count; i++) 
        {
            selected_offset2 = file_mas[i]*304 + buf+32 + 7;
            if (ESBYTE[selected_offset2]) {
                strcpy(#copy_t, #path);
                strcat(#copy_t, file_mas[i]*304+buf+72);
                path_len = strlen(#copy_t);
                strlcpy(buff_data+10+buffer_len+1, #copy_t, path_len);
                buffer_len = buffer_len + path_len;
                ind++;
            }
        }
		clipboard.SetSlotData(size_buf+10, buff_data);
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
	copy_stak = malloc(64000);
	CreateThread(#PasteThread,copy_stak+64000-4);
}

void PasteThread()
{
	char copy_rezult;
	int j;
	int cnt = 0;
	dword buf;
	dword path_len = 0;
    dword buffer_len = 0;
    file_count_copy = 0;
	copy_bar.value = 0; 
    debugln("Step 1");
    buf = clipboard.GetSlotData(clipboard.GetSlotCount()-1);
	if (DSDWORD[buf+4] != 3) return;
	cnt = ESINT[buf+8];
	for (j = 0; j < cnt; j++) {
		if (j==0) strlcpy(#copy_from, buf+10+buffer_len);
		else strlcpy(#copy_from, buf+10+buffer_len+1);
		buffer_len = buffer_len + path_len;
		GetFileInfo(#copy_from, #file_info_count);
		if ( file_info_count.isfolder ) DirFileCount(#copy_from);
		else file_count_copy++;
	}
	copy_bar.max = file_count_copy;
	
	if (cut_active)  operation_flag = MOVE_FLAG;
	else  operation_flag = COPY_FLAG;
	
	debugln("Step 2");
    path_len = 0;
    buffer_len = 0;
    DisplayOperationForm();
	for (j = 0; j < cnt; j++) {
		if (j==0) strlcpy(#copy_from, buf+10+buffer_len);
		else strlcpy(#copy_from, buf+10+buffer_len+1);
		debugln(#copy_from);
		path_len = strlen(#copy_from);
		debugi(path_len);
		debug("Step ");
		debugi(j);
		debugln(" ");
		buffer_len = buffer_len + path_len;
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
			strcpy(#file_path, #copy_from);
			Del_File2(#copy_from, 0);
			
		}
	}
	debugln("Step 3000");
	if (cut_active)
	{
		cut_active=false;
	}
	if (info_after_copy) notify(INFO_AFTER_COPY);
	DialogExit();
}*/


