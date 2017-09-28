//IO library

/*
	class IO:
		io.count(path)           - количество файлов в дирректории path
		io.size(path)            - размер (файла/папки) path
		io.run(path,param)       - запустить файл path с параметром param
		io.write(size,data,path) - записать файл с данными data в дирректорию path
		io.read(path)            - прочитать файл path и возвратить указатель на данные
		io.move(path1,path2)     - переместить (файл/папку) из path1 в path2
		io.copy(path1,path2)     - копировать (файл/папку) из path1 в path2
		io.set(path,attribute)   - установить аттрибуты дл€ (файла/папки)
		io.del(path)             - удалить (файл/папку)
		
		io.dir.load(path)        - загрузить в буффер данные папки path (следует очищать буффер сомандой free!!!)
		io.dir.position(number)  - возвратить указатель имени файла по позиции number
		io.dir.make(path)        - создать папку path
		
		io.dir.buffer            - буффер данных загруженной папки ф-цией (io.dir.load)
		io.dir.count             - количество файлов фагруженной ф-цией (io.count)
*/

#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H

#ifndef INCLUDE_DATE_H
#include "../lib/date.h"
#endif

#ifdef LANG_RUS
	#define __T__GB "Г°"
	#define __T__MB "М°"
	#define __T__KB "К°"
	#define __T___B "Б"
#else
	#define __T__GB "Gb"
	#define __T__MB "Mb"
	#define __T__KB "Kb"
	#define __T___B "B"
#endif

#define ATR_READONLY   000001b
#define ATR_HIDDEN     000100b
#define ATR_SYSTEM     010000b

#define ATR_NOREADONLY 000010b
#define ATR_NOHIDDEN   001000b
#define ATR_NOSYSTEM   100000b

:enum
{
	DIR_ALL,
	DIR_NOROOT,
	DIR_ONLYREAL
};

:struct ___f70{
	dword	func;
	dword	param1;
	dword	param2;
	dword	param3;
	dword	param4;
	char	rezerv;
	dword	name;
}__file_F70;

:int ___ReadDir(dword file_count, read_buffer, dir_path)
{
	__file_F70.func = 1;
	__file_F70.param1 = 
	__file_F70.param2 = 
	__file_F70.rezerv = 0;
	__file_F70.param3 = file_count;
	__file_F70.param4 = read_buffer;
	__file_F70.name = io.path.path(dir_path);
	$mov eax,70
	$mov ebx,#__file_F70.func
	$int 0x40
}

:dword ___GetFileInfo(dword file_path, bdvk_struct)
{    
    __file_F70.func = 5;
    __file_F70.param1 = 
    __file_F70.param2 = 
    __file_F70.param3 = 0;
    __file_F70.param4 = bdvk_struct;
    __file_F70.rezerv = 0;
    __file_F70.name = io.path.path(file_path);
    $mov eax,70
    $mov ebx,#__file_F70.func
    $int 0x40
}

:struct ____BDVK {
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

:struct __FILE
{
	dword count;
	int del(...);
	int read(...);
	int write(...);
	dword set(...);
};
:dword __FILE::set(dword file_path)
{    
    __file_F70.func = 6;
    __file_F70.param1 = 
    __file_F70.param2 = 
    __file_F70.param3 = 0;
    __file_F70.param4 = #io.BDVK;
    __file_F70.rezerv = 0;
    __file_F70.name = io.path.path(file_path);
    $mov eax,70
    $mov ebx,#__file_F70.func
    $int 0x40
}
:int __FILE::del(dword PATH)
{
	__file_F70.func = 8;
	__file_F70.param1 = 
	__file_F70.param2 = 
	__file_F70.param3 = 
	__file_F70.param4 = 
	__file_F70.rezerv = 0;
	__file_F70.name = io.path.path(PATH);
	$mov eax,70
	$mov ebx,#__file_F70.func
	$int 0x40
}
:int __FILE::read(dword read_pos, read_file_size, read_buffer, read_file_path)
{
	__file_F70.func = 0;
	__file_F70.param1 = read_pos;
	__file_F70.param2 = 0;
	__file_F70.param3 = read_file_size;
	__file_F70.param4 = read_buffer;
	__file_F70.rezerv = 0;
	__file_F70.name = io.path.path(read_file_path);
	$mov eax,70
	$mov ebx,#__file_F70.func
	$int 0x40
}
:int __FILE::write(dword write_file_size, write_buffer, write_file_path)
{
	__file_F70.func = 2;
	__file_F70.param1 = 0;
	__file_F70.param2 = 0;
	__file_F70.param3 = write_file_size;
	__file_F70.param4 = write_buffer;
	__file_F70.rezerv = 0;
	__file_F70.name = io.path.path(write_file_path);
	$mov eax,70
	$mov ebx,#__file_F70.func
	$int 0x40
}
:struct __DIR
{
	int make(dword name);
	dword load(...);
	dword position(dword i);
	dword buffer;
	signed count;
};
:dword __DIR::position(dword i)
{
	return i*304+buffer+72;
}
:int __DIR::make(dword new_folder_path)
{
	__file_F70.func = 9;
	__file_F70.param1 = 
	__file_F70.param2 = 
	__file_F70.param3 = 
	__file_F70.param4 = 
	__file_F70.rezerv = 0;
	__file_F70.name = io.path.path(new_folder_path);
	$mov eax,70
	$mov ebx,#__file_F70.func
	$int 0x40
}

:dword __DIR::load(dword PATH;byte options)
{
	io.count(PATH);
	if(count!=-1)
	{      
		buffer = malloc(count+1*304+32);
		___ReadDir(count, buffer, PATH);
		if (options == DIR_ONLYREAL)
		{
			if (!strcmp(".",buffer+72)){count--; memmov(buffer,buffer+304,count*304);}
			if (!strcmp("..",buffer+72)){count--; memmov(buffer,buffer+304,count*304);}
			return buffer;
		}
		if (options == DIR_NOROOT)
		{
			if (!strcmp(".",buffer+72)) memmov(buffer,buffer+304,count*304-304);
			return buffer;
		}
		return buffer;
	}
	return NULL;
}

:struct __PATH
{
	dword file(...);
	dword path(...);
};

:char __PATH_NEW[4096];
:dword __PATH::path(dword PATH)
{
	dword pos;
	if(!PATH) return self.dir;
	pos = PATH;
	if(DSBYTE[pos]=='/') || (!strncmp(PATH,"./",2))
	{
		return PATH;
	}
	sprintf(#__PATH_NEW,"%s/%s",self.dir,PATH);
	return #__PATH_NEW;
}

:dword __PATH::file(dword name)
{
	dword ret;
	ret = name;
	while(DSBYTE[name])
	{
		if(DSBYTE[name]=='/')ret = name+1;
		name++;
	}
	return ret;
}

:struct IO
{
	dword buffer_data;
	dword size_dir;
	dword count_dirs,count_files;
	signed FILES_SIZE;
	dword file_name;
	double size(...);
	dword get_size_dir(dword name);
	signed count(dword path);
	signed int run(dword path,param);
	byte del(...);
	dword read(...);
	int write(...);
	byte copy(...);
	byte move(...);
	dword set(...);
	dword readKPACK(dword path1);
	dword convert_size();
	__DIR dir;
	__PATH path;
	__FILE file;
	____BDVK BDVK;
}io;
:byte __ConvertSize_size_prefix[8];
:dword IO::convert_size()
{
	byte size_nm[3];
	dword bytes;
	bytes = FILES_SIZE;
	if (bytes>=1073741824) strlcpy(#size_nm, __T__GB,2);
	else if (bytes>=1048576) strlcpy(#size_nm, __T__MB,2);
	else if (bytes>=1024) strlcpy(#size_nm, __T__KB,2);
	else strlcpy(#size_nm, __T___B,1);
	while (bytes>1023) bytes/=1024;
	sprintf(#__ConvertSize_size_prefix,"%d %s",bytes,#size_nm);
	return #__ConvertSize_size_prefix;
}
:dword IO::readKPACK(dword path1)
{
	EAX = 68;
	EBX = 27;
	ECX = path1;
	$int 0x40;
	FILES_SIZE = EDX;
	buffer_data = EAX;
	return buffer_data;
}	
:int IO::write(dword PATH,data)
{
	file.write(0,strlen(data),data,PATH);
}
:char BYTE_HEAD_FILE_KPCK[4];
:dword IO::read(dword PATH)
{
	int result;
	___GetFileInfo(PATH, #BDVK);
	if(BDVK.isfolder)return false;
	file.read(0,4,#BYTE_HEAD_FILE_KPCK,PATH);
	IF(DSDWORD[#BYTE_HEAD_FILE_KPCK]=='KCPK')return readKPACK(PATH);
	FILES_SIZE = BDVK.sizelo;
	buffer_data = malloc(FILES_SIZE+1);
	result = file.read(0,FILES_SIZE,buffer_data,PATH);
	if (result!=0) buffer_data = free(buffer_data);	//file read failed
	return buffer_data;
}

:signed int IO::run(dword rpath,rparam)
{
	__file_F70.func = 7;
    __file_F70.param1 = 
    __file_F70.param3 = 
    __file_F70.param4 = 
    __file_F70.rezerv = 0;
    __file_F70.param2 = rparam;
    __file_F70.name = path.path(rpath);
    $mov eax,70
    $mov ebx,#__file_F70.func
    $int 0x40
}
:signed IO::count(dword PATH)
{
	byte buf[32];
	if(!___ReadDir(0, #buf, PATH))
	{
		dir.count = ESDWORD[#buf+8];
		return dir.count;
	}
	return -1;
}

:double IO::size(dword PATH)
{
	dword i,tmp_buf,count_dir,count_file;
	dword filename;
	double size_tmp;
	double tmp;
	if(!PATH)return 0;
	if(___GetFileInfo(PATH, #BDVK))return -1;
	if(BDVK.isfolder)
	{
		tmp_buf = dir.load(PATH,DIR_ONLYREAL);
		if(dir.count<1)return 0;
		count_dir = dir.count;
		i = 0;
		size_tmp = 0;
		count_file = malloc(4096);
		while(i<count_dir)
		{
			filename = i*304+tmp_buf+72;
			sprintf(count_file,"%s/%s",PATH,filename);
			tmp = size(count_file);
			if(tmp==-1)return -1;
			size_tmp += tmp;
			i++;
			if (TestBit(ESDWORD[filename-40], 4))count_dirs++;
			else count_files++;
		}
		
		free(tmp_buf);
		free(count_file);
		FILES_SIZE = size_tmp;
		return FILES_SIZE;
	}
	FILES_SIZE = BDVK.sizelo;
	count_files++;
	return FILES_SIZE;
}
:byte IO::del(dword PATH)
{
	dword i,tmp_buf,count_dir,count_file;
	if(!PATH)return 0;
	if(___GetFileInfo(PATH, #BDVK))return false;
	if(BDVK.isfolder)
	{
		tmp_buf = dir.load(PATH,DIR_ONLYREAL);
		count_dir = dir.count;
		i = 0;
		count_file = malloc(4096);
		while(i<count_dir)
		{
			sprintf(count_file,"%s/%s",PATH,i*304+tmp_buf+72);
			if(!del(count_file))return false;
			i++;
		}
		free(tmp_buf);
		free(count_file);
	}
	file.del(PATH);
	return true;
}
:dword IO::set(dword PATH,atr)
{
	dword i,tmp_buf,count_dir,count_file;
	byte cmd_read,cmd_hide,cmd_system;
	if(!PATH)return 0;
	if(___GetFileInfo(PATH, #BDVK))return false;
	cmd_read   = atr&11b;
	atr>>=2;
	cmd_hide   = atr&11b;
	atr>>=2;
	cmd_system = atr&11b;
	if(BDVK.isfolder)
	{
		tmp_buf = dir.load(PATH,DIR_ONLYREAL);
		count_dir = dir.count;
		i = 0;
		count_file = malloc(4096);
		while(i<count_dir)
		{
			sprintf(count_file,"%s/%s",PATH,i*304+tmp_buf+72);
			file.set(PATH,atr);
			i++;
		}
		free(tmp_buf);
		free(count_file);
		return 0;
	}
	if(cmd_read)
	{
		if(cmd_read&01b)BDVK.readonly = true;
		else BDVK.readonly = false;
	}
	if(cmd_hide)
	{
		if(cmd_hide&01b)BDVK.hidden = true;
		else BDVK.hidden = false;
	}
	if(cmd_system)
	{
		if(cmd_system&01b)BDVK.system = true;
		else BDVK.system = false;
	}
	file.set(PATH);
}
:byte IO::copy(dword PATH,PATH1)
{
	dword i,tmp_buf,count_dir,count_file;
	dword _path_;
	byte ret;
	if(!PATH)return 0;
	if(___GetFileInfo(PATH, #BDVK))return false;
	_path_ = malloc(4096);
	if(BDVK.isfolder)
	{
		sprintf(_path_,"%s/%s",PATH1,path.file(PATH));
		dir.make(_path_);
		tmp_buf = dir.load(PATH,DIR_ONLYREAL);
		count_dir = dir.count;
		i = 0;
		count_file = malloc(4096);
		while(i<count_dir)
		{
			sprintf(count_file,"%s/%s",PATH,i*304+tmp_buf+72);
			if(!copy(count_file,_path_))return false;
			i++;
		}
		free(tmp_buf);
		free(count_file);
		free(_path_);
		return true;
	}
	read(PATH);
	sprintf(_path_,"%s/%s",PATH1,path.file(PATH));
	ret = file.write(FILES_SIZE,buffer_data,_path_);
	free(_path_);
	if(!ret)return true;
	return false;
}
:byte IO::move(dword PATH,PATH1)
{
	if(copy(PATH,PATH1))if(del(PATH))return true;
	return false;
}

#endif