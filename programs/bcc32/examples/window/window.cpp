#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>

using namespace Kolibri;

const char header[] = "Title";
const char string[] = "Exit";

bool KolibriOnStart(TStartData &kos_start, TThreadData th)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 150;
	kos_start.Height = 80;
	kos_start.WinData.WindowColor = 0xFFFFFF;
	kos_start.WinData.Title = header;
	return true;
}

void KolibriOnPaint(void)
{
	DrawButton(2,0xB0B0B0, 10,10,50,20);
	DrawText(15, 15, 0, string);
}
bool KolibriOnClose(TThreadData th)
{
	return true;
}
int KolibriOnIdle(TThreadData th)
{
	return -1;
}
void KolibriOnSize(int window_rect[], TThreadData th) {}
void KolibriOnKeyPress(TThreadData th)
{
	GetKey();
}
void KolibriOnMouse(TThreadData th) {}
