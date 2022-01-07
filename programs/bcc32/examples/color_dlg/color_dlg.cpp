#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>
#include <load_lib.h>
#include <l_proc_lib.h>

using namespace Kolibri;

const char header[] = "Color Dialog";
char library_path[2048];
long color1,color2,color3;

ColorDialog_data cold;
unsigned char procinfo[1024];

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
		cold.type = 0;
		cold.procinfo = procinfo;
		cold.com_area_name = "FFFFFFFF_color_dialog";
		cold.com_area = 0;
		cold.start_path = "/sys/colrdial";
		cold.draw_window = DrawWindow;
		cold.status = 0;
		cold.x_size = 420;
		cold.x_start = 10;
		cold.y_size = 320;
		cold.y_start = 10;
		ColorDialog_Init(&cold);
	} else return false;
	color1=color2=color3=0xffffff;
	return true;
}

void KolibriOnPaint(void)
{
	// If button have ID 1, this is close button
	DrawButton(2,0xf0f0f0, 10,10,60,20);
	DrawText(20,16,0,"Color 1");
	DrawButton(3,0xf0f0f0, 80,10,60,20);
	DrawText(90,16,0,"Color 2");
	DrawButton(4,0xf0f0f0, 150,10,60,20);
	DrawText(160,16,0,"Color 3");

	DrawRect( 10,40,61,100,color1);
	DrawRect( 80,40,61,100,color2);
	DrawRect(150,40,61,100,color3);
}

void KolibriOnButton(long id, TThreadData /*th*/)
{
	switch(id){
	case 2:
		ColorDialog_Start(&cold);
		if(cold.status==1){ color1 = cold.color; }
		break;
	case 3:
		ColorDialog_Start(&cold);
		if(cold.status==1){ color2 = cold.color; }
		break;
	case 4:
		ColorDialog_Start(&cold);
		if(cold.status==1){ color3 = cold.color; }
		//break;
	};
}
