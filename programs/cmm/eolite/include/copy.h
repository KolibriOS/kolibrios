
byte copy_to[4096];
byte copy_from[4096];
byte cut_active=0;

enum {NOCUT, CUT};

void setElementSelectedFlag(dword n, int state) {
	dword selected_offset = file_mas[n]*304 + buf+32 + 7;
	ESBYTE[selected_offset] = state;
	if (n==0) && (strncmp(file_mas[n]*304+buf+72,"..",2)==0) {
		ESBYTE[selected_offset] = false; //do not selec ".." directory
		return;
	}
	if (state==true) selected_count++;
	if (state==false) selected_count--;
}

int getElementSelectedFlag(dword n) {
	dword selected_offset = file_mas[n]*304 + buf+32 + 7;
	return ESBYTE[selected_offset];
}

void Copy(dword pcth, char cut)
{
	byte copy_t[4096];
	dword buff_data;
	dword path_len = 0;
	dword size_buf = 0;
	dword copy_buf_offset = 0;
	dword i;

	if (files.count<=0) return; //no files

	//if no element selected by "Insert" key, then we copy current element
	if (!selected_count)
		setElementSelectedFlag(files.cur_y, true);

	if (!selected_count) return;
	
	size_buf = 4;
	for (i=0; i<files.count; i++) 
	{
		if (getElementSelectedFlag(i) == true) {
			sprintf(#copy_t,"%s/%s",#path,file_mas[i]*304+buf+72);
			path_len = strlen(#copy_t);
			size_buf += path_len + 1;
		}
	}
	size_buf += 20;
	buff_data = malloc(size_buf);
	ESDWORD[buff_data] = size_buf;
	ESDWORD[buff_data+4] = SLOT_DATA_TYPE_RAW;
	ESINT[buff_data+8] = selected_count;
	copy_buf_offset = buff_data + 10;
	for (i=0; i<files.count; i++) 
	{
		if (getElementSelectedFlag(i) == true) {
			sprintf(copy_buf_offset,"%s/%s",#path,file_mas[i]*304+buf+72);
			copy_buf_offset += strlen(copy_buf_offset) + 1;
		}
	}
	if (selected_count==1) setElementSelectedFlag(files.cur_y, false);
	Clipboard__SetSlotData(size_buf, buff_data);
	cut_active = cut;
	free(buff_data);
}

void Paste() {
	copy_stak = free(copy_stak);
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
	dword file_count_paste = 0;
	_dir_size paste_dir_size;
	BDVK file_info_count;

	copy_bar.value = 0; 
	
	buf = Clipboard__GetSlotData(Clipboard__GetSlotCount()-1);
	if (DSDWORD[buf+4] != 3) return;
	paste_elements_count = ESINT[buf+8];
	path_offset = buf + 10;
	//calculate copy files count for progress bar
	for (j = 0; j < paste_elements_count; j++) {
		GetFileInfo(path_offset, #file_info_count);
		if ( file_info_count.isfolder ) { 
			paste_dir_size.get(path_offset); 
			file_count_paste += paste_dir_size.files; 
		}
		else file_count_paste++;
		path_offset += strlen(path_offset) + 1;
	}
	copy_bar.max = file_count_paste;
	
	if (cut_active)  operation_flag = MOVE_FLAG;
	else  operation_flag = COPY_FLAG;
	
	path_offset = buf + 10;
	DisplayOperationForm();
	for (j = 0; j < paste_elements_count; j++) {
		strcpy(#copy_from, path_offset);
		if (!copy_from) DialogExit();
		sprintf(#copy_to, "%s/%s", #path, #copy_from+strrchr(#copy_from,'/'));
		if (!strcmp(#copy_from,#copy_to))
		{
			sprintf(#copy_to, "%s/NEW_%s", #path, #copy_from+strrchr(#copy_from,'/'));
		}
		if (strstr(#copy_to, #copy_from))
		{
			notify("Copy directory into itself is a bad idea...");
			DialogExit();
		}

		if (copy_rezult = copyf(#copy_from,#copy_to))
		{
			Write_Error(copy_rezult);
			if (copy_rezult==8) DialogExit(); //not enough space
		}
		else if (cut_active)
		{
			strcpy(#file_path, #copy_from);
			Del_File2(#copy_from, 0);
			
		}
		path_offset += strlen(path_offset) + 1;
	}
	cut_active=false;
	if (info_after_copy.checked) notify(INFO_AFTER_COPY);
	DialogExit();
}

