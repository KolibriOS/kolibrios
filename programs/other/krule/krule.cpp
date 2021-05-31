#include "kosSyst.h"
#include "kosFile.h"
#include "stdafx.h"
#include <stdio.h>

const char header[] = "Kolibri Rule";



void draw_window(void)
{
	int a=5;
	int b=10;
	// start redraw
	kos_WindowRedrawStatus(1);
	// define&draw window
	kos_DefineAndDrawWindow(10,40,360,68,
		0x33,0xFEF977,0,0,(Dword)header);
		while (a<355)
		{
			kos_DrawBar(a,0,1,30,0x000000);	
			a=a+10;
		}	
		while (b<350)
		{
			kos_DrawBar(b,0,1,20,0x000000);	
			b=b+10;
		}
		
	// end redraw
	kos_WindowRedrawStatus(2);
}

void kos_Main()
{
	draw_window();	
	for (;;)
	{
		switch (kos_WaitForEvent())
		{
		case 1:
			draw_window();			
			break;
		case 2:
			// key pressed, read it and ignore
			Byte keyCode;
			kos_GetKey(keyCode);
			break;
		case 3:
			// button pressed; we have only one button, close
			kos_ExitApp();
		}
	}
		
}