/*
 * Template C-- program
*/

#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/gui.h"

proc_info Form;

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

void draw_window()
{
	DefineAndDrawWindow(215, 100, 350, 300, 0x34, 0xEEEeee, "Window title",0);
	GetProcessInfo(#Form, SelfInfo);
}
