#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>

using namespace Kolibri;

const char header[] = "bcc32 program template";

bool KolibriOnStart(TStartData &kos_start, TThreadData th)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 500;
	kos_start.Height = 500;
	kos_start.WinData.WindowColor = 0xFFFFFF;
	kos_start.WinData.WindowType = 0x34; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;
	return true;
}

void KolibriOnPaint(void)
{
	// SOME DRAWING CODE
}

void KolibriOnButton(long id, TThreadData th)
{
	// button event (BUTTON ID)
	/*
	switch(id){
	case 2:
		SetWindowCaption("Red");
		break;
	...
	case N:
		SetWindowCaption("Yellow");
		//break;
	};
	*/
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
