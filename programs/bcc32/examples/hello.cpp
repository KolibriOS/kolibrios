#include "../include/kolibri.h"
#include "../include/kos_heap.h"
#include "../include/kos_file.h"

using namespace Kolibri;

const char header[] = "Hello World test";
const char string[] = "Hello, World!";

bool KolibriOnStart(TStartData &me_start, TThreadData /*th*/)
{
	me_start.Left = 10;
	me_start.Top = 40;
	me_start.Width = 150;
	me_start.Height = 80;
	me_start.WinData.Title = header;
	return true;
}

void KolibriOnPaint(void)
{
	DrawText(30,10,0,string);
}

bool KolibriOnClose(TThreadData /*th*/)
{return true;}
int KolibriOnIdle(TThreadData /*th*/)
{return -1;}
void KolibriOnSize(int /*window_rect*/[], TThreadData /*th*/)
{}
void KolibriOnKeyPress(TThreadData /*th*/)
{GetKey();}
void KolibriOnMouse(TThreadData /*th*/)
{}