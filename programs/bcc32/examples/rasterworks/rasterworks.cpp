#include <kolibri.h>
#include <kos_heap.h>
#include <load_lib.h>
#include <l_rasterworks.h>

using namespace Kolibri;

const char header[] = "Rasterworks example";
char library_path[2048];

namespace Kolibri{
	char CurrentDirectoryPath[2048];
}

void *buffi;

bool KolibriOnStart(TStartData &kos_start, TThreadData /*th*/)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 800;
	kos_start.Height = 300;
	kos_start.WinData.WindowColor = 0xd0d0d0;
	kos_start.WinData.WindowType = 0x33; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;
	if(LoadLibrary("rasterworks.obj", library_path, "/sys/lib/rasterworks.obj", &import_rasterworks))
	{
		buffi = new char[768*256*3 +8];
		*((long*)buffi) = 768;
		*((long*)buffi+1) = 256;	
	} else {
		buffi = 0;
	}
	return true;
}

void KolibriOnPaint(void)
{
	DrawText(10,5, 0x30000000, "Системный шрифт 8x16");
	if(!buffi) return;

	long ln_str = countUTF8Z("Пример работы", -1);

	memset((char*)buffi+8, (char)-1, 768*256*3);

	drawText(buffi, 0,  0, "Пример работы", ln_str, 0xFF000000, 0x30C18);
	drawText(buffi, 0, 32, "Пример работы", ln_str, 0xFF000000, 0x1030C18);
	drawText(buffi, 0, 64, "Пример работы", ln_str, 0xFF000000, 0x2030C18);
	drawText(buffi, 0, 96, "Пример работы", ln_str, 0xFF000000, 0x4030C18);
	drawText(buffi, 0,128, "Пример работы", ln_str, 0xFF000000, 0x8030C18);
	drawText(buffi, 0,160, "Пример работы", ln_str, 0xFF000000, 0x0F031428);

	PutImage((char*)buffi+8, 5, 25, 768, 256);
}

bool KolibriOnClose(TThreadData /*th*/) {
	if(buffi) {delete buffi; buffi = 0;}
	return true;
}