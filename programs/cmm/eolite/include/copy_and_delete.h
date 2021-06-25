
//===================================================//
//                                                   //
//                   MASS ACTIONS                    //
//                                                   //
//===================================================//

void setElementSelectedFlag(dword n, int state) {
	dword selected_offset = items.get(n)*304 + buf+32 + 7;
	ESBYTE[selected_offset] = state;
	if (n==0) && (strncmp(items.get(n)*304+buf+72,"..",2)==0) {
		ESBYTE[selected_offset] = false; //do not selec ".." directory
		return;
	}
	if (state==true) selected_count++;
	if (state==false) selected_count--;
	if (selected_count<0) selected_count=0;
}

int getElementSelectedFlag(dword n) {
	dword selected_offset = items.get(n)*304 + buf+32 + 7;
	return ESBYTE[selected_offset];
}

dword GetFilesCount(dword _in_path)
{
	BDVK file_info_count;
	DIR_SIZE paste_dir_size;

	GetFileInfo(_in_path, #file_info_count);
	if ( file_info_count.isfolder ) {
		return paste_dir_size.get(_in_path);
	} else {
		return 1;
	}
}

//===================================================//
//                                                   //
//                  COPY AND PASTE                   //
//                                                   //
//===================================================//
byte copy_to[4096];
byte copy_from[4096];
bool cut_active = false;

enum {COPY, CUT, DELETE};

void CopyFilesListToClipboard(bool _cut_active)
{
	byte copy_t[4096];
	dword buff_data;
	dword path_len = 0;
	dword size_buf = 0;
	dword copy_buf_offset = 0;
	dword i;

	if (files.count<=0) return; //no files

	if (_cut_active!=DELETE) cut_active = _cut_active;

	//if no element selected by "Insert" key, then we copy current element
	if (!selected_count) {
		setElementSelectedFlag(files.cur_y, true);
	}

	if (!selected_count) return;
	
	size_buf = 10;
	for (i=0; i<files.count; i++) 
	{
		if (getElementSelectedFlag(i) == true) {
			sprintf(#copy_t,"%s/%s",#path,items.get(i)*304+buf+72);
			path_len = strlen(#copy_t);
			size_buf += path_len + 1;
		}
	}
	buff_data = malloc(size_buf);
	ESDWORD[buff_data] = size_buf;
	ESDWORD[buff_data+4] = SLOT_DATA_TYPE_RAW;
	ESINT[buff_data+8] = selected_count;
	copy_buf_offset = buff_data + 10;
	for (i=0; i<files.count; i++) 
	{
		if (getElementSelectedFlag(i) == true) {
			sprintf(copy_buf_offset,"%s/%s",#path,items.get(i)*304+buf+72);
			copy_buf_offset += strlen(copy_buf_offset) + 1;

			if (cut_active) {
				if (i>=files.first) && (i<files.first+files.visible)
					PutShadow(files.x+4,i-files.first*files.item_h+files.y,icons16_default.w,files.item_h,1,-3);
			}
		}
	}
	if (cut_active) {
		pause(20);
		List_ReDraw();
	}
	if (selected_count==1) setElementSelectedFlag(files.cur_y, false);
	Clipboard__SetSlotData(size_buf, buff_data);
	free(buff_data);
}


void PasteThread()
{
	char copy_rezult;
	int j, i, slash_count=0;
	int paste_elements_count = 0;
	dword buf;
	dword path_offset;
	
	buf = Clipboard__GetSlotData(Clipboard__GetSlotCount()-1);
	if (DSDWORD[buf+4] != 3) return;
	paste_elements_count = ESINT[buf+8];
	path_offset = buf + 10;

	if (cut_active) {
		DisplayOperationForm(MOVE_FLAG);
	} else {
		DisplayOperationForm(COPY_FLAG);	
	} 

	if (cut_active) {
		for (j = 0; j < paste_elements_count; j++) {
			sprintf(#copy_to, "%s/%s", #path, path_offset+strrchr(path_offset,'/'));
			slash_count = 0;
			for (i=0; i<=10; i++) {
				if (copy_to[i]=='/') slash_count++;
				if (slash_count==3) break;
			}
			if (strncmp(#copy_to, path_offset, i)!=0) goto _DIFFERENT_DRIVES;
			RenameMove(#copy_to+i, path_offset);
			if (EAX!=0) goto _DIFFERENT_DRIVES;
			path_offset += strlen(path_offset) + 1;
		}
		DialogExit();
	}

_DIFFERENT_DRIVES:
	path_offset = buf + 10;
	for (j = 0; j < paste_elements_count; j++) {
		copy_bar.max += GetFilesCount(path_offset);
		path_offset += strlen(path_offset) + 1;
	}
	
	path_offset = buf + 10;
	for (j = 0; j < paste_elements_count; j++) {
		strcpy(#copy_from, path_offset);
		if (!copy_from) DialogExit();
		sprintf(#copy_to, "%s/%s", #path, #copy_from+strrchr(#copy_from,'/'));
		if (streq(#copy_from,#copy_to))
		{
			if (cut_active) continue;
			sprintf(#copy_to, "%s/NEW_%s", #path, #copy_from+strrchr(#copy_from,'/'));
		}
		if (strstr(#copy_to, #copy_from))
		{
			notify("'Not possible to copy directory into itself.\nProcess terminated.' -E");
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
			RecursiveDelete(#copy_from, false);
			
		}
		path_offset += strlen(path_offset) + 1;
	}
	DialogExit();
}


//===================================================//
//                                                   //
//                     DELETE                        //
//                                                   //
//===================================================//

int del_error;
int RecursiveDelete(dword way, bool show_progress)
{    
	dword dirbuf, fcount, i, filename;
	int error;
	char del_from[4096];
	if (dir_exists(way))
	{
		if (error = GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL)) del_error = error;
		for (i=0; i<fcount; i++)
		{
			//if (CheckEvent()==evReDraw) draw_window();
			filename = i*304+dirbuf+72;
			sprintf(#del_from,"%s/%s",way,filename);
			if ( TestBit(ESDWORD[filename-40], 4) )
			{
				RecursiveDelete(#del_from, true);
			}
			else
			{
				if (show_progress) Operation_Draw_Progress(filename);
				if (error = DeleteFile(#del_from)) del_error = error;
			}
		}
	}
	if (error = DeleteFile(way)) del_error = error;
}

void DeleteThread()
{
	int j;
	int elements_count = 0;
	dword buf;
	dword path_offset;

	DisplayOperationForm(DELETE_FLAG);
	
	buf = Clipboard__GetSlotData(Clipboard__GetSlotCount()-1);
	Clipboard__DeleteLastSlot();
	if (DSDWORD[buf+4] != 3) return;
	elements_count = ESINT[buf+8];

	path_offset = buf + 10;
	for (j = 0; j < elements_count; j++) {
		copy_bar.max += GetFilesCount(path_offset);
		path_offset += strlen(path_offset) + 1;
	}
	
	path_offset = buf + 10;
	for (j = 0; j < elements_count; j++) {
		RecursiveDelete(path_offset, true);
		path_offset += strlen(path_offset) + 1;
	}
	if (del_error) Write_Error(del_error);
	DialogExit();
}