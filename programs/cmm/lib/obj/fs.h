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
	
	dword read_pointer;
	dword read(dword path);
	
	dword run_pointer;
	dword run(dword path,arg);
	
	dword move_pointer;
	byte move(dword path1,path2);
	
	dword copy_pointer;
	byte copy(dword path1,path2);
	
	dword get_size_pointer;
	qword get_size(dword path);
} fs;

:byte FILE_SYSTEM_FUNCTION::remove(dword path)
{
	dword tmp = path;
	remove_pointer stdcall(tmp);
	return EAX;
}

:dword FILE_SYSTEM_FUNCTION::read(dword path)
{
	dword tmp = path;
	read_pointer stdcall(tmp);
	return EAX;
}

:dword FILE_SYSTEM_FUNCTION::run(dword path,arg)
{
	dword tmp1 = path1;
	dword tmp2 = arg;
	run_pointer stdcall(tmp1,tmp2);
	return EAX;
}

:qword FILE_SYSTEM_FUNCTION::get_size(dword path)
{
	dword tmp = path;
	get_size_pointer stdcall(tmp);
	return EAX;
}

:byte FILE_SYSTEM_FUNCTION::move(dword path1,path2)
{
	dword tmp1 = path1;
	dword tmp2 = path2;
	move_pointer stdcall(tmp1,tmp2);
	return EAX;
}

:byte FILE_SYSTEM_FUNCTION::copy(dword path1,path2)
{
	dword tmp1 = path1;
	dword tmp2 = path2;
	copy_pointer stdcall(tmp1,tmp2);
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
	fs.copy_pointer = library.get("fs.copy");
	fs.read_pointer = library.get("fs.read");
	fs.run_pointer = library.get("fs.execute");
	__CHECK_FS__ = true;
}

#endif