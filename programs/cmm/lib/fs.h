#ifndef INCLUDE_FILESYSTEM_H
#define INCLUDE_FILESYSTEM_H
#print "[include <fs.h>]\n"

#ifndef INCLUDE_DATE_H
#include "../lib/date.h"
#endif

//===================================================//
//                                                   //
//              Basic System Functions               //
//                                                   //
//===================================================//

:struct f70{
	dword	func;
	dword	param1;
	dword	param2;
	dword	param3;
	dword	param4;
	char	rezerv;
	dword	name;
};

:struct BDVK {
	dword	readonly:1, hidden:1, system:1, volume_label:1, isfolder:1, notarchived:1, :0;
	byte	type_name;
	byte	rez1, rez2, selected;
	dword   timecreate;
	date 	datecreate;
	dword	timelastaccess;
	date	datelastaccess;
	dword	timelastedit;
	date	datelastedit;
	dword	sizelo;
	dword	sizehi;
	char	name[518];
};



:f70 getinfo_file_70;
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

:f70 setinfo_file_70;
:dword SetFileInfo(dword file_path, bdvk_struct)
{    
    setinfo_file_70.func = 6;
    setinfo_file_70.param1 = 
    setinfo_file_70.param2 = 
    setinfo_file_70.param3 = 0;
    setinfo_file_70.param4 = bdvk_struct;
    setinfo_file_70.rezerv = 0;
    setinfo_file_70.name = file_path;
    $mov eax,70
    $mov ebx,#setinfo_file_70.func
    $int 0x40
}

:f70 run_file_70;
:signed int RunProgram(dword run_path, run_param)
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

:f70 create_dir_70;
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

:f70 del_file_70;	
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

:f70 read_file_70; 
:int ReadFile(dword offset, data_size, buffer, file_path)
{
	read_file_70.func = 0;
	read_file_70.param1 = offset;
	read_file_70.param2 = 0;
	read_file_70.param3 = data_size;
	read_file_70.param4 = buffer;
	read_file_70.rezerv = 0;
	read_file_70.name = file_path;
	$mov eax,70
	$mov ebx,#read_file_70.func
	$int 0x40
}

:f70 write_file_70; 
:int CreateFile(dword data_size, buffer, file_path)
{
	write_file_70.func = 2;
	write_file_70.param1 = 0;
	write_file_70.param2 = 0;
	write_file_70.param3 = data_size;
	write_file_70.param4 = buffer;
	write_file_70.rezerv = 0;
	write_file_70.name = file_path;
	$mov eax,70
	$mov ebx,#write_file_70.func
	$int 0x40
}

  ////////////////////////////////////////
 //     WriteInFileThatAlredyExists    //
////////////////////////////////////////
:f70 write_file_offset_70; 
:int WriteFile(dword offset, data_size, buffer, file_path)
{
	write_file_offset_70.func = 3;
	write_file_offset_70.param1 = offset;
	write_file_offset_70.param2 = 0;
	write_file_offset_70.param3 = data_size;
	write_file_offset_70.param4 = buffer;
	write_file_offset_70.rezerv = 0;
	write_file_offset_70.name = file_path;
	$mov eax,70
	$mov ebx,#write_file_offset_70.func
	$int 0x40
}

:f70 read_dir_70;
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

//ECX - buf pointer
inline fastcall void SetCurDir( ECX)
{
	EAX=30;
	EBX=1;
	$int 0x40
}

//ECX - buf pointer
//EDX - buf size
inline fastcall void GetCurDir( ECX, EDX)
{
	EAX=30;
	EBX=2;
	$int 0x40
}

//===================================================//
//                                                   //
//                        Misc                       //
//                                                   //
//===================================================//

:bool dir_exists(dword fpath)
{
	char buf[32];
	if (!ReadDir(0, #buf, fpath)) return true; 
	return false;
}

/*
// This implementation of dir_exists() is faster than
// previous but here virtual folders like
// '/' and '/tmp' are not recognised as FOLDERS
// by GetFileInfo() => BDVK.isfolder attribute :(

:bool dir_exists(dword fpath)
{
	BDVK fpath_atr;
	if (GetFileInfo(fpath, #fpath_atr) != 0) return false; 
	return fpath_atr.isfolder;
}
*/

:bool file_exists(dword fpath)
{
	BDVK ReadFile_atr;
	if (! GetFileInfo(fpath, #ReadFile_atr)) return true;
	return false;
}

enum
{
	DIRS_ALL,
	DIRS_NOROOT,
	DIRS_ONLYREAL
};
:int GetDir(dword dir_buf, file_count, path, doptions)
{
	dword buf, fcount, error;
	buf = malloc(32);
	error = ReadDir(0, buf, path);
	if (!error)
	{
		fcount = ESDWORD[buf+8];
		buf = realloc(buf, fcount+1*304+32);
		ReadDir(fcount, buf, path);
		//fcount=EBX;

		if (doptions == DIRS_ONLYREAL)
		{
			if (!strcmp(".",buf+72)) {fcount--; memmov(buf,buf+304,fcount*304);}
			if (!strcmp("..",buf+72)) {fcount--; memmov(buf,buf+304,fcount*304);}
		}
		if (doptions == DIRS_NOROOT)
		{
			if (!strcmp(".",buf+72)) {fcount--; memmov(buf,buf+304,fcount*304);}
		}

		ESDWORD[dir_buf] = buf;
		ESDWORD[file_count] = fcount;
	}
	else
	{
		ESDWORD[dir_buf] = free(buf);
		ESDWORD[file_count] = 0;
	}
	return error;
}

:dword abspath(dword relative_path) //GetAbsolutePathFromRelative()
{
	char absolute_path[4096];
	if (ESBYTE[relative_path]=='/')
	{
		strcpy(#absolute_path, relative_path);
	}
	else
	{
		strcpy(#absolute_path, I_Path);
		absolute_path[strrchr(#absolute_path, '/')] = '\0';
		strcat(#absolute_path, relative_path);
	}
	return #absolute_path;
}

:dword GetIni(dword ini_path, ini_name) //search it on /kolibrios/ then on /sys/
{
	strcpy(ini_path, "/kolibrios/settings/");
	strcat(ini_path, ini_name);
	if (!file_exists(ini_path)) {
		strcpy(ini_path, "/sys/SETTINGS/");
		strcat(ini_path, ini_name);
	}
	return ini_path;
}

:dword notify(dword notify_param)
{
	return RunProgram("/sys/@notify", notify_param);
}

:void die(dword _last_msg)
{
	notify(_last_msg);
	ExitProcess();
}

//===================================================//
//                                                   //
//                   Convert Size                    //
//                                                   //
//===================================================//

:byte ConvertSize_size_prefix[8];
:dword ConvertSize(dword bytes)
{
  byte size_nm[4];
  if (bytes>=1073741824) strlcpy(#size_nm, "Gb",2);
  else if (bytes>=1048576) strlcpy(#size_nm, "Mb",2);
  else if (bytes>=1024) strlcpy(#size_nm, "Kb",2);
  else strlcpy(#size_nm, "b ",2);
  while (bytes>1023) bytes >>= 10;
  sprintf(#ConvertSize_size_prefix,"%d %s",bytes,#size_nm);
  return #ConvertSize_size_prefix;
}

:dword ConvertSize64(dword bytes_lo, bytes_hi)
{
  if (bytes_hi > 0) {
	if (bytes_lo>=1073741824) bytes_lo >>= 30; else bytes_lo = 0;
	sprintf(#ConvertSize_size_prefix,"%d Gb",bytes_hi<<2 + bytes_lo);
	return #ConvertSize_size_prefix;
  }
  else return ConvertSize(bytes_lo);
}

:unsigned char size[25];
:dword ConvertSizeToKb(unsigned int bytes)
{
	unsigned int kb;
	dword kb_line;

	if (bytes >= 1024)
	{
		kb_line = itoa(bytes / 1024);
		strcpy(#size, kb_line);
		strcat(#size, " Kb");		
	}
	else {
		kb_line = itoa(bytes);
		strcpy(#size, kb_line);
		strcat(#size, " b");
	}

	return #size;
}

//===================================================//
//                                                   //
//                      Copy                         //
//                                                   //
//===================================================//

:int CopyFileAtOnce(dword size, copyFrom, copyTo)
dword cbuf;
int error;
{
	cbuf = malloc(size);
	if (error = ReadFile(0, size, cbuf, copyFrom))
	{
		debugln("Error: CopyFileAtOnce->ReadFile");
	}
	else
	{
		if (error = CreateFile(size, cbuf, copyTo)) debugln("Error: CopyFileAtOnce->CreateFile");
	}
	free(cbuf);
	return error;
}

:int CopyFileByBlocks(dword size, copyFrom, copyTo)
dword cbuf;
int error=-1;
dword offpos=0;
int block_size=1024*1024*4; //copy by 4 MiB
{
	if (GetFreeRAM()>1024*78) {
		//Set block size 32 MiB
		block_size <<= 3;
	}
	cbuf = malloc(block_size);
	if (error = CreateFile(0, 0, copyTo))
	{
		debugln("Error: CopyFileByBlocks->CreateFile");
		size = -1;	
	}
	while(offpos < size)
	{
		error = ReadFile(offpos, block_size, cbuf, copyFrom);
		if (error = 6) { //File ended before last byte was readed
			block_size = EBX; 
			if (block_size+offpos>=size) error=0; 
		} 
		else
			if (error!=0) {
				debugln("Error: CopyFileByBlocks->ReadFile");
				break;
			}
		if (error = WriteFile(offpos, block_size, cbuf, copyTo)) {
			debugln("Error: CopyFileByBlocks->WriteFile");
			break;
		}
		offpos += block_size;
	}
	free(cbuf);
	return error;
}

//===================================================//
//                                                   //
//                  Directory Size                   //
//                                                   //
//===================================================//

:struct _dir_size
{
	BDVK dir_info;
	dword folders;
	dword files;
	dword bytes;
	void get();	
	void calculate_loop();	
} dir_size;

:void _dir_size::get(dword way)
{
	folders = files = bytes = 0;
	if (way) calculate_loop(way);
}

:void _dir_size::calculate_loop(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	if (dir_exists(way))
	{
		cur_file = malloc(4096);
		// In the process of recursive descent, memory must be allocated dynamically, 
		// because the static memory -> was a bug !!! But unfortunately pass away to sacrifice speed.
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			sprintf(cur_file,"%s/%s",way,filename);
			
			if (TestBit(ESDWORD[filename-40], 4) )
			{
				folders++;
				calculate_loop(cur_file);
			}
			else
			{
				GetFileInfo(cur_file, #dir_info);
				bytes += dir_info.sizelo;
				files++;
			}
		}
		free(cur_file);
		free(dirbuf);
	}
}

#endif