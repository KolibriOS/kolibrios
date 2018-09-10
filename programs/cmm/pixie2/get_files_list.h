int files_mas[2000];
dword buf;


void OpenDirectory(dword folder_path)
{
	int cur;
	char temp_filename[4096];
	dword j, filesnum, end_pointer;

	list.count = 0;
	if (buf) free(buf);
	if (GetDir(#buf, #filesnum, folder_path, DIRS_ONLYREAL)==0)
	if (filesnum==0)
	{
		notify("'Error opening folder' -E");
	}
	for (j=0; j<filesnum; j++)
	{
		strcpy(#temp_filename, j*304 + buf+72);
		end_pointer = #temp_filename+strlen(#temp_filename);
		if (strcmpi(end_pointer-4,".mp3")==0) 
		|| (strcmpi(end_pointer-4,".wav")==0)
		|| (strcmpi(end_pointer-3,".xm")==0)
		{
			cur = list.count;
			files_mas[cur]=j;
			list.count++;
			if (list.count>=sizeof(files_mas)*sizeof(int)) break;			
		}
	}
	SortByName(0, list.count-1);
}

void SortByName(int a, b)
{
	int j, i = a;
	if (a >= b) return;
	for (j = a; j <= b; j++)
		if (strcmp(files_mas[j]*304 + buf+72, files_mas[b]*304 + buf+72)<=0) { files_mas[i] >< files_mas[j];   i++;}
	SortByName(a, i-2);
	SortByName(i, b);
}

void SetOpenedFileFirst(dword in_name)
{
	int i;
	dword opened_filename = in_name + strrchr(in_name, '/');
	for (i=0; i<list.count; i++)
	{
		if (strcmpi(opened_filename,files_mas[i]*304 + buf+72)==0) { files_mas[0]><files_mas[i]; return; }
	}
}

dword GetPlayingItemName() {
	return files_mas[current_playing_file_n]*304 + buf+72;
}

dword GetSelectedItemName() {
	int cury = list.cur_y;
	return files_mas[cury]*304 + buf+72;
}
