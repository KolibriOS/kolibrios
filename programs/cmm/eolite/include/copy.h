
byte copy_to[4096];
byte copy_from[4096];
byte cut_active=0;

enum {NOCUT, CUT};

Clipboard clipboard;

void setElementSelectedFlag(dword n, bool state) {
	dword selected_offset = file_mas[n]*304 + buf+32 + 7;
	if (!n) if (!strncmp(selected_offset+33, "..", 2)) return; //do not selec ".." directory
	ESBYTE[selected_offset] = state;
	if (state==true) selected_count++;
	if (state==false) selected_count--;
}

void Copy(dword pcth, char cut)
{
	dword selected_offset2;
	byte copy_t[4096];
	dword buff_data;
	dword path_len = 0;
	dword size_buf = 0;
	dword copy_buf_offset = 0;

	if (files.count<=0) return; //no files
	if (selected_count==0) setElementSelectedFlag(files.cur_y, true); //no element selected by "insert", so we copy current element
	debugi(selected_count);
	size_buf = 4;
	for (i=0; i<files.count; i++) 
	{
		selected_offset2 = file_mas[i]*304 + buf+32 + 7;
		if (ESBYTE[selected_offset2]) {
			strcpy(#copy_t, #path);
			strcat(#copy_t, file_mas[i]*304+buf+72);
			path_len = strlen(#copy_t);
			size_buf += path_len + 1;
		}
	}
	size_buf += 20;
	buff_data = malloc(size_buf);
	ESDWORD[buff_data] = size_buf;
	ESDWORD[buff_data+4] = 3;
	ESINT[buff_data+8] = selected_count;
	copy_buf_offset = buff_data + 10;
	for (i=0; i<files.count; i++) 
	{
		selected_offset2 = file_mas[i]*304 + buf+32 + 7;
		if (ESBYTE[selected_offset2]) {
			strcpy(copy_buf_offset, #path);
			strcat(copy_buf_offset, file_mas[i]*304+buf+72);
			copy_buf_offset += strlen(copy_buf_offset) + 1;
		}
	}
	WriteFile(size_buf, buff_data, "/rd/1/log.log");
	if (selected_count==1) setElementSelectedFlag(files.cur_y, false);
	clipboard.SetSlotData(size_buf, buff_data);
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
	int paste_elements_count = 0;
	dword buf;
	dword path_offset;
	file_count_copy = 0;
	copy_bar.value = 0; 
	
	buf = clipboard.GetSlotData(clipboard.GetSlotCount()-1);
	if (DSDWORD[buf+4] != 3) return;
	paste_elements_count = ESINT[buf+8];
	path_offset = buf + 10;
	//calculate copy files count for progress bar
	for (j = 0; j < paste_elements_count; j++) {
		GetFileInfo(path_offset, #file_info_count);
		if ( file_info_count.isfolder ) DirFileCount(path_offset);
		else file_count_copy++;
		path_offset += strlen(path_offset) + 1;
	}
	copy_bar.max = file_count_copy;
	
	if (cut_active)  operation_flag = MOVE_FLAG;
	else  operation_flag = COPY_FLAG;
	
	path_offset = buf + 10;
	DisplayOperationForm();
	for (j = 0; j < paste_elements_count; j++) {
		strcpy(#copy_from, path_offset);
		if (!copy_from) DialogExit();
		strcpy(#copy_to, #path);
		strcat(#copy_to, #copy_from+strrchr(#copy_from,'/'));
		if (!strcmp(#copy_from,#copy_to))
		{
			strcpy(#copy_to, #path);
			strcat(#copy_to, "NEW_");
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
		path_offset += strlen(path_offset) + 1;
	}
	cut_active=false;
	if (info_after_copy) notify(INFO_AFTER_COPY);
	DialogExit();
}

