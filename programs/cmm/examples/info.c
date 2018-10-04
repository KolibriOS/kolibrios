/*
 * Template C-- program.
*/

#define MEMSIZE 4096*15

#include "../lib/io.h"
#include "../lib/gui.h"

#define WIN_W 12*10+30
#define WIN_H 80

void main()
{
	word btn;
	loop() switch(WaitEvent())
	{
		case evButton:
			btn = GetButtonID();               
			if (btn == 1) ExitProcess();
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}


inline GetRevisionNumber()
{
	char buf[32];
	EAX = 18;
	EBX = 13;
	ECX = #buf;
	return ESDWORD[#buf+5];
}

void draw_window()
{
	system.color.get();
	DefineUnDragableWindow(screen.width-WIN_W-15, GetClientHeight()-WIN_H-15, WIN_W-1, WIN_H-1);
	DrawBar(0,0,WIN_W,WIN_H,0x414155);
	DrawWideRectangle(0,0,WIN_W,WIN_H, 3, 0x5555FF);
	WriteText(15, 20,    0x81, 0xFFFF55, " REV 7321 ");
	WriteText(15, 20+25, 0x81, 0xFFFF55, "17.09.2018");
}
