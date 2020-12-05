#include <kolibri.h>
#include <kos_heap.h>
#include <load_lib.h>
#include <l_box_lib.h>

using namespace Kolibri;

const char header[] = "Boxlib example 1";
char library_path[2048];

void* mouse_dd;
char str1[22];
char str2[22];
char str3[22] = "+380";

edit_box edit1 = { 100, 62, 10, 0xffffff, 0xa0a0a0, 0xff, 0, 0, 20, str1, &mouse_dd, ed_focus };
edit_box edit2 = { 100, 62, 30, 0xffffff, 0xa0a0a0, 0xff, 0, 0, 20, str2, &mouse_dd };
edit_box edit3 = { 100, 62, 50, 0xffffff, 0xa0a0a0, 0xff, 0, 0, 20, str3, &mouse_dd, 0, 4 };

char str4[] = "CheckBox 1";
char str5[] = "CheckBox 2";

check_box check1 = { {15, 10, 12,  80}, 8, 0xffffff, 0x80, 0, str4, ch_flag_middle+ch_flag_en };
check_box check2 = { {15, 10, 20, 100}, 8, 0xffffff, 0x80, 0, str5, ch_flag_middle };

namespace Kolibri{
	char CurrentDirectoryPath[2048];
}

bool KolibriOnStart(TStartData &kos_start, TThreadData /*th*/)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 280;
	kos_start.Height = 200;
	kos_start.WinData.WindowColor = 0xd0d0d0;
	kos_start.WinData.WindowType = 0x33; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;
	if(LoadLibrary("box_lib.obj", library_path, "/sys/lib/box_lib.obj", &import_box_lib))
	{
		check_box_init(&check1);
		check_box_init(&check2);
	}
	return true;
}

void KolibriOnPaint(void)
{
	DrawText(10,14,0,"Surname:");
	edit_box_draw(&edit1);
	DrawText(10,34,0,"Name:");
	edit_box_draw(&edit2);
	DrawText(10,54,0,"Phone:");
	edit_box_draw(&edit3);

	check_box_draw(&check1);
	check_box_draw(&check2);
}

void KolibriOnKeyPress(TThreadData th)
{
	asm{
		mcall SF_GET_KEY
	}
	edit_box_key(&edit1);
	edit_box_key(&edit2);
	edit_box_key(&edit3);
}

void KolibriOnMouse(TThreadData th)
{
	edit_box_mouse(&edit1);
	edit_box_mouse(&edit2);
	edit_box_mouse(&edit3);

	check_box_mouse(&check1);
	check_box_mouse(&check2);
}
