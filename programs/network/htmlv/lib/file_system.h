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
:int RunProgram(dword run_path, run_param)
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
:int CreateFolder(dword new_folder_path)
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


///////////////////////////
//   Параметры файла    //
///////////////////////////
f70 getinfo_file_70;
BDVK getinfo_file_info;
:dword GetFileInfo(dword file_path)
{    
    getinfo_file_70.func = 5;
    getinfo_file_70.param1 = 
    getinfo_file_70.param2 = 
    getinfo_file_70.param3 = 0;
    getinfo_file_70.param4 = #getinfo_file_info;
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
	
	if (! GetFileInfo(copy_from))
	{
		mem_Init();
		cBufer = mem_Alloc(CopyFile_atr.sizelo);	
		if (! ReadFile(dword 0, CopyFile_atr.sizelo, cBufer, copy_from))
			if (! WriteFile(CopyFile_atr.sizelo, cBufer, copy_in)) return 1;
	}
	
	return 0;
}


//Asper
void ReadAttributes(dword read_buffer, file_path)
{
	read_file_70.func = 5;
	read_file_70.param1 = 0;
	read_file_70.param2 = 0;
	read_file_70.param3 = 0;
	read_file_70.param4 = read_buffer;
	read_file_70.rezerv = 0;
	read_file_70.name = file_path;
	$mov eax,70
	$mov ebx,#read_file_70.func
	$int 0x40
}