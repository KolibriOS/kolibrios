// универсальность добавления /
// относительный путь относительно программы

void copyf(dword params)
{
	//copyf:  /sys/lib|/sys/lib2
	char from[4096], to[4096];
	BDVK from_atr;
	int border;

	if (!params) { notify("Error: no copyf params!"); return; }
	//ищем разделитель
	border = strchr(params, '|');
	if (!border) border = strchr(params, ' ');

	if (ESBYTE[params]<>'/') //абсолютный путь?
	{
		strcpy(#from, #program_path);
		strcat(#from, params);
		from[border+strlen(#program_path)-1]=NULL;
	}
	else
	{
		strcat(#from, params);
		from[border-1]=NULL;		
	}
	strcpy(#to, params+border);

	GetFileInfo(#from, #from_atr);
	if (TestBit(from_atr.attr, 4)==1)
		CopyFolder(#from, #to);
	else
		CopyFile(#from, #to);
}


void CopyFolder(dword from, to)
{
	dword dirbuf, fcount, filename;
	int i, error, isdir;
	char copy_from[4096], copy_in[4096];
	char from2[4096], to2[4096];

	error = GetDir(#dirbuf, #fcount, from);
	if (error) { debug_error(from, error); return; }
	
	if ((strcmp(to, "/sys")!=0) && (strcmp(to, "/tmp9/1")!=0))
	{
		error = CreateDir(to);
		if (error) debug_error(to, error);
	}
	chrcat(to, '/');
	chrcat(from, '/');

	for (i=0; i<fcount; i++)
	{
		filename = i*304+dirbuf+72;

		isdir = TestBit(ESDWORD[filename-40], 4);
		if (isdir)
		{
			if ( (!strcmp(filename, ".")) || (!strcmp(filename, "..")) ) continue;
			strcpy(#from2, from);
			strcpy(#to2, to);

			strcat(#from2, filename);
			strcat(#to2, filename);

			CopyFolder(#from2, #to2);
		}
		else
		{
			strcpy(#copy_from, from);
			strcat(#copy_from, filename);
			strcpy(#copy_in, to);
			strcat(#copy_in, filename);

			copyf_Action(filename);

			error = CopyFile(#copy_from, #copy_in);
			if (error) error = CopyFile(#copy_from, #copy_in); // #2 :)
			if (error) debug_error(#copy_in, error);
		}
	}
	free(dirbuf);
}

unsigned char *ERROR_TEXT[]={
"Code #0 - No error",
"Error #1 - Base or partition of a hard disk is not defined",
"Error #2 - Function isn't supported for this file system",
"Error #3 - Unknown file system",
"Error #4 - Reserved, is never returned",
"Error #5 - File or folder not found",
"Error #6 - End of file, EOF",
"Error #7 - Pointer lies outside of application memory",
"Error #8 - Too less disk space",
"Error #9 - FAT table is destroyed",
"Error #10 - Access denied",
"Error #11 - Device error",
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 
"Error #30 - Not enough memory",
"Error #31 - File is not executable",
"Error #32 - Too many processes",
0}; 

dword get_error(int N)
{
	char error[256];
	if (N<0) N*=-1;   
	if (N<33)
	{
		strcpy(#error, ERROR_TEXT[N]);
	}
	else
	{
		strcpy(#error, itoa(N));
		strcat(#error, " - Unknown error number O_o");
	}
	return #error;
}

void debug_error(dword path, error_number)
{
	if (path) debug(path);
	debug(get_error(error_number));
}