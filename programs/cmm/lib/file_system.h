struct f70{
	dword	func;
	dword	param1;
	dword	param2;
	dword	param3;
	dword	param4;
	char	rezerv;
	dword	name;
};

struct BDVK{
	dword	attr;
	byte	type_name;
	byte	rez1, rez2, rez3;
	dword	timecreate;
	dword	datecreate;
	dword	timelastaccess;
	dword	datelastaccess;
	dword	timelastedit;
	dword	datelastedit;
	dword	sizelo;
	dword	sizehi;
	char	name[518];
};


///////////////////////////
//   Запуск программы    //
///////////////////////////
f70 run_file_70;
signed int RunProgram(dword run_path, run_param)
{	
    run_file_70.func = 7;
    run_file_70.param1 = 
    run_file_70.param3 = 
    run_file_70.param4 = 
    run_file_70.rezerv = 0;
    run_file_70.param2 = run_param;
    run_file_70.name = run_path;
    $mov eax,70
    $mov ebx,#run_file_70.func
    $int 0x40
}

///////////////////////////
//    Создание папки     //
///////////////////////////
f70 create_dir_70;
:int CreateDir(dword new_folder_path)
{
	create_dir_70.func = 9;
	create_dir_70.param1 = 
	create_dir_70.param2 = 
	create_dir_70.param3 = 
	create_dir_70.param4 = 
	create_dir_70.rezerv = 0;
	create_dir_70.name = new_folder_path;
	$mov eax,70
	$mov ebx,#create_dir_70.func
	$int 0x40
}

////////////////////////////
//  Удаление файла/папки  //
////////////////////////////
f70 del_file_70;	
:int DeleteFile(dword del_file_path)
{    
	del_file_70.func = 8;
	del_file_70.param1 = 
	del_file_70.param2 = 
	del_file_70.param3 = 
	del_file_70.param4 = 
	del_file_70.rezerv = 0;
	del_file_70.name = del_file_path;
	$mov eax,70
	$mov ebx,#del_file_70.func
	$int 0x40
}

////////////////////////////
//     Прочитать файл     //
////////////////////////////
f70 read_file_70; 
:int ReadFile(dword read_pos, read_file_size, read_buffer, read_file_path)
{
	read_file_70.func = 0;
	read_file_70.param1 = read_pos;
	read_file_70.param2 = 0;
	read_file_70.param3 = read_file_size;
	read_file_70.param4 = read_buffer;
	read_file_70.rezerv = 0;
	read_file_70.name = read_file_path;
	$mov eax,70
	$mov ebx,#read_file_70.func
	$int 0x40
}

:int GetFile(dword buf, filesize, read_path)
{
	BDVK ReadFile_atr;
	dword rBuf=0;
	if (! GetFileInfo(read_path, #ReadFile_atr))
	{
		rBuf = malloc(ReadFile_atr.sizelo);	
		if (! ReadFile(0, ReadFile_atr.sizelo, rBuf, read_path))
		{
			ESDWORD[buf] = rBuf;
			ESDWORD[filesize] = ReadFile_atr.sizelo;
			return 1;
		}
	}
	free(rBuf);
	return 0;
}

////////////////////////////
//     Записать файл      //
////////////////////////////
f70 write_file_70; 
:int WriteFile(dword write_file_size, write_buffer, write_file_path)
{
	write_file_70.func = 2;
	write_file_70.param1 = 0;
	write_file_70.param2 = 0;
	write_file_70.param3 = write_file_size;
	write_file_70.param4 = write_buffer;
	write_file_70.rezerv = 0;
	write_file_70.name = write_file_path;
	$mov eax,70
	$mov ebx,#write_file_70.func
	$int 0x40
}       

///////////////////////////
//    Прочитать папку    //
///////////////////////////
f70 read_dir_70;
:int ReadDir(dword file_count, read_buffer, dir_path)
{
	read_dir_70.func = 1;
	read_dir_70.param1 = 
	read_dir_70.param2 = 
	read_dir_70.rezerv = 0;
	read_dir_70.param3 = file_count;
	read_dir_70.param4 = read_buffer;
	read_dir_70.name = dir_path;
	$mov eax,70
	$mov ebx,#read_dir_70.func
	$int 0x40
}


:int GetDir(dword dir_buf, file_count, path)
{
	dword buf, fcount, error;
	buf = malloc(32);
	error = ReadDir(0, buf, path);
	if (!error)
	{
		fcount = ESDWORD[buf+8];
		buf = realloc(buf, fcount+1*304+32);
		ReadDir(fcount, buf, path);
		if (!strcmp(".",buf+72)) {fcount--; memmov(buf,buf+304,fcount*304);}
		if (!strcmp("..",buf+72)) {fcount--; memmov(buf,buf+304,fcount*304);}
		ESDWORD[dir_buf] = buf;
		ESDWORD[file_count] = fcount;
	}
	return error;
}


///////////////////////////
//   Параметры файла    //
///////////////////////////
f70 getinfo_file_70;
:dword GetFileInfo(dword file_path, bdvk_struct)
{    
    getinfo_file_70.func = 5;
    getinfo_file_70.param1 = 
    getinfo_file_70.param2 = 
    getinfo_file_70.param3 = 0;
    getinfo_file_70.param4 = bdvk_struct;
    getinfo_file_70.rezerv = 0;
    getinfo_file_70.name = file_path;
    $mov eax,70
    $mov ebx,#getinfo_file_70.func
    $int 0x40 
}


///////////////////////////
//   Скопировать файл    //
///////////////////////////
:int CopyFile(dword copy_from, copy_in)
{
	BDVK CopyFile_atr;
	dword cBufer=0;
	char rezult = -1;
	if (! GetFileInfo(copy_from, #CopyFile_atr))
	{
		cBufer = malloc(CopyFile_atr.sizelo);	
		if (! ReadFile(0, CopyFile_atr.sizelo, cBufer, copy_from))
		{
			rezult = WriteFile(CopyFile_atr.sizelo, cBufer, copy_in);
			debugi(rezult);
		}
	}
	free(cBufer);
	debugi(rezult);
	return rezult;
}

inline fastcall void SetCurDir( ECX)
{
    $mov eax,30
    $mov ebx,1
    $int 0x40
}

inline fastcall void GetCurDir( ECX, EDX)
{
    $mov eax,30
    $mov ebx,2
    $int 0x40
}

void notify(dword notify_param)
{
	RunProgram("@notify", notify_param);
}
