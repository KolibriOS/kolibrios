#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>
#include <load_lib.h>
#include <l_proc_lib.h>

using namespace Kolibri;

const char header[] = "Open Dialog";
char library_path[2048];

OpenDialog_data ofd;
unsigned char procinfo[1024];
char plugin_path[4096], openfile_path[4096], filename_area[256];
od_filter filter1 = { 8, "TXT\0\0" };

namespace Kolibri{
	char CurrentDirectoryPath[2048];
}

void KolibriOnPaint(void);

void __stdcall DrawWindow()
{
	asm{
		push ebx
		mcall SF_REDRAW,SSF_BEGIN_DRAW
	}
	KolibriOnPaint();
	asm{
		mcall SF_REDRAW,SSF_END_DRAW
		pop ebx
	}
}

bool KolibriOnStart(TStartData &kos_start, TThreadData /*th*/)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 420;
	kos_start.Height = 320;
	kos_start.WinData.WindowColor = 0xFFFFFF;
	kos_start.WinData.WindowType = 0x33; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;
	if(LoadLibrary("proc_lib.obj", library_path, "/sys/lib/proc_lib.obj", &import_proc_lib))
	{
		ofd.procinfo = procinfo;
		ofd.com_area_name = "FFFFFFFF_open_dialog";
		ofd.com_area = 0;
		ofd.opendir_path = plugin_path;
		ofd.dir_default_path = "/sys";
		ofd.start_path = "/sys/File managers/opendial";
		ofd.draw_window = DrawWindow;
		ofd.status = 0;
		ofd.openfile_path = openfile_path;
		ofd.filename_area = filename_area;
		ofd.filter_area = &filter1;
		ofd.x_size = 420;
		ofd.x_start = 10;
		ofd.y_size = 320;
		ofd.y_start = 10;
		OpenDialog_Init(&ofd);
	} else return false;
	return true;
}

void KolibriOnPaint(void)
{
	// If button have ID 1, this is close button
	DrawButton(2,0xf0f0f0, 10,10,50,20);
	DrawText(20,16,0,"Open");
	DrawButton(3,0xf0f0f0, 70,10,50,20);
	DrawText(80,16,0,"Save");
	DrawButton(4,0xf0f0f0, 130,10,95,20);
	DrawText(140,16,0,"Select folder");

	if(ofd.openfile_path[0]) DrawText(10,40,0,ofd.openfile_path);
	if(ofd.opendir_path[0])  DrawText(10,55,0,ofd.opendir_path);
	if(ofd.filename_area[0]) DrawText(10,70,0,ofd.filename_area);
}

void KolibriOnButton(long id, TThreadData /*th*/)
{
	FileInfoBlock* file;
	long int k;

	switch(id){
	case 2:
		ofd.type = 0; // 0 - open
		OpenDialog_Start(&ofd);
		if(ofd.status==1){
			//... open ...
		}
		break;
	case 3:
		ofd.type = 1; // 1 - save
		OpenDialog_Start(&ofd);
		if(ofd.status==1){
			//... save ...
		}
		break;
	case 4:
		ofd.type = 2; // 2 - select folder
		OpenDialog_Start(&ofd);
		if(ofd.status==1){
			//...
		}
		//break;
	};
}
