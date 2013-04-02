// универсальность добавления /

:void copyf(dword from1, in1)
{
	dword error;
	BDVK CopyFile_atr1;
	if (!from1) || (!in1) { notify("Error: too less copyf params!"); notify(from1); notify(in1); return; }
	error = GetFileInfo(from1, #CopyFile_atr1);
	if (error) 
		debug("Error: copyf->GetFileInfo");
	else
		if (isdir(from1)) CopyFolder(from1, in1); else CopyFile(from1, in1);
}

:int CopyFile(dword copy_from3, copy_in3)
{
	BDVK CopyFile_atr;
	dword error, cbuf;
	debug(copy_from3);
	error = GetFileInfo(copy_from3, #CopyFile_atr);
	if (error)
		{debug("Error: CopyFile->GetFileInfo"); debug(copy_from3);}
	else
	{
		cbuf = malloc(CopyFile_atr.sizelo);	
		error = ReadFile(0, CopyFile_atr.sizelo, cbuf, copy_from3);
		if (error)
			debug("Error: CopyFile->ReadFile");
		else
		{
			error = WriteFile(CopyFile_atr.sizelo, cbuf, copy_in3);
			if (error) debug("Error: CopyFile->WriteFile");
		}
	}
	free(cbuf);
	if (error) debug(copy_from3);
	return error;
}

:void CopyFolder(dword from2, in2)
{
	dword dirbuf, fcount, filename;
	int i, error, isdir;
	char copy_from2[4096], copy_in2[4096];

	error = GetDir(#dirbuf, #fcount, from2);
	if (error)
	{
		debug("Error: CopyFolder->GetDir");
		debug_error(from2, error);
		debug_error(in2, error);
		free(dirbuf);
		return;
	}
	
	if ((strcmp(in2, "/sys")!=0) && (strcmp(in2, "/tmp9/1")!=0))
	{
		error = CreateDir(in2);
		if (error) debug_error(in2, error);
	}
	chrcat(in2, '/');
	chrcat(from2, '/');

	for (i=0; i<fcount; i++)
	{
		filename = i*304+dirbuf+72;
		isdir = TestBit(ESDWORD[filename-40], 4);
		if (isdir)
		{
			if ( (!strcmp(filename, ".")) || (!strcmp(filename, "..")) ) continue;
			strcpy(#copy_from2, from2);
			strcpy(#copy_in2, in2);
			strcat(#copy_from2, filename);
			strcat(#copy_in2, filename);

			CopyFolder(#copy_from2, #copy_in2);
		}
		else
		{
			strcpy(#copy_from2, from2);
			strcat(#copy_from2, filename);
			strcpy(#copy_in2, in2);
			strcat(#copy_in2, filename);

			copyf_Action(filename);

			if (CopyFile(#copy_from2, #copy_in2)!=0) CopyFile(#copy_from2, #copy_in2); // #2 :)
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

:dword get_error(int N)
{
	char error[256];  
	N = fabs(N);
	if (N<=33)
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

:void debug_error(dword path, error_number)
{
	if (path) debug(path);
	debug(get_error(error_number));
}