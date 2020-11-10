#ifndef __KOLIBRI_FILE_OPEN_H_INCLUDED_
#define __KOLIBRI_FILE_OPEN_H_INCLUDED_

#include <kolibri.h>

// Kolibri interface.

namespace Kolibri   // All kolibri functions, types and data are nested in the (Kolibri) namespace.
{
	struct TOpenFileStruct;   // Data for a file open dialog.
#define KOLIBRI_OPEN_FILE_INIT {}   // Initializer of the file open struct, cat be redefined in a realization of the library

	void OpenFileInit(TOpenFileStruct &ofs);
	void OpenFileDelete(TOpenFileStruct &ofs);
	bool OpenFileDialog(TOpenFileStruct &ofs);
	int OpenFileGetState(const TOpenFileStruct &ofs);
	bool OpenFileSetState(TOpenFileStruct &ofs, int state);
	char *OpenFileGetName(const TOpenFileStruct &ofs);
	bool OpenFileSetName(TOpenFileStruct &ofs, char *name);
}

#ifdef __KOLIBRI__

namespace Kolibri
{
// Structures.

	struct TOpenFileStruct
	{
		int state;
		char *name;
	};
#undef  KOLIBRI_OPEN_FILE_INIT
#define KOLIBRI_OPEN_FILE_INIT  {0,0}

// Inline functions.

	inline void OpenFileInit(TOpenFileStruct &ofs)
	{
		ofs.state = 0;
		ofs.name = 0;
	}

	inline void OpenFileDelete(TOpenFileStruct &ofs)
	{
		if (ofs.name) {Free(ofs.name); ofs.name = 0;}
	}

	inline int OpenFileGetState(const TOpenFileStruct &ofs)
	{
		return ofs.state;
	}

	inline char *OpenFileGetName(const TOpenFileStruct &ofs)
	{
		return ofs.name;
	}

// Functions.

	bool OpenFileSetState(TOpenFileStruct &ofs, int state)
	{
		if (!ofs.state) return !state;
		if (ofs.state == state) return true;
		if (state < 0) return false;
		ofs.state = state;
		return true;
	}

	bool OpenFileSetName(TOpenFileStruct &ofs, char *name)
	{
		if (!ofs.name && !name) return true;
		if (ofs.name) Free(ofs.name);
		if (!name) {ofs.name = 0; return true;}
		ofs.name = (char*)Alloc(StrLen(name) + 1);
		if (!ofs.name) return false;
		StrCopy(ofs.name, name);
		return true;
	}
}

#else   // else: def __KOLIBRI__

namespace Kolibri
{
	struct TOpenFileStruct
	{
		unsigned int data;

		TOpenFileStruct();
		~TOpenFileStruct();
	};
#undef  KOLIBRI_OPEN_FILE_INIT
#define KOLIBRI_OPEN_FILE_INIT  TOpenFileStruct()
}

#endif  // __KOLIBRI__

#endif  // __KOLIBRI_FILE_OPEN_H_INCLUDED_

