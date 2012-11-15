//CODED by Veliant, Leency 2008-2012. GNU GPL licence.

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

////////////////////////////
//     Создать файл     //
////////////////////////////
f70 create_file_70;
int CreateFile(dword file_size, read_buffer, file_path)
{    
	create_file_70.func = 2;
	create_file_70.param1 = 0;
	create_file_70.param2 = 0;
	create_file_70.param3 = file_size;
	create_file_70.param4 = read_buffer;
	create_file_70.rezerv = 0;
	create_file_70.name = file_path;
	$mov eax,70
	$mov ebx,#create_file_70.func
	$int 0x40
}

////////////////////////////
//     Прочитать файл     //
////////////////////////////
f70 read_file_70;
int ReadFile(dword pos, file_size, read_buffer, file_path)
{    
	read_file_70.func = 0;
	read_file_70.param1 = pos;
	read_file_70.param2 = 0;
	read_file_70.param3 = file_size;
	read_file_70.param4 = read_buffer;
	read_file_70.rezerv = 0;
	read_file_70.name = file_path;
	$mov eax,70
	$mov ebx,#read_file_70.func
	$int 0x40
}    

///////////////////////////
//    Прочитать папку    //
///////////////////////////
f70 read_dir_70;
int ReadDir(dword file_count, read_buffer, read_dir_path)
{    
	read_dir_70.func = 1;
	read_dir_70.param1 = 0;
	read_dir_70.param2 = 0;
	read_dir_70.param3 = file_count;
	read_dir_70.param4 = read_buffer;
	read_dir_70.rezerv = 0;
	read_dir_70.name = read_dir_path;
	$mov eax,70
	$mov ebx,#read_dir_70.func
	$int 0x40
}  

///////////////////////////
//   Запуск программv    //
///////////////////////////
f70 run_file_70;
int RunProgram(dword run_path, run_param)
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
int CreateFolder(dword new_folder_path)
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
int DeleleFile(dword del_file_path)
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

///////////////////////////
//   Скопировать файл    //
///////////////////////////
f70	CopyFile_f;
inline fastcall int CopyFile(dword EBX,ECX)
{
	BDVK CopyFile_atr;
	dword s=EBX, d=ECX, cBufer=0;
	CopyFile_f.func = 5;
	CopyFile_f.param1 = 0;
	CopyFile_f.param2 = 0;
	CopyFile_f.param3 = 0;
	CopyFile_f.param4 = #CopyFile_atr;
	CopyFile_f.rezerv = 0;
	CopyFile_f.name = s;
	$mov eax, 70
	$mov ebx, #CopyFile_f
	$int 0x40
	
	if (!EAX)
	{	
		cBufer = malloc(2*CopyFile_atr.sizelo);	
		ReadFile(dword 0, CopyFile_atr.sizelo, cBufer, s);
	
		IF (!EAX)
		{
			CopyFile_f.func = 2;
			CopyFile_f.param1 = 0;
			CopyFile_f.param2 = 0;
			CopyFile_f.param3 = CopyFile_atr.sizelo;
			CopyFile_f.param4 = cBufer;
			CopyFile_f.rezerv = 0;
			CopyFile_f.name = d;
			$mov eax, 70
			$mov ebx, #CopyFile_f
			$int 0x40
		}
	}
	return EAX;

}


