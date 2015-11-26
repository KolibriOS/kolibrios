#ifndef INCLUDE_LIBFS_H
#define INCLUDE_LIBFS_H
#print "[include <obj/fs.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

:struct FILE_SYSTEM_FUNCTION
{
	dword remove_pointer;
	byte remove(dword path);
	
	dword open_pointer;
	dword open(dword path);
	
	dword read_pointer;
	dword read(dword path);
	
	dword run_pointer;
	dword run(dword path,arg);
	
	dword move_pointer;
	byte move(dword path1,path2);
	
	dword copy_pointer;
	byte copy(dword path1,path2);
	
	dword write_pointer;
	byte write(dword path1,path2,path3);
	
	dword get_size_pointer;
	qword get_size(dword path);
	
	dword callback_copy_pointer;
	byte callback_copy(dword path1,path2,ptr);
} fs;

:byte FILE_SYSTEM_FUNCTION::remove(dword path)
{
	dword tmp = path;
	lib_init_fs();
	remove_pointer stdcall(tmp);
	return EAX;
}

:dword FILE_SYSTEM_FUNCTION::read(dword path)
{
	dword tmp = path;
	lib_init_fs();
	read_pointer stdcall(tmp);
	return EAX;
}

:byte FILE_SYSTEM_FUNCTION::write(dword path1,path2,path3)
{
	dword tmp1 = path1;
	dword tmp2 = path2;
	dword tmp3 = path3;
	lib_init_fs();
	write_pointer stdcall(tmp1,tmp2,tmp3);
	return EAX;
}

:dword FILE_SYSTEM_FUNCTION::run(dword path,arg)
{
	dword tmp1 = path1;
	dword tmp2 = arg;
	lib_init_fs();
	run_pointer stdcall(tmp1,tmp2);
	return EAX;
}

:qword FILE_SYSTEM_FUNCTION::get_size(dword path)
{
	dword tmp = path;
	lib_init_fs();
	//get_size_pointer stdcall(tmp);
	$push tmp
	$call get_size_pointer
	$add esi,4
}

:dword FILE_SYSTEM_FUNCTION::open(dword path)
{
	dword tmp = path;
	lib_init_fs();
	open_pointer stdcall(tmp);
	return EAX;
}

:byte FILE_SYSTEM_FUNCTION::move(dword path1,path2)
{
	dword tmp1 = path1;
	dword tmp2 = path2;
	lib_init_fs();
	move_pointer stdcall(tmp1,tmp2);
	return EAX;
}

:byte FILE_SYSTEM_FUNCTION::copy(dword path1,path2)
{
	dword tmp1 = path1;
	dword tmp2 = path2;
	lib_init_fs();
	copy_pointer stdcall(tmp1,tmp2);
	return EAX;
}

:byte FILE_SYSTEM_FUNCTION::callback_copy(dword path1,path2,ptr)
{
	dword tmp1 = path1;
	dword tmp2 = path2;
	lib_init_fs();
	callback_copy_pointer stdcall(tmp1,tmp2,ptr);
	return EAX;
}


:byte __CHECK_FS__ = 0;
:void lib_init_fs()
{
	IF(__CHECK_FS__)return;
	library.load("/sys/LIB/FS.OBJ");
	fs.remove_pointer = library.get("fs.remove");
	fs.get_size_pointer = library.get("fs.get_size");
	fs.move_pointer = library.get("fs.move");
	fs.open_pointer = library.get("fs.open");
	fs.copy_pointer = library.get("fs.copy");
	fs.read_pointer = library.get("fs.read");
	fs.run_pointer = library.get("fs.execute");
	fs.write_pointer = library.get("fs.write");
	fs.callback_copy_pointer = library.get("fs.callback_copy");
	__CHECK_FS__ = true;
}

#endif