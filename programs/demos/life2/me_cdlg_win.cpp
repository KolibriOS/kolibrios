#include <windows.h>
#include <commdlg.h>

#include <menuet.h>
#include <me_heap.h>
#include "me_cdlg.h"

using namespace Menuet;

extern HINSTANCE hInstance;

struct TThreadDataStruct
{
	void *user;
	void *stack_begin;
	TWindowData *win_data;
	HWND hwnd;
	int flag;
	unsigned int win_time, me_time;
	void *picture;
	unsigned int picture_width, picture_height;
	void *keys;
	unsigned int bmp_data_length;
	unsigned int *bmp_data;
	unsigned int mouse_state;
};

struct TOpenFileData
{
	int state;
	char name[1];
};

namespace Menuet
{
	TOpenFileStruct::TOpenFileStruct() : data(0) {}

	TOpenFileStruct::~TOpenFileStruct()
	{
		if (data) {delete[] (char*)data; data = 0;}
	}

	void OpenFileInit(TOpenFileStruct &ofs) {ofs.data = 0;}

	void OpenFileDelete(TOpenFileStruct &ofs)
	{
		if (ofs.data) {delete[] (char*)ofs.data; ofs.data = 0;}
	}

	bool OpenFileDialog(TOpenFileStruct &ofs)
	{
		char CustomFilter[300], *name;
		int size;
		CustomFilter[0] = 0; CustomFilter[1] = 0;
		if (!OpenFileSetState(ofs, 0)) return false;
		OPENFILENAME ofn = {sizeof(OPENFILENAME), ((TThreadDataStruct*)GetThreadData())->hwnd,
			hInstance, "All files (*.*)\0*.*\0",
			CustomFilter, sizeof(CustomFilter)-1, 1, NULL, 0, NULL, 0, NULL, NULL,
			OFN_HIDEREADONLY | OFN_EXPLORER, 0, 0, "", 0, NULL, 0};
		size = 0;
		if (ofs.data) size = strlen(((TOpenFileData*)ofs.data)->name) + 1;
		if (size < 10000) size = 10000;
		name = new char[size + 1];
		if (!name) return false;
		if (ofs.data) strcpy(name, ((TOpenFileData*)ofs.data)->name);
		else name[0] = 0;
		ofn.lpstrFile = &name[0]; ofn.nMaxFile = size;
		size = GetOpenFileName(&ofn) == TRUE;
		if (OpenFileSetName(ofs, name))
		{
			((TOpenFileData*)ofs.data)->state = (size ? 2 : 1);
		}
		else size = 0;
		delete[] name;
		return (bool)size;
	}

	int OpenFileGetState(const TOpenFileStruct &ofs)
	{
		return ofs.data ? ((TOpenFileData*)ofs.data)->state : 0;
	}

	bool OpenFileSetState(TOpenFileStruct &ofs, int state)
	{
		if (!ofs.data || !((TOpenFileData*)ofs.data)->state) return !state;
		if (((TOpenFileData*)ofs.data)->state == state) return true;
		if (state < 0) return false;
		((TOpenFileData*)ofs.data)->state = state;
		return true;
	}

	char *OpenFileGetName(const TOpenFileStruct &ofs)
	{
		if (!ofs.data) return 0;
		else return ((TOpenFileData*)ofs.data)->name;
	}

	bool OpenFileSetName(TOpenFileStruct &ofs, char *name)
	{
		if (!ofs.data && !name) return true;
		int size = (unsigned int)(((TOpenFileData*)0)->name) + 1;
		int state = 0;
		if (name) size += strlen(name);
		if (ofs.data)
		{
			state = ((TOpenFileData*)ofs.data)->state;
			delete[] (char*)ofs.data;
		}
		ofs.data = (unsigned int)(new char[size]);
		if (!ofs.data) return false;
		((TOpenFileData*)ofs.data)->state = state;
		if (name) strcpy(((TOpenFileData*)ofs.data)->name, name);
		else ((TOpenFileData*)ofs.data)->name[0] = 0;
		return true;
	}
}