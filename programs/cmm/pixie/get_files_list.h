char temp_filename[4096],
     work_folder[4096],
     current_filename[256];
int files_mas[2000];
dword buf;


void OpenDirectory(dword folder_path)
{
	int cur;
	dword j, filesnum, end_pointer;

	list.count = 0;
	//free(buf);
	if (GetDir(#buf, #filesnum, folder_path, DIRS_ONLYREAL)==0)
	if (filesnum==0)
	{
		notify("'Error opening folder' -E");
	}
	
	for (j=0; j<filesnum; j++)
	{
		strcpy(#temp_filename, j*304 + buf+72);
		end_pointer = #temp_filename+strlen(#temp_filename);
		if (strcmpi(end_pointer-4,".mp3")!=0) continue;
		cur = list.count;
		files_mas[cur]=j;
		list.count++;
	}
	SortByName(0, list.count-1);
	SetOpenedFileFirst();
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

void SetOpenedFileFirst()
{
	int i;
	dword opened_filename = #param + strrchr(#param, '/');
	for (i=0; i<list.count; i++)
	{
		if (strcmpi(opened_filename,files_mas[i]*304 + buf+72)==0) { files_mas[0]><files_mas[i]; return; }
	}
}

dword GetCurrentItemName() {
	return files_mas[list.current]*304 + buf+72;
}
