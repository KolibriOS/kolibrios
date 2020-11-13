#ifndef __KOLIBRI_FILE_H_INCLUDED_
#define __KOLIBRI_FILE_H_INCLUDED_

#include "kolibri.h"
#include "kos_heap.h"

// Kolibri file interface.

namespace Kolibri   // All kolibri functions, types and data are nested in the (Kolibri) namespace.
{
	struct FileDateTime{
		unsigned long int time;
		unsigned long int date;
	};
	struct FileInfoBlock
	{
		unsigned long int Function;
		unsigned long int Position;
		unsigned long int Flags;
		unsigned long int Count;
		char *Buffer;
		char *FileName1;
		char *FileName2;
	};
	struct FileInfoA
	{
		unsigned long int Attributes;
		unsigned long int Flags;
		FileDateTime DateCreate;
		FileDateTime DateAccess;
		FileDateTime DateModify;
		unsigned long int FileSizeLow;
		unsigned long int FileSizeHigh;
		char FileName[520];
	};

// Functions.

	int _FileAccess(FileInfoBlock *file_access);

	FileInfoBlock* FileOpen(const char *name)
	{
		if (!name || !name[0]){
			DebugPutString("name is 0");
			return 0;
		}
		FileInfoBlock* file = (FileInfoBlock*)Alloc(sizeof(FileInfoBlock)+sizeof(FileInfoA));
		if (!file){
			DebugPutString("mem_Alloc -> 0");
			return 0;
		}
		file->Function = 5; //SSF_GET_INFO
		file->Position = 0;
		file->Flags = 0;
		file->Count = 0;
		file->Buffer = (char*)file+sizeof(FileInfoBlock);
		file->FileName1 = (char*)name;
		file->FileName2 = (char*)name;
		file->FileName1 = (char*)((long)file->FileName1 <<  8);
		file->FileName2 = (char*)((long)file->FileName2 >> 24);

		_FileAccess(file);
		return file;
	}

	int FileClose(FileInfoBlock* file_data)
	{
		if (!file_data) return -1;
		Free(file_data);
		return 0;
	}

	unsigned long int FileRead(FileInfoBlock* file_data, void *mem, int size)
	{
		file_data->Function = 0; //SSF_READ_FILE
		file_data->Position = 0;
		file_data->Flags = 0;
		file_data->Count = size;
		file_data->Buffer = (char*)mem;

		if(!_FileAccess(file_data)) return file_data->Function;
		else return 0;
	}
	
// Inline functions.

	inline unsigned long int FileGetLength(FileInfoBlock* file_data)
	{
		if (!file_data) return -1;
		return (unsigned long int)*(long*)((char*)file_data+sizeof(FileInfoBlock)+32);
	}
}

#endif  // ndef __KOLIBRI_FILE_H_INCLUDED_

