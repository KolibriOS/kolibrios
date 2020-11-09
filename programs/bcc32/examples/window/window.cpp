#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>

using namespace Kolibri;

const char header[] = "Colors";

bool KolibriOnStart(TStartData &kos_start, TThreadData th)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 135;
	kos_start.Height = 80;
	kos_start.WinData.WindowColor = 0xFFFFFF;
	kos_start.WinData.WindowType = 0x34; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;
	return true;
}

void KolibriOnPaint(void)
{
	// If button have ID 1, this is close button
	DrawButton(2,0xff0000, 10,40,50,20);
	DrawButton(3,0x00ff00, 70,10,50,20);
	DrawButton(4,0x0000ff, 70,40,50,20);
	DrawButton(5,0xFFFE00, 10,10,50,20);
}

void KolibriOnButton(long id, TThreadData th)
{
	switch(id){
	case 2:
		SetWindowCaption("Red");
		break;
	case 3:
		SetWindowCaption("Green");
		break;
	case 4:
		SetWindowCaption("Blue");
		break;
	case 5:
		SetWindowCaption("Yellow");
		//break;
	};
}
/*
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
*/
