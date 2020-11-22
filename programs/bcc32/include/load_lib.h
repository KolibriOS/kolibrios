#ifndef __LOAD_LIB_H_LINCLUDED_
#define __LOAD_LIB_H_INCLUDED_

// macros '@use_library' and 'load_library' defined in file 'load_lib.mac'

asm{
	@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
}

bool LoadLibrary(const char* lib_name, char* lib_path, const char* system_path, void* myimport)
{
	asm{
	load_library [ebp+8], [ebp+12], [ebp+16], [ebp+20]
	}
	return true;
}

#endif