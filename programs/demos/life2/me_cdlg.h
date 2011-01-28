#ifndef __MENUET_FILE_OPEN_H_INCLUDED_
#define __MENUET_FILE_OPEN_H_INCLUDED_

#include <menuet.h>

// Menuet interface.

namespace Menuet   // All menuet functions, types and data are nested in the (Menuet) namespace.
{
	struct TOpenFileStruct;   // Data for a file open dialog.
#define MENUET_OPEN_FILE_INIT {}   // Initializer of the file open struct, cat be redefined in a realization of the library

	void OpenFileInit(TOpenFileStruct &ofs);
	void OpenFileDelete(TOpenFileStruct &ofs);
	bool OpenFileDialog(TOpenFileStruct &ofs);
	int OpenFileGetState(const TOpenFileStruct &ofs);
	bool OpenFileSetState(TOpenFileStruct &ofs, int state);
	char *OpenFileGetName(const TOpenFileStruct &ofs);
	bool OpenFileSetName(TOpenFileStruct &ofs, char *name);
}

#ifdef __MENUET__

namespace Menuet
{
// Structures.

	struct TOpenFileStruct
	{
		int state;
		char *name;
	};
#undef  MENUET_OPEN_FILE_INIT
#define MENUET_OPEN_FILE_INIT  {0,0}

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

#else   // else: def __MENUET__

namespace Menuet
{
	struct TOpenFileStruct
	{
		unsigned int data;

		TOpenFileStruct();
		~TOpenFileStruct();
	};
#undef  MENUET_OPEN_FILE_INIT
#define MENUET_OPEN_FILE_INIT  TOpenFileStruct()
}

#endif  // __MENUET__

#endif  // __MENUET_FILE_OPEN_H_INCLUDED_

